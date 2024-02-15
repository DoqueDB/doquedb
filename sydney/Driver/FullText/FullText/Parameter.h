// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parameter.h --
// 
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT_PARAMETER_H
#define __SYDNEY_FULLTEXT_PARAMETER_H

#include "FullText/Module.h"
#include "Common/SystemParameter.h"
#include "Os/CriticalSection.h"
#include "Os/AutoCriticalSection.h"

#include "ModCharString.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

//
//	TEMPLATE CLASS
//	FullText::Parameter
//
//	NOTES
//	パラメータ値を取得するラッパークラス
//
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
	virtual const TYPE& get()
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

	//パラメータ名を得る
	const ModCharString& getParameterName() const
	{
		return m_cstrParameterName;
	}

protected:
	//パラメータ値
	TYPE m_cValue;
	//デフォルト値
	TYPE m_cDefaultValue;

	//システムパラメータを参照したか
	bool m_bRead;

private:
	//パラメータ名
	ModCharString m_cstrParameterName;

	//排他制御用のクリティカルセクション
	Os::CriticalSection m_cLatch;
};

//
//	CLASS
//	FullText::ParameterInteger -- Integer用
//
//	NOTES
//
class ParameterInteger : public Parameter<int>
{
public:
	//コンストラクタ(1)
	ParameterInteger(const char* pParameterName_, int iDefaultValue_)
	: Parameter<int>(pParameterName_, iDefaultValue_),
	  m_iMinimumValue(-1), m_iMaximumValue(-1)
	{
	}

	//コンストラクタ(2)
	ParameterInteger(const char* pParameterName_, int iDefaultValue_,
						int iMinimumValue_, int iMaximumValue_)
	: Parameter<int>(pParameterName_, iDefaultValue_),
	  m_iMinimumValue(iMinimumValue_), m_iMaximumValue(iMaximumValue_)
	{
	}

	//値を取得する
	const int& get()
	{
		if (m_bRead == false)
		{
			int v = Parameter<int>::get();
			if (m_iMinimumValue != -1)
			{
				//上限と下限が与えられているので、
				//範囲内かチェックし、範囲外の場合は、
				//デフォルト値を設定する
				if (m_iMinimumValue > v || m_iMaximumValue < v)
				{
					//範囲外
					m_cValue = m_cDefaultValue;
				}
			}
		}

		return Parameter<int>::get();
	}

private:
	//最小値
	int m_iMinimumValue;
	//最大値
	int m_iMaximumValue;
};

//
//	TYPEDEF
//	FullText::ParameterBoolean -- Boolean用
//
typedef Parameter<bool> ParameterBoolean;

//
//	TYPDEF
//	FullText::ParameterString -- String用
//
typedef Parameter<ModUnicodeString> ParameterString;

//
//	CLASS
//	FullText::ParameterMessage -- Message用
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

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_PARAMETER_H

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
