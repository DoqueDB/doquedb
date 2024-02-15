// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SQLDispatchEntry.h -- SQL 文の種類からその SQL 文の実行の仕方に関する
//						 情報を取得するためのクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2007, 2012, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SERVER_SQLDISPATCHENTRY_H
#define __SYDNEY_SERVER_SQLDISPATCHENTRY_H

#include "Server/Module.h"
#include "Common/Privilege.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_SERVER_BEGIN

namespace Module
{
	enum Type
	{
		Undefined = 0,
		Optimizer,
		Schema,
		Admin,
		Server
	};
}

namespace Permission
{
	typedef	int		Type;
	enum
	{
		Never      = 0x00,
		Implicitly = 0x01,
		Explicitly = 0x02,
		Any        = 0x03
	};
}

//
//	STRUCT
//	Session::SQLDispatchTable ,Session::SQLDispatchEntry -- 
//      Statement の種類から Worker のモジュール呼び分けと
//      Transaction 管理方法を決定する決定表の構造体宣言
//
//	NOTES
//		テーブルを記述するために、構造体(つまりすべて public)とする。
//      また、すべてのアクセス関数は具象関数とせざるを得ない
//

class SQLDispatchEntry
{
public:
	int m_iID;						//検索キー（ROMTable<>で検索可能にするため）
	Module::Type m_eModule;			//モジュールの呼び分けを決定
	Permission::Type m_ePermission;	//Transaction 管理方法を決定

	// 読取専用トランザクションで実行可能か
	Boolean::Value			_executableInsideReadOnlyTransaction;
	// 読取専用データベースに対して実行可能か
	Boolean::Value			_executableOnReadOnlyDatabase;
	// オフラインデータベースに対して実行可能か
	Boolean::Value			_executableOnOfflineDatabase;
	// スレーブデータベースに対して実行可能か
	Boolean::Value			_executableOnSlaveDatabase;
	// データベース用の論理ログファイルに論理ログを記録するか
	Boolean::Value			_databaseLogRecorded;
	// システム用の論理ログファイルに論理ログを記録するか
	Boolean::Value			_systemLogRecorded;

	// for privilege
	Common::Privilege::Category::Value m_ePrivilegeCategory;
	Common::Privilege::Value m_ePrivilegeValue;
	
	// どのモジュールで処理するか
	Module::Type			getModuleType() const;
	// SQL 文を実行すべきトランザクションはどのように開始されたものか
	Permission::Type		getPermissionType() const;
	// 読取専用トランザクションで実行できるか
	Boolean::Value			isExecutableInsideReadOnlyTransaction() const;
	// 読取専用データベースに対して実行できるか
	Boolean::Value			isExecutableOnReadOnlyDatabase() const;
	// オフラインのデータベースに対して実行できるか
	Boolean::Value			isExecutableOnOfflineDatabase() const;
	// スレーブのデータベースに対して実行できるか
	Boolean::Value			isExecutableOnSlaveDatabase() const;
	// データベース用の論理ログファイルに論理ログを記録するか
	Boolean::Value			isDatabaseLogRecorded() const;
	// システム用の論理ログファイルに論理ログを記録するか
	Boolean::Value			isSystemLogRecorded() const;

	// 分散トランザクションが必要か
	Boolean::Value			isXATransactionNeeded() const;
};

namespace SQLDispatchTable
{
	// 指定された SQL 文に関するエントリを探す
	SYD_SERVER_FUNCTION const SQLDispatchEntry& getEntry(int iType_);
}

//	FUNCTION public
//	Server::SQLDispatchEntry::getModuleType --	どのモジュールで処理するか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Server::Module::Optimizer
//			オプティマイザで処理する
//		Server::Module::Schema
//			スキーマモジュールで処理する
//		Server::Module::Admin
//			運用管理モジュールで処理する
//		Server::Module::Session
//			セッション自体で処理する
//
//	EXCEPTIONS

inline
Module::Type
SQLDispatchEntry::getModuleType() const
{
	return m_eModule;
}

//	FUNCTION public
//	Server::SQLDispatchEntry::getPermissionType --
//		SQL 文を実行すべきトランザクションはどのように開始されたものか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Server::Permission::Never
//			トランザクション中で実行されない
//		Server::Permission::Implicitly
//			暗黙に開始されたトランザクションで実行される
//		Server::Permission::Explicitly
//			明示的に開始されたトランザクションで実行される
//		Server::Permission::Any
//			暗黙にも明示的にも開始されたトランザクションで実行される
//
//	EXCEPTIONS

inline
Permission::Type
SQLDispatchEntry::getPermissionType() const
{
	return m_ePermission;
}

//	FUNCTION public
//	Server::SQLDispatchEntry::isExecutableInsideReadOnlyTransaction --
//		読取専用トランザクションで実行できるか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Boolean::True
//			実行できる
//		Boolean::False
//			実行できない
//
//	EXCEPTIONS

inline
Boolean::Value
SQLDispatchEntry::isExecutableInsideReadOnlyTransaction() const
{
	return _executableInsideReadOnlyTransaction;
}

//	FUNCTION public
//	Server::SQLDispatchEntry::isExecutableOnReadOnlyDatabase --
//		読取専用データベースに対して実行できるか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Boolean::True
//			実行できる
//		Boolean::False
//			実行できない
//		Boolean::Unknown
//			不明である
//
//	EXCEPTIONS

inline
Boolean::Value
SQLDispatchEntry::isExecutableOnReadOnlyDatabase() const
{
	return _executableOnReadOnlyDatabase;
}

//	FUNCTION public
//	Server::SQLDispatchEntry::isExecutableOnOfflineDatabase --
//		オフラインのデータベースに対して実行できるか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Boolean::True
//			実行できる
//		Boolean::False
//			実行できない
//		Boolean::Unknown
//			不明である
//
//	EXCEPTIONS

inline
Boolean::Value
SQLDispatchEntry::isExecutableOnOfflineDatabase() const
{
	return _executableOnOfflineDatabase;
}

//	FUNCTION public
//	Server::SQLDispatchEntry::isExecutableOnSlaveDatabase --
//		スレーブデータベースに対して実行できるか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Boolean::True
//			実行できる
//		Boolean::False
//			実行できない
//		Boolean::Unknown
//			不明である
//
//	EXCEPTIONS

inline
Boolean::Value
SQLDispatchEntry::isExecutableOnSlaveDatabase() const
{
	return _executableOnSlaveDatabase;
}

//	FUNCTION public
//	Server::SQLDispatchEntry::isDatabaseLogRecorded --
//		データベース用の論理ログファイルに対して論理ログを記録するか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Boolean::True
//			記録する
//		Boolean::False
//			記録しない
//
//	EXCEPTIONS

inline
Boolean::Value
SQLDispatchEntry::isDatabaseLogRecorded() const
{
	return _databaseLogRecorded;
}

//	FUNCTION public
//	Server::SQLDispatchEntry::isSystemLogRecorded --
//		システム用の論理ログファイルに対して論理ログを記録するか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Boolean::True
//			記録する
//		Boolean::False
//			記録しない
//
//	EXCEPTIONS

inline
Boolean::Value
SQLDispatchEntry::isSystemLogRecorded() const
{
	return _systemLogRecorded;
}

_SYDNEY_SERVER_END
_SYDNEY_END

#endif //__SYDNEY_SERVER_SQLDISPATCHENTRY_H

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2007, 2012, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
