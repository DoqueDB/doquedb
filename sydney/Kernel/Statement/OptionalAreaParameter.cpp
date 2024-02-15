// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OptionalAreaParameter.cpp --
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Statement";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Statement/Type.h"
#include "Statement/OptionalAreaParameter.h"
#if 0
#include "Analysis/OptionalAreaParameter.h"
#endif

_SYDNEY_USING

namespace
{
}

using namespace Statement;

//
//	FUNCTION public
//	Statement::OptionalAreaParameter::OptionalAreaParameter -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
OptionalAreaParameter::OptionalAreaParameter(OptionType iOptionType_, Object* pcParameter_)
	: ObjectSelection(ObjectType::OptionalAreaParameter)
{
	setOptionType(iOptionType_);
	setOption(pcParameter_);
}

//
//	FUNCTION public
//	Statement::OptionalAreaParameter::~OptionalAreaParameter -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
OptionalAreaParameter::~OptionalAreaParameter()
{
}

//
//	FUNCTION public
//	Statement::OptionalAreaParameter::copy -- 自身をコピーする
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
//	なし
//
Object*
OptionalAreaParameter::copy() const
{
	return new OptionalAreaParameter(*this);
}

#if 0
namespace
{
	Analysis::OptionalAreaParameter _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
OptionalAreaParameter::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
