// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::UpdateSetClause -- UpdateSetClause
// 
// Copyright (c) 1999, 2002, 2010, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_UPDATESETCLAUSE_H
#define __SYDNEY_STATEMENT_UPDATESETCLAUSE_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class ColumnName;
class ColumnNameList;
class ValueExpression;
	
//
//	CLASS
//		UpdateSetClause -- UpdateSetClause
//
//	NOTES
//		UpdateSetClause
//
class SYD_STATEMENT_FUNCTION UpdateSetClause : public Statement::Object
{
public:
	//constructor
	UpdateSetClause()
		: Object(ObjectType::UpdateSetClause)
	{}
	// コンストラクタ (1)
	UpdateSetClause(ColumnName* pColumnName_, ValueExpression* pValue_);
	// コンストラクタ (2)
	UpdateSetClause(ColumnNameList* pColumnNameList_, ValueExpression* pRowValue_);

	// アクセサ
	// ColumnName を得る
	ColumnName* getColumnName() const;
	// ColumnName を設定する
	void setColumnName(ColumnName* pColumnName_);

	// ColumnNameList を得る
	ColumnNameList* getColumnNameList() const;
	// ColumnNameList を設定する
	void setColumnNameList(ColumnNameList* pColumnNameList_);

	// UpdateSource を得る
	ValueExpression* getSource() const;
	// UpdateSource を設定する
	void setSource(ValueExpression* pSource_);

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
	UpdateSetClause& operator=(const UpdateSetClause& cOther_);

	// メンバ変数
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_UPDATESETCLAUSE_H

//
// Copyright (c) 1999, 2002, 2010, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
