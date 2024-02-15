// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Configuration.h -- バッファ管理マネージャーの設定関連の関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2008, 2009, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BUFFER_CONFIGURATION_H
#define	__SYDNEY_BUFFER_CONFIGURATION_H

#include "Buffer/Module.h"

#include "Os/Memory.h"

_SYDNEY_BEGIN
_SYDNEY_BUFFER_BEGIN

//	NAMESPACE
//	Buffer::Configuration -- バッファ管理マネージャーの設定に関する名前空間
//
//	NOTES

namespace Configuration
{
	// 設定をすべてシステムパラメーターから読み出す
	void					get();
	// 設定のリセットを行う
	void					reset();

	//	NAMESPACE
	//	Buffer::Configuration::FileTableSize --
	//		すべてのバッファファイル記述子を管理する
	//		ハッシュ表のサイズの設定に関する名前空間
	//
	//	NOTES

	namespace FileTableSize
	{
		//	CONST
		//	Buffer::Configuration::FileTableSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "FileTableSize";

		//	CONST
		//	Buffer::Configuration::FileTableSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES
#ifdef SYD_ARCH64
		const unsigned int	Default = 1031;
#else
		const unsigned int	Default = 97;
#endif

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::FilePermission --
	//		バッファファイルの実体である OS ファイルの
	//		許可モードの設定に関する名前空間
	//
	//	NOTES

	namespace FilePermission
	{
		//	CONST
		//	Buffer::Configuration::FilePermission::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "FilePermission";

		//	CONST
		//	Buffer::Configuration::FilePermission::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

#ifdef DEBUG
		const unsigned int	Default = 0660;
#else
		const unsigned int	Default = 0600;
#endif

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::PageTableSize --
	//		あるバッファプールのすべてのバッファページ記述子を管理する
	//		ハッシュ表のサイズの設定に関する名前空間
	//
	//	NOTES

	namespace PageTableSize
	{
		//	CONST
		//	Buffer::Configuration::PageTableSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "PageTableSize";

		//	CONST
		//	Buffer::Configuration::PageTableSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES
		//		Common::HashTable は与えられた長さ以上の
		//		最小の素数を実際のハッシュ表のサイズとする
		//		Buffer_NormalPoolSize が 256M 以上の場合は、
		//		ハッシュ表のサイズも大きくなる

		const unsigned int	Default = 0x2000;	// 8192

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::PageSizeMax --
	//		処理可能なバッファページのサイズの設定に関する名前空間
	//
	//	NOTES

	namespace PageSizeMax
	{
		//	CONST
		//	Buffer::Configuration::PageSizeMax::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "PageSizeMax";

		//	CONST
		//	Buffer::Configuration::PageSizeMax::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const Os::Memory::Size	Default = 64 << 10;

		// パラメーター値を取得する
		Os::Memory::Size	get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::DirtyPageFlusherPeriod --
	//		ダーティバッファページ書き込みスレッドの
	//		停止時間の設定に関する名前空間
	//
	//	NOTES

	namespace DirtyPageFlusherPeriod
	{
		//	CONST
		//	Buffer::Configuration::DirtyPageFlusherPeriod::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "DirtyPageFlusherPeriod";

		//	CONST
		//	Buffer::Configuration::DirtyPageFlusherPeriod::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const unsigned int	Default = 5 * 1000;

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::FlushPageCoefficient --
	//		ダーティページがバッファプールサイズの
	//		なんパーセントになったらフラッシュするかの設定に関する名前空間
	//
	//	NOTES

	namespace FlushPageCoefficient
	{
		//	CONST
		//	Buffer::Configuration::FlushPageCoefficient::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "FlushPageCoefficient";

		//	CONST
		//	Buffer::Configuration::FlushPageCoefficient::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const unsigned int	Default = 95;

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::FreePageCountMax --
	//		使用済のバッファページ記述子を再利用の為に
	//		最大いくつまで保持するかの設定に関する名前空間
	//
	//	NOTES

	namespace FreePageCountMax
	{
		//	CONST
		//	Buffer::Configuration::FreePageCountMax::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "FreePageCountMax";

		//	CONST
		//	Buffer::Configuration::FreePageCountMax::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const unsigned int	Default = 100;

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::KeepingUsedMemoryTimeMax --
	//		使用済領域の最大保持時間の設定に関する名前空間
	//
	//	NOTES

	namespace KeepingUsedMemoryTimeMax
	{
		//	CONST
		//	Buffer::Configuration::KeepingUsedMemoryTimeMax::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "KeepingUsedMemoryTimeMax";

		//	CONST
		//	Buffer::Configuration::KeepingUsedMemoryTimeMax::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const unsigned int	Default = 10 * 60 * 60 * 1000;

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::NormalPoolSize --
	//		通常のバッファプールのサイズの設定に関する名前空間
	//
	//	NOTES

	namespace NormalPoolSize
	{
		//	CONST
		//	Buffer::Configuration::NormalPoolSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "NormalPoolSize";

		//	CONST
		//	Buffer::Configuration::NormalPoolSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES
#ifdef SYD_ARCH64
		const Os::Memory::LongSize	Default = 1024 << 10 << 10;
#else
		const Os::Memory::LongSize	Default = 20 << 10 << 10;
#endif

		//	CONST
		//	Buffer::Configuration::NormalPoolSize::Minimum --
		//		最小値を表す定数
		//
		//	NOTES
#ifdef SYD_ARCH64
		const Os::Memory::LongSize	Minimum = 256 << 10 << 10;
#else
		const Os::Memory::LongSize	Minimum = 20 << 10 << 10;
#endif

		// パラメーター値を取得する
		Os::Memory::LongSize	get();
		// 記録しているパラメーター値を忘れる
		void					reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::TemporaryPoolSize --
	//		一時データを格納するバッファプールのサイズの設定に関する名前空間
	//
	//	NOTES

	namespace TemporaryPoolSize
	{
		//	CONST
		//	Buffer::Configuration::TemporaryPoolSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "TemporaryPoolSize";

		//	CONST
		//	Buffer::Configuration::TemporaryPoolSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES
#ifdef SYD_ARCH64
		const Os::Memory::LongSize	Default = 64 << 10 << 10;
#else
		const Os::Memory::LongSize	Default = 5 << 10 << 10;
#endif

		//	CONST
		//	Buffer::Configuration::TemporaryPoolSize::Minimum --
		//		最小値を表す定数
		//
		//	NOTES
#ifdef SYD_ARCH64
		const Os::Memory::LongSize	Minimum = 64 << 10 << 10;
#else
		const Os::Memory::LongSize	Minimum = 5 << 10 << 10;
#endif

		// パラメーター値を取得する
		SYD_BUFFER_FUNCTION
		Os::Memory::LongSize	get();
		// 記録しているパラメーター値を忘れる
		void					reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::ReadOnlyPoolSize --
	//		読取専用データを格納するバッファプールの
	//		サイズの設定に関する名前空間
	//
	//	NOTES

	namespace ReadOnlyPoolSize
	{
		//	CONST
		//	Buffer::Configuration::ReadOnlyPoolSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "ReadOnlyPoolSize";

		//	CONST
		//	Buffer::Configuration::ReadOnlyPoolSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES
#ifdef SYD_ARCH64
		const Os::Memory::LongSize	Default = 64 << 10 << 10;
#else
		const Os::Memory::LongSize	Default = 5 << 10 << 10;
#endif

		//	CONST
		//	Buffer::Configuration::ReadOnlyPoolSize::Minimum --
		//		最小値を表す定数
		//
		//	NOTES
#ifdef SYD_ARCH64
		const Os::Memory::LongSize	Minimum = 64 << 10 << 10;
#else
		const Os::Memory::LongSize	Minimum = 5 << 10 << 10;
#endif

		// パラメーター値を取得する
		Os::Memory::LongSize	get();
		// 記録しているパラメーター値を忘れる
		void					reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::LogicalLogPoolSize --
	//		論理ログデータを格納するバッファプールの
	//		サイズの設定に関する名前空間
	//
	//	NOTES

	namespace LogicalLogPoolSize
	{
		//	CONST
		//	Buffer::Configuration::LogicalLogPoolSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "LogicalLogPoolSize";

		//	CONST
		//	Buffer::Configuration::LogicalLogPoolSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const Os::Memory::LongSize	Default = 5 << 10 << 10;

		//	CONST
		//	Buffer::Configuration::LogicalLogPoolSize::Minimum --
		//		最小値を表す定数
		//
		//	NOTES

		const Os::Memory::LongSize	Minimum = 5 << 10 << 10;

		// パラメーター値を取得する
		Os::Memory::LongSize	get();
		// 記録しているパラメーター値を忘れる
		void					reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::OpenFileCountMax --
	//		同時にオープン可能なバッファファイル数の設定に関する名前空間
	//
	//	NOTES

	namespace OpenFileCountMax
	{
		//	CONST
		//	Buffer::Configuration::OpenFileCountMax::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "OpenFileCountMax";

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::CalculateCheckSum --
	//		バッファページ全体の CRC を計算し、それを使って
	//		バッファページの整合性検査を行うかの設定に関する名前空間
	//
	//	NOTES

	namespace CalculateCheckSum
	{
		//	CONST
		//	Buffer::Configuration::CalculateCheckSum::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "CalculateCheckSum";

		//	ENUM
		//	Buffer::Configuration::CalculateCheckSum::Value --
		//		パラメーター値を表す列挙型
		//
		//	NOTES

		typedef unsigned char	Value;
		enum
		{
			// どのバッファページに対しても行わない
			None =			0,
			// 特定のバッファページに対してのみ行う
			Specified,
			// すべてのバッファページに対して行う
			All,
			// 値の数
			Count,
			// 不明
			Unknown =		Count
		};

		//	CONST
		//	Buffer::Configuration::CalculateCheckSum::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const Value			Default = Specified;

		// パラメーター値を取得する
		Value				get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::DelayTemporaryCreation --
	//		一時データを格納するバッファファイルの生成を
	//		遅延するかの設定に関する名前空間
	//
	//	NOTES

	namespace DelayTemporaryCreation
	{
		//	CONST
		//	Buffer::Configuration::DelayTemporaryCreation::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char*	const	Name = "DelayTemporaryCreation";

		//	CONST
		//	Buffer::Configuration::DelayTemporaryCreation::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const bool			Default = true;

		// パラメーター値を取得する
		bool				get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::RetryAllocationCountMax --
	//		バッファメモリ確保の再試行の最大回数の設定に関する名前空間
	//
	//	NOTES

	namespace RetryAllocationCountMax
	{
		//	CONST
		//	Buffer::Configuration::RetryAllocationCountMax::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "RetryAllocationCountMax";

		//	CONST
		//	Buffer::Configuration::RetryAllocationCountMax::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const unsigned int	Default = 3;

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::FlushingBodyCountMax --
	//		一度にフラッシュする連続する
	//		バッファメモリの最大数の設定に関する名前空間
	//
	//	NOTES

	namespace FlushingBodyCountMax
	{
		//	CONST
		//	Buffer::Configuration::FlushingBodyCountMax::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "FlushingBodyCountMax";

		//	CONST
		//	Buffer::Configuration::FlushingBodyCountMax::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const unsigned int	Default = 8000;

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::SkipDirtyCandidateCountMax --
	//		置換候補を探すときに最大いくつのダーティなバッファページを
	//		スキップするかの設定に関する名前空間
	//
	//	NOTES

	namespace SkipDirtyCandidateCountMax
	{
		//	CONST
		//	Buffer::Configuration::SkipDirtyCandidateCountMax::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "SkipDirtyCandidateCountMax";

		//	CONST
		//	Buffer::Configuration::SkipDirtyCandidateCountMax::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const unsigned int	Default = 500;

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::ReadAheadBlockSize --
	//		ページを読み込む時に先読みするブロックのサイズの設定に関する名前空間
	//
	//	NOTES

	namespace ReadAheadBlockSize
	{
		//	CONST
		//	Buffer::Configuration::ReadAheadBlockSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "ReadAheadBlockSize";

		//	CONST
		//	Buffer::Configuration::ReadAheadBlockSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const unsigned int	Default = 64 << 10;

		//	CONST
		//	Buffer::Configuration::ReadAheadBlockSize::Maximum --
		//		最大値を表す定数
		//
		//	NOTES

		const unsigned int	Maximum = 512 << 10;

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Buffer::Configuration::StatisticsReporterPeriod --
	//		統計情報出力スレッドの停止時間の設定に関する名前空間
	//
	//	NOTES

	namespace StatisticsReporterPeriod
	{
		//	CONST
		//	Buffer::Configuration::StatisticsReporterPeriod::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "StatisticsReporterPeriod";

		//	CONST
		//	Buffer::Configuration::StatisticsReporterPeriod::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const unsigned int	Default = 0;

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}
}

_SYDNEY_BUFFER_END
_SYDNEY_END

#endif	// __SYDNEY_BUFFER_CONFIGURATION_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2008, 2009, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
