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

class OkapiTfIdf : public TRMExternalScoreCalculator
{
private:
	// constant
	double k;
	int y;
	double x;					// 文書頻度調整用パラメータ
	double a;					// 文書頻度調整用パラメータ
	double s;					// 文書頻度調整用パラメータ
	double q;					// 文書頻度調整用パラメータ

	TRMeisterScoreParameterId TFVarParamId[2];
	TRMeisterScoreParameterId DFVarParamId[3];
public:
	OkapiTfIdf(const char* pParameter_);
	~OkapiTfIdf(){}
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
	return (void*)new OkapiTfIdf(parameter_);
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

OkapiTfIdf::OkapiTfIdf(const char *paramter_)
{
	// parameterStringをparseし数値列を取り出す
	// フォーマットは、数値列:数値列:...
	const char *p = paramter_;
	if(*p)
	{
		k = getConstant(p);
		x = getConstant(p);
		a = getConstant(p);
		s = getConstant(p);
		q = getConstant(p);
	}
	else
	{
		k = 1.0;
		x = 0.2;
		a = 0.0;
		s = 1.0;
		q = 0.0;
		y = 1;
	}
	// TF variable list
	TFVarParamId[0] = TRM_TermFrequency_UINT32;
	TFVarParamId[1] = static_cast<TRMeisterScoreParameterId>(0);
	// DF variable list
	DFVarParamId[0] = TRM_DocumentFrequency_UINT32;
	DFVarParamId[1] = TRM_TotalDocumentFrequency_UINT32;
	DFVarParamId[2] = static_cast<TRMeisterScoreParameterId>(0);
}


double OkapiTfIdf::tfTerm(const TRMeisterScoreParameterValue *varParam)
{
	TRM_UINT32 tf      = varParam[0].uint32_;
	return double(tf / (k + tf));
}
double OkapiTfIdf::dfTerm(const TRMeisterScoreParameterValue *varParam)
{
	TRM_UINT32 df     = varParam[0].uint32_;
	TRM_UINT32 totalDocument = varParam[1].uint32_;

	if (y == 1) {
		// Ogawa 式
		if (x == 0) {
			return 1.0;
		}
		return log(1.0 + x*(double)totalDocument/(double)df)
			/log(1.0 + x*(double)totalDocument);
	} else if (y == 4) {
		// Ogawa 式
		return log(1.0 +
								x/(1.0 - x)*(double)totalDocument/(double)df);
	} if (y == 6) {
		// Ogawa2 式
		return log((x*(double)totalDocument + df)
								/(q*(double)totalDocument + df))
		      /log((x*(double)totalDocument + 1.0)
								/(q*(double)totalDocument + 1.0));
	} else if (y == 7) {
		// Ogawa2 式
		return log((x*(double)totalDocument + df)
								/(q*(double)totalDocument + df));
	} else if (y == 0) {
		// Robertson 式
		return (x + log((double)totalDocument/(double)df))
			/(x + log((double)totalDocument));
	} else if (y == 3) {
		// Robertson 式
		return log(x/(1.0 - x)*(double)totalDocument/(double)df);
	} else if (y == 2) {
		// Haprper/Croft 式
		return (x + log((double)(totalDocument - df)/(double)df))
			/(x + log((double)(totalDocument - 1.0)));
	} else if (y == 5) {
		// Haprper/Croft 式
		return log(x/(1.0 - x)*
								(double)(totalDocument - df)/(double)df);
	} else if (y == 8) {
		// s == 1 の場合
		double T((double)df/(double)(totalDocument - df));
		if (x == 1) {
			return 0;
		}
		return log(
			((x + (1.0 + a)*T)/(1.0 - x))/
			((q + T)/(1.0 - q)));
	} else if (y == 9) {
		// まともに計算する
		double tmp1((double)totalDocument/(double)df);
		double tmp2(pow(tmp1, s));
		return log(
			(x + (1.0 + a)/(1.0 - tmp2))/(1.0 - x)/
			(q +1.0/(tmp1 - 1))*(1.0 - q));
	}
	return 0.0;
}

//
// Copyright (c) 1999, 2000, 2001, 2002, 2005, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
