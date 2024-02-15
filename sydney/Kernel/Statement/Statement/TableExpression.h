// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::TableExpression -- TableExpression
// 
// Copyright (c) 1999, 2002, 2008, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_TABLEEXPRESSION_H
#define __SYDNEY_STATEMENT_TABLEEXPRESSION_H

#include "Statement/Object.h"

_SYDNEY_BEGIN

namespace Statement
{
	class GroupByClause;
	class HavingClause;
	class TableReferenceList;
	class ValueExpression;
	
//
//	CLASS
//		TableExpression -- TableExpression
//
//	NOTES
//		TableExpression
//
class SYD_STATEMENT_FUNCTION TableExpression : public Statement::Object
{
public:
	//constructor
	TableExpression()
		: Object(ObjectType::TableExpression)
	{}
	// コンストラクタ (2)
	TableExpression(TableReferenceList* pFromClause_,
					ValueExpression* pWhereClause_,
					GroupByClause* pGroupByClause_,
					HavingClause* pHavingClause_);

	// アクセサ
	// FromClause を得る
	TableReferenceList* getFromClause() const;
	// FromClause を設定する
	void setFromClause(TableReferenceList* pFromClause_);

	// WhereClause を得る
	ValueExpression* getWhereClause() const;
	// WhereClause を設定する
	void setWhereClause(ValueExpression* pWhereClause_);

	// GroupByClause を得る
	GroupByClause* getGroupByClause() const;
	// GroupByClause を設定する
	void setGroupByClause(GroupByClause* pGroupByClause_);

	// HavingClause を得る
	HavingClause* getHavingClause() const;
	// HavingClause を設定する
	void setHavingClause(HavingClause* pHavingClause_);

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
	TableExpression& operator=(const TableExpression& cOther_);

	// メンバ変数
};

} // namescape Statement

_SYDNEY_END

#endif // __SYDNEY_STATEMENT_TABLEEXPRESSION_H

//
// Copyright (c) 1999, 2002, 2008, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
