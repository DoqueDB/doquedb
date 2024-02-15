// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedParameterizedExpGolombCoder.cpp -- 符合器の実装
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

#include "ModInvertedParameterizedExpGolombCoder.h"
#include "ModUnicodeCharTrait.h"

#include "Common/Message.h"
#include "Exception/BadArgument.h"

_SYDNEY_USING

//
// CONST
// ModInvertedParameterizedExpGolombCoder::tokenizerName -- 符合化器の名称
//
// NOTES
// ModInvertedParameterizedExpGolombCoder の符合化器の名称を表す
//
/*static*/
const ModUnicodeChar ModInvertedParameterizedExpGolombCoder::coderName[]
= {'P','E','G',0};


//
// FUNCTION
// ModInvertedParameterizedExpGolombCoder::
//	ModInvertedParameterizedExpGolombCoder -- コンストラクタ
//
// NOTES
// コンストラクタ。
// パラメータを引数にとるもの、引数オブジェクトを引数にとるもの、
// コピーコンストラクタの３種類がある。
//
// ARGUMENTS
// const int lambda_
//		パラメータ
// const Argument* argument_
//		引数オブジェクト
// const ModInvertedParameterizedExpGolombCoder& orig_
//		コピー元
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位のモジュールからの例外をそのまま返す
// 
/* purecov:begin deadcode */
ModInvertedParameterizedExpGolombCoder::ModInvertedParameterizedExpGolombCoder(
	const int lambda_)
	:
	lambda(0), lambda1(1), lambda2(0), lambda3(1), ModInvertedCoder()
{
	setLambda(lambda_);
}
/* purecov:end */

ModInvertedParameterizedExpGolombCoder::ModInvertedParameterizedExpGolombCoder(
	const ModUnicodeString& description_)
	:
	lambda(0), lambda1(1), lambda2(0), lambda3(1), ModInvertedCoder()
{
	parse(description_);
}

//
// FUNCTION
// ModInvertedParameterizedExpGolombCoder::setLambda -- パラメータの変更
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
ModInvertedParameterizedExpGolombCoder::setLambda(const int lambda_)
{
	if (lambda_ < 0 || 31 < lambda_) {
		SydErrorMessage << "invalid lambda: " << lambda_ << ModEndl;
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	lambda = lambda_;
	lambda1 = lambda + 1;
	lambda3 = 1<<lambda;
	lambda2 = lambda3 - 1;
}

//
// FUNCTION
// ModInvertedParameterizedExpGolombCoder::parse -- 符合器記述の解析
//
// NOTES
// 符合器記述を解析し、パラメータその記述の値とする。
//
// ARGUMENTS
// const ModCharString& description_
//		符号器記述
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void 
ModInvertedParameterizedExpGolombCoder::
parse(const ModUnicodeString& description_)
{
	setLambda(ModUnicodeCharTrait::toInt(description_));
}

//
// Copyright (c) 2000, 2002, 2010, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
