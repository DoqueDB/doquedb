// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TimeStamp.cpp -- タイムスタンプに関する処理を行うクラス関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2010, 2015, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Checkpoint";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Checkpoint/Configuration.h"
#include "Checkpoint/Manager.h"
#include "Checkpoint/TimeStamp.h"

#include "Common/Assert.h"
#include "Lock/Name.h"
#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"
#include "Trans/TimeStamp.h"

#include "ModHashMap.h"

_SYDNEY_USING
_SYDNEY_CHECKPOINT_USING

namespace
{

namespace _TimeStamp
{
	// あるデータベースに関する
	// 前回のチェックポイント処理終了時のタイムスタンプを得る
	const Trans::TimeStamp&
	getMostRecent(Lock::Name::Part dbPart);
	// あるデータベースに関する
	// 前々回のチェックポイント処理終了時のタイムスタンプを得る
	const Trans::TimeStamp&
	getSecondMostRecent(Lock::Name::Part dbPart);

	// 前回のチェックポイント処理終了時のタイムスタンプ
	Trans::TimeStamp			_mostRecent;
	// 前々回のチェックポイント処理終了時のタイムスタンプ
	Trans::TimeStamp			_secondMostRecent;

	typedef ModHashMap<Lock::Name::Part, Trans::TimeStamp,
					   ModHasher<Lock::Name::Part> > _HashMap;

	// 以下の2つのマップを保護するためのラッチ
	Os::CriticalSection			_cLatch;

	// データベースごとに
	// 前回のチェックポイント処理終了時のタイムスタンプを管理するハッシュ表
	_HashMap*					_mostRecentMap = 0;
	// データベースごとに
	// 前々回のチェックポイント処理終了時のタイムスタンプを管理するハッシュ表
	_HashMap*					_secondMostRecentMap = 0;
}

//	FUNCTION
//	$$$::_TimeStamp::getSecondMostRecent --
//		あるデータベースに関する
//		前回のチェックポイント処理終了時のタイムスタンプを得る
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name::Part	dbPart
//			データベースを表すロック名の構成部
//
//	RETURN
//		得られたタイムスタンプ
//
//	EXCEPTIONS
//		なし

const Trans::TimeStamp&
_TimeStamp::getMostRecent(Lock::Name::Part dbPart)
{
	//	呼び出し側で、Checkpoint::Daemon::disable() を呼び出していても、
	//	データベース定義時に、Checkpoint::Executor::cause が実行されるので、
	//	マップを排他する必要がある

	Os::AutoCriticalSection cAuto(_cLatch);
	
	if (_mostRecentMap) {
		const _HashMap* map = _mostRecentMap;
		_HashMap::ConstIterator ite = map->find(dbPart);
		if (ite != map->end())
			return map->getValue(ite);
	}
	return TimeStamp::getMostRecent();
}

//	FUNCTION
//	$$$::_TimeStamp::getSecondMostRecent --
//		あるデータベースに関する
//		前々回のチェックポイント処理終了時のタイムスタンプを得る
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name::Part	dbPart
//			データベースを表すロック名の構成部
//
//	RETURN
//		得られたタイムスタンプ
//
//	EXCEPTIONS
//		なし

const Trans::TimeStamp&
_TimeStamp::getSecondMostRecent(Lock::Name::Part dbPart)
{
	//	呼び出し側で、Checkpoint::Daemon::disable() を呼び出していても、
	//	データベース定義時に、Checkpoint::Executor::cause が実行されるので、
	//	マップを排他する必要がある

	Os::AutoCriticalSection cAuto(_cLatch);
	
	if (_secondMostRecentMap) {
		const _HashMap* map = _secondMostRecentMap;
		_HashMap::ConstIterator ite = map->find(dbPart);
		if (ite != map->end())
			return map->getValue(ite);
	}
	return TimeStamp::getSecondMostRecent();
}

}

//	FUNCTION private
//	Checkpoint::Manager::TimeStamp::initialize --
//		マネージャーの初期化のうち、タイムスタンプ関連の初期化を行う
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

void
Manager::TimeStamp::initialize()
{}

//	FUNCTION private
//	Checkpoint::Manager::TimeStamp::terminate --
//		マネージャーの後処理のうち、タイムスタンプ関連の後処理を行う
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

void
Manager::TimeStamp::terminate()
{
	Os::AutoCriticalSection cAuto(_TimeStamp::_cLatch);
	
	delete _TimeStamp::_mostRecentMap, _TimeStamp::_mostRecentMap = 0;
	delete _TimeStamp::_secondMostRecentMap,
		_TimeStamp::_secondMostRecentMap = 0;

	_TimeStamp::_mostRecent = Trans::IllegalTimeStamp;
	_TimeStamp::_secondMostRecent = Trans::IllegalTimeStamp;
}

//	FUNCTION public
//	Checkpoint::TimeStamp::getMostRecent --
//		前回のチェックポイント処理終了時のタイムスタンプを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたタイムスタンプ
//
//	EXCEPTIONS
//		なし

// static
const Trans::TimeStamp&
TimeStamp::getMostRecent()
{
	//【注意】	呼び出し側で
	//			Checkpoint::Daemon::disable() を呼び出していること

	return (_TimeStamp::_mostRecent.isIllegal()) ?
		Trans::TimeStamp::getSystemInitialized() : _TimeStamp::_mostRecent;
}

//	FUNCTION public
//	Checkpoint::TimeStamp::getMostRecent --
//		あるデータベースに関する
//		前回のチェックポイント処理終了時のタイムスタンプを得る
//
//	NOTES
//
//	ARGUMENTS
//		Lock::FileName&		lockName
//			データベースに所属するファイルのロック名
//
//	RETURN
//		得られたタイムスタンプ
//
//	EXCEPTIONS
//		なし

// static
const Trans::TimeStamp&
TimeStamp::getMostRecent(const Lock::FileName& lockName)
{
	//【注意】	呼び出し側で
	//			Checkpoint::Daemon::disable() を呼び出していること

	return _TimeStamp::getMostRecent(lockName.getDatabasePart());
}

//	FUNCTION public
//	Checkpoint::TimeStamp::getMostRecent --
//		あるデータベースに関する
//		前回のチェックポイント処理終了時のタイムスタンプを得る
//
//	NOTES
//
//	ARGUMENTS
//		Lock::LogicalLogName&		lockName
//			データベース用の論理ログファイルのロック名
//
//	RETURN
//		得られたタイムスタンプ
//
//	EXCEPTIONS
//		なし

// static
const Trans::TimeStamp&
TimeStamp::getMostRecent(const Lock::LogicalLogName& lockName)
{
	//【注意】	呼び出し側で
	//			Checkpoint::Daemon::disable() を呼び出していること

	return _TimeStamp::getMostRecent(lockName.getDatabasePart());
}

//	FUNCTION public
//	Checkpoint::TimeStamp::getSecondMostRecent --
//		前々回のチェックポイント処理終了時のタイムスタンプを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたタイムスタンプ
//
//	EXCEPTIONS
//		なし

// static
const Trans::TimeStamp&
TimeStamp::getSecondMostRecent()
{
	//【注意】	呼び出し側で
	//			Checkpoint::Daemon::disable() を呼び出していること

	return (_TimeStamp::_secondMostRecent.isIllegal()) ?
		Trans::TimeStamp::getSystemInitialized() :
		_TimeStamp::_secondMostRecent;
}

//	FUNCTION
//	Checkpoint::TimeStamp::getSecondMostRecent --
//		あるデータベースに関する
//		前々回のチェックポイント処理終了時のタイムスタンプを得る
//
//	NOTES
//
//	ARGUMENTS
//		Lock::FileName&		lockName
//			データベースに所属するファイルのロック名
//
//	RETURN
//		得られたタイムスタンプ
//
//	EXCEPTIONS
//		なし

// static
const Trans::TimeStamp&
TimeStamp::getSecondMostRecent(const Lock::FileName& lockName)
{
	//【注意】	呼び出し側で
	//			Checkpoint::Daemon::disable() を呼び出していること

	return _TimeStamp::getSecondMostRecent(lockName.getDatabasePart());
}

//	FUNCTION
//	Checkpoint::TimeStamp::getSecondMostRecent --
//		あるデータベースに関する
//		前々回のチェックポイント処理終了時のタイムスタンプを得る
//
//	NOTES
//
//	ARGUMENTS
//		Lock::LogicalLogName&		lockName
//			データベース用の論理ログファイルのロック名
//
//	RETURN
//		得られたタイムスタンプ
//
//	EXCEPTIONS
//		なし

// static
const Trans::TimeStamp&
TimeStamp::getSecondMostRecent(const Lock::LogicalLogName& lockName)
{
	//【注意】	呼び出し側で
	//			Checkpoint::Daemon::disable() を呼び出していること

	return _TimeStamp::getSecondMostRecent(lockName.getDatabasePart());
}

//	FUNCTION private
//	Checkpoint::TimeStamp::assign --
//		チェックポイント処理が終了したときのタイムスタンプを新たに設定する
//
//	NOTES
//		チェックポイントスレッドから呼び出される
//
//	ARGUMENTS
//		Trans::TimeStamp&	v
//			新たに設定するタイムスタンプ
//		bool				synchronized
//			true
//				チェックポイント処理の終了時に
//				バッファとディスクの内容が完全に一致している
//			false
//				チェックポイント処理の終了時に
//				バッファとディスクの内容が完全に一致していない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
TimeStamp::assign(const Trans::TimeStamp& v, bool synchronized)
{
	//	参照側で、Checkpoint::Daemon::disable() を呼び出していても、
	//	データベース定義時に、Checkpoint::Executor::cause が実行されるので、
	//	マップを排他する必要がある

	Os::AutoCriticalSection cAuto(_TimeStamp::_cLatch);
	
	//【注意】	チェックポイントスレッドから呼び出されること

	; _SYDNEY_ASSERT(!v.isIllegal());

	// 前回のチェックポイント処理の終了時タイムスタンプを
	// 前々回のチェックポイント処理の終了時タイムスタンプとし、
	// 指定されたタイムスタンプを
	// 前回のチェックポイント処理の終了時タイムスタンプとする
	//
	//【注意】	バッファとディスクの内容が完全に一致していれば、
	//			前々回のチェックポイント処理の終了時タイムスタンプも
	//			指定されたタイムスタンプとすることにより、
	//			前々回でなく前回のチェックポイント処理の
	//			終了時点からの障害回復を可能にする

	_TimeStamp::_secondMostRecent =
		(synchronized) ? v : _TimeStamp::_mostRecent;
	_TimeStamp::_mostRecent = v;

	// データベースごとの前回のチェックポイント処理終了時のタイムスタンプは、
	// 前々回のチェックポイント処理終了時のタイムスタンプとし、
	// データベースごとの前回のチェックポイント処理終了時の
	// タイムスタンプは忘れる

	if (synchronized) {
		delete _TimeStamp::_secondMostRecentMap,
			_TimeStamp::_secondMostRecentMap = 0;
		delete _TimeStamp::_mostRecentMap,
			_TimeStamp::_mostRecentMap = 0;
	} else
		delete _TimeStamp::_secondMostRecentMap,
		_TimeStamp::_secondMostRecentMap = _TimeStamp::_mostRecentMap,
		_TimeStamp::_mostRecentMap = 0;
}

//	FUNCTION private
//	Checkpoint::TimeStamp::assign --
//		あるデータベースに関する
//		チェックポイント処理が終了したときのタイムスタンプを新たに設定する
//
//	NOTES
//
//	ARGUMENTS
//		Lock::LogicalLogName&	lockName
//			データベース用の論理ログファイルのロック名
//		Trans::TimeStamp&	v
//			新たに設定するタイムスタンプ
//		bool				synchronized
//			true
//				チェックポイント処理の終了時に
//				バッファとディスクの内容が完全に一致している
//			false
//				チェックポイント処理の終了時に
//				バッファとディスクの内容が完全に一致していない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
TimeStamp::assign(const Lock::LogicalLogName& lockName,
				  const Trans::TimeStamp& v, bool synchronized)
{
	//	参照側で、Checkpoint::Daemon::disable() を呼び出していても、
	//	データベース定義時に、Checkpoint::Executor::cause が実行されるので、
	//	マップを排他する必要がある

	Os::AutoCriticalSection cAuto(_TimeStamp::_cLatch);
	
	; _SYDNEY_ASSERT(!v.isIllegal());

	if (!_TimeStamp::_mostRecentMap) {
		_TimeStamp::_mostRecentMap = new _TimeStamp::_HashMap(
			Configuration::TimeStampTableSize::get(), ModFalse);
		; _SYDNEY_ASSERT(_TimeStamp::_mostRecentMap);
	}

	// システム全体のチェックポイント処理の終了時タイムスタンプと同様に
	// あるデータベースのチェックポイント処理の終了時タイムスタンプも扱う

	const Lock::Name::Part dbID = lockName.getDatabasePart();
	const Trans::TimeStamp& second =
		(synchronized) ? v : (*_TimeStamp::_mostRecentMap)[dbID];

	if (!second.isIllegal()) {
		if (!_TimeStamp::_secondMostRecentMap) {
			_TimeStamp::_secondMostRecentMap = new _TimeStamp::_HashMap(
				Configuration::TimeStampTableSize::get(), ModFalse);
			; _SYDNEY_ASSERT(_TimeStamp::_secondMostRecentMap);
		}

		(*_TimeStamp::_secondMostRecentMap)[dbID] = second;
	}

	(*_TimeStamp::_mostRecentMap)[dbID] = v;
}

//
// Copyright (c) 2000, 2001, 2002, 2004, 2010, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
