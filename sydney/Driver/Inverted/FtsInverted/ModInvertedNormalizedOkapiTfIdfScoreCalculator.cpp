// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedNormalizedOkapiTfIdfScoreCalculator.cpp -- NormalizedOkapiTfIdf ランキングスコア計算器の実装
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
#include "ModInvertedNormalizedOkapiTfIdfScoreCalculator.h"
#ifdef  SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedDocumentLengthFile.h"
#else
#include "ModInvertedDocumentLengthFile.h"
#endif
#include "ModInvertedException.h"

//
// CONST
// ModInvertedNormalizedOkapiTfIdfScoreCalculator::calculatorName -- 計算器名
//
// NOTES
// スコア計算器の名称
//
/*static*/ const char
ModInvertedNormalizedOkapiTfIdfScoreCalculator::calculatorName[]
	= "NormalizedOkapiTfIdf";

//
// FUNCTION public
// ModInvertedNormalizedOkapiTfIdfScoreCalculator::ModInvertedNormalizedOkapiTfIdfScoreCalculator -- コンストラクタ
// 
// NOTES
// コンストラクタ
//
// ARGUMENTS
// const ModInvertedNormalizedOkapiTfIdfScoreCalculator& original
// 		コピーもと
// 
// RETURN
// なし
// 
// EXCEPTIONS 
// 下位からの例外をそのまま返す
//
ModInvertedNormalizedOkapiTfIdfScoreCalculator::ModInvertedNormalizedOkapiTfIdfScoreCalculator()
	: ModInvertedOkapiTfIdfScoreCalculator()
{
	setParameterLambda(0.25);
}

ModInvertedNormalizedOkapiTfIdfScoreCalculator::ModInvertedNormalizedOkapiTfIdfScoreCalculator(
	const ModInvertedNormalizedOkapiTfIdfScoreCalculator& original)
	: ModInvertedOkapiTfIdfScoreCalculator(original)
{
	setParameterLambda(original.getParameterLambda());
}

//
// FUNCTION public
// ModInvertedNormalizedOkapiTfIdfScoreCalculator::~ModInvertedNormalizedOkapiTfIdfScoreCalculator -- デストラクタ
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
ModInvertedNormalizedOkapiTfIdfScoreCalculator::~ModInvertedNormalizedOkapiTfIdfScoreCalculator()
{}

//
// FUNCTION public
// ModInvertedNormalizedOkapiTfIdfScoreCalculator::duplicate -- 自分自身の複製
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
ModInvertedNormalizedOkapiTfIdfScoreCalculator::duplicate() const
{
	return new ModInvertedNormalizedOkapiTfIdfScoreCalculator(*this);
}

// 
// FUNCTION public
// ModInvertedNormalizedOkapiTfIdfScoreCalculator::firstStep -- TF項の計算
// 
// NOTES
// tf /(k*((1 - lambda) + lambda*(ld/L)) + tf)
// 文書長ファイルがセットされていない場合、文書長の項は無視して計算する。
//
// ARGUMENTS
// const ModSize tf
//		文書内出現頻度
// const DocumentID docId
//		文書ID
// 
// RETURN
// TF項の計算結果
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//

ModInvertedDocumentScore
ModInvertedNormalizedOkapiTfIdfScoreCalculator::firstStep(
	const ModSize tf,
	const DocumentID docId,
	ModBoolean& exist) const
{
	exist = ModTrue;
	
	ModSize documentLength;

#ifdef DEBUG
	++countFirstStep;
#endif

	if (documentLengthFile == 0) {
		return DocumentScore(tf / (k + tf));
	}

	if (documentLengthFile->search(docId, documentLength) == ModFalse) {
		// 文書長は記録されているはず
		ModMessage << "documentLength not recorded " << docId << ModEndl;
		exist = ModFalse;
		return 0.0;
	}
 	return DocumentScore(tf/((pre1 + pre2*documentLength) + tf));
}

// 
// FUNCTION public
// ModInvertedNormalizedOkapiTfIdfScoreCalculator::setParameterLambda -- パラメータのアクセサ関数
// 
// NOTES
// 文書長による正規化の調整用パラメータ lambda のアクセサ関数。lambdaをセット。
//
// ARGUMENTS
// const double lambda_
// 		文書長による正規化の調整用パラメータ
// 
// RETURN
// なし
//
// EXCEPIOTNS
// ModInvertedErrorInvalidScoreCalculatorParameter
//
void
ModInvertedNormalizedOkapiTfIdfScoreCalculator::setParameterLambda(
	const double lambda_) 
{
	if (lambda_ < 0 || 1 < lambda_) {
		// 不正な値 (lambda は 0 以上 1 以下のdouble)
		ModErrorMessage << "invalid parameter: lambda=" << lambda_
						<< ModEndl;
		ModThrowInvertedFileError(
			ModInvertedErrorInvalidScoreCalculatorParameter);
	}
	lambda = lambda_;
	precalculate();
}

// 
// FUNCTION public
// ModInvertedNormalizedOkapiTfIdfScoreCalculator::getParameterLambda -- パラメータのアクセサ関数
// 
// NOTES
// 文書長による正規化の調整用パラメータ lambda のアクセサ関数。lambda を得る。
//
// ARGUMENTS
// なし
// 
// RETURN
// パラメータ
//
// EXCEPIOTNS
// なし
//
double
ModInvertedNormalizedOkapiTfIdfScoreCalculator::getParameterLambda() const
{
	return this->lambda;
}

// 
// FUNCTION public
// ModInvertedNormalizedOkapiTfIdfScoreCalculator::setParameter パラメータセット
// 
// NOTES
// NormalizedOkapiTfIdfScoreCalculatorにパラメータをセットする。
//  
// ARGUMENTS
// ModString& paramerteString
//		パラメータを表す文字列。"k:x:lambda:y:a:s"の形式で渡す。
// 
// RETURN
// なし
//
// EXCEPTIONS
// ModInvertedErrorInvalidScoreCalculatorParameter
//
void
ModInvertedNormalizedOkapiTfIdfScoreCalculator::setParameter(
	const ModString& parameterString)
{
	ModVector<ModString> parameter;

	ModSize size = divideParameter(parameterString, parameter);

	if (size > 7) {
		// パラメータは k, x, lambda, y, a, s, q の 7 つ
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
		setParameterLambda((double)ModCharTrait::toFloat(*iterator));
	}
	if (size > 3) {
		++iterator;
		setParameterY((int)ModCharTrait::toInt(*iterator));
	}
	if (size > 4) {
		++iterator;
		setParameterQ((double)ModCharTrait::toFloat(*iterator));
	}
	if (size > 5) {
		++iterator;
		setParameterA((double)ModCharTrait::toFloat(*iterator));
	}
	if (size > 6) {
		++iterator;
		setParameterS((double)ModCharTrait::toFloat(*iterator));
	}
}

//
// FUNCTION public
// ModInvertedNormalizedOkapiTfIdfScoreCalculator::getDescription -- 記述文字列の獲得
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
ModInvertedNormalizedOkapiTfIdfScoreCalculator::getDescription(
	ModString& description_,
	const ModBoolean withParameter_) const
{
	description_ = calculatorName;

	if (withParameter_ == ModTrue) {
		ModOstrStream stream;
		stream << ':' << k << ':' << x << ':' << lambda << ':' << y;
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
// FUNCTION public
// ModInvertedNormalizedOkapiTfIdfScoreCalculator::precalulate() -- 前計算
// 
// NOTES
// スコア計算TFに関するパラメータに基づく前計算を行なう。
// 
// ARGUMENTS
// なし
// 
// RETURN
// なし。
// 
// EXCEPTIONS
// なし
//
void
ModInvertedNormalizedOkapiTfIdfScoreCalculator::precalculate()
{
	pre1 = k*(1.0 - lambda);
	if (averageDocumentLength != 0) {
		pre2 = k*lambda/averageDocumentLength;
	} else {
		pre2 = 0.0;
	}
}

//
// Copyright (c) 1999, 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
