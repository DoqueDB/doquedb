// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Condition.cpp --
// 
// Copyright (c) 2005, 2006, 2007, 2008, 2009, 2011, 2023 Ricoh Company, Ltd.
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
	const char moduleName[] = "Bitmap";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Bitmap/Condition.h"
#include "Bitmap/QueryNode.h"

#include "Common/DataArrayData.h"
#include "Common/StringData.h"
#include "Common/NullData.h"
#ifdef OBSOLETE
#include "Common/DateData.h"
#endif
#include "Common/DateTimeData.h"
#include "Common/LanguageData.h"

#include "Exception/BadArgument.h"

#include "ModAutoPointer.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

namespace
{
	//
	//	FUNCTION local
	//	_$$::_null
	//
	ModUnicodeString _null = "(null)";

	//
	//	FUNCTION local
	//	_$$::_usPaddingChar
	//
	ModUnicodeChar _usPaddingChar = 0x20;
	
	//
	//	FUNCTION local
	//	_$$::_SOHChar
	//
	ModUnicodeChar _SOHChar = 0x01;
}

//
//	FUNCTION public
//	Bitmap::Condition::Condition -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Bitmap::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Condition::Condition(const FileID& cFileID_)
	: m_cFileID(cFileID_), m_eKeyType(cFileID_.getKeyType()),
	  m_bValid(true)
{
	m_cCompare.setType(m_eKeyType, false);
	m_cData.setType(m_eKeyType, 0);
	m_cLowerData.clear();
	m_cUpperData.clear();
}

//
//	FUNCTION public
//	Bitmap::Condition::~Condition -- デストラクタ
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
Condition::~Condition()
{
}

//
//	FUNCTION public
//	Bitmap::Condition::createQeuryNode -- 検索ノードを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& query_
//		検索条件
//	BitmapFile& cFile_
//		[YET]
//	bool bVerify_
//		[YET]
//
//	RETURN
//	QueryNode*
//		検索ノード
//
//	EXCEPTIONS
//
QueryNode*
Condition::createQueryNode(const ModUnicodeString& query_,
						   BitmapFile& cFile_,
						   bool bVerify_)
{
	const ModUnicodeChar* p = query_;

	// QueryNodeを得る
	ModAutoPointer<QueryNode> pNode = QueryNode::getQueryNode(p, cFile_, *this);
	// 有効化
	pNode->validate(bVerify_);

	return pNode.release();
}

//
//	FUNCTION public
//	Bitmap::Condition::isOtherConditionMatch
//		-- その他条件にマッチしているかどうか
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pBuffer_
//		比較するバッファ
//
//	RETURN
//	bool
//		マッチしている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Condition::isOtherConditionMatch(const ModUInt32* pBuffer_)
{
	bool result =  true;
	if (m_vecOtherCondition.getSize())
	{
		ModVector<Cond>::Iterator i = m_vecOtherCondition.begin();
		for (; i != m_vecOtherCondition.end(); ++i)
		{
			if ((*i).m_eType == Data::MatchType::EqualsToNull)
			{
				// nullのデータはないので
				result = false;
				break;
			}

			const ModUInt32* p1 = pBuffer_;
			const ModUInt32* p2 = (*i).m_pBuffer.get();
			
			if ((*i).m_eType == Data::MatchType::Like)
			{
				result = m_cCompare.like(p1, p2,
										 m_eKeyType, (*i).m_usOptionalChar);
			}
			else
			{
				Data::Type::Value eKeyType = getKeyType(
					m_eKeyType, (*i).m_usOptionalChar);
				int r = m_cCompare.compare(p1, p2, eKeyType);
		
				switch ((*i).m_eType)
				{
				case Data::MatchType::Equals:
					if (r != 0) result = false;
					break;
				case Data::MatchType::NotEquals:
					if (r == 0) result = false;
					break;
				case Data::MatchType::GreaterThan:
					if (r <= 0) result = false;
					break;
				case Data::MatchType::GreaterThanEquals:
					if (r < 0) result = false;
					break;
				case Data::MatchType::LessThan:
					if (r >= 0) result = false;
					break;
				case Data::MatchType::LessThanEquals:
					if (r > 0) result = false;
					break;
				}
			}
		
			if (result == false)
				break;
		}
	}
	return result;
}

//
//	FUNCTION public
//	Bitmap::Condition::setQueryString -- 検索文字列を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar*& p
//		検索条件文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Condition::setQueryString(const ModUnicodeChar*& p)
{
	m_cLowerData.clear();
	m_cUpperData.clear();
	m_vecOtherCondition.clear();
	m_bValid = true;
	
	// 下限条件を設定する
	setLowerData(p);
	// 上限条件を設定する
	setUpperData(p);
	// その他条件を設定する
	setOtherData(p);
}

//
//	FUNCTION private
//	Bitmap::Condition::setLowerData -- 下限条件を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar*& p_
//		検索条件文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Condition::setLowerData(const ModUnicodeChar*& p_)
{
	using namespace LogicalFile;

	if (*p_ == 0)
		return;
	const ModUnicodeChar* p = p_;
	p++;
	if (*p == 'e')
	{
		// #eq(....)
		p += 3;
		Common::Data::Pointer pData;
		Data::Type::Value eKeyType;
		
		while (*p != ')')
		{
			ModUnicodeString cstrValue;
			ModUnicodeChar usOptionalChar = 0;
			Data::MatchType::Value eType
				= OpenOption::ParseValue::getStream(
					p, cstrValue, usOptionalChar);
			if (eType == Data::MatchType::Unknown)
			{
				// 1件もヒットしない
				m_bValid = false;
			}
			else if (eType != Data::MatchType::EqualsToNull
				&& eType != Data::MatchType::EqualsToNull_All)
			{
				eKeyType = getKeyType(m_eKeyType, usOptionalChar);
				pData = createCommonData(eKeyType, cstrValue);
			}
			else
			{
				m_cLowerData.m_eType = eType;
			}
		}
		p++;

		if (pData.get())
		{
			// メモリーイメージを得る
			ModSize size = m_cData.getSize(*pData);
			m_cLowerData.m_pBuffer
				= syd_reinterpret_cast<ModUInt32*>(
					Os::Memory::allocate(size * sizeof(ModUInt32)));
			m_cData.dump(m_cLowerData.m_pBuffer, *pData);

			// 比較クラス
			m_cLowerData.m_cCompare.setType(eKeyType, true);
			m_cLowerData.m_eType = Data::MatchType::Equals;

			// 上限にも設定する
			m_cUpperData = m_cLowerData;
		}

		p_ = p;
	}
	else if (*p == 'g')
	{
		// #ge(....)
		bool isOnlyGT = true;
		
		p += 3;
		Common::Data::Pointer pData;
		Data::Type::Value eKeyType;
		
		while (*p != ')')
		{
			ModUnicodeString cstrValue;
			ModUnicodeChar usOptionalChar = 0;
			Data::MatchType::Value eType
				= OpenOption::ParseValue::getStream(
					p, cstrValue, usOptionalChar);
			if (eType != Data::MatchType::GreaterThan)
				isOnlyGT = false;
			eKeyType = getKeyType(m_eKeyType, usOptionalChar);
			pData = createCommonData(m_eKeyType, cstrValue);
		}
		p++;

		if (pData.get())
		{
			// メモリーイメージを得る
			ModSize size = m_cData.getSize(*pData);
			m_cLowerData.m_pBuffer
				= syd_reinterpret_cast<ModUInt32*>(
					Os::Memory::allocate(size * sizeof(ModUInt32)));
			m_cData.dump(m_cLowerData.m_pBuffer, *pData);

			// 比較クラス
			m_cLowerData.m_cCompare.setType(eKeyType, false);

			if (isOnlyGT == true)
				m_cLowerData.m_eType = Data::MatchType::GreaterThan;
			else
				m_cLowerData.m_eType = Data::MatchType::GreaterThanEquals;
		}

		p_ = p;
	}
}

//
//	FUNCTION private
//	Bitmap::Condition::setUpperData -- 上限条件を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar*& p_
//		検索条件文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Condition::setUpperData(const ModUnicodeChar*& p_)
{
	using namespace LogicalFile;

	if (*p_ == 0 || m_bValid == false)
		return;
	const ModUnicodeChar* p = p_;
	p++;
	if (*p == 'l')
	{
		// #le(....)
		bool isOnlyLT = true;
		
		p += 3;
		Common::Data::Pointer pData;
		Data::Type::Value eKeyType;

		while (*p != ')')
		{
			ModUnicodeString cstrValue;
			ModUnicodeChar usOptionalChar = 0;
			Data::MatchType::Value eType
				= OpenOption::ParseValue::getStream(
					p, cstrValue, usOptionalChar);
			if (eType != Data::MatchType::LessThan)
				isOnlyLT = false;
			eKeyType = getKeyType(m_eKeyType, usOptionalChar);
			pData = createCommonData(eKeyType, cstrValue);

			if (eType == Data::MatchType::LessThan)
			{
				m_vecOtherCondition.pushBack(
					makeCond(eType, *pData, usOptionalChar));
			}

		}
		p++;

		if (pData.get())
		{
			// メモリーイメージを得る
			ModSize size = m_cData.getSize(*pData);
			m_cUpperData.m_pBuffer
				= syd_reinterpret_cast<ModUInt32*>(
					Os::Memory::allocate(size * sizeof(ModUInt32)));
			m_cData.dump(m_cUpperData.m_pBuffer, *pData);

			// 比較クラス
			m_cUpperData.m_cCompare.setType(eKeyType, false);

			if (isOnlyLT == true)
				m_cUpperData.m_eType = Data::MatchType::LessThan;
			else
				m_cUpperData.m_eType = Data::MatchType::LessThanEquals;
		}
		
		p_ = p;
	}
}

//
//	FUNCTION private
//	Bitmap::Condition::setOtherData -- その他条件を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar*& p_
//		検索条件文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Condition::setOtherData(const ModUnicodeChar*& p_)
{
	using namespace LogicalFile;

	if (*p_ == 0 || m_bValid == false)
		return;
	const ModUnicodeChar* p = p_;
	p++;
	if (*p == 'o')
	{
		// #ot(....)
		Common::DataArrayData cArray;
		
		p += 3;
		ModVector<Data::Type::Value> vecType;
		while (*p != ')')
		{
			ModUnicodeString cstrValue;
			ModUnicodeChar usOptionalChar = 0;
			Data::MatchType::Value eType
				= OpenOption::ParseValue::getStream(
					p, cstrValue, usOptionalChar);
			Data::Type::Value eKeyType = getKeyType(m_eKeyType, usOptionalChar);
			Common::Data::Pointer pData
				= createCommonData(eKeyType, cstrValue);
			m_vecOtherCondition.pushBack(
				makeCond(eType, *pData, usOptionalChar));
		}
		p++;
		
		p_ = p;
	}
}

//
//	FUNCTION private
//	Bitmap::Condition::createCommonData -- Common::Dataを作成する
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::Data::Type::Value
//		データ型
//	ModUnicodeString& cstrValue_
//		文字列データ
//
//	RETURN
//	Common::Data::Pointer
//		データオブジェクト
//
//	EXCEPTIONS
//
Common::Data::Pointer
Condition::createCommonData(Data::Type::Value eType_,
							ModUnicodeString& cstrValue_)
{
	Common::Data::Pointer pData;

	switch (eType_)
	{
	case Data::Type::Integer:
		{
			Common::StringData cSrc(cstrValue_);
			pData = cSrc.cast(Common::DataType::Integer, true);
		}
		break;
	case Data::Type::UnsignedInteger:
		{
			Common::StringData cSrc(cstrValue_);
			pData = cSrc.cast(Common::DataType::UnsignedInteger, true);
		}
		break;
#ifdef OBSOLETE
	case Data::Type::Float:
		{
			Common::StringData cSrc(cstrValue_);
			pData = cSrc.cast(Common::DataType::Float, true);
		}
		break;
#endif
	case Data::Type::Double:
		{
			Common::StringData cSrc(cstrValue_);
			pData = cSrc.cast(Common::DataType::Double, true);
		}
		break;
	case Data::Type::Decimal:
	{
		int iPrecision;
		int iScale;
		Data::Decimal::getParameter(
			m_cFileID, 0, m_cFileID.isArray(), iPrecision, iScale);
		Common::DecimalData cDec(iPrecision, iScale);
		Common::StringData cSrc(cstrValue_);
		if ("" == cstrValue_)
			pData = Common::NullData::getInstance();
		else
			pData = cSrc.cast(cDec, true);
	}
	break;
	case Data::Type::CharString:
	case Data::Type::UnicodeString:
	case Data::Type::NoPadCharString:
	case Data::Type::NoPadUnicodeString:
		{
			Common::StringData* pStringData
				= new Common::StringData(cstrValue_);
			pData = pStringData;
		}
		break;
#ifdef OBSOLETE
	case Data::Type::Date:
		pData = new Common::DateData(cstrValue_);
		break;
#endif
	case Data::Type::DateTime:
		pData = new Common::DateTimeData(cstrValue_);
		break;
#ifdef OBSOLETE
	case Data::Type::ObjectID:
		{
			Common::StringData cSrc(cstrValue_);
			pData = cSrc.cast(Common::DataType::ObjectID, true);
		}
		break;
#endif
	case Data::Type::LanguageSet:
		pData = new Common::LanguageData(cstrValue_);
		break;
	case Data::Type::Integer64:
		{
			Common::StringData cSrc(cstrValue_);
			pData = cSrc.cast(Common::DataType::Integer64, true);
		}
		break;
	default:
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	
	return pData;
}

//
//	FUNCTION private
//	Bitmap::Condition::makeCond -- Cond構造体を作成する
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::Data::MatchType::Value eType_
//		比較タイプ
//	const Common::Data& cData_
//		データ
//	int n_
//		フィールド番号
//	ModUnicodeChar usOptionalChar_
//		エスケープ文字
//
//	RETURN
//	Bitmap::Condition::Cond
//		Cond構造体
//
//	EXCEPTIONS
//
Condition::Cond
Condition::makeCond(Data::MatchType::Value eType_,
					const Common::Data& cData_,
					ModUnicodeChar usOptionalChar_)
{
	Cond cCond;
	cCond.m_eType = eType_;
	cCond.m_usOptionalChar = usOptionalChar_;
	Data::Type::Value eKeyType = getKeyType(m_eKeyType, usOptionalChar_);
	ModSize size = Data::getSize(cData_, eKeyType);
	cCond.m_pBuffer = syd_reinterpret_cast<ModUInt32*>(
		Os::Memory::allocate(size * sizeof(ModUInt32)));
	ModUInt32* p = cCond.m_pBuffer;
	Data::dump(p, cData_, eKeyType);
	return cCond;
}

//
//	FUNCTION private
//	Bitmap::Condition::getKeyType --
//
//	NOTES
//
//	ARGUMENTS
//	Data::Type::Value eType_
//	ModUnicodeChar usOptionalChar_
//
//	RETURN
//
//	EXCEPTIONS
//
Data::Type::Value
Condition::getKeyType(Data::Type::Value eType_,
					  ModUnicodeChar usOptionalChar_) const
{
	Data::Type::Value result = eType_;
	// [OPTIMIZE]
	// usOptionalChar_ has TWO applications.
	// One is an escape character for LIKE,
	// the other is a padding character for other than LIKE.
	// But the former situation is NOT considered in the folowing code.
	// In a current implementation,
	// LIKE does NOT care whether the type with NO PAD or without NO PAD.
	// So the code is NOT wrong. But it is NOT easily understandable.
	if (usOptionalChar_ != _usPaddingChar)
	{
		// Change stirng types to the types with NO PAD.
		if (result == Data::Type::UnicodeString)
			result = Data::Type::NoPadUnicodeString;
		else if (result == Data::Type::CharString)
			result = Data::Type::NoPadCharString;
	}
	return result;
}

//
//	Copyright (c) 2005, 2006, 2007, 2008, 2009, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
