// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModWideCharTrait.cpp -- ModWideCharTrait のメンバ定義
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


#include "ModCommon.h"
#include "ModCharTrait.h"
#include "ModWideCharTrait.h"

//	FUNCTION public
//	ModWideCharTrait::compare -- ワイド文字配列どうしを比較する
//
//	NOTES
//
//	ARGUMENTS
//		ModWideChar*		l
//			r と比較するワイド文字配列
//		ModWideChar*		r
//			l と比較するワイド文字配列
//		ModBoolean			caseSensitive
//			ModTrue または指定されないとき
//				大文字小文字を区別して比較する
//			ModFalse
//				大文字小文字を区別せずに比較する
//		ModSize				len
//			0 以外の値
//				両方のワイド文字配列の先頭から比較する最大文字数
//			0 または指定されないとき
//				片方どちらかのワイド文字配列がつきるまで比較する
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
ModWideCharTrait::compare(const ModWideChar* l,
						  const ModWideChar* r, ModSize len)
{
	if (l == r)
		return 0;
	if (l == 0)
		return -1;
	if (r == 0)
		return 1;

	ModSize	n = 0;
	for (; *l != ModWideCharTrait::null() &&
			 *r != ModWideCharTrait::null() && *l == *r; ++l, ++r)
		if (++n == len)
			return 0;

	return (*l == *r) ? 0 : (*l < *r) ? -1 : 1;
}

// static
int
ModWideCharTrait::compare(const ModWideChar* l, const ModWideChar* r,
						  ModBoolean caseSensitive, ModSize len)
{
	if (caseSensitive)
		return ModWideCharTrait::compare(l, r, len);

	if (l == r)
		return 0;
	if (l == 0)
		return -1;
	if (r == 0)
		return 1;

	ModSize	n = 0;
	for (; *l != ModWideCharTrait::null() &&
			 *r != ModWideCharTrait::null() &&
			 ModWideCharTrait::toUpper(*l) == ModWideCharTrait::toUpper(*r);
		 ++l, ++r)
		if (++n == len)
			return 0;

	return (ModWideCharTrait::toUpper(*l) ==
			ModWideCharTrait::toUpper(*r)) ? 0 :
		(ModWideCharTrait::toUpper(*l) <
		 ModWideCharTrait::toUpper(*r)) ? -1 : 1;
}

//	FUNCTION public
//	ModWideCharTrait::find -- ワイド文字配列中のワイド文字を探す
//
//	NOTES
//
//	ARGUMENTS
//		ModWideChar*		s
//			ワイド文字を探すワイド文字配列
//		ModWideChar			v
//			探すワイド文字
//		ModBoolean			caseSensitive
//			ModTrue または指定されないとき
//				大文字小文字を区別して比較する
//			ModFalse
//				大文字小文字を区別せずに比較する
//		ModSize				len
//			0 以外の値
//				ワイド文字配列の先頭から調べる最大文字数
//			0 または指定されないとき
//				ワイド文字配列全体を調べる
//
//	RETURN
//		0 以外の値
//			見つかったワイド文字を格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPTIONS
//		なし

// static
ModWideChar*
ModWideCharTrait::find(const ModWideChar* s, ModWideChar v, ModSize len)
{
	if (s == 0)
		return 0;

	ModSize	n = 0;
	for (; *s != ModWideCharTrait::null(); ++s) {
		if (*s == v)
			return const_cast<ModWideChar*>(s);
		if (++n == len)
			return 0;
	}

	return (v == ModWideCharTrait::null()) ? const_cast<ModWideChar*>(s) : 0;
}

// static
ModWideChar*
ModWideCharTrait::find(const ModWideChar* s, ModWideChar v,
					   ModBoolean caseSensitive, ModSize len)
{
	if (s == 0)
		return 0;
	if (caseSensitive)
		return ModWideCharTrait::find(s, v, len);

	ModSize	n = 0;
	for (; *s != ModWideCharTrait::null(); ++s) {
		if (ModWideCharTrait::toUpper(*s) == ModWideCharTrait::toUpper(v))
			return const_cast<ModWideChar*>(s);
		if (++n == len)
			return 0;
	}

	return (v == ModWideCharTrait::null()) ? const_cast<ModWideChar*>(s) : 0;
}

//	FUNCTION public
//	ModWideCharTrait::length -- ワイド文字配列中の文字数を得る
//
//	NOTES
//
//	ARGUMENTS
//		ModWideChar*		s
//			文字数を調べるワイド文字配列
//
//	RETURN
//		得られた文字数
//		ワイド文字配列の末尾の ModWideCharTrait::null は含まない
//
//	EXCEPTIONS
//		なし

// static
ModSize
ModWideCharTrait::length(const ModWideChar* s)
{
	if (s == 0)
		return 0;

	const ModWideChar*	p = s;
	for (; *p != ModWideCharTrait::null(); ++p) ;
	return (ModSize)(p - s);
}

//	FUNCTION public
//	ModWideCharTrait::length --
//		C 文字列をワイド文字列へ変換したときの文字数を得る
//
//	NOTES
//		C 文字列をワイド文字列へ変換できたところまでの文字数を得る
//
//	ARGUMENTS
//		char*				s
//			ワイド文字列へ変換したときの文字数を調べる C 文字列
//		ModKanjiCode::KanjiCodeType	code
//			指定されたとき
//				調べる C 文字列の日本語コード
//			指定されないとき
//				ModKanjiCode::euc を指定されたものとみなす
//
//	RETURN
//		得られた文字数
//		ただし、変換したときの末尾の ModWideCharTrait::null は含まない
//
//	EXCEPTIONS
//		なし

ModSize
ModWideCharTrait::length(const char* s, ModKanjiCode::KanjiCodeType code)
{
	if (s == 0)
		return 0;

	//【注意】	変換後のワイド文字 + ModWideCharTrait::null が
	//			格納できればよい

	ModWideChar	tmp[2];
	ModSize	n = 0;

	for (; *s != ModCharTrait::null(); ++n) {

		// ワイド文字を 1 文字取り出す

		s = ModKanjiCode::toWide(tmp, s, 2, code);

		if (*tmp == ModWideCharTrait::null() && *s != ModCharTrait::null())

			// 変換に失敗したので、これまでとする

			break;
	}

	return n;
}

//
// FUNCTION 
// ModWideCharTrait::copy -- 文字列の複写
//
// TEMPLATE ARGUMENTS
// class ModWideChar
//		文字の型。char または ModWideChar。
//
// NOTES
// この関数は ModWideChar の配列をコピーするのに用いる。
//
// ARGUMENTS
// ModWideChar* destination
//		コピー先を指すポインタ
// const ModWideChar* source
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
ModWideChar*
ModWideCharTrait::copy(ModWideChar* destination, const ModWideChar* source,
					   const ModSize maxLength)
{
	ModSize count;
	ModWideChar* returnValue;

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
	for (count = 0; *source != ModWideCharTrait::null();
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

//
// FUNCTION
// ModWideCharTrait::makeWideChar -- char* から ModWideChar を作る
//
// NOTES
// この関数は char* の指すマルチバイト文字を ModWideChar に
// 変換するために用いる。
//
// ARGUMENTS
// const char* pointer
//		変換するマルチバイト文字を指すポインター
// ModKanjiCode::KanjiCodeType kanjiCode
//		string に使われている漢字コード。
//		無指定時は EUC が使用される。
//
// RETURN
// 成功した場合は変換された ModWideChar を返す。
// 失敗した場合は 0 を返す。
//
// EXCEPTIONS
// なし
//
const ModWideChar
ModWideCharTrait::makeWideChar(const char* pointer,
							   ModKanjiCode::KanjiCodeType kanjiCode)
{
	char tmpBuffer[4];

	if (kanjiCode != ModKanjiCode::euc) {
		ModKanjiCode::toEuc(tmpBuffer, pointer, 4, kanjiCode);
		pointer = (const char*)tmpBuffer;
	}

	if (pointer == 0) {
		return 0;
	}

	if (((int)((unsigned char)*pointer) & 0x80) == 0x00) { // ASCII
		return (ModCodeMaskSet0 | *pointer);
	}

	switch (*pointer) {
	case (char)0x8e:					// 半角カナのリードバイト
		if ((*++pointer & 0x80) == 0x00) {
			// 途中で切れている
			return ModCodeMaskSet2X;
		}
		return (ModCodeMaskSet2 | (*pointer & 0x7f));
	case (char)0x8f:
		++pointer;
		if ((*pointer & 0x80) == 0x00) {
			// 1バイト目で切れている
			return ModCodeMaskSet3X1;
		}
		if ((*(pointer + 1) & 0x80) == 0x00) {
			// 2バイト目で切れている
			return (ModCodeMaskSet3X2 | (*pointer & 0x7f));
		}
		return (ModCodeMaskSet3
				| ((*pointer & 0x7f) << 7) | (*(pointer + 1) & 0x7f));
	default:
		if ((*(pointer + 1) & 0x80) == 0x00) {
			// 途中で切れている
			return (ModCodeMaskSet1X | (*pointer & 0x7f));
		}
		return (ModCodeMaskSet1
				| ((*pointer & 0x7f) << 7) | (*(pointer + 1) & 0x7f));
	}
}

//
// FUNCTION
// ModWideCharTrait::makeWideChar -- int から ModWideChar を作る
//
// NOTES
// この関数はEUCコードを格納した int の表すマルチバイト文字を ModWideChar に
// 変換するために用いる。
//
// ARGUMENTS
// int code
//		変換するマルチバイト文字を表す int
//
// RETURN
// 成功した場合は変換された ModWideChar を返す。
//
// EXCEPTIONS
// なし
//
const ModWideChar
ModWideCharTrait::makeWideChar(int code)
{
	if ((code & 0x00000080) == 0x00000000) { // ASCII?
		return (ModCodeMaskSet0 | (code & 0x7f));
	}
	if ((code & 0x00ffff00) == 0x00008e00) { // SS2?
		return (ModCodeMaskSet2 | (code & 0x7f));
	}
	if ((code & 0x00ff0000) == 0x008f0000) { // SS3?
		return (ModCodeMaskSet3 | (((code & 0x7f00) >> 1) | (code & 0x7f)));
	}
	// SS1
	return (ModCodeMaskSet1 | (((code & 0x7f00) >> 1) | (code & 0x7f)));
}

//
// FUNCTION
// ModWideCharTrait::byteSize -- ワイド文字のコードのバイト長を得る
//
// NOTES
// この関数は ModWideChar で表されたワイド文字のコードが占める
// バイト長を得るために用いる。
//
// ARGUMENTS
// const ModWideChar character
//		コードのバイト長を調べる対象となるワイド文字
//
// RETURN
// 3バイトコードなら3、2バイトコードなら2、1バイトコードなら1を返す。
// ModWideChar の内容が不正の場合は1を返す。
//
// EXCEPTIONS
// その他
// なし
//

// static
ModSize
ModWideCharTrait::byteSize(ModWideChar v)
{
	v &= codeMask;
	return
		(v == ModCodeMaskSet0) ? 1 :
		(v == ModCodeMaskSet1 ||
		 v == ModCodeMaskSet2 || v == ModCodeMaskSet3X2) ? 2 :
		(v == ModCodeMaskSet3) ? 3 : 1;
}

//
// FUNCTION
// ModWideCharTrait::byteSize -- char* の指すEUCコードのバイト長を得る
//
// NOTES
// この関数は char の配列で表されたマルチバイト文字がEUCコードで
// 占めるバイト長を得るために用いる。
//
// ARGUMENTS
// const char* pointer
//		コードのバイト長を調べる対象となるchar配列へのポインタ
// ModKanjiCode::KanjiCodeType kanjiCode
//		string に使われている漢字コード。
//		無指定時は EUC が使用される。
//
// RETURN
// 原則として、3バイトコードなら3、2バイトコードなら2、
// 1バイトコードなら1が、pointerが0なら0を返す。
// ただし、pointer の指すEUCコードが不完全な場合は差し引かれた数値を返す。
// すなわち、3バイトコードでも3バイトめが不正な場合は2を返し、
// 3バイトコード、または2バイトコードで2バイトめが不正な場合は1を返す。
// kanjiCode が EUC でないときは EUC に変換して行なう。
//
// EXCEPTIONS
// その他
// なし
//
const ModSize
ModWideCharTrait::byteSize(const char* pointer,
						   ModKanjiCode::KanjiCodeType kanjiCode)
{
	char tmpBuffer[4];
	if (kanjiCode != ModKanjiCode::euc) {
		ModKanjiCode::toEuc(tmpBuffer, pointer, 4, kanjiCode);
		pointer = (const char*)tmpBuffer;
	}

	return (pointer == 0)?0:
		((*pointer == (char)0x8f && (*(pointer + 1) & 0x80) != 0x00
		  && (*(pointer + 2) & 0x80) != 0x00)?3:
		 ((((*pointer & 0x80) != 0x00) && (*(pointer + 1) & 0x80) != 0x00)?2:
		 1));
}

//
// FUNCTION
// ModWideCharTrait::convertToString -- ワイド文字をchar配列に変換し、指定したバッファに入れる
//
// NOTES
// この関数は ModWideChar で表されるワイド文字を kanjiCode で表された
// char 配列に変換し、結果を指定したバッファに格納するために用いる。
// null-terminate はしない。
//
// ARGUMENTS
// char* target
//		結果を格納するバッファのアドレス
// ModWideChar character
//		変換するワイド文字
// ModKanjiCode::KanjiCodeType kanjiCode
//		target に格納するのに用いる漢字コード。
//		省略時は euc が用いられる。
//		ここに shiftJis、euc 以外を指定してはいけない。
//
// RETURN
// target に格納したバイト数を返す。
//
// EXCEPTIONS
// なし
//
ModSize
ModWideCharTrait::convertToString(char* target, ModWideChar character,
								  ModKanjiCode::KanjiCodeType kanjiCode)
{
	char tmpBuffer[3];					// euc のものを入れる

	ModAssert(kanjiCode == ModKanjiCode::euc
			  || kanjiCode == ModKanjiCode::shiftJis);

	switch (character & codeMask) {
	case ModCodeMaskSet0: // ASCII
		*target = (char)(character & 0xff);
		return 1;
	case ModCodeMaskSet2: // SS2
		if (kanjiCode == ModKanjiCode::euc) {
			*target++ = (char)0x8e;
			*target++ = (char)((character & 0xff) | 0x80);
			return 2;
		}
		// Shift Jis なのでそのまま2バイトめを入れる
		*target++ = (char)((character & 0xff) | 0x80);
		return 1;
	case ModCodeMaskSet3: // SS3
		if (kanjiCode == ModKanjiCode::euc) {
			*target++ = (char)0x8f;
			*target++ = (char)(((character & (0xff << 7)) >> 7) | 0x80);
			*target++ = (char)((character & 0xff) | 0x80);
			return 3;
		}
		// Shift Jis なので一旦別の配列に入れる
		tmpBuffer[0] = (char)0x8f;
		tmpBuffer[1] = (char)(((character & (0xff << 7)) >> 7) | 0x80);
		tmpBuffer[2] = (char)((character & 0xff) | 0x80);
		break;
	case ModCodeMaskSet1X:				// 壊れたSS1
		if (kanjiCode == ModKanjiCode::euc) {
			*target = (char)((character & 0xff) | 0x80);
			return 1;
		}
		// Shift Jis なので一旦別の配列に入れる
		tmpBuffer[0] = (char)((character & 0xff) | 0x80);
		break;
	case ModCodeMaskSet2X:				// 壊れたSS2
		if (kanjiCode == ModKanjiCode::euc) {
			*target = (char)0x8e;
			return 1;
		}
		// Shift Jis なので何も入れない
		return 0;
	case ModCodeMaskSet3X1:				// 1バイト目で壊れたSS3
		if (kanjiCode == ModKanjiCode::euc) {
			*target = (char)0x8f;
			return 1;
		}
		// Shift Jis なので一旦別の配列に入れる
		tmpBuffer[0] = (char)0x8f;
		break;
	case ModCodeMaskSet3X2:				// 2バイト目で壊れたSS3
		if (kanjiCode == ModKanjiCode::euc) {
			*target++ = (char)0x8f;
			*target++ = (char)((character & 0xff) | 0x80);
			return 2;
		}
		// Shift Jis なので一旦別の配列に入れる
		tmpBuffer[0] = (char)0x8f;
		tmpBuffer[1] = (char)((character & 0xff) | 0x80);
		break;
	default:							// SS1
		if (kanjiCode == ModKanjiCode::euc) {
			*target++ = (char)(((character & (0xff << 7)) >> 7) | 0x80);
			*target++ = (char)((character & 0xff) | 0x80);
			return 2;
		}
		// Shift Jis なので一旦別の配列に入れる
		tmpBuffer[0] = (char)(((character & (0xff << 7)) >> 7) | 0x80);
		tmpBuffer[1] = (char)((character & 0xff) | 0x80);
		break;
	}

	ModAssert(kanjiCode == ModKanjiCode::shiftJis);
	// shiftJis に変換
	int c1;
	int c2;

	c1 = ((int)(((unsigned char)tmpBuffer[0]) & 0x7f) + 0xe1) >> 1;
	c2 = (int)(((unsigned char)tmpBuffer[1]) & 0x7f);

	if (c1 > 0x9f) {
		c1 += 0x40;
	}
	if ((tmpBuffer[0] & 0x01) == 0) {
		c2 += 0x7e;
	} else {
		c2 += 0x1f;
		if (c2 > 0x7e) {
			c2++;
		}
	}
	*target++ = (char)c1;
	*target++ = (char)c2;

	return 2;
}

//
// FUNCTION
// ModWideCharTrait::convertToLong -- ワイド文字の EUC コードを int で得る
//
// NOTES
// この関数はワイド文字が表す文字を EUC で表したときのコードを
// int で得るために用いる。
//
// ARGUMENTS
// const ModWideChar character
//		対象のワイド文字
//
// RETURN
// character を EUC コードで表したときのコードを返す。
//
// EXCEPTIONS
// なし
//
int
ModWideCharTrait::convertToLong(const ModWideChar character)
{
	switch (character & codeMask) {
	case ModCodeMaskSet0: // ASCII
		return (int)(character & 0xff);
	case ModCodeMaskSet2: // SS2
		return (0x00008e00 | ((character & 0xff) | 0x80));
	case ModCodeMaskSet3: // SS3
		return ((0x008f0000
				 | (((character & (0xff << 7)) << 1) | 0x8000))
				 | ((character & 0xff) | 0x80));
	case ModCodeMaskSet1X:				// 壊れたSS1
		return (int)((character & 0xff) | 0x80);
	case ModCodeMaskSet2X:				// 壊れたSS2
		return (int)0x8e;
	case ModCodeMaskSet3X1:				// 1バイト目で壊れたSS3
		return (int)0x8f;
	case ModCodeMaskSet3X2:				// 2バイト目で壊れたSS3
		return (int)(0x8f00 | ((character & 0xff) | 0x80));
	default:							// SS1
		return ((((character & (0xff << 7)) << 1) | 0x8000)
				| ((character & 0xff) | 0x80));
	}
}

//
// FUNCTION
// ModWideCharTrait::convertToLong -- マルチバイト文字のEUCコードを int で得る
//
// NOTES
// この関数はchar配列で表す文字を EUC で表したときのコードを
// int で得るために用いる。
//
// ARGUMENTS
// const char* character
//		対象のマルチバイト文字
// ModKanjiCode::KanjiCodeType kanjiCode
//		string に使われている漢字コード。
//		無指定時は EUC が使用される。
//
// RETURN
// character を EUC コードで表したときのコードを返す。
//
// EXCEPTIONS
// なし
//
int
ModWideCharTrait::convertToLong(const char* character,
								ModKanjiCode::KanjiCodeType kanjiCode)
{
	int i;
	ModSize byteSize;
	int returnValue;
	char tmpBuffer[4];

	if (kanjiCode != ModKanjiCode::euc) {
		ModKanjiCode::toEuc(tmpBuffer, character, 4, kanjiCode);
		character = (const char*)tmpBuffer;
	}

	byteSize = ModWideCharTrait::byteSize(character, ModKanjiCode::euc);

	returnValue = 0;
	for (i = 0; (ModSize)i < byteSize; i++) {
		returnValue <<= 8;
		returnValue += *(character + i);
	}
	return returnValue;
}

//
// FUNCTION
// ModWideCharTrait::getType -- ワイド文字のタイプを得る
//
// NOTES
// この関数は ModWideChar で表された文字の文字種を得るのに用いる。
//
// ARGUMENTS
// const ModWideChar character
//		文字種を調べたい文字
//
// RETURN
// 文字種に対応するビットの立った ModWideCharType を返す。
//
// EXCEPTIONS
// なし
//
const ModWideCharType
ModWideCharTrait::getType(const ModWideChar character)
{
	ModWideCharType type = 0;
	switch (character & codeMask) {
	case ModCodeMaskSet0: // ASCII
		type |= ModWideCharTrait::ascii;
		//
		// alphabet は 01000001〜01011010
		//             01100001〜01111010 である。
		// したがって、01*????? であることと、????? の部分が 1〜26 であること
		// によって判断できる。
		//
		if ((character & 0xffffffc0) == 0x00000040
			&& (unsigned int)((character & 0xffffff1f) - 1) <= 25UL) {
			type |= ModWideCharTrait::alphabet;
			return type;
		}
		//
		// digit は 0x30 〜 0x39 である。
		//
		if ((unsigned int)(character - codeAsciiZero)
			<= (unsigned int)(codeAsciiNine - codeAsciiZero)) {
			type |= ModWideCharTrait::digit;
			return type;
		}
		//
		// space は ' '、'\n'、'\t'、'\r' のいずれか
		//
		if (character == (ModWideChar)' ' || character == (ModWideChar)'\n'
			|| character == (ModWideChar)'\t'
			|| character == (ModWideChar)'\r') {
			type |= ModWideCharTrait::space;
			return type;
		}
		//
		// symbol は上記以外でさらに 0x21 以上、0x7e 以下である
		//
		if ((unsigned int)(character - 0x21) <= (0x7eUL - 0x21UL)) {
			type |= ModWideCharTrait::symbol;
			return type;
		}
		break;
	case ModCodeMaskSet1: // SS1
		//
		// alphabet の下2バイトは 11000001〜11011010
		//                        11100001〜11111010 である。
		// したがって、11*????? であることと、????? の部分が 1〜26 であること
		// によって判断できる。
		//
		if ((character & 0xffffffc0) == 0x300011c0
			&& (unsigned int)((character & 0xcfffee1f) - 1) <= 25UL) {
			type |= ModWideCharTrait::alphabet;
			return type;
		}
		//
		// digit は 0x300011b0 〜 0x300011b9 である。
		//
		if ((unsigned int)(character - codeEUCZero)
			<= (unsigned int)(codeEUCNine - codeEUCZero)) {
			type |= ModWideCharTrait::digit;
			return type;
		}
		//
		// 第一水準漢字は 0x30001821 〜 0x300027d3 である。
		//
		if ((unsigned int)(character - codeKanjiFirstStart)
			<= (unsigned int)(codeKanjiFirstEnd - codeKanjiFirstStart)) {
			type |= ModWideCharTrait::kanjiFirst;
			return type;
		}
		//
		// ひらがなは 0x30001221 〜 0x30001273
		//
		if ((unsigned int)(character - codeHiraganaXa)
			<= (unsigned int)(codeHiraganaN - codeHiraganaXa)) {
			type |= ModWideCharTrait::hiragana;
			return type;
		}
		//
		// カタカナは 0x300012a1 〜 0x300012f6
		//
		if ((unsigned int)(character - codeKatakanaXa)
			<= (unsigned int)(codeKatakanaXke - codeKatakanaXa)) {
			type |= ModWideCharTrait::katakana;
			return type;
		}
		//
		// 空白は 0x300010a1
		// 記号ともみなす
		//
		if (character == codeEUCSpace) {
			type |= (ModWideCharTrait::space | ModWideCharTrait::symbol);
			return type;
		}
		//
		// 第二水準漢字は 0x30002821 〜 0x30003a26 である。
		//
		if ((unsigned int)(character - codeKanjiSecondStart)
			<= (unsigned int)(codeKanjiSecondEnd - codeKanjiSecondStart)) {
			type |= ModWideCharTrait::kanjiSecond;
			return type;
		}
		//
		// ギリシャ文字は 0x30001321 〜 0x30001338
		//                0x30001341 〜 0x30001358 である
		// 下2バイトの上位に共通部分がないので alphabet のような判定法は
		// 使えない。
		//
		if ((unsigned int)(character - codeGreekSmallA)
			<= (unsigned int)(codeGreekSmallZ - codeGreekSmallA)
			|| (unsigned int)(character - codeGreekLargeA)
			<= (unsigned int)(codeGreekLargeZ - codeGreekLargeA)) {
			type |= ModWideCharTrait::greek;
			return type;
		}
		//
		// ロシア文字は 0x300013a1 〜 0x300013c1
		//              0x300013d1 〜 0x300013f1 である
		// 下2バイトの上位に共通部分がないので alphabet のような判定法は
		// 使えない。
		//
		if ((unsigned int)(character - codeRussianSmallA)
			<= (unsigned int)(codeRussianSmallZ - codeRussianSmallA)
			|| (unsigned int)(character - codeRussianLargeA)
			<= (unsigned int)(codeRussianLargeZ - codeRussianLargeA)) {
			type |= ModWideCharTrait::russian;
			return type;
		}
		//
		// 記号は 0x300010a1 〜 0x300010fe
		//        0x30001121 〜 0x3000112e
		//        0x3000113a 〜 0x30001141
		//        0x3000114a 〜 0x30001150
		//        0x3000115c 〜 0x3000116a
		//        0x30001172 〜 0x30001179
		//        0x3000117e
		// 記号&線 は
		//        0x30001421 〜 0x30001440
		//
		if ((unsigned int)(character - 0x300010a1)
			<= (unsigned int)(0x3000117e - 0x300010a1)) {
			if ((unsigned int)(character - 0x300010a1)
				<= (unsigned int)(0x300010fe - 0x300010a1)
				|| (unsigned int)(character - 0x30001121)
				<= (unsigned int)(0x3000112e - 0x30001121)
				|| (unsigned int)(character - 0x3000113a)
				<= (unsigned int)(0x30001141 - 0x3000113a)
				|| (unsigned int)(character - 0x3000114a)
				<= (unsigned int)(0x30001150 - 0x3000114a)
				|| (unsigned int)(character - 0x3000115c)
				<= (unsigned int)(0x3000116a - 0x3000115c)
				|| (unsigned int)(character - 0x30001172)
				<= (unsigned int)(0x30001179 - 0x30001172)
				|| character == 0x3000117e) {
				type |= ModWideCharTrait::symbol;
			}
			return type;
		}
		if ((unsigned int)(character - 0x30001421)
			<= (unsigned int)(0x30001440 - 0x30001421)) {
			type |= (ModWideCharTrait::symbol | ModWideCharTrait::line);
			return type;
		}
		break;
	case ModCodeMaskSet2: // SS2(半角カナ)
		type |= ModWideCharTrait::hankakuKana;
		//
		// 半角カナの記号は 0x10000021 〜 0x10000025
		//                  0x1000005e 〜 0x1000005f
		//                  0x10000030
		//
		if (((unsigned int)(character - 0x10000021)
			<= (unsigned int)(0x10000025 - 0x10000021))
			|| ((unsigned int)(character - 0x1000005e)
				<= (unsigned int)(0x1000005f - 0x1000005e))
			|| (character == 0x10000030)) {
			type |= ModWideCharTrait::symbol;
		}
		if (((unsigned int)(character - 0x10000026)
			 <= (unsigned int)(0x1000005d - 0x10000026)
			 || character == 0x1000005e || character == 0x1000005f)) {
			type |= ModWideCharTrait::katakana;
		}
		break;
	case ModCodeMaskSet3: // SS3
		type |= ModWideCharTrait::gaiji;
		break;
	default:							// 壊れた奴のタイプは0
		break;
	}
	return type;
}

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
