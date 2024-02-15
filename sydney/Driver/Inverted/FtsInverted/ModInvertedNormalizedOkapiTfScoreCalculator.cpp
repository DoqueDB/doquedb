// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedNormalizedOkapiTfScoreCalculator.cpp -- NormalizedOkapiTf ランキングスコア計算器の実装
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
#include "ModInvertedNormalizedOkapiTfScoreCalculator.h"
#ifdef  SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedDocumentLengthFile.h"
#else
#include "ModInvertedDocumentLengthFile.h"
#endif
#include "ModInvertedException.h"

//
// CONST
// ModInvertedNormalizedOkapiTfScoreCalculator::calculatorName -- 計算器名
//
// NOTES
// スコア計算器の名称
//
/*static*/ const char
ModInvertedNormalizedOkapiTfScoreCalculator::calculatorName[]
	= "NormalizedOkapiTf";

//
// FUNCTION public
// ModInvertedNormalizedOkapiTfScoreCalculator::ModInvertedNormalizedOkapiTfScoreCalculator -- コンストラクタ
// 
// NOTES
// コンストラクタ
//
// ARGUMENTS
// const ModInvertedNormalizedOkapiTfScoreCalculator& original
//		コピーもと
//
// RETURN
// なし
// 
// EXCEPTIONS
// ModInvertedErrorInvalidScoreCalculatorParameter -- 不正なパラメータ
//
ModInvertedNormalizedOkapiTfScoreCalculator::ModInvertedNormalizedOkapiTfScoreCalculator()
	: ModInvertedOkapiTfScoreCalculator()
{
	setParameterLambda(0.25);
}

ModInvertedNormalizedOkapiTfScoreCalculator::ModInvertedNormalizedOkapiTfScoreCalculator(
	const ModInvertedNormalizedOkapiTfScoreCalculator& original)
	: ModInvertedOkapiTfScoreCalculator(original)
{
	setParameterLambda(original.getParameterLambda());
}

//
// FUNCTION public
// ModInvertedNormalizedOkapiTfScoreCalculator::duplicate -- 自分自身の複製の作成
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
ModInvertedNormalizedOkapiTfScoreCalculator::duplicate() const
{
	return new ModInvertedNormalizedOkapiTfScoreCalculator(*this);
}

// 
// FUNCTION public
// ModInvertedNormalizedOkapiTfScoreCalculator::firstStep -- tf / ( k + tf ) の計算
// 
// NOTES
// tf / (k * ((1-lambda) + lambda * (ld / L) ))
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
// tf / (k * ((1-lambda) + lambda * (ld / L) ))の結果
//
// EXCEPTIONS
// なし
//
ModInvertedDocumentScore
ModInvertedNormalizedOkapiTfScoreCalculator::firstStep(
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
		return DocumentScore(tf / (k + tf));
	}

	if (documentLengthFile->search(docId, documentLength) == ModFalse) {
		// 文書長は記録されているはず
		ModMessage << "documentLength not recorded " << docId << ModEndl;
		exist = ModFalse;
		return 0.0;
	}

 	return DocumentScore(tf/(pre1 + pre2*documentLength + tf));
}

// 
// FUNCTION public
// ModInvertedNormalizedOkapiTfScoreCalculator::setParameterLambda -- 文書長による正規化の調整用パラメータのアクセサ関数
// 
// NOTES
// ModInvertedNormalizedOkapiTfScoreCalculator::setParameterLambda -- 文書長
// による正規化の調整用パラメータのアクセサ関数。lambdaをセット。
// ARGUMENTS
//	const double lambda_
// 		文書長による正規化の調整用パラメータ
// 
// RETURN
//	なし
//
// EXCEPIOTNS
// ModInvertedErrorInvalidScoreCalculatorParameter -- 不正なパラメータ
//
void
ModInvertedNormalizedOkapiTfScoreCalculator::setParameterLambda(
	const double lambda_) 
{
	if (lambda_ < 0) {
		// 不正な値 (lambda は0以上のdouble)
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
// ModInvertedNormalizedOkapiTfScoreCalculator::getParameterLambda -- 文書長による正規化の調整用パラメータのアクセサ関数
// 
// NOTES
// ModInvertedNormalizedOkapiTfScoreCalculator::getParameterLambda -- 文書長
// による正規化の調整用パラメータのアクセサ関数。lambdaを得る。
//
// ARGUMENTS
//	なし
// 
// RETURN
// lambda
//
// EXCEPIOTNS
// なし
//
double
ModInvertedNormalizedOkapiTfScoreCalculator::getParameterLambda() const
{
	return this->lambda;
}

// 
// FUNCTION public
// ModInvertedNormalizedOkapiTfScoreCalculator::setParameter パラメータセット
// 
// NOTES
// NormalizedOkapiTfScoreCalculatorにパラメータをセットする。
// "k:lambda"の形式で渡す。
//  
// ARGUMENTS
// ModString& paramerteString
//		パラメータを表す文字列。
// 
// RETURN
// なし
//
// EXCEPTIONS
// ModInvertedErrorInvalidScoreCalculatorParameter -- 不正なパラメータ
//
void
ModInvertedNormalizedOkapiTfScoreCalculator::setParameter(
	const ModString& parameterString)
{
	ModVector<ModString> parameter;

	ModSize size = divideParameter(parameterString, parameter);

	if (size > 2) {
		// パラメータは k, lambda の 2 つ
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
		setParameterLambda((double)ModCharTrait::toFloat(*iterator));
	}
}

//
// FUNCTION public
// ModInvertedNormalizedOkapiTfScoreCalculator::getDescription -- 記述文字列の獲得
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
ModInvertedNormalizedOkapiTfScoreCalculator::getDescription(
	ModString& description_,
	const ModBoolean withParameter_) const
{
	description_ = calculatorName;

	if (withParameter_ == ModTrue) {
		ModOstrStream stream;
		stream << ':' << k << ':' << lambda;
		description_ += stream.getString();
	}
}

// 
// FUNCTION public
// ModInvertedNormalizedOkapiTfScoreCalculator::precalulate() -- 前計算
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
ModInvertedNormalizedOkapiTfScoreCalculator::precalculate()
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
