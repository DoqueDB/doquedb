// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parameter.cpp -- パラメータを表すクラス
// 
// Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "SydTest";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SydTest/Parameter.h"
#include "ModCriticalSection.h"
#include "ModAutoMutex.h"

_SYDNEY_USING
using namespace SydTest;

namespace {
	// パラメータ要素表のアイテム
	struct ParameterItem
	{
		const char* m_pszParameter;
		Parameter::ParamType::Value m_eType;
	};

	ParameterItem _pParameterItem[] = {
		{"binaryfile",			Parameter::ParamType::BinaryFile},
		{"double",				Parameter::ParamType::Double},
		{"date",				Parameter::ParamType::Date},
		{"float",				Parameter::ParamType::Float},
		{"integer",				Parameter::ParamType::Integer},
		{"integer64",			Parameter::ParamType::Integer64},
		{"null",				Parameter::ParamType::NullData},
		{"string",				Parameter::ParamType::Char},
		{"textfile",			Parameter::ParamType::TextSFile},
		{"textsjisfile",		Parameter::ParamType::TextSFile},
		{"textucsfile",			Parameter::ParamType::TextUFile},
		{"time",				Parameter::ParamType::Time},
		{"unsignedinteger",		Parameter::ParamType::UnsignedInteger},
		{"unsignedinteger64",	Parameter::ParamType::UnsignedInteger64},
		{0,						Parameter::ParamType::Other}
	};
}

//static変数はここに宣言が必要

std::map<std::string, Parameter::ParamType::Value> 
Parameter::m_mapParameterType;

ModCriticalSection
Parameter::m_cCriticalSection;

//
//	FUNCTION public
//	SydTest::Parameter::Parameter -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Common::DataArrayData* pArrayData_
//		パラメータの中身を表す配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Parameter::Parameter(Common::DataArrayData* pArrayData_)
: Item(Type::Parameter), m_pArrayData(pArrayData_)
{
}

//
//	FUNCTION public
//	SydTest::Parameter::~Parameter -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Parameter::~Parameter()
{
	if (m_pArrayData != 0)
	{
		delete m_pArrayData, m_pArrayData=0;
	}
}

//
//	FUNCTION public
//	SydTest::Parameter::getParameter -- パラメータ値を得る
//
//	NOTES
//	パラメータの配列値を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::DataArrayData
//		得られた配列
//
//	EXCEPTIONS
//	なし
//
Common::DataArrayData*
Parameter::getParameter() const
{
	return m_pArrayData;
}

// (static)
Parameter::ParamType
Parameter::getParameterType(const char* pszString_)
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCriticalSection);
	cAuto.lock();

	if (m_mapParameterType.size() == 0)
	{
		// マップにはまだ要素が入っていない
		ParameterItem* pItem = _pParameterItem;
		while (pItem->m_pszParameter != 0)
		{
			m_mapParameterType.insert
				(std::map<std::string, ParamType::Value>::
				 value_type(pItem->m_pszParameter, pItem->m_eType));
			pItem++;
		}
	}
	
	// マップの検索
	std::map<std::string, ParamType::Value>::iterator
		i = m_mapParameterType.find(pszString_);
	if (i == m_mapParameterType.end())
		return ParamType::Other;
	else 
		return (*i).second;
}

//
//	Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
