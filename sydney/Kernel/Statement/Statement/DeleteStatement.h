// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::DeleteStatement -- DeleteStatement
// 
// Copyright (c) 1999, 2002, 2003, 2005, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_DELETESTATEMENT_H
#define __SYDNEY_STATEMENT_DELETESTATEMENT_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

#ifdef OBSOLETE
	class CursorName;
#endif
	class Identifier;
	class ValueExpression;
	
//
//	CLASS
//		DeleteStatement -- DeleteStatement
//
//	NOTES
//		DeleteStatement
//
class SYD_STATEMENT_FUNCTION DeleteStatement : public Statement::Object
{
public:
	//constructor
	DeleteStatement()
		: Object(ObjectType::DeleteStatement)
	{}
	// コンストラクタ (1)
	DeleteStatement(Identifier* pTableName_, ValueExpression* pSearchCondition_);
	// コンストラクタ (2)
	DeleteStatement(Identifier* pTableName_, ValueExpression* pSearchCondition_, Identifier* pCorrelationName_);
#ifdef OBSOLETE
	// コンストラクタ (3)
	DeleteStatement(Identifier* pTableName_, CursorName* pCursorName_);
	// コンストラクタ (4)
	DeleteStatement(Identifier* pTableName_, CursorName* pCursorName_, Identifier* pCorrelationName_);
#endif

	// アクセサ
	// TableName を得る
	Identifier* getTableName() const;
	// TableName を設定する
	void setTableName(Identifier* pTableReference_);

	// SearchCondition を得る
	ValueExpression* getSearchCondition() const;
	// SearchCondition を設定する
	void setSearchCondition(ValueExpression* pSearchCondition_);

	// CorrelationName を得る
	Identifier* getCorrelationName() const;
	// CorrelationName を設定する
	void setCorrelationName(Identifier* pCorrelationName_);

#ifdef OBSOLETE
	// CursorName を得る
	CursorName* getCursorName() const;
	// CursorName を設定する
	void setCursorName(CursorName* pCursorName_);
#endif

	//自身をコピーする
	Object* copy() const;

	// Analyzerを得る
#ifdef USE_OLDER_VERSION
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

private:
	// 代入オペレーターは使わない
	DeleteStatement& operator=(const DeleteStatement& cOther_);
};

//	FUNCTION public
//	Statement::DeleteStatement::copy -- 自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Statement::Object*
//
//	EXCEPTIONS

inline
Object*
DeleteStatement::copy() const
{
	return new DeleteStatement(*this);
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_DELETESTATEMENT_H

//
// Copyright (c) 1999, 2002, 2003, 2005, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
