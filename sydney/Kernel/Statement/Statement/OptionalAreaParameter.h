// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OptionalAreaParameter.h --
// 
// Copyright (c) 2000, 2002, 2003, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_OPTIONALAREAPARAMETER_H
#define __SYDNEY_STATEMENT_OPTIONALAREAPARAMETER_H

#include "Statement/ObjectSelection.h"


_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class Object;

//	CLASS
//	Statement::OptionalAreaParameter --
//
//	NOTES

class OptionalAreaParameter : public Statement::ObjectSelection
{
public:
	//
	//	動作識別子
	//
	enum OptionType
	{
		Unknown = 0,
		ParameterList,
		DropAllArea
	};

	//constructor
	OptionalAreaParameter()
		: ObjectSelection(ObjectType::OptionalAreaParameter)
	{}
	//コンストラクタ(2)
	SYD_STATEMENT_FUNCTION
	OptionalAreaParameter(OptionType iOptionType_, Statement::Object* pcParameter_);
	//デストラクタ
	SYD_STATEMENT_FUNCTION
	virtual ~OptionalAreaParameter();

	//
	//	アクセサ
	//
	//動作識別子アクセサ
	int getOptionType() const
		{ return ObjectSelection::getObjectType(); }
	void setOptionType(OptionType iOptionType_)
		{ ObjectSelection::setObjectType(iOptionType_); }

	//Option アクセサ
	Statement::Object* getOption() const
		{ return ObjectSelection::getObject(); }
	void setOption(Object* pcParameter_)
		{ ObjectSelection::setObject(pcParameter_); }

	//自身をコピーする
	SYD_STATEMENT_FUNCTION
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

private:
	//代入オペレータは使用しない
	OptionalAreaParameter& operator= (const OptionalAreaParameter& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_OPTIONALAREAPARAMETER_H

//
//	Copyright (c) 2000, 2002, 2003, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
