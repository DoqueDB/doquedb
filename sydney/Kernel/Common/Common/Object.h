// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Object.h -- 共通クラス
// 
// Copyright (c) 1999, 2000, 2003, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_OBJECT_H
#define __TRMEISTER_COMMON_OBJECT_H

#include "Common/Module.h"

#include "ModMessage.h"
#include "ModTypes.h"
#include "ModUnicodeString.h"
#include "ModDefaultManager.h"
#include "ModOstream.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

// CLASS
// Common::Object -- 共通クラス
//
// NOTES
//		共通クラス。すべてのクラスはこのクラスのサブクラスとなる。

class Object
#ifndef PURIFY
	: public ModDefaultObject
#endif
{
public:
	//コンストラクタ
	Object();
	//デストラクタ
	virtual ~Object();

	//文字列で取り出す
	SYD_COMMON_FUNCTION
	virtual ModUnicodeString toString() const;
	//ハッシュコードを取り出す
	SYD_COMMON_FUNCTION
	virtual ModSize hashCode() const;

	// Common モジュールの初期化を行う
	static void
	initialize();
	// Common モジュールの後処理を行う
	static void
	terminate();

	//メッセージを書く
	SYD_COMMON_FUNCTION
	friend ModMessageStream& operator<<(ModMessageStream& cStream,
										const Common::Object& cObject);
	//メッセージを書く
	SYD_COMMON_FUNCTION
	friend ModOstream& operator<<(ModOstream& cStream,
								  const Common::Object& cObject);
};

//
//	FUNCTION public
//	Common::Object::Object -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
Common::Object::Object()
{
}

//
//	FUNCTION public
//	Common::Object::~Object -- デストラクタ
//
//	NOTES
//	デストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
Common::Object::~Object()
{
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_OBJECT_H

//
//	Copyright (c) 1999, 2000, 2003, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
