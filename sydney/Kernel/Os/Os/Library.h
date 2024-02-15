// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Library.h -- ライブラリ関連のクラス定義、関数宣言
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

#ifndef __TRMEISTER_OS_LIBRARY_H
#define __TRMEISTER_OS_LIBRARY_H

#if !defined(SYD_OS_WINDOWS) && !defined(SYD_OS_POSIX)
#error require #include "SyDefault.h"
#endif

#include "Os/Module.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

class Path;
class UnicodeString;

//	NAMESPACE
//	Os::Library -- ライブラリ関連の名前空間
//
//	NOTES

namespace Library
{
	// ライブラリをロードする
	SYD_OS_FUNCTION
	void
	load(const UnicodeString& baseName);

	// 関数のポインタを得る
	SYD_OS_FUNCTION
	void*
	getFunction(const UnicodeString& baseName, const UnicodeString& funcName);
}

_TRMEISTER_OS_END
_TRMEISTER_END

#endif //__TRMEISTER_OS_LIBRARY_H

//
//	Copyright (c) 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

