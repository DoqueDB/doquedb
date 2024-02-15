// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Configuration.h -- 版管理マネージャーの設定関連の関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VERSION_CONFIGURATION_H
#define	__SYDNEY_VERSION_CONFIGURATION_H

#include "Version/Module.h"

#include "Os/File.h"

_SYDNEY_BEGIN
_SYDNEY_VERSION_BEGIN

//	NAMESPACE
//	Version::Configuration -- 版管理マネージャーの設定に関する名前空間
//
//	NOTES

namespace Configuration
{
#ifdef OBSOLETE
	// 設定をすべてシステムパラメーターから読み出す
	void					get();
#endif
	// 設定のリセットを行う
	void					reset();

	//	NAMESPACE
	//	Version::Configuration::FileTableSize --
	//		すべてのバージョンファイル記述子を管理する
	//		ハッシュ表のサイズの設定に関する名前空間
	//
	//	NOTES

	namespace FileTableSize
	{
		//	CONST
		//	Version::Configuration::FileTableSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "FileTableSize";

		//	CONST
		//	Version::Configuration::FileTableSize::Default --
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
	//	Version::Configuration::PageTableSize --
	//		あるバージョンファイルのすべてのバッファページ記述子を管理する
	//		ハッシュ表のサイズの設定に関する名前空間
	//
	//	NOTES

	namespace PageTableSize
	{
		//	CONST
		//	Version::Configuration::PageTableSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "PageTableSize";

		//	CONST
		//	Version::Configuration::PageTableSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES
#ifdef SYD_ARCH64
		const unsigned int	Default = 30089;
#else
		const unsigned int	Default = 3079;
#endif

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Version::Configuration::VerificationTableSize --
	//		すべての整合性検査に関する情報を表すクラスを管理する
	//		ハッシュ表のサイズの設定に関する名前空間
	//
	//	NOTES

	namespace VerificationTableSize
	{
		//	CONST
		//	Version::Configuration::VerificationTableSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "VerificationTableSize";

		//	CONST
		//	Version::Configuration::VerificationTableSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES
#ifdef SYD_ARCH64
		const unsigned int	Default = 97;
#else
		const unsigned int	Default = 7;
#endif

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Version::Configuration::MultiplexInfoTableSize --
	//		あるバージョンログファイルの多重化されたブロックのうち、
	//		どれを選択するか決めるための情報を表すクラスをすべて管理する
	//		ハッシュ表のサイズの設定に関する名前空間
	//
	//	NOTES

	namespace MultiplexInfoTableSize
	{
		//	CONST
		//	Version::Configuration::MultiplexInfoTableSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "MultiplexInfoTableSize";

		//	CONST
		//	Version::Configuration::MultiplexInfoTableSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES
#ifdef SYD_ARCH64
		const unsigned int	Default = 97;
#else
		const unsigned int	Default = 7;
#endif

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Version::Configuration::DetachedPageCleanerPeriod --
	//		参照済バージョンページ記述子破棄スレッドの停止時間の
	//		設定に関する名前空間
	//
	//	NOTES

	namespace DetachedPageCleanerPeriod
	{
		//	CONST
		//	Version::Configuration::DetachedPageCleanerPeriod::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "DetachedPageCleanerPeriod";

		//	CONST
		//	Version::Configuration::DetachedPageCleanerPeriod::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const unsigned int	Default = 60 * 1000;

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Version::Configuration::CleanPageCoefficient --
	//		あるバージョンファイルの参照済バージョンページ記述子のうち、
	//		最大で何パーセント破棄するかの設定に関する名前空間
	//
	//	NOTES

	namespace CleanPageCoefficient
	{
		//	CONST
		//	Version::Configuration::CleanPageCoefficient::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "CleanPageCoefficient";

		//	CONST
		//	Version::Configuration::CleanPageCoefficient::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const unsigned int	Default = 50;

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Version::Configuration::NoVersion --
	//		バージョンをひとつしか生成できない
	//		バージョンファイルとみなすかの設定に関する名前空間
	//
	//	NOTES

	namespace NoVersion
	{
		//	CONST
		//	Version::Configuration::NoVersion::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "NoVersion";

		//	CONST
		//	Version::Configuration::NoVersion::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const bool			Default = false;

		// パラメーター値を取得する
		bool				get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Version::Configuration::SyncPageCountMax --
	//		一度に同期するバージョンページの最大数の設定に関する名前空間
	//
	//	NOTES

	namespace SyncPageCountMax
	{
		//	CONST
		//	Version::Configuration::SyncPageCountMax::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "SyncPageCountMax";

		//	CONST
		//	Version::Configuration::SyncPageCountMax::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const unsigned int	Default = 1000;

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Version::Configuration::MasterDataExtensionSize --
	//		マスタデータファイルを拡張する単位の設定に関する名前空間
	//
	//	NOTES

	namespace MasterDataExtensionSize
	{
		//	CONST
		//	Version::Configuration::MasterDataExtensionSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "MasterDataExtensionSize";

		//	CONST
		//	Version::Configuration::MasterDataExtensionSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const Os::File::Size	Default = 64 << 10;

		// パラメーター値を取得する
		Os::File::Size		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Version::Configuration::VersionLogExtensionSize --
	//		バージョンログファイルを拡張する単位の設定に関する名前空間
	//
	//	NOTES

	namespace VersionLogExtensionSize
	{
		//	CONST
		//	Version::Configuration::VersionLogExtensionSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "VersionLogExtensionSize";

		//	CONST
		//	Version::Configuration::VersionLogExtensionSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const Os::File::Size	Default = 64 << 10;

		// パラメーター値を取得する
		Os::File::Size		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Version::Configuration::SyncLogExtensionSize --
	//		同期ログファイルを拡張する単位の設定に関する名前空間
	//
	//	NOTES

	namespace SyncLogExtensionSize
	{
		//	CONST
		//	Version::Configuration::SyncLogExtensionSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "SyncLogExtensionSize";

		//	CONST
		//	Version::Configuration::SyncLogExtensionSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const Os::File::Size	Default = 64 << 10;

		// パラメーター値を取得する
		Os::File::Size		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Version::Configuration::MaxExtensionSize --
	//		バージョンファイルを拡張する単位の最大値の設定に関する名前空間
	//
	//	NOTES
	
	namespace MaxExtensionSize
	{
		//	CONST
		//	Version::Configuration::MaxExtensionSize::Name --
		//		パラメータ名を表す定数
		//
		//	NOTES

		const char* const	Name = "MaxExtensionSize";

		//	CONST
		//	Version::Configuration::MaxExtensionSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const Os::File::Size	Default = 64 << 10 << 10;

		// パラメーター値を取得する
		Os::File::Size		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Version::Configuration::PageInstanceCacheSize --
	//		ページのインスタンスをキャッシュする数
	//
	//	NOTES

	namespace PageInstanceCacheSize
	{
		//	CONST
		//	Version::Configuration::PageInstanceCacheSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "PageInstanceCacheSize";

		//	CONST
		//	Version::Configuration::PageInstanceCacheSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const int			Default = 100;

		// パラメータ値を取得する
		int					get();
		// 記憶しているパラメータ値を忘れる
		void				reset();
	}
}

_SYDNEY_VERSION_END
_SYDNEY_END

#endif	// __SYDNEY_VERSION_CONFIGURATION_H

//
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
