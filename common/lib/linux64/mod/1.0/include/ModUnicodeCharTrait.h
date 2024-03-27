// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModUnicodeCharTrait.h -- ModUnicodeCharTrait のクラス定義
// 
// Copyright (c) 1999, 2002, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModUnicodeCharTrait_H__
#define __ModUnicodeCharTrait_H__

#include "ModCommonDLL.h"
#include "ModCommon.h"
#include "ModUnicodeChar.h"
#include "ModUnicodeCharTypes.h"

//
// CLASS
// ModUnicodeCharTrait -- Unicode文字の特性を表現するクラス
//
// NOTES
// ModUnicodeChar で表された Unicode(UCS-2) 文字の特性を表現するクラス。
// 文字種の判定や文字列操作(比較、探索、コピーなど)を行なうメソッドを
// 提供する。
//
// 本クラスのメソッドは全て static なので、本クラスのインスタンスを
// 作成しても無意味である。
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModUnicodeCharTrait
{
public:
	// 文字種と大文字/小文字変換先のテーブルのための構造体。
	// コンパイルするために public にしたが、普通のユーザーがこの構造体を
	// 利用することは禁止する。
	struct CharacterInformatin
	{
		ModUnicodeCharType	type;
		ModUnicodeChar		mappedCode;
	};

	// 文字列の最後を指す文字を返す
	static ModUnicodeChar	null();

	// 無効(Unicode で使われないことが保証されている)文字を返す
	static ModUnicodeChar	undefined();	

	// 文字変換
	static ModUnicodeChar	toUpper(const ModUnicodeChar character);
	static ModUnicodeChar	toLower(const ModUnicodeChar character);

	// Unicode 文字列(配列)の比較
	ModCommonDLL
	static int				compare(const ModUnicodeChar*	l,
									const ModUnicodeChar*	r);

	ModCommonDLL
	static int				compare(const ModUnicodeChar*	l,
									const ModUnicodeChar*	r,
									const ModSize			len);

	ModCommonDLL
	static int				compare(const ModUnicodeChar*	l,
									const ModUnicodeChar*	r,
									const ModBoolean		caseSensitive,
									const ModSize			len = 0);

	// Unicode文字列(配列)からある文字を探す
	ModCommonDLL
	static ModUnicodeChar*	find(const ModUnicodeChar*	s,
								 const ModUnicodeChar	v,
								 const ModSize			len = 0);

	ModCommonDLL
	static ModUnicodeChar*	find(const ModUnicodeChar*	s,
								 const ModUnicodeChar	v,
								 const ModBoolean		caseSensitive,
								 const ModSize			len = 0);

	// Unicode文字列(配列)からある文字列を探す
	ModCommonDLL
	static ModUnicodeChar*	find(const ModUnicodeChar*	s1,
								 const ModUnicodeChar*	s2,
								 const ModSize			len = 0);

	ModCommonDLL
	static ModUnicodeChar*	find(const ModUnicodeChar*	s1,
								 const ModUnicodeChar*	s2,
								 const ModBoolean		caseSensitive,
								 const ModSize			len = 0);

	// 文字列(配列)をコピーする
	ModCommonDLL
	static ModUnicodeChar*	copy(ModUnicodeChar*		destination,
								 const ModUnicodeChar*	source,
								 const ModSize			maxLength = 0);

	// Unicode文字列(配列)から文字数を得る
	ModCommonDLL
	static ModSize			length(const ModUnicodeChar* s);

	// int 型の数値への変換(ASCII 文字列にのみ対応)
	ModCommonDLL
	static int				toInt(const ModUnicodeChar*	string,
								  const int				base = 10);
	ModCommonDLL
	static unsigned int		toUInt(const ModUnicodeChar* string,
								   const int			base = 10);

	// short 型の数値への変換(ASCII 文字列にのみ対応)
	ModCommonDLL
	static short			toShort(const ModUnicodeChar* string,
									const int			base = 10);
	ModCommonDLL
	static unsigned short	toUShort(const ModUnicodeChar* string,
									 const int			base = 10);

	// long 型の数値への変換(ASCII 文字列にのみ対応)
	ModCommonDLL
	static long				toLong(const ModUnicodeChar* string,
								   const int			base = 10);
	ModCommonDLL
	static unsigned long	toULong(const ModUnicodeChar* string,
									const int			base = 10);

	// ModInt64 型の数値への変換(ASCII 文字列にのみ対応)
	ModCommonDLL
	static ModInt64			toModInt64(const ModUnicodeChar* string,
									   const int		base = 10);
	ModCommonDLL
	static ModUInt64		toModUInt64(const ModUnicodeChar* string,
										const int		base = 10);

	// float 型の数値への変換(ASCII 文字列にのみ対応)
	static float			toFloat(const ModUnicodeChar* string);

	// double 型の数値への変換(ASCII 文字列にのみ対応)
	ModCommonDLL
	static double			toDouble(const ModUnicodeChar* string);

	// 文字種を得る関数
	static const ModUnicodeCharType getType(const ModUnicodeChar character);

	//
	// 文字種判定関数
	//

	// 該当する文字種がないか否かの判定
	static ModBoolean isNone(const ModUnicodeChar character);
	static ModBoolean isNone(const ModUnicodeCharType type);

	// Unicode に存在する文字か否かの判定
	static ModBoolean isNotused(const ModUnicodeChar character);
	static ModBoolean isNotused(const ModUnicodeCharType type);

	// 英字か否かの判定
	static ModBoolean isAlphabet(const ModUnicodeChar character);
	static ModBoolean isAlphabet(const ModUnicodeCharType type);

	// 大文字か否かの判定
	static ModBoolean isUpper(const ModUnicodeChar character);
	static ModBoolean isUpper(const ModUnicodeCharType type);

	// 小文字か否かの判定
	static ModBoolean isLower(const ModUnicodeChar character);
	static ModBoolean isLower(const ModUnicodeCharType type);

	// 英字以外の文字か否かの判定
	static ModBoolean isLetterOther(const ModUnicodeChar character);
	static ModBoolean isLetterOther(const ModUnicodeCharType type);

	// 数字か否かの判定(10進)
	static ModBoolean isDigit(const ModUnicodeChar character);
	static ModBoolean isDigit(const ModUnicodeCharType type);

	// 数字か否かの判定(16進)
	static ModBoolean isXdigit(const ModUnicodeChar character);
	static ModBoolean isXdigit(const ModUnicodeCharType type);

	// 記号か否かの判定
	static ModBoolean isSymbol(const ModUnicodeChar character);
	static ModBoolean isSymbol(const ModUnicodeCharType type);

	// 空白か否かの判定
	static ModBoolean isSpace(const ModUnicodeChar character);
	static ModBoolean isSpace(const ModUnicodeCharType type);

	// ASCII文字(7bit 1バイトコード)か否かの判定
	static ModBoolean isAscii(const ModUnicodeChar character);
	static ModBoolean isAscii(const ModUnicodeCharType type);

	// 半角カタカナか否かの判定
	static ModBoolean isHankakuKana(const ModUnicodeChar character);
	static ModBoolean isHankakuKana(const ModUnicodeCharType type);

	// ひらがなか否かの判定
	static ModBoolean isHiragana(const ModUnicodeChar character);
	static ModBoolean isHiragana(const ModUnicodeCharType type);

	// カタカナか否かの判定
	static ModBoolean isKatakana(const ModUnicodeChar character);
	static ModBoolean isKatakana(const ModUnicodeCharType type);

	// ギリシャ文字か否かの判定
	static ModBoolean isGreek(const ModUnicodeChar character);
	static ModBoolean isGreek(const ModUnicodeCharType type);

	// ロシア文字か否かの判定
	static ModBoolean isRussian(const ModUnicodeChar character);
	static ModBoolean isRussian(const ModUnicodeCharType type);

	// 罫線か否かの判定
	static ModBoolean isLine(const ModUnicodeChar character);
	static ModBoolean isLine(const ModUnicodeCharType type);

	// 漢字か否かの判定
	static ModBoolean isKanji(const ModUnicodeChar character);
	static ModBoolean isKanji(const ModUnicodeCharType type);

	// コントロールか否かの判定
	static ModBoolean isControl(const ModUnicodeChar character);
	static ModBoolean isControl(const ModUnicodeCharType type);

	// フォーマットか否かの判定
	static ModBoolean isFormat(const ModUnicodeChar character);
	static ModBoolean isFormat(const ModUnicodeCharType type);

	// サロゲートか否かの判定
	static ModBoolean isSurrogate(const ModUnicodeChar character);
	static ModBoolean isSurrogate(const ModUnicodeCharType type);

	// 外字か否かの判定
	static ModBoolean isGaiji(const ModUnicodeChar character);
	static ModBoolean isGaiji(const ModUnicodeCharType type);

private:

	// 文字列を整数値(いちばん大きな値を表現できる整数型)に変換。
	// to*() 関数の下請けとして使用
	static void toInteger(const ModUnicodeChar*	string_,
						  const int				base_,
						  ModUInt64&				value_,
						  ModBoolean&			negative_);

	ModCommonDLL
	static const CharacterInformatin s_table[];
};

//
// FUNCTION 
// ModUnicodeCharTrait::null -- 文字列のターミネイタに使われる文字を得る
//
// NOTES
// この関数は ModUnicodeChar で構成される文字列のターミネイタに使われる
// ModUnicodeChar 型の値を得るのに用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// ターミネイタに使われる ModUnicodeChar 型の値を返す。
//
// EXCEPTIONS
// なし
//
inline ModUnicodeChar
ModUnicodeCharTrait::null()
{
	return ModUnicodeChar(0x0000);
}

//
// FUNCTION 
// ModUnicodeCharTrait::undefined -- 無効(Unicode で使われないことが保証されている)文字を返す
//
// NOTES
// この関数は Unicode で使われないことが保証されている文字を返す。
// この文字の使い方は自由である。
//
// ARGUMENTS
// なし
//
// RETURN
// Unicode で使われないことが保証されている文字
//
// EXCEPTIONS
// なし
//
inline ModUnicodeChar
ModUnicodeCharTrait::undefined()
{
	return ModUnicodeChar(0xffff);
}

//
// FUNCTION
// ModUnicodeCharTrait::toUpper -- ModUnicodeChar の大文字への変換
//
// NOTES
// この関数は ModUnicodeChar の小文字を対応する大文字に変換するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		大文字に変換する文字
//
// RETURN
// character が対応する大文字を返す。対応するものがなければ character 自身
// のコピーを返す。
//
// EXCEPTIONS
// なし
//
inline ModUnicodeChar
ModUnicodeCharTrait::toUpper(const ModUnicodeChar character)
{
	if ((s_table[character].type & ModUnicodeCharTypes::lower)
		&& s_table[character].mappedCode != 0){
		// 変換先の大文字が存在するとは限らないことに注意
		; ModAssert(s_table[character].mappedCode != character);
		return s_table[character].mappedCode;
	}
	return character;
}

//
// FUNCTION
// ModUnicodeCharTrait::toLower -- ModUnicodeChar の小文字への変換
//
// NOTES
// この関数は ModUnicodeChar の大文字を対応する小文字に変換するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		小文字に変換する文字
//
// RETURN
// character が対応する小文字を返す。対応するものがなければ character 自身
// のコピーを返す。
//
// EXCEPTIONS
// なし
//
inline ModUnicodeChar
ModUnicodeCharTrait::toLower(const ModUnicodeChar character)
{
	if ((s_table[character].type & ModUnicodeCharTypes::upper)
		&& s_table[character].mappedCode != 0) {
		// 変換先の小文字が存在するとは限らないことに注意
		; ModAssert(s_table[character].mappedCode != character);
		return s_table[character].mappedCode;
	}
	return character;
}

//
// FUNCTION
// ModUnicodeCharTrait::isNone -- 文字種が無いのかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字に文字種が無いのかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// 文字種が無い文字であると判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isNone(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::none) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isNone -- 文字種が無いのかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字に文字種が無いのか
// を判定するのに用いる。この場合の文字種の値はゼロである事を保証する。
//
// ARGUMENTS
// const ModUnicodeCharType type
//		判定の対象となる文字種
//
// RETURN
// ビットがひとつも立っていない場合は ModTrue を返し、
// 何らかの文字種を表すビットが立っている場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModUnicodeCharTrait::isNone(const ModUnicodeCharType type)
{
	; ModAssert(ModUnicodeCharTypes::none == 0);
	return ModBoolean((type & ModUnicodeCharTypes::none) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isNotused -- Unicode で未使用かどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字が Unicode で未使用かどうかを
// 判定するのに用いる。Unicode 文字表にはかなりの未使用領域が存在する。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// 未使用と判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isNotused(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::notused) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isNotused -- Unicode で未使用かどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種が未使用を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種に未使用を表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModUnicodeCharTrait::isNotused(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::notused) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isAlphabet -- 英字かどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字が英字かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// 英字と判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isAlphabet(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::alphabet) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isAlphabet -- 文字種が英字を示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種が英字を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
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
ModUnicodeCharTrait::isAlphabet(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::alphabet) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isUpper -- 大文字かどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字が大文字かどうかを判定するのに
// 用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// 大文字と判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isUpper(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::upper) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isUpper -- 文字種が大文字を示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種が大文字を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種に大文字を表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModUnicodeCharTrait::isUpper(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::upper) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isLower -- 小文字かどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字が小文字かどうかを判定するのに
// 用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// 小文字と判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isLower(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::lower) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isLower -- 文字種が小文字を示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種が小文字を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種に小文字を表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModUnicodeCharTrait::isLower(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::lower) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isLetterOther -- 英字以外の文字かどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字が英字以外の文字かどうかを
// 判定するのに用いる。
//
// 英字以外の文字とは、日本語やアラビア語などの文字を意味する。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// 英字以外の文字と判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isLetterOther(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::letterOther) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isLetterOther -- 文字種が英字以外の文字を示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種が英字以外の文字を示すか
// どうかを判定するのに用いる。
//
// 英字以外の文字とは、日本語やアラビア語などの文字を意味する。
//
// ARGUMENTS
// const ModUnicodeCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種に英字以外の文字を表すビットが立っている場合は ModTrue
// を返し、立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModUnicodeCharTrait::isLetterOther(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::letterOther) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isDigit -- 数字かどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字が数字かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// 数字と判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isDigit(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::digit) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isDigit -- 文字種が数字を示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種が数字を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種に数字を表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModUnicodeCharTrait::isDigit(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::digit) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isDigit -- 数字かどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字が数字(16進)かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// 数字と判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isXdigit(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::xdigit) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isXdigit -- 文字種が数字(16進)を示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種が数字(16進)を示すか
// どうかを判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種に数字(16進)を表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModUnicodeCharTrait::isXdigit(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::xdigit) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isSymbol -- 記号かどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字が記号かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// 記号と判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isSymbol(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::symbol) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isSymbol -- 文字種が記号を示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種が記号を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
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
ModUnicodeCharTrait::isSymbol(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::symbol) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isSpace -- 空白かどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字が空白かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// 空白と判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isSpace(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::space) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isSpace -- 文字種が空白を示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種が空白を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
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
ModUnicodeCharTrait::isSpace(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::space) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isAscii -- ASCIIコードかどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字がASCIIコードかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// ASCII と判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModUnicodeCharTrait::isAscii(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::ascii) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isAscii -- 文字種がASCIIコードを示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種がASCIIコードを示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
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
ModUnicodeCharTrait::isAscii(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::ascii) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isHankakuKana -- 半角カナかどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字が半角カナかどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// 半角カナと判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModUnicodeCharTrait::isHankakuKana(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::hankakuKana) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isHankakuKana -- 文字種が半角カナを示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種が半角カナを示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
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
ModUnicodeCharTrait::isHankakuKana(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::hankakuKana) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isHiragana -- ひらがなかどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字がひらがなかどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// ひらがなと判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isHiragana(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::hiragana) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isHiragana -- 文字種がひらがなを示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種がひらがなを示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
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
ModUnicodeCharTrait::isHiragana(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::hiragana) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isKatakana -- カタカナかどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字がカタカナかどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// カタカナと判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isKatakana(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::katakana) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isKatakana -- 文字種がカタカナを示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種がカタカナを示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
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
ModUnicodeCharTrait::isKatakana(const ModUnicodeCharType type)
{
	return ModBoolean(
		(type & ModUnicodeCharTypes::katakana) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isGreek -- ギリシャ文字かどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字がギリシャ文字かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// ギリシャ文字と判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isGreek(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::greek) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isGreek -- 文字種がギリシャ文字を示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種がギリシャ文字を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
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
ModUnicodeCharTrait::isGreek(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::greek) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isRussian -- ロシア文字かどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字がロシア文字かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// ロシア文字と判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isRussian(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::russian) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isRussian -- 文字種がロシア文字を示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種がロシア文字を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
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
ModUnicodeCharTrait::isRussian(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::russian) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isLine -- 罫線かどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字が罫線かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// 罫線と判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isLine(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::line) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isLine -- 文字種が罫線を示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種が罫線を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
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
ModUnicodeCharTrait::isLine(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::line) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isKanji -- 漢字かどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字が漢字かどうかを判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// 漢字と判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isKanji(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::kanji) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isKanji -- 文字種が漢字を示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種が漢字を示すかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
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
ModUnicodeCharTrait::isKanji(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::kanji) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isControl -- コントロールかどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字がコントロールかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// コントロールと判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isControl(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::control) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isControl -- 文字種がコントロールを示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種がコントロールを示すか
// どうかを判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
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
ModUnicodeCharTrait::isControl(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::control) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isFormat -- フォーマットかどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字がフォーマットかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// フォーマットと判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isFormat(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::format) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isFormat -- 文字種がフォーマットを示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種がフォーマットを示すか
// どうかを判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種にフォーマットを表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModUnicodeCharTrait::isFormat(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::format) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isSurrogate -- サロゲートかどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字がサロゲートかどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// サロゲートと判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isSurrogate(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::surrogate) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isSurrogate -- 文字種がサロゲートを示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種がサロゲートを示すか
// どうかを判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
//		判定の対象となる文字種
//
// RETURN
// 指定した文字種にサロゲートを表すビットが立っている場合は ModTrue を返し、
// 立っていない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline
ModBoolean
ModUnicodeCharTrait::isSurrogate(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::surrogate) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isGaiji -- 外字かどうかを判定する
//
// NOTES
// この関数は ModUnicodeChar で表される文字が外字かどうかを
// 判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		判定の対象となる文字
//
// RETURN
// 外字と判定された場合だけ ModTrue を返す。
// 判定結果は ModUnicodeCharTrait.tbl の内容に依存する。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModUnicodeCharTrait::isGaiji(const ModUnicodeChar character)
{
	return ModBoolean(
		(s_table[character].type & ModUnicodeCharTypes::gaiji) != 0);
}

//
// FUNCTION
// ModUnicodeCharTrait::isGaiji -- 文字種が外字を示すかどうかを判定する
//
// NOTES
// この関数は ModUnicodeCharType で表される文字種が外字を示すか
// どうかを判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeCharType type
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
ModUnicodeCharTrait::isGaiji(const ModUnicodeCharType type)
{
	return ModBoolean((type & ModUnicodeCharTypes::gaiji) != 0);
}

// FUNCTION
// ModUnicodeCharTrait::getType -- 文字種を得る
//
// NOTES
// この関数は ModUnicodeChar で表された文字の文字種を得るのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar character
//		文字種を調べたい文字
//
// RETURN
// 文字種に対応するビットの立った ModUnicodeCharType を返す。
//
// EXCEPTIONS
// なし
//
inline
const ModUnicodeCharType
ModUnicodeCharTrait::getType(const ModUnicodeChar character)
{
	return s_table[character].type;
}

//
// FUNCTION
// ModUnicodeCharTrait::toFloat -- 文字列の float への変換
//
// NOTES
// この関数は数字列の文字列を float に変換するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar* string
//		処理対象の文字列
//
// RETURN
// 数字列を数値に変換した結果を返す。
//
// EXCEPTIONS
// なし
//
inline
float
ModUnicodeCharTrait::toFloat(const ModUnicodeChar* string)
{
	return (float)toDouble(string);
}

#endif	// __ModUnicodeCharTrait_H__

//
// Copyright (c) 1999, 2002, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
