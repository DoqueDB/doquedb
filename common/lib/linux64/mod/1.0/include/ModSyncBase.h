// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModSyncBase.h -- 同期オブジェクト基底クラス関連のクラス定義
// 
// Copyright (c) 1998, 1999, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModSyncBase_H__
#define __ModSyncBase_H__

#include "ModCommonDLL.h"
#include "ModCommonInitialize.h"
#include "ModOsException.h"

class ModWaitingThread;

//	CLASS
//	ModSyncBase -- 同期オブジェクトの基底クラスを表すクラス
//
//	NOTES
//		このクラスを継承する同期オブジェクトは、
//		同期オブジェクト間のデッドロックの検出が可能になる

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModSyncBase
{
	friend class ModDetectDeadLock;
public:
	ModSyncBase(ModBoolean unlockKilled = ModFalse);
												// コンストラクター
	virtual ~ModSyncBase();						// デストラクター

	ModCommonDLL
	int						lock(unsigned int times = 1);
												// デッドロック検査をし、
												// ロックする
	ModCommonDLL
	int						trylock(unsigned int times = 1);
												// デッドロック検査をし、
												// ロックを試みる
	ModCommonDLL
	void					unlock(unsigned int times = 1);
												// アンロックする
	ModCommonDLL
	void					unlockAll();		// すべてアンロックする

	int						getLockCount() const;
												// 入れ子ロック数を得る
	ModBoolean				isSelfLocked() const;
												// ロックしているか

	virtual void			lockReally() = 0;	// 同期オブジェクトを
												// 実際にロックする
	virtual ModBoolean		trylockReally() = 0;// 同期オブジェクトを
												// 実際にロックを試みる
	virtual void			unlockReally() = 0;	// 同期オブジェクトを
												// 実際にアンロックする
private:
	ModThreadId				_locker;			// ロック中のスレッド ID
	int						_count;				// 入れ子ロック数
	ModBoolean				_unlockKilled;		// ロック中のスレッドの中断時に
												// 保持ロックを自動ではずすか
#ifdef MOD_DEBUG
	ModWaitingThread*		beginLock(ModBoolean doCheck);
												// ロック前の準備
	void					endLock(ModWaitingThread* lockerThread);
												// ロック後の後処理
	void					endUnlock();		// アンロック後の後処理

	ModWaitingThread*		_lockerThread;		// ロック中のスレッド情報
	unsigned int			_lockingCount;		// まさにロックしようと
												// しているスレッド数
#endif
};

//	FUNCTION public
//	ModSyncBase::ModSyncBase -- 同期オブジェクト基底クラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		ModBoolean			unlockKilled
//			ModTrue 
//				同期オブジェクトをロックしているスレッドが
//				中断されたとき、それの保持しているロックを自動的にはずす
//			ModFalse または指定されないとき
//				同期オブジェクトをロックしているスレッドが
//				中断されたとき、それの保持しているロックはそのままになる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ModSyncBase::ModSyncBase(ModBoolean unlockKilled)
	: _count(0),
	  _unlockKilled(unlockKilled)
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

#ifdef MOD_DEBUG
	_lockerThread = 0;
	_lockingCount = 0;
#endif
}

//	FUNCTION public
//	ModSyncBase::~ModSyncBase -- 同期オブジェクト基底クラスのデストラクター
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
//			同期オブジェクトをロックしたままで破棄しようとしている

inline
ModSyncBase::~ModSyncBase()
{
	if (_count)

		// 同期オブジェクトをロックしたままで、破棄しようとしている

		ModMessageThrowOsWarning(ModOsErrorStillLocked);
}

//	FUNCTION public
//	ModSyncBase::getLockCount -- 入れ子ロック数を得る
//
//	NOTES
//		同期オブジェクトにいくつロックがかかっているか調べる
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
ModSyncBase::getLockCount() const
{
	return _count;
}

//	FUNCTION public
//	ModSyncBase::isSelfLocked -- 呼び出したスレッドがロックしているか
//
//	NOTES
//		この関数を呼び出したスレッドが同期オブジェクトをロックしているか調べる
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
ModSyncBase::isSelfLocked() const
{
	return (_count && _locker == ModOsDriver::Thread::self()) ?
		ModTrue : ModFalse;
}

#endif	// __ModSyncBase_H__

//
// Copyright (c) 1998, 1999, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
