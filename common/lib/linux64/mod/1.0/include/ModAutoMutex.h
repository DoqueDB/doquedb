// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModAutoMutex.h -- ModAutoMutex のクラス定義
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

#ifndef	__ModAutoMutex_H__
#define __ModAutoMutex_H__

#include "ModCommonDLL.h"
#include "ModOsDriver.h"

//	CLASS
//	ModAutoMutexBase -- ModAutoMutex の親クラス
//
//	NOTES
//		ModAutoMutex::throwTooManyUnlocked の実装で ModThread.h を
//		必要とするが、このインクルードファイルにインクルードしたくないため、
//		このメソッドの実装を ModAutoMutex.cpp へ分けるために
//		このクラスを定義した

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModAutoMutexBase
{
public:
	void*					operator new(size_t size);
												// 領域を確保する
												// ただし、領域獲得失敗時に
												// 例外を発生する
protected:
	ModCommonDLL
	void					throwTooManyUnlocked();
												// アンロックしすぎを表す
												// 例外を発生する
};

//	FUNCTION 
//	ModAutoMutexBase::operator new -- 領域を確保する
//
//	NOTES
//		operator ::new で領域を確保する
//
//	ARGUMENTS
//		size_t		size
//			確保する領域のサイズ(B 単位)
//
//	RETURN
//		確保した領域の先頭アドレス
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			システムのメモリが確保できない

inline
void*
ModAutoMutexBase::operator new(size_t size)
{
	return ModOsDriver::newSetHandler((ModSize)size);
}

//	TEMPLATE CLASS
//	ModAutoMutex --
//		スコープ離脱時自動ロック解除ミューテックス機能を提供する
//		テンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class Mutex
//			自動ロック解除機能を提供するミューテックス
//			ModCriticalSection、ModMutex や
//			仮想 OS のミューテックスクラスなどを想定している
//
//	NOTES
//		このオブジェクトのコンストラクト時にロック数をおぼえておき、
//		デストラクト時にそのロック数と等しくなるまでアンロックする
//
//		このクラスの自動変数を定義し、スコープを抜けたときに
//		定義してからかけたロックを自動的にはずすように、
//		例外処理に使用すると便利である

template <class Mutex>
class ModAutoMutex
	: public	ModAutoMutexBase
{
public:
	ModAutoMutex(Mutex*);						// コンストラクター
	~ModAutoMutex();							// デストラクター

	int						lock(unsigned int times = 1);
												// ロックする
	int						trylock(unsigned int times = 1);
												// ロックを試みる
	void					unlock();			// ロックをはずす
	void					unlockAll();		// ロックをすべてはずす

	ModBoolean				isSelfLocked() const;
												// ロックしているか
private:
	Mutex*					_mutex;				// 操作するミューテックス
	int						_count;				// ロック数
												// 一時的に負数になるかも
};

//	TEMPLATE FUNCTION public
//	ModAutoMutex<Mutex>::ModAutoMutex --
//		スコープ離脱時自動ロック解除ミューテックス機能を
//		提供するクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class Mutex
//			自動ロック解除機能を提供するミューテックス
//
//	NOTES
//
//	ARGUMENTS
//		Mutex*				mutex
//			操作するミューテックス
// 
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

template <class Mutex>
inline
ModAutoMutex<Mutex>::ModAutoMutex(Mutex* mutex)
{
	_mutex = mutex;
	_count = 0;
}

//	TEMPLATE FUNCTION public
//	ModAutoMutex<Mutex>::~ModAutoMutex --
//		スコープ離脱時自動ロック解除ミューテックス機能を
//		提供するクラスのデストラクター
//
//	TEMPLATE ARGUMENTS
//		class Mutex
//			自動ロック解除機能を提供するミューテックス
//
//	NOTES
//		コンストラクト時のロック数と等しくなるように、
//		ロックをはずす
//
//	ARGUMENTS
//		なし
// 
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorTooUnlocked
//			コンストラクト時よりも多くロックをはずしすぎている

template <class Mutex>
inline
ModAutoMutex<Mutex>::~ModAutoMutex()
{
	if (_count > 0)
		_mutex->unlock(_count), _count = 0;
	else if (_count < 0)

		// コンストラクト時よりもロックをはずしすぎている

		ModAutoMutexBase::throwTooManyUnlocked();
}

//	TEMPLATE FUNCTION public
//	ModAutoMutex<Mutex>::lock --
//		スコープ離脱時自動ロック解除ミューテックスをロックする
//
//	TEMPLATE ARGUMENTS
//		class Mutex
//			自動ロック解除機能を提供するミューテックス
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		times
//			指定されたとき
//				ロックする回数
//			指定されないとき
//				1 が指定されたものみなす
// 
//	RETURN
//		現在のロック数(コンストラクトしてからのロック数でない)
//
//	EXCEPTIONS

template <class Mutex> 
inline
int
ModAutoMutex<Mutex>::lock(unsigned int times)
{
	int	n = _mutex->lock(times);
	_count += times;
	return n;
}

//	TEMPLATE FUNCTION public
//	ModAutoMutex<Mutex>::trylock --
//		スコープ離脱時自動ロック解除ミューテックスのロックを試みる
//
//	TEMPLATE ARGUMENTS
//		class Mutex
//			自動ロック解除機能を提供するミューテックス
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		times
//			指定されたとき
//				ロックする回数
//			指定されないとき
//				1 が指定されたものみなす
// 
//	RETURN
//		現在のロック数(コンストラクトしてからのロック数でない)
//
//	EXCEPTIONS

template <class Mutex> 
inline
int
ModAutoMutex<Mutex>::trylock(unsigned int times)
{
	int n = _mutex->trylock(times);
	if (n > 0)
		_count += times;
	return n;
}

//	TEMPLATE FUNCTION public
//	ModAutoMutex<Mutex>::unlock --
//		スコープ離脱時自動ロック解除ミューテックスをアンロックする
//
//	TEMPLATE ARGUMENTS
//		class Mutex
//			自動ロック解除機能を提供するミューテックス
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

template <class Mutex> 
inline
void
ModAutoMutex<Mutex>::unlock()
{
	_mutex->unlock();
	_count--;
}

//	TEMPLATE FUNCTION public
//	ModAutoMutex<Mutex>::unlockAll --
//		スコープ離脱時自動ロック解除ミューテックスのロックをすべてはずす
//
//	TEMPLATE ARGUMENTS
//		class Mutex
//			自動ロック解除機能を提供するミューテックス
//
//	NOTES
//		ロックをすべてはずした結果、
//		コンストラクト時よりも多くアンロックされるときがある
//		そのままデストラクトすると、例外が発生する
//
//	ARGUMENTS
//		なし
// 
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class Mutex> 
inline
void
ModAutoMutex<Mutex>::unlockAll()
{
	int	n = _mutex->getLockCount();
	_mutex->unlockAll();
	_count -= n;
}

//	TEMPLATE FUNCTION public
//	ModAutoMutex<Mutex>::isSelfLocked -- 呼び出したスレッドがロックしているか
//
//	TEMPLATE ARGUMENTS
//		class Mutex
//			自動ロック解除機能を提供するミューテックス
//
//	NOTES
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

template <class Mutex> 
inline
ModBoolean
ModAutoMutex<Mutex>::isSelfLocked() const
{
	return _mutex->isSelfLocked();
}

#endif	// __ModAutoMutex_H__

//
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
