// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModUnicodeCharType.h -- ModUnicodeCharType のクラス定義
// 
// Copyright (c) 2000, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModUnicodeCharTypes_H__
#define __ModUnicodeCharTypes_H__

// TYPEDEF
// ModUnicodeCharType -- Unicode文字の文字種を表す型
//
// NOTES

typedef unsigned int ModUnicodeCharType;

// CLASS
// ModUnicodeCharTypes -- Unicode 文字の文字種
//
// NOTES

//【注意】	名前空間を切るためだけのクラスなので dllexport しない

struct ModUnicodeCharTypes
{
	enum
	{
		// 対応する文字種が無いことを意味する
		none =			0x00000000,		// 値がゼロである事を保証する
		// Unicode で使われていないことを表す
		notused =		0x00000001,

		// 英字
		alphabet =		0x00000002,
		// 英字の大文字
		upper =			0x00000004,
		// 英字の小文字
		lower =			0x00000008,
		// 英字以外の文字
		letterOther =	0x00000010,
		// 数字
		digit =			0x00000020,
		// 数字(16進)
		xdigit =		0x00000040,
		// 記号
		symbol =		0x00000080,
		// 空白
		space =			0x00000100,
		// ASCII
		ascii =			0x00000200,

		// 半角カナ
		hankakuKana =	0x00000400,
		// ひらがな
		hiragana =		0x00000800,
		// 全角カタカナ
		katakana =		0x00001000,
		// ギリシャ文字
		greek =			0x00002000,
		// ロシア文字
		russian =		0x00004000,
		// 罫線
		line =			0x00008000,
		// 漢字
		kanji =			0x00010000,
		// コントロール文字
		control =		0x00020000,
		// フォーマット文字
		format =		0x00040000,
		// サロゲート文字
		surrogate =		0x00080000,
		// 外字
		gaiji =			0x00100000
	};
};

#endif	// __ModUnicodeCharTypes_H__

//
// Copyright (c) 2000, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
