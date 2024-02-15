// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOkapiTfScoreCalculator.cpp -- OkapiTf ランキングスコア計算器の実装
// 
// Copyright (c) 1999, 2000, 2002, 2023 Ricoh Company, Ltd.
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
#include "ModInvertedOkapiTfScoreCalculator.h"
#include "ModInvertedException.h"

//
// スコア計算式 :
//		tf / ( 1 + tf )
// 
// tf: 文書内出現数
//

//
// CONST
// ModInvertedOkapiTfScoreCalculator::calculatorName -- 計算器名
//
// NOTES
// スコア計算器の名称
//
/*static*/ const char
ModInvertedOkapiTfScoreCalculator::calculatorName[]
	= "OkapiTf";

//
// FUNCTION public
// ModInvertedOkapiTfScoreCalculator::ModInvertedOkapiTfScoreCalculator -- コンストラクタ
// 
// NOTES
// コンストラクタ
//
// ARGUMENTS
// const ModInvertedOkapiTfScoreCalculator& original
//		コピーもと
//
// RETURN
// なし
// 
// EXCEPTIONS
// ModInvertedErrorInvalidScoreCalculatorParameter -- 不正なパラメータ
//
ModInvertedOkapiTfScoreCalculator::ModInvertedOkapiTfScoreCalculator()
{
	setParameterK(1.0);
}

ModInvertedOkapiTfScoreCalculator::ModInvertedOkapiTfScoreCalculator(
	const ModInvertedOkapiTfScoreCalculator& original)
	: ModInvertedRankingScoreCalculator(original)
{
	setParameterK(original.getParameterK());
}

//
// FUNCTION public
// ModInvertedOkapiTfScoreCalculator::duplicate -- 自分自身の複製の作成
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
// 下位モジュールからの例外をそのまま返す
//
ModInvertedRankingScoreCalculator*
ModInvertedOkapiTfScoreCalculator::duplicate() const
{
	return new ModInvertedOkapiTfScoreCalculator(*this);
}

// 
// FUNCTION public
// ModInvertedOkapiTfScoreCalculator::setParameterK -- 文書内頻度調整ようパラメータのアクセサ関数。
// 
// NOTES
// 文書内頻度調整用パラメータkのアクセサ関数。kをセット。
//
// ARGUMENTS
//	const double k_
//		文書内頻度調整用パラメータ
// 
// RETURN
//	なし	
//
// EXCEPIOTNS
// ModInvertedErrorInvalidScoreCalculatorParameter -- 不正なパラメータ
//
void
ModInvertedOkapiTfScoreCalculator::setParameterK(const double k_)
{
	if (k_ < 0) {
		// 不正な値 (k は0以上のdouble)
		ModErrorMessage << "invalide ScoreCalculator Parameter: " << k_
						<< ModEndl;
		ModThrowInvertedFileError(
			ModInvertedErrorInvalidScoreCalculatorParameter);
	}
	this->k = k_;
	precalculate();
}

// 
// FUNCTION public
// ModInvertedOkapiTfScoreCalculator::getParameterK -- 文書内頻度調整ようパラメータのアクセサ関数。
// 
// NOTES
// 文書内頻度調整用パラメータkのアクセサ関数。文書内調整用パラメータを得る。
//
// ARGUMENTS
//	なし
// 
// RETURN
//	文書内頻度調整用パラメータ
//
// EXCEPIOTNS
// なし
//
double
ModInvertedOkapiTfScoreCalculator::getParameterK() const
{
	return k;
}

// 
// FUNCTION public
// ModInvertedOkapiTfScoreCalculator::setParameter パラメータセット
// 
// NOTES
// OkapiTfScoreCalculatorにパラメータをセットする。
//  
//
// ARGUMENTS
// ModString& paramerteString
//		パラメータを表す文字列。"k"の形式で渡す。
// 
// RETURN
// なし
//
// EXCEPIOTNS
// ModInvertedErrorInvalidScoreCalculatorParameter -- 不正なパラメータ
//
void
ModInvertedOkapiTfScoreCalculator::setParameter(
	const ModString& parameterString)
{
	ModVector<ModString> parameter;

	ModSize size = divideParameter(parameterString, parameter);

	if (size > 1) {
		// パラメータは k の 1 つ
		ModErrorMessage << "invalid parameter num: " << size
						<< ModEndl;
		ModThrowInvertedFileError(
			ModInvertedErrorInvalidScoreCalculatorParameter);
	}

	ModVector<ModString>::Iterator iterator(parameter.begin());

	if (size > 0) {
		setParameterK((double)ModCharTrait::toFloat(*iterator));
	}
}

//
// FUNCTION public
// ModInvertedOkapiTfScoreCalculator::getDescription -- 記述文字列の獲得
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
ModInvertedOkapiTfScoreCalculator::getDescription(
	ModString& description_,
	const ModBoolean withParameter_) const
{
	description_ = calculatorName;

	if (withParameter_ == ModTrue) {
		ModOstrStream stream;
		stream << ':' << k;
		description_ += stream.getString();
	}
}

//
// Copyright (c) 1999, 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
