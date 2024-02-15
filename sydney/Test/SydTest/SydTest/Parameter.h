// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parameter.h -- パラメータクラス
// 
// Copyright (c) 2000, 2001, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SYDTEST_PARAMETER_H
#define __SYDNEY_SYDTEST_PARAMETER_H

#include "Common/Common.h"
#include "Common/DataArrayData.h"
#include "Common/Internal.h"
#include "ModCriticalSection.h"
#include "SydTest/Item.h"
#include <map>
#include <string>

_SYDNEY_BEGIN

namespace SydTest
{

//
//	CLASS
//	SydTest::Parameter --
//
//	NOTES
//
//
//##ModelId=3A9B47470024
class Parameter : public Item
{
public:

	// パラメータの型タイプ
	struct ParamType
	{
		enum Value
		{
			Other,
			Input,
			Float,
			Integer,
			Integer64,
			UnsignedInteger,
			UnsignedInteger64,
			Char,
			Double,
			BinaryFile,
			TextSFile,
			TextUFile,
			Date,
			Time,
			NullData
		};
		
		operator Value()
		{
			return m_eType;
		}

		ParamType& operator=(Value eType_)
		{
			m_eType = eType_;
			return *this;
		}

		ParamType::ParamType(Value eType)
			: m_eType(eType) {}

		Value m_eType;
	};

	//コンストラクタ
	//##ModelId=3A9B47470039
	explicit Parameter(Common::DataArrayData* pArrayData_);
	//デストラクタ
	//##ModelId=3A9B47470038
	virtual ~Parameter();
	// パラメータ値を得る
	//##ModelId=3A9B47470034
	Common::DataArrayData* getParameter() const;

	// コマンドタイプを得る
	//##ModelId=3A9B47460367
	static ParamType getParameterType(const char* pszString_);

private:
	// パラメータ値(配列)
	//##ModelId=3A9B47470031
	Common::DataArrayData* m_pArrayData;

	// 型名を表す文字列からパラメータの型を返す
	static std::map<std::string, ParamType::Value> m_mapParameterType;
	//
	static ModCriticalSection m_cCriticalSection;
};

}

_SYDNEY_END

#endif //__SYDNEY_SYDTEST_PARAMETER_H

//
//	Copyright (c) 2000, 2001, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
