// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parameter.h --
// 
// Copyright (c) 2002, 2003, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_PARAMETER_H
#define __SYDNEY_INVERTED_PARAMETER_H

#include "Inverted/Module.h"
#include "Common/SystemParameter.h"
#include "Os/CriticalSection.h"
#include "Os/AutoCriticalSection.h"

#include "ModCharString.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

//
//	TEMPLATE CLASS
//	_$$::Parameter
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
//	Server::ParameterInteger -- Integer用
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
				//上限と下限が与えられているので、範囲内かチェックし、範囲外の場合は、
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
//	Server::ParameterBoolean -- Boolean用
//
typedef Parameter<bool> ParameterBoolean;

//
//	TYPDEF
//	Server::ParameterString -- String用
//
typedef Parameter<ModUnicodeString> ParameterString;

//
//	CLASS
//	Server::ParameterMessage -- Message用
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

//
//	CLASS
//	Server::ParameterInteger64 -- ModInt64用(ただし正の数のみ)
//
//	NOTES
//	ModInt64用は文字列でパラメータファイルに格納する。
//	数字の末尾にK, M, G, Tが利用できる。
//
class ParameterInteger64
{
public:
	//コンストラクタ(1)
	ParameterInteger64(const char* pParameterName_,
					   const ModUnicodeString& cDefaultValue_)
		: m_cParameter(pParameterName_, ModUnicodeString()),
		  m_lValue(-1), m_lMinimumValue(-1), m_lMaximumValue(-1)
	{
		m_lDefaultValue = toInt(cDefaultValue_);
	}
	// コンストラクタ(2)
	ParameterInteger64(const char* pParameterName_,
					   const ModUnicodeString& cDefaultValue_,
					   const ModUnicodeString& cMinValue_,
					   const ModUnicodeString& cMaxValue_)
		: m_cParameter(pParameterName_, ModUnicodeString()),
		  m_lValue(-1)
	{
		m_lDefaultValue = toInt(cDefaultValue_);
		m_lMinimumValue = toInt(cMinValue_);
		m_lMaximumValue = toInt(cMaxValue_);
	}
	//コンストラクタ(3)
	ParameterInteger64(const char* pParameterName_,
					   ModInt64 defaultValue_)
		: m_cParameter(pParameterName_, ModUnicodeString()),
		  m_lValue(-1), m_lDefaultValue(defaultValue_),
		  m_lMinimumValue(-1), m_lMaximumValue(-1)
	{
	}
	//コンストラクタ(4)
	ParameterInteger64(const char* pParameterName_,
					   ModInt64 defaultValue_,
					   ModInt64 minValue_,
					   ModInt64 maxValue_)
		: m_cParameter(pParameterName_, ModUnicodeString()),
		  m_lValue(-1), m_lDefaultValue(defaultValue_),
		  m_lMinimumValue(minValue_), m_lMaximumValue(maxValue_)
	{
	}

	//値を取得する
	ModInt64 get()
	{
		if (m_lValue == -1)
		{
			//パラメータを取得する
			Os::AutoCriticalSection cAuto(m_cLatch);

			const ModUnicodeString& v = m_cParameter.get();
			if (v.getLength() == 0)
				// パラメータが設定されていないので、デフォルト値を設定する
				m_lValue = m_lDefaultValue;
			else
			{
				m_lValue = toInt(v);
				if (m_lMinimumValue != -1)
				{
					//上限と下限が与えられているので、範囲内かチェックする
					if (m_lValue < m_lMinimumValue ||
						m_lValue > m_lMaximumValue)
						// 範囲外なので、デフォルト値を設定する
						m_lValue = m_lDefaultValue;
				}
			}
		}
		return m_lValue;
	}
	
	//キャスト
	operator ModInt64 ()
	{
		return get();
	}

	//値をクリアする
	void clear()
	{
		Os::AutoCriticalSection cAuto(m_cLatch);
		m_lValue = -1;
		m_cParameter.clear();
	}

	//パラメータ名を得る
	const ModCharString& getParameterName() const
	{
		return m_cParameter.getParameterName();
	}

private:
	// 文字列をModInt64に変換する(オーバーフローはチェックしない)
	ModInt64 toInt(const ModUnicodeChar* p)
	{
		ModInt64 i = 0;
		while (*p)
		{
			if (*p >= '0' && *p <= '9')
			{
				i *= 10;
				i += (*p - '0');
			}
			else if (*p == 'K' || *p == 'k')
				i = i << 10;
			else if (*p == 'M' || *p == 'm')
				i = i << 10 << 10;
			else if (*p == 'G' || *p == 'g')
				i = i << 10 << 10 << 10;
			else if (*p == 'T' || *p == 't')
				i = i << 10 << 10 << 10 << 10;
			else
				return -1;
			++p;
		}
		return i;
	}
	
	// 文字列パラメータ
	ParameterString m_cParameter;
	// 値
	ModInt64 m_lValue;
	// デフォルト値
	ModInt64 m_lDefaultValue;
	// 最小値
	ModInt64 m_lMinimumValue;
	// 最大値
	ModInt64 m_lMaximumValue;
	
	//排他制御用のクリティカルセクション
	Os::CriticalSection m_cLatch;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_PARAMETER_H

//
//	Copyright (c) 2002, 2003, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
