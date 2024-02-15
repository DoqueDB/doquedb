// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Replicator.cpp -- 非同期レプリケーションのスレーブ側
// 
// Copyright (c) 2014, 2015, 2016, 2017, 2023 Ricoh Company, Ltd.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Admin";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Admin/Replicator.h"

#include "Admin/Database.h"
#include "Admin/LogData.h"

#include "Checkpoint/Daemon.h"
#include "Checkpoint/LogData.h"

#include "Common/Assert.h"
#include "Common/AutoCaller.h"
#include "Common/Configuration.h"
#include "Common/InputArchive.h"
#include "Common/Message.h"
#include "Common/Request.h"
#include "Common/StringData.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/UnsignedInteger64Data.h"

#include "Communication/CryptMode.h"
#include "Communication/Socket.h"
#include "Communication/SocketDaemon.h"

#include "Client2/Connection.h"
#include "Client2/DataSource.h"
#include "Client2/Port.h"
#include "Client2/Session.h"

#include "Execution/Executor.h"
#include "Execution/Program.h"

#include "Opt/LogData.h"
#include "Opt/Optimizer.h"

#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"
#include "Os/SysConf.h"

#include "Schema/Database.h"
#include "Schema/Hold.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"

#include "Server/Manager.h"

#include "Trans/AutoLatch.h"
#include "Trans/AutoLogFile.h"
#include "Trans/AutoTransaction.h"
#include "Trans/LogData.h"
#include "Trans/Transaction.h"

#include "Exception/BadArgument.h"
#include "Exception/Cancel.h"
#include "Exception/CannotConnect.h"
#include "Exception/DatabaseNotAvailable.h"
#include "Exception/DatabaseNotFound.h"
#include "Exception/LogFileCorrupted.h"
#include "Exception/LogItemCorrupted.h"
#include "Exception/ModLibraryError.h"
#include "Exception/ReplicationAborted.h"
#include "Exception/ServerNotAvailable.h"
#include "Exception/NotSupported.h"		// 暗号化対応

#include "ModMap.h"
#include "ModMemory.h"
#include "ModOsDriver.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeOstrStream.h"

#define _MOD_EXCEPTION(e) \
	Exception::ModLibraryError(moduleName, srcFile, __LINE__, e)

_SYDNEY_USING
_SYDNEY_ADMIN_USING

namespace
{
	//
	//	TYPEDEF local
	//	_$$::_MapKey
	//
	typedef ModPair<ModUnicodeString, int>	_MapKey;
	
	//
	//	TYPEDEF local
	//	_$$::_ReplicatorMap
	//
	typedef ModMap<_MapKey, Replicator*, ModLess<_MapKey> >	_ReplicatorMap;
	
	//
	//	VARIABLE local
	//	_$$::_cReplicatorMap -- マスターサーバごとのレプリケーターのマップ
	//
	_ReplicatorMap _cReplicatorMap;

	//
	//	VARIABLE local
	//	_$$:_cReplicatorLatch -- 上記マップ操作のためのラッチ
	//
	Os::CriticalSection _cReplicatorLatch;

	//
	//	VARIABLE local
	//	_$$::_cWaitTime -- マスターに接続できなかった場合の待機時間(ミリ秒)
	//
	Common::Configuration::ParameterInteger
	_cWaitTime("Admin_ReplicatorWaitTime", 60 * 1000);
	
	//
	//	TYPEDEF local
	//	_$$::_TransactionInfo -- トランザクション情報
	//
	typedef Checkpoint::Log::CheckpointDatabaseData::TransactionInfo
		_TransactionInfo;

	//
	//	FUNCTION local
	//	_$$::_findTransaction -- トランザクションを探す
	//
	bool _findTransaction(const ModVector<_TransactionInfo>& log_,
						  Trans::Log::LSN beginLSN_)
	{
		// 毎回バカサーチだけど、
		// そんなに件数が多くないと思う
		
		ModVector<_TransactionInfo>::ConstIterator i = log_.begin();
		for (; i != log_.end(); ++i)
		{
			if ((*i)._beginLSN == beginLSN_)
				return true;
		}

		return false;
	}

	//
	//	VARIABLE local
	//	_$$::_cDoqueDb
	//	_$$::_cInprocess
	//
	const ModUnicodeString _cDoqueDb("doquedb");
	const ModUnicodeString _cInprocess("inprocess");

	//
	//	FUNCTION local
	//	_$$::_separate
	//
	void _separate(const ModUnicodeChar* p, ModSize len, ModUnicodeChar sep,
				   ModUnicodeString& first, ModUnicodeString& second)
	{
		first.clear();
		second.clear();
		
		const ModUnicodeChar* t = ModUnicodeCharTrait::find(p, sep, len);
		if (t == 0)
		{
			first.append(p, len);
		}
		else
		{
			first.append(p, static_cast<ModSize>(t - p));
			t += 1;
			second.append(t, len - first.getLength() - 1);
		}
	}

	//
	//	FUNCTION local
	//	_$$::_parseURL -- URLから必要な情報を切り出す
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	const ModUnicodeString& url_
	//		URL
	//	bool& isCrypt_
	//		暗号化通信か否か
	//	ModUnicodeString& user_
	//		ユーザ名
	//	ModUnicodeString& password_
	//		パスワード
	//	ModUnicodeString& host_
	//		ホスト名
	//	int& portNumber_
	//		ポート番号
	//	ModUnicodeString& database_
	//		データベース名(または、データベースID)
	//
	//	RETURN
	//	bool
	//		フォーマットが正しければtrue、それ以外の場合はfalse
	//
	//	EXCEPTIONS
	//
	bool
	_parseURL(const ModUnicodeString& url_,
			  bool& isCrypt_,
			  ModUnicodeString& user_,
			  ModUnicodeString& password_,
			  ModUnicodeString& host_,
			  int& portNumber_,
			  ModUnicodeString& database_)
	{
		bool inprocess = false;
		const ModUnicodeChar* p = url_;
	
		//
		//	マスターサーバのURLの表記は以下の通り(ただし、改行は含まない)
		//
		//	doquedb://
		//		[<ユーザ名>:<パスワード>@]<ホスト名>:<ポート番号>/
		//			(<データベース名>|<データベースID>)
		//
		//	テスト用のInProcess用の表記は以下の通り
		//
		//	inprocess://[<ユーザ名>:<パスワード>@]/(<データベース名>|<データベースID>)
		//

		//【注意】	ユーザが指定するのはデータベース名だが、
		//			認証後、スキーマファイルに保存されているのは
		//			データベースIDとなる

		//	プロトコルを確認
	
		if (url_.compare(_cDoqueDb, ModFalse,
							  _cDoqueDb.getLength()) == 0)
		{
			isCrypt_ = false;
			p += _cDoqueDb.getLength();
		}
		else if (url_.compare(_cInprocess, ModFalse,
							  _cInprocess.getLength()) == 0)
		{
			isCrypt_ = false;
			inprocess = true;
			p += _cInprocess.getLength();
		}
		else
		{
			return false;
		}

		if (*p != ':' || *(p+1) != '/' || *(p+2) != '/')
			return false;
	
		// データベース名とのセパレータ(/)を探す

		p += 3;
		const ModUnicodeChar* d = ModUnicodeCharTrait::find(p, '/');
		if (d == 0)
			return false;
		
		ModSize ulen = static_cast<ModSize>(d - p);
		d += 1;

		// ユーザ情報とのセパレータ(@)を探す

		const ModUnicodeChar* u = 0;
		ModSize hlen = ulen;
		const ModUnicodeChar* h = ModUnicodeCharTrait::find(p, '@', ulen);
		if (h == 0)
		{
			// ユーザ情報は格納されてない
			user_.clear();
			password_.clear();
			h = p;
			ulen = 0;
		}
		else
		{
			u = p;
			ulen = static_cast<ModSize>(h - p);
			hlen = hlen - ulen - 1;
			++h;
		}

		if (u)
		{
			// ユーザ名とパスワード
			_separate(u, ulen, ':', user_, password_);
		}

		// ホスト名とポート番号
		ModUnicodeString port;
		_separate(h, hlen, ':', host_, port);
		portNumber_ = ModUnicodeCharTrait::toInt(port);
		if (portNumber_ == 0)
			host_.clear();

		// データベース名(または、データベースID)
		database_.clear();
		database_.append(d);

		// エラーチェック
		if ((!inprocess && (host_.getLength() == 0 || portNumber_ == 0)) ||
			database_.getLength() == 0)
			return false;

		return true;
	}

	//
	//	FUNCTION local
	//	_$$::_createDataSource -- データソースを作成する
	//
	Client2::DataSource* _createDataSource(ModUnicodeString& host_,
										   int portNumber_)
	{
		Client2::DataSource* pDataSource = 0;
		
		if (host_.getLength())
		{
			// ソケット通信
		
			// DataSourceを得る
			Client2::DataSource::Family::Value family;
			switch (Communication::Socket::getFamily())
			{
			case Communication::Socket::Family::Only_v4:
				family = Client2::DataSource::Family::IPv4;
				break;
			case Communication::Socket::Family::Only_v6:
				family = Client2::DataSource::Family::IPv6;
				break;
			default:
				family = Client2::DataSource::Family::Unspec;
				break;
			}

			// データソース
			pDataSource
				= Client2::DataSource::createDataSource(host_,
														portNumber_,
														family,
														false);
		}
		else
		{
			// インプロセス通信

			pDataSource
				= Client2::DataSource::createDataSource(false);
		}

		return pDataSource;
	}
}

//
//	FUNCTION public
//	Admin::Replicator::Queue::Queue -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Replicator::Queue::Queue()
	: m_cQueue(), m_cLatch(), m_cEvent(Os::Event::Category::ManualReset),
	  m_bAborted(false)
{
}

//
//	FUNCTION public
//	Admin::Replicator::Queue::~Queue -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Replicator::Queue::~Queue()
{
	// キューの中身を解放する
	List::Iterator i = m_cQueue.begin();
	for (; i != m_cQueue.end(); ++i)
	{
		delete (*i).first;
	}
	m_cQueue.clear();
}

//
//	FUNCTION public
//	Admin::Replicator::Queue::pushBack -- 論理ログを書き出す
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Log::Data* pLog_
//		論理ログ
//	Trans::Log::LSN cLsn_
//		マスターの論理ログのLSN
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::Queue::pushBack(Trans::Log::Data* pLog_,
							Trans::Log::LSN cLsn_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	// リストの末尾に追加
	// 読みださなくてもどんどん追加していく
	
	m_cQueue.pushBack(
		ModPair<Trans::Log::Data*, Trans::Log::LSN>(pLog_, cLsn_));

	// 追加したので、イベントをシグナル化

	m_cEvent.set();
}

//
//	FUNCTION public
//	Admin::Replicator::Queue::popFront -- キューの先頭を取り出す
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Log::Data*& pLog_
//		キューの先頭に格納されていた論理ログ
//	Trans::Log::LSN& cLSN_
//		論理ログのマスターの論理ログのLSN
//
//	RETURN
//	bool
//		キューからデータが取得できた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Replicator::Queue::popFront(Trans::Log::Data*& pLog_,
							Trans::Log::LSN& cLSN_)
{
	Os::AutoTryCriticalSection cAuto(m_cLatch);
	cAuto.lock();

	while (true)
	{
		if (m_cQueue.getSize() != 0)
		{
			// キューにデータが格納されているので、先頭を取得する

			List::Iterator i = m_cQueue.begin();

			pLog_ = (*i).first;
			cLSN_ = (*i).second;

			m_cQueue.popFront();

			return true;
		}

		// キューにデータがないので、イベントをリセットして
		// イベントがシグナル化されるのを待つ

		m_cEvent.reset();
		cAuto.unlock();

		m_cEvent.wait();

		cAuto.lock();

		if (m_bAborted == true)
			// 終了
			break;
	}

	return false;
}

//
//	FUNCTION public
//	Admin::Replicator::Queue::abort -- キューを終了する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::Queue::abort()
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	if (m_bAborted == false)
	{
		// 終了フラグを立てる
	
		m_bAborted = true;

		// イベントをシグナル化して、待っている側に知らせる

		m_cEvent.set();
	}
}

//
//	FUNCTION public
//	Admin::Replicator::Queue::isAborted -- 終了しているか否か
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		終了している場合は true、それ以外の場合は false
//
//	EXCEPTIONS
//
bool
Replicator::Queue::isAborted()
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	return m_bAborted;
}

//
//	FUNCTION public
//	Admin::Replicator::Executor::Executor -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Admin::Replicator* pReplicator_
//		レプリケーター
//	Schema::ObjectID::Value uiSlaveDatabaseID_
//		スレーブ側のデータベースID
//	const ModUnicodeString& cSlaveDatabaseName_
//		スレーブ側のデータベース名
//	Schema::ObjectID::Value uiMasterDatabaseID_
//		マスター側のデータベースID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Replicator::Executor::Executor(Replicator* pReplicator_,
							   Schema::ObjectID::Value uiSlaveDatabaseID_,
							   const ModUnicodeString& cSlaveDatabaseName_,
							   Schema::ObjectID::Value uiMasterDatabaseID_)
	: m_pReplicator(pReplicator_),
	  m_uiMasterDatabaseID(uiMasterDatabaseID_),
	  m_uiDatabaseID(uiSlaveDatabaseID_),
	  m_cDatabaseName(cSlaveDatabaseName_),
	  m_bRestore(false), m_ulLastMasterLSN(Trans::Log::IllegalLSN),
	  m_bIsConnected(false)
{
}

//
//	FUNCTION public
//	Admin::Replicator::Executor::~Executor -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Replicator::Executor::~Executor()
{
}

//
//	FUNCTION public
//	Admin::Replicator::Executor::restoreTransaction
//		-- 終了時に実行途中だったトランザクションを復元する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Trans::Log::LSN
//		最後に受け取ったマスター側のLSN
//
//	EXCEPTIONS
//
Trans::Log::LSN
Replicator::Executor::restoreTransaction()
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	// チェックポイント処理を実行不可にする
		
	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::Executor);

	//【注意】	ここで開始するトランザクションは論理ログを読み出すだけの
	//			トランザクションであり、論理ログを再実行するトランザクション
	//			の邪魔をしないようにしないといけない
	//			通常のデータ操作と同じトランザクションであれば問題ないとの
	//			認識ににより、ここで実行する
	//			トランザクションのモードと、データベースのロックモードは
	//			Server::Worker のものを参考にした
	
	// トランザクションを開始する
	//
	//【注意】	データベースの論理ログを読み出すためのトランザクション
	//			復元するトランザクションとは別

	Trans::AutoTransaction	trans(Trans::Transaction::attach());
	trans->begin(Schema::ObjectID::SystemTable,
				 Trans::Transaction::Mode(
					 Trans::Transaction::Category::ReadWrite,
					 Trans::Transaction::IsolationLevel::Serializable,
					 Boolean::False),
				 true, true);

	// データベースを得る

	Schema::Database* pDatabase
		= Schema::Database::getLocked(*trans,  m_uiDatabaseID,
									  Lock::Name::Category::Tuple,
									  Schema::Hold::Operation::ReadForImport,
									  Lock::Name::Category::Tuple,
									  Schema::Hold::Operation::ReadForImport);
	if (pDatabase == 0)
	{
		// データベースが破棄されている
		ModUnicodeOstrStream tmp;
		tmp << m_cDatabaseName << "(ID:" << m_uiDatabaseID << ")";
		_SYDNEY_THROW1(Exception::DatabaseNotFound, tmp.getString());
	}

	//
	//	終了時に実行中だったトランザクションを再現する
	//	実行中だったトランザクションは論理ログに記録されているが、
	//	ロールバックされている
	//	そのため、記録されているトランザクションを再実行する必要がある
	//	ただし、再実行部分に関しては論理ログにログは書き出さない
	//

	// 再実行するトランザクションを得る
	
	m_ulLastMasterLSN = findRestoreLSN(*trans, pDatabase, m_mapRestoreLSN);
	if (m_ulLastMasterLSN == Trans::Log::IllegalLSN)
		_SYDNEY_THROW0(Exception::LogFileCorrupted);

	if (m_mapRestoreLSN.isEmpty() == ModTrue)
	{
		// 再実行が必要なトランザクションがないので終了

		trans->commit();
		return m_ulLastMasterLSN;
	}
	
	// 再実行が必要なトランザクションがあるので、
	// トランザクションを再実行する
		
	m_bRestore = true;	// 再実行中にする

	// 最初の論理ログのLSN

	Trans::Log::LSN lsn = (*(m_mapRestoreLSN.begin())).first;

	// 論理ログを得る
		
	Trans::Log::AutoFile logFile(pDatabase->getLogFile());
		
	while (lsn != Trans::Log::IllegalLSN)
	{
		// 論理ログを読み出す

		ModAutoPointer<Trans::Log::Data> data;
		{
			Trans::AutoLatch latch(*trans, logFile->getLockName());
			data = logFile->load(lsn);
		}
		if (!data.isOwner())
			// 最後まで読みだしたので終了
			break;

		if (data->getTimeStamp().isIllegal())
			// 不正なタイムスタンプが得られた
			_SYDNEY_THROW0(Exception::LogItemCorrupted);

		// 論理ログの末尾まで読み進める

		if (isRestoreLog(*data, lsn))
		{
			// 再現対象の論理ログだった
			
			switch (data->getCategory())
			{
			case Trans::Log::Data::Category::TransactionBegin:
				// トランザクションの開始
				transactionBegin(*data, lsn);
				break;
			case Trans::Log::Data::Category::TransactionCommit:
				// トランザクションの確定
				transactionCommit(*data, lsn);
				break;
			case Trans::Log::Data::Category::TransactionRollback:
				// トランザクションのロールバック
				transactionRollback(*data, lsn);
				break;
			case Trans::Log::Data::Category::StatementCommit:
				// SQL文の確定
				statementCommit(*data, lsn);
				break;
			case Trans::Log::Data::Category::StatementRollback:
				// SQL文のロールバック
				statementRollback(*data, lsn);
				break;
			case Trans::Log::Data::Category::TupleModify:
				// タプル操作
				tupleModify(*data, lsn);
				break;
			case Trans::Log::Data::Category::SchemaModify:
				// スキーマ操作
				schemaModify(*data, lsn);
				break;
			default:
				// その他は無視
				break;
			}
		}

		{
			// 次へ
			Trans::AutoLatch latch(*trans, logFile->getLockName());
			lsn = logFile->getNextLSN(lsn);
		}
	}

	// トランザクションを終了する

	trans->commit();

	// すべてのトランザクションを再現したので、
	// 論理ログを書くモードにする

	TransMap::Iterator i = m_mapTransaction.begin();
	for (; i != m_mapTransaction.end(); ++i)
	{
		(*i).second.m_pTransaction->setStoreLogFlag(true);
	}

	// 再実行中ではなくする

	m_bRestore = false;

	return m_ulLastMasterLSN;
}

//
//	FUNCTION private
//	Admin::Replicator::Executor::connectMaster
//		-- マスターデータベースに接続する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
// 	なし
//
//	EXCEPTIONS
//
void
Replicator::Executor::connectMaster(Client2::DataSource* pDataSource_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	
	if (m_ulLastMasterLSN == Trans::Log::IllegalLSN || m_bIsConnected)
		return;
	
	// クライアントコネクションを得る
	Client2::Connection* pClientConnection
		= pDataSource_->getClientConnection();
	
	// ワーカを起動する
	ModAutoPointer<Client2::Port> pPort = pClientConnection->beginWorker();
	
	//
	//	レプリケーションに必要な情報
	//
	// [<-] リクエスト
	Common::Request cRequest(Common::Request::TransferLogicalLog);
	pPort->writeObject(&cRequest);
	// [<-] データベースID
	Common::UnsignedIntegerData cDatabaseID(m_uiMasterDatabaseID);
	pPort->writeObject(&cDatabaseID);
	// [<-] マスター最終LSN
	Common::UnsignedInteger64Data cLSN(m_ulLastMasterLSN);
	pPort->writeObject(&cLSN);

	//
	//	スレーブの情報
	//
	// [<-] スレーブホスト名
	Common::StringData cHostName(Os::SysConf::HostName::get());
	pPort->writeObject(&cHostName);
	// [<-] スレーブポート番号
	Common::IntegerData
		cPortNumber(Communication::SocketDaemon::getPortNumber());
	pPort->writeObject(&cPortNumber);
	// [<-] スレーブデータベース名
	Common::StringData cDatabaseName(m_cDatabaseName);
	pPort->writeObject(&cDatabaseName);

	// 最後にflushする
	pPort->flush();

	try
	{
		//
		//	ステータス
		//
		// [->] ステータス
		pPort->readStatus();
	}
	catch (Exception::Object& e)
	{
		// エラーが発生
		SydErrorMessage << "Start Replication Error ["
						<< m_cDatabaseName << "]: "
						<< e << ModEndl;
		_SYDNEY_RETHROW;
	}

	// ポートを返す
	pDataSource_->pushPort(pPort.release());

	m_bIsConnected = true;
}

//
//	FUNCTION private
//	Admin::Replicator::Executor::disconnectMaster
//		-- マスターデータベースとの接続を切る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
// 	なし
//
//	EXCEPTIONS
//
void
Replicator::Executor::disconnectMaster(Client2::DataSource* pDataSource_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	
	if (m_bIsConnected == false)
		return;

	// クライアントコネクションを得る
	Client2::Connection* pClientConnection
		= pDataSource_->getClientConnection();
	
	// ワーカを起動する
	ModAutoPointer<Client2::Port> pPort = pClientConnection->beginWorker();
	
	//
	//	レプリケーションに必要な情報
	//
	// [<-] リクエスト
	Common::Request cRequest(Common::Request::StopTransferLogicalLog);
	pPort->writeObject(&cRequest);
	// [<-] データベースID
	Common::UnsignedIntegerData cDatabaseID(m_uiMasterDatabaseID);
	pPort->writeObject(&cDatabaseID);

	//
	//	スレーブの情報
	//
	// [<-] スレーブホスト名
	Common::StringData cHostName(Os::SysConf::HostName::get());
	pPort->writeObject(&cHostName);
	// [<-] スレーブポート番号
	Common::IntegerData
		cPortNumber(Communication::SocketDaemon::getPortNumber());
	pPort->writeObject(&cPortNumber);
	// [<-] スレーブデータベース名
	Common::StringData cDatabaseName(m_cDatabaseName);
	pPort->writeObject(&cDatabaseName);

	// 最後にflushする
	pPort->flush();

	//
	//	ステータス
	//
	// [->] ステータス
	pPort->readStatus();
	
	// ポートを返す
	pDataSource_->pushPort(pPort.release());

	m_bIsConnected = false;
}

//
//	FUNCTION public
//	Admin::Replicator::Executor::makeLog -- 論理ログを作成する
//
//	NOTES
//
//	ARGUMENTS
//	Admin::Log::ReplicationEndData& cLogData_
//		論理ログ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::Executor::makeLog(Log::ReplicationEndData& cLogData_)
{
	Os::AutoCriticalSection latch(m_cLatch);
	
	cLogData_.setLastMasterLSN(m_ulLastMasterLSN);
	
	TransMap::Iterator i = m_mapTransaction.begin();
	for (; i != m_mapTransaction.end(); ++i)
	{
		Trans::Transaction& trans = *((*i).second.m_pTransaction);

		const Trans::Log::Info& logInfo
			= trans.getLogInfo(Trans::Log::File::Category::Database);
		
		if (logInfo.getBeginTransactionLSN() != Trans::Log::IllegalLSN)
			
			// スレーブの論理ログに記録されているので、
			// 実行中トランザクションとする
			
			cLogData_.pushBackBeginLSN(logInfo.getBeginTransactionLSN());
	}
}

//
//	FUNCTION public
//	Admin::Replicator::Executor::storeLog
//		-- スレーブ終了の論理ログを書き出す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::Executor::storeLog()
{
	// チェックポイント処理を実行不可にする
		
	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::Executor);

	// トランザクションを開始する

	Trans::AutoTransaction	trans(Trans::Transaction::attach());
	trans->begin(Schema::ObjectID::SystemTable,
				 Trans::Transaction::Mode(
					 Trans::Transaction::Category::ReadWrite,
					 Trans::Transaction::IsolationLevel::Serializable,
					 Boolean::False),
				 true, true);

	// データベースを得る

	Schema::Database* pDatabase
		= Schema::Database::getLocked(*trans,  m_uiDatabaseID,
									  Lock::Name::Category::Tuple,
									  Schema::Hold::Operation::ReadForWrite,
									  Lock::Name::Category::Tuple,
									  Schema::Hold::Operation::ReadForWrite);
	if (pDatabase == 0)
	{
		// データベースが破棄されている
		ModUnicodeOstrStream tmp;
		tmp << m_cDatabaseName << "(ID:" << m_uiDatabaseID << ")";
		_SYDNEY_THROW1(Exception::DatabaseNotFound, tmp.getString());
	}

	// 論理ログファイルを得る

	Trans::Log::AutoFile logFile(pDatabase->getLogFile());

	// 論理ログを準備する

	Log::ReplicationEndData data;
	makeLog(data);

	{
		// 論理ログを書き出す

		Trans::AutoLatch latch(*trans, logFile->getLockName());
		logFile->store(data);
	}

	// トランザクションを終了する
	trans->commit();
}

//
//	FUNCTION public
//	Admin::Replicator::Executor::rollbackAll
//		-- 実行中のトランザクションをすべてロールバックする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETRURN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::Executor::rollbackAll()
{
	Os::AutoCriticalSection latch(m_cLatch);
	
	// 実行途中のトランザクションをロールバックする
	// ただし、再起動時にトランザクションを再実行するので、
	// ロールバックログは記録しない

	TransMap::Iterator i = m_mapTransaction.begin();
	for (; i != m_mapTransaction.end(); ++i)
	{
		Trans::Transaction* trans = (*i).second.m_pTransaction;

		// 論理ログは記録しない
		
		trans->setStoreLogFlag(false);

		// ロールバックする

		trans->rollback();
		Trans::Transaction::detach(trans);
	}

	m_mapTransaction.erase(m_mapTransaction.begin(),
						   m_mapTransaction.end());
}

//
//	FUNCTION private
//	Admin::Replicator::Executor::runnable -- スレッドして実行されるメソッド
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCPETIONS
//	
void
Replicator::Executor::runnable()
{
	{
		Os::AutoCriticalSection cAuto(m_pReplicator->getLatch());
		
		try
		{
			// 終了時に実行中だったトランザクションを復元する
		
			restoreTransaction();
			
			if (m_pReplicator->isRunning())
			{
				// レプリケーター実行中なので、
				// ここで connetMaster を実行する

				connectMaster(m_pReplicator->getDataSource());
			}
		}
		catch (Exception::Object& e)
		{
			SydErrorMessage << e << ModEndl;

			// 続行不可能なため、データベースを利用不可とする

			SydErrorMessage	<< "[" << m_cDatabaseName
							<< "(ID:" << m_uiDatabaseID << ")] "
							<< "Database is not available." << ModEndl;
			Schema::Database::setAvailability(m_uiDatabaseID, false);
			
			_SYDNEY_RETHROW;
		}
		catch (...)
		{
			// 続行不可能なため、データベースを利用不可とする

			SydErrorMessage	<< "[" << m_cDatabaseName
							<< "(ID:" << m_uiDatabaseID << ")] "
							<< "Database is not available." << ModEndl;
			Schema::Database::setAvailability(m_uiDatabaseID, false);
			
			_SYDNEY_RETHROW;
		}
	}

	SydMessage << "[" << m_cDatabaseName
			   << "(ID:" << m_uiDatabaseID << ")] "
			   << "Replication Start" << ModEndl;

	try
	{
		for (;;)
		{
			Trans::Log::Data* pLog;
			Trans::Log::LSN masterLSN;
			
			// キューから論理ログを取り出す
			
			bool r  = m_cQueue.popFront(pLog, masterLSN);
			
			if (r == false)
				// 終了
				break;

			try
			{
				// 論理ログを実行する
			
				redo(*pLog, masterLSN);
			}
			catch (Exception::Cancel&)
			{
				// ロック待ちしているときに、スレッドに対して
				// 終了要求が来ると、この例外が送出される

				delete pLog;
				break;
			}

			// 取り出した論理ログを削除する

			delete pLog;

			// 最後のマスター側の論理ログの LSN を保存する

			m_ulLastMasterLSN = masterLSN;
		}
	}
	catch (Exception::Cancel&)
	{
		// 終了なので、何もしない
		;
	}
	catch (Exception::Object& e)
	{
		// 続行不可能なエラーが発生した
		// これ以上の更新や、ログ出力を抑制するため、
		// データベースを利用不可とする

		SydErrorMessage << e << ModEndl;
		SydErrorMessage	<< "[" << m_cDatabaseName
						<< "(ID:" << m_uiDatabaseID << ")] "
						<< "Database is not available." << ModEndl;
		Schema::Database::setAvailability(m_uiDatabaseID, false);
			
		_SYDNEY_RETHROW;
	}
	catch (ModException& e)
	{
		// 続行不可能なエラーが発生した
		// これ以上の更新や、ログ出力を抑制するため、
		// データベースを利用不可とする

		SydErrorMessage << Exception::ModLibraryError(moduleName,
													  srcFile, __LINE__, e)
						<< ModEndl;
		SydErrorMessage	<< "[" << m_cDatabaseName
						<< "(ID:" << m_uiDatabaseID << ")] "
						<< "Database is not available." << ModEndl;
		Schema::Database::setAvailability(m_uiDatabaseID, false);
			
		_SYDNEY_RETHROW;
	}
	catch (std::exception& e)
	{
		// 続行不可能なエラーが発生した
		// これ以上の更新や、ログ出力を抑制するため、
		// データベースを利用不可とする
		
		SydErrorMessage << "std::exception occurred. "
						<< (e.what() ? e.what() : "") << ModEndl;
		SydErrorMessage	<< "[" << m_cDatabaseName
						<< "(ID:" << m_uiDatabaseID << ")] "
						<< "Database is not available." << ModEndl;
		Schema::Database::setAvailability(m_uiDatabaseID, false);
			
		_SYDNEY_RETHROW;
	}
	catch (...)
	{
		// 続行不可能なエラーが発生した
		// これ以上の更新や、ログ出力を抑制するため、
		// データベースを利用不可とする

		SydErrorMessage << "Unexpected exception occurred." << ModEndl;
		SydErrorMessage	<< "[" << m_cDatabaseName
						<< "(ID:" << m_uiDatabaseID << ")] "
						<< "Database is not available." << ModEndl;
		Schema::Database::setAvailability(m_uiDatabaseID, false);
			
		_SYDNEY_RETHROW;
	}
	
	SydMessage << "[" << m_cDatabaseName
			   << "(ID:" << m_uiDatabaseID << ")] "
			   << "Replication End" << ModEndl;
}

//
//	FUNCTION private
//	Admin::Replicator::Executor::redo -- 論理ログからredoする
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Log::Data& cLog_
//		論理ログ
//	Trans::Log::LSN uiLSN_
//		マスターサーバのLSN
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::Executor::redo(Trans::Log::Data& cLog_,
						   Trans::Log::LSN uiLSN_)
{
	try
	{
		// 論理ログに書かれているマスターサーバのタイムスタンプをクリアする

		cLog_.clearTimeStamp();
		
		// チェックポイント処理を実行不可にする
		
		Checkpoint::Daemon::AutoDisabler
			disabler(Checkpoint::Daemon::Category::Executor);

		switch (cLog_.getCategory())
		{
		case Trans::Log::Data::Category::TransactionBegin:
			// トランザクションの開始
			transactionBegin(cLog_, uiLSN_);
			break;
		case Trans::Log::Data::Category::TransactionCommit:
			// トランザクションの確定
			transactionCommit(cLog_, uiLSN_);
			break;
		case Trans::Log::Data::Category::TransactionRollback:
			// トランザクションのロールバック
			transactionRollback(cLog_, uiLSN_);
			break;
		case Trans::Log::Data::Category::StatementCommit:
			// SQL文の確定
			statementCommit(cLog_, uiLSN_);
			break;
		case Trans::Log::Data::Category::StatementRollback:
			// SQL文のロールバック
			statementRollback(cLog_, uiLSN_);
			break;
		case Trans::Log::Data::Category::CheckpointDatabase:
			// チェックポイント処理
			checkpointDatabase(cLog_, uiLSN_);
			break;
		case Trans::Log::Data::Category::TupleModify:
			// タプル操作
			tupleModify(cLog_, uiLSN_);
			break;
		case Trans::Log::Data::Category::SchemaModify:
			// スキーマ操作
			schemaModify(cLog_, uiLSN_);
			break;
		default:
			// その他は無視
			break;
		}
	}
	catch (Exception::Cancel&)
	{
		// 終了要求が来ているので、終了する

		_SYDNEY_RETHROW;
	}
	catch (Exception::UserLevel& e)
	{
		// ユーザレベルの例外の場合は、
		// ログだけ出力して、続行する

		SydInfoMessage << "[" << m_cDatabaseName
					   << "(ID:" << m_uiDatabaseID << ")] "
					   << e << ModEndl;
	}
	catch (...)
	{
		// その他の例外は、上位へ再送し、スレッドは停止する
		//
		//【注意】
		//	続行できる例外がある場合には、個別に catch する必要がある
		
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION private
//	Admin::Replicator::Executor::transactionBegin -- トランザクションを開始する
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Log::Data& cLog_
//		論理ログ
//	Trans::Log::LSN uiLSN_
//		マスターサーバのLSN
//
//	REUTRN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::Executor::transactionBegin(Trans::Log::Data& cLog_,
									   Trans::Log::LSN uiLSN_)
{
	Trans::Transaction* pTransaction
		= Trans::Transaction::attach();

	try
	{
		// トランザクションを開始する
		//
		// この時点ではまだログは書き出さない
		// スレーブ側のログを書くのは、何か操作が行われる直前である
		//
		// マスター側のトランザクションの開始を表す論理ログも
		// 何か操作が行われる直前に書き出される
		// その操作の論理ログの書き出しと、トランザクション開始を表す論理ログの
		// 書き出しは、アトミックであり、その間に他の論理ログが書き出される
		// ことはない
		
		pTransaction->begin(m_uiDatabaseID,
							Trans::Transaction::Category::ReadWrite);

		if (m_bRestore)
		{
			// トランザクション再現中なので、ログは書かない

			pTransaction->setStoreLogFlag(false);

			// スレーブ側のLSNをマスター側のLSNに変換する

			RestoreMap::Iterator i = m_mapRestoreLSN.find(uiLSN_);
			if (i == m_mapRestoreLSN.end())
				// ありえない
				_SYDNEY_THROW0(Exception::LogFileCorrupted);

			uiLSN_ = (*i).second;
		}

		// マスターサーバのトランザクション開始のLSNを設定する

		pTransaction->getLogInfo(Trans::Log::File::Category::Database)
			.setMasterBeginTransactionLSN(uiLSN_);

		Os::AutoCriticalSection latch(m_cLatch);

		// これから来るログを redo する時に、
		// トランザクションを参照できるように、マップに登録する

		m_mapTransaction.insert(uiLSN_, TransInfo(pTransaction));
	}
	catch (...)
	{
		// エラーが起きたらどうする？
		
		Trans::Transaction::detach(pTransaction);
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION private
//	Admin::Replicator::Executor::transactionCommit -- トランザクションを確定する
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Log::Data& cLog_
//		論理ログ
//	Trans::Log::LSN uiLSN_
//		マスターサーバのLSN
//		リカバリモードの場合はスレーブ側のLSN
//
//	REUTRN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::Executor::transactionCommit(Trans::Log::Data& cLog_,
										Trans::Log::LSN uiLSN_)
{
	// マスター側のトランザクション開始時のLSNを得る
	
	Trans::Log::LSN masterLSN = getMasterLSN(cLog_);
	if (masterLSN == Trans::Log::IllegalLSN)
	{
		// 通常あり得ない
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	
	Os::AutoCriticalSection latch(m_cLatch);
	
	// マスターサーバのトランザクションに対応する
	// スレーブ側のトランザクション記述子を得る

	TransMap::Iterator i = m_mapTransaction.find(masterLSN);
	if (i == m_mapTransaction.end())
	{
		// スレーブ側では実行しないスキーマ操作などの場合
		// スレーブ側では論理ログが記録されない
		// そのため、トランザクションが自動リカバリ後に再現されない場合がある
		//
		// 記録のため、ログ出力する

		SydMessage << "Skip transaction commit." << ModEndl;
		
		return;
	}

	if (m_bRestore)
	{
		// トランザクション内のLSN情報を再現する

		(*i).second.m_pTransaction->restoreLog(
			Trans::Log::TransactionCommitData(), uiLSN_);
	}
	else
	{
		// コミットで初めて論理ログを参照する可能性があるので、
		// トランザクションに論理ログを設定する

		// データベースを得る

		Schema::Database* pDatabase
			= Schema::Database::getLocked(
				*((*i).second.m_pTransaction),
				m_uiDatabaseID,
				Lock::Name::Category::Tuple,
				Schema::Hold::Operation::ReadForImport,
				Lock::Name::Category::Tuple,
				Schema::Hold::Operation::ReadForImport);
		if (pDatabase == 0)
		{
			// データベースが破棄されている
			ModUnicodeOstrStream tmp;
			tmp << m_cDatabaseName << "(ID:" << m_uiDatabaseID << ")";
			_SYDNEY_THROW1(Exception::DatabaseNotFound, tmp.getString());
		}

		// キャッシュが破棄されないように open する
		pDatabase->open();
		// スコープを抜けるときにデータベースのcloseを呼ぶ
		Common::AutoCaller1<Schema::Database, bool>
			autoCloser(pDatabase, &Schema::Database::close, false);

		// トランザクションに論理ログを設定する
		(*i).second.m_pTransaction->setLog(*pDatabase);
	}
	
	// トランザクションを確定する
	//
	//【注意】	リカバリモードのときは uiLSN_ はスレーブ側のLSNだが、
	//			ログには記録されないので、このままでも問題ない
	
	(*i).second.m_pTransaction->commit((*i).second.m_bSchemaModify, uiLSN_);

	// トランザクション記述子を削除する
	
	Trans::Transaction* p = (*i).second.m_pTransaction;
	Trans::Transaction::detach(p);
	m_mapTransaction.erase(i);
}

//
//	FUNCTION private
//	Admin::Replicator::Executor::transactionRollback
//		-- トランザクションをロールバックする
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Log::Data& cLog_
//		論理ログ
//	Trans::Log::LSN uiLSN_
//		マスターサーバのLSN
//		リカバリモードの場合はスレーブ側のLSN
//
//	REUTRN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::Executor::transactionRollback(Trans::Log::Data& cLog_,
										  Trans::Log::LSN uiLSN_)
{
	// マスター側のトランザクション開始時のLSNを得る
	
	Trans::Log::LSN masterLSN = getMasterLSN(cLog_);
	if (masterLSN == Trans::Log::IllegalLSN)
	{
		// 通常あり得ない
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	
	Os::AutoCriticalSection latch(m_cLatch);
	
	// マスターサーバのトランザクションに対応する
	// スレーブ側のトランザクション記述子を得る

	TransMap::Iterator i = m_mapTransaction.find(masterLSN);
	if (i == m_mapTransaction.end())
	{
		// スレーブ側では実行しないスキーマ操作などの場合
		// スレーブ側では論理ログが記録されない
		// そのため、トランザクションが自動リカバリ後に再現されない場合がある
		//
		// 記録のため、ログ出力する

		SydMessage << "Skip transaction rollback." << ModEndl;
		
		return;
	}

	if (m_bRestore)
	{
		// トランザクション内のLSN情報を再現する

		(*i).second.m_pTransaction->restoreLog(
			Trans::Log::TransactionRollbackData(), uiLSN_);
	}
	else
	{
		// ロールバックで初めて論理ログを参照する可能性があるので、
		// トランザクションに論理ログを設定する

		// データベースを得る

		Schema::Database* pDatabase
			= Schema::Database::getLocked(
				*((*i).second.m_pTransaction),
				m_uiDatabaseID,
				Lock::Name::Category::Tuple,
				Schema::Hold::Operation::ReadForImport,
				Lock::Name::Category::Tuple,
				Schema::Hold::Operation::ReadForImport);
		if (pDatabase == 0)
		{
			// データベースが破棄されている
			ModUnicodeOstrStream tmp;
			tmp << m_cDatabaseName << "(ID:" << m_uiDatabaseID << ")";
			_SYDNEY_THROW1(Exception::DatabaseNotFound, tmp.getString());
		}

		// キャッシュが破棄されないように open する
		pDatabase->open();
		// スコープを抜けるときにデータベースのcloseを呼ぶ
		Common::AutoCaller1<Schema::Database, bool>
			autoCloser(pDatabase, &Schema::Database::close, false);

		// トランザクションに論理ログを設定する
		(*i).second.m_pTransaction->setLog(*pDatabase);
	}
	
	
	// トランザクションをロールバックする
	//
	//【注意】	リカバリモードのときは uiLSN_ はスレーブ側のLSNだが、
	//			ログには記録されないので、このままでも問題ない
	
	(*i).second.m_pTransaction->rollback(uiLSN_);

	// トランザクション記述子を削除する
	
	Trans::Transaction* p = (*i).second.m_pTransaction;
	Trans::Transaction::detach(p);
	m_mapTransaction.erase(i);
}

//
//	FUNCTION private
//	Admin::Replicator::Executor::statementCommit
//		-- SQL文を確定する
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Log::Data& cLog_
//		論理ログ
//	Trans::Log::LSN uiLSN_
//		マスターサーバのLSN
//
//	REUTRN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::Executor::statementCommit(Trans::Log::Data& cLog_,
									  Trans::Log::LSN uiLSN_)
{
	// マスター側のトランザクション開始時のLSNを得る
	
	Trans::Log::LSN masterLSN = getMasterLSN(cLog_);
	if (masterLSN == Trans::Log::IllegalLSN)
	{
		// 通常あり得ない
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	
	Os::AutoCriticalSection latch(m_cLatch);
	
	// マスターサーバのトランザクションに対応する
	// スレーブ側のトランザクション記述子を得る

	TransMap::Iterator i = m_mapTransaction.find(masterLSN);
	if (i == m_mapTransaction.end())
	{
		// 通常あり得ない
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// SQL文を確定する
	
	(*i).second.m_pTransaction->commitStatement(uiLSN_);
	
	if (m_bRestore)
	{
		// トランザクション内のLSN情報を再現する

		(*i).second.m_pTransaction->restoreLog(
			Trans::Log::StatementCommitData(), uiLSN_);
	}
}

//
//	FUNCTION private
//	Admin::Replicator::Executor::statementRollback
//		-- SQL文をロールバックする
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Log::Data& cLog_
//		論理ログ
//	Trans::Log::LSN uiLSN_
//		マスターサーバのLSN
//		リカバリモードの場合はスレーブ側のLSN
//
//	REUTRN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::Executor::statementRollback(Trans::Log::Data& cLog_,
										Trans::Log::LSN uiLSN_)
{
	// マスター側のトランザクション開始時のLSNを得る
	
	Trans::Log::LSN masterLSN = getMasterLSN(cLog_);
	if (masterLSN == Trans::Log::IllegalLSN)
	{
		// 通常あり得ない
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	
	Os::AutoCriticalSection latch(m_cLatch);
	
	// マスターサーバのトランザクションに対応する
	// スレーブ側のトランザクション記述子を得る

	TransMap::Iterator i = m_mapTransaction.find(masterLSN);
	if (i == m_mapTransaction.end())
	{
		// 通常あり得ない
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// SQL文をロールバックする
	
	(*i).second.m_pTransaction->rollbackStatement(uiLSN_);

	if (m_bRestore)
	{
		// トランザクション内のLSN情報を再現する

		(*i).second.m_pTransaction->restoreLog(
			Trans::Log::StatementRollbackData(), uiLSN_);
	}
}

//
//	FUNCTION private
//	Admin::Replicator::Executor::checkpointDatabase
//		-- チェックポイント処理
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Log::Data& cLog_
//		論理ログ
//	Trans::Log::LSN uiLSN_
//		マスターサーバのLSN
//
//	REUTRN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::Executor::checkpointDatabase(Trans::Log::Data& cLog_,
										 Trans::Log::LSN uiLSN_)
{
	// CheckpointDatabaseの論理ログには、チェックポイント実行時に
	// 実行中のトランザクションが格納されている
	// 
	// こちらのマップ内に、ログに格納されていないトランザクションがあった場合、
	// そのトランザクションはロールバックする必要がある
	//
	// これは自動リカバリ等で、トランザクションがロールバックされた場合に
	// 発生する

	const Checkpoint::Log::CheckpointDatabaseData& cLog
		= _SYDNEY_DYNAMIC_CAST(const Checkpoint::Log::CheckpointDatabaseData&,
							   cLog_);

	// チェックポイント処理時に実行中のトランザクションの一覧を得る
	
	const ModVector<_TransactionInfo>& transInfo = cLog.getTransactionInfo();

	Os::AutoCriticalSection latch(m_cLatch);
	
	if (m_mapTransaction.getSize() != 0)
	{
		// スレーブ側のトランザクション記述子を得て、ログにないものは削除する
		
		TransMap::Iterator i = m_mapTransaction.begin();

		do
		{
			Trans::Log::LSN beginLSN = (*i).first;
			Trans::Transaction* pTrans = (*i).second.m_pTransaction;

			++i;	// 次へ
			
			if (_findTransaction(transInfo, beginLSN) == false)
			{
				// ロールバックで初めて論理ログを参照する可能性があるので、
				// トランザクションに論理ログを設定する

				// データベースを得る

				Schema::Database* pDatabase
					= Schema::Database::getLocked(
						*pTrans,
						m_uiDatabaseID,
						Lock::Name::Category::Tuple,
						Schema::Hold::Operation::ReadForImport,
						Lock::Name::Category::Tuple,
						Schema::Hold::Operation::ReadForImport);
				if (pDatabase == 0)
				{
					// データベースが破棄されている
					ModUnicodeOstrStream tmp;
					tmp << m_cDatabaseName << "(ID:" << m_uiDatabaseID << ")";
					_SYDNEY_THROW1(Exception::DatabaseNotFound,
								   tmp.getString());
				}

				// キャッシュが破棄されないように open する
				pDatabase->open();
				// スコープを抜けるときにデータベースのcloseを呼ぶ
				Common::AutoCaller1<Schema::Database, bool>
					autoCloser(pDatabase, &Schema::Database::close, false);

				// トランザクションに論理ログを設定する
				pTrans->setLog(*pDatabase);
		
				// 存在していないので、トランザクションをロールバックする

				pTrans->rollback(uiLSN_);

				// トランザクション記述子を削除する

				Trans::Transaction::detach(pTrans);
				m_mapTransaction.erase(beginLSN);
			}
		}
		while (i != m_mapTransaction.end());
	}
}

//
//	FUNCTION private
//	Admin::Replicator::Executor::tupleModify
//		-- タプルを操作する
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Log::Data& cLog_
//		論理ログ
//	Trans::Log::LSN uiLSN_
//		マスターサーバのLSN
//		リカバリモードの場合はスレーブ側のLSN
//
//	REUTRN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::Executor::tupleModify(Trans::Log::Data& cLog_,
								  Trans::Log::LSN uiLSN_)
{
	// マスター側のトランザクション開始時のLSNを得る
	
	Trans::Log::LSN masterLSN = getMasterLSN(cLog_);
	if (masterLSN == Trans::Log::IllegalLSN)
	{
		// 通常あり得ない
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	
	Os::AutoCriticalSection latch(m_cLatch);
	
	// マスターサーバのトランザクションに対応する
	// スレーブ側のトランザクション記述子を得る

	TransMap::Iterator i = m_mapTransaction.find(masterLSN);
	if (i == m_mapTransaction.end())
	{
		// 通常あり得ない
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// データベースを得る

	Schema::Database* pDatabase
		= Schema::Database::getLocked(*((*i).second.m_pTransaction),
									  m_uiDatabaseID,
									  Lock::Name::Category::Tuple,
									  Schema::Hold::Operation::ReadForImport,
									  Lock::Name::Category::Tuple,
									  Schema::Hold::Operation::ReadForImport);
	if (pDatabase == 0)
	{
		// データベースが破棄されている
		ModUnicodeOstrStream tmp;
		tmp << m_cDatabaseName << "(ID:" << m_uiDatabaseID << ")";
		_SYDNEY_THROW1(Exception::DatabaseNotFound, tmp.getString());
	}

	// キャッシュが破棄されないように open する
	pDatabase->open();
	// スコープを抜けるときにデータベースのcloseを呼ぶ
	Common::AutoCaller1<Schema::Database, bool>
		autoCloser(pDatabase, &Schema::Database::close, false);

	// トランザクションに論理ログを設定する
	(*i).second.m_pTransaction->setLog(*pDatabase);

	if (m_bRestore)
	{
		// このログはスレーブの論理ログに記録されているログ
		
		Opt::LogData& tupleLog
			= _SYDNEY_DYNAMIC_CAST(Opt::LogData&, cLog_);

		// トランザクションを復元する

		if ((*i).second.m_pTransaction->isLogStored(
				Trans::Log::File::Category::Database) == false)
		{
			// まだトランザクションが開始されていないので、
			// トランザクションを開始するログを設定する

			(*i).second.m_pTransaction
				->restoreLog(Trans::Log::TransactionBeginData(),
							 tupleLog.getBeginTransactionLSN());
		}

		// 論理ログのLSNを復元する

		(*i).second.m_pTransaction->restoreLog(cLog_, uiLSN_);
	}
	else
	{
		// WAL(Write Ahead Log)なので、まずは論理ログを書く
		
		(*i).second.m_pTransaction
			->storeLog(Trans::Log::File::Category::Database, cLog_, uiLSN_);
	}

	//
	// 論理ログの内容を反映する
	// 基本的には、Admin::Transaction::redoTuple と同じだが、
	// rollback が必要になる場合があるので、論理ログは記録する
	//

	Execution::Program program;

	try {
		// REDO する操作を実行するためのプログラムをオプティマイザに生成させる
		
		Opt::Optimizer().rollforward(
			pDatabase, &program, &cLog_, (*i).second.m_pTransaction);

		// エグゼキュータを起動し、生成したプログラムを実行させる

		Execution::Executor().execute(program);

	}
	catch (...)
	{
		// ログを記録する
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION private
//	Admin::Replicator::Executor::schemaModify
//		-- タプルを操作する
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Log::Data& cLog_
//		論理ログ
//	Trans::Log::LSN uiLSN_
//		マスターサーバのLSN
//		リカバリモードの場合はスレーブ側のLSN
//
//	REUTRN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::Executor::schemaModify(Trans::Log::Data& cLog_,
								   Trans::Log::LSN uiLSN_)
{
	// 論理ログの中身を確認し、実行するものだけにする
	// 実行するのは、データベース内の操作のみで、かつ、エリア操作以外

	const Schema::LogData& cLogData
		= _SYDNEY_DYNAMIC_CAST(const Schema::LogData&, cLog_);

	Schema::Hold::Operation::Value metadb
		= Schema::Hold::Operation::Unknown;

	bool execute = false;
	
	switch (cLogData.getSubCategory())
	{
	case Schema::LogData::Category::CreateTable:
	case Schema::LogData::Category::CreateIndex:
		// 実行するもの
		metadb = Schema::Hold::Operation::ReadForWrite;
		execute = true;
		break;

	case Schema::LogData::Category::DropTable:
	case Schema::LogData::Category::RenameTable:
	case Schema::LogData::Category::AddColumn:
	case Schema::LogData::Category::AlterColumn:
	case Schema::LogData::Category::AddConstraint:
	case Schema::LogData::Category::DropConstraint:
	case Schema::LogData::Category::DropIndex:
	case Schema::LogData::Category::RenameIndex:
	case Schema::LogData::Category::CreatePrivilege:
	case Schema::LogData::Category::DropPrivilege:
	case Schema::LogData::Category::AlterPrivilege:
		// 実行するもの
		metadb = Schema::Hold::Operation::Drop;
		execute = true;
		break;
		
	case Schema::LogData::Category::CreateCascade:
	case Schema::LogData::Category::DropCascade:
	case Schema::LogData::Category::AlterCascade:
	case Schema::LogData::Category::CreatePartition:
	case Schema::LogData::Category::DropPartition:
	case Schema::LogData::Category::AlterPartition:
	case Schema::LogData::Category::CreateFunction:
	case Schema::LogData::Category::DropFunction:
		// 分散関係のものはエラーかな？
		// でも、ここでエラーにしてもマスターには通知できないし...
		return;
		
	default:
		if (!m_bRestore)
			// 実行しない
			return;
		
		// 実行しない
		metadb = Schema::Hold::Operation::ReadForImport;
	}
	
	// マスター側のトランザクション開始時のLSNを得る
	
	Trans::Log::LSN masterLSN = getMasterLSN(cLog_);
	if (masterLSN == Trans::Log::IllegalLSN)
	{
		// 通常あり得ない
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	
	Os::AutoCriticalSection latch(m_cLatch);
	
	// マスターサーバのトランザクションに対応する
	// スレーブ側のトランザクション記述子を得る

	TransMap::Iterator i = m_mapTransaction.find(masterLSN);
	if (i == m_mapTransaction.end())
	{
		// 通常あり得ない
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// データベースを得る

	Schema::Database* pDatabase
		= Schema::Database::getLocked(*((*i).second.m_pTransaction),
									  m_uiDatabaseID,
									  Lock::Name::Category::Tuple,
									  metadb,
									  Lock::Name::Category::Tuple,
									  Schema::Hold::Operation::ReadForWrite);
	if (pDatabase == 0)
	{
		// データベースが破棄されている
		ModUnicodeOstrStream tmp;
		tmp << m_cDatabaseName << "(ID:" << m_uiDatabaseID << ")";
		_SYDNEY_THROW1(Exception::DatabaseNotFound, tmp.getString());
	}

	// キャッシュが破棄されないように open する
	pDatabase->open();
	// スコープを抜けるときにデータベースのcloseを呼ぶ
	Common::AutoCaller1<Schema::Database, bool>
		autoCloser(pDatabase, &Schema::Database::close, false);

	// トランザクションに論理ログを設定する
	(*i).second.m_pTransaction->setLog(*pDatabase);

	if (m_bRestore)
	{
		// このログはスレーブの論理ログに記録されているログ
		
		Schema::LogData& schemaLog
			= _SYDNEY_DYNAMIC_CAST(Schema::LogData&, cLog_);

		// トランザクションを復元する

		if ((*i).second.m_pTransaction->isLogStored(
				Trans::Log::File::Category::Database) == false)
		{
			// まだトランザクションが開始されていないので、
			// トランザクションを開始するログを設定する

			(*i).second.m_pTransaction
				->restoreLog(Trans::Log::TransactionBeginData(),
							 schemaLog.getBeginTransactionLSN());
		}

		// 論理ログのLSNを復元する

		(*i).second.m_pTransaction->restoreLog(cLog_, uiLSN_);

		if (execute == false)
			// 実行しない
			return;
	}
	else
	{
		// WAL(Write Ahead Log)なので、まずは論理ログを書く
		
		(*i).second.m_pTransaction
			->storeLog(Trans::Log::File::Category::Database, cLog_, uiLSN_);
	}

	try
	{
		// スキーマ操作をREDOするためにはUNDOする必要がある
		
		// UNDOする

		Schema::Manager::SystemTable::undo(*((*i).second.m_pTransaction),
										   cLogData,
										   m_cDatabaseName, true, true);
		
		// REDOする
		
		ModAutoPointer<Recovery::Database>	dbRecovery;
		dbRecovery
			= Schema::Manager::SystemTable::redo(*((*i).second.m_pTransaction),
												 cLogData,
												 m_cDatabaseName, true);
		
		(*i).second.m_bSchemaModify = true;
	}
	catch (...)
	{
		// ログを記録する
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION private
//	Admin::Replicator::Executor::getMasterLSN
//		-- マスター側のトランザクションのLSNを得る
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Log::Data& cLog_
//		マスターから送られてきた論理ログ
//
//	RETURN
//	Trans::Log::LSN
//		マスター側のトランザクションのLSN
//
//	EXCEPTIONS
//
Trans::Log::LSN
Replicator::Executor::getMasterLSN(Trans::Log::Data& cLog_)
{
	Trans::Log::LSN lsn = Trans::Log::IllegalLSN;
	
	if (cLog_.isInsideTransactionData())
	{
		// トランザクション内で記録される論理ログなので、
		// トランザクションの開始を表す論理ログの
		// ログシーケンス番号が格納されている

		Trans::Log::InsideTransactionData& insideLog
			= _SYDNEY_DYNAMIC_CAST(Trans::Log::InsideTransactionData&,
								   cLog_);

		lsn = insideLog.getBeginTransactionLSN();

		if (m_bRestore)
		{
			// トランザクションの再現中なので、このログはスレーブ側の論理ログ
			// 再現トランザクションマップを引いて、マスター側のLSNを求める

			RestoreMap::Iterator i = m_mapRestoreLSN.find(lsn);

			if (i == m_mapRestoreLSN.end())
				// ありえない
				_SYDNEY_THROW0(Exception::LogFileCorrupted);

			lsn = (*i).second;
		}
	}

	return lsn;
}

//
//	FUNCTION private
//	Admin::Replicator::Executor::findRestoreLSN
//		-- 復元するトランザクションを探す
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& trans_
//		論理ログを参照するためのトランザクション
//	Schema::Database* pDatabase_
//		データベース
//	Admin::Replicator::Executor::RestoreMap& restoreLSN_
//		再現するトランザクションのLSN
//
//	RETURN
//	Trans::Log::LSN
//		最後に受け取ったマスター側のLSN
//
//	EXCEPTIONS
//
Trans::Log::LSN
Replicator::Executor::
findRestoreLSN(Trans::Transaction& trans_,
			   Schema::Database* pDatabase_,
			   RestoreMap& restoreLSN_)
{
	// 論理ログを得る

	Trans::Log::AutoFile logFile(pDatabase_->getLogFile());

	Trans::Log::LSN lastLSN = Trans::Log::IllegalLSN;
	Trans::Log::LSN lsn;

	{
		Trans::AutoLatch latch(trans_, logFile->getLockName());
	
		// 論理ログファイルの末尾に記録されている
		// ログシーケンス番号を得る

		lsn = logFile->getLastLSN();

		// 最後に受け取ったマスター側のログシーケンス番号を得る
		
		lastLSN = logFile->getMasterLSN();
	}

	bool isReplicationEnd = false;
	int transactionCount = 0;

	// 論理ログファイルの先頭に向かって
	// ひとつひとつ論理ログを読み出していく
	
	while (lsn != Trans::Log::IllegalLSN)
	{
		// 論理ログを読み出す
		
		ModAutoPointer<const Trans::Log::Data> data;
		{
			Trans::AutoLatch latch(trans_, logFile->getLockName());
			data = logFile->load(lsn);
		}
		if (!data.isOwner())
			_SYDNEY_THROW0(Exception::LogFileCorrupted);

		if (data->getTimeStamp().isIllegal())
			// 不正なタイムスタンプが得られた
			_SYDNEY_THROW0(Exception::LogItemCorrupted);

		// ReplicationEnd が出るまで読み出し
		// 実行中のトランザクションを調べる

		switch (data->getCategory())
		{
		case Trans::Log::Data::Category::ReplicationEnd:
			{
				// トランザクションのコミットを表す論理ログである

				if (isReplicationEnd == true)
					// 二番目以降は利用しない
					break;

				const Admin::Log::ReplicationEndData& tmp
					= _SYDNEY_DYNAMIC_CAST(
						const Admin::Log::ReplicationEndData&, *data);

				// 論路ログファイルのヘッダーに記録されているマスター側のLSNと
				// この論理ログに記録されているものが、同じであるか確認する

				if (lastLSN > tmp.getLastMasterLSN())

					// レプリケーション終了のログの後に、論理ログが
					// 記録されているので、自動リカバリが必要

					_SYDNEY_THROW0(Exception::LogFileCorrupted);

				// 論理ログに記録されているマスター側のLSNは
				// レプリケーション終了の論理ログに記録されているものより
				// 小さいことはあるので、lastLSN を論理ログの値で上書きする

				lastLSN = tmp.getLastMasterLSN();

				// 実行中のトランザクションのリストを得る
				
				const ModVector<Trans::Log::LSN>& beginLSN
					= tmp.getBeginLSN();

				ModVector<Trans::Log::LSN>::ConstIterator i
					= beginLSN.begin();
				for (; i != beginLSN.end(); ++i)
				{
					restoreLSN_.insert(*i, Trans::Log::IllegalLSN);
					++transactionCount;
				}

				isReplicationEnd = true;
			}
			break;
			
		case Trans::Log::Data::Category::TransactionBegin:
			{
				// トランザクションの開始を表す論理ログである

				RestoreMap::Iterator i = restoreLSN_.find(lsn);
				if (i != restoreLSN_.end())
				{
					// 再現する必要のあるトランザクション
					//
					// マスターデータベースを初めてマウントする場合は、
					// マスター側のLSNは、この論理ログのLSNとなる

					Trans::Log::LSN masterLSN = lsn;

					if (data->getClassID() ==
						(Trans::Externalizable::Category::
						 TransactionBeginForSlaveLogData +
						 Common::Externalizable::TransClasses))
					{
						// マスター側のLSNを得る

						const Trans::Log::TransactionBeginForSlaveData& tmp
							= _SYDNEY_DYNAMIC_CAST(
								const Trans::Log::TransactionBeginForSlaveData&,
								*data);
						
						masterLSN = tmp.getMasterLSN();
					}

					// マスター側のLSNを設定する
					
					(*i).second = masterLSN;

					// 再現する必要のあるトランザクションを１つ処理した
					
					--transactionCount;
				}
			}
			break;
			
		default:
			break;
		}

		if (isReplicationEnd && transactionCount == 0)
			// 終了
			break;

		{
			// 次へ
			Trans::AutoLatch latch(trans_, logFile->getLockName());
			lsn = logFile->getPrevLSN(lsn);
		}
	}

	return lastLSN;
}

//
//	FUNCTION private
//	Admin::Replicator::Executor::isRestoreLog -- 再実行対象のログか否か
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Log::Data& cLog_
//		論理ログ
//	Trans::Log::LSN lsn_
//		論理ログのスレーブ側のLSN
//
//	RETURN
//	bool
//		再実行対象のログの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Replicator::Executor::isRestoreLog(const Trans::Log::Data& cLog_,
								   Trans::Log::LSN lsn_)
{
	bool result = false;
	
	switch (cLog_.getCategory())
	{
	case Trans::Log::Data::Category::TransactionBegin:
		{
			// トランザクションの開始

			RestoreMap::Iterator i = m_mapRestoreLSN.find(lsn_);

			if (i != m_mapRestoreLSN.end())
				// ヒットした
				
				result = true;
		}
		break;
		
	case Trans::Log::Data::Category::TransactionCommit:
	case Trans::Log::Data::Category::TransactionRollback:
	case Trans::Log::Data::Category::StatementCommit:
	case Trans::Log::Data::Category::StatementRollback:
	case Trans::Log::Data::Category::TupleModify:
	case Trans::Log::Data::Category::SchemaModify:
		{
			// トランザクション中のログ
			
			const Trans::Log::InsideTransactionData& insideLog
				= _SYDNEY_DYNAMIC_CAST(const Trans::Log::InsideTransactionData&,
									   cLog_);

			Trans::Log::LSN lsn = insideLog.getBeginTransactionLSN();

			RestoreMap::Iterator i = m_mapRestoreLSN.find(lsn);

			if (i != m_mapRestoreLSN.end())
				// ヒットした
				
				result = true;
		}
		break;
		
	default:
		// その他は無視
		break;
		
	}

	return result;
}

//
//	FUNCTION public
//	Admin::Replicator::Replicator -- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cMansterHostName_
//		マスターホスト名
//	int iMasterPortNumber_
//		マスターポート番号
//	int iProtocol
//		プロトコル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Replicator::Replicator(const ModUnicodeString& cMasterHostName_,
					   int iMasterPortNumber_,
					   int iProtocol_)
	: m_pDataSource(0),
	  m_cMasterHostName(cMasterHostName_),
	  m_iMasterPortNumber(iMasterPortNumber_),
	  m_iProtocol(iProtocol_),
	  m_bRunning(false)
{
	// データソースを用意する
	m_pDataSource = _createDataSource(m_cMasterHostName,
									  m_iMasterPortNumber);
}

//
//	FUNCTION public
//	Admin::Replicator::~Replicator -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Replicator::~Replicator()
{
	try {
		// 必要ならスレッドを停止する
		abort();
		join();
	}
	catch (...) {}	// 例外は無視する
	
	if (m_pDataSource)
	{
		try
		{
			m_pDataSource->close();
		}
		catch (...) {}	// 例外は無視する
		
		m_pDataSource->release();
		m_pDataSource = 0;
	}
}

//
//	FUNCTION public static
//	Admin::Replicator::initialize -- 初期化する
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::initialize()
{
	//	すべてのデータベース走査し、スレーブデータベースごとに
	//	スレッドを生成し、レプリケーションを開始する

	// トランザクションを開始する

	Trans::AutoTransaction	trans(Trans::Transaction::attach());
	trans->begin(Schema::ObjectID::SystemTable,
				 Trans::Transaction::Mode(
					 Trans::Transaction::Category::ReadOnly,
					 Trans::Transaction::IsolationLevel::Serializable,
					 Boolean::False),
				 true, true);
	
	//「データベース」表に登録されているデータベースごとに処理する
	ModVector<Schema::Database*> databases(
		Schema::Manager::ObjectTree::Database::get(*trans));

	ModVector<Schema::Database*>::ConstIterator			ite(databases.begin());
	const ModVector<Schema::Database*>::ConstIterator&	end = databases.end();

	for (; ite != end; ++ite)
	{
		Schema::Database*	database = *ite;
		; _SYDNEY_ASSERT(database);

		if (database->isSlaveStarted() == false)
			// SLAVEではないので、次へ
			continue;

		try
		{
			// レプリケーションを開始する

			startReplication(database);
		}
		catch (Exception::Object& e)
		{
			SydErrorMessage << e << ModEndl;
			SydErrorMessage << "Starting replication failed. "
							<< database->getName()
							<< ModEndl;
			
			// reset error status
			Common::Thread::resetErrorCondition();
		}
		catch (ModException& e)
		{
			SydErrorMessage << Exception::ModLibraryError(moduleName,
														  srcFile, __LINE__, e)
							<< ModEndl;
			SydErrorMessage << "Starting replication failed. "
							<< database->getName()
							<< ModEndl;

			// reset error status
			Common::Thread::resetErrorCondition();
		}
#ifndef NO_CATCH_ALL
		catch (std::exception& e)
		{
			SydErrorMessage << "std::exception occurred. "
							<< (e.what() ? e.what() : "") << ModEndl;
			SydErrorMessage << "Starting replication failed. "
							<< database->getName()
							<< ModEndl;

			// reset error status
			Common::Thread::resetErrorCondition();
		}
		catch (...)
		{
			SydErrorMessage << "Unexpected exception occurred."
							<< ModEndl;
			SydErrorMessage << "Starting replication failed. "
							<< database->getName()
							<< ModEndl;

			// reset error status
			Common::Thread::resetErrorCondition();
		}
#endif
	}

	// トランザクションを終了する

	trans->commit();
}

//
//	FUNCTION public static
//	Admin::Replicator::terminate -- 終了処理を行う
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::terminate()
{
	Os::AutoCriticalSection cAuto(_cReplicatorLatch);

	_ReplicatorMap::Iterator i = _cReplicatorMap.begin();
	for (; i != _cReplicatorMap.end(); ++i)
	{
		// すべての実行スレッドを停止する

		(*i).second->abortAll();
	}

	i = _cReplicatorMap.begin();
	for (; i != _cReplicatorMap.end(); ++i)
	{
		// レプリケーターのスレッドを停止する

		(*i).second->abort();
	}

	i = _cReplicatorMap.begin();
	for (; i != _cReplicatorMap.end(); ++i)
	{
		// レプリケーターのスレッドの停止を確認する

		(*i).second->join();

		// メモリを開放する

		delete (*i).second;
	}

	// マップをクリアする
	_cReplicatorMap.erase(_cReplicatorMap.begin(),
						  _cReplicatorMap.end());
}

//
//	FUNCTION public static
//	Admin::Replicator::start -- 該当するエグゼキュータのみ開始する
//
//	NOTES
//
//	ARGUMENTS
// 	Trans::Transaction& cTransaction_
//		トランザクション
//	Schema::Database& cDatabase_
//		データベース
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::start(Trans::Transaction& cTransaction_,
				  Schema::Database& cDatabase_)
{
	//【注意】	alter database や mount からの呼び出しを想定している
	
	startReplication(&cDatabase_);
}

//
//	FUNCTION public static
//	Admin::Replicator::stop -- 該当するエグゼキュータのみ停止する
//
//	NOTES
//
//	ARGUMENTS
// 	Trans::Transaction& cTransaction_
//		トランザクション
//	Schema::Database& cDatabase_
//		データベース
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::stop(Trans::Transaction& cTransaction_,
				 Schema::Database& cDatabase_)
{
	//【注意】	alter database からの呼び出しを想定している

	// URLを得る
	const ModUnicodeString& url = cDatabase_.getMasterURL();

	bool isCrypt;
	ModUnicodeString user;
	ModUnicodeString password;
	ModUnicodeString host;
	int portNumber = 0;
	ModUnicodeString database;
	Schema::ObjectID::Value databaseid;
	
	// まずはパースする
	if (_parseURL(url,
				  isCrypt, user, password, host, portNumber, database) == false)
		_SYDNEY_THROW2(Exception::CannotConnect, host, portNumber);

	// マスターデータベースID
	databaseid = ModUnicodeCharTrait::toUInt(database);

	Replicator* pReplicator = 0;

	{
		Os::AutoCriticalSection cAuto(_cReplicatorLatch);

		_MapKey key(host, portNumber);

		_ReplicatorMap::Iterator i = _cReplicatorMap.find(key);
		if (i == _cReplicatorMap.end())
			// 見つからない
			_SYDNEY_THROW0(Exception::BadArgument);

		pReplicator = (*i).second;
	}

	// 実行スレッドを削除する

	pReplicator->delExecutor(cTransaction_,
							 cDatabase_,
							 databaseid);
}

//
//	FUNCTION public static
//	Admin::Replicator::checkConnectMaster
//		-- マスターデータベースに接続できるかチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& url_
//		マスターデータベースのURL
//
//	RETURN
//	ModUnicodeString
//		スキーマに格納するべきURL
//
//	EXCEPTIONS
//
ModUnicodeString
Replicator::checkConnectMaster(const ModUnicodeString& url_)
{
	bool isCrypt;
	ModUnicodeString user;
	ModUnicodeString password;
	ModUnicodeString host;
	int portNumber = 0;
	ModUnicodeString database;
	
	// まずはパースする
	if (_parseURL(url_,
				  isCrypt, user, password, host, portNumber, database) == false)
		_SYDNEY_THROW2(Exception::CannotConnect, host, portNumber);

	// プロトコル
	int iProtocol = Communication::Protocol::CurrentVersion;
	if (isCrypt == true)
		_SYDNEY_THROW0(Exception::NotSupported);

	// データソース
	Client2::DataSource* pDataSource
		= _createDataSource(host, portNumber);

	// オープンする
	pDataSource->open(iProtocol);

	// セッションを作成する
	Client2::Session* pSession = 0;
	if (user.getLength() == 0)
		pSession = pDataSource->createSession(database);
	else
		pSession = pDataSource->createSession(database, user, password);

	// クライアントコネクションを得る
	Client2::Connection* pClientConnection = pDataSource->getClientConnection();

	// ワーカを起動する
	ModAutoPointer<Client2::Port> pPort = pClientConnection->beginWorker();

	// [<-] リクエスト
	Common::Request cRequest(Common::Request::CheckReplication);
	pPort->writeObject(&cRequest);
	// [<-] セッションID
	Common::IntegerData cSessionID(pSession->getID());
	pPort->writeObject(&cSessionID);
	// flushする
	pPort->flush();
	
	// [->] データベースID
	ModAutoPointer<Common::Externalizable> pObject
		= pPort->readObject();
	Common::UnsignedIntegerData* pDatabaseID
		= dynamic_cast<Common::UnsignedIntegerData*>(pObject.get());
	// [->] ステータス
	pPort->readStatus();

	// ポートを返す
	pDataSource->pushPort(pPort.release());

	// セッション終了
	pSession->close();
	pSession->release();

	// データソースクローズ
	pDataSource->close();
	pDataSource->release();

	// すべてOKなので、保存用のURLを作成する
	ModUnicodeOstrStream s;
	if (host.getLength())
	{
		s << _cDoqueDb << "://" << host << ":" << portNumber;
	}
	else
	{
		s << _cInprocess << "://";
	}

	// データベース名からデータベースIDに変換する
	s << "/" << pDatabaseID->getValue();

	return s.getString();
}

//
//	FUNCTION private
//	Admin::Replicator::startReplication -- レプリケーションを開始する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::startReplication(Schema::Database* pDatabase_)
{
	// URLを得る
	const ModUnicodeString& url = pDatabase_->getMasterURL();

	bool isCrypt;
	ModUnicodeString user;
	ModUnicodeString password;
	ModUnicodeString host;
	int portNumber = 0;
	ModUnicodeString database;
	Schema::ObjectID::Value databaseid;
	
	// まずはパースする
	if (_parseURL(url,
				  isCrypt, user, password, host, portNumber, database) == false)
		_SYDNEY_THROW2(Exception::CannotConnect, host, portNumber);

	// マスターデータベースID
	databaseid = ModUnicodeCharTrait::toUInt(database);

	// プロトコル
	int iProtocol = Communication::Protocol::CurrentVersion;
	if (isCrypt == true)
		_SYDNEY_THROW0(Exception::NotSupported);

	Replicator* pReplicator = 0;

	{
		Os::AutoCriticalSection cAuto(_cReplicatorLatch);
		
		// すでに接続済みかどうか確認する
		_MapKey key(host, portNumber);
		_ReplicatorMap::Iterator i = _cReplicatorMap.find(key);
		if (i == _cReplicatorMap.end())
		{
			// 存在していないので、作成する

			pReplicator = new Replicator(host, portNumber, iProtocol);
			pReplicator->create();

			// マップに登録する
			
			_cReplicatorMap.insert(key, pReplicator);

			// スレッドが起動するまで待機する
			
			pReplicator->m_cRunEvent.wait();
		}
		else
		{
			// 存在していた

			pReplicator = (*i).second;
		}
	}

	// 実行スレッドを追加する

	pReplicator->addExecutor(pDatabase_->getID(),
							 pDatabase_->getName(),
							 databaseid);
}

//
//	FUNCTION public
//	Admin::Replicator::addExecutor -- 実行スレッドを追加する
//
//	NOTES
//
//	ARGUMENTS
//	Schema::ObjectID::Value uiSlaveDatabaseID_
//		スレーブデータベースID
//	const ModUnicodeString& cSlaveDatabaseName_
//		スレーブデータベース名
//	Schema::ObjectID::Value uiMasterDatabaseID_
//		マスターデータベースID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::addExecutor(Schema::ObjectID::Value uiSlaveDatabaseID_,
						const ModUnicodeString& cSlaveDatabaseName_,
						Schema::ObjectID::Value uiMasterDatabaseID_)
{
	ModAutoPointer<Executor> pExecutor = new Executor(this,
													  uiSlaveDatabaseID_,
													  cSlaveDatabaseName_,
													  uiMasterDatabaseID_);

	{
		Os::AutoCriticalSection cAuto(m_cLatch);

		// マップに登録する

		Executor* p = pExecutor.release();
		m_cQueueMap.insert(uiMasterDatabaseID_, p->getQueue());
		m_cExecutorMap.insert(uiSlaveDatabaseID_, p);

		// スレッド実行する

		p->create();
	}	
}

//
//	FUNCTION public
//	Admin::Replicator::delExecutor -- 実行スレッドを削除する
//
//	NOTES
//
//	ARGUMENTS
//	Schema::Database& cDatabase_
//		スレーブデータベース
//	Schema::ObjectID::Value uiMasterDatabaseID_
//		マスターデータベースID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::delExecutor(Trans::Transaction& cTransaction_,
						Schema::Database& cSlaveDatabase_,
						Schema::ObjectID::Value uiMasterDatabaseID_)
{
	Executor* pExecutor = 0;
	
	{
		Os::AutoCriticalSection cAuto(m_cLatch);

		ModHashMap<Schema::ObjectID::Value, Queue*,
			ModHasher<Schema::ObjectID::Value> >::Iterator i0
			= m_cQueueMap.find(uiMasterDatabaseID_);
		if (i0 != m_cQueueMap.end())
		{
			m_cQueueMap.erase(i0);
		}

		ModHashMap<Schema::ObjectID::Value, Executor*,
			ModHasher<Schema::ObjectID::Value> >::Iterator i1
			= m_cExecutorMap.find(cSlaveDatabase_.getID());
		if (i1 != m_cExecutorMap.end())
		{
			pExecutor = (*i1).second;
			m_cExecutorMap.erase(i1);
		}

		if (pExecutor == 0)
			return;

		try
		{
			// マスターサーバーに切断を要求する

			pExecutor->disconnectMaster(m_pDataSource);
		}
		catch (...) {}
	}

	if (pExecutor->isAborted() == false)
		// 中断する
		pExecutor->abort();

	pExecutor->join();

	// ログを作成する

	Log::ReplicationEndData cLogData;
	pExecutor->makeLog(cLogData);

	// 論理ログファイルを得る

	Trans::Log::AutoFile logFile(cSlaveDatabase_.getLogFile());

	// 論理ログを書き出す

	Trans::AutoLatch latch(cTransaction_, logFile->getLockName());
	logFile->store(cLogData);

	// すべてのトランザクションをロールバックする

	pExecutor->rollbackAll();

	delete pExecutor;
}

//
//	FUNCTION public
//	Admin::Replicator::abortAll -- すべての実行スレッドを停止する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::abortAll()
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	// マスターサーバに切断を要求する
	ModHashMap<Schema::ObjectID::Value, Executor*,
		ModHasher<Schema::ObjectID::Value> >::Iterator i
		= m_cExecutorMap.begin();
	for (; i != m_cExecutorMap.end(); ++i)
	{
		try
		{
			(*i).second->disconnectMaster(m_pDataSource);
		}
		catch (...) {}	// 例外は無視する
	}
	
	// 実行スレッドを停止する
	i = m_cExecutorMap.begin();
	for (; i != m_cExecutorMap.end(); ++i)
	{
		(*i).second->abort();
	}

	// その他の処理
	i = m_cExecutorMap.begin();
	for (; i != m_cExecutorMap.end(); ++i)
	{
		// スレッドの終了を確認する
		(*i).second->join();

		// 論理ログを出力する
		(*i).second->storeLog();

		// 実行中トランザクションをロースバックする
		(*i).second->rollbackAll();

		// メモリを開放する
		delete (*i).second;
	}

	// マップをクリアする
	m_cQueueMap.erase(m_cQueueMap.begin(),
					  m_cQueueMap.end());
	m_cExecutorMap.erase(m_cExecutorMap.begin(),
						 m_cExecutorMap.end());
}

//
//	FUNCTION private
//	Admin::Replicator::runnable -- スレッドとして起動されるメソッド
//
//	NOTES
//
//	ARGUMENT
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::runnable()
{
	while (true)
	{
		{
			// 接続状態を初期化する
			
			Os::AutoCriticalSection cAuto(m_cLatch);
			
			ModHashMap<Schema::ObjectID::Value, Executor*,
				ModHasher<Schema::ObjectID::Value> >::Iterator i
				= m_cExecutorMap.begin();
			for (; i != m_cExecutorMap.end(); ++i)
			{
				(*i).second->clear();
			}
		}

		// マスターとの接続ポートを得る
		ModAutoPointer<Client2::Port> pPort = getConnection();
		if (pPort.get() == 0)
			// 終了
			break;

		{
			Os::AutoCriticalSection cAuto(m_cLatch);
			
			// 実行スレッドを接続する
			ModHashMap<Schema::ObjectID::Value, Executor*,
				ModHasher<Schema::ObjectID::Value> >::Iterator i
				= m_cExecutorMap.begin();
			for (; i != m_cExecutorMap.end(); ++i)
			{
				try
				{
					(*i).second->connectMaster(m_pDataSource);
				}
				catch (...) {}
			}

			// ここから後に接続するデータベースは
			// 呼び出しスレッド側で connectMaster を実行する

			m_bRunning = true;
		}

		try
		{
			if (execute(pPort.get()) == false)
				// 終了
				break;
		}
		catch (Exception::BadArgument& e)
		{
			SydErrorMessage << e << ModEndl;
			_SYDNEY_RETHROW;
		}
		catch (Exception::Object& e)
		{
			SydErrorMessage << e << ModEndl;
			m_pDataSource->close();
		}
		catch (...)
		{
			SydErrorMessage << "Unexpected Exception" << ModEndl;
			_SYDNEY_RETHROW;
		}

		{
			Os::AutoCriticalSection cAuto(m_cLatch);

			m_bRunning = false;
		}
	}

	

	{
		Os::AutoCriticalSection cAuto(m_cLatch);
			
		// 実行スレッドを停止する
		ModHashMap<Schema::ObjectID::Value, Executor*,
			ModHasher<Schema::ObjectID::Value> >::Iterator i
			= m_cExecutorMap.begin();
		for (; i != m_cExecutorMap.end(); ++i)
		{
			(*i).second->abort();
		}
	}
}
			
//
//	FUNCTION private
//	Admin::Replicator::getConnection -- マスターと接続する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Client2::Port*
//		 マスターから論理ログが送られてくるポート
//
//	EXCEPTIONS
//
Client2::Port*
Replicator::getConnection()
{
	Os::AutoTryCriticalSection cAuto(m_cLatch);
	cAuto.lock();

	// 起動イベントをシグナル化する
	m_cRunEvent.set();
	
	ModAutoPointer<Client2::Port> p;
	
	while (true)
	{
		for (;;)
		{
			try
			{
				// オープンする
				m_pDataSource->open(m_iProtocol);

				SydInfoMessage << "Connect master server. "
							   << m_cMasterHostName << ":"
							   << m_iMasterPortNumber
							   << ModEndl;

				break;
			}
			catch (Exception::Object& e)
			{
				SydInfoMessage << "Can't connect master server. "
							   << m_cMasterHostName << ":"
							   << m_iMasterPortNumber << ModEndl;
				SydInfoMessage << e << ModEndl;
			}
			catch (ModException& e)
			{
				SydInfoMessage << "Can't connect master server. "
							   << m_cMasterHostName << ":"
							   << m_iMasterPortNumber << ModEndl;
				SydInfoMessage << _MOD_EXCEPTION(e) << ModEndl;
				Common::Thread::resetErrorCondition();
			}
#ifndef NO_CATCH_ALL
			catch (std::exception& e)
			{
				SydInfoMessage << "Can't connect master server. "
							   << m_cMasterHostName << ":"
							   << m_iMasterPortNumber << ModEndl;
				SydInfoMessage << "std::exception occurred. "
							   << (e.what() ? e.what() : "") << ModEndl;
			}
			catch (...)
			{
				SydInfoMessage << "Can't connect master server. "
							   << m_cMasterHostName << ":"
							   << m_iMasterPortNumber << ModEndl;
				SydInfoMessage << "Unexpected Exception" << ModEndl;
			}
#endif
			// 一定時間待つ
			cAuto.unlock();
			if (waitConnect() == true)
				return 0;
			cAuto.lock();
		}

		try
		{
			// クライアントコネクションを得る
			Client2::Connection* pConnection
				= m_pDataSource->getClientConnection();

			// ワーカを起動する
			p = pConnection->beginWorker();

			// レプリケーションを開始する

			// [<-] リクエスト
			Common::Request cRequest(Common::Request::StartReplication);
			p->writeObject(&cRequest);
			// [<-] スレーブホスト名
			Common::StringData cHostName(Os::SysConf::HostName::get());
			p->writeObject(&cHostName);
			// [<-] スレーブポート番号
			Common::IntegerData
				cPortNumber(Communication::SocketDaemon::getPortNumber());
			p->writeObject(&cPortNumber);
	
			// flushする
			p->flush();

			// ステータスを受け取る
			p->readStatus();

			break;
		}
		catch (Exception::Object& e)
		{
			SydInfoMessage << e << ModEndl;
			m_pDataSource->close();

			// 一定時間待つ
			cAuto.unlock();
			if (waitConnect() == true)
				return 0;
			cAuto.lock();
		}
		catch (...)
		{
			SydInfoMessage << "Master connection ran out." << ModEndl;
			m_pDataSource->close();

			// 一定時間待つ
			cAuto.unlock();
			if (waitConnect() == true)
				return 0;
			cAuto.lock();
		}
	}

	return p.release();
}

//
//	FUNCTION private
//	Admin::Replicator::execute -- ログを反映する
//
//	NOTES
//
//	ARGUMENTS
//	Client2::Port* pPort_
//		マスターとの通信路
//
//	RETURN
//	bool
//		終了する場合はfalse、接続が切れたりし再接続が必要な場合はtrue
//
//	EXCEPTIONS
//
bool
Replicator::execute(Client2::Port* pPort_)
{
	try
	{
		for (;;)
		{
			// 終了するか確認する
			if (isAborted() == true)
				return false;
			
			// マスターからデータが送られてくるのを
			// ポーリングしながら待つ

			while (pPort_->wait(1000) == false)
			{
				if (isAborted() == true)
					// 終了
					return false;
			}

			// データを読み出す
			//
			// 送られてくるのは以下の３つ
			//	Common::UnsignedIntegerData		マスター側のDBID
			//	Common::BinaryData				論理ログ本体
			//	Common::UnsignedInteger64Data	マスター側のLSN

			ModAutoPointer<Common::Externalizable> pObject0
				= pPort_->readObject();
			Common::UnsignedIntegerData* pID
				= dynamic_cast<Common::UnsignedIntegerData*>(pObject0.get());
			if (pID == 0)
			{
				_SYDNEY_THROW0(Exception::BadArgument);
			}

			ModAutoPointer<Common::Externalizable> pObject1
				= pPort_->readObject();
			Common::BinaryData* pLogicalLog
				= dynamic_cast<Common::BinaryData*>(pObject1.get());
			if (pLogicalLog == 0)
			{
				_SYDNEY_THROW0(Exception::BadArgument);
			}

			ModAutoPointer<Common::Externalizable> pObject2
				= pPort_->readObject();
			Common::UnsignedInteger64Data* pLSN
				= dynamic_cast<Common::UnsignedInteger64Data*>(pObject2.get());
			if (pLSN == 0)
			{
				_SYDNEY_THROW0(Exception::BadArgument);
			}

			// 受け取った論理ログに保持されている
			// データを取り出す

			ModMemory memory(pLogicalLog->getValue(),
							 pLogicalLog->getSize());
			ModAutoPointer<Common::Externalizable> p
				= Common::InputArchive(memory).readObject();
			Trans::Log::Data* pLog
				= dynamic_cast<Trans::Log::Data*>(p.get());
			if (pLog == 0)
			{
				_SYDNEY_THROW0(Exception::BadArgument);
			}

			// ログをキューに追加する
			p.release();
			pushQueue(pID->getValue(), pLog, pLSN->getValue());
		}
	}
	catch (...)
	{
		// 再接続が必要かも
		_SYDNEY_RETHROW;
	}

	return true;
}

//
//	FUNCTION private
//	Admin::Replicator::pushQueue -- 論理ログをキューに追加する
//
//	NOTES
//
//	ARGUMENTS
//	Schema::ObjectID::Value uiDatabaseID_
//		マスターのデータベースID
//	Trans::Log::Data* pLog_
//		論理ログ
//	Trans::Log::LSN uiLSN_
//		マスターサーバのLSN
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Replicator::pushQueue(Schema::ObjectID::Value uiDatabaseID_,
					  Trans::Log::Data* pLog_,
					  Trans::Log::LSN uiLSN_)
{
	// 実行する可能性のあるログのみキューに追加する

	bool pushed = false;

	switch (pLog_->getCategory())
	{
	case Trans::Log::Data::Category::TransactionBegin:
	case Trans::Log::Data::Category::TransactionCommit:
	case Trans::Log::Data::Category::TransactionRollback:
	case Trans::Log::Data::Category::StatementCommit:
	case Trans::Log::Data::Category::StatementRollback:
	case Trans::Log::Data::Category::CheckpointDatabase:
	case Trans::Log::Data::Category::TupleModify:
	case Trans::Log::Data::Category::SchemaModify:
		{
			Os::AutoCriticalSection cAuto(m_cLatch);
			
			// キューを得る
			ModHashMap<Schema::ObjectID::Value, Queue*,
				ModHasher<Schema::ObjectID::Value> >::Iterator i
				= m_cQueueMap.find(uiDatabaseID_);
			if (i != m_cQueueMap.end())
			{
				// 見つかったので、キューに追加
				
				(*i).second->pushBack(pLog_, uiLSN_);
				pushed = true;
			}
		}
		break;
	default:
		// その他は無視
		break;
	}

	if (!pushed)
	{
		// キューに追加されなかったので削除する

		delete pLog_;
	}
}

//
//	FUNCTION private
//	Admin::Replicator::waitConnect -- 接続を一定時間待つ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		終了する必要がある場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Replicator::waitConnect()
{	
	// 接続できなかった場合は、一定時間待つ
	// ただし、１秒に一回 abort() されたかどうか確認する
			
	int n = _cWaitTime.get();
	int s = n / 1000;
	int a = n % 1000;

	while (s)
	{
		ModOsDriver::Thread::sleep(1000);
		if (isAborted() == true)
			return true;
		--s;
	}
	if (a > 0)
	{
		ModOsDriver::Thread::sleep(a);
		if (isAborted() == true)
			return true;
	}

	return false;
}

//
// Copyright (c) 2014, 2015, 2016, 2017, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
