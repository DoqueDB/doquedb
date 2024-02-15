// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::GroupingColumnReferenceList -- GroupingColumnReferenceList
// 
// Copyright (c) 1999, 2002, 2003, 2010, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_GROUPINGCOLUMNREFERENCELIST_H
#define __SYDNEY_STATEMENT_GROUPINGCOLUMNREFERENCELIST_H

#include "Statement/ObjectList.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class GroupingColumnReference;
	
//	CLASS
//	GroupingColumnReferenceList -- GroupingColumnReferenceList
//
//	NOTES

class SYD_STATEMENT_FUNCTION GroupingColumnReferenceList : public Statement::ObjectList
{
public:
	// コンストラクタ (2)
	explicit GroupingColumnReferenceList(GroupingColumnReference* pGroupingColumnReference_ = 0);

///////////////////
// ObjectList::
//	Object* getAt(int iAt_) const;
//	void setAt(int iAt_, Object* pObject_);
//	int getCount() const;
//	void append(Object* pObject_);

	//自身をコピーする
	Object* copy() const;

	// Analyzerを得る
#ifdef USE_OLDER_VERSION
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

private:
	// 代入オペレーターは使わない
	GroupingColumnReferenceList& operator=(const GroupingColumnReferenceList& cOther_);

	// メンバ変数
};

//	FUNCTION public
//	Statement::GroupingColumnReferenceList::copy -- 自身をコピーする
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
GroupingColumnReferenceList::copy() const
{
	return new GroupingColumnReferenceList(*this);
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_GROUPINGCOLUMNREFERENCELIST_H

//
// Copyright (c) 1999, 2002, 2003, 2010, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
