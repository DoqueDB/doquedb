// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::ValueExpressionList -- ValueExpressionList
// 
// Copyright (c) 1999-2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_VALUEEXPRESSIONLIST_H
#define __SYDNEY_STATEMENT_VALUEEXPRESSIONLIST_H

#include "Statement/ObjectList.h"

_SYDNEY_BEGIN

namespace Statement
{
	class ValueExpression;
	
//
//	CLASS
//		ValueExpressionList -- ValueExpressionList
//
//	NOTES
//		ValueExpressionList
//
class SYD_STATEMENT_FUNCTION ValueExpressionList : public Statement::ObjectList
{
public:
	// コンストラクタ (1)
	ValueExpressionList();
	// コンストラクタ (2)
	explicit ValueExpressionList(ValueExpression* pValueExpression_);
	// コピーコンストラクタ
	explicit ValueExpressionList(const ValueExpressionList& cOther_);

	// ExpressionType を設定する
	void setExpressionType(int iExpressionType_);

	// アクセサ
	// ExpressionType を得る
	int getExpressionType() const;

	// ValueExpression を得る
	ValueExpression* getValueExpressionAt(int iAt_) const;

	// ValueExpression をマージする(下位ノードの所有権は移る)
	void merge(ValueExpressionList& cValueExpressionList_);

	// ValueExpressionを先頭に加える
	void insertValueExpression(ValueExpression* pValueExpression_);
	// ValueExpressionを加える
	void appendValueExpression(ValueExpression* pValueExpression_);

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
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

protected:
private:
	// 代入オペレーターは使わない
	ValueExpressionList& operator=(const ValueExpressionList& cOther_);

	// メンバ変数
	int m_iExpressionType;
};

} // namescape Statement

_SYDNEY_END

#endif // __SYDNEY_STATEMENT_VALUEEXPRESSIONLIST_H

//
// Copyright (c) 1999-2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
