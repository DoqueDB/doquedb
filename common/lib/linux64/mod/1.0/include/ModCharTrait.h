// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModCharTrait.h -- 文字特性関連のクラス定義
// 
// Copyright (c) 1997, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModCharTrait_H__
#define __ModCharTrait_H__

#include "ModCommon.h"
#include "ModAlgorithm.h"

//
// TYPEDEF
// ModCharType -- charで表される文字の文字種を表す型
//
// NOTES
// この型は文字種をビットパターンで表すのに用いる。
// 具体的な文字種は ModCharTrait の定数として定義される。
//
typedef unsigned int ModCharType;

//
// CLASS
// ModCharTrait -- ASCII文字の特性を表すクラス定義
//
// NOTES
// char で表された文字の特性を表現するクラスである。
// 文字が文字列のターミネイタであるか、英字か数字か記号かを判定する
// 関数や char の配列で表された文字列の比較、探索、コピーなどを行なう
// 関数をメンバ関数として提供する。
// このクラスはインスタンスは作られない。
//

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModCharTrait
{
public:
	// 文字種を表す定数の定義

	enum
	{
		upper =			0x00000001,		// 大文字
		lower =			0x00000002,		// 小文字
		alphabetMask =	0x00000003,		// 英字のためのマスク
		digit =			0x00000004,		// 数字
		symbol =		0x00000008,		// 記号
		space =			0x00000010,		// 空白
		ascii =			0x00000020,		// 7bit
		xdigit =		0x00000040,		// 16進数字
		control =		0x00000080		// 制御文字
	};

	// 文字列の最後を指す文字を返す
	static char null();

	// 文字変換
	static char toUpper(const char character);
	static char toLower(const char character);

	// 数値への変換
	static int toInt(const char* string, const int base = 10);
	static long toLong(const char* string, const int base = 10);
	static ModSize toModSize(const char* string, const int base = 10);
	static ModFileSize toModFileSize(const char* string, const int base = 10);
	static float toFloat(const char* string);

	static int				compare(const char* l, const char* r,
									ModSize len = 0);
	static int				compare(const char* l, const char* r,
									ModBoolean caseFlag,
									ModSize len = 0);
												// 2 つの C 文字列の比較

	// 文字列中の文字を探索する
	static char* find(const char* string, const char value,
					  const ModSize maxLength = 0);
	static char* rfind(const char* string, const char value,
					   const ModSize maxLength = 0);
	// ケースを無視することができる探索
	static char* find(const char* string, const char value,
					  const ModBoolean caseFlag,
					  const ModSize maxLength = 0);
	static char* rfind(const char* string, const char value,
					   const ModBoolean caseFlag,
					   const ModSize maxLength = 0);

	// 文字列中の文字列を探索する
	static char* find(const char* string1, const char* string2,
					  const ModSize maxLength = 0);
	static char* find(const char* string1, const char* string2,
					  const ModBoolean caseFlag,
					  const ModSize maxLength = 0);

	// 長さを得る
	static const ModSize length(const char* string);
	// コピーする
	static char* copy(char* destination, const char* source,
					  const ModSize maxLength = 0);

	static char*			append(char* destination, char source);
												// 文字を付加する
	// 文字列を連結する
	static char* append(char* destination, const char* source);

	static void				reverse(char* top, char* tail);
												// 文字並びを逆転する

	// 文字種を得る関数
	static const ModCharType getType(const char character);

	// getType で得た ModCharType から文字種を判定する関数
	// 大文字か否かの判定
	static ModBoolean isUpper(const char character);
	static ModBoolean isUpper(const ModCharType type);
	// 小文字か否かの判定
	static ModBoolean isLower(const char character);
	static ModBoolean isLower(const ModCharType type);
	// 英字か否かの判定
	static ModBoolean isAlphabet(const char character);
	static ModBoolean isAlphabet(const ModCharType type);
	// 数字か否かの判定
	static ModBoolean isDigit(const char character);
	static ModBoolean isDigit(const ModCharType type);
	// 16進数字か否かの判定
	static ModBoolean isXDigit(const char character);
	static ModBoolean isXDigit(const ModCharType type);
	// 記号か否かの判定
	static ModBoolean isSymbol(const char character);
	static ModBoolean isSymbol(const ModCharType type);
	// ASCII(7bit コード)か否かの判定
	static ModBoolean isAscii(const char character);
	static ModBoolean isAscii(const ModCharType type);
	// 空白か否かの判定
	static ModBoolean isSpace(const char character);
	static ModBoolean isSpace(const ModCharType type);
	// 制御文字か否かの判定
	static ModBoolean isControl(const char character);
	static ModBoolean isControl(const ModCharType type);

private:
	// toFloat()等の下請け
	static int getSign(const char*& cp_);
};

//
// FUNCTION 
// ModCharTrait::null -- 文字列のターミネイタに使われる文字を得る
//
// NOTES
// この関数は char で構成される文字列のターミネイタに使われる
// char 型の値を得るのに用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// ターミネイタに使われる char 型の値を返す。
//
// EXCEPTIONS
// なし
//
inline
char
ModCharTrait::null()
{
	return char(0);
}

//
// FUNCTION
// ModCharTrait::toUpper -- char の大文字への変換
//
// NOTES
// この関数は char の英小文字を英大文字に変換するのに用いる。
//
// ARGUMENTS
// const char character
//		大文字に変換する ASCII 文字
//
// RETURN
// character が英小文字の場合は対応する英大文字を返し、
// 英小文字以外の場合は character 自身のコピーを返す。
//
// EXCEPTIONS
// なし
//

inline
char
ModCharTrait::toUpper(const char character)
{
	return (character >= 'a' && character <= 'z') ?
		(character + ('A' - 'a')) : character;
}

//
// FUNCTION
// ModCharTrait::toLower -- char の小文字への変換
//
// NOTES
// この関数は char の英大文字を英小文字に変換するのに用いる。
//
// ARGUMENTS
// const char character
//		小文字に変換する ASCII 文字
//
// RETURN
// character が英大文字の場合は対応する英小文字を返し、
// 英大文字以外の場合は character 自身のコピーを返す。
//
// EXCEPTIONS
// なし
//

inline
char
ModCharTrait::toLower(const char character)
{
	return (character >= 'A' && character <= 'Z') ?
		(character + ('a' - 'A')) : character;
}

//
// FUNCTION
// ModCharTrait::toInt -- 文字列の int への変換
//
// NOTES
// この関数は数字列の文字列を10進数値とみなして int に変換するのに用いる。
//
// ARGUMENTS
// const char* string
//		処理対象の文字列
// const int base
//		基数、省略すると10、16を超えてはいけない
//
// RETURN
// 先頭から数字列が続く部分を数値に変換した結果を返す。
// string の先頭に数字がない場合は 0 を返す。
//
// EXCEPTIONS
// なし
//

inline
int
ModCharTrait::toInt(const char* string, const int base)
{
	int returnValue = 0;
	int flag = 0;

	if (string) {
		const char* cp = string;
		flag = getSign(cp);
		if (base <= 10) {
			for (; ModCharTrait::isDigit(*cp); cp++) {
				returnValue *= base;
				returnValue += (int)(*cp - '0');
			}
		} else {
			for (; ModCharTrait::isXDigit(*cp); cp++) {
				returnValue *= base;
				returnValue += (int)
					(ModCharTrait::isDigit(*cp) ?
					 (*cp - '0') : (ModCharTrait::toUpper(*cp) - 'A' + 10));
			}
		}	
	}

	return returnValue * flag;
}

//
// FUNCTION
// ModCharTrait::toLong -- 文字列の long への変換
//
// NOTES
// この関数は数字列の文字列を10進数値とみなして long に変換するのに用いる。
//
// ARGUMENTS
// const char* string
//		処理対象の文字列
// const int base
//		基数、省略すると10、16を超えてはいけない
//
// RETURN
// 先頭から数字列が続く部分を数値に変換した結果を返す。
// string の先頭に数字がない場合は 0 を返す。
//
// EXCEPTIONS
// なし
//
inline
long
ModCharTrait::toLong(const char* string, const int base)
{
	long returnValue = 0;
	long flag = 0;

	if (string) {
		const char* cp = string;
		flag = getSign(cp);
		if (base <= 10) {
			for (; ModCharTrait::isDigit(*cp); cp++) {
				returnValue *= base;
				returnValue += (long)(*cp - '0');
			}
		} else {
			for (; ModCharTrait::isXDigit(*cp); cp++) {
				returnValue *= base;
				returnValue += (long)
					(ModCharTrait::isDigit(*cp) ?
					 (*cp - '0') : (ModCharTrait::toUpper(*cp) - 'A' + 10));
			}
		}
	}

	return returnValue * flag;
}

//
// FUNCTION
// ModCharTrait::toModSize -- 文字列の ModSize への変換
//
// NOTES
// この関数は数字列の文字列を10進数値とみなして ModSize に変換するのに用いる。
//
// ARGUMENTS
// const char* string
//		処理対象の文字列
// const int base
//		基数、省略すると10、16を超えてはいけない
//
// RETURN
// 先頭から数字列が続く部分を数値に変換した結果を返す。
// string の先頭に数字がない場合は 0 を返す。
//
// EXCEPTIONS
// なし
//
inline
ModSize
ModCharTrait::toModSize(const char* string, const int base)
{
	ModSize returnValue = 0;
	int flag = 0;

	if (string) {
		const char* cp = string;
		flag = getSign(cp);
		if (base <= 10) {
			for (; ModCharTrait::isDigit(*cp); cp++) {
				returnValue *= base;
				returnValue += (ModSize)(*cp - '0');
			}
		} else {
			for (; ModCharTrait::isXDigit(*cp); cp++) {
				returnValue *= base;
				returnValue += (ModSize)
					(ModCharTrait::isDigit(*cp) ?
					 (*cp - '0') : (ModCharTrait::toUpper(*cp) - 'A' + 10));
			}
		}
	}

	return returnValue * flag;
}

//
// FUNCTION
// ModCharTrait::toModFileSize -- 文字列の ModFileSize への変換
//
// NOTES
// この関数は数字列の文字列を10進数値とみなして ModFileSize に変換するのに用いる。
//
// ARGUMENTS
// const char* string
//		処理対象の文字列
// const int base
//		基数、省略すると10、16を超えてはいけない
//
// RETURN
// 先頭から数字列が続く部分を数値に変換した結果を返す。
// string の先頭に数字がない場合は 0 を返す。
//
// EXCEPTIONS
// なし
//
inline
ModFileSize
ModCharTrait::toModFileSize(const char* string, const int base)
{
	ModFileSize returnValue = 0;
	int flag = 0;

	if (string) {
		const char* cp = string;
		flag = getSign(cp);
		if (base <= 10) {
			for (; ModCharTrait::isDigit(*cp); cp++) {
				returnValue *= base;
				returnValue += (ModFileSize) (*cp - '0');
			}
		} else {
			for (; ModCharTrait::isXDigit(*cp); cp++) {
				returnValue *= base;
				returnValue += (ModFileSize)
					(ModCharTrait::isDigit(*cp) ?
					 (*cp - '0') : (ModCharTrait::toUpper(*cp) - 'A' + 10));
			}
		}
	}

	return returnValue * flag;
}

//
// FUNCTION
// ModCharTrait::toFloat -- 文字列の float への変換
//
// NOTES
// この関数は数字列の文字列を10進数値とみなして float に変換するのに用いる。
//
// ARGUMENTS
// const char* string
//		処理対象の文字列
//
// RETURN
// 先頭から数字列が続く部分を数値に変換した結果を返す。
// string の先頭に数字がない場合は 0 を返す。
//
// EXCEPTIONS
// なし
//
inline
float
ModCharTrait::toFloat(const char* string)
{
	double returnValue = 0.0;
	int flag = 0;

	//
	// ここの実装はタコい。atof を使った方が速いに決まってる。
	// あまりにここのせいで遅いようなら atof を利用するように変更すべし。
	//
	if (string) {
		const char* cp = string;
		// 符号
		flag = getSign(cp);
		// 仮数部
		for (; ModCharTrait::isDigit(*cp); cp++) {
			returnValue *= 10;
			returnValue += (double)(((int)(*cp - '0')) & 0xff);
		}
		// 小数部
		int fraction = 0;
		if (*cp == '.') {
			for (cp++; ModCharTrait::isDigit(*cp); cp++) {
				returnValue *= 10;
				returnValue += (double)(((int)(*cp - '0')) & 0xff);
				fraction--;
			}
		}
		// 指数部
		int exponential = 0;
		if (*cp == 'e' || *cp == 'E') {
			cp++;
			int expFlag = getSign(cp);
			for (; ModCharTrait::isDigit(*cp); cp++)
			{
				exponential *= 10;
				exponential += (((int)(*cp - '0')) & 0xff);
			}
			exponential *= expFlag;
		}
		// ここが特にタコ
		exponential += fraction;
		for (;exponential >= 1; exponential--) {
			returnValue *= 10.0;
		}
		for (;exponential <= -1; exponential++) {
			returnValue /= 10.0;
		}
	}

	return (float)returnValue * flag;
}

//	FUNCTION public
//	ModCharTrait::compare -- 2 つの C 文字列を比較する
//
//	NOTES
//
//	ARGUMENTS
//		char*				l
//			r と比較する C 文字列
//		char*				r
//			l と比較する C 文字列
//		ModBoolean			caseFlag
//			ModTrue または指定されないとき
//				大文字、小文字を区別する
//			ModFalse
//				大文字、小文字を区別しない
//		ModSize				len
//			0 以外
//				先頭から比較する最大文字数
//			0 または指定されないとき
//				C 文字列全体を比較する
//
//	RETURN
//		1
//			最初に異なる文字の ASCII コードが
//			r より l のもののほうが大きい
//		0
//			l と r のすべての文字の ASCII コードが等しい
//		-1
//			最初に異なる文字の ASCII コードが
//			l より r のもののほうが大きい
//
//	EXCEPTIONS
//		なし

// static
inline
int
ModCharTrait::compare(const char* l, const char* r, ModSize len)
{
	if (l == r)
		return 0;
	if (l == 0)
		return -1;
	if (r == 0)
		return 1;

	// 双方の文字列の先頭からひとつひとつ文字を比較する
	// どちらかをすべて調べ終わるか、指定された数 - 1 個の文字を調べるか、
	// 文字が異なったときにループを抜ける
	//
	//【注意】	調べた文字数と指定された数は
	//			< でなく != で比較することにより、
	//			0 が指定されたときに文字列全体を調べることになる

	ModSize n = 0;
	for (; *l != ModCharTrait::null() && *r != ModCharTrait::null() &&
		   ++n != len && *l == *r; ++l, ++r) ;

	// 最後に見ていた文字の比較結果で返り値を決める
	//
	//【注意】	unsigned char にキャストしてから調べないと、
	//			'\0' より 2 バイト文字の 前半(または後半)
	//			1 バイトのほうが小さくなってしまう

	return ((unsigned char) *l < (unsigned char) *r) ? -1 :
		((unsigned char) *l > (unsigned char) *r) ? 1 : 0;
}

// static
inline
int
ModCharTrait::compare(const char* l, const char* r,
					  ModBoolean caseFlag, ModSize len)
{
	if (caseFlag)

		// 大文字小文字を区別するなら、そのための比較関数を呼び出す

		return ModCharTrait::compare(l, r, len);

	if (l == r)
		return 0;
	if (l == 0)
		return -1;
	if (r == 0)
		return 1;

	// 双方の文字列の先頭からひとつひとつ文字を比較する
	// どちらかをすべて調べ終わるか、指定された数 - 1 個の文字を調べるか、
	// 文字が異なったときにループを抜ける
	//
	//【注意】	調べた文字数と指定された数は
	//			< でなく != で比較することにより、
	//			0 が指定されたときに文字列全体を調べることになる

	ModSize n = 0;
	for (; *l != ModCharTrait::null() && *r != ModCharTrait::null() &&
		   ++n != len &&
		   ModCharTrait::toUpper(*l) == ModCharTrait::toUpper(*r); ++l, ++r) ;

	// 最後に見ていた文字の比較結果で返り値を決める
	//
	//【注意】	unsigned char にキャストしてから調べないと、
	//			'\0' より 2 バイト文字の 前半(または後半)
	//			1 バイトのほうが小さくなってしまう

	return ((unsigned char) ModCharTrait::toUpper(*l) <
			(unsigned char) ModCharTrait::toUpper(*r)) ? -1 :
		((unsigned char) ModCharTrait::toUpper(*l) >
		 (unsigned char) ModCharTrait::toUpper(*r)) ? 1 : 0;
}

//
// FUNCTION 
// ModCharTrait::find -- 文字列中の文字の探索
//
// NOTES
// この関数は文字列中に指定文字が最初に現れるポインタを得るのに用いる。
//
// ARGUMENTS
// const char* string
//		探索の対象となる文字列
// const char value
//		string 中から探す文字
// const ModSize maxLength
//		探索範囲を文字列の先頭からの文字数で指定する。
//		この引数を省略するか、0 を渡すと文字列全体を探索する。
//
// RETURN
// string 中に value が見つかればその最初の位置を指すポインタを返す。
// string 中に value がなければ 0 を返す。
//
// EXCEPTIONS
// なし
//
inline
char*
ModCharTrait::find(const char* string, const char value,
				   const ModSize maxLength)
{
	if (string == 0) {
		// string が何も指していなければ探すまでもない
		return 0;
	}

	//
	// string を走査して最初に value と一致する場所を見つける。
	// 一致する場所が見つからないまま調べる文字数が maxLength を
	// 超えていたらループを抜ける。
	//
	for (ModSize count = 0; *string != ModCharTrait::null(); string++) {
		if (*string == value) {
			// 一致する部分が見つかったのでその場所を返す
			return (char*)string;
		}
		if (++count == maxLength) {
			// 見つからないまま調べた文字数が指定値になったら 0 を返す
			return 0;
		}
	}

	// value が null なら、今 string が指している場所がその場所

	// 最後まで見つからなかった

	return (value == ModCharTrait::null()) ? (char*) string : 0;
}

//
// FUNCTION 
// ModCharTrait::rfind -- 文字列中の文字の探索(後ろから)
//
// NOTES
// この関数は文字列中に指定文字が最後に現れるポインタを得るのに用いる。
//
// ARGUMENTS
// const char* string
//		探索の対象となる文字列
// const char value
//		string 中から探す文字
// const ModSize maxLength
//		探索範囲を文字列の先頭からの文字数で指定する。
//		この引数を省略するか、0 を渡すと文字列全体を探索する。
//
// RETURN
// string 中に value が見つかればその最後の位置を指すポインタを返す。
// string 中に value がなければ string の先頭を指すポインタを返す。
//
// EXCEPTIONS
// なし
//
inline
char*
ModCharTrait::rfind(const char* string, const char value,
					const ModSize maxLength)
{
	if (string == 0) {
		// string が何も指していなければ探すまでもない
		return (char*)string;
	}

	//
	// string を走査して最後に value と一致する場所を見つける。
	// 一致する場所が見つからないまま調べる文字数が maxLength を
	// 超えていたらループを抜ける。
	//
	const char* cp = string;
	for (ModSize count = 0; *string != ModCharTrait::null(); string++) {
		if (*string == value) {
			// 一致する部分が見つかったのでその場所を覚える
			cp = string;
		}
		if (++count == maxLength) {
			// 調べた文字数が指定値になったらループを抜ける
			break;
		}
	}

	return (char*)cp;
}

//
// FUNCTION 
// ModCharTrait::find -- 文字列中の文字の探索(ケースフラグつき)
//
// NOTES
// この関数は文字列中に指定文字が最初に現れるポインタを得るのに用いる。
// フラグによって大文字小文字を無視した探索もできる。
//
// ARGUMENTS
// const char* string
//		探索の対象となる文字列
// const char value
//		string 中から探す文字
// const ModBoolean caseFlag
//		ケースセンシティブか否かを指定するフラグ
// const ModSize maxLength
//		探索範囲を文字列の先頭からの文字数で指定する。
//		この引数を省略するか、0 を渡すと文字列全体を探索する。
//
// RETURN
// string 中に value が見つかればその最初の位置を指すポインタを返す。
// string 中に value がなければ 0 を返す。
// caseFlag が ModFalse のときは大文字と小文字を同一視する。
//
// EXCEPTIONS
// なし
//
inline
char*
ModCharTrait::find(const char* string, const char value,
				   const ModBoolean caseFlag, const ModSize maxLength)
{
	if (string == 0) {
		// string が何も指していなければ探すまでもない
		return 0;
	}

	// ケースセンシティブなら普通の関数を呼ぶ
	if (caseFlag == ModTrue) {
		return ModCharTrait::find(string, value, maxLength);
	}

	//
	// string を走査して最初に value と一致する場所を見つける。
	// 一致する場所が見つからないまま調べる文字数が maxLength を
	// 超えていたらループを抜ける。
	//
	for (ModSize count = 0; *string != ModCharTrait::null(); string++) {
		if (ModCharTrait::toUpper(*string) == ModCharTrait::toUpper(value)) {
			// 一致する部分が見つかったのでその場所を返す
			return (char*)string;
		}
		if (++count == maxLength) {
			// 見つからないまま調べた文字数が指定値になったら 0 を返す
			return 0;
		}
	}

	// value が null なら、今 string が指している場所がその場所

	// 最後まで見つからなかった

	return (value == ModCharTrait::null()) ? (char*) string : 0;
}

//
// FUNCTION 
// ModCharTrait::find -- 文字列中の文字の探索(ケースフラグつき、後ろから)
//
// NOTES
// この関数は文字列中に指定文字が最後に現れるポインタを得るのに用いる。
// フラグによって大文字小文字を無視した探索もできる。
//
// ARGUMENTS
// const char* string
//		探索の対象となる文字列
// const char value
//		string 中から探す文字
// const ModBoolean caseFlag
//		ケースセンシティブか否かを指定するフラグ
// const ModSize maxLength
//		探索範囲を文字列の先頭からの文字数で指定する。
//		この引数を省略するか、0 を渡すと文字列全体を探索する。
//
// RETURN
// string 中に value が見つかればその最後の位置を指すポインタを返す。
// string 中に value がなければ string を返す。
// caseFlag が ModFalse のときは大文字と小文字を同一視する。
//
// EXCEPTIONS
// なし
//
inline
char*
ModCharTrait::rfind(const char* string, const char value,
					const ModBoolean caseFlag, const ModSize maxLength)
{
	if (string == 0) {
		// string が何も指していなければ探すまでもない
		return (char*)string;
	}

	// ケースセンシティブなら普通の関数を呼ぶ
	if (caseFlag == ModTrue) {
		return ModCharTrait::rfind(string, value, maxLength);
	}

	//
	// string を走査して最後に value と一致する場所を見つける。
	// 調べる文字数が maxLength を超えたらループを抜ける。
	//
	const char* cp = string;
	for (ModSize count = 0; *string != ModCharTrait::null(); string++) {
		if (ModCharTrait::toUpper(*string) == ModCharTrait::toUpper(value)) {
			// 一致する部分が見つかったのでその場所を覚える
			cp = string;
		}
		if (++count == maxLength) {
			// 調べた文字数が指定値になったらループを抜ける
			break;
		}
	}

	return (char*)cp;
}

//
// FUNCTION 
// ModCharTrait::find -- 文字列中の文字列の探索
//
// NOTES
// この関数は文字列中に指定文字列が最初に現れるポインタを得るのに用いる。
//
// ARGUMENTS
// const char* string1
//		探索の対象となる文字列
// const char* string2
//		string 中から探す文字列
// const ModSize maxLength
//		探索範囲を文字列の先頭からの文字数で指定する。
//		この引数を省略するか、0 を渡すと文字列全体を探索する。
//
// RETURN
// string1 中に string2 に一致する部分が見つかればその先頭の位置を指す
// ポインタを返す。
// string1 中に string2 に一致する部分がなければ 0 を返す。
//
// EXCEPTIONS
// なし
//
inline
char*
ModCharTrait::find(const char* string1, const char* string2,
				   const ModSize maxLength)
{
	if (string1 == 0 || string2 == 0 || *string2 == ModCharTrait::null())
		// string1 が何も指していなければ探すまでもない
		// string2 が空文字列なら探すまでもない
		return 0;

	// string2 の文字列長を得ておく
	ModSize length = ModCharTrait::length(string2);

	const char* cp1 = string1;
	const char* cp2 = string2;

	while (1) {
		//
		// string1 を走査して string2 の先頭の文字と一致する場所を見つける。
		// 一致する場所が見つからないまま調べる文字数が maxLength を
		// 超えていたら 0 を返す。
		//
		cp2 = string2;
		ModSize count = 0;
		for (; *cp1 != ModCharTrait::null(); cp1++) {
			if (*cp1 == *cp2) {
				// 一致する部分が見つかった
				break;
			}
			if (++count == maxLength) {
				// 見つからないまま調べた文字数が指定値になったら 0 を返す
				return 0;
			}
		}

		// string1 の最後まで探してもなかったら 0 を返す

		// たとえあっても指定文字数を超えていたら 0 を返す

		if (*cp1 == ModCharTrait::null() ||
			(maxLength > 0 && count + length >= maxLength))
			return 0;

		//
		// 最初の文字と一致する場所が見つかったので、2文字め以降を調べる。
		// 同時に1文字めと一致する最初の場所を candid に入れておく
		//

		const char* result = cp1;
		++cp1;
		++cp2;
		const char* candid = 0;
		for (;
			 (*cp1 == *cp2) && (*cp1 != ModCharTrait::null())
				 && (*cp2 != ModCharTrait::null());
			 ++cp1, ++cp2) {
			if ((candid == 0) && (*cp1 == *string2)) {
				// 先頭と一致する場所を記憶しておく
				candid = cp1;
			}
		}
		if (*cp2 == ModCharTrait::null()) {
			// 見つかった
			return (char*)result;
		}
		// 見つからなかった
		if (candid)
			// 先頭と一致する場所があればそこまで戻す
			cp1 = candid;
	}
	return 0;
}

//
// FUNCTION 
// ModCharTrait::find -- 文字列中の文字列の探索(ケースフラグつき)
//
// NOTES
// この関数は文字列中に指定文字列が最初に現れるポインタを得るのに用いる。
// フラグによって大文字小文字を無視した探索もできる。
//
// ARGUMENTS
// const char* string1
//		探索の対象となる文字列
// const char* string2
//		string 中から探す文字列
// const ModBoolean caseFlag
//		ケースセンシティブか否かを指定するフラグ
// const ModSize maxLength
//		探索範囲を文字列の先頭からの文字数で指定する。
//		この引数を省略するか、0 を渡すと文字列全体を探索する。
//
// RETURN
// string1 中に string2 に一致する部分が見つかればその先頭の位置を指す
// ポインタを返す。
// string1 中に string2 に一致する部分がなければ 0 を返す。
//
// EXCEPTIONS
// なし
//
inline
char*
ModCharTrait::find(const char* string1, const char* string2,
				   const ModBoolean caseFlag, const ModSize maxLength)
{
	if (string1 == 0 || string2 == 0 || *string2 == ModCharTrait::null())
		// string1 が何も指していなければ探すまでもない
		// string2 が空文字列なら探すまでもない
		return 0;

	// ケースセンシティブなら普通の関数を呼ぶ
	if (caseFlag == ModTrue) {
		return ModCharTrait::find(string1, string2, maxLength);
	}

	// string2 の文字列長を得ておく
	ModSize length = ModCharTrait::length(string2);

	const char*	cp1 = string1;
	const char*	cp2 = string2;

	while (1) {
		//
		// string1 を走査して string2 の先頭の文字と一致する場所を見つける。
		// 一致する場所が見つからないまま調べる文字数が maxLength を
		// 超えていたら 0 を返す。
		//
		cp2 = string2;
		ModSize count = 0;
		for (; *cp1 != ModCharTrait::null(); cp1++) {
			if (ModCharTrait::toUpper(*cp1) == ModCharTrait::toUpper(*cp2)) {
				// 一致する部分が見つかった
				break;
			}
			if (++count == maxLength) {
				// 見つからないまま調べた文字数が指定値になったら 0 を返す
				return 0;
			}
		}

		// string1 の最後まで探してもなかったら 0 を返す

		// たとえあっても最後が指定文字数を超えていたら 0 を返す

		if (*cp1 == ModCharTrait::null() ||
			(maxLength > 0 && count + length >= maxLength))
			return 0;

		//
		// 最初の文字と一致する場所が見つかったので、2文字め以降を調べる。
		// 同時に1文字めと一致する最初の場所を candid に入れておく
		//
		const char* result = cp1;
		++cp1;
		++cp2;
		const char* candid = 0;
		for (;
			 (ModCharTrait::toUpper(*cp1) == ModCharTrait::toUpper(*cp2))
				 && (*cp1 != ModCharTrait::null())
				 && (*cp2 != ModCharTrait::null());
			 ++cp1, ++cp2) {
			if ((candid == 0)
				&& (ModCharTrait::toUpper(*cp1)
					== ModCharTrait::toUpper(*string2))) {
				// 先頭と一致する場所を記憶しておく
				candid = cp1;
			}
		}
		if (*cp2 == ModCharTrait::null()) {
			// 見つかった
			return (char*)result;
		}
		// 見つからなかった
		if (candid)
			// 先頭と一致する場所があればそこまで戻す
			cp1 = candid;
	}
	return 0;
}

//
// FUNCTION 
// ModCharTrait::length -- 文字列長を得る
//
// NOTES
// この関数は char の配列で構成された文字列の長さ(文字数)を得るのに用いる。
//
// ARGUMENTS
// const char* string
//		長さを調べる対象となる文字列
//
// RETURN
// string の文字数を返す。ただし、文字数にはターミネイタ文字の分は含まれない。
//
// EXCEPTIONS
// その他
// なし
//

inline
const ModSize
ModCharTrait::length(const char* string)
{

	if (string == 0) {
		// string が何も指していなければ調べるまでもない
		return 0;
	}

	//
	// string をターミネイタ文字に当たるまで走査しながら count を増やす
	//
	ModSize count = 0;
	for (; *string != ModCharTrait::null(); string++, count++);
	return count;
}

//
// FUNCTION 
// ModCharTrait::copy -- 文字列の複写
//
// NOTES
// この関数は char の配列をコピーするのに用いる。
//
// ARGUMENTS
// char* destination
//		コピー先を指すポインタ
// const char* source
//		コピー元を指すポインタ
// const ModSize maxLength
//		コピーする範囲を source の指す場所からの文字数で指定する。
//		この引数を省略するか、0 を渡すと文字列の終りまでコピーする。
//
// RETURN
// destination の値を返す。
//
// EXCEPTIONS
// なし
//

inline
char*
ModCharTrait::copy(char* destination, const char* source,
				   const ModSize maxLength)
{
	if (source == 0 || destination == 0 || source == destination)
		// source が 0 なら destination は変えない
		// 同じものを指していたらコピーするまでもない。
		return destination;

	char* returnValue = destination;
	//
	// destination, source を走査して source の値を destination にコピーする。
	// 領域が重なっていることは考慮しない。
	// コピーした文字数が maxLength に一致したらリターンする。
	//
	for (ModSize count = 0; *source != ModCharTrait::null();
		 source++, destination++) {
		if (++count == maxLength) {
			//
			// 調べた文字数が指定値になったらループを抜ける。
			// == で比較するため maxLength が 0 のときには
			// ここで引っかかることはない。
			//
			break;
		}
		// ポインターの指す先をコピーする
		*destination = *source;
	}
	//
	// 現在指している先をコピーする。
	// ここで最後にコピーすることにより maxLength 文字コピーする前に
	// source がターミネイトしている場合に destination もターミネイトさせる
	// ことができる。
	//
	*destination = *source;

	return returnValue;
}

//	FUNCTION public
//	ModCharTrait::append -- 文字の付加
//
//	NOTES
//
//	ARGUMENTS
//		char*				destination
//			文字を付加する文字列
//		char				source
//			付加する文字
//
//	RETURN
//		文字が付加された文字列
//
//	EXCEPTIONS
//		なし

// static
inline
char*
ModCharTrait::append(char* destination, char source)
{
	if (destination && source != ModCharTrait::null()) {
		char*	p = destination;
		for (; *p != ModCharTrait::null(); ++p) ;
		*p = source;
		*++p = ModCharTrait::null();
	}
	return destination;
}

//
// FUNCTION 
// ModCharTrait::append -- 文字列の連結
//
// NOTES
// この関数は 2 つの char の配列を連結するのに用いる。
//
// ARGUMENTS
// char* destination
//		連結先を指すポインタ
// const char* source
//		連結元を指すポインタ
//
// RETURN
// destination の値を返す。
//
// EXCEPTIONS
// なし
//

inline
char*
ModCharTrait::append(char* destination, const char* source)
{
	if (source == 0 || destination == 0 || source == destination)
		// source が 0 なら destination は変えない
		// 同じものを指していたら連結しない
		return destination;

	char* returnValue = destination;

	// destination の最後までポインタを移動する
	for (; *destination != ModCharTrait::null(); destination++);
	// source の値をコピーする
	for (; *source != ModCharTrait::null(); source++, destination++) {
		*destination = *source;
	}
	// 現在指している位置をコピーする
	*destination = *source;

	return returnValue;
}

//	FUNCTION public
//	ModCharTrait::reverse -- ある文字の並びの順序を逆にする
//
//	NOTES
//
//	ARGUMENTS
//		char*				top
//			順序を逆にする文字の並びの先頭
//		char*				tail
//			順序を逆にする文字の並びの末尾
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
inline
void
ModCharTrait::reverse(char* top, char* tail)
{
	if (top)
		for (; top < tail; ++top, --tail)
			ModSwap(*top, *tail);
}

//
// FUNCTION
// ModCharTrait::getType -- 文字種を得る
//
// NOTES
// この関数はASCII文字の文字種を得るのに用いる。
//
// ARGUMENTS
// const char character
//		判定の対象となる文字
//
// RETURN
// 文字種により以下の定数の OR をとったものを返す。
// ModCharTrait::ascii    .. ASCIIコード(MSB=0のもの)
// ModCharTrait::upper    .. 大文字
// ModCharTrait::lower    .. 小文字
// ModCharTrait::digit    .. 数字
// ModCharTrait::symbol   .. 記号
// ModCharTrait::space    .. 空白
//
// EXCEPTIONS
// なし
//

inline
const ModCharType
ModCharTrait::getType(const char character)
{
	return
		(ModCharTrait::isLower(character) ? ModCharTrait::lower :
		 (ModCharTrait::isDigit(character) ? ModCharTrait::digit :
		  (ModCharTrait::isSymbol(character) ? ModCharTrait::symbol :
		   (ModCharTrait::isUpper(character) ? ModCharTrait::upper :
			(ModCharTrait::isSpace(character) ? ModCharTrait::space : 0)
		)))) |
		(ModCharTrait::isAscii(character) ? ModCharTrait::ascii : 0) |
		(ModCharTrait::isXDigit(character) ? ModCharTrait::xdigit : 0) |
		(ModCharTrait::isControl(character) ? ModCharTrait::control : 0);
}

//
// FUNCTION
// ModCharTrait::isUpper -- 文字が大文字かどうか調べる
//
// NOTES
// この関数は与えられたバイトコードがASCII文字の大文字を表すかを調べるために
// 用いる。
//
// ARGUMENTS
// const char character
//		対象となるバイトコード
//
// RETURN
// character が大文字の範囲にあれば ModTrue を、なければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModCharTrait::isUpper(const char character)
{
	return (character >= 'A' && character <= 'Z') ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModCharTrait::isUpper -- 文字種が大文字かどうか調べる
//
// NOTES
// この関数は与えられた文字種コードの大文字を表すビットが立っているかを
// 調べるために用いる。
//
// ARGUMENTS
// const ModCharType type
//		対象となる文字種コード
//
// RETURN
// type の大文字を表すビットが立っていれば ModTrue を、
// 立っていなければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModCharTrait::isUpper(const ModCharType type)
{
	return ((type & ModCharTrait::upper) == ModCharTrait::upper) ?
		ModTrue : ModFalse;
}

//
// FUNCTION
// ModCharTrait::isLower -- 文字が小文字かどうか調べる
//
// NOTES
// この関数は与えられたバイトコードがASCII文字の小文字を表すかを調べるために
// 用いる。
//
// ARGUMENTS
// const char character
//		対象となるバイトコード
//
// RETURN
// character が小文字の範囲にあれば ModTrue を、なければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModCharTrait::isLower(const char character)
{
	return (character >= 'a' && character <= 'z') ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModCharTrait::isLower -- 文字種が小文字かどうか調べる
//
// NOTES
// この関数は与えられた文字種コードの小文字を表すビットが立っているかを
// 調べるために用いる。
//
// ARGUMENTS
// const ModCharType type
//		対象となる文字種コード
//
// RETURN
// type の小文字を表すビットが立っていれば ModTrue を、
// 立っていなければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModCharTrait::isLower(const ModCharType type)
{
	return ((type & ModCharTrait::lower) == ModCharTrait::lower) ?
		ModTrue : ModFalse;
}

//
// FUNCTION
// ModCharTrait::isAlphabet -- 文字が英字かどうか調べる
//
// NOTES
// この関数は与えられたバイトコードがASCII文字の英字を表すかを調べるために
// 用いる。
//
// ARGUMENTS
// const char character
//		対象となるバイトコード
//
// RETURN
// character が英字の範囲にあれば ModTrue を、なければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModCharTrait::isAlphabet(const char character)
{
	return ((character >= 'A' && character <= 'Z')
			|| (character >= 'a' && character <= 'z'))?ModTrue:ModFalse;
}

//
// FUNCTION
// ModCharTrait::isAlphabet -- 文字種が英字かどうか調べる
//
// NOTES
// この関数は与えられた文字種コードの大文字あるいは小文字を表すビットが立って
// いるかを調べるために用いる。
//
// ARGUMENTS
// const ModCharType type
//		対象となる文字種コード
//
// RETURN
// type の大文字あるいは小文字を表すビットが立っていれば ModTrue を、
// 立っていなければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModCharTrait::isAlphabet(const ModCharType type)
{
	return (type & ModCharTrait::alphabetMask) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModCharTrait::isDigit -- 文字が数字かどうか調べる
//
// NOTES
// この関数は与えられたバイトコードがASCII文字の数字を表すかを調べるために
// 用いる。
//
// ARGUMENTS
// const char character
//		対象となるバイトコード
//
// RETURN
// character が数字の範囲にあれば ModTrue を、なければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModCharTrait::isDigit(const char character)
{
	return (character >= '0' && character <= '9')?ModTrue:ModFalse;
}

//
// FUNCTION
// ModCharTrait::isDigit -- 文字種が数字かどうか調べる
//
// NOTES
// この関数は与えられた文字種コードの数字を表すビットが立っているかを
// 調べるために用いる。
//
// ARGUMENTS
// const ModCharType type
//		対象となる文字種コード
//
// RETURN
// type の数字を表すビットが立っていれば ModTrue を、
// 立っていなければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModCharTrait::isDigit(const ModCharType type)
{
	return ((type & ModCharTrait::digit) == ModCharTrait::digit)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModCharTrait::isXDigit -- 文字が16進数字かどうか調べる
//
// NOTES
// この関数は与えられたバイトコードがASCII文字の16進数字を表すかを調べるために
// 用いる。
//
// ARGUMENTS
// const char character
//		対象となるバイトコード
//
// RETURN
// character が16進数字の範囲にあれば ModTrue を、なければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModCharTrait::isXDigit(const char character)
{
	return ((character >= '0' && character <= '9')
			|| (character >= 'a' && character <= 'f')
			|| (character >= 'A' && character <= 'F'))
		?
		ModTrue
		:
		ModFalse;
}

//
// FUNCTION
// ModCharTrait::isXDigit -- 文字種が16進数字かどうか調べる
//
// NOTES
// この関数は与えられた文字種コードの16進数字を表すビットが立っているかを
// 調べるために用いる。
//
// ARGUMENTS
// const ModCharType type
//		対象となる文字種コード
//
// RETURN
// type の16進数字を表すビットが立っていれば ModTrue を、
// 立っていなければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModCharTrait::isXDigit(const ModCharType type)
{
	return ((type & ModCharTrait::xdigit) == ModCharTrait::xdigit)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModCharTrait::isSymbol -- 文字が記号かどうか調べる
//
// NOTES
// この関数は与えられたバイトコードがASCII文字の記号を表すかを調べるために
// 用いる。
//
// ARGUMENTS
// const char character
//		対象となるバイトコード
//
// RETURN
// character が記号の範囲にあれば ModTrue を、なければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModCharTrait::isSymbol(const char character)
{
	return ((character >= '!' && character <= '/')
			|| (character >= ':' && character <= '@')
			|| (character >= '[' && character <= '`')
			|| (character >= '{' && character <= '~'))?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModCharTrait::isSymbol -- 文字種が記号かどうか調べる
//
// NOTES
// この関数は与えられた文字種コードの記号を表すビットが立っているかを
// 調べるために用いる。
//
// ARGUMENTS
// const ModCharType type
//		対象となる文字種コード
//
// RETURN
// type の記号を表すビットが立っていれば ModTrue を、
// 立っていなければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModCharTrait::isSymbol(const ModCharType type)
{
	return ((type & ModCharTrait::symbol) == ModCharTrait::symbol)?
		ModTrue:ModFalse;
}

//
// FUNCTION
// ModCharTrait::isAscii -- 文字が7bitかどうか調べる
//
// NOTES
// この関数は与えられたバイトコードがASCII文字の7bitを表すかを調べるために
// 用いる。
//
// ARGUMENTS
// const char character
//		対象となるバイトコード
//
// RETURN
// character が7bitの範囲にあれば ModTrue を、なければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModCharTrait::isAscii(const char character)
{
	return (character & 0x80) ? ModFalse : ModTrue;
}

//
// FUNCTION
// ModCharTrait::isAscii -- 文字種が7bitかどうか調べる
//
// NOTES
// この関数は与えられた文字種コードの7bitを表すビットが立っているかを
// 調べるために用いる。
//
// ARGUMENTS
// const ModCharType type
//		対象となる文字種コード
//
// RETURN
// type の7bitを表すビットが立っていれば ModTrue を、
// 立っていなければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModCharTrait::isAscii(const ModCharType type)
{
	return ((type & ModCharTrait::ascii) == ModCharTrait::ascii) ?
		ModTrue : ModFalse;
}

//
// FUNCTION
// ModCharTrait::isSpace -- 文字が空白かどうか調べる
//
// NOTES
// この関数は与えられたバイトコードがASCII文字の空白を表すかを調べるために
// 用いる。
//
// ARGUMENTS
// const char character
//		対象となるバイトコード
//
// RETURN
// character が空白の範囲にあれば ModTrue を、なければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModCharTrait::isSpace(const char character)
{
	return (character == ' ' || character == '\n' ||
			character == '\t' || character == '\r') ?
		ModTrue : ModFalse;
}

//
// FUNCTION
// ModCharTrait::isSpace -- 文字種が空白かどうか調べる
//
// NOTES
// この関数は与えられた文字種コードの空白を表すビットが立っているかを
// 調べるために用いる。
//
// ARGUMENTS
// const ModCharType type
//		対象となる文字種コード
//
// RETURN
// type の空白を表すビットが立っていれば ModTrue を、
// 立っていなければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModCharTrait::isSpace(const ModCharType type)
{
	return ((type & ModCharTrait::space) == ModCharTrait::space) ?
		ModTrue : ModFalse;
}

//
// FUNCTION
// ModCharTrait::isControl -- 文字が制御文字かどうか調べる
//
// NOTES
// この関数は与えられたバイトコードが制御文字かを調べるために
// 用いる。
//
// ARGUMENTS
// const char character
//		対象となるバイトコード
//
// RETURN
// character が0x00〜0x19の範囲にあれば ModTrue を、なければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModCharTrait::isControl(const char character)
{
	return (character & 0xe0) ? ModFalse : ModTrue;
}

//
// FUNCTION
// ModCharTrait::isControl -- 文字種が7bitかどうか調べる
//
// NOTES
// この関数は与えられた文字種コードの7bitを表すビットが立っているかを
// 調べるために用いる。
//
// ARGUMENTS
// const ModCharType type
//		対象となる文字種コード
//
// RETURN
// type の7bitを表すビットが立っていれば ModTrue を、
// 立っていなければ ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModCharTrait::isControl(const ModCharType type)
{
	return ((type & ModCharTrait::control) == ModCharTrait::control) ?
		ModTrue : ModFalse;
}

//
// FUNCTION private
// ModCharTrait::getSign --
//
// NOTES
//
// ARGUMENTS
// const char*& cp_
//
// RETURN
// int
//
// EXCEPTIONS
//

inline
int
ModCharTrait::getSign(const char*& cp_)
{
	int iSign = 1;
		
	if (*cp_ == '-')
	{
		iSign = -1;
		++cp_;
	}
	else if (*cp_ == '+')
	{
		++cp_;
	}
		
	return iSign;
}

#endif	// __ModCharTrait_H__

//
// Copyright (c) 1997, 2009, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
