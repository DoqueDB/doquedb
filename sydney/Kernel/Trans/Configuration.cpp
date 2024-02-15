// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Configuration.cpp -- トランザクションマネージャーの設定関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2006, 2007, 2011, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Trans";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"

#include "Trans/Configuration.h"

#include "Common/Assert.h"
#include "Common/SystemParameter.h"
#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"
#include "Os/Process.h"

#include "ModCharString.h"

_SYDNEY_USING
_SYDNEY_TRANS_USING

namespace 
{

namespace _Configuration
{
	// 設定値取得の排他制御用のラッチ
	Os::CriticalSection		_latch;

	namespace _TransTableSize
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _BranchTableSize
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _LogFileTableSize
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _InfoTableSize
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _Category
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		Transaction::Category::Value _value;
	}

	namespace _IsolationLevel
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		Transaction::IsolationLevel::Value _value;
	}

	namespace _NoLock
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		bool				_value;
	}

	namespace _NoLogicalLog
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		bool				_value;
	}

	namespace _CompressLogicalLog
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		bool				_value;
	}

	namespace _NoVersion
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		bool				_value;
	}

	namespace _TimeStampPath
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		Os::Path			_value;
	}

	namespace _TimeStampPermission
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

}

}

#ifdef OBSOLETE
//	FUNCTION
//	Trans::Configuration::get --
//		トランザクションマネージャーの設定を
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
	(void) TransTableSize::get();
	(void) BranchTableSize::get();
	(void) LogFileTableSize::get();
	(void) InfoTableSize::get();
	(void) Category::get();
	(void) IsolationLevel::get();
	(void) NoLock::get();
	(void) NoLogicalLog::get();
	(void) CompressLogicalLog::get();
	(void) NoVersion::get();
	(void) TimeStampPath::get();
	(void) TimeStampPermission::get();
}
#endif

//	FUNCTION
//	Trans::Configuration::reset --
//		トランザクションマネージャーの設定をリセットする
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
	(void) TransTableSize::reset();
	(void) BranchTableSize::reset();
	(void) LogFileTableSize::reset();
	(void) InfoTableSize::reset();
	(void) Category::reset();
	(void) IsolationLevel::reset();
	(void) NoLock::reset();
	(void) NoLogicalLog::reset();
	(void) CompressLogicalLog::reset();
	(void) NoVersion::reset();
	(void) TimeStampPath::reset();
	(void) TimeStampPermission::reset();
}

//	FUNCTION
//	Trans::Configuration::TransTableSize::get --
//		すべてのトランザクション記述子を管理する
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
Configuration::TransTableSize::get()
{
	// まずラッチの外で調べる
	// ★注意★
	// Configuration::resetがterminateからしか呼ばれないことが前提
	if (!_Configuration::_TransTableSize::_first)
		return _Configuration::_TransTableSize::_value;

	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	// ラッチの中で再度調べる
	if (_Configuration::_TransTableSize::_first) {

		// 値を求め、求めた値が 0 以下であれば、デフォルトとする

		; _SYDNEY_ASSERT(Default > 0);

		int	v;
		_Configuration::_TransTableSize::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v > 0) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_TransTableSize::_first = false;
	}
	return _Configuration::_TransTableSize::_value;
}

//	FUNCTION
//	Trans::Configuration::TransTableSize::reset --
//		すべてのトランザクション記述子を管理する
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
Configuration::TransTableSize::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_TransTableSize::_first = true;
}

//	FUNCTION
//	Trans::Configuration::BranchTableSize::get --
//		すべてのトランザクションブランチ記述子を管理する
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
Configuration::BranchTableSize::get()
{
	// まずラッチの外で調べる
	//
	//【注意】Configuration::reset が terminate からしか呼ばれないことが前提

	if (!_Configuration::_BranchTableSize::_first)
		return _Configuration::_BranchTableSize::_value;

	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	// ラッチの中で再度調べる

	if (_Configuration::_BranchTableSize::_first) {

		// 値を求め、求めた値が 0 以下であれば、デフォルトとする

		; _SYDNEY_ASSERT(Default > 0);

		int	v;
		_Configuration::_BranchTableSize::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v > 0) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_BranchTableSize::_first = false;
	}

	return _Configuration::_BranchTableSize::_value;
}

//	FUNCTION
//	Trans::Configuration::BranchTableSize::reset --
//		すべてのトランザクション記述子を管理する
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
Configuration::BranchTableSize::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_BranchTableSize::_first = true;
}

//	FUNCTION
//	Trans::Configuration::LogFileTableSize::get --
//		すべての論理ログファイルに関する情報を表すクラスを管理する
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
Configuration::LogFileTableSize::get()
{
	// まずラッチの外で調べる
	// ★注意★
	// Configuration::resetがterminateからしか呼ばれないことが前提
	if (!_Configuration::_LogFileTableSize::_first)
		return _Configuration::_LogFileTableSize::_value;

	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_LogFileTableSize::_first) {

		// 値を求め、求めた値が 0 以下であれば、デフォルトとする

		; _SYDNEY_ASSERT(Default > 0);

		int	v;
		_Configuration::_LogFileTableSize::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v > 0) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_LogFileTableSize::_first = false;
	}
	return _Configuration::_LogFileTableSize::_value;
}

//	FUNCTION
//	Trans::Configuration::LogFileTableSize::reset --
//		すべての論理ログファイルに関する情報を表すクラスを管理する
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
Configuration::LogFileTableSize::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_LogFileTableSize::_first = true;
}

//	FUNCTION
//	Trans::Configuration::InfoTableSize::get --
//		データベースごとのトランザクション情報を管理する
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
Configuration::InfoTableSize::get()
{
	// まずラッチの外で調べる
	// ★注意★
	// Configuration::resetがterminateからしか呼ばれないことが前提
	if (!_Configuration::_InfoTableSize::_first)
		return _Configuration::_InfoTableSize::_value;

	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	// ラッチの中で再度調べる
	if (_Configuration::_InfoTableSize::_first) {

		// 値を求め、求めた値が 0 以下であれば、デフォルトとする

		; _SYDNEY_ASSERT(Default > 0);

		int	v;
		_Configuration::_InfoTableSize::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v > 0) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_InfoTableSize::_first = false;
	}
	return _Configuration::_InfoTableSize::_value;
}

//	FUNCTION
//	Trans::Configuration::InfoTableSize::reset --
//		データベースごとのトランザクション情報を管理する
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
Configuration::InfoTableSize::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_InfoTableSize::_first = true;
}

//	FUNCTION
//	Trans::Configuration::Category::get --
//		デフォルトの種別の設定を取得する
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

Transaction::Category::Value
Configuration::Category::get()
{
	// まずラッチの外で調べる
	// ★注意★
	// Configuration::resetがterminateからしか呼ばれないことが前提
	if (!_Configuration::_Category::_first)
		return _Configuration::_Category::_value;

	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_Category::_first) {

		// 値を求め、求めた値が Transaction::Category::Value として
		// 適切な範囲でなければ、デフォルトとする
		//
		//【注意】	Transaction::Category::Unknown は不正でない

		; _SYDNEY_ASSERT(Default >= Transaction::Category::Unknown &&
						 Default < Transaction::Category::ValueNum);

		int	v;
		_Configuration::_Category::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v >= Transaction::Category::Unknown &&
			 v < Transaction::Category::ValueNum) ?
			static_cast<Transaction::Category::Value>(v) : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_Category::_first = false;
	}
	return _Configuration::_Category::_value;
}

//	FUNCTION
//	Trans::Configuration::Category::reset --
//		デフォルトの種別の設定をリセットする
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
Configuration::Category::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_Category::_first = true;
}

//	FUNCTION
//	Trans::Configuration::IsolationLevel::get --
//		デフォルトのアイソレーションレベルの設定を取得する
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

Transaction::IsolationLevel::Value
Configuration::IsolationLevel::get()
{
	// まずラッチの外で調べる
	// ★注意★
	// Configuration::resetがterminateからしか呼ばれないことが前提
	if (!_Configuration::_IsolationLevel::_first)
		return _Configuration::_IsolationLevel::_value;

	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_IsolationLevel::_first) {

		// 値を求め、求めた値が Transaction::IsolationLevel::Value として
		// 適切な範囲でなければ、デフォルトとする

		; _SYDNEY_ASSERT(Default > Transaction::IsolationLevel::Unknown &&
						 Default < Transaction::IsolationLevel::ValueNum);

		int	v;
		_Configuration::_IsolationLevel::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v > Transaction::IsolationLevel::Unknown &&
			 v < Transaction::IsolationLevel::ValueNum) ?
			static_cast<Transaction::IsolationLevel::Value>(v) : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_IsolationLevel::_first = false;
	}
	return _Configuration::_IsolationLevel::_value;
}

//	FUNCTION
//	Trans::Configuration::IsolationLevel::reset --
//		デフォルトのアイソレーションレベルの設定をリセットする
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
Configuration::IsolationLevel::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_IsolationLevel::_first = true;
}

//	FUNCTION
//	Trans::Configuration::NoLock::get --
//		トランザクションでロックするかの設定を取得する
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
Configuration::NoLock::get()
{
	// まずラッチの外で調べる
	// ★注意★
	// Configuration::resetがterminateからしか呼ばれないことが前提
	if (!_Configuration::_NoLock::_first)
		return _Configuration::_NoLock::_value;

	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_NoLock::_first) {

		// 値を求める

		bool	v;
		_Configuration::_NoLock::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v)) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_NoLock::_first = false;
	}
	return _Configuration::_NoLock::_value;
}

//	FUNCTION
//	Trans::Configuration::NoLock:reset --
//		トランザクションでロックするかの設定をリセットする
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
Configuration::NoLock::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_NoLock::_first = true;
}

//	FUNCTION
//	Trans::Configuration::NoLogicalLog::get --
//		更新トランザクションで論理ログを記録するかの設定を取得する
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
Configuration::NoLogicalLog::get()
{
	// まずラッチの外で調べる
	// ★注意★
	// Configuration::resetがterminateからしか呼ばれないことが前提
	if (!_Configuration::_NoLogicalLog::_first)
		return _Configuration::_NoLogicalLog::_value;

	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_NoLogicalLog::_first) {

		// 値を求める

		bool	v;
		_Configuration::_NoLogicalLog::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v)) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_NoLogicalLog::_first = false;
	}
	return _Configuration::_NoLogicalLog::_value;
}

//	FUNCTION
//	Trans::Configuration::NoLogicalLog::reset --
//		更新トランザクションで論理ログを記録するかの設定をリセットする
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
Configuration::NoLogicalLog::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_NoLogicalLog::_first = true;
}

//	FUNCTION
//	Trans::Configuration::CompressLogicalLog::get --
//		論理ログを圧縮して記録するかの設定を取得する
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
Configuration::CompressLogicalLog::get()
{
	// まずラッチの外で調べる
	// ★注意★
	// Configuration::resetがterminateからしか呼ばれないことが前提
	if (!_Configuration::_CompressLogicalLog::_first)
		return _Configuration::_CompressLogicalLog::_value;

	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_CompressLogicalLog::_first) {

		// 値を求める

		bool	v;
		_Configuration::_CompressLogicalLog::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v)) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_CompressLogicalLog::_first = false;
	}
	return _Configuration::_CompressLogicalLog::_value;
}

//	FUNCTION
//	Trans::Configuration::CompressLogicalLog::reset --
//		論理ログを圧縮して記録するかの設定をリセットする
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
Configuration::CompressLogicalLog::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_CompressLogicalLog::_first = true;
}

//	FUNCTION
//	Trans::Configuration::NoVersion::get --
//		トランザクションで版管理を使用しないかの設定を取得する
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
Configuration::NoVersion::get()
{
	// まずラッチの外で調べる
	// ★注意★
	// Configuration::resetがterminateからしか呼ばれないことが前提
	if (!_Configuration::_NoVersion::_first)
		return _Configuration::_NoVersion::_value;

	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_NoVersion::_first) {

		// 値を求める

		bool	v;
		_Configuration::_NoVersion::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v)) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_NoVersion::_first = false;
	}
	return _Configuration::_NoVersion::_value;
}

//	FUNCTION
//	Trans::Configuration::NoVersion:reset --
//		トランザクションで版管理を使用しないかの設定をリセットする
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
Configuration::NoVersion::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_NoVersion::_first = true;
}

//	FUNCTION
//	Trans::Configuration::TimeStampPath::get --
//		タイムスタンプの上位 32 ビットを記録するファイルを生成する
//		ディレクトリの絶対パス名の設定を取得する
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

const Os::Path&
Configuration::TimeStampPath::get()
{
	// まずラッチの外で調べる
	// ★注意★
	// Configuration::resetがterminateからしか呼ばれないことが前提
	if (!_Configuration::_TimeStampPath::_first)
		return _Configuration::_TimeStampPath::_value;

	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_TimeStampPath::_first) {

		// 値を求める

		Os::Path	v;
		_Configuration::_TimeStampPath::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v)) ?
			v :

			// 値が設定されていないので、
			// カレントワーキングディレクトリを求める

			Os::Process::getCurrentDirectory();

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_TimeStampPath::_first = false;
	}
	return _Configuration::_TimeStampPath::_value;
}

//	FUNCTION
//	Trans::Configuration::TimeStampPath::reset --
//		タイムスタンプの上位 32 ビットを記録するファイルを生成する
//		ディレクトリの絶対パス名の設定をリセットする
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
Configuration::TimeStampPath::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_TimeStampPath::_first = true;
}

//	FUNCTION
//	Trans::Configuration::TimeStampPermission::get --
//		タイムスタンプの上位 32 ビットを記録するファイルの
//		許可モードの設定を取得する
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
Configuration::TimeStampPermission::get()
{
	// まずラッチの外で調べる
	// ★注意★
	// Configuration::resetがterminateからしか呼ばれないことが前提
	if (!_Configuration::_TimeStampPermission::_first)
		return _Configuration::_TimeStampPermission::_value;

	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_TimeStampPermission::_first) {

		// 値を求め、求めた値がファイルの許可モードとして
		// 許されない値であれば、デフォルトとする

		; _SYDNEY_ASSERT(Default >= Os::File::Permission::None &&
						 Default <= Os::File::Permission::Mask);

		int	v;
		_Configuration::_TimeStampPermission::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v >= Os::File::Permission::None &&
			 v <= Os::File::Permission::Mask) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_TimeStampPermission::_first = false;
	}
	return _Configuration::_TimeStampPermission::_value;
}

//	FUNCTION
//	Trans::Configuration::TimeStampPermission::reset --
//		タイムスタンプの上位 32 ビットを記録するファイルの
//		許可モードの設定をリセットする
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
Configuration::TimeStampPermission::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_TimeStampPermission::_first = true;
}

//
// Copyright (c) 2000, 2001, 2002, 2004, 2006, 2007, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
