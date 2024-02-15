// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Unmount.cpp -- アンマウント関連の関数定義
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2012, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Admin";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Admin/Unmount.h"
#include "Admin/Operation.h"
#include "Admin/Utility.h"

#include "Checkpoint/Daemon.h"
#include "Checkpoint/TimeStamp.h"
#include "Common/Assert.h"
#include "Common/UnsignedInteger64Data.h"
#include "Exception/DatabaseNotFound.h"
#ifdef OBSOLETE
#include "Exception/LogItemCorrupted.h"
#endif
#include "Schema/Database.h"
#include "Schema/Hold.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/SystemTable_Database.h"
#include "Trans/AutoLatch.h"
#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_ADMIN_USING

#ifdef OBSOLETE
//	FUNCTION public
//	Admin::Unmount::Unmount -- アンマウントを表すクラスのコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			アンマウントを行うトランザクションのトランザクション記述子
//		Schema::LogData&	cLog_
//			アンマウントを再実行するための論理ログデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Unmount::Unmount(Trans::Transaction& cTrans_, const Schema::LogData& cLog_)
	: m_cTrans(cTrans_),
	  m_pSession(0),
	  _dbName(Schema::Database::getName(cLog_))
{
	// ログデータから情報を取り出す
	//
	//【注意】	ログデータにはデータベース用の情報も含まれているので、
	//			Schema::Database::Log::Create::Num から処理すること

	int i = Schema::Database::Log::Create::Num;
	unpackMetaField(cLog_[i++].get(), Utility::Meta::Unmount::MostRecent);
	unpackMetaField(
		cLog_[i++].get(), Utility::Meta::Unmount::SecondMostRecent);
}
#endif

//
//	FUNCTION public
//	Admin::Unmount::execute --
//		Unmount する
//
//	NOTES
//		・Database で使用しているパスを求める
//		・アンマウントすることを論理ログに記録する
//		・トランザクションの新しいタイムスタンプ値を得て論理ログに記録する
//		・論理ログファイルをフラッシュする
//		・各論理ファイルの unmount を呼ぶ
//		・論理ログファイルの unmount を呼ぶ
//
//	ARGUMENTS
//
//		const Schema::Database& cDatabase_
//			対象データベース
//
//		Utility::PathList& cPathList_
//			パス格納リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
Unmount::
execute(const Statement::Object* pcStatement_, Utility::PathList& cPathList_)
{
	// wait for end of file synchronizer and disable it
	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::FileSynchronizer);

	Schema::Database* pcDatabase = 0;
		
	// メタデータベース、データベース表と
	// アンマウントするデータベースの情報を格納する
	// データベース表のタプルをロックしてから、
	// データベースを表すスキーマ情報を取得する

	pcDatabase = Schema::Database::getLocked(
		m_cTrans, _dbName,
		Lock::Name::Category::Tuple, Schema::Hold::Operation::Drop,
		Lock::Name::Category::Tuple, Schema::Hold::Operation::Drop,
		true /* force */);
	if (!pcDatabase)

		// 指定されたデータベース名を取得してから、
		// これまでにデータベースが破棄されている

		_SYDNEY_THROW1(Exception::DatabaseNotFound, _dbName);

	// If obtained database is flagged as not available, perform the special process
	bool bAvailable = pcDatabase->isAvailable();
	
	// トランザクションで操作するデータベースを設定する
	//
	//【注意】	すぐに論理ログを記録しなくても、
	//			スキーマ操作関数で論理ログファイルを
	//			参照しに行く可能性があるので、
	//			操作対象のデータベースのスキーマ情報を取得したら、
	//			すぐトランザクションに設定すること

	if (bAvailable)
		m_cTrans.setLog(*pcDatabase);

	// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
	// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

	Operation::isApplicable(
		m_cTrans, *pcDatabase, pcStatement_, m_pSession);

	// システム用の論理ログファイルを
	// データベース表のタプルを操作するためにロックする

	Schema::Manager::SystemTable::hold(
		m_cTrans, Schema::Hold::Target::LogicalLog,
		Lock::Name::Category::Tuple, Schema::Hold::Operation::Drop);

	if (bAvailable && !pcDatabase->isReadOnly())

		// READ WRITE なデータベースをアンマウントするので、
		// データベース用の論理ログファイルも
		// データベース表のタプルを操作するためにロックする

		Schema::Manager::SystemTable::hold(
			m_cTrans, Schema::Hold::Target::LogicalLog,
			Lock::Name::Category::Tuple, Schema::Hold::Operation::Drop,
			0, Trans::Log::File::Category::Database, pcDatabase);

	// アンマウントするデータベースを
	// データベースを操作するためにロックする

	Schema::Manager::SystemTable::hold(
		m_cTrans, Schema::Hold::Target::Database,
		Lock::Name::Category::Database, Schema::Hold::Operation::Drop,
		pcDatabase->getID());

	//--- UNMOUNT のためのファイル操作 ---

	// キャッシュが破棄されないようにopenする
	// ★注意★
	// unmountが成功するとオブジェクトが破棄されるので
	// 失敗時にcloseするのみでよい
	pcDatabase->open();

	// Status
	enum Status {
		Unknown,
		Unmounted,
		Store
	} eStat = Unknown;
		
	try {

		// 使用しているパスを求める
		Utility::makePathList(m_cTrans, *pcDatabase, m_cPathList);

		// 戻り値用のパスを設定する
		{
			ModSize n = m_cPathList.getSize();
		
			cPathList_.reserve(n);

			for ( ModSize i = 0; i < n; ++i )
				cPathList_.pushBack(new Os::Path(*m_cPathList[i]));
		}

		// アンマウントすることを表す論理ログを
		// システム用の論理ログファイルに記録する

		Schema::LogData logData(Schema::LogData::Category::Unmount);
		makeLogData(logData, *pcDatabase);
		{
			const Lock::Name& lockName = m_cTrans.getLogInfo(
											 Trans::Log::File::Category::System).getLockName();

			Trans::AutoLatch latch(m_cTrans, lockName);
			m_cTrans.storeLog(Trans::Log::File::Category::System, logData);
			m_cTrans.flushLog(Trans::Log::File::Category::System);
		}

		if (bAvailable && !pcDatabase->isReadOnly()) {

			// 先ほど記録したものとの同じものを
			// アンマウントするデータベース用の論理ログファイルに記録する

			const Lock::Name& lockName = m_cTrans.getLogInfo(
											 Trans::Log::File::Category::Database).getLockName();

			Trans::AutoLatch latch(m_cTrans, lockName);
			m_cTrans.storeLog(Trans::Log::File::Category::Database, logData);
			m_cTrans.flushLog(Trans::Log::File::Category::Database);
		}

		// j. Database に関するファイルの UNMOUNT
		pcDatabase->unmount(m_cTrans);
		eStat = Unmounted;

		// k. データベース表からの削除
		// l. キャッシュからの削除
		Schema::SystemTable::Database().store(m_cTrans, *pcDatabase);
		eStat = Store;
	}
	catch ( ... )
	{
		switch ( eStat )
		{
		case Store:
			// 再実行
			Schema::SystemTable::Database().store(m_cTrans, *pcDatabase);
			break;

		case Unmounted:
			// mountしなおす
			// ★注意★
			// 以前の実装ではデータベースのフラグだけを戻していたが
			// unmountで下位のオブジェクトでもフラグを変更するようになったので
			// mountを実行する必要がある

			if (bAvailable) {
				// mountはSystemTableとTableが別々に呼び出される
				pcDatabase->mountSystemTable(m_cTrans, true /* undo */);
				pcDatabase->mount(m_cTrans, true /* undo */);
			}
			// thru.

		case Unknown:
			// データベースをcloseする
			pcDatabase->close();
			break;
		}
		_SYDNEY_RETHROW;
	}
	
	// スキーマ情報に変化があったのでシステム表のタイムスタンプを増やす
	Schema::Manager::SystemTable::addTimestamp(m_cTrans);
}

#ifdef OBSOLETE
//	FUNCTION private
//	Admin::Unmount::getPath -- Database が使用するパスを取得する
//
//	NOTES
//
//	ARGUMENTS
//		Utility::PathList& cPath_
//			パスを格納する配列
//
//	RETURN
//	const Utility::PathList&
//		データのパスリスト
//
//	EXCEPTIONS

const ModVector< Os::Path* >&
Unmount::
getPath() const
{
	return m_cPathList;
}
#endif

//	FUNCTION private
//	Admin::Mount::makeLogData -- UNMOUNT 文用のログデータを作成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::LogData&	logData
//			作成されるログデータ
//		Schema::Database&	database
//			アンマウントするデータベースのスキーマオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Unmount::makeLogData(Schema::LogData& logData,
					 const Schema::Database& database)
{
	// データベースに関する情報を作成する

	database.makeLogData(m_cTrans, logData);

	// アンマウント独自の情報を作成する

	logData.addData(packMetaField(Utility::Meta::Unmount::MostRecent));
	logData.addData(packMetaField(Utility::Meta::Unmount::SecondMostRecent));
}

//	FUNCTION private
//	Admin::Unmount::packMetaField --
//		UNMOUNT 文用のログデータのある種別の情報を作成する
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Utility::Meta::Unmount::Type	type
//			作成したい情報の種別
//
//	RETURN
//		0 以外の値
//			作成された情報のオブジェクトポインタ
//		0
//			作成できなかった
//
//	EXCEPTIONS

Common::Data::Pointer
Unmount::packMetaField(Utility::Meta::Unmount::Type type) const
{
	switch (type) {
	case Utility::Meta::Unmount::MostRecent:

		// 前回のチェックポイント終了時のタイムスタンプ

		return new Common::UnsignedInteger64Data(
			Checkpoint::TimeStamp::getMostRecent());

	case Utility::Meta::Unmount::SecondMostRecent:

		// 前々回のチェックポイント終了時のタイムスタンプ

		return new Common::UnsignedInteger64Data(
			Checkpoint::TimeStamp::getSecondMostRecent());
	}
	return Common::Data::Pointer();
}

#ifdef OBSOLETE
//	FUNCTION private
//	Admin::Unmount::unpackMetaField --
//		UNMOUNT 文用のログデータのある種別の情報の値を得る
//
//	NOTES
//
//	ARGUMENTS
//		Common::Data*		data
//			ログ情報の要素
//		Admin::Utility::Meta::Mount::Type	type
//			値を得たい情報の種別
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		LogItemCorrupted
//			ログデータがおかしいため、値が得られなかった

void
Unmount::unpackMetaField(const Common::Data* data,
						 Utility::Meta::Unmount::Type type)
{
	if (data && data->getType() == getMetaFieldType(type))
		switch (type) {
		case Utility::Meta::Unmount::MostRecent:
		case Utility::Meta::Unmount::SecondMostRecent:

			// なにもしない

			break;
		}

	return;
}

//	FUNCTION private
//	Admin::Unmount::getMetaFieldType --
//		UNMOUNT 文用のログデータのある種別の情報の型を得る
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Utility::Meta::Unmount::Type	type
//			型を得たい情報の種別
//
//	RETURN
//		得られた型
//
//	EXCEPTIONS
//		なし

// static
Common::DataType::Type
Unmount::getMetaFieldType(Utility::Meta::Unmount::Type type)
{
	switch (type) {
	case Utility::Meta::Unmount::MostRecent:
	case Utility::Meta::Unmount::SecondMostRecent:
		return Common::DataType::UnsignedInteger64;
	}
	return Common::DataType::Undefined;
}
#endif

//
//	Copyright (c) 2001, 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
