// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedRankingScoreCalculator.cpp -- ランキングスコア計算器の実装
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2006, 2009, 2023 Ricoh Company, Ltd.
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

#include "ModString.h"
#include "ModUnicodeString.h"

//#include "ModInvertedRankingScoreCalculator.h"
#include "ModInvertedExternalScoreCalculator.h"
#include "ModInvertedTfIdfScoreCalculator.h"
#include "ModInvertedNormalizedTfIdfScoreCalculator.h"
#include "ModInvertedOkapiTfScoreCalculator.h"
#include "ModInvertedNormalizedOkapiTfScoreCalculator.h"
#include "ModInvertedOkapiTfIdfScoreCalculator.h"
#include "ModInvertedNormalizedOkapiTfIdfScoreCalculator.h"

#include "ModInvertedException.h"
#ifdef  SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedDocumentLengthFile.h"
#else
#include "ModInvertedDocumentLengthFile.h"
#endif

#ifdef DEBUG
//
// VARIABLE
// ModInvertedRankingScoreCalculator::countFirstStep -- 第１ステップの回数
//
/*static*/ int
ModInvertedRankingScoreCalculator::countFirstStep = 0;

//
// VARIABLE
// ModInvertedRankingScoreCalculator::countSecondStep -- 第２ステップの回数
//
/*static*/ int
ModInvertedRankingScoreCalculator::countSecondStep = 0;
#endif


// 
// FUNCTION public
// ModInvertedRankingScoreCalculator::create -- スコア計算器を得る
// 
// NOTES
// スコア計算器記述に対応するスコア計算器を生成して返す。
//
// ARGUMENTS
// const ModString& description
//		スコア計算器記述
// 
// RETURN
// 生成したスコア計算器
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
/*static*/ ModInvertedRankingScoreCalculator*
ModInvertedRankingScoreCalculator::create()
{
	return create(ModString());
}

/*static*/ ModInvertedRankingScoreCalculator*
ModInvertedRankingScoreCalculator::create(const ModString& description)
{
	ModCharString calculatorName;
	ModCharString parameter;
	ModCharString dllname = ModCharString("");
	ModCharString descriptionStr;

	if (description.getLength() == 0) {
		// 空の文字列の場合は引数省略とみなし、NormalizedOkapiTfIdfを作成
		descriptionStr
			= ModInvertedNormalizedOkapiTfIdfScoreCalculator::calculatorName;
	} else {
		descriptionStr = description;
	}

   	ScoreCalculator* calculator = 0;

	ModInvertedRankingScoreCalculator::parse(descriptionStr, 
											 calculatorName, parameter);

	if (calculatorName ==
		ModInvertedOkapiTfIdfScoreCalculator::calculatorName) {
		calculator = new ModInvertedOkapiTfIdfScoreCalculator();
	} else if (calculatorName ==
			   ModInvertedNormalizedOkapiTfIdfScoreCalculator::calculatorName) {
		calculator = new ModInvertedNormalizedOkapiTfIdfScoreCalculator();
	} else if (calculatorName ==
			   ModInvertedOkapiTfScoreCalculator::calculatorName) {
		calculator = new ModInvertedOkapiTfScoreCalculator();
	} else if (calculatorName ==
			   ModInvertedNormalizedOkapiTfScoreCalculator::calculatorName) {
		calculator = new ModInvertedNormalizedOkapiTfScoreCalculator();
	} else if (calculatorName ==
			   ModInvertedTfIdfScoreCalculator::calculatorName) {
		calculator = new ModInvertedTfIdfScoreCalculator();
	} else if (calculatorName ==
			   ModInvertedNormalizedTfIdfScoreCalculator::calculatorName) {
		calculator = new ModInvertedNormalizedTfIdfScoreCalculator();
	} else if (calculatorName ==
			   ModInvertedExternalScoreCalculator::calculatorName) {
		// calculatorName => "ExternalScoreCalculator"
		// コンストラクタにはdll(so)名を含む文字列を引数として渡す
		// コンストラクタ内でdll(so)名と引数を切り離す
		calculator = new ModInvertedExternalScoreCalculator(parameter);

	} else {
		ModErrorMessage << "Create failed: unknown ScoreCalculator: "
						<< description << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorInvalidScoreCalculator);
	}

	if (parameter.getLength() != 0 && calculator != 0) {
		try {
			calculator->setParameter(parameter);
		} catch (ModException& e) {
			// calculatorを破棄
			ModErrorMessage << "Create failed: invalid parameter: "
							<< description << ": " << e << ModEndl;
			delete calculator;
			ModRethrow(e);
		}
	}

	return calculator;
}

/*static*/ ModInvertedRankingScoreCalculator*
ModInvertedRankingScoreCalculator::create(const ModUnicodeString& description)
{
	ModUnicodeString tmp(description);
	return create(ModString(tmp.getString()));
}

// 
// FUNCTION public
// ModInvertedRankingScoreCalculator::ModInvertedRankingScoreCalculator -- デフォルトコンストラクタ
// 
// SYNOPSIS
// inline
// ModInvertedRankingScoreCalculator::ModInvertedRankingScoreCalculator()
// 
// NOTES
// スコア計算器を生成する。
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
ModInvertedRankingScoreCalculator::ModInvertedRankingScoreCalculator()
	: totalDocumentFrequency(1),	// 取りあえずのデフォルト
	  documentLengthFile(0), averageDocumentLength(0), prepareResult(0)
{}

// 
// FUNCTION public
// ModInvertedRankingScoreCalculator::ModInvertedRankingScoreCalculator -- コンストラクタ
// 
// NOTES
// 文書長を利用する(かもしれない)スコア計算器を生成する。
//
// ARGUMENTS
// ModInvertedDocumentLengthFile* documentLengthFile
//	文書長ファイルへのポインタ
// 
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModInvertedRankingScoreCalculator::ModInvertedRankingScoreCalculator(
	LengthFile* documentLengthFile_)
	: documentLengthFile(documentLengthFile_), averageDocumentLength(0),
	  prepareResult(0)
{
	if (documentLengthFile == 0) {
		ModErrorMessage << "documentLengthFile == 0" << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorInternal);
	}
	setAverageDocumentLength(documentLengthFile->getAverageDocumentLength());
}

// 
// FUNCTION public
// ModInvertedRankingScoreCalculator::ModInvertedRankingScoreCalculator -- コピーコンストラクタ
// 
// NOTES
// スコア計算器をコピーとして生成する。
//
// ARGUMENTS
// const ModInvertedRankingScoreCalculator& original
//	コピー元
// 
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModInvertedRankingScoreCalculator::ModInvertedRankingScoreCalculator(
	const ModInvertedRankingScoreCalculator& original)
	: totalDocumentFrequency(original.totalDocumentFrequency),
	  documentLengthFile(original.documentLengthFile),
	  averageDocumentLength(0),
	  prepareResult(original.prepareResult)
{
	  setAverageDocumentLength(original.averageDocumentLength);
}

// 
// FUNCTION public
// ModInvertedRankingScoreCalculator::getTotalDocumentFrequency -- 全文書数を得る。
// 
// NOTES
// 記憶している全文書数を返す。
// 
// ARGUMENTS
// なし
// 
// RETURN
// 全文書数。
//
// EXCEPTIONS
// なし
//
ModSize
ModInvertedRankingScoreCalculator::getTotalDocumentFrequency() const
{
	return totalDocumentFrequency;
}

// 
// FUNCTION public
// ModInvertedRankingScoreCalculator::setDocumentLengthFile -- 文書長ファイルの設定
// 
// NOTES
// 与えられた文書長ファイルを記憶する。
// 
// ARGUMENTS
// ModInvertedDocumentLengthFile* documentLengthFile
//	文書長ファイルへのポインタ
// 
// RETURN
// なし。
// 
// EXCEPTIONS
// なし
//
void
ModInvertedRankingScoreCalculator::setDocumentLengthFile(
	LengthFile* documentLengthFile_)
{
	documentLengthFile = documentLengthFile_;
	setAverageDocumentLength(documentLengthFile->getAverageDocumentLength());
}
	

// 
// FUNCTION public
// ModInvertedRankingScoreCalculator::prepare -- 重みの計算の準備
// 
// NOTES
// 重みの計算の準備
//		log( N / df ) / log( N )
// を計算する。
//
// ARGUMENTS
// const ModSize totalFrequency
//		全文書数（N）
// const ModSize documentFrequency
// 		出現数（df）
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedRankingScoreCalculator::prepare(const ModSize totalFrequency_,
										   const ModSize documentFrequency_)
{
	this->totalDocumentFrequency = totalFrequency_;
	prepareResult = this->secondStep(documentFrequency_, totalFrequency_);
#ifdef	DEBUG
	ModDebugMessage << "N = " << totalFrequency_
					<< " df = " << documentFrequency_
					<< " prepare = " << prepareResult << ModEndl;
#endif	// DEBUG
}

// 
// FUNCTION protected
// ModInvertedRankingScoreCalculator::parse -- パラメータ文字列の解析
// 
// NOTES
// create() に与えられたパラメータ文字列を解析する。
// 計算器名と計算器依存のパラメータに分解するが、その内容が正しいかの検査は
// 行わない。
//
// ARGUMENTS
// ModString& description,
//		パラメータ記述
// ModString& calculatorName,
//		計算器名
// ModString& parameters
//		計算器依存のパラメータ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//

void
ModInvertedRankingScoreCalculator::parse(
	ModString& description,
	ModString& calculatorName,
	ModString& parameters
	)
{
	char* devision = 0;

	ModSize size(description.getLength());
	for (ModSize i(0); i < size; i++) {
		if (description[i] == ':') {
			devision = &description[i];
			break;
		}
	}

	if (devision == 0) {
		parameters.clear();
		calculatorName = description;
	} else {
		parameters = devision + 1;
		description.truncate(devision);
		calculatorName = description;
	}
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
// パラメータセットに成功した場合はModTrue、失敗の場合はModFalse
//
// EXCEPTIONS
// ModInvertedErrorInvalidScoreCalculatorParameter -- 不正なパラメータ
//
ModSize
ModInvertedRankingScoreCalculator::divideParameter(
	const ModString& description,
	ModVector<ModString>& parameter)
{
	ModString parameterString(description);
	ModString tmpString;

	ModSize size = parameterString.getLength();

	if (size == 0) {
		return 0;
	}

	ModSize i(0);
	if (ModCharTrait::isDigit(parameterString[i]) == ModFalse &&
		parameterString[i] != '.' && parameterString[i] != '-') {
		// 数字以外の文字がparameterStringの先頭にある(:,もだめ)
		return ModFalse;
	}

	while (1) {
		if (i >= size) {
			// 渡されたパラメータの終端
			if (parameterString.getLength() > 0) {
				// parameterStringが空でない場合parameterにセット
				parameter.pushBack(parameterString);
			}
			// 文字列きり出し終了
			break;
		}

		if (parameterString[i] == ':') {
			// 区切り文字
			
			// 区切り文字以降の部分をtmpStringにコピー
			tmpString = (&parameterString[i] + 1);

			// parameterStringより区切り文字以降を削除しparameterにセット
			parameterString.truncate(&parameterString[i]);
			parameter.pushBack(parameterString);

			// tempStringにコピーした内容をparameterStringにコピー
			parameterString = tmpString;
			size = parameterString.getLength();
			i = 0;

			if (size == 0) {
				// 区切り文字の後に文字列がないことは不可
				ModThrowInvertedFileError(
					ModInvertedErrorInvalidScoreCalculatorParameter);
			}

		} else { // 区切り文字以外
			if (ModCharTrait::isDigit(parameterString[i]) == ModFalse &&
				parameterString[i] != '.' && parameterString[i] != '-') {
				// 数字以外で'.','-'でもない(':'でもない)
				ModThrowInvertedFileError(
					ModInvertedErrorInvalidScoreCalculatorParameter);
			}
			++i;
		}
	}

	return parameter.getSize();
}

//
// FUNCTION public
// ModInvertedRankingScoreCalculator::getDescription -- 記述文字列の獲得
//
// NOTES
// 自分を記述する文字列を獲得する。
//
// ARGUMENTS
// ModUnicodeString& description_
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
ModInvertedRankingScoreCalculator::getDescription(
	ModUnicodeString& description_,
	const ModBoolean withParameter_) const
{
	ModString tmp;
	getDescription(tmp, withParameter_);
	description_ = ModUnicodeString(tmp);
}

//
//	FUNCTION
//	ModInvertedRankingScoreCalculator::searchDocumentLength
//		-- 文書長を得る
//
ModBoolean
ModInvertedRankingScoreCalculator::searchDocumentLength(ModUInt32 docId_,
														ModSize& length_)
{
	return documentLengthFile->search(docId_, length_);
}

//
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2006, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
