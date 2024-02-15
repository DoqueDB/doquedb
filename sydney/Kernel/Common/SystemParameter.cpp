// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemParameter.cpp -- システムパラメータクラス
// 
// Copyright (c) 1999, 2001, 2002, 2003, 2004, 2005, 2007, 2009, 2010, 2013, 2017, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Common";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/Assert.h"
#include "Common/SystemParameter.h"

#include "Common/Configuration.h"
#include "Common/UnicodeString.h"
#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"
#include "Os/Limits.h"

#include "ModCharString.h"
#include "ModParameter.h"
#if MOD_CONF_REGISTRY == 0
#include "ModParameterSource.h"
#endif
#include "ModUnicodeString.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace
{

namespace _SystemParameter
{
	// 排他制御用
	Os::CriticalSection		_latch;

	// パラメータオブジェクトのマップ
	typedef ModMap<Common::Configuration::Base*,
				   Common::Configuration::Base*,
				   ModLess<Common::Configuration::Base*> > _ParameterMap;
	_ParameterMap			_paramMap;
	
#if MOD_CONF_REGISTRY == 0
	// パス名
	ModUnicodeString		_parent;
	// システムパラメータを管理するパラメータオブジェクト
	ModParameter*			_parameter = 0;

	// パラメータファイルを指定する環境変数の名前
	const char*				_envName = "SYDPARAM";
	// システムパラメータファイルを指定する環境変数の名前
	const char*				_sysEnvName = "SYDSYSPARAM";

#endif
#if MOD_CONF_REGISTRY == 1
	// 親パス名
	ModUnicodeString		_parent;
	// 親パス名のデフォルト
	const ModUnicodeString	_default(_TRMEISTER_U_STRING("HKEY_LOCAL_MACHINE\\SOFTWARE\\Ricoh\\TRMeister"));
#endif

	// TEMPLATE FUNCTION public
	//	$$$::_stringToInt -- 文字列を数値に変換する
	//
	// TEMPLATE ARGUMENTS
	//	class Value_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	const ModUnicodeChar* p_
	//	Value_& result_
	//	
	// RETURN
	//	bool
	//
	// EXCEPTIONS

	template <class Value_>
	bool
	_stringToInt(const ModUnicodeChar* p_, Value_& result_)
	{
		Value_ i = 0;
		const Value_ maxTenth = Os::Limits<Value_>::getMax() / 10;

		while (*p_)
		{
			if (*p_ >= '0' && *p_ <= '9')
			{
				int d = *p_ - '0';
				if (maxTenth < i) return false;
				i *= 10;
				if (Os::Limits<Value_>::getMax() - d < i) return false;
				i += d;
			}
			else
			{
				if (*p_ == 'K' || *p_ == 'k') {
					if ((Os::Limits<Value_>::getMax() >> 10) < i) return false;
					i = i << 10;
				} else if (*p_ == 'M' || *p_ == 'm') {
					if ((Os::Limits<Value_>::getMax() >> 10 >> 10) < i) return false;
					i = i << 10 << 10;
				} else if (*p_ == 'G' || *p_ == 'g') {
					if ((Os::Limits<Value_>::getMax() >> 10 >> 10 >> 10) < i) return false;
					i = i << 10 << 10 << 10;
				} else if (*p_ == 'T' || *p_ == 't') {
					if ((Os::Limits<Value_>::getMax() >> 10 >> 10 >> 10 >> 10) < i) return false;
					i = i << 10 << 10 << 10 << 10;
				}
				break;
			}
			++p_;
		}
		result_ = i;
		return true;
	}
}

}

//	FUNCTION
//	Common::SystemParameter::initialize --
//		システムパラメータを使うための初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	parent
//			システムパラメータの親パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemParameter::initialize(const ModUnicodeString& parent)
{
	Os::AutoCriticalSection cAuto(_SystemParameter::_latch);
#if MOD_CONF_REGISTRY == 0
	_SystemParameter::_parent = parent;
	prepare(_SystemParameter::_parent);
#endif
#if MOD_CONF_REGISTRY == 1
	if (parent.getLength()) {
		_SystemParameter::_parent = parent;
	} else
		_SystemParameter::_parent = _SystemParameter::_default;
#endif
}

//	FUNCTION
//	Common::SystemParameter::terminate --
//		システムパラメータを使うための後処理を行う
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
SystemParameter::terminate()
{
	Os::AutoCriticalSection cAuto(_SystemParameter::_latch);
#if MOD_CONF_REGISTRY == 0
	delete _SystemParameter::_parameter, _SystemParameter::_parameter = 0;
#endif
}

//	FUNCTION
//	Common::SystemParameter::reset -- リセットする
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
SystemParameter::reset()
{
	Os::AutoCriticalSection cAuto(_SystemParameter::_latch);
#if MOD_CONF_REGISTRY == 0
	prepare(_SystemParameter::_parent);
#endif
	_SystemParameter::_ParameterMap::Iterator i
		= _SystemParameter::_paramMap.begin();
	for (; i != _SystemParameter::_paramMap.end(); ++i)
	{
		(*i).first->clear();
	}
}

//	FUNCTION
//	Common::SystemParameter::getValue -- 文字列値を取り出す
//
//	NOTES
//
//	ARGUMENTS
//		ModCharString&		name
//			この名前のシステムパラメーターの文字列値を得る
//		ModUnicodeString&	value
//			得られた文字列値を格納する
//
//	RETURN
//		true
//			値が得られた
//		false
//			値は得られなかった
//
//	EXCEPTIONS
//          下位からくるかもしれない

bool
SystemParameter::getValue(const ModCharString& name, ModUnicodeString& value)
{
#if MOD_CONF_REGISTRY == 0
	; _TRMEISTER_ASSERT(_SystemParameter::_parameter);
	Os::AutoCriticalSection cAuto(_SystemParameter::_latch);
	return _SystemParameter::_parameter->getUnicodeString(
		value, name.getString());
#endif
#if MOD_CONF_REGISTRY == 1
	return getValue(
		ModUnicodeString(name, ModOs::Process::getEncodingType()), value);
#endif
}

bool
SystemParameter::getValue(
	const ModUnicodeString& name, ModUnicodeString& value)
{
	ModUnicodeString tmp(name);

#if MOD_CONF_REGISTRY == 0
	; _TRMEISTER_ASSERT(_SystemParameter::_parameter);
	Os::AutoCriticalSection cAuto(_SystemParameter::_latch);
	return _SystemParameter::_parameter->getUnicodeString(
		value, tmp.getString(ModOs::Process::getEncodingType()));
#endif
#if MOD_CONF_REGISTRY == 1
	; _TRMEISTER_ASSERT(_SystemParameter::_parent.getLength());
	return ModParameter::getString(_SystemParameter::_parent, tmp, value);
#endif	
}

//	FUNCTION
//	Common::SystemParameter::getValue -- 整数値を取り出す
//
//	NOTES
//
//	ARGUMENTS
//		ModCharString&		name
//			この名前のシステムパラメーターの整数値を得る
//		int&				value
//			得られた整数値を格納する
//
//	RETURN
//		true
//			値が得られた
//		false
//			値は得られなかった
//
//	EXCEPTIONS

bool
SystemParameter::getValue(const ModCharString& name, int& value)
{
	ModUnicodeString cstrValue;
	if (getValue(name, cstrValue)) {
		return _SystemParameter::_stringToInt(cstrValue, value);
	}
	return false;
}

bool
SystemParameter::getValue(const ModUnicodeString& name, int& value)
{
	ModUnicodeString cstrValue;
	if (getValue(name, cstrValue)) {
		return _SystemParameter::_stringToInt(cstrValue, value);
	}
	return false;
}

//	FUNCTION
//	Common::SystemParameter::getValue -- 真偽値を取り出す
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	name
//			この名前のシステムパラメーターの真偽値を得る
//		bool&				value
//			得られた真偽値を格納する
//
//	RETURN
//		true
//			値が得られた
//		false
//			値は得られなかった
//
//	EXCEPTIONS

bool
SystemParameter::getValue(const ModCharString& name, bool& value)
{
#if MOD_CONF_REGISTRY == 0
	Os::AutoCriticalSection cAuto(_SystemParameter::_latch);
	ModBoolean v = ModFalse;
	; _TRMEISTER_ASSERT(_SystemParameter::_parameter);
	const ModBoolean result =
		_SystemParameter::_parameter->getBoolean(v, name.getString());
	value = v;
	return result;
#endif
#if MOD_CONF_REGISTRY == 1
	return getValue(
		ModUnicodeString(name, ModOs::Process::getEncodingType()), value);
#endif
}

bool
SystemParameter::getValue(const ModUnicodeString& name, bool& value)
{
	ModUnicodeString tmp(name);

#if MOD_CONF_REGISTRY == 0
	Os::AutoCriticalSection cAuto(_SystemParameter::_latch);
	ModBoolean v = ModFalse;
	; _TRMEISTER_ASSERT(_SystemParameter::_parameter);
	const ModBoolean result =
		_SystemParameter::_parameter->getBoolean(
			v, tmp.getString(ModOs::Process::getEncodingType()));
	value = v;
	return result;
#endif
#if MOD_CONF_REGISTRY == 1
	; _TRMEISTER_ASSERT(_SystemParameter::_parent.getLength());

	//	【注意】	ModParameter::getBoolean は
	//				設定されている値の型が真偽値でないとき、
	//				"TRUE" が設定されているときのみ、true が設定され、
	//				それ以外はなにも設定されないので、
	//				ここでは false で初期化しておく

	ModBoolean v = ModFalse;
	if (ModParameter::getBoolean(_SystemParameter::_parent, tmp, v)) {
		value = v;
		return true;
	}
	return false;
#endif
}

bool
SystemParameter::
getValue(const ModCharString& name, unsigned int& value)
{
	ModUnicodeString cstrValue;
	if (getValue(name, cstrValue)) {
		return _SystemParameter::_stringToInt(cstrValue, value);
	}
	return false;
}

bool
SystemParameter::
getValue(const ModUnicodeString& name, unsigned int& value)
{
	ModUnicodeString cstrValue;
	if (getValue(name, cstrValue)) {
		return _SystemParameter::_stringToInt(cstrValue, value);
	}
	return false;
}

bool
SystemParameter::
getValue(const ModCharString& name, ModInt64& value)
{
	ModUnicodeString cstrValue;
	if (getValue(name, cstrValue)) {
		return _SystemParameter::_stringToInt(cstrValue, value);
	}
	return false;
}

bool
SystemParameter::
getValue(const ModUnicodeString& name, ModInt64& value)
{
	ModUnicodeString cstrValue;
	if (getValue(name, cstrValue)) {
		return _SystemParameter::_stringToInt(cstrValue, value);
	}
	return false;
}

bool
SystemParameter::
getValue(const ModCharString& name, ModUInt64& value)
{
	ModUnicodeString cstrValue;
	if (getValue(name, cstrValue)) {
		return _SystemParameter::_stringToInt(cstrValue, value);
	}
	return false;
}

bool
SystemParameter::
getValue(const ModUnicodeString& name, ModUInt64& value)
{
	ModUnicodeString cstrValue;
	if (getValue(name, cstrValue)) {
		return _SystemParameter::_stringToInt(cstrValue, value);
	}
	return false;
}

//	FUNCTION
//	Common::SystemParameter::getString -- 文字列を取り出す
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	name
//			パラメータ名
//
//	RETURN
//		得られた値
//
//	EXCEPTIONS

ModUnicodeString
SystemParameter::getString(const ModCharString& name)
{
	ModUnicodeString v;
	(void) getValue(name, v);
	return v;
}

ModUnicodeString
SystemParameter::getString(const ModUnicodeString& name)
{
	ModUnicodeString v;
	(void) getValue(name, v);
	return v;
}

//	FUNCTION
//	Common::SystemParameter::getInteger -- 32 ビット整数を取り出す
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	name
//			パラメータ名
//
//	RETURN
//		得られた値
//
//	EXCEPTIONS

#ifdef OBSOLETE
int
SystemParameter::getInteger(const ModCharString& name)
{
	int v = 0;
	(void) getValue(name, v);
	return v;
}
#endif
int
SystemParameter::getInteger(const ModUnicodeString& name)
{
	int v = 0;
	(void) getValue(name, v);
	return v;
}

//	FUNCTION
//	Common::SystemParameter::getBoolean -- 論理値を取り出す
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	name
//			パラメータ名
//
//	RETURN
//		得られた値
//
//	EXCEPTIONS

#ifdef OBSOLETE
bool
SystemParameter::getBoolean(const ModCharString& name)
{
	bool v = false;
	(void) getValue(name, v);
	return v;
}
#endif
bool
SystemParameter::getBoolean(const ModUnicodeString& name)
{
	bool v = false;
	(void) getValue(name, v);
	return v;
}

//	FUNCTION
//	Common::SystemParameter::insert -- パラメータオブジェクトを登録する
//
//	NOTES
//
//	ARGUMENTS
//	Common::Configuration::Base* p_
//		登録するオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
SystemParameter::insert(Common::Configuration::Base* p_)
{
	Os::AutoCriticalSection cAuto(_SystemParameter::_latch);

	// 重複チェックはしない
	_SystemParameter::_paramMap.insert(
		_SystemParameter::_ParameterMap::ValueType(p_, p_));
}

//	FUNCTION
//	Common::SystemParameter::erase -- パラメータオブジェクトを削除する
//
//	NOTES
//
//	ARGUMENTS
//	Common::Configuration::Base* p_
//		削除するオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
SystemParameter::erase(Common::Configuration::Base* p_)
{
	Os::AutoCriticalSection cAuto(_SystemParameter::_latch);
	_SystemParameter::_ParameterMap::Iterator i
		= _SystemParameter::_paramMap.find(p_);
	if (i != _SystemParameter::_paramMap.end())
		_SystemParameter::_paramMap.erase(i);
}

//	FUNCTION
//	Common::SystemParameter::prepare -- 準備する
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	parent
//			システムパラメータの親パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemParameter::prepare(const ModUnicodeString& parent)
{
	//【注意】呼び出し側で排他する必要がある
	
#if MOD_CONF_REGISTRY == 0
	delete _SystemParameter::_parameter, _SystemParameter::_parameter = 0;
	ModUnicodeString tmp(parent);
	_SystemParameter::_parameter = new ModParameter(
		ModParameterSource(
			tmp.getLength() ?
			tmp.getString(ModOs::Process::getEncodingType()) : 0,
			_SystemParameter::_envName, 0,
			_SystemParameter::_sysEnvName, 0), ModTrue);
	; _TRMEISTER_ASSERT(_SystemParameter::_parameter);
#endif
}

#ifdef OBSOLETE
//	FUNCTION
//	Common::SystemParameter::getEnvironment --
//		パラメータファイル名の環境変数の名前を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた環境変数名
//
//	EXCEPTIONS
//		なし

// static
const ModUnicodeString&
SystemParameter::getEnvironment()
{
	return _TRMEISTER_U_STRING(_envName);
}

//	FUNCTION
//	Common::SystemParameter::getSystemEnvironment --
//		システムパラメータファイル名の環境変数の名前を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた環境変数名
//
//	EXCEPTIONS
//		なし

// static
const ModUnicodeString&
SystemParameter::getSystemEnvironment()
{
	return _TRMEISTER_U_STRING(_sysEnvName);
}
#endif

//
//	Copyright (c) 1999, 2001, 2002, 2003, 2004, 2005, 2007, 2009, 2010, 2013, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
