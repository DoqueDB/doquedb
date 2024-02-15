// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// JapaneseBlocker.cpp
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/JapaneseBlocker.h"
#include "FullText2/Tokenizer.h"

#include "Exception/SQLSyntaxError.h"

#include "ModUnicodeCharTrait.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	const ModUnicodeChar _UND[] = {0};				// Undefined
	const ModUnicodeChar _ASC[] = {'A','S','C',0};	// Ascii
	const ModUnicodeChar _SYM[] = {'S','Y','M',0};	// Symbol
	const ModUnicodeChar _DIG[] = {'D','I','G',0};	// Digit
	const ModUnicodeChar _ALP[] = {'A','L','P',0};	// Alphabet
	const ModUnicodeChar _HIR[] = {'H','I','R',0};	// Hiragana
	const ModUnicodeChar _KAT[] = {'K','A','T',0};	// Katakana
	const ModUnicodeChar _GRK[] = {'G','R','K',0};	// Greek
	const ModUnicodeChar _RUS[] = {'R','U','S',0};	// Russian
	const ModUnicodeChar _KAN[] = {'K','A','N',0};	// Kanji
	const ModUnicodeChar _GAI[] = {'G','A','I',0};	// Gaiji
	const ModUnicodeChar _OTH[] = {'O','T','H',0};	// Other
	
	//
	// ブロック名
	// JapaneseBlocker::Name の順番である必要がある
	//
	const ModUnicodeChar* _block[] = {
		_UND,		// Undefined
		_ASC,		// Ascii
		_SYM,		// Symbol
		_DIG,		// Digit
		_ALP,		// Alphabet
		_HIR,		// Hiragana
		_KAT,		// Katakana
		_GRK,		// Greek
		_RUS,		// Russian
		_KAN,		// Kanji
		_GAI,		// Gaiji
		_OTH,		// Other
		0
	};

	//
	//	ブロックを得る
	//
	ModSize _getBlock(const ModUnicodeChar* p)
	{
		for (ModSize i = 1; i < JapaneseBlocker::ValueNum; ++i)
			if (Tokenizer::compare(p, _block[i]) == true)
				return i;
		return 0;
	}

	//
	//	すべて
	//
	const ModUnicodeString _cAll = "ALL";
}

//
//	FUNCTION public
//	FullText2::JapaneseBlocker::JapaneseBlocker -- コンストラクタ
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
JapaneseBlocker::JapaneseBlocker()
	: Blocker()
{
	m_pCheckPair = new bool[ValueNum * ValueNum];
	for (int i = 0; i < ValueNum * ValueNum; ++i)
		m_pCheckPair[i] = false;
	
	m_pBlockMin = new ModSize[ValueNum];
	m_pBlockMax = new ModSize[ValueNum];
	for (int i = 0; i < ValueNum; ++i)
	{
		m_pBlockMin[i] = 2;
		m_pBlockMax[i] = 2;
	}
	m_uiBlockCount = ValueNum;
}

//
//	FUNCTION public
//	FullText2::JapaneseBlocker::~JapaneseBlocker -- デストラクタ
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
JapaneseBlocker::~JapaneseBlocker()
{
	delete[] m_pCheckPair;
	delete[] m_pBlockMin;
	delete[] m_pBlockMax;
}

//
//	FUNCTION public
//	FullText2::JapaneseBlocker::parse -- パラメータをパースする
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar* parameter_
//		パラメータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
JapaneseBlocker::parse(const ModUnicodeChar* parameter_)
{
	Blocker::Token::Type eType;
	ModVector<const ModUnicodeChar*> element;
	
	if (Tokenizer::compare(parameter_, _cAll) == true)
	{
		ModSize min, max;
		
		// ALL: がある
		eType = Blocker::nextToken(parameter_, element);
		if (eType == Blocker::Token::Number)
		{
			min = ModUnicodeCharTrait::toInt(element[0]);
			max = min;
		}
		else if (eType == Blocker::Token::Number_Number)
		{
			min = ModUnicodeCharTrait::toInt(element[0]);
			max = ModUnicodeCharTrait::toInt(element[1]);
		}
		else
		{
			_TRMEISTER_THROW1(Exception::SQLSyntaxError, parameter_);
		}

		// すべてのデフォルトなので、設定する
		for (int i = 0; i < ValueNum; ++i)
		{
			m_pBlockMin[i] = min;
			m_pBlockMax[i] = max;
		}
	}

	while (*parameter_ != 0 && *parameter_ != '@')
	{
		eType = Blocker::nextToken(parameter_, element);
		switch (eType)
		{
		case Blocker::Token::Keyword_Number:
			{
				// KAN:2 など
				
				ModSize b = _getBlock(element[0]);
				ModSize v = ModUnicodeCharTrait::toInt(element[1]);
				m_pBlockMin[b] = v;
				m_pBlockMax[b] = v;
			}
			break;
		case Blocker::Token::Keyword_Number_Number:
			{
				// ASC:1:3 など
				
				ModSize b = _getBlock(element[0]);
				m_pBlockMin[b] = ModUnicodeCharTrait::toInt(element[1]);
				m_pBlockMax[b] = ModUnicodeCharTrait::toInt(element[2]);
			}
			break;
		case Blocker::Token::Keyword_Keyword:
			{
				// KAN:HIR など
				
				ModSize b1 = _getBlock(element[0]);
				ModSize b2 = _getBlock(element[1]);
				m_pCheckPair[b1 * ValueNum + b2] = true;
			}
			break;
		default:
			_TRMEISTER_THROW1(Exception::SQLSyntaxError, parameter_);
		}
	}
}

//
//	FUNCTION protected
//	FullText2::JapaneseBlocker::getBlock -- ブロック種別を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeChar c_
//		文字
//
//	RETURN
//	ModSize
//		ブロック種別
//
//	EXCEPTIONS
//
ModSize
JapaneseBlocker::getBlock(ModUnicodeChar c_) const
{
	ModSize r = Undefined;
	ModUnicodeCharType utype = ModUnicodeCharTrait::getType(c_);

	if (ModUnicodeCharTrait::isKanji(utype)) {
		r = Kanji;
	} else if (ModUnicodeCharTrait::isHiragana(utype)) {
		r = Hiragana;
	} else if (ModUnicodeCharTrait::isKatakana(utype)) {
		r = Katakana;
	} else if (ModUnicodeCharTrait::isAscii(utype)) {
		// alphabet, digit の判断よりも前にないと ASCII を集められない
		r = Ascii;
	} else if (ModUnicodeCharTrait::isAlphabet(utype)) {
		r = Alphabet;
	} else if (ModUnicodeCharTrait::isDigit(utype)) {
		r = Digit;
	} else if (ModUnicodeCharTrait::isSymbol(utype)) {
		r = Symbol;
	} else if (ModUnicodeCharTrait::isGaiji(utype)) {
		r = Gaiji;
	} else if (ModUnicodeCharTrait::isGreek(utype)) {
		r = Greek;
	} else if (ModUnicodeCharTrait::isRussian(utype)) {
		r = Russian;
	}

	return r;
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
