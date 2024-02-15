// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::DatabaseCreateOptionList -- DatabaseCreateOptionList
// 
// Copyright (c) 1999, 2002, 2003, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_DATABASECREATEOPTIONLIST_H
#define __SYDNEY_STATEMENT_DATABASECREATEOPTIONLIST_H

#include "Statement/ObjectList.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class DatabaseCreateOption;
class SQLParser;

//	CLASS
//	DatabaseCreateOptionList -- DatabaseCreateOptionList
//
//	NOTES

class DatabaseCreateOptionList : public Statement::ObjectList
{
public:
	//constructor
	DatabaseCreateOptionList()
		: ObjectList(ObjectType::DatabaseCreateOptionList)
	{}
	// コンストラクタ (2)
	SYD_STATEMENT_FUNCTION
	explicit DatabaseCreateOptionList(Statement::DatabaseCreateOption* pCreateOption_);

	// アクセサ
	// CreateOption を得る
	Statement::DatabaseCreateOption* getCreateOptionAt(int iAt_) const;

///////////////////
// ObjectList::
//	Object* getAt(int iAt_) const;
//	void setAt(int iAt_, Object* pObject_);
//	int getCount() const;
//	void append(Object* pObject_);

	// CreateOption 間の整合性を確認する
	void assureCreateOptions(SQLParser& cParser_)
		{ DatabaseCreateOptionList::assureElements(cParser_); }

	//override
	// 要素間の整合性を確認する
	SYD_STATEMENT_FUNCTION
	virtual void assureElements(SQLParser& cParser_);

	//自身をコピーする
	SYD_STATEMENT_FUNCTION
	Object* copy() const;

	//SQL文で得る
	ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

private:
	// 代入オペレーターは使わない
	DatabaseCreateOptionList& operator=(const DatabaseCreateOptionList& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_DATABASECREATEOPTIONLIST_H

//
// Copyright (c) 1999, 2002, 2003, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
