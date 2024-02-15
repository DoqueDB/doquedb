// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNormString.h -- ModNormString のクラス定義
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
#ifndef	__ModNormString_H_
#define __ModNormString_H_

#include "ModCommonDLL.h"
#include "ModNlpUnaJp/ModNormDLL.h"
#include "ModNlpUnaJp/ModNormChar.h"
#include "ModNlpUnaJp/Module.h"
_UNA_BEGIN
_UNA_UNAJP_BEGIN

//
// CLASS
// ModNormString -- Unicode 正規化文字列クラスの定義 
//
// NOTES
//


class ModNormDLL ModNormString : public ModDefaultObject
{
public:

	ModNormChar*		orig_str;   // UNA参照前の文字列

	// コンストラクタ
	ModNormString();
	ModNormString(const ModUnicodeChar*   a_orig_str);
	ModNormString(const ModUnicodeString& a_orig_str);

	// コンストラクタ
	~ModNormString();
	void clear();

	// 演算オペレータ
	ModNormString&	operator = (const ModUnicodeChar	a_orig_char);
	ModNormString&	operator = (const ModUnicodeChar*   a_orig_str);
	ModNormString&	operator = (const ModUnicodeString& a_orig_str);
	ModSize		getLength() const;

private:
	ModSize 	orig_len;
	ModSize		buf_len;

};
_UNA_UNAJP_END
_UNA_END
#endif // __ModNormString_H_
//
// Copyright (c) 2000-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
