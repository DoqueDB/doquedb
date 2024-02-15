// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Compare.cpp --
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Btree2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Btree2/Compare.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

_SYDNEY_USING
_SYDNEY_BTREE2_USING

//
//	FUNCTION public
//	Btree2::Compare::Compare -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Compare::Compare()
	: m_bUnique(false), m_bHeader(false)
{
}

//
//	FUNCTION public
//	Btree2::Compare::~Compare -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Compare::~Compare()
{
}

//
//	FUNCTION public
//	Btree2::Compare::setType -- 型を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<Data::Type::Value>& vecType_
//		型の配列
//	bool bUnique_
//		この比較でエントリがユニークになるかどうか
//	bool bHeader_
//		エントリヘッダーがあるか否か
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Compare::setType(const ModVector<Data::Type::Value>& vecType_,
				 bool bUnique_, bool bHeader_)
{
	m_vecType = vecType_;
	m_bUnique = bUnique_;
	m_bHeader = bHeader_;
	m_uiUsingIntegrityCheckField = vecType_.getSize();
}

//
//	FUNCTION public
//	Btree2::Compare::operator () -- 比較する
//
//	NOTES
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
	; _SYDNEY_ASSERT(m_vecType.getSize() > 0);
	
	int result;

	if (m_bHeader)
	{
		// エントリヘッダーがある
		const Data::Header* h1
			= syd_reinterpret_cast<const Data::Header*>(p1);
		p1 += h1->getSize();
		const Data::Header* h2
			= syd_reinterpret_cast<const Data::Header*>(p2);
		p2 += h2->getSize();

		ModVector<Data::Type::Value>::ConstIterator i = m_vecType.begin();
		int n = 0;
		for (; i != m_vecType.end(); ++i, ++n)
		{
			if ((result = compare(p1, h1->isNull(n),
								  p2, h2->isNull(n), *i)) != 0)
				break;
		}
	}
	else
	{
		ModVector<Data::Type::Value>::ConstIterator i = m_vecType.begin();
		for (; i != m_vecType.end(); ++i)
		{
			if ((result = compare(p1, p2, *i)) != 0)
				break;
		}
	}
	
	return result;
}

//
//	FUNCTION public
//	Btree2::Compare::operator () -- 比較する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* p1
//		比較対象1
//	unsigned char nullBitmap1_
//		p1のnullビットマップ
//	const ModUInt32* p2
//		比較対象2
//	unsigned char nullBitmap2_
//		p2のnullビットマップ
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
Compare::operator () (const ModUInt32* p1, unsigned char nullBitmap1_,
					  const ModUInt32* p2, unsigned char nullBitmap2_) const
{
	; _SYDNEY_ASSERT(m_vecType.getSize() > 0);
	; _SYDNEY_ASSERT(m_vecType.getSize() <= 8);
	
	int result;
	unsigned char bit = 1;
	ModVector<Data::Type::Value>::ConstIterator i = m_vecType.begin();
	for (; i != m_vecType.end(); ++i)
	{
		if ((result = compare(p1, (nullBitmap1_ & bit) ? true : false,
							  p2, (nullBitmap2_ & bit) ? true : false,
							  *i)) != 0)
			break;
		bit <<= 1;
	}
	return result;
}

//
//	FUNCTION public
//	Btree2::Compare::integrityCheck -- インテグリティーチェック
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* p1
//		比較対象1
//	const ModUInt32* p2
//		比較対象2
//
//	RETURN
//	bool
//		インテグリティーに違反していない場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Compare::integrityCheck(const ModUInt32* p1, const ModUInt32* p2) const
{
	; _SYDNEY_ASSERT(m_vecType.getSize() > 0);
	; _SYDNEY_ASSERT(m_vecType.getSize() <= 8);
	
	int result = 0;

	if (m_bHeader)
	{
		// エントリヘッダーがある
		const Data::Header* h1
			= syd_reinterpret_cast<const Data::Header*>(p1);
		p1 += h1->getSize();
		const Data::Header* h2
			= syd_reinterpret_cast<const Data::Header*>(p2);
		p2 += h2->getSize();

		ModSize n = 0;
		ModVector<Data::Type::Value>::ConstIterator i = m_vecType.begin();
		for (; n < m_uiUsingIntegrityCheckField; ++n, ++i)
		{
			if (h1->isNull(n) || h2->isNull(n))
				// nullが含まれていたらOK
				return true;
			if ((result = compare(p1, p2, *i)) != 0)
				break;
		}
	}
	else
	{
		ModSize n = 0;
		ModVector<Data::Type::Value>::ConstIterator i = m_vecType.begin();
		for (; n < m_uiUsingIntegrityCheckField; ++n, ++i)
		{
			if ((result = compare(p1, p2, *i)) != 0)
				break;
		}
	}
	
	return (result == 0) ? false : true;
}

//
//	FUNCTION public
//	Btree2::Compare::integrityCheck -- インテグリティーチェック
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* p1
//		比較対象1
//	unsigned char nullBitmap1_
//		p1のnullビットマップ
//	const ModUInt32* p2
//		比較対象2
//	unsigned char nullBitmap2_
//		p2のnullビットマップ
//
//	RETURN
//	bool
//		インテグリティーに違反していない場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Compare::integrityCheck(const ModUInt32* p1, unsigned char nullBitmap1_,
						const ModUInt32* p2, unsigned char nullBitmap2_) const
{
	; _SYDNEY_ASSERT(m_vecType.getSize() > 0);
	; _SYDNEY_ASSERT(m_vecType.getSize() <= 8);
	
	int result = 0;
	unsigned char bit = 1;
	ModVector<Data::Type::Value>::ConstIterator i = m_vecType.begin();
	for (ModSize n = 0; n < m_uiUsingIntegrityCheckField; ++n, ++i)
	{
		if ((nullBitmap1_ & bit) || (nullBitmap2_ & bit))
			// nullが含まれていたらOK
			return true;
		if ((result = compare(p1, false, p2, false, *i)) != 0)
			break;
		bit <<= 1;
	}
	return (result == 0) ? false : true;
}

//
//	FUNCTION public
//	Btree2::Compare::compare -- 1つ比較する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* p1
//		比較対象1
//	const ModUInt32* p2
//		比較対象2
//	Btree2::Data::Type::Value eType_
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
int
Compare::compare(const ModUInt32*& p1,
				 const ModUInt32*& p2, Data::Type::Value eType_) const
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
#ifdef OBSOLETE
	case Data::Type::Float:
		_COMPARE1(Data::Float);
		break;
#endif
	case Data::Type::Double:
		_COMPARE1(Data::Double);
		break;
	case Data::Type::Decimal:
		_COMPARE1(Data::Decimal);
		break;
	case Data::Type::CharString:
		_COMPARE1(Data::CharString);
		break;
	case Data::Type::NoPadCharString:
		_COMPARE1(Data::NoPadCharString);
		break;
	case Data::Type::UnicodeString:
		_COMPARE1(Data::UnicodeString);
		break;
	case Data::Type::NoPadUnicodeString:
		_COMPARE1(Data::NoPadUnicodeString);
		break;
#ifdef OBSOLETE
	case Data::Type::Date:
		_COMPARE1(Data::Date);
		break;
#endif
	case Data::Type::DateTime:
		_COMPARE1(Data::DateTime);
		break;
#ifdef OBSOLETE
	case Data::Type::IntegerArray:
		_COMPARE1(Data::IntegerArray);
		break;
	case Data::Type::UnsignedIntegerArray:
		_COMPARE1(Data::UnsignedIntegerArray);
		break;
#endif
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
//	Btree2::Compare::compare -- 1つ比較する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* p1
//		比較対象1
//	bool isNull1_
//		p1がnullかどうか
//	const ModUInt32* p2
//		比較対象2
//	bool isNull2_
//		p2がnullかどうか
//	Btree2::Data::Type::Value eType_
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
int
Compare::compare(const ModUInt32*& p1, bool isNull1_,
				 const ModUInt32*& p2, bool isNull2_,
				 Data::Type::Value eType_) const
{

//
//	DEFINE
//	_COMPARE2 -- nullがある要素の比較
//
#define _COMPARE2(TYPE) \
{ \
	if (isNull1_ || isNull2_) \
	{ \
		result = isNull1_ ? (isNull2_ ? 0 : -1) : (isNull2_ ? 1 : 0); \
		if (isNull1_ == false) \
			p1 += TYPE::getSize(p1); \
		if (isNull2_ == false) \
			p2 += TYPE::getSize(p2); \
	} \
	else \
	{ \
		const TYPE* d1 = syd_reinterpret_cast<const TYPE*>(p1); \
		const TYPE* d2 = syd_reinterpret_cast<const TYPE*>(p2); \
		p1 += d1->getSize(); \
		p2 += d2->getSize(); \
		result = d1->compare(*d2);\
	}\
}

	int result;
	
	switch (eType_)
	{
	case Data::Type::Integer:
		_COMPARE2(Data::Integer);
		break;
	case Data::Type::UnsignedInteger:
		_COMPARE2(Data::UnsignedInteger);
		break;
#ifdef OBSOLETE
	case Data::Type::Float:
		_COMPARE2(Data::Float);
		break;
#endif
	case Data::Type::Double:
		_COMPARE2(Data::Double);
		break;
	case Data::Type::Decimal:
		_COMPARE2(Data::Decimal);
		break;
	case Data::Type::CharString:
		_COMPARE2(Data::CharString);
		break;
	case Data::Type::NoPadCharString:
		_COMPARE2(Data::NoPadCharString);
		break;
	case Data::Type::UnicodeString:
		_COMPARE2(Data::UnicodeString);
		break;
	case Data::Type::NoPadUnicodeString:
		_COMPARE2(Data::NoPadUnicodeString);
		break;
#ifdef OBSOLETE
	case Data::Type::Date:
		_COMPARE2(Data::Date);
		break;
#endif
	case Data::Type::DateTime:
		_COMPARE2(Data::DateTime);
		break;
#ifdef OBSOLETE
	case Data::Type::IntegerArray:
		_COMPARE2(Data::IntegerArray);
		break;
	case Data::Type::UnsignedIntegerArray:
		_COMPARE2(Data::UnsignedIntegerArray);
		break;
#endif
	case Data::Type::ObjectID:
		_COMPARE2(Data::ObjectID);
		break;
	case Data::Type::LanguageSet:
		_COMPARE2(Data::LanguageSet);
		break;
	case Data::Type::Integer64:
		_COMPARE2(Data::Integer64);
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);
	}

#undef _COMPARE2

	return result;
}

//
//	FUNCTION public
//	Btree2::Compare::like -- 1つ比較する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* p1
//		比較対象1
//	const ModUInt32* p2
//		比較対象2
//	Btree2::Data::Type::Value eType_
//		データ型
//	ModUnicodeChar escape_
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
Compare::like(const ModUInt32*& p1,
			  const ModUInt32*& p2, Data::Type::Value eType_,
			  ModUnicodeChar escape_) const
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
	
	bool result = false;
	
	// LIKE is always comapred with NO PAD.
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
		_SYDNEY_THROW0(Exception::NotSupported);
	}

#undef _LIKE

	return result;
}

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2007, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
