// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// TRMExternalScoreCalculator.h --　TRMeister外部スコア計算器インタフェース
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2004, 2005,  2006, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMExternalScoreCalculator_H__
#define __TRMExternalScoreCalculator_H__

#if defined(USE_TRM_EXTERNALSCORECALCULATOR)
#include "Inverted/Module.h"
#endif


#if defined(WIN32) 
#define TRM_EXPORT  __declspec(dllexport)
#endif

#if defined(USE_TRM_EXTERNALSCORECALCULATOR)
#define TRM_FUNC
#else
#if defined(WIN32)
#define TRM_FUNC TRM_EXPORT
#else
#define TRM_FUNC
#endif
#endif

#if defined(WIN32)
// 型宣言 (windows用の型宣言
typedef INT32	TRM_INT32;
typedef UINT32	TRM_UINT32;
typedef	INT64	TRM_INT64;
typedef	UINT64	TRM_UINT64;
typedef PCHAR	TRM_PCHAR;
typedef double	TRM_DOUBLE;
#else
typedef long			TRM_INT32;
typedef unsigned long	TRM_UINT32;
typedef long long		TRM_INT64;
typedef unsigned long long	TRM_UINT64;
typedef char*	TRM_PCHAR;
typedef double	TRM_DOUBLE;
#endif

#if defined(USE_TRM_EXTERNALSCORECALCULATOR)
_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN
#endif
// 外部スコア計算器が利用できるTRMeisterのパラメータ一覧
enum TRMeisterScoreParameterId
{
	TRM_TermFrequency_UINT32 = 1,
	TRM_TotalTermFrequency_UINT32,
	TRM_TotalTermFrequency_UINT64,
	TRM_QueryTermFrequency_UINT32,
	TRM_DocumentFrequency_UINT32,
	TRM_TotalDocumentFrequency_UINT32,
	TRM_TotalDocumentFrequency_UINT64,
	TRM_DocumentLength_UINT32,
	TRM_TotalDocumentLength_UINT32,
	TRM_TotalDocumentLength_UINT64,
	TRM_AverageDocumentLength_UINT32,
	TRM_QueryTermSurface_STRING				// 'actual type' is const char *
};

union TRMeisterScoreParameterValue
{
	TRM_INT32				int32_;
	TRM_UINT32				uint32_;
	TRM_INT64				int64_;
	TRM_UINT64				uint64_;
	TRM_PCHAR /*char **/	string_;
	TRM_DOUBLE /*double*/	double_;
};




class TRMExternalScoreCalculator
{
public:
	TRMExternalScoreCalculator(){}
	virtual ~TRMExternalScoreCalculator(){}
	virtual TRMeisterScoreParameterId *getTfParameterList() = 0;
	virtual TRMeisterScoreParameterId *getDfParameterList() = 0;
	virtual double tfTerm(const TRMeisterScoreParameterValue *varParam) = 0;
	virtual double dfTerm(const TRMeisterScoreParameterValue *varParam) = 0;
};

extern "C"{

#if defined(USE_TRM_EXTERNALSCORECALCULATOR)
typedef void *(*GetScoreCalculator)(const char* );
#else
//外部スコア計算器が実装すべき関数
TRM_FUNC void *
getScoreCalculator(const char* parameter_);
#endif
}

#if defined(USE_TRM_EXTERNALSCORECALCULATOR)
_SYDNEY_INVERTED_END
_SYDNEY_END
#endif

#endif
//
// Copyright (c) 1999, 2000, 2001, 2002, 2004, 2005,  2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
