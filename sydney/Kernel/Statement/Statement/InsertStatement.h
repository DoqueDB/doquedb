// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::InsertStatement -- InsertStatement
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

#ifndef __SYDNEY_STATEMENT_INSERTSTATEMENT_H
#define __SYDNEY_STATEMENT_INSERTSTATEMENT_H

#include "Statement/Object.h"

class ModUnicodeString;

_SYDNEY_BEGIN

namespace Statement
{
	class ColumnNameList;
#ifdef OBSOLETE
	class CursorName;
#endif
	class Identifier;
	class QueryExpression;
	
//
//	CLASS
//		InsertStatement -- InsertStatement
//
//	NOTES
//		InsertStatement
//
class SYD_STATEMENT_FUNCTION InsertStatement : public Statement::Object
{
public:
	//constructor
	InsertStatement()
		: Object(ObjectType::InsertStatement)
	{}
	// コンストラクタ (2)
	InsertStatement(Identifier* pTableName_, QueryExpression* pQueryExpression_, ColumnNameList* pColumnNameList_);
#ifdef OBSOLETE
	// コンストラクタ (3)
	InsertStatement(Identifier* pTableName_, CursorName* pCursorName_, QueryExpression* pQueryExpression_, ColumnNameList* pColumnNameList_);
#endif

	// アクセサ
	// TableName を得る
	Identifier* getTableName() const;
	// TableName を設定する
	void setTableName(Identifier* pTableName_);
	// TableName を ModUnicodeString で得る
	const ModUnicodeString* getTableNameString() const;

#ifdef OBSOLETE
	// CursorName を得る
	CursorName* getCursorName() const;
	// CursorName を設定する
	void setCursorName(CursorName* pCursorName_);
#endif

	// QueryExpression を得る
	QueryExpression* getQueryExpression() const;
	// QueryExpression を設定する
	void setQueryExpression(QueryExpression* pQueryExpression_);

	// ColumnNameList を得る
	ColumnNameList* getColumnNameList() const;
	// ColumnNameList を設定する
	void setColumnNameList(ColumnNameList* pColumnNameList_);

	//自身をコピーする
	Object* copy() const;

	// Analyzerを得る
#ifdef USE_OLDER_VERSION
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

protected:
private:
	// 代入オペレーターは使わない
	InsertStatement& operator=(const InsertStatement& cOther_);

	// メンバ変数
};

} // namescape Statement

_SYDNEY_END

#endif // __SYDNEY_STATEMENT_INSERTSTATEMENT_H

//
// Copyright (c) 1999, 2002, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
