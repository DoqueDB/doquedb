// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModThread.h -- スレッド機能を提供するクラス定義
// 
// Copyright (c) 1997, 2002, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModThread_H__
#define __ModThread_H__

#include "ModCommonDLL.h"
#include "ModError.h"
#include "ModDefaultManager.h"
#include "ModOsDriver.h"
#include "ModLinkedList.h"
#include "ModList.h"
#include "ModConditionVariable.h"

//
// モジュールは汎用OSに属する。
// したがって、エラーはModOsXXXである。
//

//
// TYPEDEF
// ModThreadFunction -- スレッドとして起動する関数の型
//
// NOTES
//	スレッドとして起動する関数の型を定義したもの。
//	
// 
typedef void* (*ModThreadFunction)(void *);

class ModSyncBase;

// 
// CLASS 
// ModThread -- スレッド記述子のクラス定義
//
// NOTES
// スレッドを管理するクラスである。
// ModThreadとして起動したスレッドは、リストとして管理される。
//
// オブジェクトから呼び出す処理には
// 起動、強制終了、スレッドIDの取得、スレッドの終了待ちがある。
//
// 現在実行中のスレッドに対する処理は、ModThisThreadクラスのstaticな
// メソッドとして(終了、自分自身のスレッドIDの取得)が準備されているので
// そちらを利用する。
// 
// スレッドの終了方法には、(安全な)終了abort()と強制終了kill()の2種類がある。
// 正常に処理を注料させるには、終了abort()を利用する必要がある。
// スレッド処理内部ではisAborted()によって状態をポーリングし、必要な場合は
// 正常にスレッド処理を自主的に中断させる。
// 一方、kill()による強制終了はとにかく終了することが目的で
// あり、正常に後処理が行なわれるかどうかは保証されない。
//
// この他に、スレッド処理の一時中断を要請するinterrupt()がある。
// これは実行スレッドの方で対応を適宜判断し、必要に応じて終了させるなり、
// ステータスをrunningに戻すなりすることができる。
//
// エラーが起きたときに利用する例外オブジェクトは
// 1つのスレッドオブジェクトに対し、1つ用意される。
// メインスレッド用にはstaticに全体として1つ用意される。
// スレッドをまたがるエラーは、joinの返り値の終了ステータスを
// チェックすることによって判定する。
// 異常終了ならば、該当するスレッドの例外オブジェクトを参照し、
// エラー内容を判断する。

//
// ModPureWideStringに変更して、サブクラスとしてメモリハンドルクラスを
// 明示したクラスを作成するには、問題があり断念し、
// 直接ModDefaultObjectのサブクラスとして作成する。
// (ライブラリは作成できたものの
// テストプログラムで、ModThread* me = ModThisThread::getThread();	
// という部分でひっかかる)
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModThread
	: public	ModSinglyLinkedEntry,
	  public	ModDefaultObject
{
public:
	// スレッドオブジェクトのリストにアクセスするために必要。
	friend class ModThisThread;	
	// 初期化、後処理関数を呼び出してもらうため。他からは呼び出せない
	friend class ModCommonInitialize;
	
	//
	// ENUM
	// ModThread::Status -- スレッドの動作状態
	//
	// NOTES
	//	スレッドの実際の動作ではなく、MODスレッドとしての状態を表す。
	//	実行前、通常動作中、中断要請を受けているがまだ受け付けていない、
	//	中断処理中、終了のいずれかを表す。
	//	以下のうち、interupting, abortingは、interrupt(), abort()内部で
	//	設定される。interrupted, abortedはともに、スレッドの内部で
	//	処理を受け付けた段階で設定することが望ましい。
	// 	
	enum Status {
		notStarted 		= 0,	// 実行前		[コンストラクタでセット]
		running 		= 1,	// 通常処理中	[createもしくはsetStatusでset]
		                        // (実際動いているかどうかとは別)
		interrupting 	= 2,	// 中断要請中	[interruptでセット]
		interrupted 	= 3,	// 中断処理中	[setStatusでセット]
		aborting 		= 4,	// 終了要請中	[abortでセット]
		aborted 		= 5,	// 終了処理中	[setStatusでセット]
		exitSoon 		= 6,	// 間もなく終了	[スレッド内もしくはkill, exit()
		                        //               でセット]
		exited 			= 7		// 終了確認		[join/getStatusで確認しセット]
	};
	//
	// ENUM
	// ModThread::ExitType -- スレッドの終了形態を表す。
	//
	// NOTES
	// 	MODスレッドの終了形態を表す。
	//	関数が返り値を返して終了、exitによって終了、例外を発行して終了、
	//	強制終了の4種類と、未設定の値が用意されている。
	//
	enum ExitType {
		terminateNormally	 = ModOs::Thread::normally,
		exitThread			 = ModOs::Thread::exited,
		throwException		 = ModOs::Thread::except,
		killThread			 = ModOs::Thread::killed,
		unknown				 = ModOs::Thread::unknown
	};
	// 
	// CLASS 
	// ExitStatus -- スレッドの終了ステータスを表す
	// NOTES
	// スレッドの終了ステータスを表す。
	// 終了形態はtypeに格納されている。
	// 種類typeにあった関数を呼び出して終了ステータスを得る。
	//

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

	class ExitStatus
		: public	ModDefaultObject
	{
		friend class ModThread;
	public:
		ExitStatus();
		// デストラクター
		~ExitStatus()
		{}

		ExitType getType() const;
		void* getReturnValue() const;
		ModException* getException() const;
		unsigned int getExitCode() const;
	private:
		// 種類
		ExitType type;

		void* returnValue;
		ModException* exception;
		unsigned int exitCode;
	};

	// コンストラクタ、デストラクタ
	ModCommonDLL
	ModThread();
	~ModThread();

	// オブジェクトから呼び出すメソッド

	// スレッドとして起動
	ModCommonDLL
	void create(ModThreadFunction routine, void* argument);
	// スレッドの強制終了
	// 安全に終ることはできない
	ModCommonDLL
	void kill();
	// スレッドIDを得る
	ModThreadId getThreadId() const;
	// スレッド終了を待つ。終了状態を返す
	ModCommonDLL
	const ExitStatus& join();
	// 終了状態を返す。join後でないと意味をもたない
	const ExitStatus& getExitStatus() const;

	// スレッド処理の中断要請
	ModCommonDLL
    ModBoolean interrupt();
	// 中断処理を終了し、中断要請を元に戻す(ステータスをrunningにするのと同等)
	ModBoolean resetInterrupt();

	// スレッド処理の中断処理開始などのステータスの設定
	ModCommonDLL
    ModBoolean setStatus(ModThread::Status);

	// スレッド処理の(安全)な終了要請
	ModCommonDLL
	ModBoolean abort();

	// ステータスを返す。本当に終了しているか確認するためconstではない
	ModCommonDLL
    Status getStatus();
	// 終了要請中または終了処理中かどうかを調べる。
	ModBoolean isAborted() const;
	// 中断要請中または中断処理中かどうかを調べる。
	ModBoolean isInterrupted() const;

	ModCommonDLL
	ModBoolean				setInterruptable(ModBoolean v);
												// 中断要求、終了要求の
												// 受付の可否を設定する
	ModBoolean				isInterruptable() const;
												// 中断要求、終了要求の
												// 受付が可能か

	// ロック中のミューテックスリストの更新
	// ModSyncBase から呼び出されるため引数は ModSyncBase でなければならない

	ModCommonDLL
	void					addLockingMutex(ModSyncBase* sync);
	ModCommonDLL
	void					deleteLockingMutex(ModSyncBase* sync);
	ModCommonDLL
	void					unlockLockingMutex();

	// スレッドの状態をプリントする
	ModCommonDLL
	void print() const;

	// 以下はstatic関数

	// スレッドIDを指定してModThreadを得る
	ModCommonDLL
	static ModThread* getModThread(ModThreadId threadId);

	// スレッドリストをすべてプリントする
	ModCommonDLL
	static void printList();

	// 初期化されたかどうか
	static ModBoolean isInitialized();

	// 全スレッドに対して終了させる。
	// [注意] メインのスレッドから呼び出さないと、自分で自分を終了させる
	// ことになる。
	static void killAll();
	// スレッドIDを指定してスレッドを強制終了させる
	ModCommonDLL
	static void killById(ModThreadId threadid);

	// スレッドにひとつ用意する例外オブジェクト
	ModException exception;

private:
	ModCommonDLL
	void					destruct();			// デストラクター下位関数

	static void				initialize();		// スレッド環境の初期化
	static void				terminate();		// スレッド環境の後処理

	// スレッドとして本当に実行する関数。一般からの利用はできない
	static void* startup(void* argument);

	ModOsDriver::Thread		_thread;			// 仮想 OS のスレッド

	ModThreadFunction		_routine;			// スレッドが起動する関数
	void*					_argument;			// スレッドが起動する
												// 関数へ渡す引数

	Status					_status;			// スレッドの実行状態
	ExitStatus				_exitStatus;		// スレッドの終了状態

	ModConditionVariable	_dozeCondition;		// 中断要求待ち用の条件変数

	ModSize					_notInterruptable;	// 中断要求を受付可能か
	ModBoolean				_interruptingReserved;
												// 中断要求が保留されているか
	ModBoolean				_abortingReserved;	// 終了要求が保留されているか

	ModBoolean				_forceUnlocking;	// 強制アンロック中か
	ModList<ModSyncBase*>*	_lockingList;		// スレッドがロック中の
												// 同期オブジェクトを
												// 管理するリスト

	ModCommonDLL
	static ModBoolean		_initialized;		// スレッド環境が
												// 初期化されているか

	static ModSinglyLinkedList<ModThread>* _threadList;
												// 既存の ModThread を
												// 管理するためのリスト
	static ModOsDriver::ThreadSpecificKey* _selfStoredKey;
												// 自スレッドの ModThread を
												// 格納する領域の先頭アドレスを
												// 持つスレッド固有ストレージ
};

//
// CLASS
// ModThisThread -- 現在実行中のスレッドに対する処理
//
// NOTES
// staticなメソッドは直接ModThisThread::exit()のように呼び出し、
// 現在実行中のスレッドに関する処理だけを行う。
//

class ModCommonDLL ModThisThread
{
public:
	// スレッド終了
	static void exit(int status);
	// スレッド実行を指定時間止める
	static void sleep(const ModTimeSpan& time);
	static void sleep(ModSize milliSecond);
	// スレッド中断要求がない間、スレッド実行を指定時間止める(1)
	// こちらではなく、条件変数版の(2)を使うことを推奨する
	static ModBoolean doze(const ModTimeSpan& sleeptime, 
						   const ModTimeSpan& interval, 
						   ModThread* thread = 0);
	// スレッド中断要求がない間、スレッド実行を指定時間止める(2)
	// [条件変数による実装版]
	static ModBoolean doze(const ModTimeSpan& sleeptime, 
						   ModThread* thread = 0);
	// スレッドIDを得る
	static ModThreadId self();
	// スレッドIDを指定して待つ(必要なら用意するが今はない)

	// 現在実行しているスレッドのスレッドオブジェクトを返す
	static ModThread* getThread();

	// 終了要請中または終了処理中か
	static ModBoolean isAborted();
	// 中断要請中または中断処理中か
	static ModBoolean isInterrupted();

	static ModBoolean				setInterruptable(ModBoolean v);
												// 中断要求、終了要求の
												// 受付の可否を設定する

	// 現在実行中のスレッドで使うべき例外クラスへの参照を返す。参照で受け取り、
	// 内容を設定して送出する
	static ModException* getException();

private:
	// 全体にひとつ(MODスレッドとして起こしていないメインスレッドで利用)
//	static ModException mainException;
};

// FUNCTION public
// ModThread::isInitialized -- スレッドモジュール全体の初期化チェック
//
// NOTES
//	スレッドモジュール全体が初期化されているかどうかを返す。
//	スレッドモジュール全体が初期化されないと、ModExceptionが利用
//	できないので重要である。
//
// ARGUMENTS
//	なし
//
// RETURN
//	スレッドモジュールが初期化されていればModTrue、されていなければ
//	ModFalseを返す
//
// EXCEPTIONS
//	なし

inline
ModBoolean
ModThread::isInitialized()
{
	return _initialized;
}

//	FUNCTION public
//	ModThread::~ModThread -- スレッドを表すクラスのデストラクター
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

inline
ModThread::~ModThread()
{
	this->destruct();
}

//
// FUNCTION public
// ModThread::killById -- スレッドIDを指定したスレッド強制終了(static)
//
// NOTES
//	スレッドIDを指定し、スレッドを強制終了させる。
//	getModThread、kill()を利用したのと同等である。
//	外部から強制終了させる場合はスレッドIDしか調査の手がかりがなく、便利のために
//	作成した。
//
// ARGUMENTS
//	ModThreadId threadId
//
// RETURN
//	なし
//
// EXCEPTIONS
//	下位の例外はそのまま送出

inline
void
ModThread::killById(ModThreadId id)
{
	ModThread::getModThread(id)->kill();
}

//
// FUNCTION public
// ModThread::resetInterrupt -- 中断処理を終了し、要求をリセットする。
//
// NOTES
//	isInterrupted()によるポーリングの結果行った中断処理が終了した場合に
//	呼び出す。内部ステータスは通常「実行中」に設定され、ModTrueが返される。
//	ただし、中断処理中に終了要求があった場合にはこのタイミングで
//	内部ステータスが「終了要求」abortingに変更され、ModFalseが返される。
//
// ARGUMENTS
//	なし
//
//	RETURN
//		ModTrue
//			スレッドの状態を実行中に戻すことができた
//		ModFalse
//			スレッドの状態は実行中だったので、そのままである
//			または、中断要求や終了要求が保留されていたので、
//			スレッドの状態は中断要求中または終了要求中になった
//
// EXCEPTIONS
//	なし
//
inline
ModBoolean
ModThread::resetInterrupt()
{
	return this->setStatus(running);
}

//	FUNCTION public
//	ModThread::isInterrupted -- 中断要求中または中断処理中か
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			中断要求中または中断処理中である
//		ModFalse
//			中断要求中でも中断処理中でもない
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModThread::isInterrupted() const
{
	return (_status == interrupting ||
			_status == interrupted) ? ModTrue : ModFalse;
}

// FUNCTION public
// ModThread::isAbort -- 終了要請中または終了処理中かどうかを調べる
//
// NOTES
//	本オブジェクトが管理するスレッドに対して、終了要求があるかどうかを
//	調べる。終了要請中または終了処理中の場合はModTrueを返す。
//	スレッド処理実行中はポーリング方式で、何度も呼び出す必要がある。
//	中断処理中の終了要請は中断処理が終るまでModFalseを返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	終了要請中(aborting)または終了処理中(aborted)の場合はModTrue、なければModFalseを返す。
//	
//
// EXCEPTIONS
//	なし

inline
ModBoolean
ModThread::isAborted() const
{
	return (_status == aborting || _status == aborted) ? ModTrue : ModFalse;
}

//	FUNCTION public
//	ModThread::isInterruptable -- 中断要求、終了要求の受付が可能か
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			中断要求、終了要求の受付は可能である
//		ModFalse
//			中断要求、終了要求の受付は不可である
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModThread::isInterruptable() const
{
	return (_notInterruptable ||
			this->exception.isError()) ? ModFalse : ModTrue;
}

// FUNCTION public
// ModThread::getThreadId -- スレッドIDを得る
//
// NOTES
//	本オブジェクトが管理するスレッドの仮想OSレベルのスレッドIDを返す。
//	たとえ、実行が終了していてもIDを返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	スレッドID
//
// EXCEPTIONS
//	なし

inline
ModThreadId
ModThread::getThreadId() const
{
	return _thread.getThreadId();
}

//
// FUNCTION public
// ModThread::getExitStatus -- スレッドの終了ステータスを得る
//
// NOTES
//	本オブジェクトが管理するスレッドが終了している場合、
//	その終了ステータスへの参照を返す。終了形態がunknownである場合は
//	まだスレッドが終了していないことを示す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	終了状態クラスへの参照
//
// EXCEPTIONS
//	なし
//

inline
const ModThread::ExitStatus&
ModThread::getExitStatus() const
{
	return _exitStatus;
}

//
// FUNCTION public
// ModThread::ExitStatus::ExitStatus -- 終了ステータスクラスのコンストラクタ
//
// NOTES
//	終了ステータスクラスのコンストラクタである。内部の値を初期化する。
//	初期化時の値はすべて0であり、終了形態はunknownである
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//
inline
ModThread::ExitStatus::ExitStatus()
	:type(unknown), returnValue(0), exception(0), exitCode(0)
{
}

//
// FUNCTION public
// ModThread::ExitStatus::getType -- スレッドの終了形態を得る
//
// NOTES
//	終了ステータスクラスがもつ、スレッドの終了形態を示す値を返す。
//	返された値に応じたメソッドを実行すると、関数の返り値や終了コード、
//	例外の内容などを得ることができる。teminateNormallyの場合は
//	getReturnValue()、exitThreadの場合はgetExitCode()、
//	throwExceptionの場合はgetException()を呼び出す。killThreadの場合は
// 	未定。
//
// ARGUMENTS
//	なし
//
// RETURN
//	スレッドの終了形態をModThread::ExitTypeの値で返す。
//
// EXCEPTIONS
//	なし
//
inline
ModThread::ExitType
ModThread::ExitStatus::getType() const
{
	return this->type;
}

//
// FUNCTION public
// ModThread::ExitStatus::getException -- スレッド内部で起きた例外を得る
//
// NOTES
//	スレッドの終了形態が「例外送出を表すthrowException」であるとき、
//	スレッド内部で起きた例外オブジェクトへのポインタを返す。
//	実際の例外オブジェクトは対象スレッドを管理するModThreadが保持しており、
//	ポインタを返すだけである。つまりこの領域をdeleteしてはいけない。
//	スレッドの終了形態がその他の場合には、0を返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	スレッド内部で例外が起きた場合には、その例外オブジェクトへのポインタを返す
//
// EXCEPTIONS
//	なし

inline
ModException*
ModThread::ExitStatus::getException() const
{
	return (this->type == throwException) ? this->exception : 0;
}

//
// FUNCTION public
// ModThread::ExitStatus::getExitCode -- スレッドの終了コードを得る
//
// NOTES
//	スレッドの終了形態が「スレッドをexit()で終了したことを表すexitThread」
//	であるとき、指定された終了コードを返す。
//	スレッドの終了形態がその他の場合には、デフォルトの0を返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	スレッドが終了コードを指定したexitで終了した場合に、その終了コードを返す。
//
// EXCEPTIONS
//	なし
//
inline
unsigned int
ModThread::ExitStatus::getExitCode() const
{
	return this->exitCode;
}

//
// FUNCTION public
// ModThread::ExitStatus::getReturnValue -- スレッドとして起動した関数の返り値を得る
//
// NOTES
//	スレッドの終了形態が「返り値を返して関数が終了したことを表す
//	terminateNormally」であるとき、関数の返り値を返す。
//	ポインタの指す領域の処理はユーザに任される。
//	スレッドの終了形態がその他の場合には、0を返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	返り値を返してスレッドが終了した場合に、その返り値を返す
//
// EXCEPTIONS
//	なし
//
inline
void*
ModThread::ExitStatus::getReturnValue() const
{
	return this->returnValue;
}

#endif	// __ModThread_H__

//
// Copyright (c) 1997, 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
