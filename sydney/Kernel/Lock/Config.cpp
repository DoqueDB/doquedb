// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Config.cpp -- ロックマネージャの設定関連の関数定義
// 
// Copyright (c) 2000, 2002, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Lock";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Lock/Config.h"


#include "Common/Parameter.h"
#include "Common/SystemParameter.h"
#include "Exception/NotSupported.h"
#include "Os/AutoCriticalSection.h"

#include "ModCharString.h"

_SYDNEY_USING
_SYDNEY_LOCK_USING

namespace
{

namespace _Config
{
	// 設定値取得の排他制御用のラッチ
	Os::CriticalSection	Latch;

	namespace _CountTableSize
	{
		bool			_first = true;
		unsigned int	_value;
	}

	namespace _LackOfParentDetection
	{
		bool	_first = true;
		bool	_value;
	}

	namespace _EnableDowngrade
	{
		bool	_first = true;
		bool	_value;
	}

	namespace _LackForChildDetection
	{
		bool	_first = true;
		bool	_value;
	}

	namespace _Timeout
	{
		bool					_first = true;
		Lock::Timeout::Value	_value;
	}

	namespace _HashSize
	{
		bool					_first = true;
		Config::HashSize::Value	_value;
	}

	namespace _ItemInstanceCacheSize
	{
		bool									_first = true;
		Config::ItemInstanceCacheSize::Value	_value;
	}

	namespace _RequestInstanceCacheSize
	{
		bool									_first = true;
		Config::RequestInstanceCacheSize::Value	_value;
	}

	//	FUNCTION
	//	Lock::_Config::get() -- システムパラメータからパラメータ値を取得する
	//
	//	NOTES
	//
	//	ARGUMENTS
	//	const char*						ParamName_
	//		パラメータ名文字列へのポインタ
	//	const Common::Parameter::Type	Type_
	//		パラメータ値のデータ型
	//	void*							Value_
	//		パラメータ値へのポインタ
	//
	//	RETURN
	//	bool
	//		true  : パラメータ値を取得できた
	//		false : パラメータ値を取得できなかった
	//
	//	EXCEPTIONS

	bool
	get(const char*						ParamName_,
		const Common::Parameter::Type	Type_,
		void*							Value_)
	{
		bool	exist = false;

		ModCharString	paramName(moduleName);
		paramName.append('_');
		paramName.append(ParamName_);

		switch (Type_) {
		case Common::Parameter::TypeInteger:
		{
			int	value = 0;
			exist = Common::SystemParameter::getValue(paramName, value);
			if (exist) *(static_cast<int*>(Value_)) = value;
			break;
		}
		case Common::Parameter::TypeBoolean:
		{
			bool	value = false;
			exist = Common::SystemParameter::getValue(paramName, value);
			if (exist) *(static_cast<bool*>(Value_)) = value;
			break;
		}
		default:
			_SYDNEY_THROW0(Exception::NotSupported);
		}

		return exist;
	}

} // end of namespace _Config

} // end of namespace (global)

//	FUNCTION
//	Lock::Config::get --
//		すべてのパラメータ値を記録する
//
//	NOTES
//	必要に応じて、すべてのパラメータ値を
//	システムパラメータから取得し、記録する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Config::get()
{
	Config::CountTableSize::get();
	Config::LackOfParentDetection::get();
	Config::EnableDowngrade::get();
	Config::LackForChildDetection::get();
	Config::Timeout::get();
	Config::HashSize::get();
	Config::ItemInstanceCacheSize::get();
	Config::RequestInstanceCacheSize::get();
}

//	FUNCTION
//	Lock::Config::reset --
//		記録しているすべてのパラメータ値を抹消する
//
//	NOTES
//	（システムパラメータから取得し）記録している
//	すべてのパラメータ値を抹消する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Config::reset()
{
	Config::CountTableSize::reset();
	Config::LackOfParentDetection::reset();
	Config::EnableDowngrade::reset();
	Config::LackForChildDetection::reset();
	Config::Timeout::reset();
	Config::HashSize::reset();
	Config::ItemInstanceCacheSize::reset();
	Config::RequestInstanceCacheSize::reset();
}

//	FUNCTION
//	Lock::Config::CountTableSize::get -- パラメータ値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた値
//
//	EXCEPTIONS

unsigned int
Config::CountTableSize::get()
{
	if (_Config::_CountTableSize::_first) {

		Os::AutoCriticalSection	latch(_Config::Latch);

		if (_Config::_CountTableSize::_first) {
			int v;
			if (!_Config::get(Config::CountTableSize::Name,
							  Common::Parameter::TypeInteger, &v))
				v = Config::CountTableSize::Default;

			_Config::_CountTableSize::_value = v;
			_Config::_CountTableSize::_first = false;
		}
	}

	return _Config::_CountTableSize::_value;
}

//	FUNCTION
//	Lock::Config::CountTableSize::reset --
//		記録しているパラメータ値を抹消する
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
Config::CountTableSize::reset()
{
	Os::AutoCriticalSection	latch(_Config::Latch);

	_Config::_CountTableSize::_first = true;
}

//	FUNCTION
//	Lock::Config::LackOfParentDetection::get --
//		パラメータ値を返す
//
//	NOTES
//	パラメータ値を返す。
//	まだパラメータ値を記録していない場合には、
//	システムパラメータから取得し、記録する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		パラメータ値
//
//	EXCEPTIONS

bool
Config::LackOfParentDetection::get()
{
	if (_Config::_LackOfParentDetection::_first) {

		Os::AutoCriticalSection	latch(_Config::Latch);

		if (_Config::_LackOfParentDetection::_first) {
			bool	value;
			if (_Config::get(Config::LackOfParentDetection::Name,
							 Common::Parameter::TypeBoolean,
							 &value)
				== false) {
				value = Config::LackOfParentDetection::Default;
			}
			_Config::_LackOfParentDetection::_value = value;
			_Config::_LackOfParentDetection::_first = false;
		}
	}

	return _Config::_LackOfParentDetection::_value;
}

//	FUNCTION
//	Lock::Config::LackOfParentDetection::reset --
//		記録しているパラメータ値を抹消する
//
//	NOTES
//	（システムパラメータから取得し）記録している
//	パラメータ値を抹消する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Config::LackOfParentDetection::reset()
{
	Os::AutoCriticalSection	latch(_Config::Latch);

	_Config::_LackOfParentDetection::_first = true;
}

//	FUNCTION
//	Lock::Config::EnableDowngrade::get --
//		パラメータ値を返す
//
//	NOTES
//	パラメータ値を返す。
//	まだパラメータ値を記録していない場合には、
//	システムパラメータから取得し、記録する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		パラメータ値
//
//	EXCEPTIONS

bool
Config::EnableDowngrade::get()
{
	if (_Config::_EnableDowngrade::_first) {

		Os::AutoCriticalSection	latch(_Config::Latch);

		if (_Config::_EnableDowngrade::_first) {
			bool	value;
			if (_Config::get(Config::EnableDowngrade::Name,
							 Common::Parameter::TypeBoolean,
							 &value)
				== false) {
				value = Config::EnableDowngrade::Default;
			}
			_Config::_EnableDowngrade::_value = value;
			_Config::_EnableDowngrade::_first = false;
		}
	}

	return _Config::_EnableDowngrade::_value;
}

//	FUNCTION
//	Lock::Config::EnableDowngrade::reset --
//		記録しているパラメータ値を抹消する
//
//	NOTES
//	（システムパラメータから取得し）記録している
//	パラメータ値を抹消する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Config::EnableDowngrade::reset()
{
	Os::AutoCriticalSection	latch(_Config::Latch);

	_Config::_EnableDowngrade::_first = true;
}

//	FUNCTION
//	Lock::Config::LackForChildDetection::get --
//		パラメータ値を返す
//
//	NOTES
//	パラメータ値を返す。
//	まだパラメータ値を記録していない場合には、
//	システムパラメータから取得し、記録する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		パラメータ値
//
//	EXCEPTIONS

bool
Config::LackForChildDetection::get()
{
	if (_Config::_LackForChildDetection::_first) {

		Os::AutoCriticalSection	latch(_Config::Latch);

		if (_Config::_LackForChildDetection::_first) {
			bool	value;
			if (_Config::get(Config::LackForChildDetection::Name,
							 Common::Parameter::TypeBoolean,
							 &value)
				== false) {
				value = Config::LackForChildDetection::Default;
			}
			_Config::_LackForChildDetection::_value = value;
			_Config::_LackForChildDetection::_first = false;
		}
	}

	return _Config::_LackForChildDetection::_value;
}

//	FUNCTION
//	Lock::Config::LackForChildDetection::reset --
//		記録しているパラメータ値を抹消する
//
//	NOTES
//	（システムパラメータから取得し）記録している
//	パラメータ値を抹消する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Config::LackForChildDetection::reset()
{
	Os::AutoCriticalSection	latch(_Config::Latch);

	_Config::_LackForChildDetection::_first = true;
}

//	FUNCTION
//	Lock::Config::Timeout::set --
//		記録しているパラメータ値を変更する
//
//	NOTES
//	（システムパラメータから取得したりして）記録している
//	ロック待ち時間のパラメータ値を変更する。
//
//	ARGUMENTS
//	Lock::Timeout::Value	value
//		変更後のロック待ち時間のパラメータ値 [msec]
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Config::Timeout::set(Lock::Timeout::Value	value)
{
	Os::AutoCriticalSection	latch(_Config::Latch);

	_Config::_Timeout::_first = false;
	_Config::_Timeout::_value = value;
}

//	FUNCTION
//	Lock::Config::Timeout::get --
//		パラメータ値を返す
//
//	NOTES
//	ロック待ち時間のパラメータ値を返す。
//	まだパラメータ値を記録していない場合には、
//	システムパラメータから取得し、記録する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Lock::Timeout::Value
//		ロック待ち時間のパラメータ値 [msec]
//
//	EXCEPTIONS

Lock::Timeout::Value
Config::Timeout::get()
{
	if (_Config::_Timeout::_first) {

		Os::AutoCriticalSection	latch(_Config::Latch);

		if (_Config::_Timeout::_first) {
			int	value;
			if (_Config::get(Config::Timeout::Name,
							 Common::Parameter::TypeInteger,
							 &value)
				== false) {
				value = Config::Timeout::Default;
			}
			_Config::_Timeout::_value = static_cast<Lock::Timeout::Value>(value);
			_Config::_Timeout::_first = false;
		}
	}

	return _Config::_Timeout::_value;
}

//	FUNCTION
//	Lock::Config::Timeout::reset --
//		記録しているパラメータ値を抹消する
//
//	NOTES
//	（システムパラメータから取得したりして）記録している
//	ロック待ち時間のパラメータ値を抹消する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Config::Timeout::reset()
{
	Os::AutoCriticalSection	latch(_Config::Latch);

	_Config::_Timeout::_first = true;
}

//	FUNCTION
//	Lock::Config::HashSize::get --
//		パラメータ値を返す
//
//	NOTES
//	ロック項目のハッシュテーブルの要素数の
//	パラメータ値を返す。
//	まだパラメータ値を記録していない場合には、
//	システムパラメータから取得し、記録する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Lock::Config::HashSize::Value
//		ロック項目のハッシュテーブルの
//		要素数のパラメータ値
//
//	EXCEPTIONS

Config::HashSize::Value
Config::HashSize::get()
{
	if (_Config::_HashSize::_first) {

		Os::AutoCriticalSection	latch(_Config::Latch);

		if (_Config::_HashSize::_first) {
			int	value;
			if (_Config::get(Config::HashSize::Name,
							 Common::Parameter::TypeInteger,
							 &value)
				== false) {
				value = Config::HashSize::Default;
			}
			_Config::_HashSize::_value =
				static_cast<Config::HashSize::Value>(value);
			_Config::_HashSize::_first = false;
		}
	}

	return _Config::_HashSize::_value;
}

//	FUNCTION
//	Lock::Config::HashSize::reset --
//		記録しているパラメータ値を抹消する
//
//	NOTES
//	記録しているロック項目のハッシュテーブルの
//	要素数のパラメータ値を抹消する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Config::HashSize::reset()
{
	Os::AutoCriticalSection	latch(_Config::Latch);

	_Config::_HashSize::_first = true;
}

//	FUNCTION
//	Lock::Config::ItemInstanceCacheSize::get --
//		パラメータ値を返す
//
//	NOTES
//	まだパラメータ値を記録していない場合には、
//	システムパラメータから取得し、記録する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Lock::Config::ItemInstanceCacheSize::Value
//		パラメータ値
//
//	EXCEPTIONS

Config::ItemInstanceCacheSize::Value
Config::ItemInstanceCacheSize::get()
{
	if (_Config::_ItemInstanceCacheSize::_first) {
	
		Os::AutoCriticalSection	latch(_Config::Latch);

		if (_Config::_ItemInstanceCacheSize::_first) {
			int	value;
			if (_Config::get(Config::ItemInstanceCacheSize::Name,
							 Common::Parameter::TypeInteger,
							 &value)
				== false) {
				value = Config::ItemInstanceCacheSize::Default;
			}
			_Config::_ItemInstanceCacheSize::_value =
				static_cast<Config::ItemInstanceCacheSize::Value>(value);
			_Config::_ItemInstanceCacheSize::_first = false;
		}
	}

	return _Config::_ItemInstanceCacheSize::_value;
}

//	FUNCTION
//	Lock::Config::ItemInstanceCacheSize::reset --
//		記録しているパラメータ値を抹消する
//
//	NOTES
//	パラメータ値を抹消する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Config::ItemInstanceCacheSize::reset()
{
	Os::AutoCriticalSection	latch(_Config::Latch);

	_Config::_ItemInstanceCacheSize::_first = true;
}

//	FUNCTION
//	Lock::Config::RequestInstanceCacheSize::get --
//		パラメータ値を返す
//
//	NOTES
//	まだパラメータ値を記録していない場合には、
//	システムパラメータから取得し、記録する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Lock::Config::RequestInstanceCacheSize::Value
//		パラメータ値
//
//	EXCEPTIONS

Config::RequestInstanceCacheSize::Value
Config::RequestInstanceCacheSize::get()
{
	if (_Config::_RequestInstanceCacheSize::_first) {
	
		Os::AutoCriticalSection	latch(_Config::Latch);

		if (_Config::_RequestInstanceCacheSize::_first) {
			int	value;
			if (_Config::get(Config::RequestInstanceCacheSize::Name,
							 Common::Parameter::TypeInteger,
							 &value)
				== false) {
				value = Config::RequestInstanceCacheSize::Default;
			}
			_Config::_RequestInstanceCacheSize::_value =
				static_cast<Config::RequestInstanceCacheSize::Value>(value);
			_Config::_RequestInstanceCacheSize::_first = false;
		}
	}

	return _Config::_RequestInstanceCacheSize::_value;
}

//	FUNCTION
//	Lock::Config::RequestInstanceCacheSize::reset --
//		記録しているパラメータ値を抹消する
//
//	NOTES
//	パラメータ値を抹消する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Config::RequestInstanceCacheSize::reset()
{
	Os::AutoCriticalSection	latch(_Config::Latch);

	_Config::_RequestInstanceCacheSize::_first = true;
}

//
//	Copyright (c) 2000, 2002, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
