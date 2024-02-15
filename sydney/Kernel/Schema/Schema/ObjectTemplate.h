// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectTemplate.h -- スキーマオブジェクト関連のテンプレート定義
// 
// Copyright (c) 2002, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_OBJECTTEMPLATE_H
#define	__SYDNEY_SCHEMA_OBJECTTEMPLATE_H

#include "Schema/Module.h"
#include "Schema/Database.h"
#include "Schema/Manager.h"
#include "Trans/Transaction.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

namespace ObjectTemplate
{
	//	TEMPLATE FUNCTION
	//	Schema::ObjectTemplate::get -- データベースからスキーマオブジェクトを取得する
	//
	//	NOTES
	//
	//	TEMPLATE ARGUMENTS
	//		class _Object_
	//			取得するオブジェクト
	//		class _SystemTable_
	//			取得するオブジェクトを保持するシステム表
	//		Schema::Object::Category::Value _category_
	//			取得するオブジェクトの種別
	//
	//	ARGUMENTS
	//		Schema::Object::ID::Value iID_
	//			取得したいスキーマオブジェクトのID
	//		Schema::Database* pDatabase_
	//			オブジェクトが属するデータベース
	//			0が指定されるとすべてのデータベースを調べる
	//		Trans::Transaction& cTrans_
	//			操作を行うトランザクション記述子
	//
	//	RETURN
	//		オブジェクト
	//
	//	EXCEPTIONS

	template <class _Object_, class _SystemTable_, Object::Category::Value _category_>
	_Object_*
	get(Object::ID::Value iID_, Database* pDatabase_, Trans::Transaction& cTrans_)
	{
		if (iID_ == Object::ID::Invalid)
			return 0;

		_Object_* object = 0;

		// まず、与えられたスキーマオブジェクト ID の
		// オブジェクトを表すクラスがデータベースに登録されていないか調べる
		// データベースのIDが指定されていないときはまず一時データベースを調べる

		Database* pDatabase =
			(pDatabase_) ? pDatabase_
			: Database::getTemporary(cTrans_);

		if (pDatabase)
			object =
				_SYDNEY_DYNAMIC_CAST(_Object_*,
									 pDatabase->getCache(iID_, _category_));

		if (!object) {

			// 見つからなかった場合、
			// 指定されたデータベースについてシステム表を検索する
			// データベースの指定が0ならすべての
			// 永続データベースについてシステム表を検索する

			if (pDatabase_) {
				// IDをキーにしてシステム表を検索して得る
				// ★注意★
				// このテンプレート引数の使い方が常にできるのか疑問
				object = _SystemTable_(*pDatabase_).load(cTrans_, iID_);
			}
		}

		return object;
	}
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_OBJECTTEMPLATE_H

//
// Copyright (c) 2002, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
