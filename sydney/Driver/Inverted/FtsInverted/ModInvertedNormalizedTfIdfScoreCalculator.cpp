// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedNormalizedTfIdfScoreCalculator.cpp -- NormalizedTfIdf ランキングスコア計算器の実装
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
#include "ModInvertedNormalizedTfIdfScoreCalculator.h"
#ifdef  SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedDocumentLengthFile.h"
#else
#include "ModInvertedDocumentLengthFile.h"
#endif
#include "ModInvertedException.h"

//
// CONST
// ModInvertedNormalizedTfIdfScoreCalculator::calculatorName -- 計算器名
//
// NOTES
// スコア計算器の名称
//
/*static*/ const char
ModInvertedNormalizedTfIdfScoreCalculator::calculatorName[]
	= "NormalizedTfIdf";

//
// FUNCTION public
// ModInvertedNormalizedTfIdfScoreCalculator::ModInvertedNormalizedTfIdfScoreCalculator -- コンストラクタ
// 
// NOTES
// コンストラクタ
// ARGUMENTS
// double k1_
//		文書内頻度調整用パラメータ1。デフォルト0
// double k2_
//		文書内頻度調整用パラメータ2。デフォルト1
// double x_	
// 		文書頻度調整用パラメータ。デフォルト0
// const double lambda
// 		文書長による正規化の調整用パラメータ
// const int y_
// 		IDF計算式切り替え用パラメータ
// 
// RETURN
// なし
// 
// EXCEPTIONS 
// 下位からの例外をそのまま返す
//
ModInvertedNormalizedTfIdfScoreCalculator::ModInvertedNormalizedTfIdfScoreCalculator()
	: ModInvertedTfIdfScoreCalculator()
{
	setParameterLambda(0.25);
}

ModInvertedNormalizedTfIdfScoreCalculator::ModInvertedNormalizedTfIdfScoreCalculator(
	const ModInvertedNormalizedTfIdfScoreCalculator& original)
	: ModInvertedTfIdfScoreCalculator(original)
{
	setParameterLambda(original.getParameterLambda());
}

//
// FUNCTION public
// ModInvertedNormalizedTfIdfScoreCalculator::~ModInvertedNormalizedTfIdfScoreCalculator -- デストラクタ
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
ModInvertedNormalizedTfIdfScoreCalculator::~ModInvertedNormalizedTfIdfScoreCalculator()
{}

//
// FUNCTION public
// ModInvertedNormalizedTfIdfScoreCalculator::duplicate -- 自分自身の複製の作成
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
ModInvertedNormalizedTfIdfScoreCalculator::duplicate() const
{
	return new ModInvertedNormalizedTfIdfScoreCalculator(*this);
}

// 
// FUNCTION public
// ModInvertedNormalizedTfIdfScoreCalculator::firstStep -- TF項の計算
// 
// NOTES
// k1 + k2*tf/((1 - lambda) + lambda*(ld/L))
// 文書長ファイルがセットされていない場合、文書長の項は無視して、スコアを
// 計算する。
//
// ARGUMENTS
// const ModSize tf
//		文書内出現頻度
// const DocumentID docId
//		文書ID
// 
// RETURN
// k1 + k2*tf/((1 - lambda) + lambda*(ld/L)) の結果
//
// EXCEPTIONS 
// 下位からの例外をそのまま返す
//
ModInvertedDocumentScore
ModInvertedNormalizedTfIdfScoreCalculator::firstStep(
	const ModSize tf,
	const DocumentID docId,
	ModBoolean& exist) const
{
	exist = ModTrue;
	
	ModSize documentLength;

#ifdef DEBUG
	++countFirstStep;
#endif

	if (documentLengthFile == 0 || lambda == 0.0) {
		return (double) k1 + k2 * tf;
	}

	if (documentLengthFile->search(docId, documentLength) == ModFalse) {
		// 文書長は記録されているはず
		ModMessage << "documentLength not recorded " << docId << ModEndl;
		exist = ModFalse;
		return 0.0;
	}

 	return (double) k1
		+ k2*(tf/((1 - lambda)*averageDocumentLength + lambda*documentLength));
}

// 
// FUNCTION public
// ModInvertedNormalizedTfIdfScoreCalculator::setParameterLambda -- 文書長による正規化の調整用パラメータのアクセサ関数
// 
// NOTES
// ModInvertedNormalizedTfIdfScoreCalculator::setParameterLambda -- 文書長
// による正規化の調整用パラメータのアクセサ関数。lambdaをセット。
//
// ARGUMENTS
//	const double lambda_
// 		文書長による正規化の調整用パラメータ
// 
// RETURN
// なし
//
// EXCEPIOTNS
// ModInvertedErrorInvalidScoreCalculatorParameter
//
void
ModInvertedNormalizedTfIdfScoreCalculator::setParameterLambda(
	const double lambda_) 
{
	if (lambda_ < 0) {
		// 不正な値 (lambda_ は0以上のdouble)
		ModErrorMessage << "invalid parameter: lambda=" << lambda_
						<< ModEndl;
		ModThrowInvertedFileError(
			ModInvertedErrorInvalidScoreCalculatorParameter);
	}
	this->lambda = lambda_;
	precalculate();
}

// 
// FUNCTION public
// ModInvertedNormalizedTfIdfScoreCalculator::getParameterLambda -- 文書長による正規化の調整用パラメータのアクセサ関数
// 
// NOTES
// ModInvertedNormalizedTfIdfScoreCalculator::getParameterLambda -- 文書長
// による正規化の調整用パラメータのアクセサ関数。lambdaを得る。
//
// ARGUMENTS
// なし
// 
// RETURN
// 文書長による正規化の調整用パラメータのアクセサ関数。lambdaをセット
//
// EXCEPIOTNS
// なし
//
double
ModInvertedNormalizedTfIdfScoreCalculator::getParameterLambda() const
{
	return this->lambda;
}

// 
// FUNCTION public
// ModInvertedNormalizedTfIdfScoreCalculator::setParameter パラメータセット
// 
// NOTES
// NormalizedTfIdfScoreCalculatorにパラメータをセットする。
//  
//
// ARGUMENTS
// ModString& paramerteString
//		パラメータを表す文字列。"k1:k2:x:lambda:y"の形式で渡す。
// 
// RETURN
// なし
// 
// EXCEPTIONS
// ModInvertedErrorInvalidScoreCalculatorParameter
//
void
ModInvertedNormalizedTfIdfScoreCalculator::setParameter(
	const ModString& parameterString)
{
	ModVector<ModString> parameter;

	ModSize size = divideParameter(parameterString, parameter);

	if (size > 5) {
		// パラメータは k1, k2, x, lambda, y の 5 つ
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
		iterator++;
		setParameterK2((double)ModCharTrait::toFloat(*iterator));
	}
	if (size > 2) {
		iterator++;
		setParameterX((double)ModCharTrait::toFloat(*iterator));
	}
	if (size > 3) {
		iterator++;
		setParameterLambda((double)ModCharTrait::toFloat(*iterator));
	}
	if (size > 4) {
		iterator++;
		setParameterY(ModCharTrait::toInt(*iterator));
	}
}

//
// FUNCTION public
// ModInvertedNormalizedTfIdfScoreCalculator::getDescription -- 記述文字列の獲得
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
ModInvertedNormalizedTfIdfScoreCalculator::getDescription(
	ModString& description_,
	const ModBoolean withParameter_) const
{
	description_ = calculatorName;

	if (withParameter_ == ModTrue) {
		ModOstrStream stream;
		stream << ':' << k1 << ':' << k2 << ':' << x << ':' << lambda << ':' << y;
		description_ += stream.getString();
	}
}

//
// Copyright (c) 1999, 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
