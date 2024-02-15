// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Unicode.h -- ユニコードを表すクラス定義、関数宣言
// 
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__TRMEISTER_OS_UNICODE_H
#define	__TRMEISTER_OS_UNICODE_H

#include "Os/Module.h"

#include "ModUnicodeString.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	TYPEDEF
//	Os::Ucs2 -- UCS2 における 1 文字を現す型
//
//	NOTES

typedef	ModUnicodeChar		Ucs2;

//	CLASS
//	Os::UnicodeString -- ユニコード文字列を表すクラス
//
//	NOTES
//		このクラスは const ModUnicodeString& から static_cast されるので、
//		メンバー変数および virtual 関数は一切定義してはいけない

class UnicodeString
	: public	ModUnicodeString
{
public:
	// デフォルトコンストラクター
	UnicodeString() {}
	// コンストラクター
	UnicodeString(const char* s)
		: ModUnicodeString(s, ModOs::Process::getEncodingType()) {}
	UnicodeString(const char* s, ModSize len)
		: ModUnicodeString(s, len, ModOs::Process::getEncodingType()) {}
	UnicodeString(const char* s, ModKanjiCode::KanjiCodeType code)
		: ModUnicodeString(s, code) {}
	UnicodeString(const char* s, ModSize len, ModKanjiCode::KanjiCodeType code)
		: ModUnicodeString(s, len, code) {}

	UnicodeString(const Ucs2* s) : ModUnicodeString(s) {}
	UnicodeString(const Ucs2* s, ModSize len) : ModUnicodeString(s, len) {}

	UnicodeString(char c) : ModUnicodeString(c) {}
	UnicodeString(Ucs2 c) : ModUnicodeString(c) {}

	UnicodeString(const ModUnicodeString& src) : ModUnicodeString(src) {}

	// const char* へのキャスト演算子
	SYD_OS_FUNCTION 
	operator				const char*() const;
//	ModUnicodeString::
//	// const Ucs2* へのキャスト演算子
//	operator				const Ucs2*() const;

	// ハッシュコードを取り出す
	SYD_OS_FUNCTION
	unsigned int			hashCode() const;
};

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_UNICODE_H

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
