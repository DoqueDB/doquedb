// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedExternalScoreCalculator.cpp -- 外部ランキングスコア計算器の実装
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2006, 2007, 2010, 2023 Ricoh Company, Ltd.
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

#include "ModAssert.h"
#include "ModOsDriver.h"
#include "ModOstrStream.h"
#include "ModInvertedQueryNode.h"
#include "ModInvertedExternalScoreCalculator.h"
#ifdef  SYD_INVERTED // SYDNEY 対応
#include "Inverted/ModInvertedDocumentLengthFile.h"
#else
#include "ModInvertedDocumentLengthFile.h"
#endif
#include "ModInvertedException.h"

#include "Os/Library.h"
#include "Os/Unicode.h"

_SYDNEY_USING
//
//
// CONST
// ModInvertedExternalScoreCalculator::calculatorName -- 計算器名
//
// NOTES
// スコア計算器の名称
//
/*static*/ const char
ModInvertedExternalScoreCalculator::calculatorName[]
	= "ExternalScoreCalculator";

// 暫定処理
typedef ModInvertedDocumentScore			DocumentScore;
//
ModInvertedExternalScoreCalculator::
ModInvertedExternalScoreCalculator(ModString &parameterString_)
{
	ModString dllname;
	ModString argString;
	// ここで引数parameterString_をparseする
	// parameterString_は、"dllname:引数"の形式
	char* division = 0;
	ModSize size(parameterString_.getLength());
	for (ModSize i(0),j(0); i < size; i++) {
		if (parameterString_[i] == ':') {
			division = &parameterString_[i];
			break;
		}
	}
	if(division == 0 ){
	// 外部スコア計算器のみ指定
		dllname = parameterString_;
	}else{
	// dll(so)名とパラメータ文字列の指定
		argString = division + 1;
		parameterString_.truncate(division);
		dllname = parameterString_;
	}

	ModUnicodeString dllname_ = dllname.getString();
	// ライブラリをロードする
	Os::Library::load(dllname_);

	try
	{
		setParameters(argString,dllname_);
	}
	catch (ModException& e)
	{
		ModErrorMessage << "ExternalScoreCalculator constructor failed: " << e << ModEndl;
		ModRethrow(e);
	}
	
}

void
ModInvertedExternalScoreCalculator::setParameters(ModString &argString_,ModUnicodeString &dllname_) 
{
	parameterString = argString_;
	_SYDNEY::Inverted::GetScoreCalculator getScoreCalculator = 
				(_SYDNEY::Inverted::GetScoreCalculator)Os::Library::getFunction(dllname_,
											ModUnicodeString("getScoreCalculator"));
	dllName = dllname_;
	if(getScoreCalculator == 0)
	{
		ModThrowInvertedFileError(
		ModInvertedErrorInvalidScoreCalculatorParameter);
	}
	calculator = (_SYDNEY::Inverted::TRMExternalScoreCalculator *)getScoreCalculator(argString_);
	extendedFirstStep = false;
	varParamTf = varParamDf = 0;

	varParamIdTf = calculator->getTfParameterList();
	if(varParamIdTf)
	{
		varParamTf = setVarParameter(varParamIdTf);
	}
	varParamIdDf = calculator->getDfParameterList();
	if(varParamIdDf)
	{
		varParamDf = setVarParameter(varParamIdDf);
	}
}
ModInvertedExternalScoreCalculator::
ModInvertedExternalScoreCalculator(const ModInvertedExternalScoreCalculator& original)
	: ModInvertedRankingScoreCalculator(original)
{
	totalTermFrequency = 0;
	dllName = original.getDllName();
	parameterString = const_cast<ModInvertedExternalScoreCalculator&>(original).getParameterString();
	setParameters(const_cast<ModInvertedExternalScoreCalculator&>(original).getParameterString(),dllName);
}

ModInvertedExternalScoreCalculator::
~ModInvertedExternalScoreCalculator()
{
	delete varParamTf;
	delete varParamDf;
}

void
ModInvertedExternalScoreCalculator::getDescription(
	ModString& description_,
	const ModBoolean withParameter_) const
{
	description_ = calculatorName;

	if (withParameter_ == ModTrue) {
		ModOstrStream stream;
		; ModAssert(dllName.getLength() > 0);
		stream << ':' << dllName;
		if (parameterString.getLength() > 0)
		{
			stream << ':' << parameterString;
		}
		description_ += stream.getString();
	}
}

_SYDNEY::Inverted::TRMeisterScoreParameterValue *
ModInvertedExternalScoreCalculator::setVarParameter(
	const _SYDNEY::Inverted::TRMeisterScoreParameterId *varParamId
	) 
{
	ModSize size;
	const _SYDNEY::Inverted::TRMeisterScoreParameterId *p = varParamId;
	while(*p)
	{
		if(	*p == _SYDNEY::Inverted::TRM_TotalTermFrequency_UINT32 ||
			*p == _SYDNEY::Inverted::TRM_TotalTermFrequency_UINT64 ||
			*p == _SYDNEY::Inverted::TRM_QueryTermFrequency_UINT32)
		{
			extendedFirstStep = true;
			// このループでは、パラメータ数も検査しているので
			// breakしてはいけない
		}
		++p;
	}
	// パラメータ全体の領域確保
	return new _SYDNEY::Inverted::TRMeisterScoreParameterValue[size = (p - varParamId)];
}
void
ModInvertedExternalScoreCalculator::setVarParamTf(
		_SYDNEY::Inverted::TRMeisterScoreParameterValue * varParamTf,
		ModSize tf,
		DocumentID docId,
		ModSize documentLength) const
{
	_SYDNEY::Inverted::TRMeisterScoreParameterId *id =  varParamIdTf ; 
	if(id)
	{
		for(; *id ; id++)
		{
			int i = id - varParamIdTf;
			switch(*id)
			{
			case	_SYDNEY::Inverted::TRM_TermFrequency_UINT32:
				varParamTf[i].uint32_ = tf;
				break;
			case	_SYDNEY::Inverted::TRM_TotalTermFrequency_UINT32:
				varParamTf[i].uint32_ = ModSize(totalTermFrequency);
				break;
			case	_SYDNEY::Inverted::TRM_TotalTermFrequency_UINT64:
				varParamTf[i].uint64_ = totalTermFrequency;
				break;
			case	_SYDNEY::Inverted::TRM_QueryTermFrequency_UINT32:
				varParamTf[i].uint32_ = queryTermFrequency;
				break;
			case	_SYDNEY::Inverted::TRM_DocumentLength_UINT32:
				if(int(documentLength) < 0)
				{
					documentLengthFile->search(docId, documentLength);
				}		
				varParamTf[i].uint32_ = documentLength;
				break;
			case	_SYDNEY::Inverted::TRM_TotalDocumentLength_UINT32:
				varParamTf[i].uint32_ = ModSize(totalDocumentLength);// word索引の場合は、データベース中の全単語数
				break;											// さもなければ全文字数
			case	_SYDNEY::Inverted::TRM_TotalDocumentLength_UINT64:
				varParamTf[i].uint64_ = totalDocumentLength;	// word索引の場合は、データベース中の全単語数
				break;											// さもなければ全文字数
			case	_SYDNEY::Inverted::TRM_AverageDocumentLength_UINT32:
				varParamTf[i].uint32_ = averageDocumentLength;// word索引の場合は、データベース中の全単語数
				break;
			case	_SYDNEY::Inverted::TRM_QueryTermSurface_STRING:
				// not implemented
				break;
			default:
				break;
			}
		}
	}
}
DocumentScore
ModInvertedExternalScoreCalculator::firstStep(
	const ModSize tf,
	const DocumentID docId,
	ModSize documentLength) const
{

	setVarParamTf(varParamTf,tf,docId,documentLength);
	return calculator->tfTerm(varParamTf);
}
DocumentScore
ModInvertedExternalScoreCalculator::firstStep(
	const ModSize tf,
	const DocumentID docId,
	ModBoolean& exist) const
{
	ModSize documentLength;
	exist = ModTrue;
	if (documentLengthFile->search(docId, documentLength) == ModFalse) {
		// 文書長は記録されているはず
		ModMessage << "documentLength not recorded " << docId << ModEndl;
		exist = ModFalse;
		return DocumentScore(0.0);
	}
	if(extendedFirstStep == false)
		return DocumentScore(firstStep(tf,docId,documentLength));
	else
	{
		// extendedモードでは、tfを参照するので保存しておく。
		// extendedモードでは、...
		const_cast<ModInvertedVector<ModInvertedTermFrequency> &>(vecTF).pushBack(tf);
		const_cast<ModUInt64&>(totalTermFrequency) += tf;
		return DocumentScore(0.0);
	}
}

DocumentScore
ModInvertedExternalScoreCalculator::firstStepEx(
	const ModSize i,
	const DocumentID docId) const
{
	return firstStep(vecTF.at(i),docId,ModSize(-1));
}

DocumentScore
ModInvertedExternalScoreCalculator::secondStep(const ModSize documentFrequency,
const ModSize totalDocumentFrequency) const
{
	_SYDNEY::Inverted::TRMeisterScoreParameterId *id =  varParamIdDf;
	if(id)
	{	
		for(; *id ; id++)
		{
			int i = id - varParamIdDf;
			switch(*id)
			{
			case	_SYDNEY::Inverted::TRM_DocumentFrequency_UINT32:			// document lengthとfrequencyは違う
				varParamDf[i].uint32_ = documentFrequency;
				break;
			case	_SYDNEY::Inverted::TRM_TotalDocumentFrequency_UINT32:
				varParamDf[i].uint32_ = totalDocumentFrequency;
				break;
			case	_SYDNEY::Inverted::TRM_TotalDocumentFrequency_UINT64:	// この版では、
													// 無意味,基底クラスのsecondStep()の引数の型がModSize！
													// しかし、当分は32bitで大丈夫。将来のためのコードと考える
				varParamDf[i].uint64_ = totalDocumentFrequency;
				break;
			case	_SYDNEY::Inverted::TRM_TotalDocumentLength_UINT32:
				varParamDf[i].uint32_ = ModSize(totalDocumentLength);// word索引の場合は、データベース中の全単語数
				break;											// さもなければ全文字数
			case	_SYDNEY::Inverted::TRM_TotalDocumentLength_UINT64:
				varParamDf[i].uint64_ = totalDocumentLength;	// word索引の場合は、データベース中の全単語数
				break;											// さもなければ全文字数
			default:
				break;
			}
		}
	}
	return calculator->dfTerm(varParamDf);
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2006, 2007, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
