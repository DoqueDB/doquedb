// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModOsMutex.h -- 簡易版ミューテックス関連のクラス定義
// 
// Copyright (c) 1997, 1999, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModOsMutex_H__
#define __ModOsMutex_H__

#include "ModCommonDLL.h"
#include "ModTypes.h"
#include "ModOsDriver.h"

//	CLASS
//	MosOsMutex -- 簡易版ミューテックスを表すクラス
//
//	NOTES
//		仮想 OS のクリティカルセクションである
//		ModOsDriver::CriticalSection に、
//		ロックしているスレッドが複数回ロックできる機能を加えた
//		WIN32API のクリティカルセクションはこの機能を持つが、
//		POSIX のものは持たない
//
//		以前は ModOsDriver::Mutex を使って実装していたが、
//		WIN32API では ModOsDriver::Mutex が遅く、また、
//		異るプロセスのスレッド間の排他制御は必要としていないので、
//		ModOsDriver::CriticalSection を使用することにした
//		名前は、互換性のためにそのままにしている

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModOsMutex
{
public:
	ModOsMutex();								// コンストラクター
	ModCommonDLL
#ifdef STD_CPP11
	~ModOsMutex() noexcept(false);				// デストラクター
#else
	~ModOsMutex();								// デストラクター
#endif

	ModCommonDLL
	int						lock(unsigned int times = 1);
												// ロックする
	ModCommonDLL
	int						trylock(unsigned int times = 1);
												// ロックを試みる
	ModCommonDLL
	void					unlock(unsigned int times = 1);
												// ロックをはずす
	ModCommonDLL
	void					unlockAll();		// すべてのロックをはずす

	int						getLockCount() const;
												// ロック数を得る
	ModBoolean				isSelfLocked() const;
												// ロックしているか
private:
	ModOsDriver::CriticalSection _mutex;		// 仮想 OS の
												// クリティカルセクション

	ModThreadId				_locker;			// ロック中のスレッドの
												// スレッド ID
	int						_count;				// ロックの回数
};

//	FUNCTION public
//	ModOsMutex::ModOsMutex --
//		簡易版ミューテックスを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
ModOsMutex::ModOsMutex()
	: _count(0)
{ }

//	FUNCTION public
//	ModOsMutex::getLockCount -- ロック数を得る
//
//	NOTES
//		ミューテックスにいくつロックがかかっているか調べる
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		かかっているロック数
//
//	EXCEPTIONS
//		なし

inline
int
ModOsMutex::getLockCount() const
{
	return _count;
}

//	FUNCTION public
//	ModOsMutex::isSelfLocked -- 呼び出したスレッドがロックしているか
//
//	NOTES
//		この関数を呼び出したスレッドがミューテックスをロックしているか調べる
// 
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			ロックしている
//		ModFalse
//			ロックしていない
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModOsMutex::isSelfLocked() const
{
	return (_count && _locker == ModOsDriver::Thread::self()) ?
	  ModTrue : ModFalse;
}

#endif	// __ModOsMutex_H__

//
// Copyright (c) 1997, 1999, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
