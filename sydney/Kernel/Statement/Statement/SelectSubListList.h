// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::SelectSubListList -- SelectSubListList
// 
// Copyright (c) 1999, 2002, 2003, 2010, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_SELECTSUBLISTLIST_H
#define __SYDNEY_STATEMENT_SELECTSUBLISTLIST_H

#include "Statement/ObjectList.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class SelectSubList;
	
//
//	CLASS
//		SelectSubListList -- SelectSubListList
//
//	NOTES
//		SelectSubListList
//
class SYD_STATEMENT_FUNCTION SelectSubListList : public Statement::ObjectList
{
public:
	//constructor
	SelectSubListList()
		: ObjectList(ObjectType::SelectSubListList)
	{}
	// コンストラクタ (2)
	explicit SelectSubListList(SelectSubList* pSelectSubList_);
	explicit SelectSubListList(const SelectSubListList& cOther_)
		: ObjectList(cOther_),
		  m_iExpressionType(cOther_.m_iExpressionType)
	{}

	// アクセサ
	// SelectSubList を得る
	SelectSubList* getSelectSubListAt(int iAt_) const;

	// ExpressionTypeを得る
	int getExpressionType() const;
	// ExpressionTypeを合成する
	void mergeExpressionType(int iType_);

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
	SelectSubListList& operator=(const SelectSubListList& cOther_);

	// メンバ変数
	int m_iExpressionType;
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_SELECTSUBLISTLIST_H

//
// Copyright (c) 1999, 2002, 2003, 2010, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
