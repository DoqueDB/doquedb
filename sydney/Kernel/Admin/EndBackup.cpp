// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// EndBackup.cpp -- バックアップの終了関連の関数定義
// 
// Copyright (c) 2001, 2002, 2004, 2007, 2009, 2013, 2015, 2023 Ricoh Company, Ltd.
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

#include "Admin/EndBackup.h"
#include "Admin/Operation.h"
#include "Admin/Utility.h"

#include "Checkpoint/Daemon.h"
#include "Common/UnsignedInteger64Data.h"
#include "Exception/DatabaseChanged.h"
#include "Exception/DatabaseNotFound.h"
#include "Exception/NotStartBackup.h"
#include "Schema/Database.h"
#include "Schema/Hold.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Server/Session.h"
#include "Statement/EndBackupStatement.h"
#include "Trans/AutoLatch.h"
#include "Trans/AutoLogFile.h"
#include "Trans/LogData.h"
#include "Trans/TimeStamp.h"
#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_ADMIN_USING

namespace
{
}

//	FUNCTION public
//	Admin::EndBackup::execute -- バックアップを終了する
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::EndBackupStatement* stmt
//			END BACKUP 文を表すオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
EndBackup::execute(const Statement::EndBackupStatement* stmt)
{
	// 初期化する

	if (stmt) initialize(*stmt);
	
	// メタデータベース、データベース表と
	// バックアップを終えるデータベースの情報を格納する
	// データベース表のタプルをロックしてから、
	// データベースを表すスキーマ情報を取得する

	Schema::Database* database = Schema::Database::getLocked(
		m_cTrans, m_cstrDatabaseName,
		Lock::Name::Category::Tuple, Schema::Hold::Operation::ReadForImport,
		Lock::Name::Category::Tuple, Schema::Hold::Operation::ReadForImport);

	if (!database)

		// 指定されたデータベース名が取得されてから、
		// これまでにデータベースが破棄されている

		_SYDNEY_THROW1(Exception::DatabaseNotFound, m_cstrDatabaseName);

	// EndBackup::endAll から呼ばれる場合は、m_pSession が null となる
	// null 以外の時のみ、データベースIDを確認する
	
	if (m_pSession && database->getID() != m_pSession->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		m_pSession->setDatabaseInfo(database->getID(), database->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// トランザクションで操作するデータベースを設定する
	//
	//【注意】	すぐに論理ログを記録しなくても、
	//			スキーマ操作関数で論理ログファイルを
	//			参照しに行く可能性があるので、
	//			操作対象のデータベースのスキーマ情報を取得したら、
	//			すぐトランザクションに設定すること

	m_cTrans.setLog(*database);

	// キャッシュが破棄されないようにデータベースをオープンしておき、
	// スコープから抜けた時点で自動的にクローズされるようにする

	Utility::AutoDatabaseCloser autoCloser(*database);

	// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
	// また、操作対象であるデータベース対して実行可能な SQL 文か調べる
	//
	// 引数の stmt を利用しないのは、stmt が null の場合があるためである
	
	const Statement::EndBackupStatement stmt2;
	Operation::isApplicable(m_cTrans, *database, &stmt2, m_pSession);

	// トランザクションが指定されたデータベースの
	// バックアップを開始しているか調べる

	if (!Utility::Backup::isEntered(m_cstrDatabaseName, m_cTrans))

		// バックアップを終了しようとしたデータベースの
		// バックアップを開始していない

		_SYDNEY_THROW1(Exception::NotStartBackup, m_cstrDatabaseName);

	if (m_bDiscarding)
	{
		// 不要な論理ログを削除するオプションが指定されているので、
		// 論理ログの不要な部分の削除を指示する
		// 実際には、トランザクションのコミット時に削除される

		m_cTrans.discardLog(false);
	}

	//【注意】	以降、エラーが起きたら、継続不能である

	// 3.バックアップを終了するデータベースを構成する
	//	 論理ファイル(データベース表以外のスキーマ情報を格納するものを含む)
	//	 ごとに、それぞれの論理ファイルドライバのendBackup を呼び出して、
	//	 論理ファイルに対してバックアップの終了を指示する。
	//	 ※この endBackup は、下位モジュールの関数を使って、バックアップ中の
	//	   バッファリング内容のフラッシュや空き領域の再利用の抑制をやめる。
	
	database->endBackup(m_cTrans);

	// バックアップ中に禁止していたチェックポイント処理
	// およびバージョンファイルの同期処理を解禁する

	Checkpoint::Daemon::enable();

	// トランザクションが指定されたデータベースの
	// バックアップを開始していることを記憶していたが、忘れる

	Utility::Backup::leave(m_cstrDatabaseName, m_cTrans);
}

//	FUNCTION public
//	Admin::EndBackup::endAll
//		-- 全て EndBackup する
//
//	NOTES
//		トランザクションで開始されている Backup を全て EndBackup する
//
//	ARGUMENTS
//		const Trans::Transaction& cTrans_
//			トランザクション記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
EndBackup::endAll(Trans::Transaction& trans)
{
	// トランザクションがバックアップ中のデータベースの名前を得る

	ModVector<ModUnicodeString>	dbNames;
	Utility::Backup::getEnteredDatabaseName(trans, dbNames);

	if (dbNames.getSize()) {
		ModVector<ModUnicodeString>::Iterator			ite(dbNames.begin());
		const ModVector<ModUnicodeString>::Iterator&	end = dbNames.end();

		for (; ite != end; ++ite)
			EndBackup(trans, 0, *ite).execute();
	}
}

//	FUNCTION private
//	Admin::EndBackup::initialize -- 初期化する
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
EndBackup::initialize(const Statement::EndBackupStatement& stmt)
{
	// DISCARD LOGICALLOG が指定されたか

	m_bDiscarding =
		(static_cast<
		 Statement::LogicalLogOption::Type>(stmt.getLogicalLogOption()) ==
		 Statement::LogicalLogOption::Discard);
}

//
//	Copyright (c) 2001, 2002, 2004, 2007, 2009, 2013, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
