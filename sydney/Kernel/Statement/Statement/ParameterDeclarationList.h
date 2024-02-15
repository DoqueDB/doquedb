// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::ParameterDeclarationList -- ParameterDeclarationList
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_PARAMETERDECLARATIONLIST_H
#define __SYDNEY_STATEMENT_PARAMETERDECLARATIONLIST_H

#include "Statement/ObjectList.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class ParameterDeclaration;
	
//	CLASS
//	ParameterDeclarationList -- ParameterDeclarationList
//
//	NOTES

class SYD_STATEMENT_FUNCTION ParameterDeclarationList : public Statement::ObjectList
{
public:
	// コンストラクタ (1)
	ParameterDeclarationList();
	// コンストラクタ (2)
	explicit ParameterDeclarationList(ParameterDeclaration* pParameterDeclaration_);

	// アクセサ
	// ParameterDeclaration を得る
	ParameterDeclaration* getParameterDeclarationAt(int iAt_) const;

///////////////////
// ObjectList::
//	Object* getAt(int iAt_) const;
//	void setAt(int iAt_, Object* pObject_);
//	int getCount() const;
//	void append(Object* pObject_);

	//自身をコピーする
	Object* copy() const;

	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

private:
	// 代入オペレーターは使わない
	ParameterDeclarationList& operator=(const ParameterDeclarationList& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_PARAMETERDECLARATIONLIST_H

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
