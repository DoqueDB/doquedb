// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.cpp -- 管理マネージャー関連の関数定義
// 
// Copyright (c) 2001, 2002, 2003, 2005, 2006, 2007, 2009, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
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

#include "Admin/EndBackup.h"
#include "Admin/Externalizable.h"
#include "Admin/Manager.h"
#include "Admin/Mount.h"
#include "Admin/StartBackup.h"
#include "Admin/Unmount.h"
#include "Admin/Utility.h"

#include "Exception/DatabaseChanged.h"
#include "Exception/ModLibraryError.h"
#include "Exception/NumberCancel.h"
#include "Exception/Unexpected.h"
#include "Server/Session.h"
#include "Statement/EndBackupStatement.h"
#include "Statement/Identifier.h"
#include "Statement/MountDatabaseStatement.h"
#include "Statement/StartBackupStatement.h"
#include "Statement/Type.h"
#include "Statement/UnmountDatabaseStatement.h"
#include "Statement/VerifyStatement.h"
#include "Trans/LogData.h"

#include "Common/Externalizable.h"
#include "Common/Message.h"

_SYDNEY_USING
_SYDNEY_ADMIN_USING

//
//	FUNCTION
//	Admin::Manager::initialize -- 初期化する
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
Manager::initialize()
{
	// トランザクション関連のシリアル化可能なオブジェクトを
	// 確保するための関数を共通ライブラリに登録する

	Common::Externalizable::insertFunction(
		Common::Externalizable::AdminClasses,
			Admin::Externalizable::getClassInstance);
}

//
//	FUNCTION
//	Admin::Manager::terminate -- 終了処理する
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
Manager::terminate()
{
}

//	FUNCTION
//	Admin::Manager::executeStatement -- 管理用の SQL 文を実際に実行する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			管理用の SQL 文を実行するトランザクションのトランザクション記述子
//		Server::Session* pSession_,
//			管理用の SQL 文を実行するセッションのセッション記述子
//		Statement::Object&	stmt
//			実行する管理用の SQL 文を表すクラス
//		Communication::Connection&	connection
//			実行結果を設定するコネクションを表すクラス
//
//	RETURN
//		bool
//			true ... スキーマ情報のキャッシュを更新する必要がある
//			false... スキーマ情報のキャッシュを更新する必要はない
//
//	EXCEPTIONS

bool
Manager::
executeStatement(
	Trans::Transaction& trans, Server::Session* pSession_,
	const Statement::Object& stmt, Communication::Connection& connection)
{
	bool bResult = false;
	switch (stmt.getType()) {
	case Statement::ObjectType::UnmountDatabaseStatement:
	{
		SydMessage << stmt.toSQLStatement() << ModEndl;
	
		// アンマウントするデータベース名を得る

		const Statement::UnmountDatabaseStatement& tmp =
			_SYDNEY_DYNAMIC_CAST(
				const Statement::UnmountDatabaseStatement&, stmt);

		const Statement::Identifier* identifier = tmp.getDatabaseName();
		; _SYDNEY_ASSERT(identifier);
		; _SYDNEY_ASSERT(identifier->getIdentifier());

		// アンマウントを実行する

		Utility::PathList result;
		Unmount(trans, pSession_, *identifier->getIdentifier()).execute(&tmp, result);

		// 実行結果を設定する

		Utility::setResult(connection, result);

		bResult = true;
		break;
	}
	case Statement::ObjectType::MountDatabaseStatement:
	{
		SydMessage << stmt.toSQLStatement() << ModEndl;
	
		// マウントするデータベース名を得る

		const Statement::MountDatabaseStatement& tmp =
			_SYDNEY_DYNAMIC_CAST(
				const Statement::MountDatabaseStatement&, stmt);

		const Statement::Identifier* identifier = tmp.getDatabaseName();
		; _SYDNEY_ASSERT(identifier);
		; _SYDNEY_ASSERT(identifier->getIdentifier());

		// マウントを実行する

		Mount(trans, pSession_, *identifier->getIdentifier()).execute(&tmp);

		bResult = true;
		break;
	}
	case Statement::ObjectType::StartBackupStatement:
	{
		SydMessage << "[" << pSession_->getDatabaseName() << "] "
				   << stmt.toSQLStatement() << ModEndl;
	
		// バックアップを実行する

		const Statement::StartBackupStatement& tmp =
			_SYDNEY_DYNAMIC_CAST(const Statement::StartBackupStatement&, stmt);
		
		Utility::PathList result;
		StartBackup(trans, pSession_, pSession_->getDatabaseName()).execute(&tmp, result);

		// 実行結果を設定する

		Utility::setResult(connection, result);
		break;
	}
	case Statement::ObjectType::EndBackupStatement:
	{
		SydMessage << "[" << pSession_->getDatabaseName() << "] "
				   << stmt.toSQLStatement() << ModEndl;
	
		// バックアップの終了を実行する

		const Statement::EndBackupStatement& tmp =
			_SYDNEY_DYNAMIC_CAST(const Statement::EndBackupStatement&, stmt);
		
		EndBackup(trans, pSession_, pSession_->getDatabaseName()).execute(&tmp);
		break;
	}
	case Statement::ObjectType::VerifyStatement:
	{
		// 整合性検査を実行する

		Verification::Progress result(connection);
		try {
			const Statement::VerifyStatement& tmp =
				_SYDNEY_DYNAMIC_CAST(const Statement::VerifyStatement&, stmt);
			
			Verification::verify(trans, pSession_, tmp, pSession_->getDatabaseName(), result);

		} catch (Exception::DatabaseChanged&) {

			// 再送するのみ、ログにも出さない
			
			throw;

		} catch (Exception::Object& exception) {

			// Sydney の例外は、実行結果の末尾に加えて、クライアントに返す

			if (exception.getErrorNumber() == Exception::ErrorNumber::Cancel)
				_SYDNEY_VERIFY_INTERRUPTED(result, "", exception);
			else
				_SYDNEY_VERIFY_ABORTED(result, "", exception);
			Utility::setResult(connection, result);

			// 例外を再送する

			_SYDNEY_RETHROW;

		} catch (ModException& modException) {

			// MOD の例外は ModLibraryError に読み替え、
			// 実行結果の末尾に加えて、クライアントに返す

			Exception::ModLibraryError
				exception(moduleName, srcFile, __LINE__, modException);
			_SYDNEY_VERIFY_ABORTED(result, "", exception);
			Utility::setResult(connection, result);

			// 読み替えた例外を投げる

			throw exception;

		} catch (...) {

			// その他の例外は Unexpected に読み替え、
			// 実行結果の末尾に加えて、クライアントに返す

			Exception::Unexpected exception(moduleName, srcFile, __LINE__);
			_SYDNEY_VERIFY_ABORTED(result, "", exception);
			Utility::setResult(connection, result);

			// 読み替えた例外を投げる

			throw exception;
		}

		// 実行結果をクライアントに返す

		Utility::setResult(connection, result);
		break;
	}
	default:
		; _SYDNEY_ASSERT(false);
	}

	return bResult;
}

//
//	Copyright (c) 2001, 2002, 2003, 2005, 2006, 2007, 2009, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
