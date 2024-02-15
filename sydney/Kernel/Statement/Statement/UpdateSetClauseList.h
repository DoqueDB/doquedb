// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::UpdateSetClauseList -- UpdateSetClauseList
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

#ifndef __SYDNEY_STATEMENT_UPDATESETCLAUSELIST_H
#define __SYDNEY_STATEMENT_UPDATESETCLAUSELIST_H

#include "Statement/ObjectList.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class UpdateSetClause;
	
//	CLASS
//	UpdateSetClauseList -- UpdateSetClauseList
//
//	NOTES

class SYD_STATEMENT_FUNCTION UpdateSetClauseList : public Statement::ObjectList
{
public:
	//constructor
	UpdateSetClauseList()
		: ObjectList(ObjectType::UpdateSetClauseList)
	{}
	// コンストラクタ (2)
	explicit UpdateSetClauseList(UpdateSetClause* pSetClause_);

	// アクセサ
	// SetClause を得る
	UpdateSetClause* getSetClauseAt(int iAt_) const;

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

private:
	// 代入オペレーターは使わない
	UpdateSetClauseList& operator=(const UpdateSetClauseList& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_UPDATESETCLAUSELIST_H

//
// Copyright (c) 1999, 2002, 2003, 2010, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
