// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModWideCharTrait.h -- ModWideCharTrait のクラス定義
// 
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModWideCharTrait_H__
#define __ModWideCharTrait_H__

#include "ModCommonDLL.h"
#include "ModCommon.h"
#include "ModWideChar.h"
#include "ModCharTrait.h"
#include "ModKanjiCode.h"

//
// DEFINE
// ModCodeMaskSet0 -- ASCII を示すマスク
//
// NOTES
// この定数はワイド文字がコードセット0に対応するものかどうかを
// 判定するために用いる。
// codeMask と and をとった結果がこの定数の値と等しければコードセット0。
//
#define ModCodeMaskSet0 (0x00000000)

//
// DEFINE
// ModCodeMaskSet1 -- コードセット1を示すマスク
//
// NOTES
// この定数はワイド文字がコードセット1に対応するものかどうかを
// 判定するために用いる。
// codeMask と and をとった結果がこの定数の値と等しければコードセット1。
//
#define ModCodeMaskSet1 (0x30000000)

//
// DEFINE
// ModCodeMaskSet2 -- コードセット2を示すマスク
//
// NOTES
// この定数はワイド文字がコードセット2に対応するものかどうかを
// 判定するために用いる。
// codeMask と and をとった結果がこの定数の値と等しければコードセット2。
//
#define ModCodeMaskSet2 (0x10000000)

//
// DEFINE
// ModCodeMaskSet3 -- コードセット3を示すマスク
//
// NOTES
// この定数はワイド文字がコードセット3に対応するものかどうかを
// 判定するために用いる。
// codeMask と and をとった結果がこの定数の値と等しければコードセット3。
//
#define ModCodeMaskSet3 (0x20000000)

//
// DEFINE
// ModCodeMaskSet1X -- 不正なコードセットを示すマスク
//
// NOTES
// この定数はワイド文字がコードセット1が壊れたものに対応するものかどうかを
// 判定するために用いる。
// codeMask と and をとった結果がこの定数の値と等しければこの不正コード。
//
#define ModCodeMaskSet1X (0x40000000)

//
// DEFINE
// ModCodeMaskSet2X -- 不正なコードセットを示すマスク
//
// NOTES
// この定数はワイド文字がコードセット2が壊れたものに対応するものかどうかを
// 判定するために用いる。
// codeMask と and をとった結果がこの定数の値と等しければこの不正コード。
//
#define ModCodeMaskSet2X (0x50000000)

//
// DEFINE
// ModCodeMaskSet3X1 -- 不正なコードセットを示すマスク
//
// NOTES
// この定数はワイド文字がコードセット3が壊れたもの(続く1バイト目が不正)に
// 対応するものかどうかを判定するために用いる。
// codeMask と and をとった結果がこの定数の値と等しければこの不正コード。
//
#define ModCodeMaskSet3X1 (0x60000000)

//
// DEFINE
// ModCodeMaskSet3X2 -- 不正なコードセットを示すマスク
//
// NOTES
// この定数はワイド文字がコードセット3が壊れたもの(続く2バイト目が不正)に
// 対応するものかどうかを判定するために用いる。
// codeMask と and をとった結果がこの定数の値と等しければこの不正コード。
//
#define ModCodeMaskSet3X2 (0x70000000)

//
// TYPEDEF
// ModWideCharType -- ワイド文字の文字種を表す型
//
// NOTES
// この型はワイド文字の文字種をビットパターンで表すのに用いる。
// 具体的な文字種は ModWideCharTraitの定数として定義される。
// この型の実体は ModWideChar の実体と別にしておかないと、
// ワイド文字の文字種判定関数がコンパイルエラーになる。
//
typedef unsigned int ModWideCharType;

//
// CLASS
// ModWideCharTrait -- ワイド文字の特性を表現するクラス
//
// NOTES
// ModWideChar で表されたワイド文字の特性を表現するクラスである。
// ワイド文字が文字列のターミネイタであるか、ASCII か、第２水準かなどを
// 判定する関数や ModWideChar の配列で表された文字列の比較、探索、コピーなどを
// 行なう関数をメンバ関数として提供する。
// このクラスはインスタンスは作られない。
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModWideCharTrait
{
public:
	// 文字種を表すビットパターン定数の定義
	// ModWideCharType に代入される。また、判定にも使われる。

	enum
	{
		alphabet =		0x00000001,		// 英字
		digit =			0x00000002,		// 数字
		symbol =		0x00000004,		// 記号
		space =			0x00000008,		// 空白
		ascii =			0x00000010,		// ASCII
		hankakuKana =	0x00010000,		// 半角カタカナ
		hiragana =		0x00020000,		// ひらがな
		katakana =		0x00040000,		// カタカナ
		greek =			0x00100000,		// ギリシャ文字
		russian =		0x00200000,		// ロシア文字
		line =			0x01000000,		// 罫線
		kanjiFirst =	0x0a000000,		// 漢字第一水準
		kanjiSecond =	0x0c000000,		// 漢字第二水準
		kanji =			0x08000000,		// 漢字
		gaiji =			0x10000000		// 外字
	};

	// 文字種別の境界に位置する文字のコード

	enum
	{
		codeAsciiLargeA =		0x00000041,		// 'A'
		codeAsciiLargeZ =		0x0000005a,		// 'Z'
		codeAsciiSmallA =		0x00000061,		// 'a'
		codeAsciiSmallZ =		0x0000007a,		// 'z'
		codeEUCLargeA =			0x300011c1,		// 'Ａ'
		codeEUCLargeZ =			0x300011da,		// 'Ｚ'
		codeEUCSmallA =			0x300011e1,		// 'ａ'
		codeEUCSmallZ =			0x300011fa,		// 'ｚ'
		codeGreekLargeA =		0x30001321,		// 'Α'(greek)
		codeGreekLargeZ =		0x30001338,		// 'Ω'
		codeGreekSmallA =		0x30001341,		// 'α'
		codeGreekSmallZ =		0x30001358,		// 'ω'
		codeRussianLargeA =		0x300013a1,		// 'А'(russian)
		codeRussianLargeZ =		0x300013c1,		// 'Я'
		codeRussianSmallA =		0x300013d1,		// 'а'
		codeRussianSmallZ =		0x300013f1,		// 'я'
		codeAsciiZero =			0x00000030,		// '0'
		codeAsciiNine =			0x00000039,		// '9'
		codeEUCZero =			0x300011b0,		// '０'
		codeEUCNine =			0x300011b9,		// '９'
		codeEUCSpace =			0x300010a1,		// '　'(全角空白)
		codeHiraganaXa =		0x30001221,		// 'ぁ'
		codeHiraganaN =			0x30001273,		// 'ん'
		codeKatakanaXa =		0x300012a1,		// 'ァ'
		codeKatakanaXke =		0x300012f6,		// 'ヶ'
		codeKanjiFirstStart =	0x30001821,		// '亜'
		codeKanjiFirstEnd =		0x300027d3,		// '腕'
		codeKanjiSecondStart =	0x30002821,		// '弌'
		codeKanjiSecondEnd =	0x30003a26		// '熙'
	};

	// 文字列の最後を指す文字を返す
	static ModWideChar null();

	// 文字変換
	static ModWideChar toUpper(const ModWideChar character);
	static ModWideChar toLower(const ModWideChar character);

	ModCommonDLL
	static int				compare(const ModWideChar* l, const ModWideChar* r,
									ModSize len = 0);
	ModCommonDLL
	static int				compare(const ModWideChar* l, const ModWideChar* r,
									ModBoolean caseSensitive, ModSize len = 0);
												// ワイド文字配列どうしを
												// 比較する

	ModCommonDLL
	static ModWideChar*		find(const ModWideChar* s, ModWideChar v,
								 ModSize len = 0);
	ModCommonDLL
	static ModWideChar*		find(const ModWideChar* s, ModWideChar v,
								 ModBoolean caseSensitive, ModSize len = 0);
												// ワイド文字配列中の
												// ある文字を探す

	ModCommonDLL
	static ModSize			length(const ModWideChar* s);
												// ワイド文字配列中の
												// 文字数を得る
	ModCommonDLL
	static ModSize			length(const char* s,
								   ModKanjiCode::KanjiCodeType code =
											ModKanjiCode::euc);
												// C 文字列をワイド文字列へ
												// 変換したときの文字数を得る

	//コピーする
	ModCommonDLL
	static ModWideChar* copy(ModWideChar* destination,
							 const ModWideChar* source,
							 const ModSize maxLength = 0);

	// char* の指すバイト列の指すコードを ModWideChar に変換する
	ModCommonDLL
	static const ModWideChar makeWideChar(
		const char* pointer,
		ModKanjiCode::KanjiCodeType kanjiCode = ModKanjiCode::euc
		);
	// EUCコードを格納した int を ModWideChar に変換する
	ModCommonDLL
	static const ModWideChar makeWideChar(int code);

	ModCommonDLL
	static ModSize			byteSize(ModWideChar v);
												// ワイド文字の EUC としての
												// バイト数を得る

	// char* の指すバイト列の指すコードのEUCとしてのバイト長を得る
	ModCommonDLL
	static const ModSize byteSize(
		const char* pointer,
		ModKanjiCode::KanjiCodeType kanjiCode = ModKanjiCode::euc
		);

	// ModWideChar の表現する文字を指定した char* バッファに入れる
	ModCommonDLL
	static ModSize convertToString(
		char* target, ModWideChar character,
		ModKanjiCode::KanjiCodeType kanjiCode = ModKanjiCode::euc
		);

	// ModWideChar の表現する文字を EUC コードを格納した int で得る
	ModCommonDLL
	static int convertToLong(const ModWideChar character);
	// マルチバイト文字の EUC コードを int で得る
	ModCommonDLL
	static int convertToLong(
		const char* character,
		ModKanjiCode::KanjiCodeType kanjiCode = ModKanjiCode::euc
		);

	// 文字種を得る関数
	ModCommonDLL
	static const ModWideCharType getType(const ModWideChar character);

	// 文字種判定関数
	// 英字か否かの判定
	static ModBoolean isAlphabet(const ModWideChar character);
	static ModBoolean isAlphabet(const ModWideCharType type);
	// 数字か否かの判定
	static ModBoolean isDigit(const ModWideChar character);
	static ModBoolean isDigit(const ModWideCharType type);
	// 記号か否かの判定
	static ModBoolean isSymbol(const ModWideChar character);
	static ModBoolean isSymbol(const ModWideCharType type);
	// 空白か否かの判定
	static ModBoolean isSpace(const ModWideChar character);
	static ModBoolean isSpace(const ModWideCharType type);
	// ASCII文字(7bit 1バイトコード)か否かの判定
	static ModBoolean isAscii(const ModWideChar character);
	static ModBoolean isAscii(const ModWideCharType type);
	// 半角カタカナか否かの判定
	static ModBoolean isHankakuKana(const ModWideChar character);
	static ModBoolean isHankakuKana(const ModWideCharType type);
	// ひらがなか否かの判定
	static ModBoolean isHiragana(const ModWideChar character);
	static ModBoolean isHiragana(const ModWideCharType type);
	// カタカナか否かの判定
	static ModBoolean isKatakana(const ModWideChar character);
	static ModBoolean isKatakana(const ModWideCharType type);
	// ギリシャ文字か否かの判定
	static ModBoolean isGreek(const ModWideChar character);
	static ModBoolean isGreek(const ModWideCharType type);
	// ロシア文字か否かの判定
	static ModBoolean isRussian(const ModWideChar character);
	static ModBoolean isRussian(const ModWideCharType type);
	// 罫線か否かの判定
	static ModBoolean isLine(const ModWideChar character);
	static ModBoolean isLine(const ModWideCharType type);
	// 漢字第一水準か否かの判定
	static ModBoolean isKanjiFirst(const ModWideChar character);
	static ModBoolean isKanjiFirst(const ModWideCharType type);
	// 漢字第二水準か否かの判定
	static ModBoolean isKanjiSecond(const ModWideChar character);
	static ModBoolean isKanjiSecond(const ModWideCharType type);
	// 漢字か否かの判定
	static ModBoolean isKanji(const ModWideChar character);
	static ModBoolean isKanji(const ModWideCharType type);
	// 外字か否かの判定
	static ModBoolean isGaiji(const ModWideChar character);
	static ModBoolean isGaiji(const ModWideCharType type);

	// JISコードの区を求める
	static unsigned int getJisSection(const ModWideChar character);
	// JISコードの点を求める
	static unsigned int getJisPoint(const ModWideChar character);

	// 壊れているかどうか調べる
	static ModBoolean isIllegal(const ModWideChar character);

private:
	enum
	{
		// ワイド文字のコードを調べるマスク
		//
		// この定数はワイド文字がどのコードセットに対応するものかを
		// 判定するために用いる。
		// ワイド文字とこの定数で and をとったもので判断する。
		// 現在は 4 バイト中の上位 11 ビットのみ残したものになる。

		codeMask =				0xffe00000
	};
};

//
// FUNCTION 
// ModWideCharTrait::null -- 文字列のターミネイタに使われる文字を得る
//
// NOTES
// この関数は ModWideChar で構成される文字列のターミネイタに使われる
// ModWideChar 型の値を得るのに用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// ターミネイタに使われる ModWideChar 型の値を返す。
//
// EXCEPTIONS
// なし
//
inline
ModWideChar
ModWideCharTrait::null()
{
	return ModWideChar(0);
}

//
// FUNCTION
// ModWideCharTrait::toUpper -- ModWideChar の大文字への変換
//
// NOTES
// この関数は ModWideChar の英/ギリシャ/ロシア小文字を
// 対応する大文字に変換するのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		大文字に変換するワイド文字
//
// RETURN
// character が英/ギリシャ/ロシア小文字の場合は対応する大文字を返し、
// それ以外の場合は character 自身のコピーを返す。
//
// EXCEPTIONS
// なし
//
inline
ModWideChar
ModWideCharTrait::toUpper(const ModWideChar character)
{
	return ((character & codeMask) == ModCodeMaskSet0)
		// ASCII の小文字('a'〜'z')
		?
		((unsigned int)(character - codeAsciiSmallA)
		 <= (unsigned int)(codeAsciiSmallZ - codeAsciiSmallA)
		 ? character - (codeAsciiSmallA - codeAsciiLargeA)
		 : character)
		:
		(((character & codeMask) == ModCodeMaskSet1)
		 // EUC 英字の小文字('ａ'〜'ｚ')
		 ?
		 ((getJisSection(character) == 3 && getJisPoint(character) >= 65)
		  ? character - (codeEUCSmallA - codeEUCLargeA)
		  : ((getJisSection(character) == 6 && getJisPoint(character) >= 33)
			 ? character - (codeGreekSmallA - codeGreekLargeA)
			 : ((getJisSection(character) == 7 && getJisPoint(character) >= 49)
				? character - (codeRussianSmallA - codeRussianLargeA)
				: character)
			 )
		  )
		 :
		 character);
}

//
// FUNCTION
// ModWideCharTrait::toLower -- ModWideChar の小文字への変換
//
// NOTES
// この関数は ModWideChar の英/ギリシャ/ロシア大文字を
// 対応する小文字に変換するのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		小文字に変換するワイド文字
//
// RETURN
// character が英/ギリシャ/ロシア大文字の場合は対応する小文字を返し、
// それ以外の場合は character 自身のコピーを返す。
//
// EXCEPTIONS
// なし
//
inline
ModWideChar
ModWideCharTrait::toLower(const ModWideChar character)
{
	return ((character & codeMask) == ModCodeMaskSet0)
		// ASCII の大文字('A'〜'Z')
		?
		(((unsigned int)(character - codeAsciiLargeA)
		  <= (unsigned int)(codeAsciiLargeZ - codeAsciiLargeA))
		 ? character + (codeAsciiSmallA - codeAsciiLargeA)
		 : character)
		:
		(((character & codeMask) == ModCodeMaskSet1)
		 // EUC 英字の大文字('Ａ'〜'Ｚ')
		 ?
		 ((getJisSection(character) == 3
		   && (unsigned int)(getJisPoint(character) - 33) <= 26)
		  ? character + (codeEUCSmallA - codeEUCLargeA)
		  : ((getJisSection(character) == 6 && getJisPoint(character) <= 24)
			 ? character + (codeGreekSmallA - codeGreekLargeA)
			 : ((getJisSection(character) == 7 && getJisPoint(character) <= 33)
				? character + (codeRussianSmallA - codeRussianLargeA)
				: character)
			 )
		  )
		 : character);
}

//
// FUNCTION
// ModWideCharTrait::isAlphabet -- 英字かどうかを判定する
//
// NOTES
// この関数は ModWideChar で表される文字が英字かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		判定の対象となる文字
//
// RETURN
// 指定した文字が ASCII の英字、または EUC 2 バイトコードの英字の場合は
// ModTrue を返し、それ以外の場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModWideCharTrait::isAlphabet(const ModWideChar character)
{
	return (((character & codeMask) == ModCodeMaskSet0 &&
			 ((unsigned int) (character - codeAsciiLargeA) <=
			  (unsigned int) (codeAsciiLargeZ - codeAsciiLargeA) ||
			  (unsigned int) (character - codeAsciiSmallA) <=
			  (unsigned int) (codeAsciiSmallZ - codeAsciiSmallA))) ||
			((character & codeMask) == ModCodeMaskSet1 &&
			 getJisSection(character) == 3 && getJisPoint(character) >= 33)) ?
		ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isAlphabet -- 文字種が英字を示すかどうかを判定する
//
// NOTES
// この関数は ModWideCharType で表される文字種が英字を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModWideCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種に英字を表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isAlphabet(const ModWideCharType type)
{
	return ((type & ModWideCharTrait::alphabet) == ModWideCharTrait::alphabet)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isDigit -- 数字かどうかを判定する
//
// NOTES
// この関数は ModWideChar で表される文字が数字かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		判定の対象となる文字
//
// RETURN
// 指定した文字が ASCII の数字、または EUC 2 バイトコードの数字の場合は
// ModTrue を返し、それ以外の場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModWideCharTrait::isDigit(const ModWideChar character)
{
	return (((character & codeMask) == ModCodeMaskSet0 &&
			 ((unsigned int) (character - codeAsciiZero) <=
			  (unsigned int) (codeAsciiNine - codeAsciiZero))) ||
			((character & codeMask) == ModCodeMaskSet1 &&
			 getJisSection(character) == 3 &&
			 (unsigned int) (getJisPoint(character) - 16) <= 9)) ?
		ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isDigit -- 文字種が数字を示すかどうかを判定する
//
// NOTES
// この関数は ModWideCharType で表される文字種が数字を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModWideCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種に数字を表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isDigit(const ModWideCharType type)
{
	return ((type & ModWideCharTrait::digit) == ModWideCharTrait::digit)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isSymbol -- 記号かどうかを判定する
//
// NOTES
// この関数は ModWideChar で表される文字が記号かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		判定の対象となる文字
//
// RETURN
// 指定した文字が ASCII の記号、または EUC 2 バイトコードの記号の場合は
// ModTrue を返し、それ以外の場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModWideCharTrait::isSymbol(const ModWideChar character)
{
	return ((character & codeMask) == ModCodeMaskSet0) ?
		ModCharTrait::isSymbol((char) (character & 0xff)) :
		((((character & codeMask) == ModCodeMaskSet2 &&
		   ((character >= 0x10000021 &&			// 半角カタカナの句点
			 character <= 0x10000025) ||		// 半角カタカナの中点
			(character >= 0x1000005e &&			// 半角カタカナの濁点
			 character <= 0x1000005f) ||		// 半角カタカナの半濁点
			(character == 0x10000030))) ||		// 半角カタカナの長音
		  ((character & codeMask) == ModCodeMaskSet1 &&
		   (getJisSection(character) <= 2 || getJisSection(character) == 8))) ?
		 ModTrue : ModFalse);
}

//
// FUNCTION
// ModWideCharTrait::isSymbol -- 文字種が記号を示すかどうかを判定する
//
// NOTES
// この関数は ModWideCharType で表される文字種が記号を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModWideCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種に記号を表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isSymbol(const ModWideCharType type)
{
	return ((type & ModWideCharTrait::symbol) == ModWideCharTrait::symbol)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isAscii -- ASCIIコードかどうかを判定する
//
// NOTES
// この関数は ModWideChar で表される文字がASCIIコードかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		判定の対象となる文字
//
// RETURN
// 指定した文字が ASCIIコードの場合はModTrue を返し、
// それ以外の場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isAscii(const ModWideChar character)
{
	return ((character & codeMask) == ModCodeMaskSet0)
		?ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isAscii -- 文字種がASCIIコードを示すかどうかを判定する
//
// NOTES
// この関数は ModWideCharType で表される文字種がASCIIコードを示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModWideCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種にASCIIコードを表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isAscii(const ModWideCharType type)
{
	return ((type & ModWideCharTrait::ascii) == ModWideCharTrait::ascii)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isSpace -- 空白かどうかを判定する
//
// NOTES
// この関数は ModWideChar で表される文字が空白かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		判定の対象となる文字
//
// RETURN
// 指定した文字が ASCII の空白、または EUC 2 バイトコードの空白の場合は
// ModTrue を返し、それ以外の場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModWideCharTrait::isSpace(const ModWideChar character)
{
	return ((character & codeMask) == ModCodeMaskSet0) ?
		ModCharTrait::isSpace((char) (character & 0xff)) :
		((character & codeMask) == ModCodeMaskSet1 &&
		 character == codeEUCSpace) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isSpace -- 文字種が空白を示すかどうかを判定する
//
// NOTES
// この関数は ModWideCharType で表される文字種が空白を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModWideCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種に空白を表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isSpace(const ModWideCharType type)
{
	return ((type & ModWideCharTrait::space) == ModWideCharTrait::space)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isHankakuKana -- 半角カナかどうかを判定する
//
// NOTES
// この関数は ModWideChar で表される文字が半角カナかどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		判定の対象となる文字
//
// RETURN
// 指定した文字が半角カナの場合はModTrue を返し、
// それ以外の場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isHankakuKana(const ModWideChar character)
{
	return ((character & codeMask) == ModCodeMaskSet2)
		? ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isHankakuKana -- 文字種が半角カナを示すかどうかを判定する
//
// NOTES
// この関数は ModWideCharType で表される文字種が半角カナを示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModWideCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種に半角カナを表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isHankakuKana(const ModWideCharType type)
{
	return ((type & ModWideCharTrait::hankakuKana) == ModWideCharTrait::hankakuKana)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isHiragana -- ひらがなかどうかを判定する
//
// NOTES
// この関数は ModWideChar で表される文字がひらがなかどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		判定の対象となる文字
//
// RETURN
// 指定した文字が EUC 2 バイトコードのひらがなの場合は
// ModTrue を返し、それ以外の場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModWideCharTrait::isHiragana(const ModWideChar character)
{
	return ((character & codeMask) == ModCodeMaskSet1 &&
			ModWideCharTrait::getJisSection(character) == 4) ?
		ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isHiragana -- 文字種がひらがなを示すかどうかを判定する
//
// NOTES
// この関数は ModWideCharType で表される文字種がひらがなを示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModWideCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種にひらがなを表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isHiragana(const ModWideCharType type)
{
	return ((type & ModWideCharTrait::hiragana) == ModWideCharTrait::hiragana)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isKatakana -- カタカナかどうかを判定する
//
// NOTES
// この関数は ModWideChar で表される文字がカタカナかどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		判定の対象となる文字
//
// RETURN
// 指定した文字が EUC 2 バイトコードのカタカナの場合は
// ModTrue を返し、それ以外の場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModWideCharTrait::isKatakana(const ModWideChar character)
{
	return (((character & codeMask) == ModCodeMaskSet1 &&
			 getJisSection(character) == 5) ||
			((character & codeMask) == ModCodeMaskSet2 &&
			 ((unsigned int) (character - 0x10000026) <=
			  (unsigned int) (0x1000005d - 0x10000026) ||
			  character == 0x1000005e || character == 0x1000005f))) ?
		ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isKatakana -- 文字種がカタカナを示すかどうかを判定する
//
// NOTES
// この関数は ModWideCharType で表される文字種がカタカナを示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModWideCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種にカタカナを表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isKatakana(const ModWideCharType type)
{
	return ((type & ModWideCharTrait::katakana) == ModWideCharTrait::katakana)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isGreek -- ギリシャ文字かどうかを判定する
//
// NOTES
// この関数は ModWideChar で表される文字がギリシャ文字かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		判定の対象となる文字
//
// RETURN
// 指定した文字が EUC 2 バイトコードのギリシャ文字の場合は
// ModTrue を返し、それ以外の場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModWideCharTrait::isGreek(const ModWideChar character)
{
	return ((character & codeMask) == ModCodeMaskSet1 &&
			ModWideCharTrait::getJisSection(character) == 6) ?
		ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isGreek -- 文字種がギリシャ文字を示すかどうかを判定する
//
// NOTES
// この関数は ModWideCharType で表される文字種がギリシャ文字を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModWideCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種にギリシャ文字を表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isGreek(const ModWideCharType type)
{
	return ((type & ModWideCharTrait::greek) == ModWideCharTrait::greek)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isRussian -- ロシア文字かどうかを判定する
//
// NOTES
// この関数は ModWideChar で表される文字がロシア文字かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		判定の対象となる文字
//
// RETURN
// 指定した文字が EUC 2 バイトコードのロシア文字の場合は
// ModTrue を返し、それ以外の場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModWideCharTrait::isRussian(const ModWideChar character)
{
	return ((character & codeMask) == ModCodeMaskSet1 &&
			ModWideCharTrait::getJisSection(character) == 7) ?
		ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isRussian -- 文字種がロシア文字を示すかどうかを判定する
//
// NOTES
// この関数は ModWideCharType で表される文字種がロシア文字を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModWideCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種にロシア文字を表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isRussian(const ModWideCharType type)
{
	return ((type & ModWideCharTrait::russian) == ModWideCharTrait::russian)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isLine -- 罫線かどうかを判定する
//
// NOTES
// この関数は ModWideChar で表される文字が罫線かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		判定の対象となる文字
//
// RETURN
// 指定した文字が EUC 2 バイトコードの罫線の場合は
// ModTrue を返し、それ以外の場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModWideCharTrait::isLine(const ModWideChar character)
{
	return ((character & codeMask) == ModCodeMaskSet1 &&
			ModWideCharTrait::getJisSection(character) == 8) ?
		ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isLine -- 文字種が罫線を示すかどうかを判定する
//
// NOTES
// この関数は ModWideCharType で表される文字種が罫線を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModWideCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種に罫線を表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isLine(const ModWideCharType type)
{
	return ((type & ModWideCharTrait::line) == ModWideCharTrait::line)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isKanjiFirst -- 第一水準漢字かどうかを判定する
//
// NOTES
// この関数は ModWideChar で表される文字が第一水準漢字かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		判定の対象となる文字
//
// RETURN
// 指定した文字が EUC 2 バイトコードの第一水準漢字の場合は
// ModTrue を返し、それ以外の場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModWideCharTrait::isKanjiFirst(const ModWideChar character)
{
	return ((character & codeMask) == ModCodeMaskSet1 &&
			(unsigned int)
			(ModWideCharTrait::getJisSection(character) - 16) <= (47 - 16)) ?
		ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isKanjiFirst -- 文字種が第一水準漢字を示すかどうかを判定する
//
// NOTES
// この関数は ModWideCharType で表される文字種が第一水準漢字を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModWideCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種に第一水準漢字を表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isKanjiFirst(const ModWideCharType type)
{
	return ((type & ModWideCharTrait::kanjiFirst) == ModWideCharTrait::kanjiFirst)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isKanjiSecond -- 第二水準漢字かどうかを判定する
//
// NOTES
// この関数は ModWideChar で表される文字が第二水準漢字かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		判定の対象となる文字
//
// RETURN
// 指定した文字が EUC 2 バイトコードの第二水準漢字の場合は
// ModTrue を返し、それ以外の場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModWideCharTrait::isKanjiSecond(const ModWideChar character)
{
	return ((character & codeMask) == ModCodeMaskSet1 &&
			(unsigned int)
			(ModWideCharTrait::getJisSection(character) - 48) <= (84 - 48)) ?
		ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isKanjiSecond -- 文字種が第二水準漢字を示すかどうかを判定する
//
// NOTES
// この関数は ModWideCharType で表される文字種が第二水準漢字を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModWideCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種に第二水準漢字を表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isKanjiSecond(const ModWideCharType type)
{
	return ((type & ModWideCharTrait::kanjiSecond) == ModWideCharTrait::kanjiSecond)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isKanji -- 漢字かどうかを判定する
//
// NOTES
// この関数は ModWideChar で表される文字が漢字かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		判定の対象となる文字
//
// RETURN
// 指定した文字が EUC 2 バイトコードの漢字の場合は
// ModTrue を返し、それ以外の場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModWideCharTrait::isKanji(const ModWideChar character)
{
	return (ModWideCharTrait::isKanjiFirst(character) ||
			ModWideCharTrait::isKanjiSecond(character)) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isKanji -- 文字種が漢字を示すかどうかを判定する
//
// NOTES
// この関数は ModWideCharType で表される文字種が漢字を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModWideCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種に漢字を表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isKanji(const ModWideCharType type)
{
	return ((type & ModWideCharTrait::kanji) == ModWideCharTrait::kanji)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isGaiji -- 外字かどうかを判定する
//
// NOTES
// この関数は ModWideChar で表される文字が外字かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		判定の対象となる文字
//
// RETURN
// 指定した文字が SS3(外字)の場合はModTrue を返し、
// それ以外の場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isGaiji(const ModWideChar character)
{
	return ((character & codeMask) == ModCodeMaskSet3)
		? ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::isGaiji -- 文字種が外字を示すかどうかを判定する
//
// NOTES
// この関数は ModWideCharType で表される文字種が外字を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModWideCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種に外字を表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isGaiji(const ModWideCharType type)
{
	return ((type & ModWideCharTrait::gaiji) == ModWideCharTrait::gaiji)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModWideCharTrait::getJisSection -- JISコードの区を求める
//
// NOTES
// この関数はModWideCharで与えられた文字がJISコードの第何区にあるのかを
// 返す関数である。
//
// ARGUMENTS
// const ModWideChar character
//		判定される文字
//
// RETURN VALUE
// JISコードの範囲にあればJISコードの区を返す。
// ASCIIやSS2、SS3のときは不定である。
//
// EXCEPTIONS
// なし
//
inline
unsigned int
ModWideCharTrait::getJisSection(const ModWideChar character)
{
	return (unsigned int)(((unsigned int)(character & 0x7f80) >> 7) - 0x20);
}

//
// FUNCTION
// ModWideCharTrait::getJisPoint -- JISコードの点を求める
//
// NOTES
// この関数はModWideCharで与えられた文字がJISコードの対応する区点の点の方を
// 返す関数である。
//
// ARGUMENTS
// const ModWideChar character
//		判定される文字
//
// RETURN VALUE
// JISコードの範囲にあればJISコードの点を返す。
// ASCIIやSS2、SS3のときは不定である。
//
// EXCEPTIONS
// なし
//
inline
unsigned int
ModWideCharTrait::getJisPoint(const ModWideChar character)
{
	return (unsigned int)((character & 0x7f) - 0x20);
}

//
// FUNCTION
// ModWideCharTrait::isIllegal -- 壊れたワイド文字か調べる
//
// NOTES
// この関数はModWideCharで与えられた文字が壊れたワイド文字かを
// 返す関数である。
//
// ARGUMENTS
// const ModWideChar character
//		判定される文字
//
// RETURN VALUE
// SS1なのに1バイト分しかないなどの不正なコードから作成されたワイド文字に
// 対して ModTrue を返し、正当なコードに対しては ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModWideCharTrait::isIllegal(const ModWideChar character)
{
	return ((character & codeMask) >= ModCodeMaskSet1X) ? ModTrue : ModFalse;
}

#endif	// __ModWideCharTrait_H__

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
