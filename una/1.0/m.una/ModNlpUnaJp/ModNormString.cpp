// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNormString.cpp -- Unicode対応異表記正規化の文字列単位定義
// 
// Copyright (c) 2000-2005, 2008, 2012, 2023 Ricoh Company, Ltd.
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

#include "ModNlpUnaJp/ModNormString.h"
_UNA_USING
_UNA_UNAJP_USING

// FUNCTION
// ModNormString::ModNormString -- デフォルトコンストラクタ
//
// NOTES
//
// ARGUMENTS
// It is not
//
// RETURN
// It is not
//
// EXCEPTIONS
// It is not
//
ModNormString::ModNormString()
	: orig_str(0), orig_len(0), buf_len(0)
{
}
// FUNCTION
// ModNormString::~ModNormString -- デストラクタ
//
// NOTES
//
// ARGUMENTS
// It is not
//
// RETURN
// It is not
//
// EXCEPTIONS
// It is not
//
ModNormString::~ModNormString()
{
	delete [] orig_str;
}

// FUNCTION public
// ModNormString::clear -- 空文字列に相当する状態にする
//
// NOTES
//
// ARGUMENTS
// It is not
//
// RETURN
// It is not
//
// EXCEPTIONS
// It is not
//
void
ModNormString::clear()
{
	orig_len = 0;
}


// FUNCTION public
// ModNormString::operator = -- = 演算子
//
// NOTES
// 自分自身へ Unicode 文字を代入する
//
// ARGUMENTS
// ModUnicodeChar a_orig_char -- 自分自身へ代入するUnicode文字
//
// RETURN
// 代入後の自分自身
//
// EXCEPTIONS
// なし
//
ModNormString&
ModNormString::operator = (const ModUnicodeString& a_orig_str)
{
	orig_len = a_orig_str.getLength();
	if (buf_len < orig_len)
	{
		delete [] orig_str;
		buf_len = orig_len;
		orig_str = new ModNormChar[buf_len];
		; ModAssert (orig_str != 0);
	}

	// For accelerating, b_orig_str is utilized.
	const ModUnicodeChar* b_orig_str = a_orig_str;
	for (ModSize i = 0; i < orig_len; i++) {
		orig_str[i] = b_orig_str[i];
	}
	return *this;
}

//  FUNCTION public
//  ModNormString::getLength -- 保持する文字列の文字数を求める
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  求めた文字数
//
//  EXCEPTIONS
//  なし
//
ModSize
ModNormString::getLength() const
{
	return orig_len;
}
//
// Copyright (c) 2000-2005, 2008, 2012, 2023 RICOH Company, Ltd.
// All rights reserved.
//
