// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::SortSpecification -- SortSpecification
// 
// Copyright (c) 1999, 2002, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_SORTSPECIFICATION_H
#define __SYDNEY_STATEMENT_SORTSPECIFICATION_H

#include "Statement/Object.h"

_SYDNEY_BEGIN

namespace Statement
{
	class ValueExpression;
	
//
//	CLASS
//		SortSpecification -- SortSpecification
//
//	NOTES
//		SortSpecification
//
class SYD_STATEMENT_FUNCTION SortSpecification : public Statement::Object
{
public:
	//constructor
	SortSpecification()
		: Object(ObjectType::SortSpecification)
	{}
	// コンストラクタ (2)
	SortSpecification(ValueExpression* pSortKey_, int iOrderingSpecification_);

	// アクセサ
	// SortKey を得る
	ValueExpression* getSortKey() const;
	// SortKey を設定する
	void setSortKey(ValueExpression* pSortKey_);

	// OrderingSpecification を得る
	int getOrderingSpecification() const;
	// OrderingSpecification を設定する
	void setOrderingSpecification(int iOrderingSpecification_);
	enum OrderingSpecification {
		Ascending = 0,
		Descending
	};
   
	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

protected:
private:
	// 代入オペレーターは使わない
	SortSpecification& operator=(const SortSpecification& cOther_);

	// メンバ変数
};

} // namescape Statement

_SYDNEY_END

#endif // __SYDNEY_STATEMENT_SORTSPECIFICATION_H

//
// Copyright (c) 1999, 2002, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
