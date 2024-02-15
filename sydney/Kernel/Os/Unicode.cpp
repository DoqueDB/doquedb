// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Unicode.cpp -- ユニコード関連の関数定義
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Os";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Os/Unicode.h"

_TRMEISTER_USING
_TRMEISTER_OS_USING

namespace
{
}

//	FUNCTION public
//	Os::UnicodeString::operator const char* -- const char* へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Os の内部コードへの文字列を得る
//
//	EXCEPTIONS

UnicodeString::operator const char*() const
{
	//【注意】	OS の内部コードはタイミングによってはうまく動作しない
	//			ModOs::Process::getEncodingType に頼らずに
	//			自分でなんとかして取得すべき

	return const_cast<UnicodeString*>(this)->getString(
#ifdef SYD_OS_WINDOWS
		ModKanjiCode::shiftJis
#else
		ModOs::Process::getEncodingType()
#endif
		);
}

//	FUNCTION public
//	Os::UnicodeString::hashCode -- ハッシュコードを計算する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたハッシュコード
//
//	EXCEPTIONS
//		なし

unsigned int
UnicodeString::hashCode() const
{
	const Ucs2* p = *this;
	const Ucs2* q = getTail();

	unsigned int i = 0;
	for (; q >= p; --q)
		i += static_cast<unsigned int>(*q);

	return i;
}

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
