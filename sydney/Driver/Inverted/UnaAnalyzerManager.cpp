// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UnaAnalyzerManager.cpp --
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
#include "Inverted/UnaAnalyzerManager.h"
#include "Inverted/Parameter.h"
#include "Os/CriticalSection.h"
#include "Os/AutoCriticalSection.h"
#include "Utility/UNA.h"

#include "ModInvertedTokenizer.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$:_cLanguageParameter -- デフォルトの言語情報のパラメータ
	//
	//	NOTES
	//
	ParameterString _cLanguageParameter("Inverted_DefaultLanguageSet", ModUnicodeString(""));

	//
	//	VARIABLE local
	//	_$$::_cDefaultLanguageSet -- デフォルトの言語情報
	//
	//	NOTES
	//
	ModLanguageSet _cDefaultLanguageSet;

	//
	//	VARIABLE local
	//	_$$::_cDefaultLanguageSetName -- デフォルトの言語情報
	//
	//	NOTES
	//
	ModUnicodeString _cDefaultLanguageSetName;

	//
	//	VARIABLE local
	//	_$$::_bSetDefaultLanguageSet -- デフォルトの言語情報を設定したかどうか
	//
	//	NOTES
	//
	bool _bSetDefaultLanguageSet = false;

#ifndef SYD_USE_UNA_V10
	//
	//	VARIABLE local
	//	_$$::_cMaxWordLength -- 最大単語長
	//
	//	NOTES
	//
	ParameterInteger _cMaxWordLength("Inverted_MaxWordLength", 32);
#endif

	//
	//	VARIABLE local
	//	_$$::_cLock -- 言語情報のラッチ
	//
	Os::CriticalSection _cLock;
}

//
//	FUNCTION public
//	Inverted::UnaAnalyzerManager::UnaAnalyzerManager -- コンストラクタ
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
UnaAnalyzerManager::UnaAnalyzerManager()
{
	ModInvertedTokenizer::setGetAnalyzer(UnaAnalyzerManager::get);
}

//
//	FUNCTION public
//	Inverted::UnaAnalyzerManager::~UnaAnalyzerManager -- デストラクタ
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
UnaAnalyzerManager::~UnaAnalyzerManager()
{
}

//
//	FUNCTION public static
//	Inverted::UnaAnalyzerManager::get -- UNA解析器を得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32 unaRscId_
//		リソースID
//
//	RETURN
//	ModNlpAnalyzer*
//		UNA解析器。メモリーは呼び出し側が解放する
//
//	EXCEPTIONS
//
#ifdef SYD_USE_UNA_V10
UNA::ModNlpAnalyzer*
#else
ModNlpAnalyzer*
#endif
UnaAnalyzerManager::get(const ModUInt32 unaRscId_)
{
	// ModNlpAnalyzerのインスタンスを得る
	return Utility::Una::Manager::getModNlpAnalyzer(unaRscId_
#ifndef SYD_USE_UNA_V10
													, _cMaxWordLength.get()
#endif
		);
}

//
//	FUNCTION public static
//	Inverted::UnaAnalyzerManager::getDefaultLanguageSet
//		-- デフォルト言語情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModLanguageSet&
//		デフォルト言語情報
//
//	EXCEPTIONS
//
const ModLanguageSet&
UnaAnalyzerManager::getDefaultLanguageSet()
{
	if (_bSetDefaultLanguageSet == false)
	{
		Os::AutoCriticalSection cAuto(_cLock);

		// まだデフォルトの言語情報を読み込んでいない

		// [NOTE] パラメータが設定されていない場合、'ja+en'では「ない」。
		//  デフォルトの言語はUNAに任せる。
		_cDefaultLanguageSet = _cLanguageParameter.get();
		_cDefaultLanguageSetName = _cDefaultLanguageSet.getName();
		_bSetDefaultLanguageSet = true;
	}
	return _cDefaultLanguageSet;
}

//
//	FUNCTION public static
//	Inverted::UnaAnalyzerManager::getDefaultLanguageSetName -- デフォルト言語情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUnicodeString&
//		デフォルト言語情報
//
//	EXCEPTIONS
//
const ModUnicodeString&
UnaAnalyzerManager::getDefaultLanguageSetName()
{
	getDefaultLanguageSet();
	return _cDefaultLanguageSetName;
}

//
//	Copyright (c) 2002, 2004, 2006, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
