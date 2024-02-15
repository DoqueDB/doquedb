// -*-Mode: C++; tab-width: 4; c-basic-offmap: 4;-*-
// vi:map ts=4 sw=4:
//
// ConstraintMap.h -- スキーマオブジェクトのマップを表すクラス定義、関数宣言
// 
// Copyright (c) 2002, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_CONSTRAINT_MAP_H
#define	__SYDNEY_SCHEMA_CONSTRAINT_MAP_H

#include "Schema/ObjectMap.h"
#include "Schema/Constraint.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

//	CLASS public
//	Schema::ConstraintMap --
//
//	NOTES

class ConstraintMap
	: public ObjectMap<Constraint, ConstraintPointer>
{
public:
	ConstraintMap();

	// ConstraintMapからオブジェクトを得るのに使用する比較関数
	static bool findByCategory(Constraint* pConstraint_, Constraint::Category::Value eCategory_);
#ifdef OBSOLETE // Constraintを外部が参照することはないので以下の関数も使用されない
	static bool findByPosition(Constraint* pConstraint_, Constraint::Position iPosition_);
#endif
};

//	FUNCTION public
//	Schema::ConstraintMap::findByCategory -- カテゴリーでオブジェクトを探すための比較関数
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Constraint* pConstraint_
//			比較対象のオブジェクト
//		Constraint::Category::Value eCategory_
//			条件となる値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
inline bool
ConstraintMap::
findByCategory(Constraint* pConstraint_, Constraint::Category::Value eCategory_)
{
	return (pConstraint_->getCategory() == eCategory_);
}

#ifdef OBSOLETE // Constraintを外部が参照することはないので以下の関数も使用されない
//	FUNCTION public
//	Schema::ConstraintMap::findByPosition -- 位置でオブジェクトを探すための比較関数
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Constraint* pConstraint_
//			比較対象のオブジェクト
//		Constraint::Position iPosition_
//			条件となる値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
inline bool
ConstraintMap::
findByPosition(Constraint* pConstraint_, Constraint::Position iPosition_)
{
	return (pConstraint_->getPosition() == iPosition_);
}
#endif

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif // __SYDNEY_SCHEMA_CONSTRAINT_MAP_H

//
// Copyright (c) 2002, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
