// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Configuration.cpp -- チェックポイント処理マネージャーの設定関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Checkpoint";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"

#include "Checkpoint/Configuration.h"

#include "Common/Assert.h"
#include "Common/SystemParameter.h"
#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"

#include "ModCharString.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_CHECKPOINT_USING

namespace 
{

namespace _Configuration
{
	// 設定値取得の排他制御用のラッチ
	Os::CriticalSection		_latch;

	namespace _Period
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _TruncateLogicalLog
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		bool				_value;
	}

	namespace _EnableFileSynchronizer
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		Configuration::EnableFileSynchronizer::Value	_value;

		ModUnicodeString _size("size");
		ModUnicodeString _speed("speed");
		ModUnicodeString _off("off");

		ModUnicodeString _true("true");		// 過去との互換性のため
		ModUnicodeString _false("false");	// 過去との互換性のため
	}

	namespace _TimeStampTableSize
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _LoadSynchronizeCandidate
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		bool				_value;
	}
}

}

#ifdef OBSOLETE
//	FUNCTION
//	Checkpoint::Configuration::get --
//		チェックポイント処理マネージャーの設定を
//		すべてシステムパラメーターから読み出す
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
Configuration::get()
{
	(void) Period::get();
	(void) TruncateLogicalLog::get();
	(void) EnableFileSynchronizer::get();
	(void) TimeStampTableSize::get();
	(void) LoadSynchronizeCandidate::get();
}
#endif

//	FUNCTION
//	Checkpoint::Configuration::reset --
//		チェックポイント処理マネージャーの設定をリセットする
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
Configuration::reset()
{
	(void) Period::reset();
	(void) TruncateLogicalLog::reset();
	(void) EnableFileSynchronizer::reset();
	(void) TimeStampTableSize::reset();
	(void) LoadSynchronizeCandidate::reset();
}

//	FUNCTION
//	Checkpoint::Configuration::Period::get --
//		チェックポイント処理と処理の間の時間の設定を取得する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		取得した設定値
//
//	EXCEPTIONS

unsigned int
Configuration::Period::get()
{
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_Period::_first) {

		// 値を求め、求めた値が 0 より小さければ、デフォルトとする

		; _SYDNEY_ASSERT(Default >= 0);

		int	v;
		_Configuration::_Period::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v >= 0) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_Period::_first = false;
	}
	return _Configuration::_Period::_value;
}

//	FUNCTION
//	Checkpoint::Configuration::Period::reset --
//		チェックポイント処理と処理の間の時間の設定をリセットする
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
Configuration::Period::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_Period::_first = true;
}

//	FUNCTION
//	Checkpoint::Configuration::TruncateLogicalLog::get --
//		チェックポイント処理の終了時に可能であれば
//		論理ログファイルをトランケートするかの設定を取得する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		取得した設定値
//
//	EXCEPTIONS

bool
Configuration::TruncateLogicalLog::get()
{
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_TruncateLogicalLog::_first) {

		// 値を求める

		bool	v;
		_Configuration::_TruncateLogicalLog::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v)) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_TruncateLogicalLog::_first = false;
	}
	return _Configuration::_TruncateLogicalLog::_value;
}

//	FUNCTION
//	Checkpoint::Configuration::TruncateLogicalLog::reset --
//		チェックポイント処理の終了時に可能であれば
//		論理ログファイルをトランケートするかの設定をリセットする
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
Configuration::TruncateLogicalLog::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_TruncateLogicalLog::_first = true;
}

//	FUNCTION
//	Checkpoint::Configuration::EnableFileSynchronizer::get --
//		バージョンファイル同期スレッドを起動するかの設定を取得する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		取得した設定値
//
//	EXCEPTIONS

Configuration::EnableFileSynchronizer::Value
Configuration::EnableFileSynchronizer::get()
{
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_EnableFileSynchronizer::_first) {

		// 値を求める

		_Configuration::_EnableFileSynchronizer::_value = Default;
		ModUnicodeString v;
		if (Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v))
		{
			if (v.compare(_Configuration::_EnableFileSynchronizer::_size,
						  ModFalse) == 0
				|| v.compare(_Configuration::_EnableFileSynchronizer::_true,
							 ModFalse) == 0)
				// SIZE or TRUE
				_Configuration::_EnableFileSynchronizer::_value = SIZE;
			else if (v.compare(_Configuration::_EnableFileSynchronizer::_off,
							   ModFalse) == 0
					 || v.compare(
						 _Configuration::_EnableFileSynchronizer::_false,
						 ModFalse) == 0)
				// OFF or FALSE
				_Configuration::_EnableFileSynchronizer::_value = OFF;
			else if (v.compare(_Configuration::_EnableFileSynchronizer::_speed,
							   ModFalse) == 0)
				// SPEED
				_Configuration::_EnableFileSynchronizer::_value = SPEED;
		}

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_EnableFileSynchronizer::_first = false;
	}
	return _Configuration::_EnableFileSynchronizer::_value;
}

//	FUNCTION
//	Checkpoint::Configuration::EnableFileSynchronizer::reset --
//		バージョンファイル同期スレッドを起動するかの設定をリセットする
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
Configuration::EnableFileSynchronizer::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_EnableFileSynchronizer::_first = true;
}

//	FUNCTION
//	Checkpoint::Configuration::TimeStampTableSize::get --
//		データベースごとにチェックポイント時のタイムスタンプを管理する
//		ハッシュ表のサイズの設定を取得する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		取得した設定値
//
//	EXCEPTIONS

unsigned int
Configuration::TimeStampTableSize::get()
{
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_TimeStampTableSize::_first) {

		// 値を求め、求めた値が 0 以下であれば、デフォルトとする

		; _SYDNEY_ASSERT(Default > 0);

		int	v;
		_Configuration::_TimeStampTableSize::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v > 0) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_TimeStampTableSize::_first = false;
	}
	return _Configuration::_TimeStampTableSize::_value;
}

//	FUNCTION
//	Checkpoint::Configuration::TimeStampTableSize::reset --
//		データベースごとにチェックポイント時のタイムスタンプを管理する
//		ハッシュ表のサイズの設定をリセットする
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
Configuration::TimeStampTableSize::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_TimeStampTableSize::_first = true;
}

//	FUNCTION
//	Checkpoint::Configuration::LoadSynchronizeCandiate::get --
//	   	起動後に同期処理候補をすべてロードするかの設定を取得する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		取得した設定値
//
//	EXCEPTIONS

bool
Configuration::LoadSynchronizeCandidate::get()
{
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_LoadSynchronizeCandidate::_first) {

		// 値を求める

		bool	v;
		_Configuration::_LoadSynchronizeCandidate::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v)) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_LoadSynchronizeCandidate::_first = false;
	}
	return _Configuration::_LoadSynchronizeCandidate::_value;
}

//	FUNCTION
//	Checkpoint::Configuration::LoadSynchronizeCandidate::reset --
//	   	起動後に同期処理候補をすべてロードするかの設定をリセットする
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
Configuration::LoadSynchronizeCandidate::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_LoadSynchronizeCandidate::_first = true;
}

//
// Copyright (c) 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
