// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FunctionDefinition.h --
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

#ifndef __SYDNEY_STATEMENT_FUNCTIONDEFINITION_H
#define __SYDNEY_STATEMENT_FUNCTIONDEFINITION_H

#include "Common/Common.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Identifier;
class ParameterDeclarationList;
class ReturnsClause;
class RoutineBody;

//
//	CLASS
//	Statement::FunctionDefinition --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION FunctionDefinition  : public Statement::Object
{
public:
	//constructor
	FunctionDefinition()
		: Object(ObjectType::FunctionDefinition)
	{}
	//コンストラクタ
	FunctionDefinition(Identifier* pId_,
					   ParameterDeclarationList* pParam_,
					   ReturnsClause* pReturns_,
					   RoutineBody* pRoutine_);
	//デストラクタ
	virtual ~FunctionDefinition();

	// アクセサ

	//関数名を得る
	Identifier* getFunctionName() const;
	//関数名設定
	void setFunctionName(Identifier* pId_);

	// パラメーターリストを得る
	ParameterDeclarationList*	getParam() const;
	// パラメーターリストを設定する
	void setParam(ParameterDeclarationList* pParam_);

	// Returnsを得る
	ReturnsClause*	getReturns() const;
	// Returnsを設定する
	void setReturns(ReturnsClause* pReturns_);

	// RoutineBodyを得る
	RoutineBody*	getRoutine() const;
	// RoutineBodyを設定する
	void setRoutine(RoutineBody* pRoutine_);

	//自身をコピーする
	Object* copy() const;

	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

protected:
private:
	// 代入オペレーターは使わない
	FunctionDefinition& operator=(const FunctionDefinition& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_FUNCTIONDEFINITION_H

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
