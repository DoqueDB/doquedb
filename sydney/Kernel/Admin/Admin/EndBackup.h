// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// EndBackup.h -- バックアップの終了関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2007, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ADMIN_ENDBACKUP_H
#define __SYDNEY_ADMIN_ENDBACKUP_H

#include "Admin/Module.h"
#include "Trans/LogFile.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN

namespace Statement
{
	class EndBackupStatement;
}
namespace Server
{
	class Session;
}
namespace Trans
{
	class Transaction;
}
namespace Schema
{
	class Database;
}

_SYDNEY_ADMIN_BEGIN

//	CLASS
//	Admin::EndBackup -- バックアップの終了に関するクラス
//
//	NOTES
//		このクラスは new することはないはずなので、
//		Common::Object の子クラスにしない

class EndBackup
{
public:
	// コンストラクター
	EndBackup(Trans::Transaction& cTrans_, Server::Session* pSession_,
			  const ModUnicodeString& cstrDatabaseName_);
	// デストラクター
	~EndBackup();

	// 実行する
	SYD_ADMIN_FUNCTION
	void
	execute(const Statement::EndBackupStatement* stmt = 0);

	SYD_ADMIN_FUNCTION
	static void				endAll(Trans::Transaction& cTrans_);
												// 全て EndBackup する
private:
	// 初期化する
	void
	initialize(const Statement::EndBackupStatement& stmt);
	
	Trans::Transaction&		m_cTrans;			// トランザクション記述子
	Server::Session*		m_pSession;			// セッション記述子
	ModUnicodeString		m_cstrDatabaseName;	// 対象のデータベース名

	// DISCARD LOGICALLOG が指定されたか
	bool					m_bDiscarding;
};

//	FUNCTION public
//	Admin::EndBackup::EndBackup --
//		バックアップの終了を表すクラスのコンストラクター
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
EndBackup::EndBackup(
	Trans::Transaction& cTrans_, Server::Session* pSession_,
			  const ModUnicodeString& cstrDatabaseName_)
	: m_cTrans(cTrans_),
	  m_pSession(pSession_),
	  m_cstrDatabaseName(cstrDatabaseName_)
{}

//	FUNCTION public
//	Admin::EndBackup::~EndBackup --
//		バックアップの終了を行うクラスのデストラクター
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

inline
EndBackup::~EndBackup()
{}

_SYDNEY_ADMIN_END
_SYDNEY_END

#endif //__SYDNEY_ADMIN_ENDBACKUP_H

//
//	Copyright (c) 2001, 2002, 2007, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
