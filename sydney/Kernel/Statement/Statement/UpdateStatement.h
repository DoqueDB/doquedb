// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::UpdateStatement -- UpdateStatement
// 
// Copyright (c) 1999, 2002, 2003, 2005, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_UPDATESTATEMENT_H
#define __SYDNEY_STATEMENT_UPDATESTATEMENT_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class Identifier;
	class UpdateSetClauseList;
	class ValueExpression;
	
//	CLASS
//	UpdateStatement -- UpdateStatement
//
//	NOTES

class SYD_STATEMENT_FUNCTION UpdateStatement : public Statement::Object
{
public:
	//constructor
	UpdateStatement()
		: Object(ObjectType::UpdateStatement)
	{}
	// コンストラクタ (1)
	UpdateStatement(Identifier* pTargetTable_,
					UpdateSetClauseList* pSetClauseList_,
					ValueExpression* pSearchCondition_);
	// コンストラクタ (2)
	UpdateStatement(Identifier* pTargetTable_,
					UpdateSetClauseList* pSetClauseList_,
					ValueExpression* pSearchCondition_,
					Identifier* pCorrelationName_);
#ifdef OBSOLETE
	// コンストラクタ (3)
	UpdateStatement(Identifier* pTargetTable_,
					UpdateSetClauseList* pSetClauseList_,
					Identifier* pCursorName_);
	// コンストラクタ (4)
	UpdateStatement(Identifier* pTargetTable_,
					UpdateSetClauseList* pSetClauseList_,
					Identifier* pCursorName_,
					Identifier* pCorrelationName_);
#endif

	// アクセサ
	// TargetTable を得る
	Identifier* getTargetTable() const;
	// TargetTable を設定する
	void setTargetTable(Identifier* pTargetTable_);

	// SearchCondition を得る
	ValueExpression* getSearchCondition() const;
	// SearchCondition を設定する
	void setSearchCondition(ValueExpression* pSearchCondition_);

	// SetClauseList を得る
	UpdateSetClauseList* getSetClauseList() const;
	// SetClauseList を設定する
	void setSetClauseList(UpdateSetClauseList* pSetClauseList_);

	// CorrelationName を得る
	Identifier* getCorrelationName() const;
	// CorrelationName を設定する
	void setCorrelationName(Identifier* pCorrelationName_);

#ifdef OBSOLETE
	// CursorName を得る
	Identifier* getCursorName() const;
	// CursorName を設定する
	void setCursorName(Identifier* pCursorName_);
	// CursorName を ModUnicodeString で得る
	const ModUnicodeString* getCursorNameString() const;
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
	UpdateStatement& operator=(const UpdateStatement& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_UPDATESTATEMENT_H

//
// Copyright (c) 1999, 2002, 2003, 2005, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
