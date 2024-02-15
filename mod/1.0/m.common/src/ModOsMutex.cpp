// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModOsMutex.cpp -- 簡易版ミューテックス関連のメソッド定義
// 
// Copyright (c) 1997, 1999, 2005, 2023 Ricoh Company, Ltd.
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


#include "ModOsMutex.h"
#include "ModOsException.h"
#include "ModAssert.h"

//	FUNCTION public
//	ModOsMutex::~ModOsMutex -- 簡易版ミューテックスを表すクラスのデストラクター
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
//		ModOsErrorStillLocked
//			ロックされたままである

ModOsMutex::~ModOsMutex()
{
#if 0 // (2005/12/08)
	  // ModCommonMutexでも使用されており、終了時に例外が投げられると
	  // アプリケーションエラーになってしまうため、アサートにする
	if (_count)

		// ロックされたままである
		//
		//【注意】	ロックをはずしたいが、
		//			自分がかけたものとは限らないのでアンロックできない

		ModMessageThrowOsWarning(ModOsErrorStillLocked);
#else
	; ModAssert(_count == 0);
#endif
}

//	FUNCTION public
//	ModOsMutex::lock -- 簡易版ミューテックスをロックする
//
//	NOTES
//		他のスレッドがロックしていれば、
//		そのスレッドがロックをすべてはずすまで、
//		ロック待ちする(つまり、呼び出しから返らない)
//
//	ARGUMENTS
//		unsigned int		times
//			指定されたとき
//				ロックする回数
//			指定されないとき
//				1 が指定されたものみなす
//
//	RETURN
//		現在のロック数
//
//	EXCEPTIONS
//		なし

int
ModOsMutex::lock(unsigned int times)
{
	ModThreadId self = ModOsDriver::Thread::self();
	if (_count && _locker == self)

		// 自分がロックしている
		//
		//【注意】	ロックしているスレッドのスレッド ID は
		//			ロックしていなければ設定されていないので、
		//			まず、ロックされていることを調べる必要がある

		return _count += times;

	// 他のスレッドがロックしているか、誰もロックしていないか
	// ここで待つかもしれない

	_mutex.lock();

	// 自分がロックした

	_locker = self;
	return _count = times;
}

//	FUNCTION public
//	ModOsMutex::trylock -- 簡易版ミューテックスのロックを試みる
//
//	NOTES
//		他のスレッドがロックしていれば、
//		ロック待ちせずに、すぐ呼び出しから返る
//
//	ARGUMENTS
//		unsigned int		times
//			指定されたとき
//				ロックする回数
//			指定されないとき
//				1 が指定されたものみなす
//
//	RETURN
//		0 より大きい値
//			現在のロック数
//		0
//			ロックできなかった
//
//	EXCEPTIONS
//		なし

int
ModOsMutex::trylock(unsigned int times)
{
	ModThreadId self = ModOsDriver::Thread::self();
	if (_count && _locker == self)

		// 自分がロックしている
		//
		//【注意】	ロックしているスレッドのスレッド ID は
		//			ロックしていなければ設定されていないので、
		//			まず、ロックされていることを調べる必要がある

		return _count += times;

	if (_mutex.trylock() == ModFalse)
		return 0;

	// 自分がロックした

	_locker = self;
	return _count = times;
}

//	FUNCTION
//	ModOsMutex::unlock -- 簡易版ミューテックスをアンロックする
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		times
//			指定されたとき
//				アンロックする回数
//			指定されないとき
//				1 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorNotLocked
//			ロックしていないのにアンロックしようとしている

void
ModOsMutex::unlock(unsigned int times)
{
	if (_count < (int) times || _locker != ModOsDriver::Thread::self())

		// ロックしていないのにアンロックしようとしている
		// または、ロックしている以上にはずそうとしている
		//
		//【注意】	ロックしているスレッドのスレッド ID は
		//			ロックしていなければ設定されていないので、
		//			まず、ロックされていることを調べる必要がある

		ModMessageThrowOsError(ModOsErrorNotLocked);

	// 自分がロックしている

	if (!(_count -= times))

		// 最後のロックだったら実際にロック解除

		_mutex.unlock();
}

//	FUNCTION public
//	ModOsMutex::unlockAll -- 簡易版ミューテックスのロックをすべてはずす
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
//		ModOsErrorNotLocked
//			ロックしていないのにアンロックしようとしている

void
ModOsMutex::unlockAll()
{
	if (_count <= 0 || _locker != ModOsDriver::Thread::self())

		// 自分がロックしていないのに、ロックをはずそうとしている
		// または、ロック数がおかしい
		//
		//【注意】	ロックしているスレッドのスレッド ID は
		//			ロックしていなければ設定されていないので、
		//			まず、ロックされていることを調べる必要がある

		ModMessageThrowOsError(ModOsErrorNotLocked);

	// 自分がロックしている

	_count = 0;
	_mutex.unlock();
}

//
// Copyright (c) 1997, 1999, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
