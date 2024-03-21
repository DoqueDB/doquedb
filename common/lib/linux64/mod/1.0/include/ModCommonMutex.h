// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModCommonMutex.h -- 汎用ライブラリー専用Mutex
// 
// Copyright (c) 1997, 1998, 2002, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __ModCommonMutex_H__
#define __ModCommonMutex_H__

#include "ModCommonDLL.h"
#include "ModOsMutex.h"

//
// CLASS
// ModCommonMutex -- 汎用ライブラリーの排他制御を行なうMutex
//
// NOTES
// このクラスは汎用ライブラリーの排他制御を一元管理するmutexを
// 提供するクラスである。
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModCommonMutex
{
#ifdef OBSOLETE
	friend class ModCommonMutexInitializer;
#endif
public:
	static void lock();
	static void unlock();
	ModCommonDLL
	static ModOsMutex* getMutex();
	static int getLockCount();

private:
	// インスタンスを作らないので宣言して実装しない
	ModCommonMutex();
	~ModCommonMutex();

	// ミューテックス
	static ModOsMutex mutex;
};

//
// FUNCTION
// ModCommonMutex::lock -- mutexをロックする
//
// NOTES
// この関数は汎用ライブラリーの関数が排他制御を行なうときに
// mutex をロックするのに用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位の例外をそのまま送出する
//
inline
void
ModCommonMutex::lock()
{
	getMutex()->lock();
}

//
// FUNCTION
// ModCommonMutex::unlock -- mutexをアンロックする
//
// NOTES
// この関数は汎用ライブラリーの関数が排他制御を行なうときに
// mutex をアンロックするのに用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位の例外をそのまま送出する
//
inline
void
ModCommonMutex::unlock()
{
	getMutex()->unlock();
}

//
// FUNCTION
// ModCommonMutex::getLockCount -- mutexのロック回数を得る
//
// NOTES
// この関数は汎用ライブラリーの関数が排他制御を行なうときに
// mutex の入れ子ロックが何回かかっているかを得るときに用いる
//
// ARGUMENTS
// なし
//
// RETURN
// 現在かかっている入れ子ロックの回数
//
// EXCEPTIONS
// 下位の例外をそのまま送出する
//
inline
int
ModCommonMutex::getLockCount()
{
	return getMutex()->getLockCount();
}

#ifdef OBSOLETE
//
// CLASS
// ModCommonMutexInitializer -- ModCommonMutexの初期化を行う
//
// NOTES
// ModCommonMutexを1回だけ初期化するために用いる。
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModCommonMutexInitializer
{
public:
	ModCommonDLL
	ModCommonMutexInitializer();
	ModCommonDLL
	~ModCommonMutexInitializer();

private:
	static int initializeCount;
};

static ModCommonMutexInitializer	commonMutexInitializer;
#endif

#endif	// __ModCommonMutex_H__

//
// Copyright (c) 1997, 1998, 2002, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
