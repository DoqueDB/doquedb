// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4;	
//
// ModCommonInitialize.h -- 汎用ライブラリー初期化
// 
// Copyright (c) 1997, 1998, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __ModCommonInitialize_H__
#define __ModCommonInitialize_H__

#include "ModCommonDLL.h"
#include "ModTypes.h"
#include "ModCommonMutex.h"

//
// CLASS
// ModCommonInitialize -- 汎用ライブラリーの初期化を行なうクラス
//
// NOTES
// このクラスは汎用ライブラリーの初期化を必要なときに正しい順番で
// 行なうためのメソッドを提供するクラスである。

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModCommonInitialize
{
public:
	enum Status {
		novice,							// 初期化されてない
		initialized,					// 初期化された
		exited							// プログラムが終了した
	};

	static void checkAndInitialize();
	static void checkAndTerminate();
	static ModBoolean isInitialized();
	static ModBoolean isExited();		// プログラム終了

private:
	// 実際に初期化を行なう関数
	ModCommonDLL
	static void initialize();
	// 実際に終了処理を行なう関数
	ModCommonDLL
	static void terminate();
	// atexit で終了処理が行なう関数
	static void terminateAtExit();

	// 初期化されたかどうかを示すフラグ
	ModCommonDLL
	static Status status;
};

//
// FUNCTION
// ModCommonInitialize::checkAndInitialize -- 初期化を行なう
//
// NOTES
// この関数は汎用ライブラリーの関数が必要になったときにすべての
// 初期化を一度だけ行なうために用いる。
// 汎用ライブラリーのクラスのコンストラクタ、staticメソッドの最初に
// 呼ばれる必要がある。
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
ModCommonInitialize::checkAndInitialize()
{
	if (ModCommonInitialize::status == ModCommonInitialize::novice) {
		ModCommonMutex::lock();
		ModCommonInitialize::initialize();
		ModCommonMutex::unlock();
	}
}

//
// FUNCTION
// ModCommonInitialize::checkAndTerminate -- 終了処理を行なう
//
// NOTES
// この関数は汎用ライブラリーの関数の使用が終了したときに
// 後処理を行なうために用いる。
// ただし、すべての後処理はプログラム終了時に自動的に呼ばれるので、
// 特に必要がない限りこの関数を呼ぶ必要はない。
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
ModCommonInitialize::checkAndTerminate()
{
	if (ModCommonInitialize::status == ModCommonInitialize::initialized) {
		ModCommonMutex::lock();
		ModCommonInitialize::terminate();
		ModCommonMutex::unlock();
	}
}

//
// FUNCTION public
// ModCommonInitialize::isInitialized -- 初期化されたかを返す
//
// NOTES
// この関数は汎用ライブラリーが初期化されたかを示す。
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

// static
inline
ModBoolean
ModCommonInitialize::isInitialized()
{
	//【注意】	たんに参照するだけなので、
	//			汎用ライブラリーミューテックスをロックしない

	return (ModCommonInitialize::status == ModCommonInitialize::novice) ?
		ModFalse : ModTrue;
}

//
// FUNCTION public
// ModCommonInitialize::isExited -- プログラムが終了したかを返す
//
// NOTES
// この関数は汎用ライブラリーがプログラム終了に伴う後処理をされたかを示す。
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

// static
inline
ModBoolean
ModCommonInitialize::isExited()
{
	//【注意】	たんに参照するだけなので、
	//			汎用ライブラリーミューテックスをロックしない

	return (ModCommonInitialize::status == ModCommonInitialize::exited) ?
		ModTrue : ModFalse;
}

#endif	// __ModCommonInitialize_H__

//
// Copyright (c) 1997, 1998, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
