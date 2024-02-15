// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalizerManager.cpp --
// 
// Copyright (c) 2002, 2004, 2006, 2009, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Inverted/NormalizerManager.h"
#include "Utility/UNA.h"

#include "ModInvertedTokenizer.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::NormalizerManager::NormalizerManager -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
NormalizerManager::NormalizerManager()
{
	ModInvertedTokenizer::setGetNormalizer(NormalizerManager::get);
}

//
//	FUNCTION public
//	Inverted::NormalizerManager::~NormalizerManager -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
NormalizerManager::~NormalizerManager()
{
}

//
//	FUNCTION public static
//	Inverted::NormalizerManager::get -- 異表記正規化器を得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32 normRscId_
//		リソースID
//
//	RETURN
//	ModNormalizer*
//		異表記正規化器。メモリーは呼び出し側が解放する
//
//	EXCEPTIONS
//
#ifdef SYD_USE_UNA_V10
UNA::ModNlpAnalyzer*
#else
ModNormalizer*
#endif
NormalizerManager::get(const ModUInt32 normRscId_)
{
#ifdef SYD_USE_UNA_V10
	// ModNlpAnalyserのインスタンスを得る
	return Utility::Una::Manager::getModNlpAnalyzer(normRscId_);
#else
	// ModNormalizerのインスタンスを得る
	return Utility::Una::Manager::getModNormalizer(normRscId_);
#endif
}

//
//	Copyright (c) 2002, 2004, 2006, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
