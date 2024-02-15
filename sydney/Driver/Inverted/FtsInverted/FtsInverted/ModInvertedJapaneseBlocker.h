// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedJapaneseBlocker.h -- 転置のための日本語処理器の定義
// 
// Copyright (c) 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedJapaneseBlocker_H__
#define __ModInvertedJapaneseBlocker_H__

#include "ModUnicodeChar.h"
#include "ModUnicodeCharTrait.h"
#include "ModInvertedUnicodeCharBlocker.h"

class ModCharString;

//
// CLASS
// ModInvertedJapaneseBlocker -- 転置のための日本語処理器
//
// NOTES
// 分割器の日本語化のために用いる。
// JIS コードの区分と、日常感覚による文字種の判定のずれを補正する。
// 
class
ModInvertedJapaneseBlocker : public ModInvertedUnicodeCharBlocker
{
public:
//	ModInvertedJapaneseBlocker() {}
//	virtual ~ModInvertedJapaneseBlocker() {}

	ModSize getBlockNum() const;
	void getBlockRegion(const ModSize, ModUnicodeChar&, ModUnicodeChar&) const;
	ModSize getBlock(const ModUnicodeChar) const;

protected:
	//
	// ENUM
	// CharType -- 文字種
	//
	// NOTES
	// 日本語化分割器で使用する文字種を表す。
	//
	enum BlockName {
		undefined = 0,			// 未定義ないしは下記以外
		ascii,
		symbol,
		digit,
		alphabet,
		hiragana,
		katakana,
		greek,
		russian,
		line,
		kanji,
		hankakukana,
		gaiji,
		blockNum		// 異なり数
	};

	static ModUnicodeChar blockBegin[];
	static ModUnicodeChar blockEnd[];
};


inline ModSize
ModInvertedJapaneseBlocker::getBlockNum() const
{
	return blockNum;
}

inline ModSize
ModInvertedJapaneseBlocker::getBlock(const ModUnicodeChar uchar) const
{
	ModUnicodeCharType utype = ModUnicodeCharTrait::getType(uchar);

	if (ModUnicodeCharTrait::isKanji(utype)) {
		return kanji;
	} else if (ModUnicodeCharTrait::isHiragana(utype)) {
		return hiragana;
	} else if (ModUnicodeCharTrait::isKatakana(utype)) {
#if 0
		// 半角かなも katakana ビットが立っている
		if (utype&ModUnicodeCharTrait::hankakuKana) {
			return hankakukana;
		}
#endif
		return katakana;
	} else if (ModUnicodeCharTrait::isAscii(utype)) {
		// alphabet, digit の判断よりも前にないと ASCII を集められない
		return ascii;
	} else if (ModUnicodeCharTrait::isAlphabet(utype)) {
		return alphabet;
	} else if (ModUnicodeCharTrait::isDigit(utype)) {
		return digit;
	} else if (ModUnicodeCharTrait::isSymbol(utype)) {
#if 0
		// 記号に含まれる文字には以下のような例外がある
		// 処理順序はカバレージテストの結果に基づいて修正した
		if (wchar == longVowelMark) {
			return katakana;
		} else if (wchar == kanjiRepetitionMark) {
			return kanji;
		} else if (wchar == pSoundMark) {
			return katakana;
		} else if (wchar == sonantMark) {
			return katakana;
		} else if (wchar == hiraganaRepetitionMark) {
			return hiragana;
		} else if (wchar == hiraganaSonantRepetitionMark) {
			return hiragana;
		} else if (wchar == katakanaRepetitionMark) {
			return katakana;
		} else if (wchar == katakanaSonantRepetitionMark) {
			return katakana;
		} else if (wchar == kanjiShimeMark) {
			return kanji;
//		} else if (wchar == kanjiRepetitionMark1) {
//			return ModUnicodeCharTrait::kanjiFirst;
		}
		// 罫線は ModUnicodeCharTrait::getType では記号の１種となっている
		if (ModUnicodeCharTrait::line) {
			return line;
		}
		// 半角かな記号は記号の１種となっている
		if (ModUnicodeCharTrait::hankakuKana) {
			return hankakukana;
		}
#endif
		// 記号以外の文字種はそのまま返す
		return symbol;
	} else if (ModUnicodeCharTrait::isGaiji(utype)) {
		return gaiji;
	} else if (ModUnicodeCharTrait::isGreek(utype)) {
		return greek;
	} else if (ModUnicodeCharTrait::isRussian(utype)) {
		return russian;
	}

	return undefined;
}

#endif	__ModInvertedJapaneseBlocker_H__

//
// Copyright (c) 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

