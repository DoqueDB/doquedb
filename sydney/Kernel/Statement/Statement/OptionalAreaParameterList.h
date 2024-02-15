// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::OptionalAreaParameterList -- OptionalAreaParameterList
// 
// Copyright (c) 1999, 2002, 2003, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef _SYDNEY_STATEMENT_OPTIONALAREAPARAMETERLIST_H
#define _SYDNEY_STATEMENT_OPTIONALAREAPARAMETERLIST_H

#include "Statement/ObjectList.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class AlterAreaStatement;
	class DropAreaStatement;

//	CLASS
//	OptionalAreaParameterList -- OptionalAreaParameterList
//
//	NOTES

class OptionalAreaParameterList : public Statement::ObjectList
{
public:
	//constructor
	OptionalAreaParameterList()
		: ObjectList(ObjectType::OptionalAreaParameterList)
	{}
	// コンストラクタ (2)
	SYD_STATEMENT_FUNCTION
	explicit OptionalAreaParameterList(Statement::AlterAreaStatement* pOptionalAreaParameter_);
	// コンストラクタ (2)
	SYD_STATEMENT_FUNCTION
	explicit OptionalAreaParameterList(Statement::DropAreaStatement* pOptionalAreaParameter_);

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

private:
	// 代入オペレーターは使わない
	SYD_STATEMENT_FUNCTION
	OptionalAreaParameterList& operator=(const OptionalAreaParameterList& cOther_);
};

//	FUNCTION public
//	Statement::OptionalAreaParameterList::copy -- 自身をコピーする
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
OptionalAreaParameterList::copy() const
{
	return new OptionalAreaParameterList(*this);
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // _SYDNEY_STATEMENT_OPTIONALAREAPARAMETERLIST_H

//
// Copyright (c) 1999, 2002, 2003, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
