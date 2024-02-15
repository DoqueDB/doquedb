// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileSynchronizer.cpp -- バージョンファイル同期スレッドの関数定義
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Checkpoint";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Checkpoint/Daemon.h"
#include "Checkpoint/Database.h"
#include "Checkpoint/FileSynchronizer.h"
#include "Checkpoint/LogData.h"
#include "Checkpoint/Manager.h"
#include "Checkpoint/Configuration.h"

#include "Common/Assert.h"
#include "Common/AutoCaller.h"
#include "Common/Message.h"

#include "Exception/Cancel.h"
#include "Exception/FakeError.h"
#include "Exception/ModLibraryError.h"
#include "Exception/Unexpected.h"

#include "Schema/Database.h"
#include "Schema/Manager.h"
#include "Schema/Hold.h"
#include "Server/Manager.h"
#include "Trans/AutoLatch.h"
#include "Trans/AutoTransaction.h"

#include "ModMap.h"

#define _MOD_EXCEPTION(e) \
	Exception::ModLibraryError(moduleName, srcFile, __LINE__, e)

_SYDNEY_USING
_SYDNEY_CHECKPOINT_USING

namespace
{

namespace _FileSynchronizer
{
	// 同期処理の候補であるデータベースの
	// オブジェクトID を管理するマップが生成されていなければ、用意する
	bool
	prepare();
	// データベースに対して同期処理を行う
	bool
	sync(Schema::ObjectID::Value dbID);

	typedef ModMap<Schema::ObjectID::Value, int, ModLess<Schema::ObjectID::Value> >	_DatabaseMap;

	// 同期処理の候補であるデータベースのオブジェクトID を管理するマップ

	_DatabaseMap*	_candidateMap = 0;

	// 今回の同期処理のタイミングでは同期処理を実行しないデータベースの
	// オブジェクトIDを管理するマップ

	_DatabaseMap	_skipMap;
}

//	FUNCTION
//	$$$::_FileSynchronizer::prepare --
//		同期処理の候補であるデータベースの
//		オブジェクトIDを管理するマップが生成されていなければ、用意する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			マップが生成されている
//		false
//			マップを生成できなかった
//
//	EXCEPTIONS

bool
_FileSynchronizer::prepare()
{
	if (!_candidateMap)
	{
		if (Configuration::LoadSynchronizeCandidate::get())
		{
			// 起動時に同期処理候補を列挙する
			
			if (!Checkpoint::Database::isAvailable(
					Schema::ObjectID::SystemTable))

				// システム表が利用不可なので、あきらめる

				return false;

			// システム表を操作するトランザクションを開始する

			Trans::AutoTransaction	trans(Trans::Transaction::attach());
			trans->begin(Schema::ObjectID::SystemTable,
						 Trans::Transaction::Mode(
							 Trans::Transaction::Category::ReadWrite,
							 Trans::Transaction::IsolationLevel::Serializable,
							 Boolean::False));

			// 「データベース」表を他から更新不可にロックする

			if (!(Schema::Manager::SystemTable::hold(
					  *trans, Schema::Hold::Target::MetaDatabase,
					  Lock::Name::Category::Tuple,
					  Schema::Hold::Operation::ReadForImport,
					  Schema::ObjectID::Invalid,
					  Trans::Log::File::Category::System, 0, 0) &&
				  Schema::Manager::SystemTable::hold(
					  *trans, Schema::Hold::Target::MetaTable,
					  Lock::Name::Category::Table,
					  Schema::Hold::Operation::ReadForImport,
					  Schema::ObjectID::Invalid,
					  Trans::Log::File::Category::System, 0, 0)))

				// ロックできなかったので、あきらめる

				return false;

			// 「データベース」表を得る
			
			Schema::Database* system
				= Schema::Database::get(Schema::ObjectID::SystemTable, *trans);

			// そのデータベース用の論理ログファイルを得る
			
			system->getLogFile();

			//「データベース」表に登録されている
			// すべてのデータベースに関する情報を得る

			const ModVector<Schema::Database*>&
				databases =	Schema::Manager::ObjectTree::Database::get(*trans);
			const unsigned int n = databases.getSize();

			// 次にすべてのデータベースの論理ログファイルを得る

			for (unsigned int i = 0; i < n; ++i)
				databases[i]->getLogFile();

			// システム表を操作するトランザクションをコミットし、
			// トランザクション中にかけたロックをはずす

			trans->commit();
		}

		// マップのインスタンスを得るだけにする
		// 論理ログのヘッダーに同期処理が完了したかどうか記録されているので、
		// ここですべてのデータベースを候補にすることはやめる

		_candidateMap = new _DatabaseMap();

	}

	return true;
}

//	FUNCTION
//	$$$::_FileSynchronizer::sync --　データベースに対して同期処理を行う
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectID::Value	dbID
//			同期処理対象のデータベースのオブジェクトID
//
//	RETURN
//		true
//			データベースのある部分を処理し残した
//		false
//			完全に同期処理した
//
//	EXCEPTIONS

bool
_FileSynchronizer::sync(Schema::ObjectID::Value dbID)
{
	// データベースに対して同期処理を行うトランザクションを開始する

	Trans::AutoTransaction	trans(Trans::Transaction::attach());
	trans->begin(dbID, Trans::Transaction::Mode(
					 Trans::Transaction::Category::ReadWrite,
					 Trans::Transaction::IsolationLevel::Serializable,
					 Boolean::False));

	bool incomplete = false;

	if (dbID == Schema::ObjectID::SystemTable) {

		// システム表に対して同期処理を行う

		if (Checkpoint::Database::isAvailable(dbID)) {

			// システム用の論理ログファイルに
			// システム表を構成するバージョンファイルの
			// 同期の開始を表す論理ログを記録する

			const Lock::Name& lockName = trans->getLogInfo(
				Trans::Log::File::Category::System).getLockName();
			{
			Trans::AutoLatch latch(*trans, lockName);
			trans->storeLog(Trans::Log::File::Category::System,
							Log::FileSynchronizeBeginData());
			}
			// システム表を構成するバージョンファイルの同期を取る

			bool modified = false;
			try {
				Schema::Manager::sync(*trans, incomplete, modified);

			} catch (Exception::Cancel&) {

				// 同期処理が中断されたときも
				// 通常に処理し終えたときと同様に論理ログを記録する
			}

			// システム表を構成するバージョンファイルの
			// 同期の終了を表す論理ログを記録する
			{
			Trans::AutoLatch latch(*trans, lockName);
			trans->storeLog(Trans::Log::File::Category::System,
							Log::FileSynchronizeEndData(modified));
			if (incomplete == false)
				// 完全に同期処理が完了しているので論理ログに記録する
				trans->setSynchronizeDone(Trans::Log::File::Category::System);
			}
		}
	} else {

		// システム表以外のデータベースに対して同期処理を行う

		if (!(Schema::Manager::SystemTable::hold(
				  *trans, Schema::Hold::Target::MetaDatabase,
				  Lock::Name::Category::Tuple,
				  Schema::Hold::Operation::ReadForImport,
				  Schema::ObjectID::Invalid,
				  Trans::Log::File::Category::System, 0, 0) &&
			  Schema::Manager::SystemTable::hold(
				  *trans, Schema::Hold::Target::MetaTable,
				  Lock::Name::Category::Tuple,
				  Schema::Hold::Operation::ReadForImport,
				  Schema::ObjectID::Invalid,
				  Trans::Log::File::Category::System, 0, 0) &&
			  Schema::Manager::SystemTable::hold(
				  *trans, Schema::Hold::Target::MetaTuple,
				  Lock::Name::Category::Tuple,
				  Schema::Hold::Operation::ReadForImport, dbID,
				  Trans::Log::File::Category::System, 0, 0)))

			// このデータベースの「データベース」表のタプルを
			// 他から更新不可にロックできなかった

			return true;

		// このデータベースが利用可能であるか調べる

		if (Checkpoint::Database::isAvailable(dbID))

			// このデータベースのスキーマオブジェクトを得る
			//
			//【注意】	データベースが利用不可だと例外が発生してしまうので、
			//			事前にそうでないことを確認しておく必要がある

			if (Schema::Database* database =
				Schema::Database::get(dbID, *trans)) {

				// キャッシュが破棄されないように
				// データベースの使用開始を宣言する

				database->open();

				// スコープを抜けるときに
				// 自動的にデータベースを使用終了を宣言し、
				// キャッシュをクリアされやすくする

				Common::AutoCaller1<Schema::Database, bool>
					closer(database, &Schema::Database::close, true);

				if (!database->isReadOnly() && database->isOnline()) {

					// 更新可でオンラインのデータベースであれば、処理する

					// 一時表でないはず

					; _SYDNEY_ASSERT(database->getScope() ==
									 Schema::Object::Scope::Permanent);

					// このデータベースのオブジェクトIDを得てから、
					// これまでにデータベースが破棄されていない

					// このデータベース用の論理ログファイルに
					// このデータベースを構成するバージョンファイルの
					// 同期の開始を表す論理ログを記録する

					trans->setLog(*database);

					const Lock::Name& lockName = trans->getLogInfo(
						Trans::Log::File::Category::Database).getLockName();
					{
					Trans::AutoLatch latch(*trans, lockName);
					trans->storeLog(Trans::Log::File::Category::Database,
									Log::FileSynchronizeBeginData());
					}
					// このデータベースを構成するバージョンファイルの同期を取る

					bool modified = false;
					try {
						database->sync(*trans, incomplete, modified);

					} catch (Exception::Cancel&) {

						// 同期処理が中断されたときも
						// 通常に処理し終えたときと同様に論理ログを記録する
					}

					// このデータベースを構成するバージョンファイルの
					// 同期の終了を表す論理ログを記録する
					{
					Trans::AutoLatch latch(*trans, lockName);
					trans->storeLog(Trans::Log::File::Category::Database,
									Log::FileSynchronizeEndData(modified));
					if (incomplete == false)
						// 完全に同期処理が完了しているので論理ログに記録する
						trans->setSynchronizeDone(
							Trans::Log::File::Category::Database);
					}
				}
			}
	}

	// システム表を操作するトランザクションをコミットし、
	// トランザクション中にかけたロックをはずす

	trans->commit();

	return incomplete;
}

}

//	FUNCTION private
//	Checkpoint::Manager::FileSynchronizer::initialize --
//		マネージャーの初期化のうち、
//		バージョンファイルの同期処理関連の初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Manager::FileSynchronizer::initialize()
{}

//	FUNCTION private
//	Checkpoint::Manager::FileSynchronizer::terminate --
//		マネージャーの後処理のうち、
//		バージョンファイルの同期処理関連の後処理を行う
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::FileSynchronizer::terminate()
{
	// バージョンファイルの同期処理を行うべき
	// データベースのオブジェクトIDを管理するマップを空にする

	delete _FileSynchronizer::_candidateMap,
		_FileSynchronizer::_candidateMap = 0;
}

//	FUNCTION private
//	Checkpoint::FileSynchronizer::repeatable --
//		存在するすべてのバージョンファイルの同期を取る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileSynchronizer::repeatable()
{
	// チェックポイント処理を実行不可にする

	Daemon::AutoDisabler disabler(Daemon::Category::Executor);

	// ステータス報告

	Daemon::AutoState cState;

	if (!(getStatus() != Aborting && isEnabled() &&
		  Server::Manager::isAvailable() && _FileSynchronizer::_candidateMap))
	{
		// スレッドが終了処理中、実行不可、サーバーが利用不可、
		// 同期処理の候補が求められなかったので、なにもしない

		SydMessage << "Skip database synchronization." << ModEndl;

		return;
	}

	// スレッドが終了処理中でなく、実行可、サーバーが利用可であれば、
	// 候補に対してひとつづつ同期処理を行っていく

	; _SYDNEY_ASSERT(_FileSynchronizer::_candidateMap);
	_FileSynchronizer::_DatabaseMap::Iterator
		ite(_FileSynchronizer::_candidateMap->begin());
	const _FileSynchronizer::_DatabaseMap::Iterator&
		end = _FileSynchronizer::_candidateMap->end();
	_FileSynchronizer::_DatabaseMap::Iterator next;

	try
	{

		for (; ite != end &&
				 getStatus() != Aborting && isEnabled() &&
				 Server::Manager::isAvailable(); ite = next) {

			// 反復子の指す要素を削除すると、
			// 次の要素が得られなくなるので、
			// ここで次の要素を指す反復子を求めておく

			++(next = ite);

			// 今回の同期処理のタイミングでは実行しないデータベースに
			// 存在しているかチェックする
		
			if (_FileSynchronizer::_skipMap.find((*ite).first)
				!= _FileSynchronizer::_skipMap.end())
			{
				// データベースが更新されている限り
				// 同期処理は実行しない
				
				// 今回のタイミングでは実行しない
				// スキップした回数を記憶する
				
				(*ite).second++;
			
				continue;
			}

			; _SYDNEY_FAKE_ERROR("Checkpoint::FileSynchronizer::execute",
								 Exception::Unexpected(moduleName,
													   srcFile, __LINE__));
			; _SYDNEY_FAKE_ERROR("Checkpoint::FileSynchronizer::execute", 0);

			if (!_FileSynchronizer::sync((*ite).first))

				// 候補に対して完全に同期処理を行ったので、候補でなくする

				_FileSynchronizer::_candidateMap->erase(ite);

			else
				// 同期処理を行ったので、スキップした回数を0にする

				(*ite).second = 0;

		}

	}
	catch (Exception::Object& e)
	{
		// Sydneyの例外が発生した場合にはログに記録するだけ
		
		SydErrorMessage << "Database synchronization failed. " << e << ModEndl;
	}
	catch (ModException& e)
	{
		// Modの例外が発生した場合にはログに記録するだけ
		
		SydErrorMessage << "Database synchronization failed. "
						<< _MOD_EXCEPTION(e) << ModEndl;
	}
#ifndef NO_CATCH_ALL
	catch (...)
	{
		// 実行しないデータベースのマップを空にする
	
		_FileSynchronizer::_skipMap.erase(
			_FileSynchronizer::_skipMap.begin(),
			_FileSynchronizer::_skipMap.end());

		SydErrorMessage << "Database synchronization failed. Server is not available." << ModEndl;

		Server::Manager::setAvailability(false);

		throw;
	}
#endif
	
	// 実行しないデータベースのマップを空にする
	
	_FileSynchronizer::_skipMap.erase(
		_FileSynchronizer::_skipMap.begin(),
		_FileSynchronizer::_skipMap.end());

	SydMessage << "Database synchronized" << ModEndl;

}

//	FUNCTION private
//	Checkpoint::FileSynchronizer::enter --
//		同期処理の候補であるデータベースを登録する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectID::Value	dbID
//			候補として登録するデータベースのオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
FileSynchronizer::enter(Schema::ObjectID::Value dbID)
{
	try {

		if (_FileSynchronizer::_candidateMap)
		{
			(void) _FileSynchronizer::_candidateMap->insert(dbID, 0);
		}
		
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		// 同期処理ができなくても、
		// データベースサイズが小さくならないだけなので、エラーは無視する

		Common::Thread::resetErrorCondition();
	}
}

//
//	FUNCTION public
//	Checkpoint::FileSynchronizer::skip --
//		今回の同期処理のタイミングでは同期処理を実行しないデータベースを登録する
//
//	NOTES
//
//	ARGUMENTS
//	Schema::ObjectID::Value dbID
//		実行しないデータベースのオブジェクトID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
// static
void
FileSynchronizer::skip(Schema::ObjectID::Value dbID)
{
	try
	{
		(void) _FileSynchronizer::_skipMap.insert(dbID, dbID);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// エラーは無視する
		Common::Thread::resetErrorCondition();
	}
}

//
//	FUNCTION public
//	Checkpoint:FileSynchronizer::prepare -- 同期処理の前準備を行う
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
FileSynchronizer::prepare()
{
	//	【注意】
	//	チェックポイント処理の中からしか実行してはならない
	
	_FileSynchronizer::prepare();
}

//
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
