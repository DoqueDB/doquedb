// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNormChar.cpp -- Unicode対応異表記正規化の文字単位定義
// 
// Copyright (c) 2000-2005, 2023 Ricoh Company, Ltd.
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

#include "ModNlpUnaJp/ModNormChar.h"
_UNA_USING
_UNA_UNAJP_USING

// FUNCTION
// ModNormChar::ModNormChar -- デフォルトコンストラクタ
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
ModNormChar::ModNormChar()
	: orig_char(0)
{
	// 最大瞬間メモリ量を減らすために、バッファ領域の拡張単位を最小限(2バイト)
	// に設定する(Mod/FTS ノーツDB 00/5/14-22 参照)。他のコンストラクターでは、
	// メモリへの影響は少ないなので、設定しなくていい。

	repl_str.setAllocateStep(2);
}

// FUNCTION
// ModNormChar::ModNormChar -- ModUnicodeCharとModUnicodeStringを引数としたコンストラクタ
//
// NOTES
//
// ARGUMENTS
// const ModUnicodeChar a_orig_char   -- 自分のorig_charに挿入する
// const ModUnicodeString& a_repl_str -- 自分のrepl_strに挿入する
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModNormChar::ModNormChar(const ModUnicodeChar a_orig_char,
				 const ModUnicodeString& a_repl_str)
	: orig_char(a_orig_char), repl_str(a_repl_str)
{
}



// FUNCTION
// ModNormChar::~ModNormChar -- デストラクタ
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
ModNormChar::~ModNormChar()
{
	;
}

// FUNCTION public
// ModNormChar::operator = -- = 演算子
//
// NOTES
// 自分自身へのUnicode 文字を代入する
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
ModNormChar&
ModNormChar::operator = (const ModUnicodeChar a_orig_char)
{
	orig_char = a_orig_char;

	/// 次のclear()をしないと、約4%高速化ができた。
	/// 性能テストによれば、悪影響はないようですが、論理的に証明されていない。
	repl_str.clear (); ///fix 2 for speed; is this ok?? ===> No. This line is needed. Don't comment out!!

	return *this;
}

// FUNCTION
// ModNormSurrogateChar::ModNormSurrogateChar -- デフォルトコンストラクタ
//
// NOTES
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
ModNormSurrogateChar::ModNormSurrogateChar()
	: orig_high(0), orig_low(0)
{
	repl_str.setAllocateStep(2);
}

// FUNCTION
// ModNormSurrogateChar::ModNormSurrogateChar -- ModUnicodeChar 2つとModUnicodeStringを引数としたコンストラクタ
//
// NOTES
//
// ARGUMENTS
// const ModUnicodeChar a_orig_high   -- 自分のorig_highに挿入する
// const ModUnicodeChar a_orig_low    -- 自分のorig_lowに挿入する
// const ModUnicodeString& a_repl_str -- 自分のrepl_strに挿入する
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
ModNormSurrogateChar::ModNormSurrogateChar(const ModUnicodeChar a_orig_high,
				 const ModUnicodeChar a_orig_low,
				 const ModUnicodeString& a_repl_str)
	: orig_high(a_orig_high), orig_low(a_orig_low), repl_str(a_repl_str)
{
}

// FUNCTION
// ModNormSurrogateChar::~ModNormSurrogateChar -- デストラクタ
//
// NOTES
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
ModNormSurrogateChar::~ModNormSurrogateChar()
{
	;
}

//
// Copyright (c) 2000-2005, 2023 RICOH Company, Ltd.
// All rights reserved.
//
