// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModUnicodeString.cpp -- ModUnicodeString のメンバ定義
// 
// Copyright (c) 1999, 2000, 2001, 2003, 2004, 2007, 2023 Ricoh Company, Ltd.
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
#include "ModUnicodeString.h"
#include "ModCharTrait.h"
#include "ModUnicodeCharTrait.h"
#include "ModKanjiCode.h"

//	CONST private
//	ModUnicodeString::s_allocateStepDefault -- バッファ領域の拡張単位
//
//	NOTES
//		Unicode 文字列を格納するためのバッファ領域を
//		拡張するときの単位のデフォルト値(B 単位)

const ModSize
ModUnicodeString::s_allocateStepDefault = 16 * sizeof(ModUnicodeChar);

//	CONST private
//	ModUnicodeString::s_nul -- 空文字列を意味する領域
//
//	NOTES
//		Unicode 文字列を格納するためのバッファ領域を
//		拡張するときの単位のデフォルト値(B 単位)

const ModUnicodeChar
ModUnicodeString::s_nul = ModUnicodeCharTrait::null();

//	CONST private
//	ModUnicodeString::CharString::s_biggestNulSize -- 終端文字の最大サイズ
//
//	NOTES
// あらゆる文字コードの終端文字の中でもバイト数が最大のもの(単位:バイト)

const ModSize
ModUnicodeString::CharString::s_biggestNulSize = sizeof(ModUnicodeChar);

//
// FUNCTION 
// ModUnicodeString::~ModUnicodeString -- デストラクタ
//
// NOTES
// ModUnicodeStringのデストラクタ
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
// その他
// ModStandardManager::free の例外参照
//
ModUnicodeString::~ModUnicodeString()
{
	// d_charString のデストラクタから呼ばれるので、わざわざ呼び出す必要なし
	// d_charString.free();		

	if (d_buffer) {
		ModStandardManager::free(d_buffer, this->getBufferSize());
		d_buffer = 0;
	}
}

//	FUNCTION public
//	ModUnicodeString::operator = -- = 演算子
//
//	NOTES
//		自分自身へ Unicode 文字列を代入する
//
//	ARGUMENTS
//		ModUnicodeString&		src
//			自分自身へ代入する Unicode 文字列
//
//	RETURN
//		Unicode 文字列が代入された自分自身
//
//	EXCEPTIONS

ModUnicodeString& 
ModUnicodeString::operator =(const ModUnicodeString& src)
{
	if (this != &src) {

		// 複写する文字数ぶんの領域を確保し、
		// 空文字列が格納されていることにする
		this->clear();
		ModSize	len = src.getLength();
		if (len > 0) {
			if (d_buffer) {
				this->reallocate(len);
			} else {
				this->allocate(len);
			}			

			// 与えられた文字列を末尾の ModUnicodeCharTrait::null を含めて複写
			ModOsDriver::Memory::copy(d_buffer, src.d_buffer,
									  (len + 1) * sizeof(ModUnicodeChar));
			d_endOfString = d_buffer + len;

			// マルチバイト文字列のコード種類も引き継ぐ
			d_charString.setCode(src.d_charString.getCode());
		}
	}
	return *this;
}

//	FUNCTION public
//	ModUnicodeString::append -- 文字を付加する
//
//	NOTES
//		与えられた文字を自分自身の格納 Unicode 文字列の末尾に追加する
//
//	ARGUMENTS
//		char				character
//			自分自身の格納文字列の末尾に付加する ASCII 文字(英数字)
//		ModUnicodeChar			src
//			自分自身の格納文字列の末尾に付加する Unicode 文字
//
//	RETURN
//		文字を付加後の自分自身
//
//	EXCEPTIONS

ModUnicodeString& 
ModUnicodeString::append(const char				character)
{
	if (character != ModCharTrait::null()) {
		return append(ModUnicodeChar(character));
	}
	return *this;
}

ModUnicodeString& 
ModUnicodeString::append(const ModUnicodeChar character)
{
	if (character != ModUnicodeCharTrait::null()) {

		reallocate(this->getLength() + 1);

		// Unicode のバッファが更新される時は、内容の矛盾が起きないように
		// マルチバイト用のバッファを破棄
		d_charString.free();

		// 文字を追加
		*d_endOfString = character;
		*(++d_endOfString) = ModUnicodeCharTrait::null();
	}
	return *this;
}

//	FUNCTION public
//	ModUnicodeString::append -- 部分文字列を付加する
//
//	NOTES
//		与えられた Unicode 文字列の先頭から
//		ある長さの部分 Unicode 文字列を取り出して、
//		自分自身の文字列バッファの末尾に付加する
//
//	ARGUMENTS
//		ModUnicodeString&	src
//			自分自身の文字列の末尾に付加する Unicode 文字列
//		ModSize				appendLength
//			0 より大きい値
//				指定された文字列の先頭から何文字目までを付加するか
//				ただし、nul 文字に遭遇したら処理を終了する
//			0 または指定されないとき
//				指定された文字列全体を nul 文字が現れるまで付加する
//
//	RETURN
//		部分 Unicode 文字列を付加後の自分自身
//
//	EXCEPTIONS

ModUnicodeString& 
ModUnicodeString::append(const ModUnicodeString&	src,
						 const ModSize				appendLength /* = 0 */)
{
	return append(src.d_buffer, appendLength);
}

//
// FUNCTION 
// ModUnicodeString::append -- Unicode 文字配列を文字列に追加する
//
// NOTES
// ModUnicodeString に ModUnicodeChar配列を追加する。
// 追加する文字列が空文字列の場合は何もしない。
//
// ARGUMENTS
// ModUnicodeChar* string
//		Unicode 文字の配列
// ModSize		appendLength
//		0 より大きい値
//			指定された文字列の先頭から何文字目までを付加するか
//			ただし、nul 文字に遭遇したら処理を終了する
//		0 または指定されないとき
//			指定された文字列全体を nul 文字が現れるまで付加する
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
ModUnicodeString& 
ModUnicodeString::append(const ModUnicodeChar*	string,
						 const ModSize			appendLength /* = 0 */)
{
	if (string == 0) {
		return *this;	// !! 終了
	}

	// Unicode のバッファを更新する時は、内容の矛盾が起きないように
	// マルチバイト用のバッファを破棄
	d_charString.free();

	// 部分文字列の文字数
	// - 以前は appendLength が 0 以外の場合であっても string の文字数を数えて
	//   いたが、それだと無駄なので、 appendLenght が 0 以外であればその値を
	//   そのまま用いることとした
	// (2007/03/07)
	// appendLengthを直接使うと実際の文字数がそれより少ないときにコピーしすぎる
	ModSize characterNum = 0;
	if (appendLength) {
		const ModUnicodeChar* p = string;
		for (; characterNum < appendLength && *p; ++characterNum, ++p) {}
	} else {
		characterNum = ModUnicodeCharTrait::length(string);
	}

	// 渡されたものが、空文字列でない場合だけ処理を行なう
	if (characterNum > 0) {
		// 現在のUnicode文字列に部分Unicode文字列を加えた
		// 長さぶんの領域を確保する
		this->reallocate(getLength() + characterNum);

		// 与えられたワイド文字列から計算した範囲のワイド文字を複写し、
		// 末尾に ModUnicodeCharTrait::null を付加する
		ModOsDriver::Memory::copy(d_endOfString,
								  string,
								  characterNum * sizeof(ModUnicodeChar));
		*(d_endOfString += characterNum) = ModUnicodeCharTrait::null();
	}

	return *this;
}

//
// FUNCTION
// ModUnicodeString::operator+ -- + オペレータ
//
// NOTES
// この関数は 2 つの ModUnicodeString の表す文字列をつなげたものを
// 表す ModUnicodeString を得るために用いる。
//
// ARGUMENTS
// const ModUnicodeString& string
//		つなげる対象の文字列への参照
//
// RETURN
// 2 つの文字列をつなげたものを表す ModUnicodeString を返す。
//
// EXCEPTIONS
// なし
//

ModUnicodeString
ModUnicodeString::operator +(const ModUnicodeString& string)
{
	return ModUnicodeString(*this).append(string);
}

//
// FUNCTION
// ModUnicodeString::operator+ --  + オペレータ
//
// NOTES
// この関数は ModUnicodeString に ModUnicodeChar* をつなげた文字列を表す
// ModUnicodeString を得るために用いる。
//
// ARGUMENTS
// const ModUnicodeChar* string
//		つなげる対象のModUnicodeChar配列へのポインタ
//
// RETURN
// 2 つの文字列をつなげたものを表す ModUnicodeString を返す。
//
// EXCEPTIONS
// なし
//

ModUnicodeString
ModUnicodeString::operator +(const ModUnicodeChar* string)
{
	return ModUnicodeString(*this).append(string);
}

//
// FUNCTION
// ModUnicodeString::operator+ -- ModUnicodeChar* を第一オペランドに持つ + オペレータ
//
// NOTES
// この関数は char* に ModUnicodeString をつなげた文字列を表す
// ModUnicodeString を得るために用いる。
//
// ARGUMENTS
// const ModUnicodeChar* string1
//		つなげる元のModUnicodeChar配列へのポインタ
// const ModUnicodeString& string2
//		string1 につなげる文字列への参照
//
// RETURN
// 2 つの文字列をつなげたものを表す ModUnicodeString を返す。
//
// EXCEPTIONS
// なし
//

ModUnicodeString
operator +(const ModUnicodeChar* string1, const ModUnicodeString& string2)
{
	return ModUnicodeString(string1).append(string2);
}

ModUnicodeString
operator +(const char* string1, const ModUnicodeString& string2)
{
	return ModUnicodeString(string1).append(string2);
}

//
// FUNCTION
// ModUnicodeString::freeNewString -- "getNewString" で取得したバッファを解放する
//
// NOTES
// "getNewString" で取得したバッファを解放する
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
// 
void
ModUnicodeString::freeNewString(char* newString_)
{
	if (newString_ != 0) {
		newString_ -= sizeof(ModSize);
		const ModSize bufferSize = *((ModSize*)newString_);
		ModStandardManager::free((void*)newString_, bufferSize);
	}
}

//	FUNCTION public
//	ModUnicodeString::reallocate -- バッファ領域の拡張
//
//	NOTES
//		この関数を呼び出すと、既存のバッファ領域の中身を持つ
//		指定された文字数の文字列を格納できるバッファが確保される
//		既存のバッファ領域がなければ、
//		指定された文字数の文字列が格納可能なバッファが確保される
//
//		もともとは private だったが、append を複数呼び出す前に最終的な
//		大きさがわかっている場合などは、本メソッドでバッファを拡張できる
//		ように public にした。
//
//	ARGUMENTS
//		ModSize				len
//			バッファ領域に格納したい文字列の文字数
//			ただし、文字列末尾の ModUnicodeCharTrait::null は除く
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ModUnicodeString::reallocate(const ModSize len)
{
	if (d_buffer == 0) {
		// まだバッファが存在しないうちは普通にアロケートする
		allocate(len);
		return;
	}

	ModSize	newSize = (len + 1) * sizeof(ModUnicodeChar);
	ModSize	oldSize = getBufferSize();

	if (oldSize < newSize) {

		// 既存のバッファ領域は、指定されたサイズより小さい

		// 現在のサイズの 1/10 より小さいサイズしか
		// 増えないかもしれないとき、
		// 現在のサイズの 2 倍にするほうが、
		// 結果的に、再度、この関数が呼び出される可能性が減る

		ModSize	current = getLength();
		; ModAssert(d_allocateStep != 0);
		if (current / 10 > d_allocateStep)
			newSize = ModMax(newSize, current * 2);


		// 確保するサイズを _allocateStep 単位に切り上げる

		; ModAssert(d_allocateStep != 0);
		newSize = d_allocateStep * (newSize / d_allocateStep +
									((newSize % d_allocateStep) ? 1 : 0));

		// 計算したサイズの領域を確保する

		ModUnicodeChar*	buf = (ModUnicodeChar*)
			ModStandardManager::allocate(newSize);
		; ModAssert(buf != 0);

		if (current) {

			// 現在のバッファ中の末尾の
			// ModUnicodeCharTrait::null を含む文字列を
			// 新しいバッファへ複写する

			; ModAssert(d_buffer != 0);
			ModOsDriver::Memory::copy(buf, d_buffer,
									  (current + 1) * sizeof(ModUnicodeChar));
		} else
			*buf = ModUnicodeCharTrait::null();

		if (d_buffer)

			// 現在のバッファ領域を破棄する

			ModStandardManager::free(d_buffer, oldSize);

		// 新しいバッファを設定する

		d_buffer = buf;
		d_endOfBuffer = d_buffer + newSize / sizeof(ModUnicodeChar);
		d_endOfString = d_buffer + current;
	}
}

//	FUNCTION public
//	ModUnicodeString::serialize -- Unicode 文字列のシリアライザー
//
//	NOTES
//	文字列のバッファと文字数(終端文字を含まない)をアーカイバーに読み書きする。
//	アロケートステップは読み書きしない。
//
//	ARGUMENTS
//		ModArchive&			archiver
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//
void
ModUnicodeString::serialize(ModArchive& archiver)
{
	ModSize	len;

	if (archiver.isStore()) {
		// 要素数 len の配列(要素の型は unsigned short)を書き込む
		len = this->getLength();
		archiver << len;
		(void) archiver.writeArchive(d_buffer, len);
	} else {
		// 要素数 len の配列(要素の型は unsigned short)を読み込む
		archiver >> len;

		// 読み出される文字数ぶんの領域を確保し、
		// 空文字列が格納されていることにする

		this->reallocate(len);
		this->clear();

		if (len > 0) {
			// 文字列を読み出し、末尾に '\0' を付加する
			(void) archiver.readArchive(d_buffer, len);
			*(d_endOfString = d_buffer + len) = ModUnicodeCharTrait::null();
		}
	}
}

//
// アクセッサ
//

//
// FUNCTION
// ModUnicodeString::getString -- 任意の文字コードのマルチバイト文字を取得
//
// NOTES
// 任意のマルチバイト文字列を格納したバッファのアドレスを取得
// (バッファの寿命管理は本クラスが責任を持つ)
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
const char*
ModUnicodeString::getString(
	ModKanjiCode::KanjiCodeType code /* = ModKanjiCode::utf8 */)
{
 	return d_charString.getBuffer(d_buffer, d_endOfString, code);
}

//
// FUNCTION
// ModUnicodeString::getStringBufferSize -- getString が返した文字列のサイズ
//
// NOTES
// getString が返す文字列(終端文字を含む)を格納するのに充分な領域の
// バイト数を返す。getString の実行直後に呼び出すこと。
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
ModSize
ModUnicodeString::getStringBufferSize() const
{
 	return d_charString.getBufferSize();
}

//
// FUNCTION
// ModUnicodeString::getNewString -- 任意の文字コードのマルチバイト文字を取得
//
// NOTES
// 任意のマルチバイト文字列を格納したバッファをアロケートして返す
// (バッファの寿命管理は呼び出し側が行なう)
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
// なし
//
char*
ModUnicodeString::getNewString(
	ModKanjiCode::KanjiCodeType code_ /* = ModKanjiCode::utf8 */) const
{
	// Unicode 文字列に含まれる文字数(終端文字は含まない)を取得
	ModSize characterNum = (ModSize)(d_endOfString - d_buffer);

	// マルチバイト文字列(終端文字は含まない)に必要なバイト数を求める
	ModSize	mbBufferSize = 0;
	if (d_buffer == 0 || d_buffer == d_endOfString) {
		// 文字列が空ならばコード変換後のバイト数はゼロ
		mbBufferSize = 0;
	} else if (code_ == ModKanjiCode::unknown) {
		// 変換先が ASCII (7bit)文字ならば変換後のバイト数は文字数と同じ
		mbBufferSize = characterNum;
	} else {
		// マルチバイト文字列の文字数を取得
		mbBufferSize
			= ModKanjiCode::jjGetTransferredSize((char*)d_buffer,
												 ModKanjiCode::ucs2,
												 code_);
	}

	// 「バッファ情報領域＋マルチバイト文字列＋終端文字」の領域を確保
	// - バッファ情報領域の大きさは sizeof(ModSize) バイトである
	// - 今のところ、いちばんバイト数が多い"終端文字"は UCS2 のものである
	; ModAssert(CharString::s_biggestNulSize > 0);
	ModSize	bufferSize	=
		sizeof(ModSize) + mbBufferSize + CharString::s_biggestNulSize;
	char*	buffer		= (char*)ModStandardManager::allocate(bufferSize);
	; ModAssert(buffer != 0);

	// バッファ情報(バッファのサイズ)をバッファの先頭に書き込む
	ModSize*	infoPtr = (ModSize*)buffer;
	*infoPtr			= bufferSize;
	// バッファ情報の直後に文字列を格納するためにアドレスを進める
	buffer				+= sizeof(bufferSize);

	if (mbBufferSize > 0) {
		if (code_ == ModKanjiCode::unknown) {
			// ASCII 文字に変換する。
			const ModUnicodeChar*	srcPtr = d_buffer;
			char*					dstPtr = buffer;
			for (; srcPtr < d_endOfString; ++srcPtr, ++dstPtr) {
			    *dstPtr = char(*srcPtr);
			}
		} else {
			// マルチバイト文字列に変換する
			ModKanjiCode::jjTransfer(buffer,
									 mbBufferSize,
									 code_,
									 (char*)d_buffer,
									 ModKanjiCode::ucs2);
		}
	}
	// nul 文字を追加
	char* nullPointer = buffer + mbBufferSize;
	if (code_ == ModKanjiCode::ucs2) {
		*((ModUnicodeChar*)nullPointer) = ModUnicodeCharTrait::null();
	} else {
		*nullPointer = ModCharTrait::null();
	}

	return buffer;
}

//	FUNCTION public
//	ModUnicodeString::copy -- 部分 Unicode 文字列の取得
//
//	NOTES
//		自分自身が格納している Unicode 文字列のある一部分を取り出す
//
//	ARGUMENTS
//		ModSize				start
//			指定されたとき
//				自分自身の格納文字列の先頭から何文字目から
//				部分文字列を取り出すか
//				先頭の文字は、先頭から 0 文字目になる
//			範囲外を指定されたとき
//				空文字列を返す。デフォルト漢字コード自分と同じ。
//			指定されないとき
//				0 が指定されたものとみなす
//		ModSize				len
//			0 より大きい値
//				取り出す部分文字列の文字数。もし、コピー中に nul 文字に
//				遭遇した場合は末尾までを取り出す
//			0 または指定されないとき
//				自分自身の格納文字列の末尾まで取り出す
//
//	RETURN
//		取り出した部分 Unicode 文字列
//
//	EXCEPTIONS

ModUnicodeString
ModUnicodeString::copy(const ModSize start	/* = 0 */,
					   const ModSize len	/* = 0 */) const
{
	// Unicode 文字列の文字数を取得
	ModSize	characterNum = getLength();

	if (start >= characterNum) {
		// コピーの開始位置が範囲外の場合は空文字列を返す
		return ModUnicodeString();
	}

	ModSize copyNum = len;	// コピーする文字数
	if (len == 0 || characterNum < start + len) {
		// 文字数に0が渡された時や、指示された文字数だけ文字をコピー
		// する前に nul 文字に遭遇する場合には Unicode 文字列バッファの
		// nul 文字までをコピーする
		copyNum = characterNum - start;
	}

	// 残りの文字数は複写される文字数以上である
	return ModUnicodeString(d_buffer + start, copyNum);
}

void
ModUnicodeString::copy(ModUnicodeString& target,
					   const ModSize start	/* = 0 */,
					   const ModSize len	/* = 0 */) const
{
	// Unicode 文字列の文字数を取得
	ModSize	characterNum = getLength();

	if (start >= characterNum) {
		// コピーの開始位置が範囲外の場合は空文字列を返す
		target.clear();
		return;
	}

	ModSize copyNum = len;	// コピーする文字数
	if (len == 0 || characterNum < start + len) {
		// 文字数に0が渡された時や、指示された文字数だけ文字をコピー
		// する前に nul 文字に遭遇する場合には Unicode 文字列バッファの
		// nul 文字までをコピーする
		copyNum = characterNum - start;
	}

	// 残りの文字数は複写される文字数以上である
	target.allocateCopy(d_buffer + start, copyNum);
}

//
// FUNCTION 
// ModUnicodeString::search -- ModUnicodeChar*の文字列を検索する(ケースフラグつき)
//
// NOTES
// この関数は ModUnicodeString から ModUnicodeChar 配列を探すのに用いられる。
//
// ARGUMENTS
// const ModUnicodeChar* string
//		検索する文字列
// const ModBoolean caseFlag
//		ケースセンシティブか否かを指定するフラグ。ModTrue のときセンシティブ。
//
// RETURN
// 一致する文字列が見つかった場合、その位置を示す ModUnicodeChar* 型を返す。
// 見つからなかった場合、0 を返す。
//
// EXCEPTIONS
// その他
// なし
//

ModUnicodeChar*
ModUnicodeString::search(const ModUnicodeChar*	string,
						 const ModBoolean		caseFlag)
{
	ModUnicodeChar*		wp		= d_buffer;
	ModUnicodeChar*		wpEnd 	= d_endOfString;
	const ModUnicodeChar*	sp;
	ModUnicodeChar*		candid;
	ModUnicodeChar*		nextCandid;

	while (wp < wpEnd) {
		nextCandid = 0;
		// string に一致する位置まで進む
		for (;
			 wp < wpEnd
				 && (*wp != *string
					 && (caseFlag == ModTrue
						 || (ModUnicodeCharTrait::toUpper(*wp)
							 != ModUnicodeCharTrait::toUpper(*string))));
			 wp++);
		if (wp == wpEnd) {
			// 一致する部分はない
			return 0;
		}

		// 返り値候補
		candid = wp;

		// 残りを調べる
		for (wp++, sp = string + 1;
			 wp < wpEnd && *sp != ModUnicodeCharTrait::null()
				 && (*wp == *sp
					 || (caseFlag == ModFalse
						 && (ModUnicodeCharTrait::toUpper(*wp)
							 == ModUnicodeCharTrait::toUpper(*sp))));
			 wp++, sp++) {
			// ループのついでに次の調査開始候補を探しておく
			if (nextCandid == 0
				&& (*wp == *string
					|| (caseFlag == ModFalse
						&& (ModUnicodeCharTrait::toUpper(*wp)
							== ModUnicodeCharTrait::toUpper(*string))))) {
				nextCandid = wp;
			}
		}

		if (*sp == ModUnicodeCharTrait::null()) {
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

//	FUNCTION public
//	ModUnicodeString::rsearch -- 文字列の末尾に一番近いある文字を調べる
//
//	NOTES
//		文字列の末尾からある文字があるか調べて、あればその位置を返す
//
//	ARGUMENTS
//		ModUnicodeChar		character
//			探す文字(ASCII : 7 bit)
//		ModBoolean			caseFlag
//			ModTrue または指定されないとき
//				大文字、小文字を異なる文字とみなす
//			ModFalse
//				大文字、小文字を同じ文字とみなす
//
//	RETURN
//		0 以外
//			文字列中の指定された文字が格納されている領域のアドレス
//		0
//			指定された文字は見つからなかった
//
//	EXCEPTIONS
//		なし

ModUnicodeChar*
ModUnicodeString::rsearch(const char character, const ModBoolean caseFlag)
{
	if (d_buffer) {
		ModUnicodeChar* p = d_endOfString;
		if (caseFlag) {
			const ModUnicodeChar unicodeCharacter(character);
			while (--p >= d_buffer)
				if (*p == unicodeCharacter)
					return p;
		} else {
			const ModUnicodeChar unicodeCharacter(ModCharTrait::toUpper(character));
			while (--p >= d_buffer)
				if (ModUnicodeCharTrait::toUpper(*p) == unicodeCharacter)
					return p;
		}
	}

	return 0;
}

//
// FUNCTION
// ModUnicodeString::compare -- ModUnicodeChar*と比較する
//
// NOTES
// この関数は ModUnicodeString と ModUnicodeChar の配列を比較するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar* string
//		比較対象の文字列
// const ModSize maxLength
//		比較する文字数。この引数を 0 とすると
//		文字列全体を比較する。
//
// RETURN
// maxLength の範囲で、2 つの文字列がすべて一致する場合は 0 を返し、
// 一致していなかったら初めて一致していない場所の Unicode 文字コードが
// string のものの方が小さい場合は 1 を返し、大きい場合は -1 を返す。
//
// EXCEPTIONS
// なし
//

int
ModUnicodeString::compare(const ModUnicodeChar*	string)	const
{
	if (d_buffer) {
		return ModUnicodeCharTrait::compare(d_buffer, string);
	} else {
		return ModUnicodeCharTrait::compare(&s_nul, string);
	}
}

int
ModUnicodeString::compare(const ModUnicodeChar*	string,
						  const ModSize			len)	const
{
	if (d_buffer) {
		return ModUnicodeCharTrait::compare(d_buffer, string, len);
	} else {
		return ModUnicodeCharTrait::compare(&s_nul, string, len);
	}
}

//
// FUNCTION
// ModUnicodeString::compare -- ModUnicodeChar*と比較する(ケースフラグつき)
//
// NOTES
// この関数は ModUnicodeString と ModUnicodeChar の配列を比較するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar* string
//		比較対象の文字列
// const ModBoolean caseFlag
//		この引数が ModFalse のときは大文字と小文字を同一視する。
// const ModSize maxLength
//		比較する文字数。この引数を 0 とすると
//		文字列全体を比較する。
//
// RETURN
// maxLength の範囲で、2 つの文字列がすべて一致する場合は 0 を返し、
// 一致していなかったら初めて一致していない場所の Unicode 文字コードが
// string のものの方が小さい場合は 1 を返し、大きい場合は -1 を返す。
// caseFlag が ModFalse のときは上記を大文字と小文字を同一視して行なう。
//
// EXCEPTIONS
// なし
//

int
ModUnicodeString::compare(const ModUnicodeChar*	string,
						  const ModBoolean		caseSensitive,
						  const ModSize			len) const
{
	if (d_buffer) {
		return ModUnicodeCharTrait::compare(d_buffer, string, caseSensitive, len);
	} else {
		return ModUnicodeCharTrait::compare(&s_nul, string, caseSensitive, len);
	}
}

//	FUNCTION public
//	ModUnicodeString::allocateCopy -- 指定されたマルチバイト文字列を複写する
//
//	NOTES
//		指定されたマルチバイト文字列から指定された数の文字を取り出し、
//		格納文字列とする
//
//		そのとき、必要であればバッファ領域を拡張する
//
//		code == ModKanjiCode::ucs2 のときと、
//		code なしの allocateCopy の動作は異なる
//
//	ARGUMENTS
//		char*				s
//			複写するマルチバイト文字列を格納する領域の先頭アドレス
//			0 が指定されると、空文字列に相当する状態になる
//
//			領域中に終端文字がなくても正しく動作する
//
//		ModSize				len
//			0 より大きい値
//				指定された文字列の先頭からユニコード文字へ変換しながら
//				複写する文字数
//				ただし、終端文字に遭遇した場合は、その位置で処理を終了する
//			0 または指定されないとき
//				指定された文字列を終端文字が現れるまで複写する
//
//		ModKanjiCode::KanjiCodeType	code
//			指定された文字列の漢字コード
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ModUnicodeString::allocateCopy(
	const char* s, const ModSize len, const ModKanjiCode::KanjiCodeType code)
{
	// まず、空文字列が格納された状態に初期化する

	this->clear();

	if (s) {

		// なん文字複写すべきかを求める

		ModSize n = 0;
		if (code == ModKanjiCode::ucs2)

			// 複写元の文字列の符号化方式が UCS2 のとき
			//
			//【注意】	short は 2 バイト境界でアクセスする
			//			必要のある CPU があるため、char 単位で調べる

			for (const char* p = s; (*p || *(p + 1)) && ++n != len; p += 2) ;

		else if (code == ModKanjiCode::unknown)

			// 複写元の文字列の符号化方式が ASCII のとき

			for (const char* p = s; *p && ++n != len; ++p) ;

		else

			// 複写元の文字列の符号化方式が UCS2 以外の多バイト文字のとき

			// 複写元の文字列に終端文字が含まれないとき、
			// len 文字複写し、複写元の文字列に終端文字が含まれるとき、
			// 最大 len 文字複写する

			//【注意】	最初は ModKanjiCode::getCharacterSize で数えていたが、
			//			jj が変換する文字数と食い違うケースがあるので
			//			jjGetTransferredSize を使うようにした
			//
			//			が、入力に終端文字が含まれないと
			//			jjGetTransferredSize は正しく動作しないので、
			//			結局、最大 len 文字複写することにする

			n = (len) ?	len : ModKanjiCode::jjGetTransferredSize(
				s, code, ModKanjiCode::ucs2) / sizeof(ModUnicodeChar);

		// 複写すべき文字数ぶんのバッファ領域を確保し、
		// 空文字列を格納済の状態に初期化する

		this->reallocate(n);

		// 確保したバッファ領域に
		// 複写元の文字列を必要に応じて UCS2 に変換しながらコピーする

		if (code == ModKanjiCode::ucs2)
			ModOsDriver::Memory::copy(
				(char*)d_buffer,
				s, n * sizeof(ModUnicodeChar));

		else if (code == ModKanjiCode::unknown) {
			ModUnicodeChar*	p = d_buffer;
			for (ModSize i = 0; i < n; ++i, ++s, ++p)
			    *p = ModUnicodeChar(*s);
		} else {
			ModKanjiCode::jjTransfer(
				(char*)d_buffer,
				n * sizeof(ModUnicodeChar),	ModKanjiCode::ucs2, s, code);

			if (len) {

				// 複写先の文字列が何文字か数える

				n = 0;
				for (const ModUnicodeChar* p = d_buffer;
					 *p != ModUnicodeCharTrait::null() && ++n != len; ++p) ;
			}
		}

		// 終端文字を設定する

		*(d_endOfString = d_buffer + n) = ModUnicodeCharTrait::null();
	}
}

//	FUNCTION private
//	ModUnicodeString::allocateCopy -- 指定された Unicode 文字配列を複写する
//
//	NOTES
//		指定された Unicode 文字配列から指定された数の文字を取り出し、
//		格納文字列とする
//		そのとき必要であれば、バッファ領域を拡張する
//
//	ARGUMENTS
//		ModUnicodeChar*		s
//			複写する文字配列を格納する領域の先頭アドレス
//			0 が指定されると、空文字列が複写される
//		ModSize				len
//			0 より大きい値
//				指定された文字配列の先頭から複写する文字数
//				複写する文字の中に  nul 文字があっても全ての文字を複写する
//			0 または指定されないとき
//				指定された Unicode 文字配列をすべて複写する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ModUnicodeString::allocateCopy(const ModUnicodeChar* s, const ModSize len)
{
	if (s == 0) {
		// コピーするワイド文字配列を格納する領域のアドレスが 0 なので、
		// 空文字列が格納されている状態にする
		this->clear();

	} else {
		// 実際に変換する文字数を取得
		const ModSize characterNum =
			(len != 0) ? len : ModUnicodeCharTrait::length(s);

		// 「複写する文字数 + nul 文字」の領域を確保してから、空文字列が
		// 格納されている状態にする
		if (d_buffer) {
			this->reallocate(characterNum);
		} else {
			this->allocate(characterNum);
		}			
		this->clear();

		// 与えられた Unicode 文字配列を複写
		ModOsDriver::Memory::copy(
			d_buffer, s, characterNum * sizeof(ModUnicodeChar));
		*(d_endOfString = d_buffer + characterNum) = ModUnicodeCharTrait::null();
	}
}

//	FUNCTION private
//	ModUnicodeString::allocate -- バッファ領域の確保
//
//	NOTES
//		既存のバッファ領域がないときにこの関数を呼び出すと、
//		ModUnicodeString::reallocate より高速に
//		指定された文字数の文字列を格納できるバッファが確保される
//
//	ARGUMENTS
//		ModSize				len
//			バッファ領域に格納したい文字列の文字数
//			ただし、文字列末尾の ModUnicodeCharTrait::null は除く
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ModUnicodeString::allocate(const ModSize len)
{
	; ModAssert(this->d_buffer == 0);
	; ModAssert(this->d_endOfBuffer == 0);
	; ModAssert(this->d_endOfString == 0);

	// 指定された文字数は文字列末尾の
	// ModUnicodeCharTrait::null のぶんを含まないので、
	// そのぶんを加えて、必要なバイト数を求める

	ModSize	size = (len + 1) * sizeof(ModUnicodeChar);

	// 確保するサイズを _allocateStep 単位に切り上げる

	; ModAssert(d_allocateStep != 0);
	size = d_allocateStep * (size / d_allocateStep +
							((size % d_allocateStep) ? 1 : 0));

	// 計算したサイズの領域を確保する

	d_buffer = (ModUnicodeChar*) ModStandardManager::allocate(size);
	; ModAssert(d_buffer != 0);
	d_endOfBuffer = d_buffer + size / sizeof(ModUnicodeChar);
	*(d_endOfString = d_buffer) = ModUnicodeCharTrait::null();
}

// FUNCTION private
// ModUnicodeString::checkAsciiCharacter -- ASCII でない場合は例外を送出
//
// NOTES
// char 型の文字の8ビット目を調べる。もし、ビットが立っていれば ASCII では
// ないとみなして例外を送出する。
//
// ARGUMENTS
//	const char character
//		ASCII か否かを調べたい文字
//
// RETURN
//		なし
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		わたされた文字は ASCII ではない(8ビット目が立っていた)
//	

void
ModUnicodeString::checkAsciiCharacter(const char character) const
{
	if (character & 0x80) {
		// 8ビット目が立っていた
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}
}

///////////////////////////////////////
// 内部クラス

// FUNCTION public
// ModUnicodeString::CharString::CharString -- 
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
ModUnicodeString::CharString::CharString(const ModKanjiCode::KanjiCodeType code)
	: d_code(code),
	  d_buffer(0),
	  d_size(0)
{
	; // do nothing
}

// FUNCTION public
// ModUnicodeString::CharString::~CharString -- 
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
ModUnicodeString::CharString::~CharString()
{
	free();
}

// FUNCTION public
// ModUnicodeString::CharString::setCode -- 
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
void
ModUnicodeString::CharString::setCode(const ModKanjiCode::KanjiCodeType code)
{
	d_code = code;
}

// FUNCTION public
// ModUnicodeString::CharString::getCode -- 
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS

ModKanjiCode::KanjiCodeType
ModUnicodeString::CharString::getCode() const
{
	return d_code;
}

// FUNCTION public
// ModUnicodeString::CharString::getBuffer -- マルチバイト文字列の先頭アドレス
//
// NOTES
// マルチバイト文字列の先頭アドレスを取得。この文字列は nul-terminate
// している。
//
// ARGUMENTS
//
// RETURN
// マルチバイト文字列の先頭アドレス
//
// EXCEPTIONS

const char*
ModUnicodeString::CharString::getBuffer(
	const ModUnicodeChar*				unicodeBufferBegin,
	const ModUnicodeChar*				unicodeBufferEnd,
	const ModKanjiCode::KanjiCodeType	code)
{
	if (d_buffer) {
		if (code == d_code) {
			// 前に求めた値が残っているので、そのまま返す
			return d_buffer;
		} else {
			// 前に求めた時と漢字コードが違うのでバッファ領域を解放
			free();
		}
	}

	// !! ここを通るならば、マルチバイト文字列を生成しなければいけない。

	// Unicode 文字列に含まれる文字数を取得
	ModSize characterNum = (ModSize)(unicodeBufferEnd - unicodeBufferBegin);

	// マルチバイト文字列(nul 文字を含まない)に必要なバイト数を求めて
	// メンバ変数を初期化する
	d_code = code;
	if (unicodeBufferBegin == 0 || unicodeBufferBegin == unicodeBufferEnd) {
		// 文字列が空ならばコード変換後のバイト数はゼロ
		d_size = 0;
	} else if (d_code == ModKanjiCode::unknown) {
		// 変換先が ASCII (7bit)文字ならば変換後のバイト数は文字数と同じ
		d_size = characterNum;
	} else {
		// マルチバイト文字列の文字数を取得
		d_size = ModKanjiCode::jjGetTransferredSize((char*)unicodeBufferBegin,
													ModKanjiCode::ucs2,
													d_code);
	}

	// 少なくとも終端文字を記憶できるだけの領域を確保する

	d_buffer = (char*) ModStandardManager::allocate(this->getBufferSize());
	; ModAssert(d_buffer);

	if (d_size > 0) {
		if (d_code == ModKanjiCode::unknown) {
			// ASCII 文字に変換する。
			const ModUnicodeChar*	srcPtr = unicodeBufferBegin;
			char*					dstPtr = d_buffer;
			for (; srcPtr < unicodeBufferEnd; ++srcPtr, ++dstPtr) {
			    *dstPtr = char(*srcPtr);
			}
		} else {
			// マルチバイト文字列に変換する
			ModKanjiCode::jjTransfer(d_buffer,
									 d_size,
									 d_code,
									 (char*)unicodeBufferBegin,
									 ModKanjiCode::ucs2);
		}
	}
	// nul 文字を追加
	if (code == ModKanjiCode::ucs2) {
		*((ModUnicodeChar*)(d_buffer + d_size)) = ModUnicodeCharTrait::null();
	} else {
		*(d_buffer + d_size) = ModCharTrait::null();
	}		

	return d_buffer;
}

// FUNCTION public
// ModUnicodeString::CharString::getBufferSize -- マルチバイト文字列のバッファサイズ
//
// NOTES
// マルチバイト文字列(終端文字も含む)のバッファサイズを返す。
// サイズの単位はバイト数。
//
// ARGUMENTS
//
// RETURN
// バッファサイズ(単位:バイト数)
//
// EXCEPTIONS

ModSize
ModUnicodeString::CharString::getBufferSize() const
{
	return d_size + ((d_code == ModKanjiCode::ucs2) ?
					 sizeof(ModUnicodeCharTrait::null()) :
					 sizeof(ModCharTrait::null()));		
}

// FUNCTION public
// ModUnicodeString::CharString::free -- マルチバイト文字列情報をクリア
//
// NOTES
// マルチバイト文字列情報をクリア。ただし、デフォルト漢字コードは変更しない。
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS

void
ModUnicodeString::CharString::free()
{
	if (d_buffer) {
		ModStandardManager::free((void*) d_buffer, this->getBufferSize());
		d_buffer = 0;
	}
	d_size = 0;
}

//
// Copyright (c) 1999, 2000, 2001, 2003, 2004, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
