// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DatabaseDefinition.h --
// 
// Copyright (c) 2000, 2002, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_VARIABLENAME_H
#define __SYDNEY_STATEMENT_VARIABLENAME_H


#include "Common/Common.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN

namespace Statement
{

	class Identifier;


//
//	CLASS
//	Statement::VariableName --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION VariableName  : public Statement::Object
{
public:
	//constructor
	VariableName()
		: Object(ObjectType::VariableName)
	{}
	//コンストラクタ(2)
	VariableName(Identifier* pId_);

	// アクセサ

	// 変数名を取得する
	const ModUnicodeString* getName() const;

	//自身をコピーする
	Object* copy() const;

protected:
private:
	void setIdentifier(Identifier* pIdentifier_);
	// 代入オペレーターは使わない
	VariableName& operator=(const VariableName& cOther_);

};

}

_SYDNEY_END

#endif //__SYDNEY_STATEMENT_DATABASEDEFINITION_H

//
//	Copyright (c) 2000, 2002, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
