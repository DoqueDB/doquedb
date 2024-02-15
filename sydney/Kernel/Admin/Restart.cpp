// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Restart.cpp -- システムの再起動関連の関数定義
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#include "Admin/Database.h"
#include "Admin/Restart.h"
#include "Admin/FakeError.h"

#include "Checkpoint/Database.h"
#include "Checkpoint/Executor.h"
#include "Checkpoint/LogData.h"
#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/Thread.h"
#include "Schema/Database.h"
#include "Schema/Manager.h"
#include "Schema/SystemTable.h"
#include "Server/Manager.h"
#include "Trans/AutoLogFile.h"
#include "Trans/AutoTransaction.h"
#include "Trans/LogData.h"

#include "Exception/ModLibraryError.h"
#include "Exception/LogFileCorrupted.h"

#include "ModAutoPointer.h"

#include <exception>

_SYDNEY_USING
_SYDNEY_ADMIN_USING

namespace
{

namespace _Restart
{
	// 再起動時に回復処理が必要か調べる
	bool					isNecessary(int iRetry_);
}

namespace _Recovery
{
	namespace _Database
	{
		struct _LessEqual
			: public ModBinaryFunction<Trans::TimeStamp::Value,
									   Trans::TimeStamp::Value, ModBoolean>
		{
			ModBoolean	operator ()(const Trans::TimeStamp::Value& l,
									const Trans::TimeStamp::Value& r) const
			{
				return (l <= r) ? ModTrue : ModFalse;
			}
		};

		//	CLASS
		//	$$$::_Recovery::_Database::_AscMap --
		//		Admin::Recovery::Database* を管理するためのマップ
		//
		//	NOTES

		struct _AscMap
			: public	ModMap<Trans::TimeStamp::Value,
							   Recovery::Database*,
							   _LessEqual >
		{
			// コンストラクター
			_AscMap();
			// デストラクター
			~_AscMap();

			// 必要があれば、登録する
			bool
			insert(Recovery::Database* dbRecovery);
		};
	}
}

//	FUNCTION
//	$$$::_Restart::isNecessary -- 再起動時に回復処理が必要か調べる
//
//	NOTES
//		この関数の呼び出し前に Trans::TimeStamp::assign や
//		Trans::TimeStamp::getSystemInitialized により、
//		タイムスタンプを生成してはいけない
//
//	ARGUMENTS
//		int iRetry_
//			残り再試行回数
//
//	RETURN
//		true
//			回復処理が必要である
//		false
//			回復処理が必要でない
//
//	EXCEPTIONS

bool
_Restart::isNecessary(int iRetry_)
{
	// システム用の論理ログファイルを操作するための情報を得る

	Trans::Log::AutoFile logFile(Schema::Manager::SystemTable::getLogFile());

	// システム用の論理ログファイルの回復処理を実行する

	logFile->recover();

	// システム用の論理ログファイルの
	// 末尾に記録されている論理ログのログシーケンス番号を得る
	//
	//【注意】	この時点では、論理ログファイルをラッチする必要はない

	Trans::Log::LSN lsn = logFile->getLastLSN();
	if (lsn != Trans::Log::IllegalLSN) {

		// 論理ログファイルの末尾の論理ログが存在するので、読み出す

		ModAutoPointer<const Trans::Log::Data>	data(logFile->load(lsn));
		if (!data.isOwner())
			_SYDNEY_THROW0(Exception::LogFileCorrupted);

		if (data->getCategory() ==
			Trans::Log::Data::Category::CheckpointSystem) {

			// システムに関するチェックポイント処理の終了を表す論理ログが
			// 論理ログファイルの末尾に記録されている

			const Checkpoint::Log::CheckpointSystemData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Checkpoint::Log::CheckpointSystemData&, *data);

			if (tmp.isTerminated() &&
				(tmp.getUnavailableDatabase().isEmpty() || iRetry_ <= 0)) {

				// チェックポイントスレッドの終了処理中の
				// チェックポイント処理が無事に終了しているおり、
				// そのとき、利用不可なデータベースがひとつもなかったか、
				// 再起動時の回復処理を繰り返しても利用不可な
				// データベースを無くせなかったので、
				// 再起動時の回復処理は必要ない

				// 利用不可なデータベースの情報を設定する

				Checkpoint::Database::setAvailability(
					tmp.getUnavailableDatabase());

				// この時点でヒューリスティックに解決済の
				// トランザクションブランチの情報を得て、
				// ヒューリスティックに解決済の
				// トランザクションブランチが存在するようにする

				Trans::Branch::redo(tmp);

				return false;
			}
		}

		// 前回起動時に、チェックポイントスレッドは
		// 終了処理なしで終了しているので、
		// 再起動時の回復処理は必要になる

		return true;
	}

	// システム用の論理ログファイルにひとつも論理ログが記録されていないので、
	// システムは一度も起動したことがない

	return false;
}

//	FUNCTION public
//	$$$::_Recovery::_Database::_AscMap::_AscMap --
//		Admin::Recovery::Database* を管理するためのマップのコンストラクター
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

inline
_Recovery::_Database::_AscMap::_AscMap()
{}

//	FUNCTION public
//	$$$::_Recovery::_Database::_AscMap::~_AscMap --
//		Admin::Recovery::Database* を管理するためのマップのデストラクター
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

_Recovery::_Database::_AscMap::~_AscMap()
{
	if (getSize()) {
		Iterator	ite(begin());
		do {
			delete (*ite).second;
		} while (++ite != end()) ;
	}
}

//	FUNCTION public
//	$$$::_Recovery::_Database::_AscMap::insert --
//		与えられた Admin::Recovery::Database* を必要があれば登録する
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Recovery::Database*	dbRecovery
//			登録するデータベースを回復するためのクラスを格納する領域
//
//	RETURN
//		true
//			登録した
//		false
//			登録しなかった
//
//	EXCEPTIONS

inline
bool
_Recovery::_Database::_AscMap::insert(Recovery::Database* dbRecovery)
{
	if (dbRecovery && (!dbRecovery->isRedone() || dbRecovery->_first)) {
		(void) ModMap<Trans::TimeStamp::Value,
					  Recovery::Database*, _LessEqual>::insert(
						  static_cast<Trans::TimeStamp::Value>(
							  (!dbRecovery->isRedone()) ?
							  dbRecovery->_logData->getTimeStamp() :
							  dbRecovery->_starting), dbRecovery);
		return true;
	}
	return false;
}

}

//	FUNCTION
//	Admin::Restart::recover --
//		システムの再起動時にシステムを必要があれば、回復する
//
//	NOTES
//
//	ARGUMENTS
//		int iRetry_
//			残り再試行回数
//
//	RETURN
//		true
//			回復処理を行ったので、一度システムを起動しなおす必要がある
//		false
//			回復処理の必要はなかったので、起動処理を継続してよい
//
//	EXCEPTIONS

bool
Restart::recover(int iRetry_)
{
	if (!_Restart::isNecessary(iRetry_))

		// 回復処理は必要ない

		return false;

	SydMessage << "Start database recovery." << ModEndl;

	// recover password file
	Server::Manager::recoverPasswordFile();

	// 回復処理用のトランザクションを開始する

	Trans::AutoTransaction	trans(Trans::Transaction::attach());
	trans->begin(Schema::ObjectID::SystemTable,
				 Trans::Transaction::Mode(
					 Trans::Transaction::Category::ReadWrite,
					 Trans::Transaction::IsolationLevel::Serializable,
					 Boolean::False),
				 true, true);

	// スキーマデータベースの回復処理を行うためのクラスを生成する

	ModAutoPointer<Recovery::Database>	dbRecovery0(
		new Recovery::Database(
			*Schema::Database::get(Schema::ObjectID::SystemTable, *trans)));

	Checkpoint::Database::UnavailableMap	unavailableDatabase;

	try
	{

		// まず、システム用の論理ログファイルを調べて、
		// スキーマデータベースの回復処理の基点となる
		// 論理ログのログシーケンス番号を得る
		//
		// 同時に、回復処理を開始する時点のタイムスタンプを得る

		dbRecovery0->findStartLSN(*trans, unavailableDatabase);

		if (!dbRecovery0->_recovered) {

			// スキーマデータベースのうち、
			//「データベース」表を構成するすべての物理ファイルを
			// 回復処理の開始時点に戻す

			Schema::Manager::recover(*trans, dbRecovery0->_starting);
			dbRecovery0->_recovered = true;
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// 「データベース」表のリカバリーに失敗した
		// 実際に Server not available に設定するのは上位層

		SydErrorMessage << "Recovering database '"
						<< dbRecovery0->_database.getName()
						<< "' failed. Server is not available." << ModEndl;
		Common::Thread::resetErrorCondition();

		_SYDNEY_RETHROW;
	}

	// データベースの回復処理を行うクラスを
	// 登録するためのマップを用意する
	//
	//【注意】	マップには、データベースの回復処理を行うためのクラスが、
	//			それぞれの REDO すべき操作のうち、
	//			最も昔に実行したものを表す論理ログを
	//			記録するときに取得したタイムスタンプの昇順に登録される

	_Recovery::_Database::_AscMap	ascMap;

	//「データベース」表に登録されているデータベースごとに処理する
	// ★注意★
	// 回復処理の中でデータベース一覧のキャッシュがクリアされることがあるので
	// ここでVectorにコピーしておく
	ModVector<Schema::Database*>	databases(Schema::Manager::ObjectTree::Database::get(*trans));

	ModVector<Schema::Database*>::ConstIterator			ite(databases.begin());
	const ModVector<Schema::Database*>::ConstIterator&	end = databases.end();

	for (; ite != end; ++ite) {

		Schema::Database*	database = *ite;
		; _SYDNEY_ASSERT(database);

		// このデータベースが利用不可か調べる

		const Checkpoint::Database::UnavailableMap::Iterator&
			i = unavailableDatabase.find(database->getID());

		// 障害回復を開始するタイムスタンプを求める
		
		Trans::TimeStamp timestamp = ((i == unavailableDatabase.end()) ?
									  dbRecovery0->_starting : (*i).second._t);
		try {

			// このデータベースの回復処理を行うためのクラスを生成する
			//
			// データベースが利用不可であれば、
			// データベースが利用不可になったときの
			// 2 つ前のチェックポイント処理の終了時点から障害回復を開始する

			ModAutoPointer<Recovery::Database>	dbRecovery1(
				new Recovery::Database(*database, timestamp));

			// データベース用の論理ログファイルを調べて、
			// データベースの回復処理の基点となる
			// 論理ログのログシーケンス番号を得る
			//
			//【注意】	データベースが利用不可であれば、
			//			データベースが利用不可になったときの
			//			2 つ前のチェックポイント処理の終了時点から
			//			障害回復を開始する

			dbRecovery1->findStartLSN(*trans);

			// このデータベースの
			// 回復処理を行うためのクラスをマップに登録する

			if (ascMap.insert(dbRecovery1.get()))
				dbRecovery1.release();

		}
		catch (Exception::Object& e)
		{
			SydErrorMessage << e << ModEndl;
			SydErrorMessage << "Recovering database '"
							<< database->getName()
							<< "' failed. Database is not available."
							<< ModEndl;
			
			// リカバリーする前に利用不可かチェックし、
			// 利用不可な場合は、利用不可になった時点のタイムスタンプを設定する
			
			Checkpoint::Database::setAvailability(database->getID(),
												  false, timestamp);
			// reset error status
			Common::Thread::resetErrorCondition();
		}
		catch (ModException& e)
		{
			SydErrorMessage << Exception::ModLibraryError(moduleName,
														  srcFile, __LINE__, e)
							<< ModEndl;
			SydErrorMessage << "Recovering database '"
							<< database->getName()
							<< "' failed. Database is not available."
							<< ModEndl;

			// リカバリーする前に利用不可かチェックし、
			// 利用不可な場合は、利用不可になった時点のタイムスタンプを設定する
			
			Checkpoint::Database::setAvailability(database->getID(),
												  false, timestamp);
			// reset error status
			Common::Thread::resetErrorCondition();
		}
#ifndef NO_CATCH_ALL
		catch (std::exception& e)
		{
			SydErrorMessage << "std::exception occurred. "
							<< (e.what() ? e.what() : "") << ModEndl;
			SydErrorMessage << "Recovering database '"
							<< database->getName()
							<< "' failed. Database is not available."
							<< ModEndl;

			// リカバリーする前に利用不可かチェックし、
			// 利用不可な場合は、利用不可になった時点のタイムスタンプを設定する
			
			Checkpoint::Database::setAvailability(database->getID(),
												  false, timestamp);
			// reset error status
			Common::Thread::resetErrorCondition();
		}
		catch (...)
		{
			SydErrorMessage << "Recovering database '"
							<< database->getName()
							<< "' failed. Database is not available."
							<< ModEndl;

			// リカバリーする前に利用不可かチェックし、
			// 利用不可な場合は、利用不可になった時点のタイムスタンプを設定する
			
			Checkpoint::Database::setAvailability(database->getID(),
												  false, timestamp);
			// reset error status
			Common::Thread::resetErrorCondition();
		}
#endif
	}

	// スキーマデータベースの
	// 回復処理を行うためのクラスをマップに登録する

	if (ascMap.insert(dbRecovery0.get()))
		dbRecovery0.release();

	while (ascMap.getSize()) {

		// マップの先頭に登録されている
		// データベースの回復処理を行うためのクラスを得る
		//
		//【注意】	dbRecovery0 の領域は ascMap が管理しているので、
		//			直接 delete してはいけない

		Recovery::Database*	dbRecovery0 = (*ascMap.begin()).second;
		; _SYDNEY_ASSERT(dbRecovery0);
		ModAutoPointer<Recovery::Database> dbRecovery1;

		try
		{

			if (dbRecovery0->_first) {

				// このデータベースを初めて処理しようとしている

				if (Trans::TimeStamp::getLast() < dbRecovery0->_starting)

					// 回復処理を開始する時点のタイムスタンプを
					// 現在のタイムスタンプ値とする

					Trans::TimeStamp::redo(
						Trans::Log::TimeStampAssignData(
							dbRecovery0->_starting));

				if (!dbRecovery0->_recovered) {

					// 必要があれば、データベースを構成する
					// 物理ファイルを回復処理の開始時点の状態に戻す
					//
					//【注意】	この処理にはスキーマ情報の参照が必要になるので、
					//			実行順に行う必要がある

					dbRecovery0->_database.recover(*trans,
												   dbRecovery0->_starting);
					dbRecovery0->_recovered = true;
				}

				ADMIN_FAKE_ERROR("Admin::Restart", "Recover", "Undo");
				_SYDNEY_FAKE_ERROR("Admin::Restart_Recover_Undo",
								   ModException());
				_SYDNEY_FAKE_ERROR("Admin::Restart_Recover_Undo", 0);

				// 回復処理の開始時点に実行中のトランザクションのうち、
				// 障害発生時までにコミットされなかったものによる操作をUNDOする

				dbRecovery0->undoAll(*trans);

				// 次からは以上の処理をしないようにする

				dbRecovery0->_first = false;
			}

			// データベースに対してひとつの操作を REDO する

			dbRecovery1 = dbRecovery0->redo(*trans);
			
		}
		catch (Exception::Object& e)
		{
			SydErrorMessage << e << ModEndl;
			SydErrorMessage << "Recovering database '"
							<< dbRecovery0->_database.getName()
							<< "' failed. Database is not available."
							<< ModEndl;

			Schema::Database::ID::Value id = dbRecovery0->_database.getID();
			if (id != Schema::ObjectID::SystemTable)
			{
				// システム表以外のデータベースの場合
				// 障害回復開始時点のタイムスタンプを設定する

				Trans::TimeStamp timestamp = dbRecovery0->_starting;

				Checkpoint::Database::setAvailability(id, false, timestamp);
			}

			ascMap.erase(ascMap.begin());
			delete dbRecovery0;

			if (id == Schema::ObjectID::SystemTable)
				// システム表のデータベースの場合は、Server not available
				// にする。実際の設定は上位層で行う
				_SYDNEY_RETHROW;
				
			// 次のデータベースを処理する
			continue;
		}
		catch (ModException& e)
		{
			SydErrorMessage << Exception::ModLibraryError(moduleName, srcFile, __LINE__, e) << ModEndl;
			SydErrorMessage << "Recovering database '"
							<< dbRecovery0->_database.getName()
							<< "' failed. Database is not available."
							<< ModEndl;
			
			Schema::Database::ID::Value id = dbRecovery0->_database.getID();
			if (id != Schema::ObjectID::SystemTable)
			{
				// システム表以外のデータベースの場合
				// 障害回復開始時点のタイムスタンプを設定する

				Trans::TimeStamp timestamp = dbRecovery0->_starting;

				Checkpoint::Database::setAvailability(id, false, timestamp);
			}

			// reset error status
			Common::Thread::resetErrorCondition();

			ascMap.erase(ascMap.begin());
			delete dbRecovery0;

			if (id == Schema::ObjectID::SystemTable)
				// システム表のデータベースの場合は、Server not available
				// にする。実際の設定は上位層で行う
				_SYDNEY_RETHROW;
				
			// 次のデータベースを処理する
			continue;
		}
#ifndef NO_CATCH_ALL
		catch (std::exception& e)
		{
			SydErrorMessage << "std::exception occurred. "
							<< (e.what() ? e.what() : "") << ModEndl;
			SydErrorMessage << "Recovering database '"
							<< dbRecovery0->_database.getName()
							<< "' failed. Database is not available."
							<< ModEndl;
			
			Schema::Database::ID::Value id = dbRecovery0->_database.getID();
			if (id != Schema::ObjectID::SystemTable)
			{
				// システム表以外のデータベースの場合
				// 障害回復開始時点のタイムスタンプを設定する

				Trans::TimeStamp timestamp = dbRecovery0->_starting;

				Checkpoint::Database::setAvailability(id, false, timestamp);
			}

			// reset error status
			Common::Thread::resetErrorCondition();

			ascMap.erase(ascMap.begin());
			delete dbRecovery0;

			if (id == Schema::ObjectID::SystemTable)
				// システム表のデータベースの場合は、Server not available
				// にする。実際の設定は上位層で行う
				_SYDNEY_RETHROW;
				
			// 次のデータベースを処理する
			continue;
		}
		catch (...)
		{
			SydErrorMessage << "Recovering database '"
							<< dbRecovery0->_database.getName()
							<< "' failed. Database is not available."
							<< ModEndl;
			
			ascMap.erase(ascMap.begin());
			delete dbRecovery0;

			// Server not availableにする。実際の設定は上位層で行う
			_SYDNEY_RETHROW;
		}
#endif

		// データベースの回復処理を行うためのクラスを登録しなおす

		ascMap.erase(ascMap.begin());
		if (!ascMap.insert(dbRecovery0))
			delete dbRecovery0;

		// 今回、生成またはマウントされた
		// データベースの回復処理を行うためのクラスを登録する

		if (ascMap.insert(dbRecovery1.get()))
			dbRecovery1.release();
			
	}

	// 回復処理が終了したので、
	// スキーマモジュールが回復処理用に確保したデータを破棄する

	Schema::Manager::Recovery::notifyDone();

	// 回復処理用のトランザクションを終了する

	trans->commit();

	// 回復処理を行ったので、システムを終了し、
	// 回復したデータベースを完全に永続化する必要がある

	return true;
}

//
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
