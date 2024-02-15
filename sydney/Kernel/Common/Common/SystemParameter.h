// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemParameter.h -- システムパラメータクラス
// 
// Copyright (c) 1999, 2000, 2002, 2003, 2004, 2010, 2013, 2017, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_SYSTEMPARAMETER_H
#define __TRMEISTER_COMMON_SYSTEMPARAMETER_H

#include "Common/Module.h"
#include "ModTypes.h"

class ModCharString;
class ModUnicodeString;

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

namespace Configuration
{
class Base;
}

//	NAMESPACE
//	Common::SystemParatemer -- システムパラメータを操作するための名前空間
//
//	NOTES

namespace SystemParameter
{
	// システムパラメータを使うための初期化を行う
	SYD_COMMON_FUNCTION
	void
	initialize(const ModUnicodeString& parent);
	// システムパラメータを使うための後処理を行う
	SYD_COMMON_FUNCTION
	void
	terminate();
	
	// システムパラメータをリセットする
	SYD_COMMON_FUNCTION
	void reset();

	// 値を取り出す
	SYD_COMMON_FUNCTION
	bool
	getValue(const ModCharString& key, ModUnicodeString& value);
	SYD_COMMON_FUNCTION
	bool
	getValue(const ModUnicodeString& key, ModUnicodeString& value);
	SYD_COMMON_FUNCTION
	bool
	getValue(const ModCharString& key, int& value);
	SYD_COMMON_FUNCTION
	bool
	getValue(const ModUnicodeString& key, int& value);
	SYD_COMMON_FUNCTION
	bool
	getValue(const ModCharString& key, bool& value);
	SYD_COMMON_FUNCTION
	bool
	getValue(const ModUnicodeString& key, bool& value);
	SYD_COMMON_FUNCTION
	bool
	getValue(const ModCharString& key, unsigned int& value);
	SYD_COMMON_FUNCTION
	bool
	getValue(const ModUnicodeString& key, unsigned int& value);
	SYD_COMMON_FUNCTION
	bool
	getValue(const ModCharString& key, ModInt64& value);
	SYD_COMMON_FUNCTION
	bool
	getValue(const ModUnicodeString& key, ModInt64& value);
	SYD_COMMON_FUNCTION
	bool
	getValue(const ModCharString& key, ModUInt64& value);
	SYD_COMMON_FUNCTION
	bool
	getValue(const ModUnicodeString& key, ModUInt64& value);

	// 文字列を取り出す
	SYD_COMMON_FUNCTION
	ModUnicodeString
	getString(const ModCharString& key);
	SYD_COMMON_FUNCTION
	ModUnicodeString
	getString(const ModUnicodeString& key);
	// 32 ビット整数を取り出す
#ifdef OBSOLETE
	SYD_COMMON_FUNCTION
	int
	getInteger(const ModCharString& key);
#endif
	SYD_COMMON_FUNCTION
	int
	getInteger(const ModUnicodeString& key);
	// 論理値を取り出す
#ifdef OBSOLETE
	SYD_COMMON_FUNCTION
	bool
	getBoolean(const ModCharString& key);
#endif
	SYD_COMMON_FUNCTION
	bool
	getBoolean(const ModUnicodeString& key);

	// Common::Configuration::Objectを登録する
	SYD_COMMON_FUNCTION
	void insert(Common::Configuration::Base* p_);
	// Common::Configuration::Objectを削除する
	SYD_COMMON_FUNCTION
	void erase(Common::Configuration::Base* p_);

	// システムパラメータの準備をする
	void prepare(const ModUnicodeString& parent);

#ifdef OBSOLETE
	// パラメーターファイルを指定する環境変数の名前を得る
	SYD_COMMON_FUNCTION
	const ModUnicodeString&
	getEnvironment();
	// システムパラメーターファイルを指定する環境変数の名前を得る
	SYD_COMMON_FUNCTION
	const ModUnicodeString&
	getSystemEnvironment();
#endif
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_SYSTEMPARAMETER_H

//
//	Copyright (c) 1999, 2000, 2002, 2003, 2004, 2010, 2013, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
