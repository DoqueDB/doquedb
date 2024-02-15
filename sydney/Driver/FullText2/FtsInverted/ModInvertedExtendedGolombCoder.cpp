// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedExtendedGolombCoder.cpp -- 符合器の実装
// 
// Copyright (c) 2000, 2002, 2010, 2013, 2023 Ricoh Company, Ltd.
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
#include "ModInvertedExtendedGolombCoder.h"

#include "ModUnicodeCharTrait.h"

#include "Common/Message.h"

#include "Exception/BadArgument.h"

_SYDNEY_USING

//
// CONST
// ModInvertedExtendedGolombCoder::tokenizerName -- 符合化器の名称
//
// NOTES
// ModInvertedExtendedGolombCoder の符合化器の名称を表す
//
/*static*/
const ModUnicodeChar ModInvertedExtendedGolombCoder::coderName[]
= {'E','T','G',0};


//
// FUNCTION
// ModInvertedExtededGolombCoder::setLambda -- パラメータの変更
//
// NOTES
// パラメータを変更する。
//
// ARGUMENTS
// const int lambda_
//		パラメータ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
// 
void
ModInvertedExtendedGolombCoder::setLambda(const int lambda_)
{
	if (lambda_ < 0 || 31 < lambda_) {
		SydErrorMessage << "invalid lambda: " << lambda_ << ModEndl;
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	lambda = lambda_;
	lambda1 = lambda + 1;
	lambda2 = 1<<lambda;
}

//
// FUNCTION
// ModInvertedExtededGolombCoder::setLambda -- パラメータの変更
//
// NOTES
// パラメータを変更する。
//
// ARGUMENTS
// const int lambda_
//		パラメータ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
// 
void
ModInvertedExtendedGolombCoder::setFactor(const int factor_)
{
	if (factor_ < 1 || 31 < factor_) {
		SydErrorMessage << "invalid factor: " << factor_ << ModEndl;
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	factor = factor_;
	factor1 = factor + 1;
	factor2 = 1<<factor;
}

void
ModInvertedExtendedGolombCoder::setValues()
{
	for (int i(0); i < 32; ++i) {
		values[i] = lambda2*((1<<i) - 1)/(factor2 - 1) + 1;
	}
}

//
// FUNCTION
// ModInvertedExtendedGolombCoder::parse -- 符合器記述の解析
//
// NOTES
// 符合器記述を解析し、パラメータその記述の値とする。
//
// ARGUMENTS
// const ModUnicodeString& description_
//		符号器記述
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void 
ModInvertedExtendedGolombCoder::parse(const ModUnicodeString& description_)
{
	ModUnicodeString tmp(description_);
	ModUnicodeChar* delim = tmp.search(':');
	if (delim == 0) {
		// 後半部分がない場合
		setFactor(1);
	} else {
		setFactor(ModUnicodeCharTrait::toInt(delim + 1));
		tmp.truncate(delim);
	}
	setLambda(ModUnicodeCharTrait::toInt(tmp));
#ifdef DEBUG
	ModDebugMessage << "lambda=" << lambda << " factor=" << factor << ModEndl;
#endif
	setValues();
}

//
// Copyright (c) 2000, 2002, 2010, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

