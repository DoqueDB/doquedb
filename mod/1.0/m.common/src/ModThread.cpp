// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModThread.cpp -- スレッド機能を提供するクラスのメソッド定義
// 
// Copyright (c) 1997, 1999, 2003, 2023 Ricoh Company, Ltd.
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


#include "ModThread.h"
#include "ModLinkedList.h"
#include "ModAutoMutex.h"
#include "ModCommonMutex.h"
#include "ModSyncBase.h"
#include "ModCommonInitialize.h"
#include "ModConditionVariable.h"
#include "ModTimeSpan.h"
#include "ModList.h"
#include "ModOsException.h"

//	VARIABLE private
//	ModThread::_initialized -- スレッド環境が初期化されているか
//
//	NOTES

ModBoolean	ModThread::_initialized = ModFalse;

//	VARIABLE private
//	ModThread::_threadList -- 既存の ModThread を管理するためのリスト
//
//	NOTES

ModSinglyLinkedList<ModThread>*	ModThread::_threadList = 0;

//	VARIABLE private
//	ModThread::_selfStoredKey --
//		自スレッドの ModThread を格納する領域の先頭アドレスを持つ
//		スレッド固有ストレージ
//
//	NOTES

ModOsDriver::ThreadSpecificKey*	ModThread::_selfStoredKey = 0;

//
// FUNCTION public
// ModThread::initialize -- スレッドモジュール全体の初期化
//
// NOTES
//	スレッドモジュール全体の初期化を行なう。
//	スレッドオブジェクトリストの初期化、ModThread検索用の
//	スレッドローカル変数の初期化を行なう。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModOsErrorSystemMemoryExhaust		(::new)
//		システムメモリが不足
//	その他
//		ModOsDriver::ThreadSpecificKey::ThreadSpecificKeyの例外参照(主なものは以下に書き下す)
//	ModOsErrorResourceExhaust	(ModOsDriver::ThreadSpecificKey::同)
//		リソースが不足しているか、キー数がプロセスごとの制限数を超えている
//	ModOsErrorSystemMemoryExhaust	(ModOsDriver::ThreadSpecificKey::同)
//		実行に必要なシステムのメモリ不足
//

// static
void
ModThread::initialize()
{
	if (ModThread::isInitialized() == ModFalse) {

		// 生成されているスレッドを表すクラスを管理するリストを生成する

		_threadList = new ModSinglyLinkedList<ModThread>();
		; ModAssert(_threadList != 0);

		// スレッドを表すクラスをそのスレッドのどこからでも参照するために、
		// スレッドを表すクラスを登録するためのスレッド固有ストレージ

		_selfStoredKey = new ModOsDriver::ThreadSpecificKey();
		; ModAssert(_selfStoredKey != 0);

		// 初期化したことを記録しておく

		_initialized = ModTrue;
	}
}

//
// FUNCTION public
// ModThread::terminate -- スレッドモジュール全体の後処理
//
// NOTES
//	スレッドモジュール全体の後処理を行なう。
//	スレッドオブジェクトリストの後処理、ModThread検索用の
//	スレッドローカル変数の後処理を行なう。
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

// static
void
ModThread::terminate()
{
	if (ModThread::isInitialized() == ModTrue) {

		delete _threadList, _threadList = 0;
		delete _selfStoredKey, _selfStoredKey = 0;

		_initialized = ModFalse;
	}
}

//
// FUNCTION public
// ModThread::killAll -- 全MODスレッドの強制終了
//
// NOTES
// staticなメソッド。
// ModThreadとして作成された全スレッドを強制終了させる。
// メインのスレッドから呼び出さなくてはならない。
// そうでないと自分で自分を終了させることになりかねない。
//	(メインのスレッドとは、ModThreadとして起動した以外のスレッドを指す)
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModOsErrorNotMainThread
//		実行スレッドがメインスレッドではない
//	その他
//		ModCommonInitialize::checkAndInitialize(初期化前のみ)、ModOsDriver::Mutex::lock, unlock, ModOsDriver::Thread::killの例外参照
//

// static
void
ModThread::killAll()
{
	//【注意】	ModThisThread::getThread 内部で
	//			必要ならば汎用ライブラリーが初期化される

	if (ModThisThread::getThread() != 0)

		// メインスレッド以外でスレッドをすべて破棄しようとしている

		ModThrowOsError(ModOsErrorNotMainThread);
	
	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	m.lock();

	ModSinglyLinkedListIteratorBase item(*_threadList);
	for (; *item; ++item)
		((ModThread*) *item)->kill();
}

//
// FUNCTION public
// ModThread::ModThread -- スレッド記述子のコンストラクタ
//
// NOTES
//	スレッドクラスのコンストラクタである。
//	全部のスレッドオブジェクトを管理するリストに自身を挿入する。
//	内部ステータスはnotStartedに設定される。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		ModCommonInitialize::checkAndInitialize(初期化前のみ)、ModOsDriver::Mutex::lock, unlock, ModOsDriver::ConditionVariable::ConditionVariableの例外参照
//	ModOsErrorSystemMemoryExhaust		(::new)
//		システムメモリが不足
//

ModThread::ModThread()
	: _routine(0),
	  _argument(0),
	  _status(notStarted),
	  _dozeCondition(ModTrue),
	  _notInterruptable(0),
	  _interruptingReserved(ModFalse),
	  _abortingReserved(ModFalse),
	  _forceUnlocking(ModFalse),
	  _lockingList(0)
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	// 生成されているスレッドを管理するリストへ登録する

	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	m.lock();

	; ModAssert(_threadList != 0);
	_threadList->insert(this);
}

//	FUNCTION private
//	ModThread::destruct -- スレッドを表すクラスのデストラクター下位関数
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

void
ModThread::destruct()
{
	if (_threadList != 0) {

		// 生成されているスレッドを管理するリストに登録されていれば、
		// リストから削除する
		//
		//【注意】	ModThread::terminate() 以降は
		//			このリストは存在しない

		ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
		m.lock();

		_threadList->expunge(this);
	}
	if (_lockingList != 0)

		// スレッドがロック中の同期オブジェクトを
		// 管理するためのリストが生成されていれば、破棄する

		delete _lockingList, _lockingList = 0;
}

//
// FUNCTION public
// ModThread::create -- スレッド関数の実行
//
// NOTES
//	引数に指定された関数をスレッドとして起動し、内部ステータスをrunnningに
//	設定する。
//	関数routineへ渡す引数は、引数argumentに指定する。routineが返す値は
//	関数join()で受け取ることができる。
//	内部ステータスがnotStarted(将来的にはexitedの場合も)だけ、実行できる。
//
//	ModThreadが管理するスレッドの実行が終了した後、同じModThreadオブジェクト
//	を再利用して再度別のスレッドを実行することは保証しない。
//	(可能な場合もあるが、スレッド稼働中かどうかのチェックや再初期化が甘い)
//
// ARGUMENTS
//	ModThreadFunction routine
//		スレッドとして起動する関数。
//	void* argument
//		関数に渡す引数
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		ModOsDriver::Mutex::lock, unlock, ModOsDriver::Thread::createの例外参照(主なものを以下に書き下す)
// ModOsErrorSystemMemoryExhaust
//		実行に必要なシステムメモリが不足
//	ModOsErrorTooManyThreads
//		スレッドが多すぎる
//

void
ModThread::create(ModThreadFunction routine, void* argument)
{
	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	m.lock();

	// 指定関数と引数の設定
	// これも仮想関数にした方がかっこよくできることが後で判明した。
	// そのように変更することはもはやすぐであるが、仕様は狭くなる
	// (スレッドとして起動したものは必ずModThreadのサブクラスに仮想関数として
	// 作成しなければならない)のでやめている。

	_routine = routine;
	_argument = argument;

	// 仮想OSスレッドの起動

	// startup内部で本スレッドを示すthisポインタをスレッドローカル変数から
	// 取り出せるように設定

	_thread.create(ModThread::startup, this);
	ModBoolean result = this->setStatus(running);
	; ModAssert(result == ModTrue);
}

//
// FUNCTION private
// ModThread::startup -- スレッドとして実行する関数の外側部分
//
// NOTES
//	スレッドとして実際に関数を起動される関数である。内部の設定をした後、
//	引数に指定されたModThreadに設定されている関数を呼び出し、
//	内部ステータスを設定した後、その関数の返り値を返す。
//
// ARGUMENTS
//	void* thread
//		実行スレッドのModThreadを指すポインタ
//
// RETURN
//	スレッドとして実行した関数の返す値
//
// EXCEPTIONS
//	routineに設定されている関数が送出する例外
//		ModThread::createに指定した関数の例外
//	その他
//		ModOsDriver::ThreadSpecificKey::setValueの例外参照
//		設定に必要なシステムのメモリ不足
//

// static
void*
ModThread::startup(void* argument)
{
	// 引数には起動されたスレッドを表す ModThread が渡されている

	ModThread* thread = (ModThread*) argument;

	// 起動されたスレッド内部から自スレッドを表す
	// ModThread を参照できるように、スレッド固有ストレージに
	// 自スレッドを表す ModThread を登録しておく

	_selfStoredKey->setValue(thread);

	// ModThread::create で指定された関数に引数を与えて実行する

	void*	ret;
	try {
		ret = (*thread->_routine)(thread->_argument);

	} catch (ModException& exception) {

		// 実行した関数で例外が発生したので、
		// スレッドの状態を終了処理中に設定する

		(void) thread->setInterruptable(ModTrue);
		(void) thread->setStatus(aborting);
		(void) thread->setStatus(aborted);

		ModRethrow(exception);
	}

	// 実行した関数の実行が終了したので、
	// スレッドの状態をまもなく終了に設定する

	(void) thread->setInterruptable(ModTrue);
	(void) thread->setStatus(exitSoon);

	return ret;
}

//
// FUNCTION public
// ModThread::kill -- 管理するスレッドの強制終了
//
// NOTES
//	非常手段として、管理するスレッドの実行を強制終了させる。
//	強制終了なので、内部が正常に終了するかどうかは全く保証されない。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		ModOsDriver::Thread::killの例外参照(主なものは以下に書き下す)
//	ModOsErrorKillSelfThread		(ModOsDriver::Thread::kill)
//		自分自身を強制終了させている
//

void
ModThread::kill()
{
	// 仮想 OS のスレッドを強制終了する

	_thread.kill();

	// スレッドの状態をまもなく終了に設定する

	(void) this->setStatus(exitSoon);
}

//
// FUNCTION public
// ModThread::join -- スレッド関数の終了待ち
//
// NOTES
//	本オブジェクトが管理するスレッド関数の終了を待つ。
//	内部ステータスは内部でexitSoonに設定済みのはずであるが、joinできたら
//	改めてexitedにセットする。joinされなかった場合はexitSoonのままとなるが、
//	関数getStatusが呼ばれた段階でスレッドの存在を確認し、
//	なくなっていれば確実にexitedにセットしなおされる。exit()やkill()から
//	実際にスレッドの実行が終了するまでの非常に短い時間にgetStatus()した場合
//	には、exitSoonが起こり得る。(これを待って必ずexitedにするかもしれない)
//	スレッドとして実行された関数が返した値またはModThisThread::exit()の
//	引数に指定された終了ステータスへのポインタを返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	void*
//		スレッドとして実行された関数の返り値
//
// EXCEPTIONS
//	その他
//		ModOsDriver::Thread::joinの例外参照(主なものは以下に書き下す)
//	ModOsErrorDeadLockInJoin		(ModOsDriver::Thread::join)
//		デッドロックを起こしている(自分自身を待つなど)
//

const ModThread::ExitStatus&
ModThread::join()
{
	// 仮想 OS のスレッドが終了するまで待つ

	void*	ret = _thread.join();

	if (_status == exitSoon)

		// スレッドの状態がまもなく終了であれば、
		// スレッドの状態を終了確認にする
		//
		//【注意】	自明なので ModThread::getStatus を使って、
		//			スレッドが存在しないことを確認しない

		(void) this->setStatus(exited);

	// 仮想 OS のスレッドの終了状態をから、
	// スレッドの終了状態を生成する

	switch (_thread.getExitType()) {
	case ModOs::Thread::normally:
		_exitStatus.type = terminateNormally;
		_exitStatus.returnValue = ret;
		break;
	case ModOs::Thread::except:
		_exitStatus.type = throwException;
		_exitStatus.exception = &this->exception;
		break;
	case ModOs::Thread::exited:
		_exitStatus.type = exitThread;
		; ModAssert(ret != 0);
		_exitStatus.exitCode = *(unsigned int *) ret;
		break;
	case ModOs::Thread::killed:
		_exitStatus.type = killThread;
		; ModAssert(ret == 0 || *(unsigned int *) ret == 0);
		_exitStatus.exitCode = 0;
		break;
	case ModOs::Thread::unknown:
		_exitStatus.type = unknown;
		; ModAssert(ret == 0);
		_exitStatus.exitCode = 0;
	}

	return _exitStatus;
}

//
// FUNCTION public
// ModThread::abort -- 安全な終了を要求
//
// NOTES
//	本オブジェクトが管理するスレッド関数の安全な終了を要求する。
//	内部的には、ステータスを「終了要求」に設定する。
//	実行前(notStarted)や、終了直前から終了済み(exitSoon, exited)の場合、
//	終了処理中(aborted)の場合は要求できない。
//	要求できた場合にはModTrue、できなかった場合にはModFalseを返す。
//	終了処理中(aborted)の場合は既に設定済みとしてステータス変更は
//	されないが、ModTrueを返す。終了要求中(aborting)の場合はそのままModTrueを
//	返す。
//	aborting状態になった場合、対象スレッドは自主的にすみやかに終了する
//	必要がある。
//
//	終了要求は中断要求interrupt()よりも強く、中断中(interrupting)や
//	中断処理中(interrupted)だった場合も要求は可能であるが、中断処理中で
//	突然ステータスが変更されると混乱する可能性があるので、
//	すぐにはステータスに反映されずに本関数はTrueを返す。
//	この場合ステータス「終了要求」abortingは、中断処理が終了してステータスを
//	runningに戻すときに反映される。このとき、resetIntterupt(), 
//	setStatus(running)はModFalseを返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	要求できた場合にはModTrue、できなかった場合にはModFalseを返す。
//
// EXCEPTIONS
//		ModOsDriver::Mutex::lock, unlock, ModOsDriver::ConditionVariable::signalの例外参照
//

ModBoolean
ModThread::abort()
{
	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	m.lock();

	// スレッドの状態を終了要求中に変更する
	//
	//【注意】	終了要求中に変更可能なスレッドの状態は
	//			running, interrupting, interrupted, aborting, aborted
	//			のときである
	//
	//【注意】	中断要求中、中断処理中、中断要求受付不可のとき、
	//			終了要求は保留され、_abortingReserved が ModTrue になる

	Status old = _status;
	ModBoolean done = this->setStatus(aborting);

	if (_status == aborting && old != aborting)

		// 今回、初めて終了要求中になったときのみ
		// 中断要求待ちしているスレッドを復帰させる
		//
		//【注意】	以前の状態が aborting のときは
		//			すでに条件変数がシグナル化されているので、
		//			今回、シグナル化しない

		_dozeCondition.signal();

	return done;
}

//
// FUNCTION public
// ModThread::interrupt -- 中断を要求
//
// NOTES
//	本オブジェクトが管理するスレッド関数の中断を要求する。
//	内部ステータスを「中断要求」に設定する。
//	終了要請中(aborting)もしくは終了処理中(aborted)の場合や、
//	実行前(notStarted)、終了直前から終了済み(exitSoon, exited)の場合、
//	は要求できない。
//	要求できた場合にはModTrue、できなかった場合にはModFalseを返す。
//	中断処理中(interrupted)の場合は既に設定済みとしてステータス変更は
//	されないが、ModTrueを返す。中断要求中(interrupting)の場合はそのままModTrueを
//	返す。
//
//	中断要求がされた場合、対象スレッドがそれを受け付けた段階で、
//	スレッドの処理が中断される(はずである)。その後
//	しかるべき処理の後処理を再開するか、終了するかはスレッド側の実装による。
//	スレッドの実装では、中断処理をはじめる場合にsetStatus等で内部ステータスを
//	interruptedに設定しなければならない。そのタイミングで
//	doze中のスレッドに中断を知らせるための条件変数もリセットされる。
//	その後、正常処理を再開する場合には、resetInterruptもしくはsetStatusを
//	呼び出して、ステータスをrunningに戻すこと。
//
//	interrupt中(interrupting, interrupted)の終了要求abort()の呼び出しでは、
//	実際の処理とフラグの状態の不一致のを避けるために受け付けだけを行い
//	関数はいったん終了する。実際にはinterrupt処理が
//	終了してruuningに戻すタイミングで処理が行われ、ステータスがabortingに
//	設定される。つまり中断中の終了処理はステータスがrunningになるのを待って
//	処理が行われることになる。この場合、resetInterrupt(), setStatus(running)は
//	Falseを返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	要求できた場合ModTrue、できなかった場合ModFalseを返す
//
// EXCEPTIONS
//		ModOsDriver::Mutex::lock, unlock, ModOsDriver::ConditionVariable::signalの例外参照
//

ModBoolean
ModThread::interrupt()
{
	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	m.lock();

	// スレッドの状態を中断要求中に変更する
	//
	//【注意】	中断要求中に変更可能なスレッドの状態は
	//			running, interrupting, interrupted
	//			のときである
	//
	//【注意】	中断要求受付不可のとき、
	//			中断要求は保留され、_interruptingReserved が ModTrue になる


	Status old = _status;
	ModBoolean done = this->setStatus(interrupting);

	if (_status == interrupting && old != interrupting)

		// 今回、初めて中断要求中になったときのみ
		// 中断要求待ちしているスレッドを復帰させる
		// 
		//【注意】	以前の状態が interrupting のときは
		//			すでに条件変数がシグナル化されているので、
		//			今回、シグナル化しない

		_dozeCondition.signal();

	return done;
}

//	FUNCTION public
//	ModThread::setInterruptable -- 中断要求、終了要求の受付の可否を設定する
//
//	NOTES
//
//	ARGUMENTS
//		ModBoolean			v
//			ModTrue
//				中断要求、終了要求の受付を可能にする
//			ModFalse
//				中断要求、終了要求の受付を不可にする
//
//	RETURN
//		ModTrue
//			指定されたように設定できた
//		ModFalse
//			中断要求中、中断処理中、終了要求中、終了処理中のため、
//			集団要求、終了要求の受付を不可にできなかった
//
//	EXCEPTIONS

ModBoolean
ModThread::setInterruptable(ModBoolean v)
{
	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	m.lock();

	if (v == ModFalse) {

		// 中断要求、終了要求の受付を不可にするとき

		switch (_status) {
		case interrupting:

			// 中断要求中であれば、中断要求を保留にする

			_status = running;
			_interruptingReserved = ModTrue;	break;
		case aborting:

			// 終了要求中であれば、終了要求を保留にする

			_status = running;
			_abortingReserved = ModTrue;		break;
		case interrupted:
		case aborted:

			// 中断処理中、終了処理中のときは、設定できない

			return ModFalse;
		}

		// 要求受付不可の回数を増やす

		_notInterruptable++;

	} else {

		// 必要であれば、要求受付不可の回数を減らす

		if (_notInterruptable)
			--_notInterruptable;

		if (this->isInterruptable())

			// 中断要求、終了要求の受付が可能になったので、
			// 保留されている中断要求または終了要求があれば、実行する

			(void) this->setStatus(running);
	}

	return ModTrue;
}

//
// FUNCTION public
// ModThread::getStatus -- 内部ステータスを得る
//
// NOTES
//	内部ステータスを返す。スレッドの終りぎわに「もうすぐ終る」意味の
//	exitSoonが設定されている場合は、終了しているかチェックし、
//	終了していればexitedに設定してから返す。
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
ModThread::Status 
ModThread::getStatus()
{
	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	m.lock();
	if (_status == exitSoon && _thread.isAlive() == ModFalse)

		// スレッドの状態がまもなく終了で、
		// 仮想 OS のスレッドが存在していなければ、
		// スレッドの状態を終了確認にする

		(void) this->setStatus(exited);

	return _status;
}
//
// FUNCTION public
// ModThread::setStatus -- 内部ステータスを設定する
//
// NOTES
//	内部ステータスを設定する。
//	設定に成功するとModTrueを返す。
//	中断処理をはじめる場合にはinterrupted、終了処理をはじめる場合には
//	abortedを設定する。中断処理が一段落してまた再開する場合にはrunningに
//	設定する必要がある。このとき、既に終了要求abort()を受け取って
//	いる場合にはその時点でステータスをabortingに変更し、ModFalseを返すので
//	注意する。
//
//	矛盾した設定であったり、直前のステータスに影響を及ぼす場合には、
//	ModFalseを返し、設定は失敗する。例えば、interrupting/interrupted時に
//	abortingは設定できるが(実際には設定待ちとなり、本関数だけが終了、
//	後で反映される)、逆(aborting/aborted時にinterrupting)は設定できない。
//	また、前述のように、中断処理後に終了要求を受け、runningに再設定できなかった
//	場合にもModFalseを返す。
//
// ARGUMENTS
//	ModThread::Status	v
//		設定する新たなステータス
// RETURN
//	設定できればModTrue、設定できなければModFalseを返す。
//
// EXCEPTIONS
//		ModOsDriver::Mutex::lock, unlock, ModOsDriver::ConditionVariable::reset、ModThread::abortの例外参照
//

ModBoolean
ModThread::setStatus(ModThread::Status v)
{
	ModBoolean	doSet = ModTrue;			// 指定された状態に変更するか
	ModBoolean	doInterrupt = ModFalse;		// 中断要求の受付が不可なために
											// 保留している中断要求を実行するか
	ModBoolean	doAbort = ModFalse;			// 中断要求、中断処理のために
											// 保留している終了要求を実行するか
	ModBoolean	ret = ModTrue;				// 返り値

	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	m.lock();

	switch (v) {
	case notStarted:
		doSet = ret = ModFalse;	break;

	case running:
		switch (_status) {
		case notStarted:
		case exited:
			break;
		case interrupting:

			// 中断要求が中断要求待ちしているスレッドに受理されていない

			// 条件変数がシグナル化されたままかもしれないので、
			// リセットする

			_dozeCondition.reset();
			// thru
		case interrupted:
			if (_abortingReserved == ModTrue) {

				// 中断要求、中断処理のために終了要求が保留されている

				doAbort = ModTrue;
				ret = ModFalse;
			}
			break;
		case running:
			if (this->isInterruptable()) {
				if (_abortingReserved == ModTrue)

					// 中断要求の受付が不可だったために終了要求が保留されている

					doAbort = ModTrue;
				else if (_interruptingReserved == ModTrue)

					// 中断要求の受付が不可だったために中断要求が保留されている

					doInterrupt = ModTrue;
				ret = ModFalse;
			}
			// thru
		default:
			doSet = ret = ModFalse;	break;
		}
		break;

	case interrupting:
		switch (_status) {
		case running:
			if (this->isInterruptable())
				break;

			// 中断要求の受付が不可のとき、状態を変更しない
			// ただし、設定できたことにする

			if (_abortingReserved == ModFalse)

				// 終了要求を保留していないので、中断要求を保留にする

				_interruptingReserved = ModTrue;
			// thru
		case interrupted:

			// すでに中断処理中のときは、状態を変更しない
			// ただし、設定できたことにする

			doSet = ModFalse;
			// thru
		case interrupting:
			break;
		default:
			doSet = ret = ModFalse;	break;
		}
		break;

	case interrupted:
		switch (_status) {
		case interrupting:

			// 誰も中断要求待ちしていなければ、
			// 条件変数のシグナル化待ちをしていないはずなので、
			// リセットしても問題ない
			//
			// 逆に、誰かが条件変数待ちしていても
			// ModThisThread::doze の実装により、
			// ここに来たということはすでに条件変数は
			// 非シグナル化されているのでリセットしても問題ない

			_dozeCondition.reset();
			// thru
		case interrupted:
			break;
		default:
			doSet = ret = ModFalse;	break;
		}
		break;

	case aborting:
		switch (_status) {
		case running:
			if (this->isInterruptable())
				break;

			// 中断要求の受付が不可のとき、状態を変更しない
			// ただし、設定できたことにする

			// thru
		case interrupting:
		case interrupted:

			// 中断要求中、中断処理中のときは、終了要求を保留し、
			// 状態を変更しない
			// スレッドが実行中に戻されたときに
			// 保留された終了要求を実行する

			_abortingReserved = ModTrue;
			// thru
		case aborted:

			// すでに終了処理中のときは、状態を変更しない
			// ただし、設定できたことにする

			doSet = ModFalse;
			// thru
		case aborting:
			break;
		default:
			doSet = ret = ModFalse;	break;
		}
		break;

	case aborted:
		switch (_status) {
		case aborting:
			_dozeCondition.reset();
			// thru
		case aborted:
			break;
		default:
			doSet = ret = ModFalse;	break;
		}
		break;

	case exitSoon:
		break;

	case exited:
		switch (_status) {
		case running:
		case exitSoon:
			break;
		default:
			doSet = ret = ModFalse;	break;
		}
		break;

	default:
		; ModAssert(ModFalse);
	}

	if (doSet == ModTrue)
		_status = v;

	if (doAbort == ModTrue) {

		// 保留されていた終了要求を実行する

	    _abortingReserved = ModFalse;

		; ModAssert(_status == running);
		(void) this->abort();
		; ModAssert(_status == aborting);

	} else if (doInterrupt == ModTrue) {

		// 保留されていた中断要求を実行する

		_interruptingReserved = ModFalse;

		; ModAssert(_status == running);
		(void) this->interrupt();
		; ModAssert(_status == interrupting);
	}

	return ret;
}

//
// FUNCTION public
// ModThread::print -- スレッドクラスの内容をデバッグ出力
//
// NOTES
//	本オブジェクトの内容をデバッグ出力する。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ないはず(デバッグメッセージの<<に依存する。)
//

void
ModThread::print() const
{
	ModDebugMessage << "ID: " << this->getThreadId() << ModEndl;

	ModDebugMessage << "status: ";
	switch(_status) {
	case notStarted:
		ModDebugMessage << "notStarted";	break;
	case running:
		ModDebugMessage << "running";		break;
	case interrupting:
		ModDebugMessage << "interrupting";	break;
	case interrupted:
		ModDebugMessage << "interrupted";	break;
	case aborting:
		ModDebugMessage << "aborting";		break;
	case aborted:
		ModDebugMessage << "aborted";		break;
	case exitSoon:
		ModDebugMessage << "exitSoon";		break;
	case exited:
		ModDebugMessage << "exited";		break;
	default:
		ModDebugMessage << "unknown";		break;
	}
	ModDebugMessage << ModEndl;
}

//
// FUNCTION public
// ModThread::addLockingMutex -- ロック中のミューテックスをリストに登録
//
// NOTES
//	ロック中のミューテックスをリストに登録する。
//	このリストはスレッドが強制終了される場合に、ロック中のミューテックス
//	を強制的にアンロックするために利用される。
//	本メソッドを含め、このリストはその目的から考えて本クラスが管理する
//	スレッドからしかアクセスされないことを前提とする。
//	(ロック時、アンロック時、シグナルハンドラの中でのアンロック時)
//	ロックをかけるとシグナルハンドラの中で面倒になるので、作業は
//	特にロックをかけずに行う。
//
// ARGUMENTS
//	ModSyncBase* sync
//		現在ロックしようとするミューテックス
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		ModList::pushBackの例外参照(メモリ関係のエラー?)
//

void
ModThread::addLockingMutex(ModSyncBase* sync)
{
	if (_lockingList == 0) {

		// このスレッドがロック中の同期オブジェクトを
		// 管理するためのリストを生成する

		_lockingList = new ModList<ModSyncBase*>();
		; ModAssert(_lockingList != 0);
	}

	_lockingList->pushBack(sync);
}

//
// FUNCTION public
// ModThread::deleteLockingMutex -- ロック中のミューテックスをリストから削除
//
// NOTES
//	ミューテックスをロック中リストから削除する。
//	このリストはスレッドが強制終了される場合に、ロック中のミューテックス
//	を強制的にアンロックするために利用される。アンロックされてから、
//	実際にリストからはずされるまでに時間差があるので、
//	リストを利用するときには実際にロックされているかどうかチェックする
//	必要がある。
//	本メソッドを含め、このリストはその目的から考えて本クラスが管理する
//	スレッドからしかアクセスされないことを前提とする。詳しくはaddLockingThreadの
//	項を参照のこと。
//
// ARGUMENTS
//	ModSyncBase* sync
//		アンロックしたばかりのミューテックス
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModOsErrorNotLocked
//		本スレッドの管理するロック中リストには存在しないエントリである
//

void
ModThread::deleteLockingMutex(ModSyncBase* sync)
{
	if (_lockingList != 0 && _forceUnlocking == ModFalse) {

		// 強制アンロック中でない

		ModListIterator<ModSyncBase*>	iterator(_lockingList->begin());
		const ModListIterator<ModSyncBase*>&	end = _lockingList->end();

		for (; iterator != end; ++iterator)
			if (*iterator == sync) {

				// このスレッドが指定された
				// 同期オブジェクトをロックしている

				_lockingList->erase(iterator);
				return;
			}

		// このスレッドがロックしていない同期オブジェクトが指定された

		ModThrow(ModModuleStandard,
				 ModCommonErrorEntryNotFound, ModErrorLevelError);
	}
}

//
// FUNCTION public
// ModThread::unlockLockingMutex -- 自スレッドがロック中のミューテックスを全てアンロック
//
// NOTES
//	スレッドが強制終了される場合に呼び出され、実行スレッドがロックしている
//	ミューテックスをすべて強制的にアンロックする。
//	本メソッドを含め、このリストはその目的から考えて本クラスが管理する
//	スレッドからしかアクセスされないことを前提とする。
//	詳しくはaddLockingThreadの項を参照のこと。
//	このリストをたどるのは、このスレッドだけのはずなので、
//	共通の汎用ライブラリミューテックスもはずして作業を行う。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS

void
ModThread::unlockLockingMutex()
{
	// ConditionVariableの中のmutexがかかっていたらアウト。
	// ロックはかけない。
	// 自分のリストをたどるのは自分=1スレッドだけ。

	// 強制アンロックを開始する

	_forceUnlocking = ModTrue;

	if (ModCommonMutex::getMutex()->isSelfLocked() == ModTrue)

		// 汎用ライブラリー用ミューテックスのロックをまずはずす

		ModCommonMutex::unlock();

	if (_lockingList != 0) {
		ModListIterator<ModSyncBase*>	iterator(_lockingList->begin());
		const ModListIterator<ModSyncBase*>&	end = _lockingList->end();

		for (; iterator != end; ++iterator) {
			ModSyncBase* sync = *iterator;
			if (sync->isSelfLocked() == ModTrue)
				try {
					// 入れ子ロックもすべてアンロックする。
					// sync->unlockAll();
					// もっと強力なメソッドでないと物足りないかもしれない。
					// さらに、このメソッドが成功するためには
					// CommonMutexなどのロックが必要となる。
					// unlockReally() ならよい。

					// ********** 未確認 ***********
					// しかし、killの後も正常に処理を続けようと思えば
					// 本当は ModOsMutex が保持しているロック情報は
					// 正しく更新しておかなければならないので
					// 専用のメソッドが必要。

					// killなら以下かも
					//sync->unlockAll();

					// killAll ならば以下でも構わない。

					// killはまだうまく動いていないので深く追求しない。
					sync->unlockReally();

				} catch (ModException) {

					// 発生した例外は無視して、処理を続行する

					ModErrorHandle::reset();
				}
		}

		// 登録されている同期オブジェクトをすべて削除する

		_lockingList->erase(_lockingList->begin(), end);
	}

	// 強制アンロックを終了する

	_forceUnlocking = ModFalse;
}

//
// FUNCTION public
// ModThread::getModThread -- スレッドIDからスレッド管理オブジェクトを得る(static)
//
// NOTES
//	スレッドのリストから指定されたスレッドIDをもつスレッドを管理する
//	スレッドオブジェクトを探して返す。存在しない場合は例外を送出する。
//	(その場合はModThreadとして作成されたスレッドでないか、メインスレッドの
//	可能性がある)
//
// ARGUMENTS
//	ModThreadId threadId
//
// RETURN
//	指定されたスレッドを管理するスレッドオブジェクト
//
// EXCEPTIONS
//	その他
//		ModCommonInitialize::checkAndInitialize(初期化前のみ)、ModOsDriver::Mutex::lock, unlockの例外参照
//	ModOsErrorModThreadNotFound
//		Modスレッドオブジェクトがみつからない
//

// static
ModThread*
ModThread::getModThread(ModThreadId id)
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	m.lock();

	ModSinglyLinkedListIteratorBase item(*_threadList);
	for (; *item; ++item) {
		ModThread*	thread = (ModThread*) *item;
		if (thread->getThreadId() == id)
			return thread;
	}

	ModThrowOsError(ModOsErrorModThreadNotFound);
	return 0;
}

//
// FUNCTION public
// ModThread::printList -- スレッドリストの内容をデバッグ出力
//
// NOTES
//	MODで利用中のスレッドオブジェクトすべての状態をデバッグ出力する。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		ModCommonInitialize::checkAndInitialize(初期化前のみ)、ModOsDriver::Mutex::lock, unlockの例外参照
//

// static
void
ModThread::printList()
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	ModAutoMutex<ModOsMutex>	m(ModCommonMutex::getMutex());
	m.lock();

	ModDebugMessage << "ModThreadList printing: ";
	ModDebugMessage << _threadList->getSize() << ModEndl;

	ModSinglyLinkedListIteratorBase item(*_threadList);
	for (; *item; ++item)
		((ModThread*) *item)->print();
}

// ** 以下はModThisThreadの関数
// **
// ** 下記のようなコーディングを想定している。
// **
// **	void routine(void* arg) {
// **		.......
// **		ModThisThead::exit(status);
// **	}
// **

//
// VARIABLE
// ModThisThread::mainException -- メインスレッドの例外オブジェクト
//
// NOTES
// MODを起動したスレッドで利用する例外オブジェクトである。
// 
//ModException ModThisThread::mainException;

//
// FUNCTION public
// ModThisThread::exit -- 実行中のスレッドの終了
//
// NOTES
//	実行中のスレッドを終了させる。引数に終了時のステータスを設定する。
//
// ARGUMENTS
//	int status
//		終了ステータス
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		ModCommonInitialize::checkAndInitialize(初期化前のみ)の例外参照
//

// static
void
ModThisThread::exit(int status)
{
	//【注意】	ModThisThread::getThread 内部で
	//			必要ならば汎用ライブラリーが初期化される

	ModThread*	thread = ModThisThread::getThread();
	if (thread != 0)

		// スレッドの状態をこれから終了することにする

		(void) thread->setStatus(ModThread::exitSoon);

	ModOsDriver::Thread::exit(status);
}

//
// FUNCTION public
// ModThisThread::sleep -- 実行中のスレッドを止める
//
// NOTES
//	実行中のスレッドを指定時間止める。
//
// ARGUMENTS
//	const ModTimeSpan& time
//		止める時間
//	ModSize time
//		止めるミリ秒単位の時間
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		ModCommonInitialize::checkAndInitialize(初期化前のみ)、ModOsDriver::Thread::sleepの例外参照
//

// static
void
ModThisThread::sleep(const ModTimeSpan& time)
{
	ModThisThread::sleep((ModSize)time.getTotalMilliSeconds());
}

// static
void
ModThisThread::sleep(ModSize time)
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	ModOsDriver::Thread::sleep(time);
}

//
// FUNCTION public
// ModThisThread::doze -- スレッド中断がない間実行中のスレッドを止める(1)
//
// NOTES
//	スレッド中断要求をチェックしながら、スレッド実行を指定時間止める。
//	引数には、止める時間とインターバルの時間をミリ秒単位の値で指定する。
//	細切れにスリープを実行し、インターバルごとに中断要求がないかチェックする。
//	スリープを細かく行うことにより、かえってスレッドの負荷が増えて
//	切り替えが行われない可能性がG-BASEその他のデバッグ上の経験から指摘されて
//	いるので、もう一つの条件変数版の方を使うことを推奨する。
//
//	最後の引数には、実行スレッドを管理するオブジェクトへのポインタを指定する。
//	0が指定されている場合には、関数内部でスレッドオブジェクトを調べる。
//
//	中断要求もしくは終了要求があればそれを検出し、
//	要求対応開始のsetStatus()まで、実行してから返る。
//
//	本関数内で中断要求があることを検出した段階で、即座にModTrueを返す。
//	指定時間が経過しても中断要求がなかった場合にはModFalseを返す。
//
// ARGUMENTS
//	ModSize time
//		止めるミリ秒単位の時間
//	ModSize	interval
//		中断要求をチェックするミリ秒単位の時間
//	ModThread* thread
//		対象となるスレッドオブジェクト
//
// RETURN
//	本関数内で中断要求があることを検出した段階で、即座にModTrueを返す。
//	指定時間が経過しても中断要求がなかった場合にはModFalseを返す。
//
// EXCEPTIONS
//	その他
//		ModCommonInitialize::checkAndInitialize(初期化前のみ)、ModOsDriver::Thread::sleepの例外参照
// 

// static
ModBoolean
ModThisThread::doze(const ModTimeSpan& time, const ModTimeSpan& interval,
					ModThread* thread)
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	if (thread == 0) {

		// 対象となるスレッドが指定されていないときは、
		// 自分自身を対象にする

		thread = ModThisThread::getThread();
		if (thread == 0)

			// メインスレッドは対象にできない

			ModThrowOsError(ModOsErrorMainThread);
	}

	ModSize	rest = (ModSize)time.getTotalMilliSeconds();
	ModSize	i = (ModSize)interval.getTotalMilliSeconds();

	while (rest > 0) {

		// 指定された間隔でスレッドの実行を停止する

		ModSize	t = (rest > i) ? i : rest;
		ModOsDriver::Thread::sleep(t);

		if (thread->isAborted() == ModTrue) {

			// スレッドは終了要求されている

			(void) thread->setStatus(ModThread::aborted);
			return ModTrue;
		}
		if (thread->isInterrupted() == ModTrue) {

			// スレッドは中断要求されている

			(void) thread->setStatus(ModThread::interrupted);
			return ModTrue;
		}

		rest -= t;
	}

	return ModFalse;
}

//
// FUNCTION public
// ModThisThread::doze -- スレッド中断がない間実行中のスレッドを止める(2)[条件変数版]
//
// NOTES
//	スレッド中断要求をチェックしながら、スレッド実行を指定時間止める。
//	タイムアウト付の条件変数で実装している。
//	引数には、止める時間をミリ秒単位の値で指定する。
//	最後の引数には、実行スレッドを管理するオブジェクトへのポインタを指定する。
//	0が指定されている場合には、関数内部でスレッドオブジェクトを調べる。
//	中断要求(interrupt)もしくは終了要求(abort)があればそれを検出し、
//	要求対応開始のsetStatus()まで、実行してから返る。
//
//	本関数内で中断要求があることを検出した段階で、即座にModTrueを返す。
//	指定時間が経過しても中断要求がなかった場合にはModFalseを返す。
//
// ARGUMENTS
//	ModSize time
//		止めるミリ秒単位の時間
//	ModSize interval
//		中断要求をチェックするミリ秒単位の時間
//	ModThread* thread
//		対象となるスレッドオブジェクト
//
// RETURN
//	本関数内で中断要求があることを検出した段階で、即座にModTrueを返す。
//	指定時間が経過しても中断要求がなかった場合にはModFalseを返す。
//
// EXCEPTIONS
//	その他
//		ModCommonInitialize::checkAndInitialize(初期化前のみ)、ModOsDriver::ConditionVariable::waitの例外参照
//

// static
ModBoolean
ModThisThread::doze(const ModTimeSpan& time, ModThread* thread)
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	if (thread == 0) {

		// 対象となるスレッドが指定されていないときは、
		// 自分自身を対象にする

		thread = ModThisThread::getThread();
		if (thread == 0)

			// メインスレッドは対象にできない

			ModThrowOsError(ModOsErrorMainThread);
	}

	// 指定された時間、中断要求を待つ

	if (thread->_dozeCondition.wait(time) == ModFalse)

		// 指定された時間待ったが、中断要求はなかった

		return ModFalse;

	if (thread->isAborted() == ModTrue)

		// スレッドは終了要求されている

		(void) thread->setStatus(ModThread::aborted);

	else if (thread->isInterrupted() == ModTrue)

		// スレッドは中断要求されている

		(void) thread->setStatus(ModThread::interrupted);

	else
		// こんなことはないはずなのだが、
		// スレッドはなぜか終了も中断も要求されていない

		return ModFalse;

	return ModTrue;
}

//
// FUNCTION public
// ModThisThread::self -- 現在実行中のスレッドのスレッドIDを得る。
//
// NOTES
//	現在実行中のスレッドのスレッドIDを返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	実行中のスレッドID
//
// EXCEPTIONS
//	その他
//		ModCommonInitialize::checkAndInitializeの例外(初期化前のみ)参照
//

// static
ModThreadId
ModThisThread::self()
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	// 自分自身のスレッド ID を求める

	return ModOsDriver::Thread::self();
}

//
// FUNCTION public
// ModThisThread::getThread -- 現在実行中のスレッドを管理するスレッドオブジェクトを得る
//
// NOTES
//	現在実行しているスレッドのスレッドオブジェクトを返す。
//	メインスレッドなどModThreadとして実行されていない場合には0を返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	実行中のスレッドを管理するスレッドオブジェクトへのポインタ
//
// EXCEPTIONS
//	その他
//		ModCommonInitialize::checkAndInitialize(初期化前のみ)の例外参照
//

// static
ModThread* 
ModThisThread::getThread()
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	// スレッド固有ストレージに記憶されている
	// 自分自身を表すスレッドクラスを得る
	//
	//【注意】	記録されていないときや、
	//			すでにスレッドが終了しているときは、0 を返す

	return (ModCommonInitialize::isExited() || !ModThread::_selfStoredKey)
		? 0 :
		(ModThread*) ModThread::_selfStoredKey->getValue();
}

//
// FUNCTION public
// ModThisThread::isInterrupted -- 中断要求があるかどうか調べる(static)
//
// NOTES
//	実行スレッドに対して、中断要求があるかどうかを調べる。
//
// ARGUMENTS
//	なし
//
// RETURN
//	中断要求があればModTrue、なければModFalseを返す。
//
// EXCEPTIONS
//	なし

// static
ModBoolean
ModThisThread::isInterrupted()
{
	//【注意】	ModThisThread::getThread 内部で
	//			必要ならば汎用ライブラリーが初期化される

	ModThread*	thread = ModThisThread::getThread();
	return (thread != 0) ? thread->isInterrupted() : ModFalse;
}

//
// FUNCTION public
// ModThisThread::isAborted -- 終了要求があるかどうか調べる(static)
//
// NOTES
//	実行スレッドに対して、終了要求があるかどうかを調べる。
//	中断処理中の終了要請は中断処理が終るまでModFalseを返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	中断要求があればModTrue、なければModFalseを返す。
//
// EXCEPTIONS
//	なし

// static
ModBoolean
ModThisThread::isAborted()
{
	//【注意】	ModThisThread::getThread 内部で
	//			必要ならば汎用ライブラリーが初期化される

	ModThread*	thread = ModThisThread::getThread();
	return (thread != 0) ? thread->isAborted() : ModFalse;
}

//
// FUNCTION public
// ModThisThread::getException -- 利用すべき例外オブジェクトへの参照を返す
//
// NOTES
// 必要なところでModThisThread::getException()の形で呼び出すことにより、
// 利用すべき例外オブジェクトを調べ、そのオブジェクトへの参照を返す。
//
// ARGUMENTS
// なし
// 
// RETURN
// 例外オブジェクトへの参照を返す
//
// EXCEPTIONS
//	その他
//		ModCommonInitialize::checkAndInitialize(初期化前のみ)の例外参照
//

// static
ModException*
ModThisThread::getException()
{
	// 自分自身を表すスレッドクラスを得て、
	// その中の例外を表すクラスを返す
	//
	//【注意】	自分自身がメインスレッドのときなどは
	//			スレッドクラスが記録されていないので、
	//			メインスレッド専用の例外を表すクラスを返す
	//
	//【注意】	ModThisThread::getThread 内部で
	//			必要ならば汎用ライブラリーが初期化される

	ModThread*	thread = ModThisThread::getThread();
	return (thread != 0) ? &(thread->exception) : 0;
}

//	FUNCTION public
//	ModThisThread::setInterruptable -- 中断要求、終了要求の受付の可否を設定する
//
//	NOTES
//
//	ARGUMENTS
//		ModBoolean			v
//			ModTrue
//				中断要求、終了要求の受付を可能にする
//			ModFalse
//				中断要求、終了要求の受付を不可にする
//
//	RETURN
//		ModTrue
//			指定されたように設定できた
//		ModFalse
//			呼び出しスレッドが
//			中断要求中、中断処理中、終了要求中、終了処理中のため、
//			集団要求、終了要求の受付を不可にできなかった
//			または、呼び出しスレッドがメインスレッドであるため、
//			設定できなかった
//
//	EXCEPTIONS

// static
ModBoolean
ModThisThread::setInterruptable(ModBoolean v)
{
	ModThread*	thread = ModThisThread::getThread();
	return (thread != 0) ? thread->setInterruptable(v) : ModFalse;
}

//
// Copyright (c) 1997, 1999, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
