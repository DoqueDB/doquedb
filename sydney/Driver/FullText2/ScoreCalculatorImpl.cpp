// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ScoreCalculatorImpl.cpp --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/ScoreCalculatorImpl.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::ScoreCalculatorImpl::ScoreCalculatorImpl -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ScoreCalculatorImpl::ScoreCalculatorImpl()
{
}

//
//	FUNCTION public
//	FullText2::ScoreCalculatorImpl::~ScoreCalculatorImpl -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ScoreCalculatorImpl::~ScoreCalculatorImpl()
{
}

//
//	FUNCTION public
//	FullText2::ScoreCalculatorImpl::prepare
//		-- TF項計算の前準備を行う
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<FullText2::ScoreCalculator::Argument>& arg_
//		引数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ScoreCalculatorImpl::
prepare(const ModVector<FullText2::ScoreCalculator::Argument>& arg_)
{
}

//
//	FUNCTION public
//	FullText2::ScoreCalculatorImpl::secondStep
//		-- IDF項を計算する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<FullText2::ScoreCalculator::Argument>& arg_
//		引数
//
//	RETURN
//	double
//		TF項
//
//	EXCEPTIONS
//
double
ScoreCalculatorImpl::secondStep(const ModVector<Argument>& arg_)
{
	return 1.0;
}

//
//	FUNCTION protected
//	FullText2::ScoreCalculatorImpl::nextPos
//		-- パラメータのポインターを次に進める
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar* p_
//		パラメータ文字列へのポインタ
//
//	RETURN
//	const ModUnicodeChar*
//	   	次のパラメータ文字列へのポインタ
//
//	EXCEPTIONS
//
const ModUnicodeChar*
ScoreCalculatorImpl::nextPos(const ModUnicodeChar* p_)
{
	if (p_ == 0)
		return p_;
	
	// ScoreCalculatorに渡されるパラメータは、数値がコロン(:)で
	// 区切られている文字列である。
	
	while (*p_ != 0)
	{
		if (*p_ == ':')
		{
			++p_;
			break;
		}

		++p_;
	}

	return p_;
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
