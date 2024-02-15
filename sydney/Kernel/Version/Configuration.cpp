// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Configuration.cpp -- 版管理マネージャーの設定関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Version";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"

#include "Version/Configuration.h"

#include "Common/Assert.h"
#include "Common/SystemParameter.h"
#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"

#include "ModAlgorithm.h"
#include "ModCharString.h"

_SYDNEY_USING
_SYDNEY_VERSION_USING

namespace
{

namespace _Configuration
{
	// 設定値取得の排他制御用のラッチ
	Os::CriticalSection		_latch;

	namespace _FileTableSize
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _PageTableSize
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _VerificationTableSize
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _MultiplexInfoTableSize
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _DetachedPageCleanerPeriod
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _CleanPageCoefficient
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _NoVersion
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		bool				_value;
	}

	namespace _SyncPageCountMax
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _MasterDataExtensionSize
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		Os::File::Size		_value;
	}

	namespace _VersionLogExtensionSize
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		Os::File::Size		_value;
	}

	namespace _SyncLogExtensionSize
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		Os::File::Size		_value;
	}

	namespace _MaxExtensionSize
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		Os::File::Size		_value;
	}

	namespace _PageInstanceCacheSize
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		int					_value;
	}

}

}

#ifdef OBSOLETE
//	FUNCTION
//	Version::Configuration::get --
//		版管理マネージャーの設定をすべてシステムパラメーターから読み出す
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
	(void) FileTableSize::get();
	(void) PageTableSize::get();
	(void) VerificationTableSize::get();
	(void) MultiplexInfoTableSize::get();
	(void) DetachedPageCleanerPeriod::get();
	(void) CleanPageCoefficient::get();
	(void) NoVersion::get();
	(void) SyncPageCountMax::get();
	(void) MasterDataExtensionSize::get();
	(void) VersionLogExtensionSize::get();
	(void) SyncLogExtensionSize::get();
	(void) MaxExtensionSize::get();
	(void) PageInstanceCacheSize::get();
}
#endif

//	FUNCTION
//	Version::Configuration::reset --
//		版管理マネージャーの設定をリセットする
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
	(void) FileTableSize::reset();
	(void) PageTableSize::reset();
	(void) VerificationTableSize::reset();
	(void) MultiplexInfoTableSize::reset();
	(void) DetachedPageCleanerPeriod::reset();
	(void) CleanPageCoefficient::reset();
	(void) NoVersion::reset();
	(void) SyncPageCountMax::reset();
	(void) MasterDataExtensionSize::reset();
	(void) VersionLogExtensionSize::reset();
	(void) SyncLogExtensionSize::reset();
	(void) MaxExtensionSize::reset();
	(void) PageInstanceCacheSize::reset();
}

//	FUNCTION
//	Version::Configuration::FileTableSize::get --
//		すべてのバージョンファイル記述子を管理する
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
Configuration::FileTableSize::get()
{
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_FileTableSize::_first) {

		// 値を求め、求めた値が 0 以下であれば、デフォルトとする

		; _SYDNEY_ASSERT(Default > 0);

		int	v;
		_Configuration::_FileTableSize::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v > 0) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_FileTableSize::_first = false;
	}
	return _Configuration::_FileTableSize::_value;
}

//	FUNCTION
//	Version::Configuration::FileTableSize::reset --
//		すべてのバージョンファイル記述子を管理する
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
Configuration::FileTableSize::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_FileTableSize::_first = true;
}

//	FUNCTION
//	Version::Configuration::PageTableSize::get --
//		あるバージョンファイルのすべてのバージョンページ記述子を管理する
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
Configuration::PageTableSize::get()
{
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_PageTableSize::_first) {

		// 値を求め、求めた値が 0 以下であれば、デフォルトとする

		; _SYDNEY_ASSERT(Default > 0);

		int	v;
		_Configuration::_PageTableSize::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v > 0) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_PageTableSize::_first = false;
	}
	return _Configuration::_PageTableSize::_value;
}

//	FUNCTION
//	Version::Configuration::PageTableSize::reset --
//		あるバージョンファイルのすべてのバージョンページ記述子を管理する
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
Configuration::PageTableSize::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_PageTableSize::_first = true;
}

//	FUNCTION
//	Version::Configuration::VerificationTableSize::get --
//		すべての整合性検査に関する情報を表すクラスを管理する
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
Configuration::VerificationTableSize::get()
{
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_VerificationTableSize::_first) {

		// 値を求め、求めた値が 0 以下であれば、デフォルトとする

		; _SYDNEY_ASSERT(Default > 0);

		int	v;
		_Configuration::_VerificationTableSize::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v > 0) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_VerificationTableSize::_first = false;
	}
	return _Configuration::_VerificationTableSize::_value;
}

//	FUNCTION
//	Version::Configuration::VerificationTableSize::reset --
//		すべての整合性検査に関する情報を表すクラスを管理する
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
Configuration::VerificationTableSize::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_VerificationTableSize::_first = true;
}

//	FUNCTION
//	Version::Configuration::MultiplexInfoTableSize::get --
//		あるバージョンログファイルの多重化されたブロックのうち、
//		どれを選択するか決めるための情報を表すクラスを
//		すべて管理するハッシュ表のサイズの設定を取得する
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
Configuration::MultiplexInfoTableSize::get()
{
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_MultiplexInfoTableSize::_first) {

		// 値を求め、求めた値が 0 以下であれば、デフォルトとする

		; _SYDNEY_ASSERT(Default > 0);

		int	v;
		_Configuration::_MultiplexInfoTableSize::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v > 0) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_MultiplexInfoTableSize::_first = false;
	}
	return _Configuration::_MultiplexInfoTableSize::_value;
}

//	FUNCTION
//	Version::Configuration::MultiplexInfoTableSize::reset --
//		あるバージョンログファイルの多重化されたブロックのうち、
//		どれを選択するか決めるための情報を表すクラスを
//		すべて管理するハッシュ表のサイズの設定をリセットする
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
Configuration::MultiplexInfoTableSize::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_MultiplexInfoTableSize::_first = true;
}

//	FUNCTION
//	Version::Configuration::DetachedPageCleanerPeriod::get --
//		参照済バージョンページ記述子破棄スレッドの停止時間の設定を取得する
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
Configuration::DetachedPageCleanerPeriod::get()
{
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_DetachedPageCleanerPeriod::_first) {

		// 値を求め、求めた値が 0 より小さければ、デフォルトとする

		; _SYDNEY_ASSERT(Default >= 0);

		int	v;
		_Configuration::_DetachedPageCleanerPeriod::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v >= 0) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_DetachedPageCleanerPeriod::_first = false;
	}
	return _Configuration::_DetachedPageCleanerPeriod::_value;
}

//	FUNCTION
//	Version::Configuration::DetachedPageCleanerPeriod::reset --
//		参照済バージョンページ記述子破棄スレッドの停止時間の設定をリセットする
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
Configuration::DetachedPageCleanerPeriod::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_DetachedPageCleanerPeriod::_first = true;
}

//	FUNCTION
//	Version::Configuration::CleanPageCoefficient::get --
//		あるバージョンファイルの参照済バージョンページ記述子のうち、
//		最大で何パーセント破棄するかの設定を取得する
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
Configuration::CleanPageCoefficient::get()
{
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_CleanPageCoefficient::_first) {

		// 値を求め、求めた値が 0 より小さければ 0 とし、
		// 100 より大きければ 100 とする

		; _SYDNEY_ASSERT(Default >= 0 && Default <= 100);

		int	v;
		_Configuration::_CleanPageCoefficient::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v)) ?
			ModMin(ModMax(0, v), 100) : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_CleanPageCoefficient::_first = false;
	}
	return _Configuration::_CleanPageCoefficient::_value;
}

//	FUNCTION
//	Version::Configuration::CleanPageCoefficient::reset --
//		あるバージョンファイルの参照済バージョンページ記述子のうち、
//		最大で何パーセント破棄するかの設定をリセットする
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
Configuration::CleanPageCoefficient::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_CleanPageCoefficient::_first = true;
}

//	FUNCTION
//	Version::Configuration::NoVersion::get --
//		バージョンをひとつしか生成できない
//		バージョンファイルとみなすかの設定を取得する
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
//	Version::Configuration::NoVersion::reset --
//		バージョンをひとつしか生成できない
//		バージョンファイルとみなすかの設定をリセットする
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
//	Version::Configuration::SyncPageCountMax::get --
//		一度に同期するバージョンページの最大数の設定を取得する
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
Configuration::SyncPageCountMax::get()
{
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_SyncPageCountMax::_first) {

		// 値を求め、求めた値が 0 以下であれば、デフォルトとする

		; _SYDNEY_ASSERT(Default > 0);

		int	v;
		_Configuration::_SyncPageCountMax::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v > 0) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_SyncPageCountMax::_first = false;
	}
	return _Configuration::_SyncPageCountMax::_value;
}

//	FUNCTION
//	Version::Configuration::SyncPageCountMax::reset --
//		一度に同期するバージョンページの最大数の設定をリセットする
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
Configuration::SyncPageCountMax::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_SyncPageCountMax::_first = true;
}

//	FUNCTION
//	Version::Configuration::MasterDataExtensionSize::get --
//		マスタデータファイルを拡張する単位の設定を取得する
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

Os::File::Size
Configuration::MasterDataExtensionSize::get()
{
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_MasterDataExtensionSize::_first) {

		// 値を求め、求めた値が 0 より小さければ、デフォルトとする
		//
		//【注意】	0 を指定すると、ファイル格納戦略に
		//			指定されたものかページサイズになる

		int	v;
		_Configuration::_MasterDataExtensionSize::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v >= 0) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_MasterDataExtensionSize::_first = false;
	}
	return _Configuration::_MasterDataExtensionSize::_value;
}

//	FUNCTION
//	Version::Configuration::MasterDataExtensionSize::reset --
//		マスタデータファイルを拡張する単位の設定をリセットする
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
Configuration::MasterDataExtensionSize::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_MasterDataExtensionSize::_first = true;
}

//	FUNCTION
//	Version::Configuration::VersionLogExtensionSize::get --
//		バージョンログファイルを拡張する単位の設定を取得する
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

Os::File::Size
Configuration::VersionLogExtensionSize::get()
{
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_VersionLogExtensionSize::_first) {

		// 値を求め、求めた値が 0 より小さければ、デフォルトとする
		//
		//【注意】	0 を指定すると、ファイル格納戦略に
		//			指定されたものかページサイズになる

		; _SYDNEY_ASSERT(Default > 0);

		int	v;
		_Configuration::_VersionLogExtensionSize::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v >= 0) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_VersionLogExtensionSize::_first = false;
	}
	return _Configuration::_VersionLogExtensionSize::_value;
}

//	FUNCTION
//	Version::Configuration::VersionLogExtensionSize::reset --
//		バージョンログファイルを拡張する単位の設定をリセットする
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
Configuration::VersionLogExtensionSize::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_VersionLogExtensionSize::_first = true;
}

//	FUNCTION
//	Version::Configuration::SyncLogExtensionSize::get --
//		同期ログファイルを拡張する単位の設定を取得する
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

Os::File::Size
Configuration::SyncLogExtensionSize::get()
{
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_SyncLogExtensionSize::_first) {

		// 値を求め、求めた値が 0 より小さければ、デフォルトとする
		//
		//【注意】	0 を指定すると、ファイル格納戦略に
		//			指定されたものかページサイズになる

		; _SYDNEY_ASSERT(Default > 0);

		int	v;
		_Configuration::_SyncLogExtensionSize::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v >= 0) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_SyncLogExtensionSize::_first = false;
	}
	return _Configuration::_SyncLogExtensionSize::_value;
}

//	FUNCTION
//	Version::Configuration::SyncLogExtensionSize::reset --
//		同期ログファイルを拡張する単位の設定をリセットする
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
Configuration::SyncLogExtensionSize::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_SyncLogExtensionSize::_first = true;
}

//	FUNCTION
//	Version::Configuration::MaxExtensionSize::get --
//		バージョンファイルを拡張する単位の最大値の設定を取得する
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

Os::File::Size
Configuration::MaxExtensionSize::get()
{
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	if (_Configuration::_MaxExtensionSize::_first) {

		// 値を求め、求めた値が 0 より小さければ、デフォルトとする
		//
		//【注意】	0 を指定すると、ファイル格納戦略に
		//			指定されたものかページサイズになる

		; _SYDNEY_ASSERT(Default > 0);

		int	v;
		_Configuration::_MaxExtensionSize::_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(Name), v) &&
			 v >= 0) ?
			v : Default;

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_Configuration::_MaxExtensionSize::_first = false;
	}
	return _Configuration::_MaxExtensionSize::_value;
}

//	FUNCTION
//	Version::Configuration::MaxExtensionSize::reset --
//		バージョンファイルを拡張する単位の最大値の設定をリセットする
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
Configuration::MaxExtensionSize::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_MaxExtensionSize::_first = true;
}

//	FUNCTION
//	Version::Configuration::PageInstanceCacheSize::get --
//		ページのインスタンスをキャッシュする数
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

int
Configuration::PageInstanceCacheSize::get()
{
	if (_Configuration::_PageInstanceCacheSize::_first) {

		// 設定値取得の排他制御のためにラッチする

		Os::AutoCriticalSection	latch(_Configuration::_latch);

		if (_Configuration::_PageInstanceCacheSize::_first) {

			// 値を求め、求めた値が 0 より小さければ、デフォルトとする

			; _SYDNEY_ASSERT(Default > 0);

			int	v;
			_Configuration::_PageInstanceCacheSize::_value =
				(Common::SystemParameter::getValue(
					ModCharString(moduleName).append('_').append(Name), v) &&
				 v >= 0) ?
				v : Default;

			// 次はパラメーターを取得せずに
			// 記憶している値をそのまま返すようにする

			_Configuration::_PageInstanceCacheSize::_first = false;
		}
	}
	return _Configuration::_PageInstanceCacheSize::_value;
}

//	FUNCTION
//	Version::Configuration::PageInstanceCacheSize::reset --
//		ページのインスタンスをキャッシュする数
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
Configuration::PageInstanceCacheSize::reset()
{
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);

	_Configuration::_PageInstanceCacheSize::_first = true;
}

//
// Copyright (c) 2000, 2001, 2002, 2004, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
