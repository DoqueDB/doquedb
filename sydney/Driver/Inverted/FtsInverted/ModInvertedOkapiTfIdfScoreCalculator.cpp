// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOkapiTfIdfScoreCalculator.cpp -- OkapiTfIdf ランキングスコア計算器の実装
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
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
#include "ModInvertedOkapiTfIdfScoreCalculator.h"
#include "ModInvertedException.h"

//
// CONST
// ModInvertedOkapiTfIdfScoreCalculator::calculatorName -- 計算器名
//
// NOTES
// スコア計算器の名称
//
/*static*/ const char
ModInvertedOkapiTfIdfScoreCalculator::calculatorName[]
	= "OkapiTfIdf";

//
// FUNCTION public
// ModInvertedOkapiTfIdfScoreCalculator::ModInvertedOkapiTfIdfScoreCalculator -- コンストラクタ
// 
// NOTES
// コンストラクタ
//
// ARGUMENTS
// const ModInvertedOkapiTfIdfScoreCalculator& original
//		コピーもと
//
// RETURN
// なし
//
// EXCEPTIONS 
// 下位からの例外をそのまま返す
//
ModInvertedOkapiTfIdfScoreCalculator::ModInvertedOkapiTfIdfScoreCalculator()
	: ModInvertedOkapiTfScoreCalculator()
{
	setParameterX(0.2);
	setParameterY(1);			// Ogawa式をデフォルトとする
	setParameterA(0.0);
	setParameterS(1.0);
	setParameterQ(0.0);
}

ModInvertedOkapiTfIdfScoreCalculator::ModInvertedOkapiTfIdfScoreCalculator(
	const ModInvertedOkapiTfIdfScoreCalculator& original)
	: ModInvertedOkapiTfScoreCalculator(original)
{
	setParameterX(original.getParameterX());
	setParameterY(original.getParameterY());
	setParameterA(original.getParameterA());
	setParameterS(original.getParameterS());
	setParameterQ(original.getParameterQ());
}

//
// FUNCTION public
// ModInvertedOkapiTfIdfScoreCalculator::~ModInvertedOkapiTfIdfScoreCalculator -- デストラクタ
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
ModInvertedOkapiTfIdfScoreCalculator::~ModInvertedOkapiTfIdfScoreCalculator()
{}

//
// FUNCTION public
// ModInvertedOkapiTfIdfScoreCalculator::duplicate -- 自分自身の複製
// 
// NOTES
// 自分自身の複製の作成する。
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
ModInvertedOkapiTfIdfScoreCalculator::duplicate() const
{
	return new ModInvertedOkapiTfIdfScoreCalculator(*this);
}

// 
// FUNCTION public
// ModInvertedOkapiTfIdfScoreCalculator::secondStep -- DF項の計算
// 
// NOTES
// DF項を計算する。詳しくは先頭部分のコメント参照。
//
// ARGUMENTS
// const ModSize df
//		出現数
// const ModSize totalDocument
//		データーベースに登録されている全登録文書数
// 
// RETURN
// DF項の計算結果
//
// EXCEPTIONS
// なし
//
ModInvertedDocumentScore
ModInvertedOkapiTfIdfScoreCalculator::secondStep(const ModSize df,
												 const ModSize totalDocument)
	const
{
#ifdef DEBUG
	++countSecondStep;
#endif

	if (y == 1) {
		// Ogawa 式
		if (x == 0) {
			return 1.0;
		}
		return ModOsDriver::log(1.0 + x*(double)totalDocument/(double)df)
			/ModOsDriver::log(1.0 + x*(double)totalDocument);
	} else if (y == 4) {
		// Ogawa 式
		return ModOsDriver::log(1.0 +
								x/(1.0 - x)*(double)totalDocument/(double)df);
	} if (y == 6) {
		// Ogawa2 式
		return ModOsDriver::log((x*(double)totalDocument + df)
								/(q*(double)totalDocument + df))
		      /ModOsDriver::log((x*(double)totalDocument + 1.0)
								/(q*(double)totalDocument + 1.0));
	} else if (y == 7) {
		// Ogawa2 式
		return ModOsDriver::log((x*(double)totalDocument + df)
								/(q*(double)totalDocument + df));
	} else if (y == 0) {
		// Robertson 式
		return (x + ModOsDriver::log((double)totalDocument/(double)df))
			/(x + ModOsDriver::log((double)totalDocument));
	} else if (y == 3) {
		// Robertson 式
		return ModOsDriver::log(x/(1.0 - x)*(double)totalDocument/(double)df);
	} else if (y == 2) {
		// Haprper/Croft 式
		return (x + ModOsDriver::log((double)(totalDocument - df)/(double)df))
			/(x + ModOsDriver::log((double)(totalDocument - 1.0)));
	} else if (y == 5) {
		// Haprper/Croft 式
		return ModOsDriver::log(x/(1.0 - x)*
								(double)(totalDocument - df)/(double)df);
#if 0
	} else if (y == 8) {
		// s == 1, q == 0 の場合
		return ModOsDriver::log(
			1.0 + (a + x*(double)totalDocument/(double)df)/(1.0 - x));
#endif
	} else if (y == 8) {
		// s == 1 の場合
		ModInvertedDocumentScore T((double)df/(double)(totalDocument - df));
		if (x == 1) {
			return 0;
		}
		return ModOsDriver::log(
			((x + (1.0 + a)*T)/(1.0 - x))/
			((q + T)/(1.0 - q)));
	} else if (y == 9) {
		// まともに計算する
		ModInvertedDocumentScore tmp1((double)totalDocument/(double)df);
		ModInvertedDocumentScore tmp2(ModOsDriver::pow(tmp1, s));
		return ModOsDriver::log(
			(x + (1.0 + a)/(1.0 - tmp2))/(1.0 - x)/
			(q +1.0/(tmp1 - 1))*(1.0 - q));
	}
	; ModAssert(0);
}

// 
// FUNCTION public
// ModInvertedOkapiTfIdfScoreCalculator::setParameterX -- パラメータのアクセサ関数
// 
// NOTES
// 文書頻度調整用パラメータ x のアクセサ関数。x をセット。
//
// ARGUMENTS
// const double x_
//		文書頻度調整用パラメータ
// 
// RETURN
// なし	
//
// EXCEPIOTNS
// ModInvertedErrorInvalidScoreCalculatorParameter
//
void
ModInvertedOkapiTfIdfScoreCalculator::setParameterX(const double x_)
{
#if 0
	// 論文ではマイナスも認めている
	if (x_ < 0) {
		// 不正な値 (x は0以上のdouble)
		ModErrorMessage << "invalide parameter: x=" << x_ << ModEndl;
		ModThrowInvertedFileError(
			ModInvertedErrorInvalidScoreCalculatorParameter);
	}
#endif
	this->x = x_;
}

// 
// FUNCTION public
// ModInvertedOkapiTfIdfScoreCalculator::getParameterX -- パラメータのアクセサ関数
// 
// NOTES
// 文書頻度調整用パラメータ x のアクセサ関数。x を得る
//
// ARGUMENTS
// なし
// 
// RETURN
// 文書頻度調整用パラメータ
//
// EXCEPIOTNS
// なし
//
double
ModInvertedOkapiTfIdfScoreCalculator::getParameterX() const
{
	return x;
}

// 
// FUNCTION public
// ModInvertedOkapiTfIdfScoreCalculator::setParameterY -- パラメータのアクセサ関数
// 
// NOTES
// スコア計算式変更用パラメータ y のアクセサ関数。y をセット。
//
// ARGUMENTS
// const int y_
//		スコア計算式変更用パラメータ
// 
// RETURN
// なし	
//
// EXCEPIOTNS
// ModInvertedErrorInvalidScoreCalculatorParameter
//
void
ModInvertedOkapiTfIdfScoreCalculator::setParameterY(const int y_)
{
	if (y_ < 0 || 10 < y_) {
		ModErrorMessage << "invalid parameter: y=" << y_ << ModEndl;
		ModThrowInvertedFileError(
			ModInvertedErrorInvalidScoreCalculatorParameter);
	}
	this->y = y_;
}

// 
// FUNCTION public
// ModInvertedOkapiTfIdfScoreCalculator::getParameterY -- パラメータのアクセサ関数
// 
// NOTES
// スコア計算式変更用パラメータ y のアクセサ関数。y を得る。
//
// ARGUMENTS
// なし
// 
// RETURN
// スコア計算式変更用パラメータ
//
// EXCEPIOTNS
// なし
//
int
ModInvertedOkapiTfIdfScoreCalculator::getParameterY() const
{
	return y;
}

// 
// FUNCTION public
// ModInvertedOkapiTfIdfScoreCalculator::setParameterS -- パラメータのアクセサ関数
// 
// NOTES
// 文書頻度調整用パラメータ s のアクセサ関数。s をセット。
//
// ARGUMENTS
// const double a_
//		文書頻度調整用パラメータ
// 
// RETURN
// なし	
//
// EXCEPIOTNS
// なし
//
void
ModInvertedOkapiTfIdfScoreCalculator::setParameterA(const double a_)
{
	this->a = a_;
}

// 
// FUNCTION public
// ModInvertedOkapiTfIdfScoreCalculator::getParameterS -- パラメータのアクセサ関数
// 
// NOTES
// 文書頻度調整用パラメータ a のアクセサ関数。a を得る。
//
// ARGUMENTS
// なし
// 
// RETURN
// 文書頻度調整用パラメータ a
//
// EXCEPIOTNS
// なし
//
double
ModInvertedOkapiTfIdfScoreCalculator::getParameterA() const
{
	return a;
}

// 
// FUNCTION public
// ModInvertedOkapiTfIdfScoreCalculator::setParameterS -- パラメータのアクセサ関数
// 
// NOTES
// 文書頻度調整用パラメータ s のアクセサ関数。s をセット。
//
// ARGUMENTS
// const double s_
//		文書頻度調整用パラメータ
// 
// RETURN
// なし	
//
// EXCEPIOTNS
// なし
//
void
ModInvertedOkapiTfIdfScoreCalculator::setParameterS(const double s_)
{
	this->s = s_;
}

// 
// FUNCTION public
// ModInvertedOkapiTfIdfScoreCalculator::getParameterS -- パラメータのアクセサ関数
// 
// NOTES
// 文書頻度調整用パラメータ s のアクセサ関数。s を得る。
//
// ARGUMENTS
// なし
// 
// RETURN
// 文書頻度調整用パラメータ s
//
// EXCEPIOTNS
// なし
//
double
ModInvertedOkapiTfIdfScoreCalculator::getParameterS() const
{
	return s;
}

// 
// FUNCTION public
// ModInvertedOkapiTfIdfScoreCalculator::setParameterQ -- パラメータのアクセサ関数
// 
// NOTES
// 文書頻度調整用パラメータ q のアクセサ関数。q をセット。
//
// ARGUMENTS
// const double q_
//		文書頻度調整用パラメータ
// 
// RETURN
// なし	
//
// EXCEPIOTNS
// なし
//
void
ModInvertedOkapiTfIdfScoreCalculator::setParameterQ(const double q_)
{
	this->q = q_;
}

// 
// FUNCTION public
// ModInvertedOkapiTfIdfScoreCalculator::getParameterQ -- パラメータのアクセサ関数
// 
// NOTES
// 文書頻度調整用パラメータ q のアクセサ関数。q を得る。
//
// ARGUMENTS
// なし
// 
// RETURN
// 文書頻度調整用パラメータ q
//
// EXCEPIOTNS
// なし
//
double
ModInvertedOkapiTfIdfScoreCalculator::getParameterQ() const
{
	return q;
}

// 
// FUNCTION public
// ModInvertedOkapiTfIdfScoreCalculator::setParameter -- パラメータセット
// 
// NOTES
// OkapiTfIdfScoreCalculatorにパラメータをセットする。
//  
//
// ARGUMENTS
// ModString& paramerteString
//		パラメータを表す文字列。"k:x:y:a:s"の形式で渡す。
// 
// RETURN
// なし
//
// EXCEPTIONS
// ModInvertedErrorInvalidScoreCalculatorParameter
//
void
ModInvertedOkapiTfIdfScoreCalculator::setParameter(
	const ModString& parameterString)
{
	ModVector<ModString> parameter;

	ModSize size = divideParameter(parameterString, parameter);

	if (size > 6) {
		// パラメータは k, x, y, a, s, q の 6 つ
		ModErrorMessage << "invalid parameter num: " << size
						<< ModEndl;
		ModThrowInvertedFileError(
			ModInvertedErrorInvalidScoreCalculatorParameter);
	}

	ModVector<ModString>::Iterator iterator(parameter.begin());

	if (size > 0) {
		setParameterK((double)ModCharTrait::toFloat(*iterator));
	}
	if (size > 1) {
		++iterator;
		setParameterX((double)ModCharTrait::toFloat(*iterator));
	}
	if (size > 2) {
		++iterator;
		setParameterY(ModCharTrait::toInt(*iterator));
	}
	if (size > 3) {
		++iterator;
		setParameterQ((double)ModCharTrait::toFloat(*iterator));
	}
	if (size > 4) {
		++iterator;
		setParameterA((double)ModCharTrait::toFloat(*iterator));
	}
	if (size > 5) {
		++iterator;
		setParameterS((double)ModCharTrait::toFloat(*iterator));
	}
}

//
// FUNCTION public
// ModInvertedOkapiTfIdfScoreCalculator::getDescription -- 記述文字列の獲得
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
ModInvertedOkapiTfIdfScoreCalculator::getDescription(
	ModString& description_,
	const ModBoolean withParameter_) const
{
	description_ = calculatorName;

	if (withParameter_ == ModTrue) {
		ModOstrStream stream;
		stream << ':' << k << ':' << x << ':' << y;
		if (6 <= y) { 
			stream << ':' << q;
		}
		if (8 <= y) { 
			stream << ':' << a;
		}
		if (9 <= y) { 
			stream << ':' << s;
		}
		description_ += stream.getString();
	}
}

//
// Copyright (c) 1999, 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
