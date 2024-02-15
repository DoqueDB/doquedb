// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
//
// ModUnicodeCharacterType.cpp -
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

#include "UnicodeCharacterType.h"

#if 1

#include "ModUnicodeCharTypes.h"

const unsigned long UnicodeCharacterType::s_none		= ModUnicodeCharTypes::none;
const unsigned long UnicodeCharacterType::s_notused	= ModUnicodeCharTypes::notused;
const unsigned long UnicodeCharacterType::s_alphabet	= ModUnicodeCharTypes::alphabet;
const unsigned long UnicodeCharacterType::s_upper		= ModUnicodeCharTypes::upper;
const unsigned long UnicodeCharacterType::s_lower		= ModUnicodeCharTypes::lower;
const unsigned long UnicodeCharacterType::s_letterOther = ModUnicodeCharTypes::letterOther;
const unsigned long UnicodeCharacterType::s_digit		= ModUnicodeCharTypes::digit;
const unsigned long UnicodeCharacterType::s_xdigit		= ModUnicodeCharTypes::xdigit;
const unsigned long UnicodeCharacterType::s_symbol		= ModUnicodeCharTypes::symbol;
const unsigned long UnicodeCharacterType::s_space		= ModUnicodeCharTypes::space;
const unsigned long UnicodeCharacterType::s_ascii		= ModUnicodeCharTypes::ascii;
const unsigned long UnicodeCharacterType::s_hankakuKana = ModUnicodeCharTypes::hankakuKana;
const unsigned long UnicodeCharacterType::s_hiragana	= ModUnicodeCharTypes::hiragana;
const unsigned long UnicodeCharacterType::s_katakana	= ModUnicodeCharTypes::katakana;
const unsigned long UnicodeCharacterType::s_greek		= ModUnicodeCharTypes::greek;
const unsigned long UnicodeCharacterType::s_cyrillic	= ModUnicodeCharTypes::russian;
const unsigned long UnicodeCharacterType::s_line		= ModUnicodeCharTypes::line;
const unsigned long UnicodeCharacterType::s_kanji		= ModUnicodeCharTypes::kanji;

const unsigned long UnicodeCharacterType::s_control	= ModUnicodeCharTypes::control;
const unsigned long UnicodeCharacterType::s_format		= ModUnicodeCharTypes::format;
const unsigned long UnicodeCharacterType::s_surrogate	= ModUnicodeCharTypes::surrogate;
const unsigned long UnicodeCharacterType::s_gaiji		= ModUnicodeCharTypes::gaiji;

#else // ModCharTrait の文字種を使いたくない場合はこちらを使いましょう

//
// 文字種の変更に追い付いていない可能性あり
// (文字種の名前が違うかも	--- コンパイルできないかも)
// (文字種が足りないかも	--- リンクできないかも)
//

const unsigned long UnicodeCharacterType::s_none		= 0x00000001;
const unsigned long UnicodeCharacterType::s_notused	= 0x00000002;
const unsigned long UnicodeCharacterType::s_alphabet	= 0x00000004;
const unsigned long UnicodeCharacterType::s_upper		= 0x00000008;
const unsigned long UnicodeCharacterType::s_lower		= 0x00000010;
const unsigned long UnicodeCharacterType::s_digit		= 0x00000020;
const unsigned long UnicodeCharacterType::s_xdigit		= 0x00000040;
const unsigned long UnicodeCharacterType::s_symbol		= 0x00000080;
const unsigned long UnicodeCharacterType::s_space		= 0x00000100;
const unsigned long UnicodeCharacterType::s_ascii		= 0x00000200;
const unsigned long UnicodeCharacterType::s_hankakuKana = 0x00000400;
const unsigned long UnicodeCharacterType::s_hiragana	= 0x00000800;
const unsigned long UnicodeCharacterType::s_katakana	= 0x00001000;
const unsigned long UnicodeCharacterType::s_greek		= 0x00002000;
const unsigned long UnicodeCharacterType::s_cyrillic	= 0x00004000;
const unsigned long UnicodeCharacterType::s_line		= 0x00008000;
const unsigned long UnicodeCharacterType::s_kanji		= 0x00010000;

#endif
