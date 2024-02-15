// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNormChar.h -- ModNormChar のクラス定義
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
#ifndef	__ModNormChar_H_
#define __ModNormChar_H_

#include <string.h>
#include "ModCommonDLL.h"
#include "ModUnicodeString.h"
#include "ModNlpUnaJp/ModNormDLL.h"

#include "ModNlpUnaJp/Module.h"
_UNA_BEGIN
_UNA_UNAJP_BEGIN

//
// CLASS
// ModNormChar -- Unicode 正規化文字クラスの定義
//
// NOTES
//
class ModNormDLL ModNormChar : public ModDefaultObject
{
public:
	ModUnicodeChar		orig_char;      // 原表記
	ModUnicodeString	repl_str;       // 正規化表記(cjp 参照前)

	// コンストラクタ
	ModNormChar();
	ModNormChar(const ModUnicodeChar a_orig_char,
				const ModUnicodeString& a_repl_str);
	// デストラクタ
	~ModNormChar();

	// 演算オペレータ
	ModNormChar& 	operator = (const ModUnicodeChar a_orig_char);
};

//
// CLASS
// ModNormSurrogateChar -- Unicode 正規化サロゲート文字クラスの定義
//
// NOTES
//
class ModNormDLL ModNormSurrogateChar : public ModDefaultObject
{
public:
	ModUnicodeChar		orig_high;      // 原表記ハイサロゲート
	ModUnicodeChar		orig_low;       // 原表記ローサロゲート
	ModUnicodeString	repl_str;       // 正規化表記

	// コンストラクタ
	ModNormSurrogateChar();
	ModNormSurrogateChar(const ModUnicodeChar a_orig_high,
	                     const ModUnicodeChar a_orig_low,
	                     const ModUnicodeString& a_repl_str);
	// デストラクタ
	~ModNormSurrogateChar();
};
_UNA_UNAJP_END
_UNA_END
#endif // __ModNormChar_H_
//
// Copyright (c) 2000-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
