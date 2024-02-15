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

#if defined(WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#include <stdlib.h>
#include <math.h>
#include "Inverted/TRMExternalScoreCalculator.h"

//////////////////////////////
// 外部スコア計算器 DLL 本体
//

class NormalizedOkapiTfScoreCalculator : public TRMExternalScoreCalculator
{
private:
	// constant passed through SQL statement.
	double k;
	double lambda;	
	TRMeisterScoreParameterId TFVarParamId[4];
public:
	NormalizedOkapiTfScoreCalculator(const char* pParameter_);
	~NormalizedOkapiTfScoreCalculator(){}
	double tfTerm(const TRMeisterScoreParameterValue *varParam);
	double dfTerm(const TRMeisterScoreParameterValue *varParam);
	TRMeisterScoreParameterId *getTfParameterList(){return TFVarParamId;}
	TRMeisterScoreParameterId *getDfParameterList(){return 0;}
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
	return (void*)new NormalizedOkapiTfScoreCalculator(parameter_);
}


////////////////////
// DLL main body
//
#if defined(WIN32)
BOOL WINAPI DllMain(HINSTANCE hInstance,DWORD reason,LPVOID lpReserved)
{
	return TRUE;
}
#endif

NormalizedOkapiTfScoreCalculator::NormalizedOkapiTfScoreCalculator(const char *paramter_)
{
	// parameterStringをparseし数値列を取り出す
	// フォーマットは、数値列:数値列:...
	const char *p = paramter_;
	if(*p)
	{
		k = getConstant(p);
		lambda = getConstant(p);
	}
	else
	{
		k = 1.0;
		lambda = 0.25;
	}


	// TF variable list
	TFVarParamId[0] = TRM_TermFrequency_UINT32;
	TFVarParamId[1] = TRM_DocumentLength_UINT32;
	TFVarParamId[2] = TRM_AverageDocumentLength_UINT32;
	TFVarParamId[3] = static_cast<TRMeisterScoreParameterId>(0);
}

double NormalizedOkapiTfScoreCalculator::tfTerm(const TRMeisterScoreParameterValue *varParam)
{
	TRM_UINT32 tf                    = varParam[0].uint32_;
	TRM_UINT32 documentLength        = varParam[1].uint32_;
	TRM_UINT32 averageDocumentLength = varParam[2].uint32_;
	double pre1,pre2;

	if ( lambda == 0.0) {
		return double(tf / (k + tf));
	}
	pre1 = k*(1.0 - lambda);
	if (averageDocumentLength != 0) {
		pre2 = k*lambda/averageDocumentLength;
	} else {
		pre2 = 0.0;
	}

 	return double(tf/(pre1 + pre2*documentLength + tf));
}
double NormalizedOkapiTfScoreCalculator::dfTerm(const TRMeisterScoreParameterValue *varParam)
{
	return 1.0;
}

//
// Copyright (c) 1999, 2000, 2001, 2002, 2005, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
