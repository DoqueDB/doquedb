// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::GroupByClause -- GroupByClause
// 
// Copyright (c) 1999, 2002, 2003, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_GROUPBYCLAUSE_H
#define __SYDNEY_STATEMENT_GROUPBYCLAUSE_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class GroupingElementList;
	
//	CLASS
//	GroupByClause -- GroupByClause
//
//	NOTES

class SYD_STATEMENT_FUNCTION GroupByClause : public Statement::Object
{
public:
	//constructor
	GroupByClause()
		: Object(ObjectType::GroupByClause)
	{}
	// コンストラクタ (2)
	explicit GroupByClause(GroupingElementList* pGroupingElementList_);

	// アクセサ
	// GroupingElementList を得る
	GroupingElementList* getGroupingElementList() const;
	// GroupingElementList を設定する
	void setGroupingElementList(GroupingElementList* pGroupingElementList_);

	void setHasHavingClause(bool bHaving_) { m_bHavingBy = bHaving_;}

	bool hasHavingClause() {return m_bHavingBy;}

	//自身をコピーする
	Object* copy() const;

	// Analyzerを得る
#ifdef USE_OLDER_VERSION
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

private:
	// 代入オペレーターは使わない
	GroupByClause& operator=(const GroupByClause& cOther_);
	bool m_bHavingBy;

	// メンバ変数
};

//	FUNCTION public
//	Statement::GroupByClause::copy -- 自身をコピーする
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
GroupByClause::copy() const
{
	return new GroupByClause(*this);
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_GROUPBYCLAUSE_H

//
// Copyright (c) 1999, 2002, 2003, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
