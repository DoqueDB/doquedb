// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::DerivedColumn -- DerivedColumn
// 
// Copyright (c) 1999, 2002, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_DERIVEDCOLUMN_H
#define __SYDNEY_STATEMENT_DERIVEDCOLUMN_H

#include "Statement/Object.h"

_SYDNEY_BEGIN

namespace Statement
{
	class ColumnName;
	class ValueExpression;
	
//
//	CLASS
//		DerivedColumn -- DerivedColumn
//
//	NOTES
//		DerivedColumn
//
class SYD_STATEMENT_FUNCTION DerivedColumn : public Statement::Object
{
public:
	//constructor
	DerivedColumn()
		: Object(ObjectType::DerivedColumn)
	{}
	// コンストラクタ (2)
	DerivedColumn(ValueExpression* pValuExpression_, ColumnName* pColumnName_);

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

	// アクセサ
	// ValueExpression を得る
	ValueExpression* getValueExpression() const;
	// ValueExpression を設定する
	void setValueExpression(ValueExpression* pValueExpression_);

	// ColumnName を得る
	ColumnName* getColumnName() const;
	// ColumnName を設定する
	void setColumnName(ColumnName* pColumnName_);

	// ExpressionType を得る
	int getExpressionType() const;

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

protected:
private:
	// 代入オペレーターは使わない
	DerivedColumn& operator=(const DerivedColumn& cOther_);

	// メンバ変数
};

} // namescape Statement

_SYDNEY_END

#endif // __SYDNEY_STATEMENT_DERIVEDCOLUMN_H

//
// Copyright (c) 1999, 2002, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
