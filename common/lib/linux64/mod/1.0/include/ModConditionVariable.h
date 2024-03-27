// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModConditionVariable.h -- 条件変数機能を提供するクラス定義
// 
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModConditionVariable_H__
#define __ModConditionVariable_H__

#include "ModOsDriver.h"
#include "ModDefaultManager.h"
#include "ModTimeSpan.h"

//	CLASS 
//	ModPureConditionVariable -- 条件変数機能を提供するクラス
//
//	NOTES
//		条件変数機能を提供するクラスである
//		実際は、ユーザーはこのクラスの子クラスであり、
//		メモリーハンドルが明示されている ModConditionVariable を利用する
//
//		シグナル化する、シグナル化を待つ、シグナル化されたらリセットする
//		というようなイベントの機能を提供する
//
//		ModMutex などと異なりこのクラスの表す条件変数は
//		デッドロック検査の対象とならない

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModPureConditionVariable
	: public	ModSinglyLinkedEntry
{
public:
	// コンストラクター
	ModPureConditionVariable(ModBoolean doWakeUpAll = ModFalse,
							 ModBoolean doManualReset = ModFalse);
	// デストラクター
	~ModPureConditionVariable()
	{}

	void					signal();			// シグナル化する

	ModBoolean				wait();
	ModBoolean				wait(const ModTimeSpan& limit);
												// シグナル化を(指定時間)待つ

	void					reset();			// リセットする
private:
	ModOsDriver::ConditionVariable _conditionVariable;
												// 仮想 OS の条件変数
};

//	FUNCTION public
//	ModPureConditionVariable::ModPureConditionVariable --
//		条件変数機能を提供するクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		ModBoolean			doWakeUpAll
//			ModTrue
//				シグナル化されたときに待ちスレッドがすべて起きる
//			ModFalse または指定されないとき
//				シグナル化されたときに待ちスレッドのうち 1 つだけ起きる
//		ModBoolean			doManualReset
//			ModTrue
//				doWakeUpAll が ModTrue のとき、
//				条件変数が一度シグナル化されると、
//				ModPureConditionVariable::reset を
//				呼び出すまで非シグナル化されない
//			ModFalse または指定されないとき
//				ModPureConditionVariable::wait を呼び出して
//				シグナル化を待っていたスレッドが起きた時点で、
//				自動的に非シグナル化される
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ModPureConditionVariable::
ModPureConditionVariable(ModBoolean doWakeUpAll, ModBoolean doManualReset)
	: _conditionVariable(doWakeUpAll, doManualReset)
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();
}

//	FUNCTION public
//	ModPureConditionVariable::wait -- シグナル状態になるまで待つ
//
//	NOTES
//		条件変数がシグナル状態になるまで、呼び出しスレッドの実行を停止する
//		シグナル状態になった時点で、自動的にリセットがかかる
//
//	ARGUMENTS
//		ModTimeSpan&		limit
//			指定されたとき
//				シグナル状態になるまでの最大待ち時間
//			指定されないとき
//				シグナル状態になるまで永久に待つ
//
//	RETURN
//		ModTrue
//			シグナル状態になった
//		ModFalse
//			シグナル状態にならなかった
//
//	EXCEPTIONS

inline
ModBoolean
ModPureConditionVariable::wait()
{
	return _conditionVariable.wait();
}

inline
ModBoolean
ModPureConditionVariable::wait(const ModTimeSpan& limit)
{
	return _conditionVariable.wait((ModUInt32)limit.getTotalMilliSeconds());
}

//	FUNCTION public
//	ModPureConditionVariable::signal -- 条件変数をシグナル化する
//
//	NOTES
//		条件変数のシグナル化を待っているスレッドがあれば、
//		ひとつだけ待ち状態を解除する
//		複数のスレッドが待っているとき、任意のスレッドの待ち状態が解除される
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
ModPureConditionVariable::signal()
{
	_conditionVariable.signal();
}

//	FUNCTION public
//	ModPureConditionVariable::reset -- 条件変数をリセットする
//
//	NOTES
//		シグナル化されていても、シグナル化を待っているスレッドが
//		存在しないとき、条件変数の状態は初期化される
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
ModPureConditionVariable::reset()
{
	_conditionVariable.reset();
}

//	CLASS
//	ModConditionVariable --
//		ModPureConditionVariable クラスのメモリハンドル明示クラス
//
//	NOTES
//		デフォルトメモリーハンドルの管理下で
//		ModPureConditionVariable クラスを利用するためのクラス
//		通常、ユーザーは本クラスを利用する

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModConditionVariable
	: public	ModObject<ModDefaultManager>,
	  public	ModPureConditionVariable
{
public:
	// コンストラクター
	ModConditionVariable(ModBoolean doWakeUpAll = ModFalse,
						 ModBoolean doManualReset = ModFalse);
	// デストラクター
	~ModConditionVariable()
	{}
};

//	FUNCTION public
//	ModConditionVariable::ModConditionVariable --
//		ModPureConditionVariable クラスの
//		メモリハンドル明示クラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		ModBoolean			doWakeUpAll
//			ModTrue
//				シグナル化されたときに待ちスレッドがすべて起きる
//			ModFalse または指定されないとき
//				シグナル化されたときに待ちスレッドのうち 1 つだけ起きる
//		ModBoolean			doManualReset
//			ModTrue
//				doWakeUpAll が ModTrue のとき、
//				条件変数が一度シグナル化されると、
//				ModConditionVariable::reset を
//				呼び出すまで非シグナル化されない
//			ModFalse または指定されないとき
//				ModConditionVariable::wait を呼び出して
//				シグナル化を待っていたスレッドが起きた時点で、
//				自動的に非シグナル化される
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ModConditionVariable::
ModConditionVariable(ModBoolean doWakeUpAll, ModBoolean doManualReset)
	: ModPureConditionVariable(doWakeUpAll, doManualReset)
{ }

#endif	// __ModConditionVariable_H__

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
