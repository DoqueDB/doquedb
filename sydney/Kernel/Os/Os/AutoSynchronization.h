// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoSynchronization.h --
//		自動同期オブジェクト関連のテンプレートクラス定義、関数宣言
// 
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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

#ifndef	__TRMEISTER_OS_AUTOSYNCHRONIZATION_H
#define	__TRMEISTER_OS_AUTOSYNCHRONIZATION_H

#include "Os/Module.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	TEMPLATE CLASS
//	Os::AutoSynchronization --
//		オブジェクト生成時に自動的に同期オブジェクトをロックし、
//		破棄時に自動的にロックをはずすテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class SyncObj
//			操作する同期オブジェクト
//
//	NOTES

template <class SyncObj>
class AutoSynchronization
{
public:
	// コンストラクター
	AutoSynchronization(SyncObj& object);
	// デストラクター
	~AutoSynchronization();
private:
	// 操作する同期オブジェクト
	SyncObj&				_object;
};

//	TEMPLATE CLASS
//	Os::AutoTrySynchronization --
//		オブジェクト生成時に自動的に同期オブジェクトをロックを試み、
//		破棄時にロックしていれば、自動的にはずすテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class SyncObj
//			操作する同期オブジェクト
//
//	NOTES

template <class SyncObj>
class AutoTrySynchronization
{
public:
	// コンストラクター
	AutoTrySynchronization(SyncObj& object, bool immediately = true);
	// デストラクター
	~AutoTrySynchronization();

	// ロックする
	void					lock();
	// ロックを試みる
	bool					trylock();
	// ロックをはずす
	void					unlock();
	// ロックしているか
	bool					isLocked() const;
private:
	// 操作する同期オブジェクト
	SyncObj&				_object;
	// ロックしているか
	bool					_locked;
};

//	TEMPLATE FUNCTION public
//	Os::AutoSynchronization<SyncObj>::AutoSynchronization --
//		オブジェクト生成時に自動的に同期オブジェクトをロックし、
//		破棄時に自動的にロックをはずすテンプレートクラスのコンストラクター
//
//	TEMPLATE ARGUMENTS
//		class SyncObj
//			操作する同期オブジェクト
//
//	NOTES
//
//	ARGUMENTS
//		SyncObj&			object
//			操作する同期オブジェクトのリファレンス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class SyncObj>
inline
AutoSynchronization<SyncObj>::AutoSynchronization(SyncObj& object)
	: _object(object)
{
	_object.lock();
}

//	TEMPLATE FUNCTION public
//	Os::AutoSynchronization<SyncObj>::~AutoSynchronization --
//		オブジェクト生成時に自動的に同期オブジェクトをロックし、
//		破棄時に自動的にロックをはずすテンプレートクラスのデストラクター
//
//	TEMPLATE ARGUMENTS
//		class SyncObj
//			操作する同期オブジェクト
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

template <class SyncObj>
inline
AutoSynchronization<SyncObj>::~AutoSynchronization()
{
	_object.unlock();
}

//	TEMPLATE FUNCTION public
//	Os::AutoTrySynchronization<SyncObj>::AutoTrySynchronization --
//		オブジェクト生成時に自動的に同期オブジェクトをロックを試み、
//		破棄時にロックしていれば、自動的にはずすテンプレートクラスの
//		コンストラクター
//
//	TEMPLATE ARGUMENTS
//		class SyncObj
//			操作する同期オブジェクト
//
//	NOTES
//
//	ARGUMENTS
//		SyncObj&			object
//			操作する同期オブジェクトのリファレンス
//		bool				immediately
//			true または指定されないとき
//				オブジェクトの生成とともにロックを試みる
//			false
//				ロックは後で試みることにする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

template <class SyncObj>
inline
AutoTrySynchronization<SyncObj>::
AutoTrySynchronization(SyncObj& object, bool immediately)
	: _object(object),
	  _locked(immediately ? object.trylock() : false)
{} 

//	TEMPLATE FUNCTION public
//	Os::AutoTrySynchronization<SyncObj>::~AutoTrySynchronization --
//		オブジェクト生成時に自動的に同期オブジェクトをロックを試み、
//		破棄時にロックしていれば、自動的にはずすテンプレートクラスの
//		デストラクター
//
//	TEMPLATE ARGUMENTS
//		class SyncObj
//			操作する同期オブジェクト
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

template <class SyncObj>
inline
AutoTrySynchronization<SyncObj>::~AutoTrySynchronization()
{
	unlock();
}

//	TEMPLATE FUNCTION public
//	Os::AutoTrySynchronization<SyncObj>::lock --
//		ロックする
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

template <class SyncObj>
inline
void
AutoTrySynchronization<SyncObj>::lock()
{
	if (!isLocked()) {
		_object.lock();
		_locked = true;
	}
}

//	TEMPLATE FUNCTION public
//	Os::AutoTrySynchronization<SyncObj>::trylock --
//		ロックしていなければ、ロックを試みる
//
//	TEMPLATE ARGUMENTS
//		class SyncObj
//			操作する同期オブジェクト
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			結果的にロックしている
//		false
//			結果的にロックしていない
//
//	EXCEPTIONS

template <class SyncObj>
inline
bool
AutoTrySynchronization<SyncObj>::trylock()
{
	return (isLocked()) ? true : (_locked = _object.trylock());
}

//	TEMPLATE FUNCTION public
//	Os::AutoTrySynchronization<SyncObj>::unlock --
//		ロックされていれば、ロックをはずす
//
//	TEMPLATE ARGUMENTS
//		class SyncObj
//			操作する同期オブジェクト
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

template <class SyncObj>
inline
void
AutoTrySynchronization<SyncObj>::unlock()
{
	if (isLocked()) {
		_object.unlock();
		_locked = false;
	}
}

//	TEMPLATE FUNCTION public
//	Os::AutoTrySynchronization<SyncObj>::isLocked --
//		同期オブジェクトをロックしているか調べる
//
//	TEMPLATE ARGUMENTS
//		class SyncObj
//			操作する同期オブジェクト
//
//	NOTES
//	
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			ロックしている
//		false
//			ロックしていない
//
//	EXCEPTIONS
//		なし

template <class SyncObj>
inline
bool
AutoTrySynchronization<SyncObj>::isLocked() const
{
	return _locked;
}

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_AUTOSYNCHRONIZATION_H

//
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

