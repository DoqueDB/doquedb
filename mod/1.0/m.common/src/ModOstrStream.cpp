// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModOstrStream.cpp -- MOD の出力用ストリーム
// 
// Copyright (c) 1997, 2000, 2023 Ricoh Company, Ltd.
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
#include "ModOstrStream.h"
#include "ModCharString.h"
#include "ModWideString.h"
#include "ModUnicodeString.h"

//	FUNCTION public
//	ModOstrStream::put -- 文字を追加する
//
//	NOTES
//		出力ストリームに与えられた文字を追加する
//
//	ARGUMENTS
//		char				v
//			追加する ASCII 文字
//		ModWideChar			v
//			追加するワイド文字
//		ModUnicodeChar			v
//			追加する Unicode 文字
//
//	RETURN
//		与えられた文字を追加後の自分自身
//
//	EXCEPTIONS
//		ModKanjiCode::jjGetTransferdSize、ModKanjiCode::jjTransfer の
//		例外を参照

ModOstream&
ModOstrStream::put(const char v)
{
	_strStream.putChar(v);
	return *this;
}

ModOstrStream&
ModOstrStream::put(const ModWideChar v)
{
	char	buf[5];
	buf[ModWideCharTrait::convertToString(buf, v, ModOs::Process::getEncodingType())] =	ModCharTrait::null();
	_strStream.putString(buf);
	return *this;
}

ModOstream&
ModOstrStream::put(const ModUnicodeChar v)
{
	ModUnicodeChar	src[2] = {v, ModUnicodeCharTrait::null()};
	char			dst[5];	// UTF-8 は1文字で最大4バイト
	const ModKanjiCode::KanjiCodeType type = ModOs::Process::getEncodingType();
	const ModSize len = ModKanjiCode::jjGetTransferredSize((char*)src,
														   ModKanjiCode::ucs2,
														   type);
	ModKanjiCode::jjTransfer(dst, sizeof(dst), type,
							 (char*)src, ModKanjiCode::ucs2);
	dst[len] = ModCharTrait::null();
	_strStream.putString(dst);
	return *this;
}

//	FUNCTION public
//	ModOstrStream::write -- 文字列を追加する
//
//	NOTES
//		出力ストリームに与えられた文字列を追加する
//
//	ARGUMENTS
//		char*				v
//			追加する ASCII 文字列
//			文字列の末尾は '\0' である必要がある
//		ModWideChar*		v
//			追加するワイド文字列
//			文字列の末尾は ModWideCharTrait::null() である必要がある
//		ModUnicodeChar*		v
//			追加する Unicode 文字列
//			文字列の末尾は ModUnicodeCharTrait::null() である必要がある
//
//	RETURN
//		与えられた文字列を追加後の自分自身
//
//	EXCEPTIONS

ModOstream&
ModOstrStream::write(const char* v)
{
	_strStream.putString(v);
	return *this;
}

ModOstrStream&
ModOstrStream::write(const ModWideChar* v)
{
	if (v)
		for (const ModWideChar* wp = v; *wp != ModWideCharTrait::null(); ++wp)
			this->put(*wp);
	return *this;
}

ModOstream&
ModOstrStream::write(const ModUnicodeChar* v)
{
	if (v)
		for (const ModUnicodeChar* wp = v; *wp != ModUnicodeCharTrait::null(); ++wp)
			this->put(*wp);
	return *this;
}

//	FUNCTION public
//	ModOstrStream::getString -- 出力ストリームの内容を C 文字列で得る
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

char*
ModOstrStream::getString()
{
	const char*	s = _strStream.getString();
	if (s == 0) {
		_strStream.freeze(ModFalse);
		return 0;
	}

	if (_buffer)
		ModStandardManager::free(_buffer, _size);

	ModSize	len = _strStream.getSize();
	_buffer = (char*) ModStandardManager::allocate(_size = len + 1);
	; ModAssert(_buffer != 0);

	ModOsDriver::Memory::copy(_buffer, s, len);
	_buffer[len] = ModCharTrait::null();

	_strStream.freeze(ModFalse);

	return _buffer;
}

//
// Copyright (c) 1997, 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
