// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModCommonMutex.c -- 汎用ライブラリー専用mutex
// 
// Copyright (c) 1997, 1998, 2005, 2023 Ricoh Company, Ltd.
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

// ident情報

#include "ModCommonMutex.h"

//
// VARIABLE private
// ModCommonMutex::mutex -- 汎用ライブラリー専用mutex
//
// NOTES
// この変数は汎用ライブラリーの排他制御に使うmutexである。
//
ModOsMutex ModCommonMutex::mutex;

#ifdef OBSOLETE
//
// VARIABLE private
// ModCommonMutexInitializer::initializeCount -- 初期化カウント
//
// NOTES
// ModCommonMutex の初期化が1回だけなされていることを保証する。
//
int ModCommonMutexInitializer::initializeCount = 0;
#endif

//
// FUNCTION
// ModCommonMutex::getMutex -- mutex へのポインタを得る
//
// NOTES
// この関数は汎用ライブラリーの関数が排他制御を行なうときに使う
// mutex へのポインタを得るのに用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// mutex へのポインタを返す。
//
// EXCEPTIONS
// なし
//
ModOsMutex*
ModCommonMutex::getMutex()
{
	return &ModCommonMutex::mutex;
}

#ifdef OBSOLETE
//
// FUNCTION
// ModCommonMutexInitializer::ModCommonMutexInitializer -- コンストラクタ
//
// NOTES
// この関数は ModCommonMutex.h をインクルードしているファイルで呼ばれ、
// ModCommonMutex が1回だけ初期化されることを保証するために用いる。
//
// ARGUMENTS
// なし
//
// RETURN VALUE
// なし
//
// EXCEPTIONS
// なし
//
ModCommonMutexInitializer::ModCommonMutexInitializer()
{
}

//
// FUNCTION
// ModCommonMutexInitializer::~ModCommonMutexInitializer -- デストラクタ
//
// NOTES
// この関数は ModCommonMutex.h をインクルードしているファイルで呼ばれ、
// 最後に1回だけ後処理することを保証する。
//
// ARGUMENTS
// なし
//
// RETURN VALUE
// なし
//
// EXCEPTIONS
// なし
//
ModCommonMutexInitializer::~ModCommonMutexInitializer()
{
}
#endif

//
// Copyright (c) 1997, 1998, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

