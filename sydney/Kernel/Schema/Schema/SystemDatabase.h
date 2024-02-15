// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemDatabase.h -- メタデータベース関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_SYSTEMDATABASE_H
#define	__SYDNEY_SCHEMA_SYSTEMDATABASE_H

#include "Schema/Database.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

//	ENUM
//	Schema::SystemDatabase -- メタデータベースを表すクラス
//
//	NOTES

class SystemDatabase
	: public	Database
{
public:
	static SystemDatabase*	getInstance(Trans::Transaction& cTrans_);
												// メタデータベースを表すオブジェクトを得る
	static void				terminate();		// メタデータベースの終了処理
	~SystemDatabase();							// デストラクター

	static Table*			createSystemTable(Trans::Transaction& cTrans_,
											  Database& cDatabase_,
											  Object::Category::Value eCategory_,
											  Object::ID::Value& iObjectID_);
												// システム表を表すオブジェクトを作る

private:
	SystemDatabase();							// コンストラクター
	void					destruct();			// デストラクター下位関数
	void					create(Trans::Transaction& cTrans_);
												// 表などのオブジェクトを作る
};

//	FUNCTION public
//	Schema::SystemDatabase::~SystemDatabase -- データベースを表すクラスのデストラクター
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
SystemDatabase::
~SystemDatabase()
{
	destruct();
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_SYSTEMDATABASE_H

//
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
