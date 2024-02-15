// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModCharString.cpp -- 文字列を表すクラス関連のメソッド定義
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


extern "C" {
#include <stdarg.h>
#include <string.h>
}

#include "ModCommon.h"
#include "ModError.h"
#include "ModCharString.h"
#include "ModCharTrait.h"
#include "ModOsDriver.h"

//	VARIABLE private
//	ModCharString::_null -- バッファ領域を未確保時に使用する値
//
//	NOTES
//		C 文字列を格納するためのバッファ領域を確保していないときに、
//		空文字列を得るために使用する
//		ModCharString::compare で、これが比較できなければならないので、
//		型は CompareUnit である必要がある

const ModCharString::CompareUnit
					ModCharString::_null = (ModCharString::CompareUnit) 0;

//	VARIABLE private
//	ModCharString::_allocateStepDefault -- バッファ領域の拡張単位
//
//	NOTES
//		C 文字列を格納するためのバッファ領域を
//		拡張するときの単位のデフォルト値(B 単位)
//
//		sizeof(CompareUnit) 以上である必要がある

const ModSize	ModCharString::_allocateStepDefault = 16;

//	FUNCTION public
//	ModCharString::serialize -- char を文字単位とする文字列のシリアライザー
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
ModCharString::serialize(ModArchive& archiver)
{
	ModSize	len;

	if (archiver.isStore()) {
		len = this->getLength();
		archiver << _allocateStep;
		archiver << len;
		(void) archiver.writeArchive(this->buffer, len);
	} else {
		archiver >> _allocateStep;
		archiver >> len;

		// 読み出される文字数ぶんの領域を確保し、
		// 空文字列が格納されていることにする

		this->reallocate(len);
		this->clear();

		if (len) {

			// 文字列を読み出し、末尾に '\0' を付加する

			(void) archiver.readArchive(this->buffer, len);
			*(this->endOfString = this->buffer + len) = ModCharTrait::null();
		}
	}
}

//	FUNCTION public
//	ModCharString::operator = -- = 演算子
//
//	NOTES
//		自分自身へ文字列を代入する
//
//	ARGUMENTS
//		ModCharString&		src
//			自分自身へ代入する文字列
//
//	RETURN
//		文字列が代入された自分自身
//
//	EXCEPTIONS

ModCharString&
ModCharString::operator =(const ModCharString& src)
{
	if (this != &src) {

		// 複写する文字数ぶんの領域を確保し、
		// 空文字列が格納されていることにする

		ModSize	len = src.getLength();
		this->reallocate(len);
		this->clear();

		if (len) {

			// 与えられた文字列を末尾の '\0' を含めて複写する

			ModOsDriver::Memory::copy(this->buffer, src.buffer,
									  (len + 1) * sizeof(char));
			this->endOfString = this->buffer + len;
		}
	}
	return *this;
}

//	FUNCTION public
//	ModCharString::append -- 文字を付加する
//
//	NOTES
//		与えられた文字を自分自身の格納文字列の末尾に追加する
//
//	ARGUMENTS
//		char				src
//			自分自身の格納文字列の末尾に付加する文字
//
//	RETURN
//		文字を付加後の自分自身
//
//	EXCEPTIONS

ModCharString& 
ModCharString::append(char src)
{
	if (src != ModCharTrait::null()) {
		this->reallocate(this->getLength() + 1);
		*(this->endOfString) = src;
		*(++this->endOfString) = ModCharTrait::null();
	}
	return *this;
}

//	FUNCTION public
//	ModCharString::append -- 部分文字列を付加する
//
//	NOTES
//		与えられた文字列の先頭からある長さの部分文字列を取り出して、
//		自分自身の格納文字列の末尾に付加する
//
//	ARGUMENTS
//		const ModCharString&	src
//			自分自身の格納文字列の末尾に付加する文字列
//		ModSize				len
//			0 より大きい値
//				指定された文字列の先頭からなん文字目までを付加するか
//				ただし、指定された文字列の文字数以上は付加できない
//			0 または指定されないとき
//				指定された文字列全体を付加する
//
//	RETURN
//		部分文字列を付加後の自分自身
//
//	EXCEPTIONS

ModCharString& 
ModCharString::append(const ModCharString& src, ModSize len)
{
	ModSize	l = src.getLength();
	if (len == 0 || l < len)
		len = l;

	if (len) {

		// 現在の文字列に部分文字列を加えた長さぶんの領域を確保する

		this->reallocate(this->getLength() + len);

		// 与えられた格納文字列から計算した範囲の文字を複写し、
		// 末尾に '\0' を付加する

		ModOsDriver::Memory::copy(this->endOfString,
								  src.buffer, len * sizeof(char));
		*(this->endOfString += len) = ModCharTrait::null();
	}
	return *this;
}

//	FUNCTION public
//	ModCharString::append -- C 部分文字列を付加する
//
//	NOTES
//		与えられた C 文字列の先頭からある長さの部分文字列を取り出して、
//		自分自身の格納文字列の末尾に付加する
//
//	ARGUMENTS
//		const char*			src
//			自分自身の格納文字列の末尾に付加する C 文字列
//			0 が指定されたときは、なにも付加しない
//		ModSize				len
//			指定された C 文字列の先頭からなん文字目までを付加するか
//			ただし、指定された C 文字列の文字数以上は付加できない
//			0 が指定されたときは、全体を付加する
//
//	RETURN
//		C 部分文字列を付加後の自分自身
//
//	EXCEPTIONS

ModCharString& 
ModCharString::append(const char* src, ModSize len)
{
	ModSize l = 0;
	if (src) {
		if (len) {
			// don't proceed over len
			const char* p = src;
			for (; l < len && *p; ++l, ++p) {}
		} else {
			l = (ModSize)::strlen(src);
		}
	}
	if (len == 0 || l < len)
		len = l;

	if (len) {

		// 現在の文字列に C 部分文字列を加えた長さぶんの領域を確保する

		this->reallocate(this->getLength() + len);

		// 与えられた C 文字列から計算した範囲の文字を複写し、
		// 末尾に '\0' を付加する

		ModOsDriver::Memory::copy(this->endOfString, src, len * sizeof(char));
		*(this->endOfString += len) = ModCharTrait::null();
	}
	return *this;
}

//	FUNCTION public
//	ModCharString::clear -- 文字列を格納していれば、空文字列を格納する
//
//	NOTES
//		文字列を格納していれば、その文字列は破棄し、空文字列を格納する
//		ただし、文字列を格納するためのバッファ領域を破棄または縮小しない
//
//	ARUGMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
ModCharString::clear()
{
	// 格納文字列の先頭と終わりを一致させる

	if (this->endOfString = this->buffer)

		// 文字列の比較は CompareUnit 単位で行うので、
		// 最低でもバッファの先頭から sizeof(CompareUnit) バイトぶんは
		// 0 埋めされていないと、文字列末尾の '\0' を含めて
		// sizeof(CompareUnit) バイトより小さな文字列を比較できなくなる

		*((CompareUnit*) this->buffer) = (CompareUnit) 0;
}

//	FUNCTION public
// ModCharString::format -- 書式付の代入
//
// NOTES
// この関数は ModCharString に sprintf のような書式付の代入を行なうために
// 用いる。
// ただし、%s、%d、%u、%c、%x、%X のみをサポート。
// 表示幅と左詰め右詰めの指定はできる。
//
// ARGUMENTS
// const char* format
//		書式を指定する文字列
// ...
//		任意の数の引数。formatString の %d などの指定と型があっている必要がある
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS

ModCharString&
ModCharString::format(const char* format, ...)
{
	va_list argumentList;
	va_start(argumentList, format);

	int digit;
	int tmpDigit;
	unsigned int uDigit;
	unsigned int uTmpDigit;
	ModSize digitLength;
	ModSize stringLength;
	ModBoolean minusFlag;
	ModBoolean zeroFlag;
	ModSize width;
	ModCharString tmpString;

	this->clear();
	//
	// format の解析
	//
	for (const char* cp = format; *cp != ModCharTrait::null(); ++cp) {
		if (*cp != '%') {
			// '%'がなければそのまま追加
			this->append(*cp);
			continue;
		}
		// '%' があった -> 書式の判定

		// 表示幅を得る
		minusFlag = ModFalse;
		zeroFlag = ModFalse;
		width = 0;
		for (++cp;
			 *cp != 's' && *cp != 'c' && *cp != 'd' && *cp != 'u'
				 && *cp != 'x' && *cp != 'X' && *cp != 0;
			 ++cp) {
			if (*cp == '-') {
				// 最初でなければ正常なフォーマットでない
				if (minusFlag == ModTrue || zeroFlag == ModTrue || width > 0) {
					break;
				}
				minusFlag = ModTrue;	// 左詰めフラグセット
				continue;
			}
			if (*cp == '.') {
				// '.'や数字の後では正常なフォーマットでない
				if (zeroFlag == ModTrue || width > 0) {
					break;
				}
				zeroFlag = ModTrue;
				minusFlag = ModFalse;	// '-'があっても無視する
				continue;
			}
			if ((int)(*cp) >= (int)'0' && (int)(*cp) <= (int)'9') {
				if (width == 0 && *cp == '0') {
					// 最初に '0' -> zeroFlag セット
					zeroFlag = ModTrue;
					continue;
				}
				// 幅の数値
				width *= 10;
				width += (ModSize)(*cp) - (ModSize)'0';
				continue;
			}
			// 上記以外が来たら正常なフォーマットでない
			break;
		}
		switch (*cp) {
		case 's':
		{
			// 文字列を挿入する
			char*		v = (char*)va_arg(argumentList, char*);
			const char*	string = (v) ? v : "<null>";

			if (width == 0) {
				// 幅指定がなければそのまま追加
				this->append(string);
			} else {
				// 幅指定あり
				stringLength = ModCharTrait::length(string);
				if (width <= stringLength || minusFlag == ModTrue) {
					// 幅あふれ、または左詰め
					this->append(string);
				}
				if (width > stringLength) {
					// 左詰め、または右詰めで幅が余ってる
					// 幅の残りを空白で埋める
					for (; stringLength < width; ++stringLength) {
						this->append(' ');
					}
					if (minusFlag == ModFalse) {
						// 右詰め
						this->append(string);
					}
				}
			}
			break;
		}
		case 'c':
		{
			char character = (char)(int)va_arg(argumentList, int);

			if (width == 0) {
				// 幅指定がなければそのまま追加
				this->append(character);
			} else {
				// 幅指定あり
				stringLength = 1;
				if (width <= stringLength || minusFlag == ModTrue) {
					// 幅あふれ、または左詰め
					this->append(character);
				}
				if (width > stringLength) {
					// 左詰め、または右詰めで幅が余ってる
					// 幅の残りを空白で埋める
					for (; stringLength < width; ++stringLength) {
						this->append(' ');
					}
					if (minusFlag == ModFalse) {
						// 右詰め
						this->append(character);
					}
				}
			}
			break;
		}
		case 'd':
		case 'u':
		case 'x':
		case 'X':
			tmpString.clear();
			if (*cp == 'd') {
				digit = (int)va_arg(argumentList, int);
				// 数値を文字列に変換する
				if (digit == 0) {
					tmpString.append('0');
				}
				for (digitLength = (digit <= 0)?1:0,
						 tmpDigit = (digit < 0)?-digit:digit;
					 tmpDigit > 0; ++digitLength, tmpDigit /= 10) {
					tmpString.append('0' + (tmpDigit%10));
				}
			} else if (*cp == 'u') {
				uDigit = (unsigned int)(int)va_arg(argumentList, int);
				// 数値を文字列に変換する
				if (uDigit == 0) {
					tmpString.append('0');
				}
				for (digitLength = (uDigit == 0)?1:0, uTmpDigit = uDigit;
					 uTmpDigit > 0; ++digitLength, uTmpDigit /= 10) {
					tmpString.append('0' + (const char)(uTmpDigit%10));
				}
			} else {
				// 16進数
				uDigit = (unsigned int)(int)va_arg(argumentList, int);
				// 数値を文字列に変換する
				if (uDigit == 0) {
					tmpString.append('0');
				}
				for (digitLength = (uDigit == 0)?1:0, uTmpDigit = uDigit;
					 uTmpDigit > 0; ++digitLength, uTmpDigit /= 16) {
					int tmp = uTmpDigit % 16;
					tmpString.append((tmp >= 10)
									 ?
									 ((*cp == 'x')
									  ?
									  'a' + (tmp - 10)
									  :
									  'A' + (tmp - 10))
									 :
									 '0' + tmp);
				}
			}
			tmpString.reverse();
			if (width == 0) {
				// 幅指定なし
				if (*cp == 'd' && digit < 0) {
					this->append('-');
				}
				this->append(tmpString);
			} else {
				// 幅指定あり
				if (width <= digitLength || minusFlag == ModTrue) {
					// 幅あふれ、または左詰め
					if (*cp == 'd' && digit < 0) {
						this->append('-');
					}
					this->append(tmpString);
				}
				if (width > digitLength) {
					// 左詰め、または右詰めで幅が余ってる
					if (minusFlag == ModFalse && zeroFlag == ModTrue) {
						// 右詰め、かつ 0 パッディング指定
						if (*cp == 'd' && digit < 0) {
							this->append('-');
						}
						for (; digitLength < width; ++digitLength) {
							this->append('0');
						}
					} else {
						// 空白で埋める
						for (; digitLength < width; ++digitLength) {
							this->append(' ');
						}
					}
					if (minusFlag == ModFalse) {
						// 右詰め
						if (zeroFlag == ModFalse && *cp == 'd' && digit < 0) {
							this->append('-');
						}
						this->append(tmpString);
					}
				}
			}
			break;
		default:
			break;
		} // of switch
		if (*cp == ModCharTrait::null()) {
			break;
		}
	}

	va_end(argumentList);

	return *this;
}

//	FUNCTION public
//	ModCharString::rsearch -- 文字列の末尾に一番近いある文字を調べる
//
//	NOTES
//		文字列の末尾からある文字があるか調べて、あればその位置を返す
//
//	ARGUMENTS
//		char				c
//			探す文字
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

char*
ModCharString::rsearch(char c, ModBoolean caseFlag)
{
	if (this->buffer) {
		char* p = this->endOfString;
		if (caseFlag) {
			while (--p >= this->buffer)
				if (*p == c)
					return p;
		} else {
			c = ModCharTrait::toUpper(c);
			while (--p >= this->buffer)
				if (ModCharTrait::toUpper(*p) == c)
					return p;
		}
	}

	return 0;
}

//	FUNCTION public
//	ModCharString::copy -- 部分文字列の取得
//
//	NOTES
//		自分自身が格納している文字列のある一部分を取り出す
//
//	ARGUMENTS
//		ModSize				start
//			指定されたとき
//				自分自身の格納文字列の先頭から何文字目から
//				部分文字列を取り出すか
//				先頭の文字は、先頭から 0 文字目になる
//			指定されないとき
//				0 が指定されたものとみなす
//		ModSize				len
//			0 より大きい値
//				取り出す部分文字列の文字数
//			0 または指定されないとき
//				自分自身の格納文字列の末尾まで取り出す
//
//	RETURN
//		取り出した部分文字列
//
//	EXCEPTIONS

ModCharString
ModCharString::copy(ModSize start, ModSize len) const
{
	ModSize	l = this->getLength();
	if (len == 0 || l < start + len)
		len = (start < l) ? l - start : 0;

	ModCharString	dst;

	if (len) {

		// 複写する文字数ぶんの領域を確保し、
		// 空文字列が格納されていることにする

		dst.allocate(len);

		// 自分の格納文字列から計算した範囲の文字を複写し、
		// 末尾に '\0' を付加する

		ModOsDriver::Memory::copy(dst.buffer, this->buffer + start,
								  len * sizeof(char));
		*(dst.endOfString = dst.buffer + len) = ModCharTrait::null();
	}

	return dst;
}

//	FUNCTION private
//	ModCharString::allocate -- バッファ領域の確保
//
//	NOTES
//		既存のバッファ領域がないときにこの関数を呼び出すと、
//		ModCharString::reallocate より高速に
//		指定された文字数の文字列を格納できるバッファが確保される
//
//	ARGUMENTS
//		ModSize				len
//			バッファ領域に格納したい文字列の文字数
//			ただし、文字列末尾の '\0' は除く
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ModCharString::allocate(ModSize len)
{
	; ModAssert(this->buffer == 0);
	; ModAssert(this->endOfBuffer == 0);
	; ModAssert(this->endOfString == 0);

	if (len) {

		// 指定された文字数は文字列末尾の '\0' のぶんを含まないので、
		// '\0' のぶんを加えて、必要なバイト数を求める
		//
		//【注意】	0 が指定されたときは、バッファ領域を確保しない

		ModSize	size = (len + 1) * sizeof(char);

		// 確保するサイズを _allocateStep 単位に切り上げる

		; ModAssert(_allocateStep != 0);
		size = _allocateStep * (size / _allocateStep +
								((size % _allocateStep) ? 1 : 0));

		// 計算したサイズの領域を確保する

		this->buffer = this->endOfString =
			(char*) ModStandardManager::allocate(size);
		; ModAssert(this->buffer != 0);
		this->endOfBuffer = this->buffer + size / sizeof(char);

		// 文字列の比較は CompareUnit 単位で行うので、
		// 最低でもバッファの先頭から sizeof(CompareUnit) バイトぶんは
		// 0 埋めされていないと、文字列末尾の'\0' を含めて
		// sizeof(CompareUnit) バイトより小さな文字列を比較できなくなる

		*((CompareUnit*) this->buffer) = (CompareUnit) 0;
	}
}

//	FUNCTION public
//	ModCharString::reallocate -- バッファ領域の拡張
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
//			ただし、文字列末尾の '\0' は除く
//			0 が指定されたとき、バッファ領域は確保されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ModCharString::reallocate(ModSize len)
{
	//	バッファ領域は以下のように複数のメンバーにより参照される
	//
	//	┌───┬───────┐
	//	│abcdef│\0            │
	//	└───┴───────┘
	//	↑      ↑              ↑
	//	buffer  endOfString     endOfBuffer
	//
	//	buffer			バッファ領域の先頭
	//	endOfString		文字列の末尾(\0 を含まない)
	//	endOfBuffer		バッファ領域の末尾
	//					ModMax(buffer + sizeof(CompareUnit),
	//                         endOfString + 1) 以上である

	// 指定された文字数は文字列末尾の '\0' のぶんを含まないので、
	// '\0' のぶんを加えて、必要なバイト数を求める
	//
	//【注意】	0 が指定されたときは、バッファ領域は必要ない

	ModSize newSize = (len) ? ((len + 1) * sizeof(char)) : 0;
	ModSize oldSize = this->getBufferSize();

	if (oldSize < newSize) {

		// 既存のバッファ領域は、指定されたサイズより小さい

		// バッファ領域は _allocateStep 単位で確保されるので、
		// 最小の増分も _allocateStep になる
		//
		// 現在のサイズの 1/10 より小さいサイズしか
		// 増えないかもしれないとき、
		// 現在のサイズの 2 倍にするほうが、
		// 結果的に、再度、この関数が呼び出される可能性が減る

		ModSize	current = this->getSize();
		; ModAssert(_allocateStep != 0);
		if (current / 10 > _allocateStep)
			newSize = ModMax(newSize, current * 2);

		// 確保するサイズを _allocateStep 単位に切り上げる

		; ModAssert(_allocateStep != 0);
		newSize = _allocateStep * (newSize / _allocateStep +
								   ((newSize % _allocateStep) ? 1 : 0));

		// 計算したサイズの領域を確保する

		char*	buf = (char*) ModStandardManager::allocate(newSize);
		; ModAssert(buf != 0);

		// 文字列の比較は CompareUnit 単位で行うので、
		// 最低でもバッファの先頭から sizeof(CompareUnit) バイトぶんは
		// 0 埋めされていないと、文字列末尾の'\0' を含めて
		// sizeof(CompareUnit) バイトより小さな文字列を比較できなくなる

		*((CompareUnit*) buf) = (CompareUnit) 0;

		if (current) {

			// 現在のバッファ中の末尾の '\0' も含む文字列を
			// 新しいバッファへ複写する

			; ModAssert(this->buffer != 0);
			ModOsDriver::Memory::copy(buf, this->buffer,
									  current + sizeof(char));
		}
		if (this->buffer)

			// 現在のバッファ領域を破棄する

			ModStandardManager::free(this->buffer, oldSize);

		// 新しいバッファを設定する

		this->buffer = buf;
		this->endOfBuffer = this->buffer + newSize / sizeof(char);
		this->endOfString = this->buffer + current / sizeof(char);
	}
}

//	FUNCTION private
//	ModCharString::allocateCopy -- 指定された文字列を複写する
//
//	NOTES
//		指定された文字列から指定された数の文字を取り出し、格納文字列とする
//		そのとき必要であれば、バッファ領域を拡張する
//
//	ARGUMENTS
//		char*				s
//			複写する C 文字列を格納する領域の先頭アドレス
//			0 が指定されると、空文字列が複写される
//		ModSize				len
//			指定された C 文字列の先頭から複写する文字数
//			ただし、指定された C 文字列の文字数以上は複写されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ModCharString::allocateCopy(const char* s, ModSize len)
{
	if (s == 0)

		// コピーする文字列を格納する領域のアドレスが 0 なので、
		// 空文字列が格納されていることにする

		this->clear();
	else {

		// 与えられた文字列の文字数を求める
		if (len == 0) {
			len = (ModSize) ::strlen(s);
		} else {
			// 与えられた文字列がターミネイトしていないかもしれないので
			// findで調べる
			char* p = ModCharTrait::find(s, ModCharTrait::null(), len);
			if (p != 0 && (p - s) < len) {
				// 複写する文字数が 0 または実際の文字数より大きいときは、
				// 実際の文字数ぶん複写する
				len = (ModSize)(p - s);
			}
		}

		// 複写する文字数ぶんの領域を確保し、
		// 空文字列が格納されていることにする

		this->reallocate(len);
		this->clear();

		if (len) {

			// 与えられた文字列を末尾の '\0' を含めて複写する

			ModOsDriver::Memory::copy(this->buffer, s, len * sizeof(char));
			*(this->endOfString = this->buffer + len) = ModCharTrait::null();
		}
	}
}

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
