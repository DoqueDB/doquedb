// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// StartBackup.h -- バックアップの開始関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ADMIN_STARTBACKUP_H
#define __SYDNEY_ADMIN_STARTBACKUP_H

#include "Admin/Module.h"

#include "Statement/StartBackupStatement.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Database;
}
namespace Server
{
	class Session;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_ADMIN_BEGIN

namespace Utility
{
	class PathList;
}

//	CLASS
//	Admin::StartBackup -- バックアップの開始に関するクラス
//
//	NOTES
//		このクラスは new することはないはずなので、
//		Common::Object の子クラスにしない

class StartBackup
{
public:
	// コンストラクター
    StartBackup(Trans::Transaction& trans, Server::Session* pSession_,
				const ModUnicodeString& dbName);
	// デストラクター
	~StartBackup();

	// 実行する
	SYD_ADMIN_FUNCTION
	void
	execute(const Statement::StartBackupStatement* stmt,
			Utility::PathList& pathList);

private:
	// 初期化する
	void
	initialize(const Statement::StartBackupStatement& stmt);
	// バックアップの開始を表す論理ログを記録する
	void
	writeLog(Schema::Database& database);

	// バックアップを開始するトランザクションのトランザクション記述子
	Trans::Transaction&		_trans;
	// バックアップを開始するセッションのセッション記述子
	Server::Session*		m_pSession;
	// バックアップ対象のデータベース名
	ModUnicodeString		_dbName;
	// どのようなバックアップを行うか
	Statement::StartBackupStatement::BackupType	_type;
	// DISCARD SNAPSHOT が指定されたか
	bool					_discarding;
};

//	FUNCTION public
//	Admin::StartBackup::StartBackup --
//		バックアップの開始を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バックアップを開始するトランザクションのトランザクション記述子
//		ModUnicodeString&	dbName
//			バックアップ対象のデータベースの名前
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
StartBackup::StartBackup(
	Trans::Transaction& trans, Server::Session* pSession_, const ModUnicodeString& dbName)
	: _trans(trans),
	  m_pSession(pSession_),
	  _dbName(dbName),
	  _type(Statement::StartBackupStatement::Unknown),
	  _discarding(false)
{}

//	FUNCTION public
//	Admin::StartBackup::~StartBackup --
//		バックアップの開始を行うクラスのデストラクター
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
StartBackup::~StartBackup()
{}

_SYDNEY_ADMIN_END
_SYDNEY_END

#endif //__SYDNEY_ADMIN_STARTBACKUP_H

//
//	Copyright (c) 2001, 2002, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
