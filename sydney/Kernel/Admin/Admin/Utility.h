// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Utility.h -- ユーティリティクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2004, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ADMIN_UTILITY_H
#define __SYDNEY_ADMIN_UTILITY_H

#include "Admin/Module.h"

#include "Common/AutoCaller.h"
#include "Os/Path.h"
#include "Schema/Database.h"

class ModUnicodeString;

_SYDNEY_BEGIN

namespace Communication
{
	class Connection;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_ADMIN_BEGIN

namespace Verification
{
	class Progress;
}

_SYDNEY_ADMIN_UTILITY_BEGIN

class PathList;

// Database で使用しているパスリストを作成する
void
makePathList(Trans::Transaction& cTrans_,
			 const Schema::Database& cDatabase_, 
			 Utility::PathList& cPathList_);

// コネクションに戻り値を設定する
void
setResult(Communication::Connection& cConnection_,
		  const Utility::PathList& cPathList);
void
setResult(Communication::Connection& connection,
		  const Verification::Progress& result);

namespace Algorithm
{
	//	CLASS InstanceLess
	//		-- ポインタ先の実体を比較するテンプレート関数
	//	NOTES

	template < class ValueType >
	class InstanceLess : public ModBinaryFunction< ValueType, ValueType, ModBoolean >
	{
	public:
		ModBoolean operator()(const ValueType& value1, const ValueType& value2) const
		{	return (*value1 < *value2) ? ModTrue : ModFalse; }
	};
}

//	NAMESPACE
// 	Admin::Utility::Meta -- ログデータの分類
//
//	NOTES

namespace Meta
{
	//	ENUM
	//	Admin::Utility::Meta::Mount -- mount ログに書き込むログ種別
	//
	//	NOTES

   	namespace Mount
	{
		enum Type
		{
			//	Schema::Database::makeLogData により以下の情報が記録される
			//
			//	0: データベース名
			//	1: オブジェクトID
			//	2: パスリスト
			//	3: 属性

			// マウントオプション
			Option = Schema::Database::Log::Create::Num,
			// アンマウントされたデータベースをマウントするか
			Unmounted,
			AreaInfo,		// Area定義変更情報
			TableInfo,		// Table変更情報
			IndexInfo,		// Index変更情報
			MemberNum,
			MasterURL = MemberNum,		// Replication master URL
			MemberNum2
		};
	}

	//	ENUM
	//	Admin::Utility::Meta::Unmount -- unmount ログに書き込むログ種別
	// 
	//	NOTES

	namespace Unmount
	{
		enum Type
		{
			//	Schema::Database::makeLogData により以下の情報が記録される
			//
			//	0: データベース名
			//	1: オブジェクトID
			//	2: パスリスト

			// 前回のチェックポイント時タイムスタンプ
			MostRecent = Schema::Database::Log::Create::Num,
			// 前々回のチェックポイント時タイムスタンプ
			SecondMostRecent,
			// 種類数
			MemberNum
		};
	}
}

// バックアップ関連
namespace Backup
{
	// トランザクションがあるデータベースの
	// バックアップを開始したことを記憶する
	bool
	enter(const ModUnicodeString& dbName, const Trans::Transaction& trans);
#ifdef OBSOLETE
	// あるデータベースのバックアップが開始されているか調べる
	bool
	isEntered(const ModUnicodeString& dbName);
#endif
	// トランザクションがあるデータベースの
	// バックアップを開始しているか調べる
	bool
	isEntered(const ModUnicodeString& dbName, const Trans::Transaction& trans);

	// トランザクションがあるデータベースの
	// バックアップを開始したことを忘れる
	void
	leave(const ModUnicodeString& dbName, const Trans::Transaction& trans);

	// あるトランザクションがバックアップ中のデータベースの名前を得る
	void
	getEnteredDatabaseName(const Trans::Transaction& trans,
						   ModVector<ModUnicodeString>& dbNames);
}

//	CLASS
//	Admin::Utility::PathList
//
//	NOTES

class PathList : public ModVector< Os::Path* >
{
public:
	PathList(){}
	PathList(ModSize n_, Os::Path* pPath_ = 0);
	virtual ~PathList();
											// コンストラクタ、デストラクタ

	// 重複したものが登録されていなければ、登録する
	void
	addUnique(const Os::Path& v);
};

//	CLASS
//	Admin::Utility::AutoDatabaseCloser --
//
//	NOTES

class AutoDatabaseCloser
	: public Common::AutoCaller1<Schema::Database, bool>
{
public:
	// コンストラクター
	AutoDatabaseCloser(Schema::Database& database, bool volatileCache = true)
		: Common::AutoCaller1<Schema::Database, bool>(
			&database, &Schema::Database::close, volatileCache)
	{
		database.open();
	}
};

//	FUNCTION public
//	Admin::Utility::PathList::PathList
//		-- PathList のコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		ModSize n
//			予約領域サイズ
//		Os::Path* pList
//			初期化値 (Default: 0)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
PathList::PathList(ModSize n_, Os::Path* pPath_)
	: ModVector< Os::Path* >(n_, pPath_)
{}

//	FUNCTION public
//	Admin::Utility::PathList::~PathList
//		-- PathList のデストラクタ
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
PathList::~PathList()
{
	Iterator it = begin();
	Iterator iEnd = end();
	for ( ; it != iEnd; it++ )
		delete (*it);
}

_SYDNEY_ADMIN_UTILITY_END
_SYDNEY_ADMIN_END
_SYDNEY_END

#endif //__SYDNEY_ADMIN_UTILITY_H

//
//	Copyright (c) 2001, 2002, 2004, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
