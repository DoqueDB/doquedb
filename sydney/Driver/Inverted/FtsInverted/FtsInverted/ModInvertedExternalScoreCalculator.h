// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedExternalScoreCalculator.h -- ModInvertedExternalScoreCalculator 外部ランキングスコア計算器のインターフェース
// 
// Copyright (c) 1999, 2000, 2002, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedExternalScoreCalculator_h_
#define __ModInvertedExternalScoreCalculator_h_

#include "ModInvertedRankingScoreCalculator.h"

#define USE_TRM_EXTERNALSCORECALCULATOR 
#if defined(OS_WINDOWSNT4_0) || defined(OS_WINDOWS98) || defined(OS_WINDOWS95)
#include <windows.h>
#endif
#include "Inverted/TRMExternalScoreCalculator.h"


class ModInvertedExternalScoreCalculator 
	: public ModInvertedRankingScoreCalculator
{
	friend class ModInvertedRankingScoreCalculator;
private:
	ModString parameterString;
	_SYDNEY::Inverted::TRMeisterScoreParameterId *varParamIdTf;
	_SYDNEY::Inverted::TRMeisterScoreParameterId *varParamIdDf;
	_SYDNEY::Inverted::TRMeisterScoreParameterValue *varParamTf;
	_SYDNEY::Inverted::TRMeisterScoreParameterValue *varParamDf;
	ModUnicodeString dllName;
	bool extendedFirstStep;
    _SYDNEY::Inverted::TRMExternalScoreCalculator *calculator;
	static const char calculatorName[];
	ModInvertedVector<ModInvertedTermFrequency> vecTF;
public:
	typedef ModInvertedRankingScoreCalculator	ScoreCalculator;
	typedef ModInvertedDocumentLengthFile		LengthFile;
	typedef ModInvertedDocumentScore			DocumentScore;
	typedef ModInvertedDocumentID				DocumentID;

	ModInvertedExternalScoreCalculator(){}
	ModInvertedExternalScoreCalculator(ModString &parameterString_);
	ModInvertedExternalScoreCalculator(const ModInvertedExternalScoreCalculator &);
	virtual ~ModInvertedExternalScoreCalculator();
	ModUnicodeString getDllName() const {return dllName;}
	void setParameters(ModString &argString_,ModUnicodeString &dllname_) ;
	_SYDNEY::Inverted::TRMeisterScoreParameterValue * 
	setVarParameter(const _SYDNEY::Inverted::TRMeisterScoreParameterId *paramId);
	void setVarParamTf(	_SYDNEY::Inverted::TRMeisterScoreParameterValue * varParamTf,
						ModSize tf,
						DocumentID docid,
						ModSize documentLength = ModSize(-1)) const;

	DocumentScore firstStep(const ModSize tf,
							const DocumentID docId,
							ModSize documentLength) const;

	DocumentScore firstStep(const ModSize tf,
							const DocumentID ID,
							ModBoolean& exist) const;

	DocumentScore firstStepEx(
							const ModSize i,
							const DocumentID ID
							) const;

	DocumentScore secondStep(const ModSize df,
							 const ModSize totalDocument) const;

	bool isExtendedFirstStep()
	{
		return extendedFirstStep;
	}
	void
	prepareEx(const ModUInt64 totalTermFrequency_,
			  const ModUInt64 totalDocumentLength_,
			  const ModUInt32 queryTermFrequency_)
	{
		this->totalTermFrequency  = totalTermFrequency_;
		this->totalDocumentLength = totalDocumentLength_;
        this->queryTermFrequency = queryTermFrequency_;
	}

	// 記述文字列の取得
	// パラメータのアクセサ関数
	void setParameter(const ModString&)
	{
	// nothing to do
	// パラメータの解析は、外部DLLで行うので、Sydneyでは何もしない。
	// パラメータは、ModInvertedExternalScoreCalculatorのコンストラクタ
	// から外部DLLに渡される
	}
	// 自分自身の複製
	ScoreCalculator* duplicate() const {return new ModInvertedExternalScoreCalculator(*this);}
	virtual ModUInt64 getTotalTermFrequency(){ return totalTermFrequency;}
	void getDescription(ModString&, const ModBoolean = ModTrue) const;
	ModString &getParameterString(){ return parameterString;}
protected:
};

#endif // __ModInvertedExternalScoreCalculator_h_

//
// Copyright (c) 1999, 2000, 2002, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
