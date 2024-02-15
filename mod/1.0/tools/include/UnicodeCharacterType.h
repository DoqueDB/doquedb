// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
//
// ModUnicodeCharacterType.h -
// 
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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

#include <iostream>
#include <assert.h>

using namespace std;

class UnicodeCharacterType
{
public:
	//
	// コンストラクタ、デストラクタ
	// (コピーコンストラクタはデフォルトのものを使用)
	//

	UnicodeCharacterType();
	UnicodeCharacterType(const unsigned long type);
	~UnicodeCharacterType();

	//
	// オペレータ
	//

	// ビットを全てゼロにする(文字種を none に変更することに相当)
	void	clear();

	// 文字種の追加
	void	addNotused();
	void	addAlphabet();
	void	addUpper();
	void	addLower();
	void	addLetterOther();
	void 	addDigit();
	void 	addXdigit();
	void 	addSymbol();
	void	addSpace();
	void	addAscii();
	void	addHankakuKana();
	void	addHiragana();
	void	addKatakana();
	void	addGreek();
	void	addCyrillic();
	void	addLine();
	void	addKanji();
	void	addControl();
	void	addFormat();
	void	addSurrogate();
	void	addGaiji();

	// 文字種の削除
	void	deleteLetterOther();

	//
	// アクセッサ
	//

	// 何らかの文字種が設定されている場合は true が返る
	bool	isNotNone() const;

	// 文字種の確認
	bool	isNotused() const;
	bool	isAlphabet() const;
	bool	isUpper() const;
	bool	isLower() const;
	bool	isLetterOther() const;
	bool 	isDigit() const;
	bool 	isXdigit() const;
	bool 	isSymbol() const;
	bool	isSpace() const;
	bool	isAscii() const;
	bool	isHankakuKana() const;
	bool	isHiragana() const;
	bool	isKatakana() const;
	bool	isGreek() const;
	bool	isCyrillic() const;
	bool	isLine() const;
	bool	isKanji() const;
	bool	isControl() const;
	bool	isFormat() const;
	bool	isSurrogate() const;
	bool	isGaiji() const;

	// その他
	unsigned long	getValue() const;

	// 演算子
	UnicodeCharacterType operator | (const UnicodeCharacterType& charType);

private:
	static const unsigned long s_none;		// 値が 0 である事を保証する
	static const unsigned long s_notused;
	static const unsigned long s_alphabet;
	static const unsigned long s_upper;
	static const unsigned long s_lower;
	static const unsigned long s_letterOther;
	static const unsigned long s_digit;
	static const unsigned long s_xdigit;
	static const unsigned long s_symbol;
	static const unsigned long s_space;
	static const unsigned long s_ascii;
	static const unsigned long s_hankakuKana;
	static const unsigned long s_hiragana;
	static const unsigned long s_katakana;
	static const unsigned long s_greek;
	static const unsigned long s_cyrillic;
	static const unsigned long s_line;
	static const unsigned long s_kanji;
	static const unsigned long s_control;
	static const unsigned long s_format;
	static const unsigned long s_surrogate;
	static const unsigned long s_gaiji;

	unsigned long d_value;
};


inline
UnicodeCharacterType::UnicodeCharacterType()
	: d_value(UnicodeCharacterType::s_none)
{
	// s_none はゼロであることを保証しなければいけない
	if (s_none != 0) {
		cerr << "CharacterType::s_none != 0" << endl;
		; assert(0);
		throw 1;
	}
}

inline
UnicodeCharacterType::UnicodeCharacterType(const unsigned long type)
	: d_value(type)
{
	// s_none はゼロであることを保証しなければいけない
	if (s_none != 0) {
		cerr << "CharacterType::s_none != 0" << endl;
		; assert(0);
		throw 1;
	}
}

inline
UnicodeCharacterType::~UnicodeCharacterType()
{
	// do nothing
}

//
// アクセッサ
// (オペレータの中で inline なアクセッサを使っていると問題が起こる(C++ の仕様)
// ので、オペレータの先にアクセッサを定義)
//

inline
unsigned long
UnicodeCharacterType::getValue() const
{
	return d_value;
}

inline
bool
UnicodeCharacterType::isNotNone() const
{
	return bool((d_value != s_none) != 0);
}

inline
bool
UnicodeCharacterType::isNotused() const
{
	return bool((d_value & s_notused) != 0);
}

inline
bool
UnicodeCharacterType::isAlphabet() const
{
	return bool((d_value & s_alphabet) != 0);
}

inline
bool
UnicodeCharacterType::isUpper() const
{
	return bool((d_value & s_upper) != 0);
}

inline
bool
UnicodeCharacterType::isLower() const
{
	return bool((d_value & s_lower) != 0);
}

inline
bool
UnicodeCharacterType::isLetterOther() const
{
	return bool((d_value & s_letterOther) != 0);
}

inline
bool
UnicodeCharacterType::isDigit() const
{
	return bool((d_value & s_digit) != 0);
}

inline
bool
UnicodeCharacterType::isXdigit() const
{
	return bool((d_value & s_xdigit) != 0);
}

inline
bool
UnicodeCharacterType::isSymbol() const
{
	return bool((d_value & s_symbol) != 0);
}

inline
bool
UnicodeCharacterType::isSpace() const
{
	return bool((d_value & s_space) != 0);
}

inline
bool
UnicodeCharacterType::isAscii() const
{
	return bool((d_value & s_ascii) != 0);
}

inline
bool
UnicodeCharacterType::isHankakuKana() const
{
	return bool((d_value & s_hankakuKana) != 0);
}

inline
bool
UnicodeCharacterType::isHiragana() const
{
	return bool((d_value & s_hiragana) != 0);
}

inline
bool
UnicodeCharacterType::isKatakana() const
{
	return bool((d_value & s_katakana) != 0);
}

inline
bool
UnicodeCharacterType::isGreek() const
{
	return bool((d_value & s_greek) != 0);
}

inline
bool
UnicodeCharacterType::isCyrillic() const
{
	return bool((d_value & s_cyrillic) != 0);
}

inline
bool
UnicodeCharacterType::isLine() const
{
	return bool((d_value & s_line) != 0);
}

inline
bool
UnicodeCharacterType::isKanji() const
{
	return bool((d_value & s_kanji) != 0);
}

inline
bool
UnicodeCharacterType::isControl() const
{
	return bool((d_value & s_control) != 0);
}

inline
bool
UnicodeCharacterType::isFormat() const
{
	return bool((d_value & s_format) != 0);
}

inline
bool
UnicodeCharacterType::isSurrogate() const
{
	return bool((d_value & s_surrogate) != 0);
}

inline
bool
UnicodeCharacterType::isGaiji() const
{
	return bool((d_value & s_gaiji) != 0);
}

//
// オペレータ
//

inline
void
UnicodeCharacterType::addNotused()
{
	if (isNotNone()) {
		// 他の文字種と同時に成り立つ事はありえない
		cerr << "err in CharacterType::addNotused" << endl;
		; assert(0);
		throw 1;
	}

	d_value |= s_notused;
}


inline
void
UnicodeCharacterType::addAlphabet()
{
	d_value |= s_alphabet;
}

inline
void
UnicodeCharacterType::addUpper()
{
	d_value |= s_upper;
}

inline
void
UnicodeCharacterType::addLower()
{
	d_value |= s_lower;
}

inline
void
UnicodeCharacterType::addLetterOther()
{
	d_value |= s_letterOther;
}

inline
void
UnicodeCharacterType::addDigit()
{
	d_value |= s_digit;
}

inline
void
UnicodeCharacterType::addXdigit()
{
	d_value |= s_xdigit;
}

inline
void
UnicodeCharacterType::addSymbol()
{
	d_value |= s_symbol;
}

inline
void
UnicodeCharacterType::addSpace()
{
	d_value |= s_space;
}

inline
void
UnicodeCharacterType::addAscii()
{
	d_value |= s_ascii;
}

inline
void
UnicodeCharacterType::addHankakuKana()
{
	d_value |= s_hankakuKana;
}

inline
void
UnicodeCharacterType::addHiragana()
{
	d_value |= s_hiragana;
}

inline
void
UnicodeCharacterType::addKatakana()
{
	d_value |= s_katakana;
}

inline
void
UnicodeCharacterType::addGreek()
{
	d_value |= s_greek;
}

inline
void
UnicodeCharacterType::addCyrillic()
{
	d_value |= s_cyrillic;
}

inline
void
UnicodeCharacterType::addLine()
{
	d_value |= s_line;
}

inline
void
UnicodeCharacterType::addKanji()
{
	d_value |= s_kanji;
}

inline
void
UnicodeCharacterType::addControl()
{
	d_value |= s_control;
}

inline
void
UnicodeCharacterType::addFormat()
{
	d_value |= s_format;
}

inline
void
UnicodeCharacterType::addSurrogate()
{
	d_value |= s_surrogate;
}

inline
void
UnicodeCharacterType::addGaiji()
{
	d_value |= s_gaiji;
}

inline
void
UnicodeCharacterType::deleteLetterOther()
{
	d_value &= ~s_letterOther;
}

inline
void
UnicodeCharacterType::clear()
{
	d_value = 0;
}

// 演算子
inline
UnicodeCharacterType
UnicodeCharacterType::operator | (const UnicodeCharacterType& charType)
{
	return UnicodeCharacterType(d_value | charType.d_value);
}
