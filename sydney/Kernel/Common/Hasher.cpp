// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Hasher.cpp -- ハッシュコードを取り出す
// 
// Copyright (c) 1999, 2001, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Common";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/Hasher.h"

#include "Exception/BadArgument.h"

_TRMEISTER_USING

//
//	FUNCTION public
//	operator() -- ポインターからハッシュコードを得る
//
//	NOTES
//	ポインタのキーからハッシュコードを得る。
//
//	ARGUMENTS
//	const Common::Object* pKey_
//		ハッシュコードを取り出すキーへのポインタ
//
//	RETURN
//	ModSize
//		ハッシュコード
//
//	EXCEPTIONS
//	Exception::BadArgument
//		引数がnullである。
//	その他
//		下位の例外はそのまま再送
//
ModSize
Common::Hasher::operator()(const Common::Object* pKey_) const
{
	if (pKey_ == 0)
	{
		//引数がnullなので例外を投げる
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}
	return pKey_->hashCode();
}

//
//	Copyright (c) 1999, 2001, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
