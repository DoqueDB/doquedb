// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Object.cpp -- クライアントモジュール共通の基底クラスの関数定義
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Client2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Client2/Object.h"

#include "Os/CriticalSection.h"
#include "Os/AutoCriticalSection.h"

#include "ModHashMap.h"
#include "ModHasher.h"
#include "ModOsDriver.h"

_TRMEISTER_USING
_TRMEISTER_CLIENT2_USING

//	FUNCTION public
//	Client2::Object::Object -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Client2::Object::Type::Value	cType_
//		派生クラスのタイプ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

Object::Object(Type::Value eType_)
	: m_eType(eType_)
{
}

//	FUNCTION public
//	Client2::Object::~Object -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

Object::~Object()
{
}

//	FUNCTION public
//	Client2::Object::release -- メモリーを解放する
//
//	NOTES
//	DLL 間で new と delete が違っているとよろしくないので、
//	利用者にはこのメソッドを使用してメモリーを解放してもらう。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Object::release()
{
	delete this;
}

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
