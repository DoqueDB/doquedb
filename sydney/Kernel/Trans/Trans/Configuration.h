// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Configuration.h -- トランザクションマネージャーの設定関連の関数宣言
// 
// Copyright (c) 2001, 2002, 2004, 2007, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_TRANS_CONFIGURATION_H
#define	__SYDNEY_TRANS_CONFIGURATION_H

#include "Trans/Module.h"
#include "Trans/Transaction.h"

_SYDNEY_BEGIN

namespace Os
{
	class Path;
}

_SYDNEY_TRANS_BEGIN

//	NAMESPACE
//	Trans::Configuration -- トランザクションマネージャーの設定に関する名前空間
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
	//	Trans::Configuration::TransTableSize --
	//		すべてのトランザクションを管理する
	//		ハッシュ表のサイズの設定に関する名前空間
	//
	//	NOTES

	namespace TransTableSize
	{
		//	CONST
		//	Trans::Configuration::TransTableSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "TransTableSize";

		//	CONST
		//	Trans::Configuration::TransTableSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES
		
#ifdef SYD_ARCH64
		const unsigned int	Default = 1031;
#else
		const unsigned int	Default = 137;
#endif

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Trans::Configuration::BranchTableSize --
	//		すべてのトランザクションブランチを管理する
	//		ハッシュ表のサイズの設定に関する名前空間
	//
	//	NOTES

	namespace BranchTableSize
	{
		//	CONST
		//	Trans::Configuration::BranchTableSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "BranchTableSize";

		//	CONST
		//	Trans::Configuration::BranchTableSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES
		
#ifdef SYD_ARCH64
		const unsigned int	Default = 257;
#else
		const unsigned int	Default = 13;
#endif

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Trans::Configuration::LogFileTableSize --
	//		すべての論理ログファイルに関する情報を表すクラスを管理する
	//		ハッシュ表のサイズの設定に関する名前空間
	//
	//	NOTES

	namespace LogFileTableSize
	{
		//	CONST
		//	Trans::Configuration::LogFileTableSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "LogFileTableSize";

		//	CONST
		//	Trans::Configuration::LogFileTableSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES
		
#ifdef SYD_ARCH64
		const unsigned int	Default = 257;
#else
		const unsigned int	Default = 13;
#endif

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Trans::Configuration::InfoTableSize --
	//		データベースごとのトランザクション情報を管理する
	//		ハッシュ表のサイズの設定に関する名前空間
	//
	//	NOTES

	namespace InfoTableSize
	{
		//	CONST
		//	Trans::Configuration::InfoTableSize::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "InfoTableSize";

		//	CONST
		//	Trans::Configuration::InfoTableSize::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES
		
#ifdef SYD_ARCH64
		const unsigned int	Default = 257;
#else
		const unsigned int	Default = 13;
#endif

		// パラメーター値を取得する
		unsigned int		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Trans::Configuration::Category --
	//		デフォルトの種別の設定に関する名前空間
	//
	//	NOTES

	namespace Category
	{
		//	CONST
		//	Trans::Configuration::Category::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "Category";

		//	CONST
		//	Trans::Configuration::Category::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const Transaction::Category::Value	Default =
			Transaction::Category::Unknown;

		// パラメーター値を取得する
		Transaction::Category::Value get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Trans::Configuration::IsolationLevel --
	//		デフォルトのアイソレーションレベルの設定に関する名前空間
	//
	//	NOTES

	namespace IsolationLevel
	{
		//	CONST
		//	Trans::Configuration::IsolationLevel::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "IsolationLevel";

		//	CONST
		//	Trans::Configuration::IsolationLevel::Default --
		//		デフォルト値を表す定数
		//
		//	NOTES

		const Transaction::IsolationLevel::Value Default =
			Transaction::IsolationLevel::ReadCommitted;

		// パラメーター値を取得する
		Transaction::IsolationLevel::Value get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Trans::Configuration::NoLock --
	//		トランザクションでロックするかの設定に関する名前空間
	//
	//	NOTES

	namespace NoLock
	{
		//	CONST
		//	Trans::Configuration::NoLock::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "NoLock";

		//	CONST
		//	Trans::Configuration::NoLock::Default --
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
	//	Trans::Configuration::NoLogicalLog --
	//		更新トランザクションで論理ログを記録するかの設定に関する名前空間
	//
	//	NOTES

	namespace NoLogicalLog
	{
		//	CONST
		//	Trans::Configuration::NoLogicalLog::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "NoLogicalLog";

		//	CONST
		//	Trans::Configuration::NoLogicalLog::Default --
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
	//	Trans::Configuration::CompressLogicalLog --
	//		論理ログを圧縮して記録するかの設定に関する名前空間
	//
	//	NOTES

	namespace CompressLogicalLog
	{
		//	CONST
		//	Trans::Configuration::CompressLogicalLog::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "CompressLogicalLog";

		//	CONST
		//	Trans::Configuration::CompressLogicalLog::Default --
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
	//	Trans::Configuration::NoVersion --
	//		トランザクションで版管理を使用しないかの設定に関する名前空間
	//
	//	NOTES

	namespace NoVersion
	{
		//	CONST
		//	Trans::Configuration::NoVersion::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "NoVersion";

		//	CONST
		//	Trans::Configuration::NoVersion::Default --
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
	//	Trans::Configuration::TimeStampPath --
	//		タイムスタンプの上位 32 ビットを記録するファイルを生成する
	//		ディレクトリの絶対パス名の設定に関する名前空間
	//
	//	NOTES

	namespace TimeStampPath
	{
		//	CONST
		//	Trans::Configuration::TimeStampPath::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "TimeStampPath";

		// パラメーター値を取得する
		const Os::Path&		get();
		// 記録しているパラメーター値を忘れる
		void				reset();
	}

	//	NAMESPACE
	//	Trans::Configuration::TimeStampPermission --
	//		タイムスタンプの上位 32 ビットを記録する
	//		ファイルの許可モードの設定に関する名前空間
	//
	//	NOTES

	namespace TimeStampPermission
	{
		//	CONST
		//	Trans::Configuration::TimeStampPermission::Name --
		//		パラメーター名を表す定数
		//
		//	NOTES

		const char* const	Name = "TimeStampPermission";

		//	CONST
		//	Trans::Configuration::TimeStampPermission::Default --
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
}

_SYDNEY_TRANS_END
_SYDNEY_END

#endif	// __SYDNEY_TRANS_CONFIGURATION_H

//
// Copyright (c) 2001, 2002, 2004, 2007, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
