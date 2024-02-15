// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModUnicodeCharTrait.cpp -- ModUnicodeCharTrait のメンバ定義
// 
// Copyright (c) 1999, 2002, 2007, 2010, 2023 Ricoh Company, Ltd.
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

#include "ModCommon.h"
#include "ModCharTrait.h"
#include "ModUnicodeCharTrait.h"

// to*() でプリミティブな型(intなど)の最大値/最小値が必要なのでインクルード
#include "limits.h"

const ModUnicodeCharTrait::CharacterInformatin ModUnicodeCharTrait::s_table[] = {
#include "ModUnicodeCharTrait.tbl"
};

namespace
{
	//
	//	ENUM local
	//
	//	NOTES
	//	特殊な用途(toInt で 16進表現の文字列を数値化する)のための定数
	//
	//	(Basic Latin の 0x0020 〜 0x007e と FullWidth の 0xff00 〜 0xff5f
	//	の並びは一致するので(スペースだけは違う)それぞれの先頭を利用する)
	//
	enum
	{
		s_codeBasicLatinSpace =			0x0020,		// スペース
		s_codeBasicLatinPlus =			0x002b,		// 正符合
		s_codeBasicLatinHyphenMinus =	0x002d,		// 負符合
		s_codeBasicLatinZero =			0x0030,		// ゼロ
		s_codeBasicLatinA =				0x0041		// 大文字 A
	};

	//
	//	FUNCTION local
	//	_getSign -- 
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	const ModUnicodeChar*& cp_
	//
	//	RETURN
	//	int
	//
	//	EXCEPTIONS
	//
	int
	_getSign(const ModUnicodeChar*& cp_)
	{
		int iSign = 1;
		
		if (*cp_ == s_codeBasicLatinHyphenMinus)
		{
			iSign = -1;
			++cp_;
		}
		else if (*cp_ == s_codeBasicLatinPlus)
		{
			++cp_;
		}
		
		return iSign;
	}
}

//	FUNCTION public
//	ModUnicodeCharTrait::compare -- Unicode 文字配列どうしを比較する
//
//	NOTES
//	高速化のため、len ありとなしを分けた。
//
//	ARGUMENTS
//		ModUnicodeChar*		l
//			r と比較するUnicode文字配列
//		ModUnicodeChar*		r
//			l と比較するUnicode文字配列
//		ModBoolean			caseSensitive
//			ModTrue または指定されないとき
//				大文字小文字を区別して比較する
//			ModFalse
//				大文字小文字を区別せずに比較する
//		ModSize				len
//			0 以外の値
//				両方の文字配列の先頭から比較する最大文字数
//			0 または指定されないとき
//				片方どちらかの文字配列がつきるまで比較する
//
//	RETURN
//		1
//			最初に異なるワイド文字が r より l のほうが大きい
//		0
//			l と r のすべてのワイド文字が等しい
//		-1
//			最初に異なるワイド文字の l より r のほうが大きい
//
//	EXCEPTIONS
//		なし

// static
int
ModUnicodeCharTrait::compare(const ModUnicodeChar*	l,
							 const ModUnicodeChar*	r)
{
	// l == r であることは少ないとして、高速化のために修正
	// if (l == r)
	//	return 0;
	// if (l == 0)
	//	return -1;
	// if (r == 0)
	//	return 1;
	if (l == 0)
		return (r) ? -1 : 0;
	if (r == 0)
		return (l) ? 1 : 0;

	for (; *l != null() && *r != null() && *l == *r; ++l, ++r)
		;

	// 同じである異なる方が多いとして比較順序を入れ換えた
	return (*l < *r) ? -1 : (*l == *r) ? 0 : 1;
}

int
ModUnicodeCharTrait::compare(const ModUnicodeChar*	l,
							 const ModUnicodeChar*	r,
							 const ModSize			len)
{
	if (l == 0)
		return (r) ? -1 : 0;
	if (r == 0)
		return (l) ? 1 : 0;

	ModSize	n = 0;
	for (; *l != null() && *r != null() && *l == *r; ++l, ++r)
		if (++n == len)
			return 0;

	return (*l < *r) ? -1 : (*l == *r) ? 0 : 1;
}

// static
int
ModUnicodeCharTrait::compare(const ModUnicodeChar*	l,
							 const ModUnicodeChar*	r,
							 const ModBoolean		caseSensitive,
							 const ModSize			len)
{
	if (caseSensitive)
		return compare(l, r, len);

	if (l == r)
		return 0;
	if (l == 0)
		return -1;
	if (r == 0)
		return 1;

	ModSize	n = 0;
	for (; *l != null() && *r != null() && toUpper(*l) == toUpper(*r);
		 ++l, ++r)
		if (++n == len)
			return 0;

	return (toUpper(*l) < toUpper(*r)) ? -1 :
		(toUpper(*l) == toUpper(*r)) ? 0 : 1;
}

//	FUNCTION public
//	ModUnicodeCharTrait::find -- Unicode 文字配列中の文字を探す
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeChar*		s
//			Unicode 文字を探す Unicode 文字配列
//		ModUnicodeChar		v
//			探すUnicode文字
//		ModBoolean			caseSensitive
//			ModTrue または指定されないとき
//				大文字小文字を区別して比較する
//			ModFalse
//				大文字小文字を区別せずに比較する
//		ModSize				len
//			0 以外の値
//				Unicode 文字配列の先頭から調べる最大文字数
//			0 または指定されないとき
//				Unicode 文字配列全体を調べる
//
//	RETURN
//		0 以外の値
//			見つかったUnicode文字を格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPTIONS
//		なし

// static
ModUnicodeChar*
ModUnicodeCharTrait::find(const ModUnicodeChar*		s,
						  const ModUnicodeChar		v,
						  const ModSize				len)
{
	if (s == 0)
		return 0;

	ModSize	n = 0;
	for (; *s != null(); ++s) {
		if (*s == v)
			return const_cast<ModUnicodeChar*>(s);
		if (++n == len)
			return 0;
	}

	return (v == null()) ? const_cast<ModUnicodeChar*>(s) : 0;
}

// static
ModUnicodeChar*
ModUnicodeCharTrait::find(const ModUnicodeChar*		s,
						  const ModUnicodeChar		v,
						  const ModBoolean			caseSensitive,
						  const ModSize				len)
{
	if (s == 0)
		return 0;
	if (caseSensitive)
		return find(s, v, len);

	ModSize	n = 0;
	for (; *s != null(); ++s) {
		if (toUpper(*s) == toUpper(v))
			return const_cast<ModUnicodeChar*>(s);
		if (++n == len)
			return 0;
	}

	return (v == null()) ? const_cast<ModUnicodeChar*>(s) : 0;
}

//	FUNCTION public
//	ModUnicodeCharTrait::find -- Unicode 文字配列中の文字列を探す
//
//	NOTES
//
//	ARGUMENTS
//	const char* s1
//		探索の対象となる文字列
//	const char* s2
//		string 中から探す文字列
//	const ModBoolean caseFlag
//		ケースセンシティブか否かを指定するフラグ
//	const ModSize maxLength
//		探索範囲を文字列の先頭からの文字数で指定する。
//		この引数を省略するか、0 を渡すと文字列全体を探索する。
//
//	RETURN
//	string1 中に string2 に一致する部分が見つかればその先頭の位置を指す
//	ポインタを返す。
//	string1 中に string2 に一致する部分がなければ 0 を返す。
//
//	EXCEPTIONS
//	なし

// static
ModUnicodeChar*
ModUnicodeCharTrait::find(const ModUnicodeChar*		s1,
						  const ModUnicodeChar*		s2,
						  const ModSize				maxLength)
{
	// 実装は全てModCharTrait.h から引用(2000/05/24)

	if (s1 == 0 || s2 == 0 || *s2 == ModUnicodeCharTrait::null())
		// string1 が何も指していなければ探すまでもない
		// string2 が空文字列なら探すまでもない
		return 0;

	// string2 の文字列長を得ておく
	ModSize length = ModUnicodeCharTrait::length(s2);

	const ModUnicodeChar* cp1 = s1;
	const ModUnicodeChar* cp2 = s2;

	while (1) {
		//
		// string1 を走査して string2 の先頭の文字と一致する場所を見つける。
		// 一致する場所が見つからないまま調べる文字数が maxLength を
		// 超えていたら 0 を返す。
		//
		cp2 = s2;
		ModSize count = 0;
		for (; *cp1 != ModUnicodeCharTrait::null(); cp1++) {
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

		if (*cp1 == ModUnicodeCharTrait::null() ||
			(maxLength > 0 && count + length >= maxLength))
			return 0;

		//
		// 最初の文字と一致する場所が見つかったので、2文字め以降を調べる。
		// 同時に1文字めと一致する最初の場所を candid に入れておく
		//

		const ModUnicodeChar* result = cp1;
		++cp1;
		++cp2;
		const ModUnicodeChar* candid = 0;
		for (;
			 (*cp1 == *cp2) && (*cp1 != ModUnicodeCharTrait::null())
				 && (*cp2 != ModUnicodeCharTrait::null());
			 ++cp1, ++cp2) {
			if ((candid == 0) && (*cp1 == *s2)) {
				// 先頭と一致する場所を記憶しておく
				candid = cp1;
			}
		}
		if (*cp2 == ModUnicodeCharTrait::null()) {
			// 見つかった
			return (ModUnicodeChar*)result;
		}
		// 見つからなかった
		if (candid)
			// 先頭と一致する場所があればそこまで戻す
			cp1 = candid;
	}
	return 0;
}

// static
ModUnicodeChar*
ModUnicodeCharTrait::find(const ModUnicodeChar*		s1,
						  const ModUnicodeChar*		s2,
						  const ModBoolean			caseFlag,
						  const ModSize				maxLength)
{
	// 実装は全てModCharTrait.h から引用(2000/05/24)

	if (s1 == 0 || s2 == 0 || *s2 == ModUnicodeCharTrait::null())
		// string1 が何も指していなければ探すまでもない
		// string2 が空文字列なら探すまでもない
		return 0;

	// ケースセンシティブなら普通の関数を呼ぶ
	if (caseFlag == ModTrue) {
		return ModUnicodeCharTrait::find(s1, s2, maxLength);
	}

	// string2 の文字列長を得ておく
	ModSize length = ModUnicodeCharTrait::length(s2);

	const ModUnicodeChar*	cp1 = s1;
	const ModUnicodeChar*	cp2 = s2;

	while (1) {
		//
		// string1 を走査して string2 の先頭の文字と一致する場所を見つける。
		// 一致する場所が見つからないまま調べる文字数が maxLength を
		// 超えていたら 0 を返す。
		//
		cp2 = s2;
		ModSize count = 0;
		for (; *cp1 != ModUnicodeCharTrait::null(); cp1++) {
			if (ModUnicodeCharTrait::toUpper(*cp1)
				== ModUnicodeCharTrait::toUpper(*cp2)) {
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

		if (*cp1 == ModUnicodeCharTrait::null() ||
			(maxLength > 0 && count + length >= maxLength))
			return 0;

		//
		// 最初の文字と一致する場所が見つかったので、2文字め以降を調べる。
		// 同時に1文字めと一致する最初の場所を candid に入れておく
		//
		const ModUnicodeChar* result = cp1;
		++cp1;
		++cp2;
		const ModUnicodeChar* candid = 0;
		for (;
			 (ModUnicodeCharTrait::toUpper(*cp1)
			  == ModUnicodeCharTrait::toUpper(*cp2))
				 && (*cp1 != ModUnicodeCharTrait::null())
				 && (*cp2 != ModUnicodeCharTrait::null());
			 ++cp1, ++cp2) {
			if ((candid == 0)
				&& (ModUnicodeCharTrait::toUpper(*cp1)
					== ModUnicodeCharTrait::toUpper(*s2))) {
				// 先頭と一致する場所を記憶しておく
				candid = cp1;
			}
		}
		if (*cp2 == ModUnicodeCharTrait::null()) {
			// 見つかった
			return (ModUnicodeChar*)result;
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
// ModUnicodeCharTrait::copy -- 文字列の複写
//
// NOTES
// この関数は ModUnicodeChar の配列をコピーするのに用いる。
//
// ARGUMENTS
// ModUnicodeChar* destination
//		コピー先を指すポインタ
// const ModUnicodeChar* source
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
ModUnicodeChar*
ModUnicodeCharTrait::copy(ModUnicodeChar*			destination,
						  const ModUnicodeChar*		source,
						  const ModSize				maxLength)
{
	ModSize count;
	ModUnicodeChar* returnValue;

	if (destination == source) {
		// 同じものを指していたらコピーするまでもない。
		return destination;
	}
	if (destination == 0) {
		return 0;
	}
	if (source == 0) {
		// source が 0 なら destination は変えない
		return destination;
	}

	returnValue = destination;
	//
	// destination, source を走査して source の値を destination にコピーする。
	// 領域が重なっていることは考慮しない。
	// コピーした文字数が maxLength に一致したらリターンする。
	//
	for (count = 0; *source != null(); source++, destination++) {
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

// FUNCTION public
// ModUnicodeCharTrait::length -- Unicode 文字配列中の文字数を得る
//
// NOTES
// Unicode 文字配列中の文字数を得る。渡された配列が NULL だった場合は
// 0 を返す
//
// ARGUMENTS
//		ModUnicodeChar*		s
//			文字数を調べる文字配列
//
// RETURN
//		得られた文字数
//		文字配列の末尾の ModUnicodeCharTrait::null は含まない
//
// EXCEPTIONS
//		なし

// static
ModSize
ModUnicodeCharTrait::length(const ModUnicodeChar* s)
{
	if (s == 0)
		return 0;

	const ModUnicodeChar*	p = s;
	for (; *p != null(); ++p) ;
	return (ModSize)(p - s);
}

//
// FUNCTION
// ModUnicodeCharTrait::to* -- 整数を表現する文字列の * 型 への変換
//
//	NOTES
//	この関数は数字列の文字列を base_ 進数値とみなして * 型の値に変換する。
//	文字列に数値または符合以外のものが含まれている場合と、基数が16を
//	越えた場合の動作は未定義(文字列先頭の 0x と 00 は無視する)。
//
// ARGUMENTS
// const ModUnicodeChar* string
//		処理対象の文字列
//		(Basic Latin の数字または符合)
//		(基数が 16 の場合だけ最初の 0 と X を無視する)
// const int base
//		基数、省略すると 10 になる。16 を超えてはいけない
//
// RETURN
// 数字列を数値に変換した結果を返す。
//
// EXCEPTIONS
// なし
//

// to int
int
ModUnicodeCharTrait::toInt(const ModUnicodeChar* string, const int base)
{
	// 文字列を整数値に変換
	ModUInt64	absValue;
	ModBoolean	negative;
	toInteger(string, base, absValue, negative);

	if (negative) {	// 負数の場合は下限を検査
		// (正の最大値より負の最大値の方が絶対値が１つ大きい)
		if (absValue > ModUInt64(INT_MAX) + 1) {
			ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
					 ModErrorLevelError);
		}
	} else if (absValue > INT_MAX) { // 正数の場合は上限を検査
		ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
				 ModErrorLevelError);
	}

	return negative ? ((-1) * (int)absValue) : (int)absValue;
}

// to unsigned int
unsigned int
ModUnicodeCharTrait::toUInt(const ModUnicodeChar* string, const int base)
{
	// 文字列を整数値に変換
	ModUInt64	absValue;
	ModBoolean	negative;
	toInteger(string, base, absValue, negative);
	
	// 下限と上限を検査
	if ((absValue != 0 && negative) || absValue > UINT_MAX) {
		ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
				 ModErrorLevelError);
	}

	return (unsigned int)absValue;
}

// to short
short
ModUnicodeCharTrait::toShort(const ModUnicodeChar* string, const int base)
{
	// 文字列を整数値に変換
	ModUInt64	absValue;
	ModBoolean	negative;
	toInteger(string, base, absValue, negative);

	if (negative) {	// 負数の場合は下限を検査
		// (正の最大値より負の最大値の方が絶対値が１つ大きい)
		if (absValue > ModUInt64(SHRT_MAX) + 1) {
			ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
					 ModErrorLevelError);
		}
	} else if (absValue > SHRT_MAX) { // 正数の場合は上限を検査
		ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
				 ModErrorLevelError);
	}

	return negative ? ((-1) * (short)absValue) : (short)absValue;
}

// to unsigned short
unsigned short
ModUnicodeCharTrait::toUShort(const ModUnicodeChar* string, const int base)
{
	// 文字列を整数値に変換
	ModUInt64	absValue;
	ModBoolean	negative;
	toInteger(string, base, absValue, negative);
	
	// 下限と上限を検査
	if ((absValue != 0 && negative) || absValue > USHRT_MAX) {
		ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
				 ModErrorLevelError);
	}

	return (unsigned short)absValue;
}

// to long
long
ModUnicodeCharTrait::toLong(const ModUnicodeChar* string, const int base)
{
	// 文字列を整数値に変換
	ModUInt64	absValue;
	ModBoolean	negative;
	toInteger(string, base, absValue, negative);

	if (negative) {	// 負数の場合は下限を検査
		// (正の最大値より負の最大値の方が絶対値が１つ大きい)
		if (absValue > ModUInt64(LONG_MAX) + 1) {
			ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
					 ModErrorLevelError);
		}
	} else if (absValue > LONG_MAX) { // 正数の場合は上限を検査
		ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
				 ModErrorLevelError);
	}

	return negative ? ((-1) * (long)absValue) : (long)absValue;
}

// to unsigned long
unsigned long
ModUnicodeCharTrait::toULong(const ModUnicodeChar* string, const int base)
{
	// 文字列を整数値に変換
	ModUInt64	absValue;
	ModBoolean	negative;
	toInteger(string, base, absValue, negative);
	
	// 下限と上限を検査
	if ((absValue != 0 && negative) || absValue > ULONG_MAX) {
		ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
				 ModErrorLevelError);
	}

	return (unsigned long)absValue;
}

// to ModInt64
ModInt64
ModUnicodeCharTrait::toModInt64(const ModUnicodeChar* string, const int base)
{
	// 文字列を整数値に変換
	ModUInt64	absValue;
	ModBoolean	negative;
	toInteger(string, base, absValue, negative);

	if (negative) {	// 負数の場合は下限を検査
		// (正の最大値より負の最大値の方が絶対値が１つ大きい)
		if (absValue > ModUInt64(ModInt64Max) + 1) {
			ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
					 ModErrorLevelError);
		}
	} else if (absValue > ModInt64Max) { // 正数の場合は上限を検査
		ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
				 ModErrorLevelError);
	}

	return negative ? ((-1) * (ModInt64)absValue) : (ModInt64)absValue;
}

// to ModUInt64
ModUInt64
ModUnicodeCharTrait::toModUInt64(const ModUnicodeChar* string, const int base)
{
	// 文字列を整数値に変換
	ModUInt64	absValue;
	ModBoolean	negative;
	toInteger(string, base, absValue, negative);

	// 下限を検査
	if (absValue != 0 && negative) {
		ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
				 ModErrorLevelError);
	}

	return absValue;
}

//
// FUNCTION
// ModUnicodeCharTrait::toDouble -- 文字列の duble への変換
//
// NOTES
// この関数は数字列の文字列を double に変換するのに用いる。
//
// この関数の実装は ModCharTarit.h にあった toFloat の実装をそっくり真似
// している(コメントまでいっしょ)。異なるのは、 char を ModUnicodeChar に
// 置き換えている事、それと、戻り値を float にキャストしていない事だけ。
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
double
ModUnicodeCharTrait::toDouble(const ModUnicodeChar* string)
{
	double returnValue = 0.0;
	int flag = 0;

	//
	// atof を使った方が速い。
	// ここのせいで遅いようなら atof を利用するように変更すべし。
	//
	if (string) {
		const ModUnicodeChar* cp = string;
		// 符号
		flag = _getSign(cp);
		// 仮数部
		for (; ModUnicodeCharTrait::isDigit(*cp); cp++) {
			returnValue *= 10;
			returnValue += (double)(((int)(*cp - '0')) & 0xff);
		}
		// 小数部
		int fraction = 0;
		if (*cp == ModUnicodeChar('.')) {
			for (cp++; ModUnicodeCharTrait::isDigit(*cp); cp++) {
				returnValue *= 10;
				returnValue += (double)(((int)(*cp - '0')) & 0xff);
				fraction--;
			}
		}
		// 指数部
		int exponential = 0;
		if (*cp == ModUnicodeChar('e') || *cp == ModUnicodeChar('E')) {
			cp++;
			int expFlag = _getSign(cp);
			for (; ModUnicodeCharTrait::isDigit(*cp); cp++)
			{
				exponential *= 10;
				exponential += (((int)(*cp - ModUnicodeChar('0'))) & 0xff);
			}
			exponential *= expFlag;
		}
		exponential += fraction;
		for (;exponential >= 1; exponential--) {
			returnValue *= 10.0;
		}
		for (;exponential <= -1; exponential++) {
			returnValue /= 10.0;
		}
	}

	return returnValue * flag;
}

#if 0 // 簡単にしてインライン化した
//
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
const ModUnicodeCharType
ModUnicodeCharTrait::getType(const ModUnicodeChar character)
{
	ModUnicodeCharType type = 0;
	
	// 注意 :
	// 文字種 none は「全てのビットが 0 である」ことを意味する特殊なもの
	// なので、このメソッドの中で調べる必要はない

	if (isNotused(character)) {
		type |= ModUnicodeCharTypes::notused;
	}

	if (isAlphabet(character)) {
		type |= ModUnicodeCharTypes::alphabet;
	}

	if (isUpper(character)) {
		type |= ModUnicodeCharTypes::upper;
	}

	if (isLower(character)) {
		type |= ModUnicodeCharTypes::lower;
	}

	if (isLetterOther(character)) {
		type |= ModUnicodeCharTypes::letterOther;
	}

	if (isDigit(character)) {
		type |= ModUnicodeCharTypes::digit;
	}

	if (isXdigit(character)) {
		type |= ModUnicodeCharTypes::xdigit;
	}

	if (isSymbol(character)) {
		type |= ModUnicodeCharTypes::symbol;
	}

	if (isSpace(character)) {
		type |= ModUnicodeCharTypes::space;
	}
 
	if (isAscii(character)) {
		type |= ModUnicodeCharTypes::ascii;
	}

	if (isHankakuKana(character)) {
		type |= ModUnicodeCharTypes::hankakuKana;
	}

	if (isHiragana(character)) {
		type |= ModUnicodeCharTypes::hiragana;
	}

	if (isKatakana(character)) {
		type |= ModUnicodeCharTypes::katakana;
	}

	if (isGreek(character)) {
		type |= ModUnicodeCharTypes::greek;
	}

	if (isRussian(character)) {
		type |= ModUnicodeCharTypes::russian;
	}

	if (isLine(character)) {
		type |= ModUnicodeCharTypes::line;
	}

	if (isKanji(character)) {
		type |= ModUnicodeCharTypes::kanji;
	}

	if (isControl(character)) {
		type |= ModUnicodeCharTypes::control;
	}

	if (isFormat(character)) {
		type |= ModUnicodeCharTypes::format;
	}

	if (isSurrogate(character)) {
		type |= ModUnicodeCharTypes::surrogate;
	}

	if (isGaiji(character)) {
		type |= ModUnicodeCharTypes::gaiji;
	}

	return type;
}
#endif

//
// PRIVATE
//


//
//	FUNCTION
//	ModUnicodeCharTrait::toInteger -- 文字列を数値に変換
//
//	NOTES
//	文字列を整数値(いちばん大きな値を表現できる整数型)に変換。
//	to*() 関数の下請けとして使用。
//
//  (!!注意!! 絶対値が ModUInt64 の範囲を越える場合の動作は不定
//	           -- 検査コストが高くなりそうなので検査しない)
//
//	ARGUMENTS
//	const ModUnicodeChar*	string_
//		整数値を表す文字列
//		(Basic Latin の数字か符合)
//		(基数が 16 の場合だけ最初の 0 と X を無視する)
//	const int				base_
//		基数、省略すると 10 になる。16 を超えてはいけない
//	ModUInt64&				value_
//		(戻り値) 文字列を整数値(unsigned)に変換した結果
//	ModBoolean&				negative_
//		(戻り値) 整数値に負符号が付いていた場合だけ ModTrue
//
//	RETURN
//	先頭から数字列が続く部分を数値に変換した結果を返す。
//	string の先頭に数字がない場合は 0 を返す。
//
//	EXCEPTIONS
//

void
ModUnicodeCharTrait::toInteger(const ModUnicodeChar*	string_,
							   const int				base_,
							   ModUInt64&				value_,
							   ModBoolean&				negative_)
{
	value_		= 0;		// 変換結果を0で初期化
	negative_	= ModFalse;	// 符号は正で初期化

	if (string_) {
		const ModUnicodeChar*	cp		= string_;

		// ModUInt64 で表現できる範囲を検査するための定数
		const ModUInt64			majorLimit	= ModUInt64Max / ModUInt64(base_);
		const ModUInt64			minorLimit	= ModUInt64Max % ModUInt64(base_);

		// 符合の決定
		negative_ = (_getSign(cp) == 1) ? ModFalse : ModTrue;
		
		if (base_ <= 10) {
			// 文字列を数値に変換
			for (; isDigit(*cp); cp++) {
				ModUnicodeChar character = *cp;
				int tmpValue = character - s_codeBasicLatinZero;

				// base_ を乗算するとオーバーフローする場合、または、
				// tmpValue_ を加算するとオーバーフローする場合はエラー
				if (value_ > majorLimit
					|| (value_ == majorLimit && tmpValue > minorLimit)) {
					ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
							 ModErrorLevelError);
				}
					
				value_ *= base_;
				value_ += tmpValue;
			}
		} else {
			// 16進数の場合は先頭に"0x" があるかもしれないので除去する
			// (大文字 X は大文字 A に 23 加算した値 だと仮定した処理)
			if (base_ == 16
				&& *cp == s_codeBasicLatinZero
				&& toUpper(*(cp + 1)) == s_codeBasicLatinA + 23) {
				cp+=2;
			}
			// 文字列を数値に変換 
			for (; isXdigit(*cp); cp++) {
				ModUnicodeChar character = *cp;
				int tmpValue =
					(isDigit(character) ?
					 (character - s_codeBasicLatinZero)
					 : (toUpper(character) - s_codeBasicLatinA + 10));
				if (tmpValue > base_) {
					break;
				}

				// base_ を乗算するとオーバーフローする場合、または、
				// tmpValue_ を加算するとオーバーフローする場合はエラー
				if (value_ > majorLimit
					|| (value_ == majorLimit && tmpValue > minorLimit)) {
					ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
							 ModErrorLevelError);
				}

				value_ *= base_;
				value_ += tmpValue;
			}
		}	
	}
}

//
// Copyright (c) 1999, 2002, 2007, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
