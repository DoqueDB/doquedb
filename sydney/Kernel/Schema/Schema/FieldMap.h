// -*-Mode: C++; tab-width: 4; c-basic-offmap: 4;-*-
// vi:map ts=4 sw=4:
//
// FieldMap.h -- スキーマオブジェクトのマップを表すクラス定義、関数宣言
// 
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_FIELD_MAP_H
#define	__SYDNEY_SCHEMA_FIELD_MAP_H

#include "Schema/ObjectMap.h"
#include "Schema/Field.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

//	CLASS public
//	Schema::FieldMap --
//
//	NOTES

class FieldMap
	: public ObjectMap<Field, FieldPointer>
{
public:
	FieldMap();

	// FieldMapからオブジェクトを得るのに使用する比較関数
	static bool findByCategory(Field* pField_, Field::Category::Value eCategory_);
	static bool findByPosition(Field* pField_, Field::Position iPosition_);
	static bool findBySourceID(Field* pField_, Object::ID::Value iSourceID_);
	static bool findByFunction(Field* pField_, Field::Function::Value eFunction_, Object::ID::Value iColumnID_);
};

//	FUNCTION public
//	Schema::FieldMap::findByCategory -- カテゴリーでオブジェクトを探すための比較関数
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field* pField_
//			比較対象のオブジェクト
//		Field::Category::Value eCategory_
//			条件となる値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
inline bool
FieldMap::
findByCategory(Field* pField_, Field::Category::Value eCategory_)
{
	return (pField_->getCategory() == eCategory_);
}

//	FUNCTION public
//	Schema::FieldMap::findByPosition -- 位置でオブジェクトを探すための比較関数
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field* pField_
//			比較対象のオブジェクト
//		Field::Position iPosition_
//			条件となる値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
inline bool
FieldMap::
findByPosition(Field* pField_, Field::Position iPosition_)
{
	return (pField_->getPosition() == iPosition_);
}

//	FUNCTION public
//	Schema::FieldMap::findBySourceID -- 派生元でオブジェクトを探すための比較関数
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field* pField_
//			比較対象のオブジェクト
//		Schema::Object::ID::Value iSourceID_
//			条件となる値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
inline bool
FieldMap::
findBySourceID(Field* pField_, Object::ID::Value iSourceID_)
{
	return (pField_->getSourceID() == iSourceID_);
}

//	FUNCTION public
//	Schema::FieldMap::findByFunction -- 関数フィールドでオブジェクトを探すための比較関数
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field* pField_
//			比較対象のオブジェクト
//		Schema::Field::Function::Value	function
//			関数を表す列挙子の値
//		Schema::Object::ID::Value iColumnID_
//			関数の引数になる列のID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
inline bool
FieldMap::
findByFunction(Field* pField_, Field::Function::Value eFunction_, Object::ID::Value iColumnID_)
{
	return (pField_->getCategory() == Field::Category::Function
			&& pField_->getFunction() == eFunction_
			&& pField_->getColumnID() == iColumnID_);
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif // __SYDNEY_SCHEMA_FIELD_MAP_H

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
