// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModCriticalSection.h -- クリティカルセクション機能を提供するクラス定義
// 
// Copyright (c) 1998, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModCriticalSection_H__
#define __ModCriticalSection_H__

#include "ModSyncBase.h"
#include "ModOsDriver.h"
#include "ModAutoMutex.h"
#include "ModDefaultManager.h"

//	CLASS
//	ModPureCriticalSection -- クリティカルセクション機能を提供するクラス
//
//	NOTES
//		クリティカルセクション機能を提供するクラスである
//		実際は、ユーザーはこのクラスの子クラスであり、
//		メモリーハンドルが明示されている ModCriticalSection を利用する
//
//		同じスレッドが続けて入れ子のように所有権を獲得可能である
//		その場合、獲得した回数だけ放棄するまで、
//		所有権を保持することになる
//
//		誰かが所有権を保持したままデストラクトすると、
//		ModSyncBase::~ModSyncBase で ModOsErrorStillLocked の例外が発生する

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModPureCriticalSection
	: public	ModSyncBase
{
public:
	// コンストラクター
	ModPureCriticalSection(ModBoolean unlockKilled = ModFalse);
	// デストラクター
	~ModPureCriticalSection()
	{}

	void					lockReally();		// 実際に所有権を獲得する
	ModBoolean				trylockReally();	// 実際に所有権の獲得を試みる
	void					unlockReally();		// 実際に所有権を放棄する
private:
	ModOsDriver::CriticalSection _criticalSection;
												// 仮想 OS の
												// クリティカルセクション
};

//	FUNCTION public
//	ModPureCriticalSection::ModPureCriticalSection --
//		ミューテックス機能を提供するクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		ModBoolean			unlockKilled
//			ModTrue 
//				クリティカルセクションの所有権を獲得しているスレッドが
//				中断されたとき、それの獲得している所有権を自動的に放棄する
//			ModFalse または指定されないとき
//				クリティカルセクションの所有権を獲得しているスレッドが
//				中断されたとき、それの獲得している所有権はそのままになる
//				
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ModPureCriticalSection::ModPureCriticalSection(ModBoolean unlockKilled)
	: ModSyncBase(unlockKilled)
{ }

//	FUNCTION public
//	ModPureCriticalSection::lockReally --
//		クリティカルセクションの所有権を実際に獲得する
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
ModPureCriticalSection::lockReally()
{
	_criticalSection.lock();
}

//	FUNCTION public
//	ModPureCriticalSection::trylockReally --
//		クリティカルセクションの所有権の獲得を実際に試みる
//
//	NOTES
//		ModSyncBase::trylock から本関数が呼び出される
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			所有権を獲得した
//		ModFalse
//			所有権を獲得できなかった
//
//	EXCEPTIONS

inline
ModBoolean
ModPureCriticalSection::trylockReally()
{
	return _criticalSection.trylock();
}

//	FUNCTION public
//	ModPureCriticalSection::unlockReally --
//		クリティカルセクションの所有権を実際に放棄する
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
ModPureCriticalSection::unlockReally()
{
	_criticalSection.unlock();
}

//	TYPEDEF
//	ModSmartCriticalSection --
//		スコープ離脱時自動解除機能付きクリティカルセクションを表すクラス
//
//	NOTES
//		デストラクトされると自動的に
//		その時の所有権を全て放棄するクリティカルセクション

typedef	ModAutoMutex<ModPureCriticalSection>	ModSmartCriticalSection;

//	CLASS
//	ModCriticalSection --
//		ModPureCriticalSection クラスのメモリーハンドル明示クラス
//
//	NOTES
//		デフォルトメモリーハンドルの管理化で
//		ModPureCriticalSection クラスを利用するためのクラス
//		通常、ユーザーは本クラスを利用する

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModCriticalSection
	: public	ModObject<ModDefaultManager>,
	  public	ModPureCriticalSection
{
public:
	// コンストラクター
	ModCriticalSection()
	{}
	// デストラクター
	~ModCriticalSection()
	{}
};

#endif	// __ModCriticalSection_H__

//
// Copyright (c) 1998, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
