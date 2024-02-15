// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Hasher.h -- ハッシュコードを取り出す
// 
// Copyright (c) 1999, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_HASHER_H
#define __TRMEISTER_COMMON_HASHER_H

#include "Common/Object.h"
#include "ModTypes.h"

_TRMEISTER_BEGIN

namespace Common
{

//
//	CLASS
//	Common::Hasher -- ハッシュコードを取り出す
//
//	NOTES
//	ハッシュコードを取り出すオペレータを含むクラス
//
class SYD_COMMON_FUNCTION Hasher : public Common::Object
{
public:
	//
	//	FUNCTION public
	//	operator() -- ハッシュコードを取り出すオペレータ
	//
	//	NOTES
	//	参照からハッシュコードを取り出すオペレータ。
	//
	//	ARGUMENTS
	//	const Common::Object& cKey_
	//		ハッシュコードを取り出すキー
	//
	//	RETURN
	//	ModSize
	//		ハッシュコード
	//
	//	EXCEPTIONS
	//	その他
	//		下位の例外はそのまま再送
	//
	ModSize operator()(const Common::Object& cKey_) const
	{
		return cKey_.hashCode();
	}

	//ポインターからハッシュコードを取り出すオペレータ
	ModSize operator()(const Common::Object* pKey_) const;
};

}

_TRMEISTER_END


#endif //__TRMEISTER_COMMON_HASER_H

//
//	Copyright (c) 1999, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

