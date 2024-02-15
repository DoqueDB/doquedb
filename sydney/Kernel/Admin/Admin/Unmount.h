// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Unmount.h -- データベースのアンマウント関連のクラス定義、関数宣言
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

#ifndef __SYDNEY_ADMIN_UNMOUNT_H
#define __SYDNEY_ADMIN_UNMOUNT_H

#include "Admin/Module.h"
#include "Admin/Utility.h"

#include "ModUnicodeString.h"

template <class T>
class ModVector;

_SYDNEY_BEGIN

namespace Os
{
	class Path;
}
namespace Schema
{
	class Database;
	class LogData;
}
namespace Server
{
	class Session;
}
namespace Statement
{
	class Object;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_ADMIN_BEGIN

//	CLASS
//	Admin::Unmount -- データベースのアンマウント処理を行うクラス
//
//	NOTES
//		このクラスは new することはないはずなので、
//		Common::Object の子クラスにしない

class Unmount
{
public:
	// コンストラクター
	Unmount(Trans::Transaction& cTrans_, Server::Session* pSession_, const ModUnicodeString& strDbName_);
#ifdef OBSOLETE
	SYD_ADMIN_FUNCTION
	Unmount(Trans::Transaction& cTrans_, const Schema::LogData& cLog_);
#endif
	// デストラクター
	~Unmount();

	SYD_ADMIN_FUNCTION
	void					execute(const Statement::Object* pcStatement_,
									Utility::PathList& cPathList_);
												// UNMOUNT する
private:
	// ログデータを作成する
	void
	makeLogData(Schema::LogData& logData, const Schema::Database& database);
	// ログデータのある種別の情報を作成する
	Common::Data::Pointer
	packMetaField(Utility::Meta::Unmount::Type type) const;
#ifdef OBSOLETE
	// ログデータのある種別の情報の値を得る
	bool
	unpackMetaField(const Common::Data* data,
					Utility::Meta::Unmount::Type type);
	// ログデータのある種別の情報の型を得る
	static Common::DataType::Type
	getMetaFieldType(Utility::Meta::Unmount::Type type);

	// Database が使用するパスを取得する
	const ModVector<Os::Path*>&
	getPath() const;
#endif
	Trans::Transaction&		m_cTrans;			// トランザクション記述子
	Server::Session*		m_pSession;			// セッション記述子
	ModUnicodeString		_dbName;			// 操作対象のデータベース名

	Utility::PathList		m_cPathList;		// データ保存パスのリスト
};

//	FUNCTION public
//	Admin::Unmount::Unmount -- アンマウントを表すクラスのコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transction&	cTrans_
//			アンマウントを行うトランザクションのトランザクション記述子
//		ModUnicodeString&	dbName
//			アンマウントするデータベースの名前
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
Unmount::Unmount(
	Trans::Transaction& cTrans_, Server::Session* pSession_, const ModUnicodeString& dbName)
	: m_cTrans(cTrans_),
	  m_pSession(pSession_),
	  _dbName(dbName)
{}

//	FUNCTION public
//	Admin::Unmount::~Unmount -- アンマウントを表すクラスのデストラクタ
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
Unmount::~Unmount()
{}

_SYDNEY_ADMIN_END
_SYDNEY_END

#endif // __SYDNEY_ADMIN_UNMOUNT_H

//
//	Copyright (c) 2001, 2002, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
