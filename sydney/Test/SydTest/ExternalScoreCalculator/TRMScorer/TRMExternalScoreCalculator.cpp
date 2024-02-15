//
// 外部スコア計算器サンプルソースコード
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "TRMExternalScoreCalculator.h"
#include "math.h"

//////////////////////////////
// 外部スコア計算器 DLL 本体
//

class SRCBScoreCalculator : public TRMExternalScoreCalculator
{
private:
	// constant
	double mu;
	double K1;
	double K3;

	TRMeisterScoreParameterId TFVarParamId[6];
	TRMeisterScoreParameterId DFVarParamId[3];
public:
	SRCBScoreCalculator(const char* pParameter_);
	~SRCBScoreCalculator(){}
	double tfTerm(const TRMeisterScoreParameterValue *varParam);
	double dfTerm(const TRMeisterScoreParameterValue *varParam);
	TRMeisterScoreParameterId *getTfParameterList(){return TFVarParamId;}
	TRMeisterScoreParameterId *getDfParameterList(){return DFVarParamId;}
};
// helper func.
double getConstant(const char *&p)
{
	char buf[128];
	char *q = buf;
	while(*q++ = *p++)
	{
		if( *(q - 1) == ':')
		{
			*(q - 1) = 0;
			break;
		}
	}
	return atof(buf);
}

// 文書スコア計算器オブジェクトの作成
TRM_FUNC void *
getScoreCalculator(const char* parameter_)
{
	return (void*)new SRCBScoreCalculator(parameter_);
}


////////////////////
// DLL main body
//
BOOL WINAPI DllMain(HINSTANCE hInstance,DWORD reason,LPVOID lpReserved)
{
	return TRUE;
}


SRCBScoreCalculator::SRCBScoreCalculator(const char *paramter_)
{
	// parameterStringをparseし数値列を取り出す
	// フォーマットは、数値列:数値列:...
	const char *p = paramter_;
	mu  = getConstant(p);
	K1 = getConstant(p);
	K3 = getConstant(p);

	// TF variable list
	TFVarParamId[0] = TRM_TermFrequency_UINT32;
	TFVarParamId[1] = TRM_TotalTermFrequency_UINT64;
	TFVarParamId[2] = TRM_DocumentLength_UINT32;
	TFVarParamId[3] = TRM_TotalDocumentLength_UINT64;
	TFVarParamId[4] = TRM_QueryTermFrequency_UINT32;
	TFVarParamId[5] = static_cast<TRMeisterScoreParameterId>(0);
	// DF variable list
	DFVarParamId[0] = TRM_DocumentFrequency_UINT32;
	DFVarParamId[1] = TRM_TotalDocumentFrequency_UINT32;
	DFVarParamId[2] = static_cast<TRMeisterScoreParameterId>(0);
}

double SRCBScoreCalculator::tfTerm(const TRMeisterScoreParameterValue *varParam)
{
	UINT32 tf                  = varParam[0].uint32_;
	UINT32 totalTermFrequency  = varParam[1].uint32_;
	UINT32 documentLength      = varParam[2].uint32_;
	UINT32 totalDocumentLength = varParam[3].uint32_;
	UINT32 queryTF             = varParam[4].uint32_;

	double A = ((tf + mu)*totalTermFrequency)/totalDocumentLength;
	double B = documentLength + mu;
	double termTF = A/B;
	return (((K1 + 1)*termTF)/(K1 + termTF))*(((K3+1)*queryTF)/(K3 + queryTF));
}
double SRCBScoreCalculator::dfTerm(const TRMeisterScoreParameterValue *varParam)
{
	UINT32 DocumentFrequency      = varParam[0].uint32_;
	UINT32 totalDocumentFrequency = varParam[1].uint32_;
	return log((totalDocumentFrequency - DocumentFrequency + 0.5)/(DocumentFrequency + 0.5));
}

//
// Copyright (c) 1999, 2000, 2001, 2002, 2005, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
