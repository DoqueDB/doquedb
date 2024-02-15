// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Configuration.h -- SystemParameterの便利ラッパー
// 
// Copyright (c) 2009, 2017, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_CONFIGURATION_H
#define __TRMEISTER_COMMON_CONFIGURATION_H

#include "Common/Module.h"
#include "Common/Internal.h"
#include "Common/SystemParameter.h"

#include "Os/CriticalSection.h"
#include "Os/AutoCriticalSection.h"

#include "ModCharString.h"
#include "ModUnicodeString.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

namespace Configuration
{
	// CLASS
	//	Common::Configuration::Getter -- 通常のgetter
	//
	// NOTES
	class Getter
	{
	public:
		//コンストラクタ
		Getter() {}
		~Getter() {}

		// 取得関数
		template <class Type_>
		bool operator()(const ModCharString& cstrParameterName_,
						Type_& cValue_) const
		{
			return Common::SystemParameter::getValue(cstrParameterName_,
													 cValue_);
		}
	};

	// TEMPLATE CLASS
	//	Common::Configuration::RangeGetter -- 上限下限つきのgetter
	//
	// TEMPLATE ARGUMENTS
	//	class Type_
	//		ターゲットとなるパラメーターの型(数値型である必要がある)
	//
	// NOTES
	template <class Type_>
	class RangeGetter
	{
	public:
		//コンストラクタ
		RangeGetter()
			: m_cMin(-1), m_cMax(-1)
		{}
		RangeGetter(Type_ cMin_,
					Type_ cMax_)
			: m_cMin(cMin_), m_cMax(cMax_)
		{}
		~RangeGetter() {}

		// 取得関数
		bool operator()(const ModCharString& cstrParameterName_,
						Type_& cValue_) const
		{
			Type_ v;
			if (Common::SystemParameter::getValue(cstrParameterName_, v)) {
				cValue_ = (m_cMin < 0 || v >= m_cMin) ?
					((m_cMax < 0 || v <= m_cMax) ? v
					 : m_cMax)
					: m_cMin;
				return true;
			}
			return false;
		}
	protected:
	private:
		Type_ m_cMin;
		Type_ m_cMax;
	};

	//
	//	CLASS
	//	Common::Configuration::Base
	//		-- パラメータ値を取得するクラスの基底クラス
	//
	class Base
	{
	public:
		// 読み直す
		virtual void clear() = 0;
	};

	//
	//	TEMPLATE CLASS
	//	Common::Configuration::Parameter -- パラメータ値を取得するクラス
	//
	//	TEMPLATE ARGUMENT
	//	class Type_
	//		ターゲットとなるパラメーターの型
	//	class Getter_
	//		取得関数
	//
	//	NOTES
	//
	template <class Type_, class Getter_>
	class Parameter : public Base
	{
	public:
		//コンストラクタ
		Parameter(const char* pParameterName_, const Type_& cDefaultValue_,
				  bool bReload_ = true)
			: Base(),
			  m_cValue(cDefaultValue_),
			  m_cDefaultValue(cDefaultValue_),
			  m_bRead(false), m_bReload(bReload_),
			  m_cstrParameterName(pParameterName_),
			  m_cLatch()
		{}
		virtual ~Parameter()
		{
			// マップから削除する
			if (m_bReload)
				Common::SystemParameter::erase(this);
		}

		//値を取得する
		const Type_& get()
		{
			if (m_bRead == false)
			{
				//パラメータを取得する
				Os::AutoCriticalSection cAuto(m_cLatch);
				if (m_bRead == false)
				{
					// マップに登録する
					if (m_bReload)
						Common::SystemParameter::insert(this);
					
					if (getByGetter() == false)
					{
						//パラメータが設定されていないので、デフォルト値
						m_cValue = m_cDefaultValue;
					}
					m_bRead = true;
				}
			}

			return m_cValue;
		}

		//キャスト
		operator const Type_& ()
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
		//値を取得する(サブクラスがオーバーライドする)
		virtual bool getByGetter()
		{
			return getBy(Getter_());
		}
		//値を取得する(サブクラスが呼ぶ)
		bool getBy(const Getter_& cGetter_)
		{
			return cGetter_(m_cstrParameterName, m_cValue);
		}

		//パラメータ値
		Type_ m_cValue;
		//デフォルト値
		Type_ m_cDefaultValue;

	private:
		//システムパラメータを参照したか
		bool m_bRead;
		//読み直すか否か
		bool m_bReload;

		//パラメータ名
		ModCharString m_cstrParameterName;

		//排他制御用のクリティカルセクション
		Os::CriticalSection m_cLatch;
	};

	//
	//	TEMPLATE CLASS
	//	Common::Configuration::ParameterInRange
	//		-- パラメータ値を取得するクラス(上限下限つき)
	//
	//	TEMPLATE ARGUMENT
	//	class Type_
	//		ターゲットとなるパラメーターの型
	//	class Getter_
	//		取得関数
	//
	//	NOTES
	//
	template <class Type_, class Getter_>
	class ParameterInRange
		: public Parameter<Type_, Getter_>
	{
	public:
		typedef Parameter<Type_, Getter_> Super;
		//コンストラクタ
		ParameterInRange(const char* pParameterName_,
						 const Type_& cDefaultValue_,
						 Type_ cMin_ = -1, Type_ cMax_ = -1,
						 bool bReload_ = true)
			: Super(pParameterName_, cDefaultValue_, bReload_),
			  m_cMin(cMin_),
			  m_cMax(cMax_)
		{}
		~ParameterInRange() {}

	protected:
		//値を取得する
		virtual bool getByGetter()
		{
			return getBy(Getter_(m_cMin, m_cMax));
		}
	private:
		Type_ m_cMin;
		Type_ m_cMax;
	};

	//
	//	CLASS
	//	Common::Configuration::ParameterInteger -- Integer用
	//
	//	NOTES
	//
	typedef Parameter<int, Getter> ParameterInteger;

	//
	//	CLASS
	//	Common::Configuration::ParameterIntegerInRange -- Integer用(範囲つき)
	//
	//	NOTES
	//
	typedef ParameterInRange<int, RangeGetter<int> > ParameterIntegerInRange;

	//
	//	CLASS
	//	Common::Configuration::ParameterUnsignedInteger -- unsigned integer用
	//
	//	NOTES
	//
	typedef Parameter<unsigned int, Getter> ParameterUnsignedInteger;

	//
	//	CLASS
	//	Common::Configuration::ParameterUnsignedIntegerInRange
	//		-- unsigned integer用(範囲つき)
	//
	//	NOTES
	//
	typedef ParameterInRange<unsigned int, RangeGetter<unsigned int> >
	ParameterUnsignedIntegerInRange;

	//
	//	CLASS
	//	Common::Configuration::ParameterInteger64 -- Integer64用
	//
	//	NOTES
	//
	typedef Parameter<ModInt64, Getter> ParameterInteger64;

	//
	//	CLASS
	//	Common::Configuration::ParameterInteger64InRange
	//		-- Integer64用(範囲つき)
	//
	//	NOTES
	//
	typedef ParameterInRange<ModInt64, RangeGetter<ModInt64> >
	ParameterInteger64InRange;

	//
	//	CLASS
	//	Common::Configuration::ParameterUnsignedInteger64
	//		-- unsigned integer64用
	//
	//	NOTES
	//
	typedef Parameter<ModUInt64, Getter> ParameterUnsignedInteger64;

	//
	//	CLASS
	//	Common::Configuration::ParameterUnsignedInteger64InRange
	//		-- unsigden integer64用(範囲つき)
	//
	//	NOTES
	//
	typedef ParameterInRange<ModUInt64, RangeGetter<ModUInt64> >
	ParameterUnsignedInteger64InRange;

	//
	//	TYPEDEF
	//	Common::Configuration::ParameterBoolean -- Boolean用
	//
	typedef Parameter<bool, Getter> ParameterBoolean;

	//
	//	TYPDEF
	//	Common::Configuration::ParameterString -- String用
	//
	typedef Parameter<ModUnicodeString, Getter> ParameterString;

	//
	//	CLASS
	//	Common::Configuration::ParameterMessage -- Message用
	//
	class ParameterMessage : public ParameterString
	{
	public:
		//コンストラクタ
		ParameterMessage(const char* pParameterName_, bool bReload_ = true)
			: ParameterString(pParameterName_, ModUnicodeString(), bReload_) {}

 		//出力するか
		bool isOutput()
		{
			const ModUnicodeString& cstrFileName = get();
			if (cstrFileName.getLength() != 0 && cstrFileName[0] != '0')
				return true;
			return false;
		}
	};

} // namespace Configuration

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif //__TRMEISTER_COMMON_CONFIGURATION_H

//
//	Copyright (c) 2009, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
