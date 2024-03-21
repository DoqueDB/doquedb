// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModMutex.h -- ミューテックス機能を提供するクラス定義
// 
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModMutex_H__
#define __ModMutex_H__

#include "ModSyncBase.h"
#include "ModOsDriver.h"
#include "ModAutoMutex.h"
#include "ModDefaultManager.h"

//	CLASS
//	ModPureMutex -- ミューテックス機能を提供するクラス
//
//	NOTES
//		ミューテックス機能を提供するクラスである
//		実際は、ユーザーはこのクラスの子クラスであり、
//		メモリーハンドルが明示されている ModMutex を利用する
//
//		同じスレッドが続けて入れ子のようにロック可能である
//		その場合、獲得した回数だけアンロックするまで、
//		ロックを保持することになる
//
//		誰かが所有権を保持したままデストラクトすると、
//		ModSyncBase::~ModSyncBase で ModOsErrorStillLocked の例外が発生する
//
//		ModOsMutex とはメモリー管理の対象であり、
//		デッドロック検査の対象となる点が異なる
//
//		現在、ModDetectDeadLock は異プロセススレッド間の
//		デッドロックの検証をできないにもかかわらず
//		将来を見越してこのクラスを ModSyncBase の派生クラスとしている

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModPureMutex
	: public	ModSyncBase
{
public:
	// コンストラクター
	ModPureMutex(ModBoolean unlockKilled = ModFalse);
	// デストラクター
	~ModPureMutex()
	{}

	void					lockReally();		// 実際にロックする
	ModBoolean				trylockReally();	// 実際にロックを試みる
	void					unlockReally();		// 実際にアンロックする
private:
	ModOsDriver::Mutex		_mutex;				// 仮想 OS のミューテックス
};

//	FUNCTION public
//	ModPureMutex::ModPureMutex --
//		ミューテックス機能を提供するクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		ModBoolean			unlockKilled
//			ModTrue 
//				ミューテックスをロックしているスレッドが
//				中断されたとき、それの保持しているロックを自動的にはずす
//			ModFalse または指定されないとき
//				ミューテックスをロックしているスレッドが
//				中断されたとき、それの保持しているロックはそのままになる
//				
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ModPureMutex::ModPureMutex(ModBoolean unlockKilled)
	: ModSyncBase(unlockKilled)
{ }

//	FUNCTION public
//	ModPureMutex::lockReally -- ミューテックスを実際にロックする
//
//	NOTES
//		ModSyncBase::lock から本関数が呼び出される
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
ModPureMutex::lockReally()
{
	_mutex.lock();
}

//	FUNCTION public
//	ModPureMutex::trylockReally -- ミューテックスを実際にロックを試みる
//
//	NOTES
//		ModSyncBase::trylock から本関数が呼び出される
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			ロックできた
//		ModFalse
//			ロックできなかった
//
//	EXCEPTIONS

inline
ModBoolean
ModPureMutex::trylockReally()
{
	return _mutex.trylock();
}

//	FUNCTION public
//	ModPureMutex::unlockReally -- ミューテックスを実際にアンロックする
//
//	NOTES
//		ModSyncBase::unlock から本関数が呼び出される
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
ModPureMutex::unlockReally()
{
	_mutex.unlock();
}

//	TYPEDEF
//	ModSmartMutex -- スコープ離脱時自動解除機能付きミューテックスを表すクラス
//
//	NOTES
//		デストラクトされると自動的に
//		その時のロックが全てはずれるミューテックス

typedef	ModAutoMutex<ModPureMutex>		ModSmartMutex;

//	CLASS
//	ModMutex -- ModPureMutex クラスのメモリーハンドル明示クラス
//
//	NOTES
//		デフォルトメモリーハンドルの管理化で
//		ModPureMutex クラスを利用するためのクラス
//		通常、ユーザーは本クラスを利用する

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModMutex
	: public	ModObject<ModDefaultManager>,
	  public	ModPureMutex
{
public:
	// コンストラクター
	ModMutex()
	{}
	// デストラクター
	~ModMutex()
	{}
};

#endif	// __ModMutex_H__

//
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
