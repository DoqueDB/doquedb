// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModWideString.cpp -- ModWideString のメンバ定義
// 
// Copyright (c) 1997, 1999, 2023 Ricoh Company, Ltd.
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
#include "ModOsDriver.h"
#include "ModError.h"
#include "ModWideString.h"
#include "ModCharTrait.h"
#include "ModWideCharTrait.h"
#include "ModKanjiCode.h"

//	VARIABLE private
//	ModWideString::_allocateStepDefault -- バッファ領域の拡張単位
//
//	NOTES
//		ワイド文字列を格納するためのバッファ領域を
//		拡張するときの単位のデフォルト値(B 単位)

const ModSize	ModWideString::_allocateStepDefault = 16 * sizeof(ModWideChar);

//	FUNCTION public
//	ModWideString::serialize --
//		ModWideChar を文字単位とする文字列のシリアライザー
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&			archiver
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ModWideString::serialize(ModArchive& archiver)
{
	ModSize	len;

	if (archiver.isStore()) {
		len = this->getLength();
		archiver << _allocateStep;
		archiver << len;
		archiver << _size;
		(void) archiver.writeArchive(this->buffer, len);
	} else {
		archiver >> _allocateStep;
		archiver >> len;
		archiver >> _size;

		// 読み出される文字数ぶんの領域を確保し、
		// 空文字列が格納されていることとする

		this->reallocate(len);
		this->clear();

		if (len) {

			// 文字列を読み出し、末尾に ModWideCharTrait::null を付加する

			(void) archiver.readArchive(this->buffer, len);
			*(this->endOfString = this->buffer + len) =
				ModWideCharTrait::null();
		}
	}
}

//	FUNCTION public
//	ModWideString::operator = -- = 演算子
//
//	NOTES
//		自分自身へワイド文字列を代入する
//
//	ARGUMENTS
//		ModWideString&		src
//			自分自身へ代入するワイド文字列
//
//	RETURN
//		ワイド文字列が代入された自分自身
//
//	EXCEPTIONS

ModWideString& 
ModWideString::operator =(const ModWideString& src)
{
	if (this != &src) {

		// 複写する文字数ぶんの領域を確保し、
		// 空文字列が格納されていることにする

		ModSize	len = src.getLength();
		this->reallocate(len);
		this->clear();

		if (len) {

			// 与えられた文字列を末尾の ModWideCharTrait::null を含めて複写する

			ModOsDriver::Memory::copy(this->buffer, src.buffer,
									  (len + 1) * sizeof(ModWideChar));
			this->endOfString = this->buffer + len;
			_size = src.getSize();
		}
	}
	return *this;
}

//	FUNCTION public
//	ModWideString::append -- 文字を付加する
//
//	NOTES
//		与えられた文字を自分自身の格納ワイド文字列の末尾に追加する
//
//	ARGUMENTS
//		char				src
//			自分自身の格納ワイド文字列の末尾に付加する ASCII 文字
//		ModWideChar			src
//			自分自身の格納ワイド文字列の末尾に付加するワイド文字
//
//	RETURN
//		文字を付加後の自分自身
//
//	EXCEPTIONS

ModWideString& 
ModWideString::append(char src)
{
	if (src != ModCharTrait::null()) {
		char	tmp[] = { src, ModCharTrait::null() };
		return this->append(
			ModWideCharTrait::makeWideChar(tmp, ModKanjiCode::euc));
	}
	return *this;
}

ModWideString& 
ModWideString::append(ModWideChar src)
{
	if (src != ModWideCharTrait::null()) {
		this->reallocate(this->getLength() + 1);

		if (this->charBuffer) {
			ModStandardManager::free(this->charBuffer,
									 (this->getSize() + 1) * sizeof(char));
			this->charBuffer = 0;
		}

		*(this->endOfString) = src;
		*(++this->endOfString) = ModWideCharTrait::null();
		_size += ModWideCharTrait::byteSize(src);
	}
	return *this;
}

//	FUNCTION public
//	ModWideString::append -- 部分文字列を付加する
//
//	NOTES
//		与えられたワイド文字列の先頭から
//		ある長さの部分ワイド文字列を取り出して、
//		自分自身の格納ワイド文字列の末尾に付加する
//
//	ARGUMENTS
//		const ModWideString&	src
//			自分自身の格納ワイド文字列の末尾に付加するワイド文字列
//		ModSize				len
//			0 より大きい値
//				指定されたワイド文字列の先頭から
//				なんワイド文字目までを付加するか
//				ただし、指定されたワイド文字列のワイド文字数以上は付加できない
//			0 または指定されないとき
//				指定されたワイド文字列全体を付加する
//
//	RETURN
//		部分ワイド文字列を付加後の自分自身
//
//	EXCEPTIONS

ModWideString& 
ModWideString::append(const ModWideString& src, ModSize len)
{
	ModSize	l = src.getLength();
	ModSize	n = src.getSize();

	if (len == 0 || len >= l)
		len = l;
	else if (len <= l - len) {
		n = 0;
		for (ModWideChar* p = src.buffer; p < src.buffer + len; ++p)
			n += ModWideCharTrait::byteSize(*p);
	} else
		for (ModWideChar* p = src.buffer + len;
			 *p != ModWideCharTrait::null(); ++p)
			n -= ModWideCharTrait::byteSize(*p);

	if (len) {

		// 現在のワイド文字列に部分ワイド文字列を加えた
		// 長さぶんの領域を確保する

		this->reallocate(this->getLength() + len);

		if (this->charBuffer) {
			ModStandardManager::free(this->charBuffer,
									 (this->getSize() + 1) * sizeof(char));
			this->charBuffer = 0;
		}

		// 与えられたワイド文字列から計算した範囲のワイド文字を複写し、
		// 末尾に ModWideCharTrait::null を付加する

		ModOsDriver::Memory::copy(this->endOfString,
								  src.buffer, len * sizeof(ModWideChar));
		*(this->endOfString += len) = ModWideCharTrait::null();
		_size += n;
	}
	return *this;
}

//
// FUNCTION 
// ModWideString::append -- char* を文字列に追加する
//
// NOTES
// ModWideString に char* で表した文字列を加える。
//
// ARGUMENTS
// const char* string
//		文字列
// const ModSize appendLength
//		追加する文字数。0 なら文字列全体。
// ModKanjiCode::KanjiCodeType kanjiCode
//		string に使われている漢字コード。
//		無指定時は EUC が使用される。
//
// RETURN
//		自分自身への参照を返す。	
//
// EXCEPTIONS
//		なし
// その他
//		ModStandardManager::allocate、ModStandardManager::free の例外参照
//
ModWideString& 
ModWideString::append(const char* string, const ModSize appendLength,
					  ModKanjiCode::KanjiCodeType kanjiCode)
{
	//
	// string が 0 ならそのままでいいので
	// string が 0 でないときのみ考慮すればよい。
	//
	if (string) {
		// reallocate をなるべく減らすため、必要なメモリを確保する

		ModSize usedLength = (appendLength) ?
			appendLength : ModCharTrait::length(string);
		this->reallocate(this->getLength() + usedLength);

		//
		// string 上を走査しながら１文字ずつ追加する。
		// あらかじめ必要なメモリ容量の下限を確保してあるので
		// reallocate による遅延は抑えられるはず。
		//
		ModWideChar wideCharacter[2];
		const char* cp = string;
		for (ModSize i = 0;
			 *cp != ModCharTrait::null() && i < usedLength; i++) {
			cp = ModKanjiCode::toWide(wideCharacter, cp, 2, kanjiCode);
			this->append(wideCharacter[0]);
		}
	}
	return *this;
}

//
// FUNCTION 
// ModWideString::append -- ワイド文字配列を文字列に追加する
//
// NOTES
// ModWideString に ModWideChar配列を追加する
//
// ARGUMENTS
// const ModWideChar* string
//		ワイド文字の配列
//
// RETURN
//		自分自身への参照を返す。	
//
// EXCEPTIONS
//		なし
//
// その他
//		ModStandardManager::allocate、ModStandardManager::free の例外参照
//
ModWideString& 
ModWideString::append(const ModWideChar* string, const ModSize appendLength)
{
	//
	// string が 0 でないときのみ考慮すればよい。
	//
	if (string) {
		ModSize usedLength = (appendLength) ?
			appendLength : ModWideCharTrait::length(string);
		this->reallocate(this->getLength() + usedLength);

		// size を計算する必要があるので一文字ずつ append を呼ぶ
		ModSize i = 0;
		const ModWideChar* wp;			// string上を走査するポインター
		for (wp = string; *wp != ModWideCharTrait::null() && i < usedLength;
			 wp++, i++) {
			this->append(*wp);
		}
	}
	return *this;
}

//
// FUNCTION
// ModWideString::operator+ -- + オペレータ
//
// NOTES
// この関数は 2 つの ModWideString の表す文字列をつなげたものを
// 表す ModWideString を得るために用いる。
//
// ARGUMENTS
// const ModWideString& string
//		つなげる対象の文字列への参照
//
// RETURN
// 2 つの文字列をつなげたものを表す ModWideString を返す。
//
// EXCEPTIONS
// なし
//

ModWideString
ModWideString::operator +(const ModWideString& string)
{
	return ModWideString(*this) += string;
}

//
// FUNCTION
// ModWideString::operator+ -- char* との + オペレータ
//
// NOTES
// この関数は ModWideString に char* をつなげた文字列を表す
// ModWideString を得るために用いる。
//
// ARGUMENTS
// const char* string
//		つなげる対象のchar配列へのポインタ
//
// RETURN
// 2 つの文字列をつなげたものを表す ModWideString を返す。
//
// EXCEPTIONS
// なし
//

ModWideString
ModWideString::operator +(const char* string)
{
	return ModWideString(*this) += string;
}

//
// FUNCTION
// ModWideString::operator+ --  + オペレータ
//
// NOTES
// この関数は ModWideString に ModWideChar* をつなげた文字列を表す
// ModWideString を得るために用いる。
//
// ARGUMENTS
// const ModWideChar* string
//		つなげる対象のModWideChar配列へのポインタ
//
// RETURN
// 2 つの文字列をつなげたものを表す ModWideString を返す。
//
// EXCEPTIONS
// なし
//

ModWideString
ModWideString::operator +(const ModWideChar* string)
{
	return ModWideString(*this) += string;
}

//
// FUNCTION
// ModWideString::operator+ -- char* を第一オペランドに持つ + オペレータ
//
// NOTES
// この関数は char* に ModWideString をつなげた文字列を表す
// ModWideString を得るために用いる。
//
// ARGUMENTS
// const char* string1
//		つなげる元のchar配列へのポインタ
// const ModWideString& string2
//		string1 につなげる文字列への参照
//
// RETURN
// 2 つの文字列をつなげたものを表す ModWideString を返す。
//
// EXCEPTIONS
// なし
//

ModWideString
operator +(const char* string1, const ModWideString& string2)
{
	return ModWideString(string1) += string2;
}

//	FUNCTION public
//	ModWideString::copy -- 部分ワイド文字列の取得
//
//	NOTES
//		自分自身が格納しているワイド文字列のある一部分を取り出す
//
//	ARGUMENTS
//		ModSize				start
//			指定されたとき
//				自分自身の格納文字列の先頭から何文字目から
//				部分ワイド文字列を取り出すか
//				先頭のワイド文字は、先頭から 0 文字目になる
//			指定されないとき
//				0 が指定されたものとみなす
//		ModSize				len
//			0 より大きい値
//				取り出す部分ワイド文字列の文字数
//			0 または指定されないとき
//				自分自身の格納文字列の末尾まで取り出す
//
//	RETURN
//		取り出した部分ワイド文字列
//
//	EXCEPTIONS

ModWideString
ModWideString::copy(ModSize start, ModSize len) const
{
	ModSize	l = this->getLength();
	if (len == 0 || l < start + len)
		len = (start < l) ? l - start : 0;

	if (l - len >= len)

		// 残りの文字数は複写される文字数以上である

		return ModWideString(this->buffer + start, len);

	ModWideString	dst;
	dst.reallocate(len);
	; ModAssert(dst.buffer != 0);

	ModOsDriver::Memory::copy(dst.buffer, this->buffer + start,
							  len * sizeof(ModWideChar));
	*(dst.endOfString += len) = ModWideCharTrait::null();

	dst._size = this->getSize();
	ModWideChar*	p = this->buffer;
	for (; p != this->buffer + start; ++p)
		dst._size -= ModWideCharTrait::byteSize(*p);
	for (p += len; *p != ModWideCharTrait::null(); ++p)
		dst._size -= ModWideCharTrait::byteSize(*p);

	return dst;
}

//	FUNCTION public
//	ModWideString::getString -- C 文字列を得る 
//
//	NOTES
//		文字列を表す C 文字列を得る
//
//	ARGUMENTS
//		ModKanjiCode::KanjiCodeType	code
//			得る C 文字列の漢字コード
//
//	RETURN
//		得られた C 文字列を格納する領域の先頭アドレス
//
//	EXCEPTIONS

const char*
ModWideString::getString(ModKanjiCode::KanjiCodeType code)
{
	if (this->charBuffer)

		// すでに C 文字列へ変換済である

		return this->charBuffer;

	char*	p = this->charBuffer = (char*)
		ModStandardManager::allocate((this->getSize() + 1) * sizeof(char));
	; ModAssert(p != 0);

	for (ModWideChar* q = this->buffer; q != this->endOfString; ++q) {
		ModSize	n = ModWideCharTrait::byteSize(*q);
		if ((ModSize) (p - this->charBuffer) * sizeof(char) + n >
			this->getSize())

			// サイズが思ったよりも大きい

			break;

		n = ModWideCharTrait::convertToString(p, *q, code);
		if (n == -1)

			// おかしなワイド文字があって変換できなかった

			break;

		p += n;
	}
	*p = ModCharTrait::null();

	return this->charBuffer;
}

//
// FUNCTION
// ModWideString::compare -- char*と比較する
//
// NOTES
// この関数は ModWideString と char* の比較をするのに用いる
//
// ARGUMENTS
// const char* string
//		比較対象の文字列
// const MaxSize maxLength
//		比較する文字数。この引数を 0 とすると
//		文字列全体を比較する。
// ModKanjiCode::KanjiCodeType kanjiCode
//		string に使われている漢字コード。
//		無指定時は EUC が使用される。
//
// RETURN
// maxLength の範囲で、2 つの文字列がすべて一致する場合は 0 を返し、
// 一致していなかったら初めて一致していない場所の EUC コードが
// string のものの方が小さい場合は 1 を返し、大きい場合は -1 を返す。
//
// EXCEPTIONS
// なし
// 

int
ModWideString::compare(const char* string, ModSize maxLength,
					   ModKanjiCode::KanjiCodeType kanjiCode) const
{
	const ModWideChar*	wp = this->buffer;
	const char*			cp = string;

	if (wp == 0)
		return (cp) ? -1 : 0;
	if (cp == 0)
		return 1;

	//
	// 両方の配列要素を１つずつ比較する。
	// 違う部分が見つかったら break する。判定はループの外で行なう。
	//

	ModWideChar cpWideChar[] = { ModWideCharTrait::null(), 0 };
	const ModWideChar*	wpEnd =
		(maxLength == 0 || this->endOfString < this->buffer + maxLength) ?
		this->endOfString : (this->buffer + maxLength);
	const char* nextCp;

	for (; wp < wpEnd && *cp != ModCharTrait::null(); wp++, cp = nextCp) {

		// cp の指すコードを ModWideChar に変換する
		nextCp = ModKanjiCode::toWide(cpWideChar, cp, 2, kanjiCode);

		if (*wp != *cpWideChar)
			// 違う部分が見つかったので break する。
			break;
	}

	// まったく同じだった -> 0
	// string が this の先頭に部分一致していた -> 1
	// this が string の先頭に部分一致していた -> -1
	//
	//【注意】	大小は EUC コードの比較で判定する

	return (wp == wpEnd) ? 0 :
		(*cp == ModCharTrait::null() ||
		 ModWideCharTrait::convertToLong(*wp) >
		 ModWideCharTrait::convertToLong(*cpWideChar)) ? 1 : -1;
}

//
// FUNCTION
// ModWideString::compare -- char*と比較する(ケースフラグつき)
//
// NOTES
// この関数は ModWideString と char* の比較をするのに用いる
//
// ARGUMENTS
// const char* string
//		比較対象の文字列
// const ModBoolean caseFlag
//		この引数が ModFalse のときは大文字と小文字を同一視する。
// const ModSize maxLength
//		比較する文字数。この引数を 0 とすると
//		文字列全体を比較する。
// ModKanjiCode::KanjiCodeType kanjiCode
//		string に使われている漢字コード。
//		無指定時は EUC が使用される。
//
// RETURN
// maxLength の範囲で、2 つの文字列がすべて一致する場合は 0 を返し、
// 一致していなかったら初めて一致していない場所の EUC コードが
// string のものの方が小さい場合は 1 を返し、大きい場合は -1 を返す。
// caseFlag が ModFalse のときは上記を大文字と小文字を同一視して行なう。
//
// EXCEPTIONS
// なし
//

int
ModWideString::compare(const char* string, ModBoolean caseFlag,
					   ModSize maxLength,
					   ModKanjiCode::KanjiCodeType kanjiCode) const
{
	if (caseFlag == ModTrue) {
		// ケースセンシティブなら通常の compare と同じ
		return this->compare(string, maxLength, kanjiCode);
	}

	const ModWideChar*	wp = this->buffer;
	const char*			cp = string;

	if (wp == 0)
		return (cp) ? -1 : 0;
	if (cp == 0)
		return 1;

	//
	// 両方の配列要素を１つずつ比較する。
	// 違う部分が見つかったら break する。判定はループの外で行なう。
	//

	ModWideChar cpWideChar[] = { ModWideCharTrait::null(), 0 };
	const ModWideChar*	wpEnd =
		(maxLength == 0 || this->endOfString < this->buffer + maxLength) ?
		this->endOfString :	(this->buffer + maxLength);
	const char* nextCp;

	for (; wp < wpEnd && *cp != ModCharTrait::null(); wp++, cp = nextCp) {

		// cp の指すコードを ModWideChar に変換する
		nextCp = ModKanjiCode::toWide(cpWideChar, cp, 2, kanjiCode);

		if (ModWideCharTrait::toUpper(*wp) !=
			ModWideCharTrait::toUpper(*cpWideChar))
			// 違う部分が見つかったので break する。
			break;
	}

	// まったく同じだった -> 0
	// string が this の先頭に部分一致していた -> 1
	// this が string の先頭に部分一致していた -> -1
	//
	//【注意】	大小は EUC コードの比較で判定する

	return (wp == wpEnd) ? 0 :
		(*cp == ModCharTrait::null() ||
		 ModWideCharTrait::convertToLong(ModWideCharTrait::toUpper(*wp)) >
		 ModWideCharTrait::convertToLong(
			 ModWideCharTrait::toUpper(*cpWideChar))) ? 1 : -1;
}

//
// FUNCTION
// ModWideString::compare -- ModWideChar*と比較する
//
// NOTES
// この関数は ModWideString と ModWideChar の配列を比較するのに用いる。
//
// ARGUMENTS
// const ModWideChar* string
//		比較対象の文字列
// const ModSize maxLength
//		比較する文字数。この引数を 0 とすると
//		文字列全体を比較する。
//
// RETURN
// maxLength の範囲で、2 つの文字列がすべて一致する場合は 0 を返し、
// 一致していなかったら初めて一致していない場所の EUC コードが
// string のものの方が小さい場合は 1 を返し、大きい場合は -1 を返す。
//
// EXCEPTIONS
// なし
//

int
ModWideString::compare(const ModWideChar* s, ModSize len) const
{
	const ModWideChar*	wp = this->buffer;
	const ModWideChar*	sp = s;

	if (wp == 0)
		return (sp) ? -1 : 0;
	if (sp == 0)
		return 1;

	//
	// 両方の配列要素を１つずつ比較し、
	// 異なるところが見つかるまでループする。
	//

	const ModWideChar*	wpEnd =
		(len == 0 || this->endOfString < this->buffer + len) ?
		this->endOfString : (this->buffer + len);
	
	for (; wp < wpEnd && *sp != ModWideCharTrait::null() && *wp == *sp;
		 wp++, sp++);

	// まったく同じだった -> 0
	// this が s の先頭に部分一致していた -> -1
	// s が this の先頭に部分一致していた -> 1
	//
	//【注意】	大小は EUC コードの比較で判定する

	return (wp == wpEnd &&
			(*sp == ModWideCharTrait::null() || (ModSize) (sp - s) == len)) ?
		0 :
		(ModWideCharTrait::convertToLong(*wp) <
		 ModWideCharTrait::convertToLong(*sp)) ? -1 : 1;
}

//
// FUNCTION
// ModWideString::compare -- ModWideChar*と比較する(ケースフラグつき)
//
// NOTES
// この関数は ModWideString と ModWideChar の配列を比較するのに用いる。
//
// ARGUMENTS
// const ModWideChar* string
//		比較対象の文字列
// const ModBoolean caseFlag
//		この引数が ModFalse のときは大文字と小文字を同一視する。
// const ModSize maxLength
//		比較する文字数。この引数を 0 とすると
//		文字列全体を比較する。
//
// RETURN
// maxLength の範囲で、2 つの文字列がすべて一致する場合は 0 を返し、
// 一致していなかったら初めて一致していない場所の EUC コードが
// string のものの方が小さい場合は 1 を返し、大きい場合は -1 を返す。
// caseFlag が ModFalse のときは上記を大文字と小文字を同一視して行なう。
//
// EXCEPTIONS
// なし
//

int
ModWideString::compare(const ModWideChar* s,
					   ModBoolean caseSensitive, ModSize len) const
{
	// ケースセンシティブなら普通の関数を呼ぶ
	if (caseSensitive == ModTrue) {
		return this->compare(s, len);
	}

	const ModWideChar*	wp = this->buffer;
	const ModWideChar*	sp = s;

	if (wp == 0)
		return (sp) ? -1 : 0;
	if (sp == 0)
		return 1;

	//
	// 両方の配列要素を１つずつ比較する。
	// 違う部分が見つかったら break する。判定はループの外で行なう。
	//

	const ModWideChar*	wpEnd =
		(len == 0 || this->endOfString < this->buffer + len) ?
		this->endOfString : (this->buffer + len);

	for (; wp < wpEnd && *sp != ModWideCharTrait::null() &&
		   (ModWideCharTrait::toUpper(*wp) == ModWideCharTrait::toUpper(*sp));
		 wp++, sp++);

	// まったく同じだった -> 0
	// this が s の先頭に部分一致していた -> -1
	// s が this の先頭に部分一致していた -> 1
	//
	//【注意】	大小は EUC コードの比較で判定する

	return (wp == wpEnd &&
			(*sp == ModWideCharTrait::null() || (ModSize) (sp - s) == len)) ?
		0 :
		(ModWideCharTrait::convertToLong(ModWideCharTrait::toUpper(*wp)) <
		 ModWideCharTrait::convertToLong(ModWideCharTrait::toUpper(*sp))) ?
		-1 : 1;
}

//
// FUNCTION 
// ModWideString::search -- char*の文字列を検索する(ケースフラグつき)
//
// NOTES
// この関数は ModWideString から char* で与えられる文字列を探すのに用いられる。
//
// ARGUMENTS
// const char* string
//		検索する文字列
// const ModBoolean caseFlag
//		ケースセンシティブか否かを指定するフラグ。ModTrue のときセンシティブ。
// ModKanjiCode::KanjiCodeType kanjiCode
//		string に使われている漢字コード。
//		無指定時は EUC が使用される。
//
// RETURN
// 一致する文字列が見つかった場合、その位置を示す ModWideChar* 型を返す。
// 見つからなかった場合、0 を返す。
//
// EXCEPTIONS
// その他
// なし
//

ModWideChar*
ModWideString::search(const char* string, ModBoolean caseFlag,
					  ModKanjiCode::KanjiCodeType kanjiCode)
{
	ModWideChar*	wp = this->buffer;
	ModWideChar*	wpEnd = this->endOfString;
	const char*		cp;
	const char*		nextCp;
	ModWideChar		topWideChar = (caseFlag == ModTrue) ?
		ModWideCharTrait::makeWideChar(string, kanjiCode) :
		ModWideCharTrait::toUpper(
			ModWideCharTrait::makeWideChar(string, kanjiCode));
	ModWideChar		cpWideChar[2];
	ModWideChar*	candid;
	ModWideChar*	nextCandid;

	while (wp < wpEnd) {
		nextCandid = 0;
		// topWideChar に一致する位置まで進む
		for (;
			 wp < wpEnd
				 && (*wp != topWideChar
					 && (caseFlag == ModTrue
						 || (ModWideCharTrait::toUpper(*wp) != topWideChar)));
			 wp++);
		if (wp == wpEnd) {
			// 一致する部分はない
			return 0;
		}

		// 返り値候補
		candid = wp;

		// 残りを調べる
		for (wp++, cp = string + ModWideCharTrait::byteSize(topWideChar);
			 wp < wpEnd && *cp != ModCharTrait::null();
			 wp++, cp = nextCp) {
			// cp の指す文字をワイド文字に変換する
			nextCp = ModKanjiCode::toWide(cpWideChar, cp, 2, kanjiCode);
			// 違うところが見つかったらループを抜ける
			if (*wp != cpWideChar[0]
				&& (caseFlag == ModTrue
					|| (ModWideCharTrait::toUpper(*wp)
						!= ModWideCharTrait::toUpper(cpWideChar[0])))) {
				break;
			}
			// ループのついでに次の調査開始候補を探しておく
			if (nextCandid == 0
				&& (*wp == topWideChar
					|| (caseFlag == ModFalse
						&& (ModWideCharTrait::toUpper(*wp)
							== ModWideCharTrait::toUpper(topWideChar))))) {
				nextCandid = wp;
			}
		}

		if (*cp == ModCharTrait::null()) {
			// すべて一致した
			return candid;
		}

		// ループの途中に string の先頭と一致するものがあったらそこまで戻る
		if (nextCandid != 0) {
			wp = nextCandid;
		}
	}
	return 0;
}

//
// FUNCTION 
// ModWideString::search -- ModWideChar*の文字列を検索する(ケースフラグつき)
//
// NOTES
// この関数は ModWideString から ModWideChar 配列を探すのに用いられる。
//
// ARGUMENTS
// const ModWideChar* string
//		検索する文字列
// const ModBoolean caseFlag
//		ケースセンシティブか否かを指定するフラグ。ModTrue のときセンシティブ。
//
// RETURN
// 一致する文字列が見つかった場合、その位置を示す ModWideChar* 型を返す。
// 見つからなかった場合、0 を返す。
//
// EXCEPTIONS
// その他
// なし
//

ModWideChar*
ModWideString::search(const ModWideChar* string, ModBoolean caseFlag)
{
	ModWideChar*		wp = this->buffer;
	ModWideChar*		wpEnd = this->endOfString;
	const ModWideChar*	sp;
	ModWideChar*		candid;
	ModWideChar*		nextCandid;

	while (wp < wpEnd) {
		nextCandid = 0;
		// string に一致する位置まで進む
		for (;
			 wp < wpEnd
				 && (*wp != *string
					 && (caseFlag == ModTrue
						 || (ModWideCharTrait::toUpper(*wp)
							 != ModWideCharTrait::toUpper(*string))));
			 wp++);
		if (wp == wpEnd) {
			// 一致する部分はない
			return 0;
		}

		// 返り値候補
		candid = wp;

		// 残りを調べる
		for (wp++, sp = string + 1;
			 wp < wpEnd && *sp != ModWideCharTrait::null()
				 && (*wp == *sp
					 || (caseFlag == ModFalse
						 && (ModWideCharTrait::toUpper(*wp)
							 == ModWideCharTrait::toUpper(*sp))));
			 wp++, sp++) {
			// ループのついでに次の調査開始候補を探しておく
			if (nextCandid == 0
				&& (*wp == *string
					|| (caseFlag == ModFalse
						&& (ModWideCharTrait::toUpper(*wp)
							== ModWideCharTrait::toUpper(*string))))) {
				nextCandid = wp;
			}
		}

		if (*sp == ModWideCharTrait::null()) {
			// すべて一致した
			return candid;
		}

		// ループの途中に string の先頭と一致するものがあったらそこまで戻る
		if (nextCandid != 0) {
			wp = nextCandid;
		}
	}
	return 0;
}

//	FUNCTION private
//	ModWideString::allocateCopy -- 指定された C 文字列を複写する
//
//	NOTES
//		指定された C 文字列から指定された数の文字を取り出し、格納文字列とする
//		そのとき必要であれば、バッファ領域を拡張する
//
//	ARGUMENTS
//		char*				s
//			複写する C 文字列を格納する領域の先頭アドレス
//			0 が指定されると、空文字列が複写される
//		ModSize				len
//			0 より大きい値
//				指定された C 文字列の先頭からワイド文字へ変換しながら
//				複写するワイド文字数
//				ただし、指定された C 文字列をすべてワイド文字へ
//				変換したときのワイド文字数以上は複写されない
//			0 または指定されないとき
//				指定された C 文字列をすべて複写する
//		ModKanjiCode::KanjiCodeType	code
//			指定された C 文字列の日本語コード
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ModWideString::allocateCopy(const char* s, ModSize len,
							ModKanjiCode::KanjiCodeType code)
{
	if (s == 0)

		// コピーするワイド文字配列を格納する領域のアドレスが 0 なので、
		// 空文字列が格納されていることにする

		this->clear();
	else {

		// 与えられた C 文字列をワイド文字列に変換したときの
		// ワイド文字数とバイト数を計算する

		ModWideChar tmp[2];

		ModSize		l = 0;
		ModSize		n = 0;

		const char* p = s;
		while (*p != ModCharTrait::null()) {

			// ワイド文字を 1 文字取り出す

			p = ModKanjiCode::toWide(tmp, p, 2, code);

			if (*tmp == ModWideCharTrait::null() && *p != ModCharTrait::null())

				// 変換に失敗したので、これまでとする

				break;

			++l, n += ModWideCharTrait::byteSize(*tmp);
			if (l == len)

				// 指定された文字数に達した

				break;
		}

		// 複写する文字数ぶんの領域を確保し、
		// 空文字列が確保されていることとする

		this->reallocate(l);
		this->clear();

		while (l--)
			s = ModKanjiCode::toWide(this->endOfString++, s, 2, code);
		*this->endOfString = ModWideCharTrait::null();
		_size = n;
	}
}

//	FUNCTION private
//	ModWideString::allocateCopy -- 指定されたワイド文字配列を複写する
//
//	NOTES
//		指定されたワイド文字配列から指定された数の文字を取り出し、
//		格納文字列とする
//		そのとき必要であれば、バッファ領域を拡張する
//
//	ARGUMENTS
//		ModWideChar*		s
//			複写するワイド文字配列を格納する領域の先頭アドレス
//			0 が指定されると、空文字列が複写される
//		ModSize				len
//			0 より大きい値
//				指定されたワイド文字配列の先頭から複写する文字数
//				ただし、指定されたワイド文字配列の文字数以上は複写されない
//			0 または指定されないとき
//				指定されたワイド文字配列をすべて複写する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ModWideString::allocateCopy(const ModWideChar* s, ModSize len)
{
	if (s == 0)

		// コピーするワイド文字配列を格納する領域のアドレスが 0 なので、
		// 空文字列が格納されていることにする

		this->clear();
	else {

		// 与えられたワイド文字配列の文字数とバイト数を計算する

		ModSize		l = 0;
		ModSize		n = 0;

		for (const ModWideChar* p = s; *p != ModWideCharTrait::null(); ++p) {
			++l, n += ModWideCharTrait::byteSize(*p);
			if (l == len)

				// 指定された文字数に達した

				break;
		}

		// 複写する文字数ぶんの領域を確保し、
		// 空文字列が確保されていることにする

		this->reallocate(l);
		this->clear();

		if (l) {

			// 与えられたワイド文字配列を
			// 末尾の ModWideCharTrait::null を含めて複写する

			ModOsDriver::Memory::copy(
				this->buffer, s, l * sizeof(ModWideChar));
			*(this->endOfString = this->buffer + l) = ModWideCharTrait::null();
			_size = n;
		}
	}
}

//	FUNCTION private
//	ModWideString::allocate -- バッファ領域の確保
//
//	NOTES
//		既存のバッファ領域がないときにこの関数を呼び出すと、
//		ModWideString::reallocate より高速に
//		指定された文字数の文字列を格納できるバッファが確保される
//
//	ARGUMENTS
//		ModSize				len
//			バッファ領域に格納したい文字列の文字数
//			ただし、文字列末尾の ModWideCharTrait::null は除く
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ModWideString::allocate(ModSize len)
{
	; ModAssert(this->buffer == 0);
	; ModAssert(this->endOfBuffer == 0);
	; ModAssert(this->endOfString == 0);

	// 指定された文字数は文字列末尾の
	// ModWideCharTrait::null のぶんを含まないので、
	// そのぶんを加えて、必要なバイト数を求める

	ModSize	size = (len + 1) * sizeof(ModWideChar);

	// 確保するサイズを _allocateStep 単位に切り上げる

	; ModAssert(_allocateStep != 0);
	size = _allocateStep * (size / _allocateStep +
							((size % _allocateStep) ? 1 : 0));

	// 計算したサイズの領域を確保する

	this->buffer = (ModWideChar*) ModStandardManager::allocate(size);
	; ModAssert(this->buffer != 0);
	this->endOfBuffer = this->buffer + size / sizeof(ModWideChar);
	*(this->endOfString = this->buffer) = ModWideCharTrait::null();
}

//	FUNCTION private
//	ModWideString::reallocate -- バッファ領域の拡張
//
//	NOTES
//		この関数を呼び出すと、既存のバッファ領域の中身を持つ
//		指定された文字数の文字列を格納できるバッファが確保される
//		既存のバッファ領域がなければ、
//		指定された文字数の文字列が格納可能なバッファが確保される
//
//	ARGUMENTS
//		ModSize				len
//			バッファ領域に格納したい文字列の文字数
//			ただし、文字列末尾の ModWideCharTrait::null は除く
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ModWideString::reallocate(ModSize len)
{
	ModSize	newSize = (len + 1) * sizeof(ModWideChar);
	ModSize	oldSize = this->getBufferSize();

	if (oldSize < newSize) {

		// 既存のバッファ領域は、指定されたサイズより小さい

		// 確保するサイズを _allocateStep 単位に切り上げる

		; ModAssert(_allocateStep != 0);
		newSize = _allocateStep * (newSize / _allocateStep +
								   ((newSize % _allocateStep) ? 1 : 0));

		// 計算したサイズの領域を確保する

		ModWideChar*	buf = (ModWideChar*)
			ModStandardManager::allocate(newSize);
		; ModAssert(buf != 0);

		ModSize	current = this->getLength();
		if (current) {

			// 現在のバッファ中の末尾の
			// ModWideCharTrait::null を含む文字列を
			// 新しいバッファへ複写する

			; ModAssert(this->buffer != 0);
			ModOsDriver::Memory::copy(buf, this->buffer,
									  (current + 1) * sizeof(ModWideChar));
		} else
			*buf = ModWideCharTrait::null();

		if (this->buffer)


			// 現在のバッファ領域を破棄する

			ModStandardManager::free(this->buffer, oldSize);

		// 新しいバッファを設定する

		this->buffer = buf;
		this->endOfBuffer = this->buffer + newSize / sizeof(ModWideChar);
		this->endOfString = this->buffer + current;
	}
}

//
// Copyright (c) 1997, 1999, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
