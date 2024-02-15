// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Configuration.cpp -- バッファ管理マネージャーの設定関連の関数定義
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2011, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Buffer";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"

#include "Buffer/Configuration.h"

#define GET_ALL_AT_ONCE
#undef CONCURRENT_EXECUTION

#include "Common/Assert.h"
#include "Common/SystemParameter.h"
#ifdef CONCURRENT_EXECUTION
#include "Os/AutoCriticalSection.h"
#endif
#include "Os/File.h"
#include "Os/SysConf.h"

#include "ModCharString.h"

_SYDNEY_USING
_SYDNEY_BUFFER_USING

namespace 
{

namespace _Configuration
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御用のラッチ
	Os::CriticalSection		_latch;
#endif

	// 文字列をModInt64に変換する(オーバーフローはチェックしない)
	ModInt64 toInt(const ModUnicodeChar* p)
	{
		ModInt64 i = 0;
		while (*p)
		{
			if (*p >= '0' && *p <= '9')
			{
				i *= 10;
				i += (*p - '0');
			}
			else if (*p == 'K' || *p == 'k')
				i = i << 10;
			else if (*p == 'M' || *p == 'm')
				i = i << 10 << 10;
			else if (*p == 'G' || *p == 'g')
				i = i << 10 << 10 << 10;
			else if (*p == 'T' || *p == 't')
				i = i << 10 << 10 << 10 << 10;
			else
				break;
			++p;
		}
		return i;
	}

	namespace _FileTableSize
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _FilePermission
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _PageTableSize
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _PageSizeMax
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		Os::Memory::Size	_value;
	}

	namespace _DirtyPageFlusherPeriod
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _FlushPageCoefficient
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _FreePageCountMax
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _KeepingUsedMemoryTimeMax
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _NormalPoolSize
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool					_first = true;
		// 取得した設定値
		Os::Memory::LongSize	_value;
	}

	namespace _TemporaryPoolSize
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool					_first = true;
		// 取得した設定値
		Os::Memory::LongSize	_value;
	}

	namespace _ReadOnlyPoolSize
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool					_first = true;
		// 取得した設定値
		Os::Memory::LongSize	_value;
	}

	namespace _LogicalLogPoolSize
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool					_first = true;
		// 取得した設定値
		Os::Memory::LongSize	_value;
	}

	namespace _OpenFileCountMax
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;

		// バッファファイル以外の消費用の予約数
		const unsigned int	Reserved = 100;
	}

	namespace _CalculateCheckSum
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		Configuration::CalculateCheckSum::Value	_value;
	}

	namespace _DelayTemporaryCreation
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		bool				_value;
	}

	namespace _RetryAllocationCountMax
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _FlushingBodyCountMax
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _SkipDirtyCandidateCountMax
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _ReadAheadBlockSize
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _StatisticsReporterPeriod
	{
		// 取得する準備を行う
		void
		prepare();

		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}
}

//	FUNCTION
//	$$$::_Configuration::_FileTableSize::prepare --
//		すべてのバッファファイル記述子を管理する
//		ハッシュ表のサイズの設定の取得を準備する
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
_Configuration::_FileTableSize::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値が 0 以下であれば、デフォルトとする

		; _SYDNEY_ASSERT(Configuration::FileTableSize::Default > 0);

		int	v;
		_value = (Common::SystemParameter::getValue(
					  ModCharString(moduleName).append('_').
					  append(Configuration::FileTableSize::Name), v) &&
				  v > 0) ?
			v : Configuration::FileTableSize::Default;

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_FilPermission::prepare --
//		バッファファイルの実体である OS ファイルの
//		許可モードの設定の取得を準備する
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
_Configuration::_FilePermission::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値がファイルの許可モードとして
		// 許されない値であれば、デフォルトとする

		; _SYDNEY_ASSERT(Configuration::FilePermission::Default >=
						 Os::File::Permission::None &&
						 Configuration::FilePermission::Default <=
						 Os::File::Permission::Mask);

		int	v;
		_value =
			(Common::SystemParameter::getValue(
				 ModCharString(moduleName).append('_').
				 append(Configuration::FilePermission::Name), v) &&
			 v >= Os::File::Permission::None &&
			 v <= Os::File::Permission::Mask) ?
			v : Configuration::FilePermission::Default;

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_PageTableSize::prepare --
//		あるバッファプールのすべてのバッファページ記述子を管理する
//		ハッシュ表のサイズの設定の取得を準備する
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
_Configuration::_PageTableSize::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値が 0 以下であれば、デフォルトとする

		; _SYDNEY_ASSERT(Configuration::PageTableSize::Default > 0);

		// デフォルト値
		
		unsigned int def = Configuration::PageTableSize::Default;
		
		int	v;
		_value =
			(Common::SystemParameter::getValue(
				 ModCharString(moduleName).append('_').
				 append(Configuration::PageTableSize::Name), v) &&
			 v > 0) ?
			v : def;

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_PageSizeMax::prepare --
//		処理可能なバッファページのサイズの設定の取得を準備する
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
_Configuration::_PageSizeMax::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値が 0 より小さければ、デフォルトとする

		; _SYDNEY_ASSERT(Configuration::PageSizeMax::Default >= 0);

		int v;
		_value = (Common::SystemParameter::getValue(
					  ModCharString(moduleName).append('_').
					  append(Configuration::PageSizeMax::Name), v) &&
				  v >= 0) ?
			v : Configuration::PageSizeMax::Default;

		// 指定された値以上の
		// システムのメモリページの境界の最小の倍数を求める

		const Os::Memory::Size unit = Os::SysConf::PageSize::get();
		_value = (_value + unit - 1) & ~(unit - 1);

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_DirtyPageFlusherPeriod::prepare --
//		ダーティバッファページ書き込みスレッドの
//		停止時間の設定の取得を準備する
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
_Configuration::_DirtyPageFlusherPeriod::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値が 0 より小さければ、デフォルトとする

		; _SYDNEY_ASSERT(Configuration::DirtyPageFlusherPeriod::Default >= 0);

		int	v;
		_value =
			(Common::SystemParameter::getValue(
				 ModCharString(moduleName).append('_').
				 append(Configuration::DirtyPageFlusherPeriod::Name), v) &&
			 v >= 0) ?
			v : Configuration::DirtyPageFlusherPeriod::Default;

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_FlushPageCoefficient::prepare --
//		ダーティページがバッファプールサイズの
//		なんパーセントになったらフラッシュするかの設定の取得を準備する
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
_Configuration::_FlushPageCoefficient::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値が 0 より小さければ 0 とし、
		// 100 より大きければ 100 とする

		; _SYDNEY_ASSERT(Configuration::FlushPageCoefficient::Default >= 0 &&
						 Configuration::FlushPageCoefficient::Default <= 100);

		int	v;
		_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').
				append(Configuration::FlushPageCoefficient::Name), v)) ?
			ModMin(ModMax(0, v), 100) :
			Configuration::FlushPageCoefficient::Default;

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_FreePageCountMax::prepare --
//		使用済のバッファページ記述子を再利用の為に
//		最大いくつまで保持するかの設定の取得を準備する
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
_Configuration::_FreePageCountMax::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値が 0 より小さければ、デフォルトする

		; _SYDNEY_ASSERT(Configuration::FreePageCountMax::Default >= 0);

		int	v;
		_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').
				append(Configuration::FreePageCountMax::Name), v) &&
			 v >= 0) ?
			v : Configuration::FreePageCountMax::Default;

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_KeepingUsedMemoryTimeMax::prepare --
//		使用済領域の最大保持期間の設定の取得を準備する
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
_Configuration::_KeepingUsedMemoryTimeMax::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値が 0 より小さければ、デフォルトとする

		; _SYDNEY_ASSERT(Configuration::KeepingUsedMemoryTimeMax::Default >= 0);

		int	v;
		_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').
				append(Configuration::KeepingUsedMemoryTimeMax::Name), v) &&
			 v >= 0) ?
			v : Configuration::KeepingUsedMemoryTimeMax::Default;

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_NormalPoolSize::prepare --
//		通常のバッファプールのサイズの設定の取得を準備する
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
_Configuration::_NormalPoolSize::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値が Minimum より小さければ、Minimum とする

		; _SYDNEY_ASSERT(Configuration::NormalPoolSize::Default >= 0);

		ModInt64 vv = Configuration::NormalPoolSize::Default;
		ModUnicodeString v;
		if (Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').
				append(Configuration::NormalPoolSize::Name), v))
			vv = _Configuration::toInt(v);
		
		_value = static_cast<Os::Memory::LongSize>(
			(vv >= Configuration::NormalPoolSize::Minimum) ?
			vv : Configuration::NormalPoolSize::Minimum);

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_TemporaryPoolSize::prepare --
//		一時データを格納するバッファプールのサイズの設定の取得を準備する
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
_Configuration::_TemporaryPoolSize::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値が Minimum より小さければ、Minimum とする

		; _SYDNEY_ASSERT(Configuration::TemporaryPoolSize::Default >= 0);	

		ModInt64 vv = Configuration::TemporaryPoolSize::Default;
		ModUnicodeString v;
		if (Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').
				append(Configuration::TemporaryPoolSize::Name), v))
			vv = _Configuration::toInt(v);
		
		_value = static_cast<Os::Memory::LongSize>(
			(vv >= Configuration::TemporaryPoolSize::Minimum) ?
			vv : Configuration::TemporaryPoolSize::Minimum);

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_ReadOnlyPoolSize::prepare --
//		読取専用データを格納するバッファプールのサイズの設定の取得を準備する
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
_Configuration::_ReadOnlyPoolSize::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値が Minimum より小さければ、Minimum とする

		; _SYDNEY_ASSERT(Configuration::ReadOnlyPoolSize::Default >= 0);	

		ModInt64 vv = Configuration::ReadOnlyPoolSize::Default;
		ModUnicodeString v;
		if (Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').
				append(Configuration::ReadOnlyPoolSize::Name), v))
			vv = _Configuration::toInt(v);
		
		_value = static_cast<Os::Memory::LongSize>(
			(vv >= Configuration::ReadOnlyPoolSize::Minimum) ?
			vv : Configuration::ReadOnlyPoolSize::Minimum);
		
#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_LogicalLogPoolSize::prepare --
//		論理ログデータを格納するバッファプールのサイズの設定の取得を準備する
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
_Configuration::_LogicalLogPoolSize::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値が Minimum より小さければ、Minimum とする

		; _SYDNEY_ASSERT(Configuration::LogicalLogPoolSize::Default >= 0);	

		ModInt64 vv = Configuration::LogicalLogPoolSize::Default;
		ModUnicodeString v;
		if (Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').
				append(Configuration::LogicalLogPoolSize::Name), v))
			vv = _Configuration::toInt(v);
		
		_value = static_cast<Os::Memory::LongSize>(
			(vv >= Configuration::LogicalLogPoolSize::Minimum) ?
			vv : Configuration::LogicalLogPoolSize::Minimum);
		
#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_OpenFileCountMax::prepare --
//		同時にオープン可能なバッファファイル数の設定の取得を準備する
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
_Configuration::_OpenFileCountMax::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値が 0 以下であれば、デフォルトとする

		; _SYDNEY_ASSERT(Os::SysConf::OpenMax::get() > Reserved);

		int v;
		_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').
				append(Configuration::OpenFileCountMax::Name), v) &&
			 v > 0) ?
			v : (Os::SysConf::OpenMax::get() - Reserved);

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_CalculateCheckSum::prepare --
//		バッファページ全体の CRC を計算し、それを使って
//		バッファページの整合性検査を行うかの設定の取得を準備する
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
_Configuration::_CalculateCheckSum::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値が Value として
		// 適切な範囲でなければ、デフォルトとする

		; _SYDNEY_ASSERT(Configuration::CalculateCheckSum::Default >=
						 Configuration::CalculateCheckSum::None &&
						 Configuration::CalculateCheckSum::Default <
						 Configuration::CalculateCheckSum::Count);

		int		v;
		_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').
				append(Configuration::CalculateCheckSum::Name), v) &&
			 v >= Configuration::CalculateCheckSum::None &&
			 v < Configuration::CalculateCheckSum::Count) ?
			static_cast<Configuration::CalculateCheckSum::Value>(v) :
			Configuration::CalculateCheckSum::Default;

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_DelayTemporaryCreation::prepare --
//		一時データを格納するバッファファイルの生成を
//		遅延するかの設定の取得を準備する
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
_Configuration::_DelayTemporaryCreation::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求める

		bool	v;
		_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').append(
					Configuration::DelayTemporaryCreation::Name), v)) ?
			v : Configuration::DelayTemporaryCreation::Default;

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_RetryAllocationCountMax::prepare --
//		バッファメモリ確保の再試行の最大回数の設定の取得を準備する
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
_Configuration::_RetryAllocationCountMax::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値が 0 以下であれば、デフォルトとする

		; _SYDNEY_ASSERT(Configuration::RetryAllocationCountMax::Default > 0);

		int v;
		_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').
				append(Configuration::RetryAllocationCountMax::Name), v) &&
			 v > 0) ?
			v : Configuration::RetryAllocationCountMax::Default;

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_FlushingBodyCountMax::prepare --
//		一度にフラッシュする連続するバッファメモリの最大数の設定の取得を準備する
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
_Configuration::_FlushingBodyCountMax::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値が 0 以下であれば、デフォルトとする

		; _SYDNEY_ASSERT(Configuration::FlushingBodyCountMax::Default > 0);

		int v;
		_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').
				append(Configuration::FlushingBodyCountMax::Name), v) &&
			 v > 0) ?
			v : Configuration::FlushingBodyCountMax::Default;

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_SkipDirtyCandidateCountMax::prepare --
//		置換候補を探すときに最大いくつのダーティなバッファページを
//		スキップするかの設定の取得を準備する
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
_Configuration::_SkipDirtyCandidateCountMax::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値が 0 未満であれば、デフォルトとする

		; _SYDNEY_ASSERT(
			Configuration::SkipDirtyCandidateCountMax::Default >= 0);

		int v;
		_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').
				append(Configuration::SkipDirtyCandidateCountMax::Name), v) &&
			 v >= 0) ?
			v : Configuration::SkipDirtyCandidateCountMax::Default;

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_ReadAheadBlockSize::prepare --
//		ページを読み込む時に先読みするブロックのサイズの設定の取得を準備する
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
_Configuration::_ReadAheadBlockSize::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		const Os::Memory::Size unit = Os::SysConf::PageSize::get();
		
		// 値を求め、求めた値が 0 未満であれば、デフォルトとする

		; _SYDNEY_ASSERT(
			Configuration::ReadAheadBlockSize::Default >= unit);
		; _SYDNEY_ASSERT(
			Configuration::ReadAheadBlockSize::Maximum >=
			Configuration::ReadAheadBlockSize::Default);
		; _SYDNEY_ASSERT(
			(Configuration::ReadAheadBlockSize::Default % unit) == 0);
		; _SYDNEY_ASSERT(
			(Configuration::ReadAheadBlockSize::Maximum % unit) == 0);

		int v;
		_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').
				append(Configuration::ReadAheadBlockSize::Name), v) &&
			 v >= 0) ?
			v : Configuration::ReadAheadBlockSize::Default;
		
		// 指定された値以上の
		// システムのメモリページの境界の最小の倍数を求める

		_value = (_value + unit - 1) & ~(unit - 1);

		// 最大値を超えているかチェックする
		
		if (_value > Configuration::ReadAheadBlockSize::Maximum)
			_value = Configuration::ReadAheadBlockSize::Maximum;

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

//	FUNCTION
//	$$$::_Configuration::_StatisticsReporterPeriod::prepare --
//		統計情報出力スレッドの停止時間の設定の取得を準備する
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
_Configuration::_StatisticsReporterPeriod::prepare()
{
#ifndef GET_ALL_AT_ONCE
	if (_first) {
#endif
		// 値を求め、求めた値が 0 より小さければ、デフォルトとする

		; _SYDNEY_ASSERT(Configuration::StatisticsReporterPeriod::Default >= 0);

		int	v;
		_value =
			(Common::SystemParameter::getValue(
				ModCharString(moduleName).append('_').
				append(Configuration::StatisticsReporterPeriod::Name), v) &&
			 v >= 0) ?
			v : Configuration::StatisticsReporterPeriod::Default;

#ifndef GET_ALL_AT_ONCE

		// 次はパラメーターを取得せずに
		// 記憶している値をそのまま返すようにする

		_first = false;
	}
#endif
}

}

//	FUNCTION
//	Buffer::Configuration::get --
//		バッファ管理マネージャーの設定をすべてシステムパラメーターから読み出す
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
#ifdef GET_ALL_AT_ONCE
	(void) _Configuration::_NormalPoolSize::prepare();	// 必ず最初
	
	(void) _Configuration::_FileTableSize::prepare();
	(void) _Configuration::_FilePermission::prepare();
	(void) _Configuration::_PageTableSize::prepare();
	(void) _Configuration::_PageSizeMax::prepare();
	(void) _Configuration::_DirtyPageFlusherPeriod::prepare();
	(void) _Configuration::_FlushPageCoefficient::prepare();
	(void) _Configuration::_FreePageCountMax::prepare();
	(void) _Configuration::_KeepingUsedMemoryTimeMax::prepare();
	(void) _Configuration::_TemporaryPoolSize::prepare();
	(void) _Configuration::_ReadOnlyPoolSize::prepare();
	(void) _Configuration::_LogicalLogPoolSize::prepare();
	(void) _Configuration::_OpenFileCountMax::prepare();
	(void) _Configuration::_CalculateCheckSum::prepare();
	(void) _Configuration::_DelayTemporaryCreation::prepare();
	(void) _Configuration::_RetryAllocationCountMax::prepare();
	(void) _Configuration::_FlushingBodyCountMax::prepare();
	(void) _Configuration::_SkipDirtyCandidateCountMax::prepare();
	(void) _Configuration::_ReadAheadBlockSize::prepare();
	(void) _Configuration::_StatisticsReporterPeriod::prepare();
#endif
}

//	FUNCTION
//	Buffer::Configuration::reset --
//		バッファ管理マネージャーの設定をリセットする
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
	(void) FilePermission::reset();
	(void) PageTableSize::reset();
	(void) PageSizeMax::reset();
	(void) DirtyPageFlusherPeriod::reset();
	(void) FlushPageCoefficient::reset();
	(void) FreePageCountMax::reset();
	(void) KeepingUsedMemoryTimeMax::reset();
	(void) NormalPoolSize::reset();
	(void) TemporaryPoolSize::reset();
	(void) ReadOnlyPoolSize::reset();
	(void) LogicalLogPoolSize::reset();
	(void) OpenFileCountMax::reset();
	(void) CalculateCheckSum::reset();
	(void) DelayTemporaryCreation::reset();
	(void) RetryAllocationCountMax::reset();
	(void) FlushingBodyCountMax::reset();
	(void) SkipDirtyCandidateCountMax::reset();
	(void) ReadAheadBlockSize::reset();
	(void) StatisticsReporterPeriod::reset();
}

//	FUNCTION
//	Buffer::Configuration::FileTableSize::get --
//		すべてのバッファファイル記述子を管理する
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
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_FileTableSize::prepare();
#endif
	return _Configuration::_FileTableSize::_value;
}

//	FUNCTION
//	Buffer::Configuration::FileTableSize::reset --
//		すべてのバッファファイル記述子を管理する
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
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_FileTableSize::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::FilePermission::get --
//		バッファファイルの実体である OS ファイルの
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
Configuration::FilePermission::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_FilePermission::prepare();
#endif
	return _Configuration::_FilePermission::_value;
}

//	FUNCTION
//	Buffer::Configuration::FilePermission::reset --
//		バッファファイルの実体である OS ファイルの
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
Configuration::FilePermission::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_FilePermission::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::PageTableSize::get --
//		あるバッファプールのすべてのバッファページ記述子を管理する
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
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_NormalPoolSize::prepare();
	_Configuration::_PageTableSize::prepare();
#endif
	return _Configuration::_PageTableSize::_value;
}

//	FUNCTION
//	Buffer::Configuration::PageTableSize::reset --
//		あるバッファファイルのすべてのバッファページ記述子を管理する
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
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_PageTableSize::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::PageSizeMax::get --
//		処理可能なバッファページのサイズの設定を取得する
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

Os::Memory::Size
Configuration::PageSizeMax::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_PageSizeMax::prepare();
#endif
	return _Configuration::_PageSizeMax::_value;
}

//	FUNCTION
//	Buffer::Configuration::PageSizeMax::reset --
//		処理可能なバッファページのサイズの設定をリセットする
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
Configuration::PageSizeMax::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_PageSizeMax::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::DirtyPageFlusherPeriod::get --
//		ダーティバッファページ書き込みスレッドの
//		停止時間の設定を取得する
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
Configuration::DirtyPageFlusherPeriod::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_DirtyPageFlusherPeriod::prepare()
#endif
	return _Configuration::_DirtyPageFlusherPeriod::_value;
}

//	FUNCTION
//	Buffer::Configuration::DirtyPageFlusherPeriod::reset --
//		ダーティバッファページ書き込みスレッドの
//		停止時間の設定をリセットする
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
Configuration::DirtyPageFlusherPeriod::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_DirtyPageFlusherPeriod::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::FlushPageCoefficient::get --
//		ダーティページがバッファプールサイズの
//		なんパーセントになったらフラッシュするかの設定を取得する
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
Configuration::FlushPageCoefficient::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_FlushPageCoefficient::prepare();
#endif
	return _Configuration::_FlushPageCoefficient::_value;
}

//	FUNCTION
//	Buffer::Configuration::FlushPageCoefficient::reset --
//		ダーティページがバッファプールサイズの
//		なんパーセントになったらフラッシュするかの設定をリセットする
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
Configuration::FlushPageCoefficient::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_FlushPageCoefficient::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::FreePageCountMax::get --
//		使用済のバッファページ記述子を再利用の為に
//		最大いくつまで保持するかの設定を取得する
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
Configuration::FreePageCountMax::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_FreePageCountMax::prepare();
#endif
	return _Configuration::_FreePageCountMax::_value;
}

//	FUNCTION
//	Buffer::Configuration::FreePageCountMax::reset --
//		使用済のバッファページ記述子を再利用の為に
//		最大いくつまで保持するかの設定をリセットする
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
Configuration::FreePageCountMax::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_FreePageCountMax::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::KeepingUsedMemoryTimeMax::get --
//		使用済領域の最大保持期間の設定を取得する
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
Configuration::KeepingUsedMemoryTimeMax::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_KeepingUsedMemoryTimeMax::prepare();
#endif
	return _Configuration::_KeepingUsedMemoryTimeMax::_value;
}

//	FUNCTION
//	Buffer::Configuration::KeepingUsedMemoryTimeMax::reset --
//		使用済領域の最大保持期間の設定をリセットする
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
Configuration::KeepingUsedMemoryTimeMax::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_KeepingUsedMemoryTimeMax::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::NormalPoolSize::get --
//		通常のバッファプールのサイズの設定を取得する
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

Os::Memory::LongSize
Configuration::NormalPoolSize::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_NormalPoolSize::prepare();
#endif
	return _Configuration::_NormalPoolSize::_value;
}

//	FUNCTION
//	Buffer::Configuration::NormalPoolSize::reset --
//		通常のバッファプールのサイズの設定をリセットする
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
Configuration::NormalPoolSize::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_NormalPoolSize::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::TemporaryPoolSize::get --
//		一時データを格納するバッファプールのサイズの設定を取得する
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

Os::Memory::LongSize
Configuration::TemporaryPoolSize::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_TemporaryPoolSize::prepare();
#endif
	return _Configuration::_TemporaryPoolSize::_value;
}

//	FUNCTION
//	Buffer::Configuration::TemporaryPoolSize::reset --
//		一時データを格納するバッファプールのサイズの設定をリセットする
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
Configuration::TemporaryPoolSize::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_TemporaryPoolSize::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::ReadOnlyPoolSize::get --
//		読取専用データを格納するバッファプールのサイズの設定を取得する
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

Os::Memory::LongSize
Configuration::ReadOnlyPoolSize::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_ReadOnlyPoolSize::prepare();
#endif
	return _Configuration::_ReadOnlyPoolSize::_value;
}

//	FUNCTION
//	Buffer::Configuration::ReadOnlyPoolSize::reset --
//		読取専用データを格納するバッファプールのサイズの設定をリセットする
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
Configuration::ReadOnlyPoolSize::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_ReadOnlyPoolSize::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::LogicalLogPoolSize::get --
//		論理ログデータを格納するバッファプールのサイズの設定を取得する
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

Os::Memory::LongSize
Configuration::LogicalLogPoolSize::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_LogicalLogPoolSize::prepare();
#endif
	return _Configuration::_LogicalLogPoolSize::_value;
}

//	FUNCTION
//	Buffer::Configuration::LogicalLogPoolSize::reset --
//		論理ログデータを格納するバッファプールのサイズの設定をリセットする
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
Configuration::LogicalLogPoolSize::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_LogicalLogPoolSize::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::OpenFileCountMax::get --
//		同時にオープン可能なバッファファイル数の設定を取得する
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
Configuration::OpenFileCountMax::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_OpenFileCountMax::prepare();
#endif
	return _Configuration::_OpenFileCountMax::_value;
}

//	FUNCTION
//	Buffer::Configuration::OpenFileCountMax::reset --
//		同時にオープン可能なバッファファイル数の設定をリセットする
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
Configuration::OpenFileCountMax::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_OpenFileCountMax::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::CalculateCheckSum::get --
//		バッファページ全体の CRC を計算し、それを使って
//		バッファページの整合性検査を行うかの設定を取得する
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

Configuration::CalculateCheckSum::Value
Configuration::CalculateCheckSum::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_CalculateCheckSum::prepare();
#endif
	return _Configuration::_CalculateCheckSum::_value;
}

//	FUNCTION
//	Buffer::Configuration::CalculateCheckSum::reset --
//		バッファページ全体の CRC を計算し、それを使って
//		バッファページの整合性検査を行うかの設定をリセットする
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
Configuration::CalculateCheckSum::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_CalculateCheckSum::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::DelayTemporaryCreation::get --
//		一時データを格納するバッファファイルの生成を
//		遅延するかの設定を取得する
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
Configuration::DelayTemporaryCreation::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_DelayTemporaryCreation::prepare();
#endif
	return _Configuration::_DelayTemporaryCreation::_value;
}

//	FUNCTION
//	Buffer::Configuration::DelayTemporaryCreation::reset --
//		一時データを格納するバッファファイルの生成を
//		遅延するかの設定をリセットする
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
Configuration::DelayTemporaryCreation::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_DelayTemporaryCreation::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::RetryAllocationCountMax::get --
//		バッファメモリ確保の再試行の最大回数の設定を取得する
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
Configuration::RetryAllocationCountMax::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_RetryAllocationCountMax::prepare();
#endif
	return _Configuration::_RetryAllocationCountMax::_value;
}

//	FUNCTION
//	Buffer::Configuration::RetryAllocationCountMax::reset --
//		バッファメモリ確保の再試行の最大回数の設定をリセットする
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
Configuration::RetryAllocationCountMax::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_RetryAllocationCountMax::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::FlushingBodyCountMax::get --
//		一度にフラッシュする連続するバッファメモリの最大数の設定を取得する
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
Configuration::FlushingBodyCountMax::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_FlushingBodyCountMax::prepare();
#endif
	return _Configuration::_FlushingBodyCountMax::_value;
}

//	FUNCTION
//	Buffer::Configuration::FlushingBodyCountMax::reset --
//		一度にフラッシュする連続するバッファメモリの最大数の設定をリセットする
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
Configuration::FlushingBodyCountMax::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_FlushingBodyCountMax::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::SkipDirtyCandidateCountMax::get --
//		置換候補を探すときに最大いくつのダーティなバッファページを
//		スキップするかの設定を取得する
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
Configuration::SkipDirtyCandidateCountMax::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_SkipDirtyCandidateCountMax::prepare();
#endif
	return _Configuration::_SkipDirtyCandidateCountMax::_value;
}

//	FUNCTION
//	Buffer::Configuration::SkipDirtyCandidateCountMax::reset --
//		置換候補を探すときに最大いくつのダーティなバッファページを
//		スキップするかの設定をリセットする
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
Configuration::SkipDirtyCandidateCountMax::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_SkipDirtyCandidateCountMax::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::ReadAheadBlockSize::get --
//		ページを読み込む時に先読みするブロックのサイズの設定を取得する
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
Configuration::ReadAheadBlockSize::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_ReadAheadBlockSize::prepare();
#endif
	return _Configuration::_ReadAheadBlockSize::_value;
}

//	FUNCTION
//	Buffer::Configuration::ReadAheadBlockSize::reset --
//		ページを読み込む時に先読みするブロックのサイズの設定をリセットする
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
Configuration::ReadAheadBlockSize::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_ReadAheadBlockSize::_first = true;
#endif
}

//	FUNCTION
//	Buffer::Configuration::StatisticsReporterPeriod::get --
//		統計情報出力スレッドの停止時間の設定を取得する
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
Configuration::StatisticsReporterPeriod::get()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値取得の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_StatisticsReporterPeriod::prepare();
#endif
	return _Configuration::_StatisticsReporterPeriod::_value;
}

//	FUNCTION
//	Buffer::Configuration::StatisticsReporterPeriod::reset --
//		統計情報出力スレッドの停止時間の設定をリセットする
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
Configuration::StatisticsReporterPeriod::reset()
{
#ifdef CONCURRENT_EXECUTION
	// 設定値初期化の排他制御のためにラッチする

	Os::AutoCriticalSection	latch(_Configuration::_latch);
#endif
#ifndef GET_ALL_AT_ONCE
	_Configuration::_StatisticsReporterPeriod::_first = true;
#endif
}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
