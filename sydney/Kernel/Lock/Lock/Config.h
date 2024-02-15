// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Config.h -- ロックマネージャの設定関連の関数宣言
// 
// Copyright (c) 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOCK_CONFIG_H
#define __SYDNEY_LOCK_CONFIG_H

#include "Common/Common.h"

#include "Lock/Module.h"
#include "Lock/Timeout.h"

#include "ModTypes.h"

_SYDNEY_BEGIN
_SYDNEY_LOCK_BEGIN

//	NAMESPACE
//	Lock::Config -- ロックマネージャの設定に関する名前空間
//
//	NOTES

namespace Config
{
	// すべてのパラメータ値を記録する
	SYD_LOCK_FUNCTION void get();

	// 記録しているすべてのパラメータ値を抹消する
	SYD_LOCK_FUNCTION void reset();

	//	NAMESPACE
	//	Lock::Config::CountTableSize --
	//		すべてのロック数を管理するハッシュ表のサイズの設定に関する名前空間
	//
	//	NOTES

	namespace CountTableSize
	{
		//	CONST
		//	Lock::Config::CountTableSize::Name --
		//		パラメータ名を表す文字列定数
		//
		//	NOTES

		const char* const	Name = "CountTableSize";

		//	CONST
		//	Lock::Config::CountTableSize::Default --
		//		デフォルト値
		//
		//	NOTES

		const unsigned int	Default = 0x2000;	// 8192

		// パラメータ値を返す
		SYD_LOCK_FUNCTION
		unsigned int		get();
		// 記録しているパラメータ値を抹消する
		SYD_LOCK_FUNCTION
		void				reset();
	}

	//	NAMESPACE
	//	Lock::Config::LackOfParentDetection -- 
	//		あるロック項目（表／タプル）にロックを掛ける際に、
	//		親となるロック項目（データベース／表）に
	//		十分なロックが掛けられているかを検査するかどうかの
	//		設定に関する名前空間
	//
	//	NOTES

	namespace LackOfParentDetection
	{
		//	CONST
		//	Lock::Config::LackOfParentDetection::Name --
		//		パラメータ名を表す文字列定数
		//
		//	NOTES

		const char* const	Name = "LackOfParentDetection";

		//	CONST
		//	Lock::Config::LackOfParentDetection::Default --
		//		デフォルト値
		//
		//	NOTES

		const bool	Default = false;

		// パラメータ値を返す
		SYD_LOCK_FUNCTION bool get();

		// 記録しているパラメータ値を抹消する
		SYD_LOCK_FUNCTION void reset();
	}

	//	NAMESPACE
	//	Lock::Config::EnableDowngrade --
	//		ロックのダウングレードが可能かどうかの設定に関する名前空間
	//
	//	NOTES

	namespace EnableDowngrade
	{
		//	CONST
		//	Lock::Config::EnableDowngrade::Name --
		//		パラメータ名を表す文字列定数
		//
		//	NOTES

		const char* const	Name = "EnableDowngrade";

		//	CONST
		//	Lock::Config::EnableDowngrade::Default --
		//		デフォルト値
		//
		//	NOTES

		const bool	Default = true;

		// パラメータ値を返す
		SYD_LOCK_FUNCTION bool get();

		// 記録しているパラメータ値を抹消する
		SYD_LOCK_FUNCTION void reset();
	}

	//	NAMESPACE
	//	Lock::Config::LackForChildDetection --
	//		あるロック項目（データベース／表）に掛けられている
	//		ロックを外す際に、子となるロック項目（表／タプル）に対して
	//		十分なロックでなくなるかを検査するかどうかの設定に関する名前空間
	//
	//	NOTES

	namespace LackForChildDetection
	{
		//	CONST
		//	Lock::Config::LackForChildDetection::Name --
		//		パラメータ名を表す文字列定数
		//
		//	NOTES

		const char* const	Name = "LackForChildDetection";

		//	CONST
		//	Lock::Config::LackForChildDetection::Default --
		//		デフォルト値
		//
		//	NOTES

		const bool	Default = false;

		// パラメータ値を返す
		SYD_LOCK_FUNCTION bool get();

		// 記録しているパラメータ値を抹消する
		SYD_LOCK_FUNCTION void reset();
	}

	//	NAMESPACE
	//	Lock::Config::TimeOut --
	//		ロック待ち時間の設定に関する名前空間
	//
	//	NOTES

	namespace Timeout
	{
		//	CONST
		//	Lock::Config::Timeout::Name --
		//		パラメータ名を表す文字列定数
		//
		//	NOTES

		const char* const	Name = "Timeout";

		//	CONST
		//	Lock::Config::Timeout::Default --
		//		デフォルト値
		//
		//	NOTES

		const Lock::Timeout::Value	Default = Lock::Timeout::Unlimited;

		// 記録しているパラメータ値を変更する
		SYD_LOCK_FUNCTION void set(Lock::Timeout::Value	value);

		// パラメータ値を返す
		SYD_LOCK_FUNCTION Lock::Timeout::Value get();

		// 記録しているパラメータ値を抹消する
		SYD_LOCK_FUNCTION void reset();
	}

	//	NAMESPACE
	//	Lock::Config::HashSize --
	//		ロック項目のハッシュテーブルの要素数の
	//		設定に関する名前空間
	//
	//	NOTES

	namespace HashSize
	{
		//	TYPEDEF
		//	Lock::Config::HashSize::Value --
		//		ロック項目のハッシュテーブルの要素数を表す値の型
		//
		//	NOTES

		typedef ModSize	Value;

		//	CONST
		//	Lock::Config::HashSize::Name --
		//		パラメータ名を表す文字列定数
		//
		//	NOTES

		const char* const	Name = "HashSize";

		//	CONST
		//	Lock::Config::HashSize::Default --
		//		デフォルト値
		//
		//	NOTES

		const Value	Default = 3000;

		// パラメータ値を返す
		SYD_LOCK_FUNCTION Value get();

		// 記録しているパラメータ値を抹消する
		SYD_LOCK_FUNCTION void reset();
	}

	//	NAMESPACE
	//	Lock::Config::ItemInstanceCacheSize --
	//		ロック項目のインスタンスをキャッシュするサイズ
	//
	//	NOTES

	namespace ItemInstanceCacheSize
	{
		//	TYPEDEF
		//	Lock::Config::ItemInstanceCacheSize::Value --
		//		ロック項目のインスタンスをキャッシュするサイズを表す値の型
		//
		//	NOTES

		typedef ModSize	Value;

		//	CONST
		//	Lock::Config::ItemInstanceCacheSize::Name --
		//		パラメータ名を表す文字列定数
		//
		//	NOTES

		const char* const	Name = "ItemInstanceCacheSize";

		//	CONST
		//	Lock::Config::ItemInstanceCacheSize::Default --
		//		デフォルト値
		//
		//	NOTES

		const Value	Default = 5000;

		// パラメータ値を返す
		SYD_LOCK_FUNCTION Value get();

		// 記録しているパラメータ値を抹消する
		SYD_LOCK_FUNCTION void reset();
	}

	//	NAMESPACE
	//	Lock::Config::RequestInstanceCacheSize --
	//		リクエストのインスタンスをキャッシュするサイズ
	//
	//	NOTES

	namespace RequestInstanceCacheSize
	{
		//	TYPEDEF
		//	Lock::Config::RequestInstanceCacheSize::Value --
		//		リクエストのインスタンスをキャッシュするサイズを表す値の型
		//
		//	NOTES

		typedef ModSize	Value;

		//	CONST
		//	Lock::Config::RequestInstanceCacheSize::Name --
		//		パラメータ名を表す文字列定数
		//
		//	NOTES

		const char* const	Name = "RequestInstanceCacheSize";

		//	CONST
		//	Lock::Config::RequestInstanceCacheSize::Default --
		//		デフォルト値
		//
		//	NOTES

		const Value	Default = 5000;

		// パラメータ値を返す
		SYD_LOCK_FUNCTION Value get();

		// 記録しているパラメータ値を抹消する
		SYD_LOCK_FUNCTION void reset();
	}

}

_SYDNEY_LOCK_END
_SYDNEY_END

#endif //__SYDNEY_LOCK_CONFIG_H

//
//	Copyright (c) 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
