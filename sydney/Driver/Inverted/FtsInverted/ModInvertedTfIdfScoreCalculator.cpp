// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedTfIdfScoreCalculator.cpp -- TfIdf ランキングスコア計算器の実装
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
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

#include "ModOsDriver.h"				// log()
#include "ModOstrStream.h"
#include "ModInvertedTypes.h"
#include "ModInvertedTfIdfScoreCalculator.h"
#include "ModInvertedException.h"

//
// CONST
// ModInvertedTfIdfScoreCalculator::calculatorName -- 計算器名
//
// NOTES
// スコア計算器の名称
//
/*static*/ const char
ModInvertedTfIdfScoreCalculator::calculatorName[]
	= "TfIdf";

//
// FUNCTION public
// ModInvertedTfIdfScoreCalculator::ModInvertedTfIdfScoreCalculator -- コンストラクタ
// 
// NOTES
// コンストラクタ
//
// ARGUMENTS
// const ModInvertedTfIdfScoreCalculator& original
// 		コピーもと
//
// RETURN
// なし
// 
// EXCEPTIONS 
// 下位からの例外をそのまま返す
//
ModInvertedTfIdfScoreCalculator::ModInvertedTfIdfScoreCalculator()
	: ModInvertedRankingScoreCalculator()
{
	setParameterK1(0);
	setParameterK2(1);
	setParameterX(0);
	setParameterY(0);
}

ModInvertedTfIdfScoreCalculator::ModInvertedTfIdfScoreCalculator(
	const ModInvertedTfIdfScoreCalculator& original)
	: ModInvertedRankingScoreCalculator(original)
{
	setParameterK1(original.getParameterK1());
	setParameterK2(original.getParameterK2());
	setParameterX(original.getParameterX());
	setParameterY(original.getParameterY());
}

//
// FUNCTION public
// ModInvertedTfIdfScoreCalculator::~ModInvertedTfIdfScoreCalculator -- デストラクタ
// 
// NOTES
// デストラクタ
//
// ARGUMENTS
// なし
//
// RETURN
// なし
// 
// EXCEPTIONS 
// なし
//
ModInvertedTfIdfScoreCalculator::~ModInvertedTfIdfScoreCalculator()
{}

//
// FUNCTION public
// ModInvertedTfIdfScoreCalculator::duplicate -- 自分自身の複製の作成
// 
// NOTES
// 自分自身の複製の作成
// 本来はデータの内容の複製も必要であると考えていたが現時点では必要な
// いので単にnewしているだけ
//
// ARGUMENTS
// なし
//
// RETURN
// なし
// 
// EXCEPTIONS 
// 下位からの例外をそのまま返す
//
ModInvertedRankingScoreCalculator*
ModInvertedTfIdfScoreCalculator::duplicate() const
{
	return new ModInvertedTfIdfScoreCalculator(*this);
}

// 
// FUNCTION public
// ModInvertedTfIdfScoreCalculator::firstStep -- k1 + k2 * tf の計算
// 
// NOTES
// k1 + k2 * tf の計算
//
// ARGUMENTS
// const ModSize tf
//		文書内出現頻度
// 
// RETURN
// k1 + k2 * tf の結果
//
// EXCEPTIONS 
// なし
//
ModInvertedDocumentScore
ModInvertedTfIdfScoreCalculator::firstStep(const ModSize tf,
										   const DocumentID ID,
										   ModBoolean& exist) const
{
#ifdef DEBUG
	++countFirstStep;
#endif

	exist = ModTrue;
	return ModInvertedDocumentScore(k1 + k2 * (double)tf);
}

// 
// FUNCTION public
// ModInvertedTfIdfScoreCalculator::secondStep -- log( N / df ) / log( N ) の計算
// 
// NOTES
// log( N / df ) / log( N ) の計算
//
// ARGUMENTS
// const ModSize df
//		出現数
// const ModSize totalDocument
//		データーベースに登録されている全登録文書数
// 
// RETURN
// 計算結果
//
// EXCEPTIONS
// なし
//
ModInvertedDocumentScore
ModInvertedTfIdfScoreCalculator::secondStep(const ModSize df,
											const ModSize totalDocument) const
{
#ifdef DEBUG
	++countSecondStep;
#endif

	if (y == 0) {
		// Robertson オリジナルの計算式
		return (x + ModOsDriver::log((double)totalDocument/(double)df))
			/(x + ModOsDriver::log((double)totalDocument));
	} else if (x == 0) {
		// 改良計算式
		return 1.0;
	} else {
		// 改良計算式
		return ModOsDriver::log(1.0 + x*(double)totalDocument/(double)df)
			/ModOsDriver::log(1.0 + x*(double)totalDocument);
	}
}

// 
// FUNCTION public
// ModInvertedTfIdfScoreCalculator::setParameterK1 -- 文書内頻度調整ようパラメータ1のアクセサ関数。
// 
// NOTES
// 文書内頻度調整用パラメータk1のアクセサ関数。k1をセット。
//
// ARGUMENTS
// const double k_
//		文書内頻度調整用パラメータ
// 
// RETURN
// なし	
//
// EXCEPIOTNS
// ModInvertedErrorInvalidScoreCalculatorParameter
//
void
ModInvertedTfIdfScoreCalculator::setParameterK1(const double k1_)
{
	if (k1_ < 0) {
		// 不正な値 (k1 は0以上のdouble)
		ModErrorMessage << "invalid parameter: k1=" << k1_ << ModEndl;
		ModThrowInvertedFileError(
			ModInvertedErrorInvalidScoreCalculatorParameter);
	}
	this->k1 = k1_;
}

// 
// FUNCTION public
// ModInvertedTfIdfScoreCalculator::getParameterK1 -- 文書内頻度調整ようパラメータのアクセサ関数。
// 
// NOTES
// 文書内頻度調整用パラメータk1のアクセサ関数。文書内調整用パラメータを得る。
//
// ARGUMENTS
// なし
// 
// RETURN
// 文書内頻度調整用パラメータ
//
// EXCEPIOTNS
// なし
//
double
ModInvertedTfIdfScoreCalculator::getParameterK1() const
{
	return this->k1;
}

// 
// FUNCTION public
// ModInvertedTfIdfScoreCalculator::setParameterK2 -- 文書内頻度調整ようパラメータ2のアクセサ関数。
// 
// NOTES
// 文書内頻度調整用パラメータk2のアクセサ関数。k2をセット。
//
// ARGUMENTS
// const double k2_
//		文書内頻度調整用パラメータ
// 
// RETURN
// なし	
//
// EXCEPIOTNS
// ModInvertedErrorInvalidScoreCalculatorParameter
//
void
ModInvertedTfIdfScoreCalculator::setParameterK2(const double k2_)
{
	if (k2_ < 0) {
		// 不正な値 (k2 は0以上のdouble)
		ModErrorMessage << "invalid parameter: k2=" << k2_ << ModEndl;
		ModThrowInvertedFileError(
			ModInvertedErrorInvalidScoreCalculatorParameter);
	}
	this->k2 = k2_;
}

// 
// FUNCTION public
// ModInvertedTfIdfScoreCalculator::getParameterK2 -- 文書内頻度調整ようパラメータ2のアクセサ関数。
// 
// NOTES
// 文書内頻度調整用パラメータk2のアクセサ関数。文書内調整用パラメータを得る。
//
// ARGUMENTS
// なし
// 
// RETURN
// 文書内頻度調整用パラメータ
//
// EXCEPIOTNS
// なし
//
double
ModInvertedTfIdfScoreCalculator::getParameterK2() const
{
	return this->k2;
}

// 
// FUNCTION public
// ModInvertedTfIdfScoreCalculator::setParameterX -- 文書内頻度調整ようパラメータのアクセサ関数。
// 
// NOTES
// 文書内頻度調整用パラメータkのアクセサ関数。xをセット。
//
// ARGUMENTS
// const double x_
//		文書内頻度調整用パラメータ
// 
// RETURN
// なし	
//
// EXCEPIOTNS
// ModInvertedErrorInvalidScoreCalculatorParameter
//
void
ModInvertedTfIdfScoreCalculator::setParameterX(const double x_){
	if (x_ < 0) {
		// 不正な値 (x は0以上のdouble)
		ModErrorMessage << "invalid parameter: x=" << x_ << ModEndl;
		ModThrowInvertedFileError(
			ModInvertedErrorInvalidScoreCalculatorParameter);
	}
	this->x = x_;
}

// 
// FUNCTION public
// ModInvertedTfIdfScoreCalculator::getParameterX -- 文書内頻度調整ようパラメータのアクセサ関数。
// 
// NOTES
// 文書内頻度調整用パラメータxのアクセサ関数。文書内調整用パラメータを得る。
//
// ARGUMENTS
// なし
// 
// RETURN
// 文書内頻度調整用パラメータ
//
// EXCEPIOTNS
// なし
//
double
ModInvertedTfIdfScoreCalculator::getParameterX() const
{
	return this->x;
}

// 
// FUNCTION public
// ModInvertedTfIdfScoreCalculator::setParameterY -- 文書内頻度調整用パラメータのアクセサ関数。
// 
// NOTES
// 文書内頻度調整用パラメータkのアクセサ関数。xをセット。
//
// ARGUMENTS
// const int y_
//		IDF計算式選択用パラメータ
// 
// RETURN
// なし	
//
// EXCEPIOTNS
// ModInvertedErrorInvalidScoreCalculatorParameter
//
void
ModInvertedTfIdfScoreCalculator::setParameterY(const int y_)
{
	this->y = y_;
}

// 
// FUNCTION public
// ModInvertedTfIdfScoreCalculator::getParameterY -- 文書内頻度調整用パラメータのアクセサ関数。
// 
// NOTES
// 文書内頻度調整用パラメータxのアクセサ関数。文書内調整用パラメータを得る。
//
// ARGUMENTS
// なし
// 
// RETURN
// 文書内頻度調整用パラメータ
//
// EXCEPIOTNS
// なし
//
int
ModInvertedTfIdfScoreCalculator::getParameterY() const
{
	return y;
}

// 
// FUNCTION public
// ModInvertedTfIdfScoreCalculator::setParameter パラメータセット
// 
// NOTES
// TfIdfScoreCalculatorにパラメータをセットする。
//  
//
// ARGUMENTS
// ModString& paramerteString
//		パラメータを表す文字列。"k1:k2:x:y"の形式で渡す。
// 
// RETURN
// なし
// 
// EXCEPTIONS
// ModInvertedErrorInvalidScoreCalculatorParameter
//
void
ModInvertedTfIdfScoreCalculator::setParameter(const ModString& parameterString)
{
	ModVector<ModString> parameter;

	ModSize size = divideParameter(parameterString, parameter);

	if (size > 4) {
		// パラメータは k1, k2, x, y の 4 つ
		ModErrorMessage << "invalid parameter num: " << size
						<< ModEndl;
		ModThrowInvertedFileError(
			ModInvertedErrorInvalidScoreCalculatorParameter);
	}

	ModVector<ModString>::Iterator iterator(parameter.begin());

	if (size > 0) {
		setParameterK1((double)ModCharTrait::toFloat(*iterator));
	}
	if (size > 1) {
		++iterator;
		setParameterK2((double)ModCharTrait::toFloat(*iterator));
	}
	if (size > 2) {
		++iterator;
		setParameterX((double)ModCharTrait::toFloat(*iterator));
	}
	if (size > 3) {
		++iterator;
		setParameterY(ModCharTrait::toInt(*iterator));
	}
}

//
// FUNCTION public
// ModInvertedTfIdfScoreCalculator::getDescription -- 記述文字列の獲得
//
// NOTES
// 自分を記述する文字列を獲得する。
//
// ARGUMENTS
// ModString& description_
//		文字列表現
// const ModBoolean withParameter_
//		パラメータ出力指示
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedTfIdfScoreCalculator::getDescription(
	ModString& description_,
	const ModBoolean withParameter_) const
{
	description_ = calculatorName;

	if (withParameter_ == ModTrue) {
		ModOstrStream stream;
		stream << ':' << k1 << ':' << k2 << ':' << x << ':' << y;
		description_ += stream.getString();
	}
}

//
// Copyright (c) 1999, 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
