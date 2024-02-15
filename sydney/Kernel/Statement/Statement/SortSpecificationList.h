// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::SortSpecificationList -- SortSpecificationList
// 
// Copyright (c) 1999, 2002, 2003, 2008, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_SORTSPECIFICATIONLIST_H
#define __SYDNEY_STATEMENT_SORTSPECIFICATIONLIST_H

#include "Statement/ObjectList.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class SortSpecification;
class ValueExpression;
	
//
//	CLASS
//		SortSpecificationList -- SortSpecificationList
//
//	NOTES
//		SortSpecificationList
//
class SYD_STATEMENT_FUNCTION SortSpecificationList : public Statement::ObjectList
{
public:
	//constructor
	SortSpecificationList()
		: ObjectList(ObjectType::SortSpecificationList)
	{}
	// コンストラクタ (2)
	explicit SortSpecificationList(SortSpecification* pSortSpecification_);
	// コピーコンストラクタ
	SortSpecificationList(const SortSpecificationList& cOther_);

	// デストラクタ
	~SortSpecificationList();

	// アクセサ
	// SortSpecification を得る
	SortSpecification* getSortSpecificationAt(int iAt_) const;

	// PartitionByを得る
	ValueExpression* getPartitionBy() const;
	// PartitionByをセットする
	void setPartitionBy(ValueExpression* pPartitionBy_);

///////////////////
// ObjectList::
//	Object* getAt(int iAt_) const;
//	void setAt(int iAt_, Object* pObject_);
//	int getCount() const;
//	void append(Object* pObject_);

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

private:
	// 代入オペレーターは使わない
	SortSpecificationList& operator=(const SortSpecificationList& cOther_);

	// メンバ変数
	ValueExpression* m_pPartitionBy;
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_SORTSPECIFICATIONLIST_H

//
// Copyright (c) 1999, 2002, 2003, 2008, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
