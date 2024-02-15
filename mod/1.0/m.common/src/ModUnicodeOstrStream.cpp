// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModUnicodeOstrStream.cpp -- MOD の出力用ストリーム
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


#include <stdio.h>
#include "ModUnicodeOstrStream.h"
#include "ModCharString.h"
#include "ModUnicodeString.h"

// 一時バッファのアロケートステップ
const ModSize
ModUnicodeOstrStream::_allocateStep = 16;

//	FUNCTION public
//	ModUnicodeOstrStream::put -- 文字を追加する
//
//	NOTES
//		出力ストリームに与えられた文字を追加する
//
//	ARGUMENTS
//		char				character_
//			追加する ASCII 文字
//		ModUnicodeChar		character_
//			追加する Unicode 文字
//
//	RETURN
//		与えられた文字を追加後の自分自身
//
//	EXCEPTIONS
//		ModKanjiCode::jjGetTransferdSize、ModKanjiCode::jjTransfer の
//		例外を参照

ModOstream&
ModUnicodeOstrStream::put(const char character_)
{
	if ((const unsigned char)character_ < 0x80
		|| _encodingType == ModKanjiCode::unknown) {
		// 文字コードに依存しないコード値ならばコード変換不要。
		// 入力マルチバイト文字のエンコード方法が unknown の場合も変換不要。
		_strStream.putChar(character_);
	} else {
		// 文字コードに依存したコード値なのでコード変換する。
		// ASCII の1文字は UCS2 に変換しても1文字になる。
		char			src[2] = {character_, ModCharTrait::null()};
		ModUnicodeChar	dst[2] = {0, 0};

		ModKanjiCode::jjTransfer((char*)dst, sizeof(dst), ModKanjiCode::ucs2,
								 src, _encodingType);
		_strStream.putChar(dst[0]);
	}

	return *this;
}

ModOstream&
ModUnicodeOstrStream::put(const ModUnicodeChar character_)
{
	_strStream.putChar(character_);
	return *this;
}

//	FUNCTION public
//	ModUnicodeOstrStream::write -- 文字列を追加する
//
//	NOTES
//		出力ストリームに与えられた文字列を追加する
//
//	ARGUMENTS
//		char*				string_
//			追加するマルチバイト文字列
//			文字列の末尾は '\0' である必要がある
//		ModUnicodeChar*		v
//			追加する Unicode 文字列
//			文字列の末尾は ModUnicodeCharTrait::null() である必要がある
//
//	RETURN
//		与えられた文字列を追加後の自分自身
//
//	EXCEPTIONS

ModOstream&
ModUnicodeOstrStream::write(const char* string_)
{
	if (string_ != 0) {
		if (_encodingType == ModKanjiCode::unknown) {
			// 入力マルチバイト文字のエンコード方法が指定されていないので
			// 変換しない
			for (const char* p = string_; *p != ModCharTrait::null(); ++p) {
				put(ModUnicodeChar(*p));
			}
		} else {
			// 入力マルチバイト文字を UCS2 に変更するときに必要な
			// バイト数を求める(終端文字も含める)
			const ModSize ucs2BufferSize =
				ModKanjiCode::jjGetTransferredSize((char*)string_,
												   _encodingType,
												   ModKanjiCode::ucs2)
				+ sizeof(ModUnicodeChar);

			if (_workBufferSize < ucs2BufferSize) {
				// 一時バッファが小さかったので拡張する
				// (現在の一時バッファの内容は捨てる)
				const ModSize newBufferSize =
					_allocateStep * (ucs2BufferSize / _allocateStep +
									 ((ucs2BufferSize % _allocateStep) ? 1 : 0));
				ModUnicodeChar* newBuffer =
					(ModUnicodeChar*)ModStandardManager::allocate(newBufferSize);
				; ModAssert(newBuffer != 0);

				if (_workBuffer != 0) {
					ModStandardManager::free(_workBuffer, _workBufferSize);
				}
				_workBuffer		= newBuffer;
				_workBufferSize	= newBufferSize;
			}
			
			ModKanjiCode::jjTransfer((char*)_workBuffer,
									 _workBufferSize,
									 ModKanjiCode::ucs2,
									 string_,
									 _encodingType);

			// 変換結果に終端文字が入っているはず
			; ModAssert(_workBuffer[ucs2BufferSize/sizeof(ModUnicodeChar) - 1]
				== ModUnicodeCharTrait::null());
		   
			write(_workBuffer);
		}
	}

	return *this;
}

ModOstream&
ModUnicodeOstrStream::write(const ModUnicodeChar* v)
{
	_strStream.putString(v);
	return *this;
}

//	FUNCTION public
//	ModUnicodeOstrStream::getString -- 出力ストリームの内容を文字列で得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた文字列が格納されている領域の先頭アドレス
//
//	EXCEPTIONS

ModUnicodeChar*
ModUnicodeOstrStream::getString()
{
	const ModUnicodeChar*	s = _strStream.getString();
	if (s == 0) {
		_strStream.freeze(ModFalse);
		return 0;
	}

	if (_buffer)
		ModStandardManager::free(_buffer, _size);

	ModSize	len = _strStream.getSize();
	_size = (len + 1) * sizeof(ModUnicodeChar);
	_buffer = (ModUnicodeChar*) ModStandardManager::allocate(_size);
	; ModAssert(_buffer != 0);

	ModOsDriver::Memory::copy(_buffer, s, len * sizeof(ModUnicodeChar));
	_buffer[len] = ModUnicodeCharTrait::null();

	_strStream.freeze(ModFalse);

	return _buffer;
}

//
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
