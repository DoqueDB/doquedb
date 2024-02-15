// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Compare.cpp --
// 
// Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Array";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Array/Compare.h"
#include "Array/Tree.h"

#include "Common/Assert.h"
#include "Common/StringData.h"

#include "Exception/NotSupported.h"

_SYDNEY_USING
_SYDNEY_ARRAY_USING

//
//	FUNCTION public
//	Array::Compare::Compare -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
Compare::Compare() : m_iFieldCount(0), m_bUnique(false)
{
}

//
//	FUNCTION public
//	Array::Compare::~Compare -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
Compare::~Compare()
{
}

//
//	FUNCTION public
//	Array::Compare::setTypeCount --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Compare::setTypeCount(int iFieldCount_)
{
	; _SYDNEY_ASSERT(iFieldCount_ >= 0 && iFieldCount_ <= MaxFieldCount);

	m_iFieldCount = iFieldCount_;
}

//
//	FUNCTION public
//	Array::Compare::setType --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Compare::setType(Data::Type::Value eFieldType_, int iFieldPosition_)
{
	; _SYDNEY_ASSERT(iFieldPosition_ >= 0
					 && iFieldPosition_ < m_iFieldCount);

	m_cFieldTypes[iFieldPosition_] = eFieldType_;
}

//
//	FUNCTION public
//	Array::Compare::operator () -- 比較する
//
//	NOTES
//	When m_iFieldCount == 0, return 0.
//
//	ARGUMENTS
//	const ModUInt32* p1
//		比較対象1
//	const ModUInt32* p2
//		比較対象2
//
//	RETURN
//	int
//		p1 < p2		-1
//		p1 == p2	0
//		p1 > p2		1
//
//	EXCEPTIONS
//
int
Compare::operator () (const ModUInt32* p1, const ModUInt32* p2) const
{
	; _SYDNEY_ASSERT(m_iFieldCount >= 0);

	int result = 0;
	for (int i = 0; i < m_iFieldCount; ++i)
	{
		if ((result = compare(p1, p2, m_cFieldTypes[i])) != 0)
			break;
	}
	return result;
}

//
//	FUNCTION public
//	Array::Compare::compare -- 1つ比較する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* p1
//		比較対象1
//	const ModUInt32* p2
//		比較対象2
//	Array::Data::Type::Value eType_
//		データ型
//
//	RETURN
//	int
//		p1 < p2		-1
//		p1 == p2	0
//		p1 > p2		1
//
//	EXCEPTIONS
//

// static
int
Compare::compare(const ModUInt32*& p1,
				 const ModUInt32*& p2, Data::Type::Value eType_)
{
//
//	DEFINE
//	_COMPARE1 -- nullがありえない要素の比較
//
#define _COMPARE1(TYPE) \
{ \
	const TYPE* d1 = syd_reinterpret_cast<const TYPE*>(p1); \
	const TYPE* d2 = syd_reinterpret_cast<const TYPE*>(p2); \
	p1 += d1->getSize(); \
	p2 += d2->getSize(); \
	result = d1->compare(*d2); \
}
	
	int result;
	
	switch (eType_)
	{
	case Data::Type::Integer:
		_COMPARE1(Data::Integer);
		break;
	case Data::Type::UnsignedInteger:
		_COMPARE1(Data::UnsignedInteger);
		break;
	case Data::Type::Double:
		_COMPARE1(Data::Double);
		break;
	case Data::Type::Decimal:
		_COMPARE1(Data::Decimal);
		break;
	case Data::Type::CharString:
		_COMPARE1(Data::CharString);
		break;
	case Data::Type::UnicodeString:
		_COMPARE1(Data::UnicodeString);
		break;
	case Data::Type::NoPadCharString:
		_COMPARE1(Data::NoPadCharString);
		break;
	case Data::Type::NoPadUnicodeString:
		_COMPARE1(Data::NoPadUnicodeString);
		break;
	case Data::Type::DateTime:
		_COMPARE1(Data::DateTime);
		break;
	case Data::Type::ObjectID:
		_COMPARE1(Data::ObjectID);
		break;
	case Data::Type::LanguageSet:
		_COMPARE1(Data::LanguageSet);
		break;
	case Data::Type::Integer64:
		_COMPARE1(Data::Integer64);
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}

#undef _COMPARE1

	return result;
}

//
//	FUNCTION public
//	Array::Compare::like -- Compare one data with like operator.
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* p1
//		比較対象1
//	const ModUInt32* p2
//		比較対象2
//	Array::Data::Type::Value eType_
//		データ型
//	ModUnicodeChar escape_
//		Escape character
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//

// static
bool
Compare::like(const ModUInt32*& p1, const ModUInt32*& p2,
			  Data::Type::Value eType_, ModUnicodeChar escape_)
{
//
//	DEFINE
//	_LIKE -- 
//
#define _LIKE(TYPE) \
{ \
	const TYPE* d1 = syd_reinterpret_cast<const TYPE*>(p1); \
	const TYPE* d2 = syd_reinterpret_cast<const TYPE*>(p2); \
	p1 += d1->getSize(); \
	p2 += d2->getSize(); \
	result = d1->like(*d2, escape_); \
}
	
	bool result;
	
	switch (eType_)
	{
	case Data::Type::CharString:
	case Data::Type::NoPadCharString:
		_LIKE(Data::NoPadCharString);
		break;
	case Data::Type::UnicodeString:
	case Data::Type::NoPadUnicodeString:
		_LIKE(Data::NoPadUnicodeString);
		break;
	default:
		result = (compare(p1, p2, eType_) == 0) ? true : false;
		break;
	}

#undef _LIKE

	return result;
}

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
