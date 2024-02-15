// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Configuration.h -- 論理ログモジュールの設定関連
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

#ifndef __SYDNEY_LOGICALLOG_CONFIGURATION_H
#define __SYDNEY_LOGICALLOG_CONFIGURATION_H

#include "LogicalLog/Module.h"
#include "Os/File.h"

_SYDNEY_BEGIN
_SYDNEY_LOGICALLOG_BEGIN

//
//	MACRO
//	DEFPARAM -- パラメータ定義
//
//	NOTES
//	パラメータ取得関数get()、reset()を宣言する
//
//  ARGUMENTS
//	  name_			パラメータ名
//	  type_			get()の返す型
//	  default_		パラメータが存在しない場合のデフォルト値(type_型)
//	  paramtype_	パラメータに記述するときの型
//					type_と同じときはValueTypeと書く
#define DEFPARAM(name_, type_, default_, paramtype_) 	\
	namespace name_										\
	{													\
		typedef type_ ValueType;						\
		ValueType get();								\
		void reset();									\
	}

//
//	NAMESPACE
//	LogicalLog::Configuration -- 論理ログモジュールの設定に関する名前空間
//
//	NOTES
//	パラメータ取得関数のための名前空間

//  FUNCTION
//    LogicalLog::Configuration::パラメータ名::get() -- パラメータ値を得る
//  ARGUMENTNS
//    なし
//  RETURN
//    LogicalLog::Configuration::パラメータ名::ValueType
//		得た値。一度得た値はメモリ内にキャッシュされる

//  FUNCTION
//    LogicalLog::Configuration::パラメータ名::reset() -- パラメータ値を忘れる
//	NOTES
//	  get()で得たキャッシュを忘れ、次回取得時に取り直すようにする
//  ARGUMENTNS
//    なし
//  RETURN
//    なし

namespace Configuration 
{
	//
	//	TYPEDEF
	//	LogicalLog::Configuration::ParamName -- パラメータ名
	//
	//	NOTES
	//	パラメータ名をあらわす型
	//
	typedef const char* ParamName;
#ifdef OBSOLETE
	// すべてのパラメータを読み出し、キャッシュする
	void					get();
#endif
	// すべてのパラメータのキャッシュを忘れる
	void					reset();

	//  論理ログファイル名から記述子を引くためのハッシュのサイズ
	DEFPARAM(FileTableSize,	int,			17,			ValueType)

	//  論理ログファイルの1ページの大きさ(バイト数)
	DEFPARAM(PageSize, 		int,			4 * 1024,	ValueType)

	//  論理ログファイルを一度に拡張するサイズ(B 単位)
	DEFPARAM(ExtensionSize, Os::File::Size,	4 * 1024 * 16,	int)

	// 論理ログをローテートするかどうかを決める閾値(B 単位)
	DEFPARAM(RotateThreshold, Os::File::Size, 100 * 1024 * 1024, int)

	//  論理ログモジュールのデバッグメッセージ出力先
	extern const ParamName MessageOutputDebug;
}

#undef DEFPARAM

_SYDNEY_LOGICALLOG_END
_SYDNEY_END

#endif //__SYDNEY_LOGICALLOG_CONFIGURATION_H

//
//	Copyright (c) 2000, 2004, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
