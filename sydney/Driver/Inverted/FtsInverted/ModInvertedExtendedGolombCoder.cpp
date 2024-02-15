// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedExtendedGolombCoder.cpp -- 符合器の実装
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
#include "ModInvertedExtendedGolombCoder.h"

//
// CONST
// ModInvertedExtendedGolombCoder::tokenizerName -- 符合化器の名称
//
// NOTES
// ModInvertedExtendedGolombCoder の符合化器の名称を表す
//
/*static*/
const char ModInvertedExtendedGolombCoder::coderName[] = "ETG";


#ifdef CODER_HAS_CURRENT_LOCATION
//
// FUNCTION
// ModInvertedExtendedCoder::duplicate -- 複製の生成
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
ModInvertedExtendedGolombCoder::duplicate() const
{
	return new ModInvertedExtendedGolombCoder(*this);
}
#endif

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
		ModErrorMessage << "invalid lambda: " << lambda_ << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorInvalidCoderArgument);
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
		ModErrorMessage << "invalid factor: " << factor_ << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorInvalidCoderArgument);
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

#if 0
void
ModInvertedExtendedGolombCoder::setParameters(const int lambda_, const int factor_)
{
	setLambda(lambda_);
	setFactor(factor_);
	setValues();
}
#endif

//
// FUNCTION
// ModInvertedExtendedGolombCoder::parse -- 符合器記述の解析
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
ModInvertedExtendedGolombCoder::parse(const ModCharString& description_)
{
	ModCharString tmp(description_);
	char* delim = tmp.search(':');
	if (delim == 0) {
		// 後半部分がない場合
		setFactor(1);
	} else {
		setFactor(ModCharTrait::toInt(delim + 1));
		tmp.truncate(delim);
	}
	setLambda(tmp.toInt());
#ifdef DEBUG
	ModDebugMessage << "lambda=" << lambda << " factor=" << factor << ModEndl;
#endif
	setValues();
}

//
// FUNCTION
// ModInvertedExtendedGolombCoder::getDescription -- 符合器記述の取得
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
ModInvertedExtendedGolombCoder::getDescription(ModCharString& description_) const
{
	description_.format("%s:%d:%d", coderName, lambda, factor);
}


//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

