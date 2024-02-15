// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedParameterizedExpGolombCoder.cpp -- 符合器の実装
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

#ifdef SYD_INVERTED // SYDNEY 対応
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#endif

#include "ModCharString.h"
#include "ModInvertedException.h"
#include "ModInvertedParameterizedExpGolombCoder.h"

//
// CONST
// ModInvertedParameterizedExpGolombCoder::tokenizerName -- 符合化器の名称
//
// NOTES
// ModInvertedParameterizedExpGolombCoder の符合化器の名称を表す
//
/*static*/
const char ModInvertedParameterizedExpGolombCoder::coderName[] = "PEG";


//
// FUNCTION
// ModInvertedParameterizedExpGolombCoder::ModInvertedParameterizedExpGolombCoder -- コンストラクタ
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
	const ModCharString& description_)
	:
	lambda(0), lambda1(1), lambda2(0), lambda3(1), ModInvertedCoder()
{
	parse(description_);
}

#ifdef CODER_HAS_CURRENT_LOCATION
ModInvertedParameterizedExpGolombCoder::ModInvertedParameterizedExpGolombCoder(
	const ModInvertedParameterizedExpGolombCoder& orig_)
	:
	lambda(orig_.lambda), lambda1(orig_.lambda1),
	lambda2(orig_.lambda2), lambda3(orig_.lambda3),
	ModInvertedCoder(orig_)
{}
#endif

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
		ModErrorMessage << "invalid lambda: " << lambda_ << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorInvalidCoderArgument);
	}
	lambda = lambda_;
	lambda1 = lambda + 1;
	lambda3 = 1<<lambda;
	lambda2 = lambda3 - 1;
}

#ifdef CODER_HAS_CURRENT_LOCATION
//
// FUNCTION
// ModInvertedParameterizedExpGolombCoder::duplicate -- 複製の生成
//
// NOTES
// 自分の複製を生成する。
//
// ARGUMENTS
// なし
//
// RETURN
// 複製された符号器
//
// EXCEPTIONS
// 下位のモジュールからの例外をそのまま返す
// 
ModInvertedCoder*
ModInvertedParameterizedExpGolombCoder::duplicate() const
{
	return new ModInvertedParameterizedExpGolombCoder(*this);
}
#endif

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
ModInvertedParameterizedExpGolombCoder::parse(const ModCharString& description_)
{
	setLambda(description_.toInt());
}

//
// FUNCTION
// ModInvertedParameterizedExpGolombCoder::getDescription -- 符合器記述の取得
//
// NOTES
// 符合器記述を取得する。
//
// ARGUMENTS
// ModCharString& description_
//		出力バッファ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedParameterizedExpGolombCoder::getDescription(ModCharString& description_) const
{
	description_.format("%s:%d", coderName, lambda);
}

//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
