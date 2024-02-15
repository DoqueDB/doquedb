// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parameter.h -- パラメータ関連のクラス定義、関数宣言
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_CLIENT2_PARAMETER_H
#define __TRMEISTER_CLIENT2_PARAMETER_H

#include "Client2/Module.h"

#include "Common/SystemParameter.h"
#include "Os/CriticalSection.h"
#include "Os/AutoCriticalSection.h"

#include "ModCharString.h"

_TRMEISTER_BEGIN
_TRMEISTER_CLIENT2_BEGIN

//	TEMPLATE CLASS
//	_$$::Parameter -- パラメータ値を取得するラッパークラス
//
//	NOTES

template <class TYPE>
class Parameter
{
public:
	//コンストラクタ
	Parameter(const char* pParameterName_, const TYPE& cDefaultValue_)
	: m_bRead(false)
	{
		//パラメータ名をコピー
		m_cstrParameterName = pParameterName_;
		//デフォルト値をコピー
		m_cDefaultValue = cDefaultValue_;
	}

	//値を取得する
	const TYPE& get()
	{
		if (m_bRead == false)
		{
			//パラメータを取得する
			Os::AutoCriticalSection cAuto(m_cLatch);

			if (Common::SystemParameter::getValue(
					m_cstrParameterName, m_cValue) == false)
			{
				//パラメータが設定されていないので、デフォルト値
				m_cValue = m_cDefaultValue;
			}

			m_bRead = true;
		}

		return m_cValue;
	}

	//キャスト
	operator const TYPE& ()
	{
		return get();
	}

	//値をクリアする
	void clear()
	{
		m_bRead = false;
	}

private:
	//パラメータ名
	ModCharString m_cstrParameterName;
	//デフォルト値
	TYPE m_cDefaultValue;

	//システムパラメータを参照したか
	bool m_bRead;

	//パラメータ値
	TYPE m_cValue;

	//排他制御用のクリティカルセクション
	Os::CriticalSection m_cLatch;
};

//
//	TYPEDEF
//	Client2::ParameterInteger -- Integer用
//
typedef Parameter<int> ParameterInteger;

//
//	TYPEDEF
//	Client2::ParameterBoolean -- Boolean用
//
typedef Parameter<bool> ParameterBoolean;

//
//	TYPDEF
//	Client2::ParameterString -- String用
//
typedef Parameter<ModUnicodeString> ParameterString;

//
//	CLASS
//	Client2::ParameterMessage -- Message用
//
class ParameterMessage : public ParameterString
{
public:
	//コンストラクタ
	ParameterMessage(const char* pParameterName_)
		: ParameterString(pParameterName_, ModUnicodeString()) {}

	//出力するか
	bool isOutput()
	{
		const ModUnicodeString& cstrFileName = get();
		if (cstrFileName.getLength() != 0 && cstrFileName[0] != '0')
			return true;
		return false;
	}
};

_TRMEISTER_CLIENT2_END
_TRMEISTER_END

#endif //__TRMEISTER_CLIENT2_PARAMETER_H

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
