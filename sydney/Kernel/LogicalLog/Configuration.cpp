// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Configuration.cpp --
// 
// Copyright (c) 2000, 2004, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "LogicalLog";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"
#include "LogicalLog/Configuration.h"
#include "Common/SystemParameter.h"
#include "Os/CriticalSection.h"
#include "Os/AutoCriticalSection.h"

#include "ModCharString.h"

_SYDNEY_BEGIN
_SYDNEY_LOGICALLOG_BEGIN

namespace 
{
namespace _Configuration
{
	// 設定値取得の排他制御用のラッチ
	Os::CriticalSection		_latch;
}
}

namespace Configuration {

#ifdef OBSOLETE
//	FUNCTION
//	LogicalLog::Configuration::get -- パラメータをキャッシュする
//
//	NOTES
//	LogicalLogモジュールで使うパラメータを読み込み、
//  メモリ内にキャッシュする
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
get()
{
	(void) FileTableSize::get();
	(void) PageSize::get();
	(void) ExtensionSize::get();
	(void) RotateThreshold::get();
}
#endif

//	FUNCTION
//	LogicalLog::Configuration::reset -- パラメータキャッシュをクリア
//
//	NOTES
//	メモリ内にキャッシュしたパラメータ値を忘れ、
//  次回レジストリから取り直すようにする
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void reset()
{
	FileTableSize::reset();
	PageSize::reset();
	ExtensionSize::reset();
	RotateThreshold::reset();
}

//
//	MACRO
//	DEFPARAM -- パラメータ定義
//
//	NOTES
//	パラメータ取得関数get()、reset()の実体を定義する
//
//  ARGUMENTS
//	  name_			パラメータ名
//	  type_			get()の返す型
//	  default_		パラメータが存在しない場合のデフォルト値(type_型)
//	  paramtype_	パラメータに記述するときの型
//					type_と同じときはValueTypeと書く
#define DEFPARAM(name_, type_, default_, paramtype_) 						\
namespace name_												\
{																			\
	ParamName const Name = #name_;											\
    typedef type_ ValueType;												\
	typedef paramtype_ ParamType;											\
	ValueType const Default = (default_);									\
																			\
	bool bCached = false;													\
	ValueType Value;														\
																			\
	ValueType get()															\
	{																		\
        if (!bCached) {														\
			Os::AutoCriticalSection	latch(_Configuration::_latch);			\
			if (!bCached) {													\
				ParamType v;												\
				if (Common::SystemParameter::getValue(						\
							ModCharString(moduleName) + '_' + Name, v)) { 	\
					Value = v;												\
				} else {													\
					Value = Default;										\
				}															\
 				bCached = true;												\
			}																\
		}																	\
		return Value;														\
	}																		\
																			\
	void reset()															\
	{																		\
		Os::AutoCriticalSection	latch(_Configuration::_latch);				\
		bCached = false;													\
	}																		\
}

//
//  NAMESPACE
//  LogicalLog::Configuration::FileTableSize -- ファイルハッシュサイズ
//
//  NOTES
//  パラメータ
//  論理ログファイル名から記述子を引くためのハッシュのサイズ
//
DEFPARAM(FileTableSize, int, 17, ValueType)

//
//  NAMESPACE
//  LogicalLog::Configuration::PageSize -- ページサイズ
//
//  NOTES
//  パラメータ
//  論理ログファイルの1ページの大きさ(バイト数)
//
DEFPARAM(PageSize, int, 4 * 1024, ValueType)

//  NAMESPACE
//  LogicalLog::Configuration::ExtensionSize --
//		論理ログファイルを一度に拡張するサイズ
//
//  NOTES

DEFPARAM(ExtensionSize, Os::File::Size, 4 * 1024 * 16, int)

//  NAMESPACE
//  LogicalLog::Configuration::RotateThreshold --
//		論理ログをローテートするかどうかを決める閾値
//
//  NOTES

DEFPARAM(RotateThreshold, Os::File::Size, 100 * 1024 * 1024, int)

#undef DEFPARAM

//
//  VARIABLE
//	LogicalLog::Configuration::MessageOutputDebug
//
//	NOTES
//	デバッグログ出力先を指定するパラメータ名
//
const ParamName MessageOutputDebug = "LogicalLog_MessageOutputDebug";

}

_SYDNEY_LOGICALLOG_END
_SYDNEY_END

//
//	Copyright (c) 2000, 2004, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
