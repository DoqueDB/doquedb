// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Condition.cpp --
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "Btree2/Condition.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/StringData.h"
#include "Common/NullData.h"
#include "Common/UnicodeString.h"
#ifdef OBSOLETE
#include "Common/DateData.h"
#endif
#include "Common/DateTimeData.h"
#include "Common/LanguageData.h"

#include "Exception/BadArgument.h"
#include "Exception/InvalidEscape.h"

#include "FileCommon/FileOption.h"
#include "FileCommon/OpenOption.h"

#include "Utility/CharTrait.h"

#include "ModUnicodeString.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_BTREE2_USING

namespace
{
	//
	//	FUNCTION local
	//	_$$::_freeVector
	//
	void _freeVector(ModVector<Condition::ParseValue*>& v)
	{
		ModVector<Condition::ParseValue*>::Iterator i = v.begin();
		for (;i != v.end(); ++i)
		{
			delete *i;
			*i = 0;
		}
	}

	//
	//	FUNCTION local
	//	_$$::_null
	//
	ModUnicodeString _null = "(null)";

	//
	//	CONSTANT local
	//	_$$::_maxDataSize
	//
	ModSize _maxDataSize = (FileID::MAX_SIZE + 1) * sizeof(ModUInt32);

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
//	Btree2::Condition::LimitCond::allocate
//		-- 検索条件バッファを確保する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiSize_
//		バッファサイズ(byte)
//
//	RETURN
//	なし
//
//	EXCPETIONS
//
void
Condition::LimitCond::allocate(ModSize uiSize_)
{
	m_pBuffer =
		syd_reinterpret_cast<ModUInt32*>(
			Os::Memory::allocate(uiSize_));
	m_uiBufferSize = uiSize_;
}

//
//	FUNCTION public
//	Btree2::Condition::LimitCond::copy
//		-- 中身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCPETIONS
//
void
Condition::LimitCond::copy(const Condition::LimitCond& other_)
{
	m_eType = other_.m_eType;
	if (other_.m_uiBufferSize)
	{
		m_uiBufferSize = other_.m_uiBufferSize;
		m_pBuffer = syd_reinterpret_cast<ModUInt32*>(
			Os::Memory::allocate(m_uiBufferSize));
		Os::Memory::copy(m_pBuffer.get(), other_.m_pBuffer.get(),
						 m_uiBufferSize);
	}
	m_nullBitmap = other_.m_nullBitmap;
	m_cCompare = other_.m_cCompare;
}

//
//	FUNCTION public
//	Btree2::Condition::Cond::allocate
//		-- 検索条件バッファを確保する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiSize_
//		バッファサイズ(byte)
//
//	RETURN
//	なし
//
//	EXCPETIONS
//
void
Condition::Cond::allocate(ModSize uiSize_)
{
	m_pBuffer =
		syd_reinterpret_cast<ModUInt32*>(
			Os::Memory::allocate(uiSize_));
	m_uiBufferSize = uiSize_;
}

//
//	FUNCTION public
//	Btree2::Condition::Cond::copy
//		-- 中身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCPETIONS
//
void
Condition::Cond::copy(const Condition::Cond& other_)
{
	m_eType = other_.m_eType;
	if (other_.m_uiBufferSize)
	{
		m_uiBufferSize = other_.m_uiBufferSize;
		m_pBuffer = syd_reinterpret_cast<ModUInt32*>(
			Os::Memory::allocate(m_uiBufferSize));
		Os::Memory::copy(m_pBuffer.get(), other_.m_pBuffer.get(),
						 m_uiBufferSize);
	}
	m_iFieldID = other_.m_iFieldID;
	m_usOptionalChar = other_.m_usOptionalChar;
}

//
//	FUNCTION public
//	Btree2::Condition::Condition -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Btree2::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Condition::Condition(const FileID& cFileID_)
	: m_cFileID(cFileID_), m_vecKeyType(cFileID_.getKeyType()),
	  m_vecKeyPosition(cFileID_.getKeyPosition()),
	  m_isNormalized(cFileID_.isNormalized()),
	  m_bValid(true), m_bConstraintLock(false),
	  m_bHeader(m_cFileID.isUseHeader()),
	  m_iFetchField(0), m_iExpandedFetchField(-1)
{
	m_cCompare.setType(m_vecKeyType, 0, m_bHeader);
	m_cData.setType(m_vecKeyType, 0, m_bHeader);
	m_cLowerData.clear();
	m_cUpperData.clear();
	m_bLowerIsUpper = false;
}

//
//	FUNCTION public
//	Btree2::Condition::~Condition -- デストラクタ
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
//	Btree2::Condition::copy
//		-- コピーする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Btree2::Condition*
//		コピーした条件クラス
//
//	EXCEPTIONS
//
Condition*
Condition::copy()
{
	Condition* pNew = new Condition(m_cFileID);
	
	pNew->m_cLowerData.copy(m_cLowerData);
	pNew->m_cUpperData.copy(m_cUpperData);
	pNew->m_bLowerIsUpper = m_bLowerIsUpper;
	pNew->m_vecOtherCondition.assign(m_vecOtherCondition.getSize());
	ModVector<Cond>::Iterator d = pNew->m_vecOtherCondition.begin();
	ModVector<Cond>::Iterator s = m_vecOtherCondition.begin();
	for (; s != m_vecOtherCondition.end(); ++s)
	{
		(*d).copy(*s);
	}
	pNew->m_bValid = m_bValid;
	pNew->m_cFetchData = m_cFetchData;
	pNew->m_iFetchField = m_iFetchField;
	pNew->m_vecFetchType = m_vecFetchType;
	pNew->m_iExpandedFetchField = m_iExpandedFetchField;
	pNew->m_vecExpandedFetchType = m_vecExpandedFetchType;

	return pNew;
}

//
//	FUNCTION public
//	Btree2::Condition::getSearchParameter
//		-- TreeNodeから検索構文を作成する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		TreeNodeの検索条件
//	LogicalFile::OpenOption& cOpenOption_
//		検索構文を設定するオープンオプション
//
//	RETURN
//	bool
//		B木で実行できる検索文の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Condition::getSearchParameter(const LogicalFile::TreeNodeInterface* pCondition_,
							  LogicalFile::OpenOption& cOpenOption_)
{
	using namespace LogicalFile;

	if (pCondition_)
	{
		const TreeNodeInterface* pNode = pCondition_;
		
		if (pNode->getType() == TreeNodeInterface::Fetch)
		{
			// Fetchモード

			// 第一要素がFetchされるカラムリスト
			const TreeNodeInterface* pFetchFields
				= pNode->getOptionAt(0);

			// B木のFetchは先頭フィールドから順番に指定されているばあいのみ可能
			int n = 1;
			for (int i = 0;
				 i < static_cast<int>(pFetchFields->getOperandSize()); ++i)
			{
				const TreeNodeInterface* pField = pFetchFields->getOperandAt(i);
				int v = ModUnicodeCharTrait::toInt(pField->getValue());
				if (pField->getType() != TreeNodeInterface::Field || v != n)
					return false;
				n++;
			}

			// Fetchの時はOpenModeはSearch
			cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::OpenMode::Key),
									FileCommon::OpenOption::OpenMode::Search);

			// Fetchに利用するフィールド数
			cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
										Key::FetchFieldNumber),	n - 1);
		}
		else
		{
			int size = 1;
			int n = 0;
	
			if (pNode->getType() == TreeNodeInterface::Or)
			{
				if (m_cFileID.isLastRowID() == false)
				{
					// 最後の要素がROWIDでないときは、ORは実行できない
					return false;
				}
				
				size = static_cast<int>(pNode->getOperandSize());
				pNode = pCondition_->getOperandAt(n);
			}
			
			while (n != size)
			{
				if (parseTreeNode(pNode, n++, cOpenOption_) == false)
					return false;

				if (n != size)
					pNode = pCondition_->getOperandAt(n);
			}
			
			// 検索条件数
			cOpenOption_.setInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(Key::ConditionCount), size);
		}
	}
	
	// オブジェクトをドライバ側で保持しない
	cOpenOption_.setBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::CacheAllObject::Key),
		false);

	// オープンモード
	int iValue;
	if (cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(
		FileCommon::OpenOption::OpenMode::Key), iValue) == false)
	{
		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
			FileCommon::OpenOption::OpenMode::Key),
								FileCommon::OpenOption::OpenMode::Read);
	}
	
	return true;
}

//
//	FUNCTION public
//	Btree2::Condition::setOpenOption -- オープンオプションを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::OpenOption& cOpenOption_
//		検索条件を得るオープンオプション
//	int iNumber_
//		検索条件の要素数 (default 0)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Condition::setOpenOption(const LogicalFile::OpenOption& cOpenOption_,
						 int iNumber_)
{
	// オープンモードを得る
	LogicalFile::OpenOption::OpenMode::Value eMode
		= static_cast<LogicalFile::OpenOption::OpenMode::Value>(
			cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::OpenMode::Key)));

	m_cLowerData.clear();
	m_cUpperData.clear();
	m_vecOtherCondition.clear();
	m_bValid = true;
	
	ModUnicodeString c = cOpenOption_.getString(
		_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(Key::Condition, iNumber_));
	const ModUnicodeChar* p = c;

	// 下限条件を設定する
	setLowerData(p, eMode);
	// 上限条件を設定する
	setUpperData(p, eMode);
	// その他条件を設定する
	setOtherData(p);
}

//
//	FUNCTION public
//	Btree2::Condition::setFetchKey -- fetchのキーを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cKey_
//		fetchのキー
//
//	RETURN
//
//	EXCEPTIONS
//
void
Condition::setFetchKey(const Common::DataArrayData& cKey_)
{
	using namespace LogicalFile;
	
	if (m_cFetchData.getTypeCount() == 0)
	{
		// First time to fetch.

		// Initialize
		m_cLowerData.clear();
		m_cUpperData.clear();

		; _SYDNEY_ASSERT(cKey_.getCount() > 0);
		; _SYDNEY_ASSERT(m_vecKeyType.getSize() > 0);
		int iCount = ModMin(cKey_.getCount(),
							static_cast<int>(m_vecKeyType.getSize()));
		; _SYDNEY_ASSERT(m_vecFetchType.getSize() == 0);
		m_vecFetchType.reserve(iCount);
		; _SYDNEY_ASSERT(m_iExpandedFetchField == -1);
		m_iExpandedFetchField = 0;
		; _SYDNEY_ASSERT(m_vecExpandedFetchType.getSize() == 0);
		m_vecExpandedFetchType.reserve(iCount);
		
		for (int i = 0; i < iCount; ++i)
		{
			// Set the type.
			Data::Type::Value eKeyType = m_vecKeyType[i];
			m_vecFetchType.pushBack(eKeyType);

			// Set the type for OtherCondition.
			Data::Type::Value eExpandedKeyType = Data::Type::Undefined;
			if (eKeyType == Data::Type::UnicodeString ||
				eKeyType == Data::Type::CharString)
			{
				// Check the collation in the case of the string type.
				const Common::Data& cTemp = *cKey_.getElement(i).get();
				if (cTemp.isNull() == false
					&& cTemp.getType() == Common::DataType::String)
				{
					const Common::StringData& cStringData
						= _SYDNEY_DYNAMIC_CAST(const Common::StringData&,
											   cTemp);
					if (cStringData.getCollation()
						== Common::Collation::Type::NoPad)
					{
						if (eKeyType == Data::Type::UnicodeString)
							eExpandedKeyType = Data::Type::NoPadUnicodeString;
						else
							eExpandedKeyType = Data::Type::NoPadCharString;
						++m_iExpandedFetchField;
					}
				}
			}
			m_vecExpandedFetchType.pushBack(eExpandedKeyType);
		}
		m_cFetchData.setType(m_vecFetchType, 0, m_bHeader);
		m_cLowerData.allocate(_maxDataSize);
		bool isUnique = (m_vecFetchType.getSize() == m_vecKeyType.getSize());
		m_cLowerData.m_cCompare.setType(m_vecFetchType, isUnique, m_bHeader);
		m_cLowerData.m_eType = TreeNodeInterface::Equals;

		// Set Upper condition.
		m_bLowerIsUpper = true;
	}
	else
	{
		// Second time or later
		
		; _SYDNEY_ASSERT(m_iExpandedFetchField >= 0);
		for (int i = 0; i < m_iExpandedFetchField; ++i)
		{
			// This instance is reused in some cases.
			// So remove the conditions of the previous fetch.
			m_vecOtherCondition.popBack();
		}
	}
	m_cLowerData.m_nullBitmap = 0;
	ModSize size = m_cFetchData.getSize(cKey_);
	if (m_cLowerData.m_uiBufferSize < size)
	{
		ModSize s = (size + _maxDataSize - 1) / _maxDataSize * _maxDataSize;
		m_cLowerData.allocate(s);
	}

	// Set Lower condition.
	if (m_bHeader)
		m_cFetchData.dump(m_cLowerData.m_pBuffer, cKey_);
	else
		m_cFetchData.dump(m_cLowerData.m_pBuffer, cKey_,
						  m_cLowerData.m_nullBitmap);

	// Set Other condition.
	if (m_iExpandedFetchField > 0)
	{
		ModVector<Data::Type::Value>::ConstIterator i =
			m_vecExpandedFetchType.begin();
		int n = 0;
		for (;i != m_vecExpandedFetchType.end(); ++i, ++n)
		{
			if (*i != Data::Type::Undefined)
			{
				// Compare with NO PAD.
				TreeNodeInterface::Type eKeyType = TreeNodeInterface::Equals;
				// Not need to check whether the pData is null or not.
				// It has been checked in the first time to fetch.
				Common::Data::Pointer pData = cKey_.getElement(n);
				ModUnicodeChar usOptionalChar = 0;
				m_vecOtherCondition.pushBack(
					makeCond(eKeyType, *pData, n, usOptionalChar));
			}
		}
	}
}

//
//	FUNCTION public
//	Btree2::Condition::isOtherConditionMatch
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
	using namespace LogicalFile;

	if (m_bHeader)
	{
		const Data::Header* h
			= syd_reinterpret_cast<const Data::Header*>(pBuffer_);
		pBuffer_ += Data::Header::getSize();

		if (h->isExpunge() && m_bConstraintLock == false)
		{
			// 削除されているエントリなので、ヒットしてはだめ
			
			return false;
		}

		// その他条件にマッチしているか確認する

		return isOtherConditionMatch(pBuffer_, h->getNullBitmap());
	}

	bool result =  true;
	if (m_vecOtherCondition.getSize())
	{
		const ModUInt32* vecpBuffer[FileID::MAX_FIELD_COUNT];
		ModVector<Data::Type::Value>::ConstIterator j = m_vecKeyType.begin();
		int n = 0;
		for (; j != m_vecKeyType.end(); ++j, ++n)
		{
			vecpBuffer[n] = pBuffer_;
			Data::getSize(pBuffer_, *j);
		}
	
		ModVector<Cond>::Iterator i = m_vecOtherCondition.begin();
		for (; i != m_vecOtherCondition.end(); ++i)
		{
			if ((*i).m_eType == TreeNodeInterface::EqualsToNull)
			{
				// nullのデータはないので
				result = false;
				break;
			}

			const ModUInt32* p1 = vecpBuffer[(*i).m_iFieldID];
			const ModUInt32* p2 = (*i).m_pBuffer.get();
			
			if ((*i).m_eType == TreeNodeInterface::Like)
			{
				result = m_cCompare.like(p1, p2,
										 m_vecKeyType[(*i).m_iFieldID],
										 (*i).m_usOptionalChar);
			}
			else
			{
				Data::Type::Value eKeyType = getKeyType(
					m_vecKeyType[(*i).m_iFieldID], (*i).m_usOptionalChar);
				int r = m_cCompare.compare(p1, p2, eKeyType);
				
				switch ((*i).m_eType)
				{
				case TreeNodeInterface::Equals:
					if (r != 0) result = false;
					break;
				case TreeNodeInterface::NotEquals:
					if (r == 0) result = false;
					break;
				case TreeNodeInterface::GreaterThan:
					if (r <= 0) result = false;
					break;
				case TreeNodeInterface::GreaterThanEquals:
					if (r < 0) result = false;
					break;
				case TreeNodeInterface::LessThan:
					if (r >= 0) result = false;
					break;
				case TreeNodeInterface::LessThanEquals:
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
//	Btree2::Condition::isOtherConditionMatch
//		-- その他条件にマッチしているかどうか
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pBuffer_
//		比較するバッファ
//	unsigned char nullBitmap_
//		nullビットマップ
//
//	RETURN
//	bool
//		マッチしている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Condition::isOtherConditionMatch(const ModUInt32* pBuffer_,
								 unsigned char nullBitmap_)
{
	using namespace LogicalFile;

	bool result =  true;
	if (m_vecOtherCondition.getSize())
	{
		const ModUInt32* vecpBuffer[FileID::MAX_FIELD_COUNT];
		int bit = 1;
		ModVector<Data::Type::Value>::ConstIterator j = m_vecKeyType.begin();
		int n = 0;
		for (; j != m_vecKeyType.end(); ++j, ++n)
		{
			if (nullBitmap_ & bit)
			{
				vecpBuffer[n] = 0;
			}
			else
			{
				vecpBuffer[n] = pBuffer_;
				Data::getSize(pBuffer_, *j);
			}
			bit <<= 1;
		}
	
		ModVector<Cond>::Iterator i = m_vecOtherCondition.begin();
		for (; i != m_vecOtherCondition.end(); ++i)
		{
			bool b1 = false;
			bool b2 = false;
			if (vecpBuffer[(*i).m_iFieldID] == 0)
			{
				b1 = true;
			}

			const ModUInt32* p1 = vecpBuffer[(*i).m_iFieldID];
			const ModUInt32* p2 = (*i).m_pBuffer.get();

			if ((*i).m_eType == TreeNodeInterface::Like)
			{
				if (b1 == true)
					result = false;
				else
					result = m_cCompare.like(
						p1, p2,	m_vecKeyType[(*i).m_iFieldID],
						(*i).m_usOptionalChar);
			}
			else if ((*i).m_eType == TreeNodeInterface::EqualsToNull)
			{
				if (b1 != true)
					result = false;
			}
			else
			{
				if (b1 == true)
					result = false;
				else
				{
					Data::Type::Value eKeyType = getKeyType(
						m_vecKeyType[(*i).m_iFieldID], (*i).m_usOptionalChar);
					int r = m_cCompare.compare(p1, b1, p2, b2, eKeyType);
					
					switch ((*i).m_eType)
					{
					case TreeNodeInterface::Equals:
						if (r != 0) result = false;
						break;
					case TreeNodeInterface::NotEquals:
						if (r == 0) result = false;
						break;
					case TreeNodeInterface::GreaterThan:
						if (r <= 0) result = false;
						break;
					case TreeNodeInterface::GreaterThanEquals:
						if (r < 0) result = false;
						break;
					case TreeNodeInterface::LessThan:
						if (r >= 0) result = false;
						break;
					case TreeNodeInterface::LessThanEquals:
						if (r > 0) result = false;
						break;
					}
				}
			}
		
			if (result == false)
				break;
		}
	}
	return result;
}

//
//	FUNCTION private
//	Btree2::Condition::parseTreeNode -- TreeNodeをパースし、OpenOptionに設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	int n
//		何番目か
//	LogicalFile::OpenOption& cOpenOption_
//		設定するOpenOption
//
//	RETURN
//	bool
//		B木で実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Condition::parseTreeNode(const LogicalFile::TreeNodeInterface* pCondition_,
						 int n_,
						 LogicalFile::OpenOption& cOpenOption_)
{
	using namespace LogicalFile;
	
	ModVector<ParseValue*> vecMain;
	ModVector<ParseValue*> vecOther;

	vecMain.assign(m_vecKeyType.getSize(), 0);
	vecOther.assign(m_vecKeyType.getSize(), 0);

	int n = 0;
	int size = 1;
	const TreeNodeInterface* pNode = pCondition_;
	if (pNode->getType() == TreeNodeInterface::And)
	{
		size = static_cast<int>(pNode->getOperandSize());
		pNode = pCondition_->getOperandAt(n);
	}

	while (n != size)
	{
		// 1つ1つパースする
		if (parseOneNode(pNode, vecMain, vecOther) == false)
		{
			_freeVector(vecMain);
			_freeVector(vecOther);
			return false;
		}

		if (++n != size)
			pNode = pCondition_->getOperandAt(n);
	}

	// OpenOptionに設定する
	if (setToOpenOption(vecMain, vecOther, n_, cOpenOption_) == false)
	{
		_freeVector(vecMain);
		_freeVector(vecOther);
		return false;
	}
	
	_freeVector(vecMain);
	_freeVector(vecOther);
	return true;
}

//
//	FUNCTION private
//	Btree2::Condition::parseOneNode -- 1つのノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ModVector<ParseValue*>& vecMain_
//		主検索条件
//	ModVector<ParseValue*>& vecOther_
//		副検索条件
//
//	RETURN
//	bool
//		B木で実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Condition::parseOneNode(const LogicalFile::TreeNodeInterface* pCondition_,
						ModVector<ParseValue*>& vecMain_,
						ModVector<ParseValue*>& vecOther_)
{
	using namespace LogicalFile;

	bool result = false;
	
	switch (pCondition_->getType())
	{
	case TreeNodeInterface::Equals:
		result = parseEqualsNode(pCondition_, vecMain_, vecOther_);
		break;
	case TreeNodeInterface::LessThan:
	case TreeNodeInterface::LessThanEquals:
		result = parseLessThanNode(pCondition_, vecMain_, vecOther_);
		break;
	case TreeNodeInterface::GreaterThan:
	case TreeNodeInterface::GreaterThanEquals:
		result = parseGreaterThanNode(pCondition_, vecMain_, vecOther_);
		break;
	case TreeNodeInterface::NotEquals:
		result = parseNotEqualsNode(pCondition_, vecMain_, vecOther_);
		break;
	case TreeNodeInterface::EqualsToNull:
		result = parseEqualsToNullNode(pCondition_, vecMain_, vecOther_);
		break;
	case TreeNodeInterface::Like:
		result = parseLikeNode(pCondition_, vecMain_, vecOther_);
		break;
	default:
		;
	}

	return result;
}

//
//	FUNCTION private
//	Btree2::Condition::parseEqualsNode -- Equalsノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ModVector<ParseValue*>& vecMain_
//		主検索条件
//	ModVector<ParseValue*>& vecOther_
//		副検索条件
//
//	RETURN
//	bool
//		B木で実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Condition::parseEqualsNode(const LogicalFile::TreeNodeInterface* pCondition_,
						   ModVector<ParseValue*>& vecMain_,
						   ModVector<ParseValue*>& vecOther_)
{
	using namespace LogicalFile;

	if (vecMain_[0] != 0 && vecMain_[0]->m_eType == TreeNodeInterface::Unknown)
		return true;

	TreeNodeInterface::Type eMatch = TreeNodeInterface::Equals;
	ModUnicodeString cstrValue;
	bool isValid;
	bool bNoPadKey;
	int field = checkTwoTerm(
		pCondition_, eMatch, cstrValue, isValid, bNoPadKey);
	if (field < 0)
	{
		return false;
	}
	bool bNoPadField = checkNoPadSortOrder(
		m_vecKeyType[field], m_cFileID.isFixed(field), bNoPadKey);

	if (isValid == false)
	{
		// nullとの比較はつねにunknown
		setUnknownParseValue(vecMain_, vecOther_);
		return true;
	}

	ParseValue* pNew = new ParseValue;
	pNew->m_eType = TreeNodeInterface::Equals;
	pNew->m_cValue = cstrValue;
	pNew->m_usOptionalChar = getPaddingChar(bNoPadField || bNoPadKey);

	setEqualsParseValue(pNew, vecMain_[field], vecOther_[field],
						bNoPadField, bNoPadKey);
	
	return true;
}

//
//	FUNCTION private
//	Btree2::Condition::parseLessThanNode -- LessThanノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ModVector<ParseValue*>& vecMain_
//		主検索条件
//	ModVector<ParseValue*>& vecOther_
//		副検索条件
//
//	RETURN
//	bool
//		B木で実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Condition::parseLessThanNode(const LogicalFile::TreeNodeInterface* pCondition_,
							 ModVector<ParseValue*>& vecMain_,
							 ModVector<ParseValue*>& vecOther_)
{
	using namespace LogicalFile;

	if (vecMain_[0] != 0 && vecMain_[0]->m_eType == TreeNodeInterface::Unknown)
		return true;

	TreeNodeInterface::Type eMatch = pCondition_->getType();
	ModUnicodeString cstrValue;
	bool isValid;
	bool bNoPadKey;
	int field = checkTwoTerm(
		pCondition_, eMatch, cstrValue, isValid, bNoPadKey);
	if (field < 0)
		return false;
	bool bNoPadField = checkNoPadSortOrder(
		m_vecKeyType[field], m_cFileID.isFixed(field), bNoPadKey);

	if (isValid == false)
	{
		// nullとの比較はつねにunknown
		setUnknownParseValue(vecMain_, vecOther_);
		return true;
	}

	ParseValue* pNew = new ParseValue;
	pNew->m_eType = eMatch;
	pNew->m_cValue = cstrValue;
	pNew->m_usOptionalChar = getPaddingChar(bNoPadField || bNoPadKey);

	if (eMatch == TreeNodeInterface::LessThan ||
		eMatch == TreeNodeInterface::LessThanEquals)
	{
		setLessThanParseValue(pNew, vecMain_[field], vecOther_[field],
							  bNoPadField, bNoPadKey);
	}
	else
	{
		; _SYDNEY_ASSERT(eMatch == TreeNodeInterface::GreaterThan
						 || eMatch == TreeNodeInterface::GreaterThanEquals);
		
		// Field and Value were interchanged in checkTwoTerm().
		setGreaterThanParseValue(pNew, vecMain_[field], vecOther_[field],
								 bNoPadField, bNoPadKey);
	}

	return true;
}

//
//	FUNCTION private
//	Btree2::Condition::parseGreaterThanNode -- GreaterThanノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ModVector<ParseValue*>& vecMain_
//		主検索条件
//	ModVector<ParseValue*>& vecOther_
//		副検索条件
//
//	RETURN
//	bool
//		B木で実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Condition::parseGreaterThanNode(
	const LogicalFile::TreeNodeInterface* pCondition_,
	ModVector<ParseValue*>& vecMain_,
	ModVector<ParseValue*>& vecOther_)
{
	using namespace LogicalFile;

	if (vecMain_[0] != 0 && vecMain_[0]->m_eType == TreeNodeInterface::Unknown)
		return true;

	TreeNodeInterface::Type eMatch = pCondition_->getType();
	ModUnicodeString cstrValue;
	bool isValid;
	bool bNoPadKey;
	int field = checkTwoTerm(
		pCondition_, eMatch, cstrValue, isValid, bNoPadKey);
	if (field < 0)
		return false;
	bool bNoPadField = checkNoPadSortOrder(
		m_vecKeyType[field], m_cFileID.isFixed(field), bNoPadKey);

	if (isValid == false)
	{
		// nullとの比較はつねにunknown
		setUnknownParseValue(vecMain_, vecOther_);
		return true;
	}

	ParseValue* pNew = new ParseValue;
	pNew->m_eType = eMatch;
	pNew->m_cValue = cstrValue;
	pNew->m_usOptionalChar = getPaddingChar(bNoPadField || bNoPadKey);

	if (eMatch == TreeNodeInterface::GreaterThan ||
		eMatch == TreeNodeInterface::GreaterThanEquals)
	{
		setGreaterThanParseValue(pNew, vecMain_[field], vecOther_[field],
								 bNoPadField, bNoPadKey);
	}
	else
	{
		; _SYDNEY_ASSERT(eMatch == TreeNodeInterface::LessThan
						 || eMatch == TreeNodeInterface::LessThanEquals);

		// Field and Value were interchanged in checkTwoTerm().
		setLessThanParseValue(pNew, vecMain_[field], vecOther_[field],
							  bNoPadField, bNoPadKey);
	}

	return true;
}

//
//	FUNCTION private
//	Btree2::Condition::parseNotEqualsNode -- NotEqualsノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ModVector<ParseValue*>& vecMain_
//		主検索条件
//	ModVector<ParseValue*>& vecOther_
//		副検索条件
//
//	RETURN
//	bool
//		B木で実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Condition::parseNotEqualsNode(const LogicalFile::TreeNodeInterface* pCondition_,
							  ModVector<ParseValue*>& vecMain_,
							  ModVector<ParseValue*>& vecOther_)
{
	using namespace LogicalFile;

	if (vecMain_[0] != 0 && vecMain_[0]->m_eType == TreeNodeInterface::Unknown)
		return true;

	TreeNodeInterface::Type eMatch = TreeNodeInterface::NotEquals;
	ModUnicodeString cstrValue;
	bool isValid;
	bool bNoPadKey;
	int field = checkTwoTerm(
		pCondition_, eMatch, cstrValue, isValid, bNoPadKey);
	if (field < 0)
		return false;
	bool bNoPadField = checkNoPadSortOrder(
		m_vecKeyType[field], m_cFileID.isFixed(field), bNoPadKey);

	if (isValid == false)
	{
		// nullとの比較はつねにunknown
		setUnknownParseValue(vecMain_, vecOther_);
		return true;
	}
	
	ParseValue* pNew = new ParseValue;
	pNew->m_eType = eMatch;
	pNew->m_cValue = cstrValue;
	pNew->m_usOptionalChar = getPaddingChar(bNoPadField || bNoPadKey);

	// NotEquals operator is always used in OtherCondition.
	// So set the char for NO PAD,
	// even if the sort order of the field is PAD SPACE
	// and the collation of the key is NO PAD.
	pNew->m_pNext = vecOther_[field];
	vecOther_[field] = pNew;

	return true;
}

//
//	FUNCTION private
//	Btree2::Condition::parseEqualsToNullNode -- EqualsToNullノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ModVector<ParseValue*>& vecMain_
//		主検索条件
//	ModVector<ParseValue*>& vecOther_
//		副検索条件
//
//	RETURN
//	bool
//		B木で実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Condition::parseEqualsToNullNode(
	const LogicalFile::TreeNodeInterface* pCondition_,
	ModVector<ParseValue*>& vecMain_,
	ModVector<ParseValue*>& vecOther_)
{
	using namespace LogicalFile;

	if (vecMain_[0] != 0 && vecMain_[0]->m_eType == TreeNodeInterface::Unknown)
		return true;

	int field = checkOneTerm(pCondition_);
	if (field < 0)
		return false;

	ParseValue* pNew = new ParseValue;
	pNew->m_eType = TreeNodeInterface::EqualsToNull;

	if (vecMain_[field] != 0)
	{
		if (vecMain_[field]->m_eType != TreeNodeInterface::Equals
			&& vecMain_[field]->m_eType != TreeNodeInterface::EqualsToNull)
		{
			// equal検索が一番強いのでそれ以外の検索条件はvecOther_へ
			ParseValue* p = vecMain_[field];
			while (p->m_pNext) p = p->m_pNext;
			p->m_pNext = vecOther_[field];
			vecOther_[field] = vecMain_[field];
			vecMain_[field] = pNew;
		}
		else
		{
			// もうequalがあるのでvecOther_へ
			pNew->m_pNext = vecOther_[field];
			vecOther_[field] = pNew;
		}
	}
	else
	{
		// 初めて
		vecMain_[field] = pNew;
	}

	return true;
}

//
//	FUNCTION private
//	Btree2::Condition::parseLikeNode -- Likeノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ModVector<ParseValue*>& vecMain_
//		主検索条件
//	ModVector<ParseValue*>& vecOther_
//		副検索条件
//
//	RETURN
//	bool
//		B木で実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Condition::parseLikeNode(
	const LogicalFile::TreeNodeInterface* pCondition_,
	ModVector<ParseValue*>& vecMain_,
	ModVector<ParseValue*>& vecOther_)
{
	using namespace LogicalFile;

	if (vecMain_[0] != 0 && vecMain_[0]->m_eType == TreeNodeInterface::Unknown)
		return true;

	// Check the condition.
	TreeNodeInterface::Type eMatch = TreeNodeInterface::Like;
	ModUnicodeString cstrValue;
	bool isValid;
	bool bNoPadKey;
	int field = checkTwoTerm(
		pCondition_, eMatch, cstrValue, isValid, bNoPadKey);
	if (field < 0)
		return false;
	// Like is alwyas used with NO PAD.
	bNoPadKey = true;

	// Check the field.
	if (m_vecKeyType[field] != Data::Type::UnicodeString &&
		m_vecKeyType[field] != Data::Type::CharString &&
		m_vecKeyType[field] != Data::Type::NoPadUnicodeString &&
		m_vecKeyType[field] != Data::Type::NoPadCharString)
	{
		// Like supports only string type.
		return false;
	}
	bool bNoPadField = checkNoPadSortOrder(
		m_vecKeyType[field], m_cFileID.isFixed(field), bNoPadKey);
	
	// Check the condition again.
	if (isValid == false)
	{
		// nullとの比較はつねにunknown
		setUnknownParseValue(vecMain_, vecOther_);
		return true;
	}

	// 前方一致を上限下限に変換する
	ModUnicodeChar usEscape = 0;
	if (pCondition_->getOptionSize() >= 1)
	{
		// LIKE operator has two options, ESCAPE and LANGUAGE.
		// LANGUAGE option is ignored except for FullText Index.
		
		// ESCAPE option is set in the top, if it exists.
		const Common::Data* pData = pCondition_->getOptionAt(0)->getData();
		if (pData != 0 && pData->getType() == Common::DataType::String)
		{
			// The type of ESCAPE option is String,
			// and the type of LANGUAGE option is Language.
			
			// [CAUTION]
			// If new option is added,
			// new TreeNodeInterface::Type has to be added
			// and the type has to be used
			// for distinguishing ESCAPE from the new option.
			
			ModUnicodeString cstrEscape = pData->getString();
			if (cstrEscape.getLength() != 1)
				_SYDNEY_THROW0(Exception::InvalidEscape);
			usEscape = cstrEscape[0];
		}
	}
	ModUnicodeString cstrLower;
	ModUnicodeString cstrUpper;
	bool bNormalizedUpper = false;
	const ModUnicodeChar* p = cstrValue;
	while (*p != 0)
	{
		if (*p == usEscape)
		{
			p++;
			if (*p == 0)
			{
				break;
			}
			cstrLower += *p;
			cstrUpper += *p;
			p++;
			continue;
		}

		if (*p == Common::UnicodeChar::usPercent
			|| *p == Common::UnicodeChar::usLowLine)
		{
			if (cstrLower.getLength() == 0)
				return false;
			if (m_isNormalized == true)
			{
				// ++する前に正規化しないといけない
				Utility::CharTrait::normalize(
					cstrLower,
					m_cFileID.getNormalizingMethod(),
					cstrUpper);
				// Instead, this must not be normalized in createCommonData().
				bNormalizedUpper = true;
			}
			
			(cstrUpper[cstrUpper.getLength() - 1])++;
			if (*p == Common::UnicodeChar::usPercent)
				p++;
			break;
		}
		else
		{
			cstrLower += *p;
			cstrUpper += *p;
		}

		p++;
	};

	// Set the conditions.

	// Like is always used with NO PAD.
	// When the field is sorted with NO PAD,
	// just search the tuple with NO PAD.
	// But when the field is sorted with PAD SPACE, Like needs two steps.
	// 1. Search the tuple with PAD SPACE.
	// 2. Compare the searched tuple and the pattern with NO PAD.

	if (cstrLower == cstrUpper)
	{
		// 完全一致

		ParseValue* pNew = new ParseValue;
		pNew->m_eType = TreeNodeInterface::Equals;
		pNew->m_cValue = cstrLower;
		pNew->m_usOptionalChar = getPaddingChar(bNoPadField || bNoPadKey);

		setEqualsParseValue(pNew, vecMain_[field], vecOther_[field],
							bNoPadField, bNoPadKey);

		// The remains of the condition does NOT exist.
		; _SYDNEY_ASSERT(*p == 0);
	}
	else
	{
		// 前方一致

		ParseValue* pNew1 = new ParseValue;
		pNew1->m_eType = TreeNodeInterface::GreaterThanEquals;
		pNew1->m_cValue = cstrLower;
		pNew1->m_usOptionalChar = getPaddingChar(bNoPadField || bNoPadKey);
		
		ParseValue* pNew2 = new ParseValue;
		pNew2->m_eType = TreeNodeInterface::LessThan;
		pNew2->m_cValue = cstrUpper;
		pNew2->m_bNormalized = bNormalizedUpper;
		pNew2->m_usOptionalChar = pNew1->m_usOptionalChar;
		pNew1->m_pNext = pNew2;

		setPrefixMatchParseValue(pNew1, vecMain_[field], vecOther_[field],
								 bNoPadField, bNoPadKey);

		if (*p != 0 || bNoPadField == false && bNoPadKey == true)
		{
			// The reminds of the condition exist.
			setLikeParseValue(cstrValue, usEscape, vecOther_[field]);
		}
	}

	return true;
}

//
//	FUNCTION private
//	Btree2::Condition::checkOneTerm -- 単項演算をチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		単項演算のノード
//
//	RETURN
//	int
//		フィールド番号。B木で検索できないものの場合は-1
//
//	EXCEPTIONS
//
int
Condition::checkOneTerm(const LogicalFile::TreeNodeInterface* pCondition_)
{
	using namespace LogicalFile;
	
	int field = -1;
	if (pCondition_->getOperandSize() == 1)
	{
		const TreeNodeInterface* pField = pCondition_->getOperandAt(0);
		
		if (pField->getType() == TreeNodeInterface::Field)
		{
			// OIDが0なので、1を引く
			int n = ModUnicodeCharTrait::toInt(pField->getValue()) - 1;
			if (n < static_cast<int>(m_vecKeyType.getSize()))
			{
				field = n;
			}
		}
	}
	return field;
}

//
//	FUNCTION private
//	Btree2::Condition::checkTwoTerm -- 2項演算をチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		2項演算のノード
//	LogicalFile::TreeNodeInterface::Type& eMatch_
//		一致条件
//	ModUnicodeString& cstrValue_
//		データ
//	bool& isValid_
//		有効な条件かどうか
//	bool& bNoPadKey_
//
//	RETURN
//	int
//		フィールド番号。B木で検索できないものの場合は-1
//
//	EXCEPTIONS
//
int
Condition::checkTwoTerm(const LogicalFile::TreeNodeInterface* pCondition_,
						LogicalFile::TreeNodeInterface::Type& eMatch_,
						ModUnicodeString& cstrValue_,
						bool& isValid_,
						bool& bNoPadKey_)
{
	using namespace LogicalFile;
	
	int field = -1;
	isValid_ = false;
	bNoPadKey_ = false;
	eMatch_ = pCondition_->getType();
	
	if (pCondition_->getOperandSize() == 2)
	{
		const TreeNodeInterface* pField = pCondition_->getOperandAt(0);
		const TreeNodeInterface* pValue = pCondition_->getOperandAt(1);

		if (pValue->getType() == TreeNodeInterface::Field
			&& (pField->getType() == TreeNodeInterface::ConstantValue
				|| pField->getType() == TreeNodeInterface::Variable))
		{
			// Value was set to pField and Field was set to pValue.
			// These data is set to pCondition_ in order of SQL's string.
			alternateTerm(pValue, pField, eMatch_);
		}
		
		if (pField->getType() == TreeNodeInterface::Field
			&& (pValue->getType() == TreeNodeInterface::ConstantValue
				|| pValue->getType() == TreeNodeInterface::Variable))
		{
			// OIDが0なので、1を引く
			int n = ModUnicodeCharTrait::toInt(pField->getValue()) - 1;
			if (n < static_cast<int>(m_vecKeyType.getSize()))
			{
				field = n;
				const Common::Data* pData = pValue->getData();
				if (pData == 0)
				{
					// getDataできないもの
					// 互換性のためにエラーにしない
					
					cstrValue_ = pValue->getValue();
					isValid_ = true;
				}
				else if (!pData->isNull())
				{
					if (m_vecKeyType[field] == Data::Type::Integer
						&& (pData->getType() == Common::DataType::Double
							|| pData->getType() == Common::DataType::Integer64))
					{
						// フィールドがintで、比較対象がdoubleかint64だったら
						int value;
						if (Data::Integer::round(*pData,
												 eMatch_,
												 value) == true)
						{
							ModUnicodeOstrStream s;
							s << value;
							cstrValue_ = s.getString();
							isValid_ = true;
						}
					}
					else if (m_vecKeyType[field] == Data::Type::Integer64
							 && pData->getType() == Common::DataType::Double)
					{
						// フィールドがint64で、比較対象がdoubleだったら
						ModInt64 value;
						if (Data::Integer64::round(*pData,
												   eMatch_,
												   value) == true)
						{
							ModUnicodeOstrStream s;
							s << value;
							cstrValue_ = s.getString();
							isValid_ = true;
						}
					}
					else if ((m_vecKeyType[field] == Data::Type::UnicodeString
							  || m_vecKeyType[field] == Data::Type::CharString)
							 && pData->getType() == Common::DataType::String)
					{
						const Common::StringData* pStringData
							= _SYDNEY_DYNAMIC_CAST(const Common::StringData*,
												   pData);
						if (pStringData->getCollation()
							== Common::Collation::Type::NoPad)
						{
							// The collation of the condition is NO PAD.
							bNoPadKey_ = true;
						}
						// When the field is variable,
						// the white space of the end of the string data
						// in the field has been truncated.
						// But, NOT need to truncate the white space
						// from the condition of the key string.
						// 
						cstrValue_ = pStringData->getValue();
						isValid_ = true;
					}
					else if (m_vecKeyType[field]
							 == Data::Type::NoPadUnicodeString ||
							 m_vecKeyType[field]
							 == Data::Type::NoPadCharString)
					{
						// The collation of the field is NO PAD.
						cstrValue_ = pValue->getValue();
						isValid_ = true;
						bNoPadKey_ = true;

						// BACKWARD COMPATIBILITY
						//
						// The field of varchar created by v3 or early
						// is sorted with NO PAD.
						//
						// It is difficult to resort the data with PAD SPACE.
						// So this case is treated as a KNOWN BUG.
						// And the below procedure is for simulation of v3.
						//
						// order of data:
						//		abc < abc\n < abca
						//
						// original condition:
						//		f > 'abc '
						//
						// modified condition: <- The below procedure
						//		f > 'abc'
						//
						// result:
						//		'abc\n', 'abca'
						//
						// If the condition is NOT modified,
						// the result is only 'abca'.
						// It's different from the result using v3.
						//
						if (m_cFileID.checkVersion(FileID::Version4) == false &&
							m_cFileID.isFixed(field) == false &&
							pData->getType() == Common::DataType::String &&
							eMatch_ != TreeNodeInterface::Like)
						{
							const Common::StringData* pStringData
								= _SYDNEY_DYNAMIC_CAST(
									const Common::StringData*, pData);
							if (pStringData->getCollation()
								!= Common::Collation::Type::NoPad)
							{
								// NO PADではないので、末尾の空白を無視する
								const ModUnicodeString& v =
									pStringData->getValue();
								const ModUnicodeChar* s = v;
								const ModUnicodeChar* p = s + v.getLength();

								// 末尾からスペースを調べる
								while (s != p)
								{
									--p;
									if (*p != 0x20)
									{
										++p;
										break;
									}
								}

								// スペースを除いた範囲をコピーする
								cstrValue_.allocateCopy(
									s,
									static_cast<ModSize>(p - s));
							}
						}
					}
					else if (m_vecKeyType[field] == Data::Type::Decimal)
					{
						ModSize iPrecision = m_cFileID.getInteger(
							_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
								FileCommon::FileOption::FieldLength::Key,
								m_vecKeyPosition[field]));
						ModSize iScale = m_cFileID.getInteger(
							_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
								FileCommon::FileOption::FieldFraction::Key,
								m_vecKeyPosition[field]));

						isValid_ = Data::Decimal::round(
							*pData, eMatch_, iPrecision, iScale, cstrValue_);
					}
					else
					{
						cstrValue_ = pValue->getValue();
						isValid_ = true;
					}
				}
			}
		}
	}
	return field;
}

//
//	FUNCTION private
//	Btree2::Condition::alternateTerm -- Alternate two terms.
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pFirst_
//		First term
//	const LogicalFile::TreeNodeInterface* pSecond_
//		Second term
//	LogicalFile::TreeNodeInterface::Type& eMatch_
//		Comparison operator
//
//	RETURN
//
//	EXCEPTIONS
//
void
Condition::alternateTerm(const LogicalFile::TreeNodeInterface*& pFirst_,
						 const LogicalFile::TreeNodeInterface*& pSecond_,
						 LogicalFile::TreeNodeInterface::Type& eMatch_)
{
	using namespace LogicalFile;
	
	switch (eMatch_)
	{
	case TreeNodeInterface::Equals:
	case TreeNodeInterface::NotEquals:
		break;
	case TreeNodeInterface::GreaterThan:
		eMatch_ = TreeNodeInterface::LessThan;
		break;
	case TreeNodeInterface::GreaterThanEquals:
		eMatch_ = TreeNodeInterface::LessThanEquals;
		break;
	case TreeNodeInterface::LessThan:
		eMatch_ = TreeNodeInterface::GreaterThan;
		break;
	case TreeNodeInterface::LessThanEquals:
		eMatch_ = TreeNodeInterface::GreaterThanEquals;
		break;
	case TreeNodeInterface::Like:
		// Not alternate terms.
		// The right operand has to be a pattern,
		// so 'pSecond' being Field is NOT supported.
		return;
	default:
		// The others are alternated.
		break;
	}

	const TreeNodeInterface* p = pFirst_;
	pFirst_ = pSecond_;
	pSecond_ = p;
}

//
//	FUNCTION private
//	Btree2::Condition::checkNoPadSortOrder --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	bool
//		Whether the sort order of the field is NO PAD
//
//	EXCEPTIONS
//
bool
Condition::checkNoPadSortOrder(Data::Type::Value eType_,
							   bool bFixedField_, bool bNoPadKey_) const
{
	bool result = false;

	switch (eType_)
	{
	case Data::Type::UnicodeString:
	case Data::Type::CharString:
		if (bFixedField_ == true && bNoPadKey_ == true)
		{
			// When the field is fixed one,
			// the PadSpace sort order is equal to the NoPad one.
			// And the collation of the key is NO PAD.
			// So it had better to treat the order as the NoPad one.
			result = true;
		}
		break;
	case Data::Type::NoPadUnicodeString:
	case Data::Type::NoPadCharString:
		result = true;
		break;
	default:
		break;
	}

	return result;
}

//
//	FUNCTION private
//	Btree2::Condition::setEqualsParseValue --
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
Condition::setEqualsParseValue(ParseValue* pNew_,
							   ParseValue*& pMain_,
							   ParseValue*& pOther_,
							   bool bNoPadField_,
							   bool bNoPadKey_)
{
	using namespace LogicalFile;
	
	if (pMain_ != 0)
	{
		if (pMain_->m_eType != TreeNodeInterface::Equals
			&& pMain_->m_eType != TreeNodeInterface::EqualsToNull)
		{
			// equal検索が一番強いのでそれ以外の検索条件はvecOther_へ
			ParseValue* p = pMain_;
			while (p->m_pNext) p = p->m_pNext;
			p->m_pNext = pOther_;
			pOther_ = pMain_;

			setEqualsParseValueWithSortOrder(pNew_, pMain_, pOther_,
											 bNoPadField_, bNoPadKey_);
		}
		else
		{
			// もうequalがあるのでvecOther_へ

			pNew_->m_pNext = pOther_;
			pOther_ = pNew_;
		}
	}
	else
	{
		// 初めて
		setEqualsParseValueWithSortOrder(pNew_, pMain_, pOther_,
										 bNoPadField_, bNoPadKey_);
	}
}

//
//	FUNCTION private
//	Btree2::Condition::setEqualsParseValueWithSortOrder --
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
Condition::setEqualsParseValueWithSortOrder(ParseValue* pNew1_,
											ParseValue*& pMain_,
											ParseValue*& pOther_,
											bool bNoPadField_,
											bool bNoPadKey_)
{
	; _SYDNEY_ASSERT(pNew1_ != 0 &&
					 pNew1_->m_eType == LogicalFile::TreeNodeInterface::Equals);
	
	if (bNoPadField_ == false && bNoPadKey_ == true)
	{
		// The sort order of the field is PAD SPACE,
		// But the collation of the key is NO PAD.
		// So, search with PAD SPACE and then compare with NO PAD.
		
		// Set the ParseValue with PAD SPACE in the main condition.
		ParseValue* pNew2 = new ParseValue;
		pNew2->m_eType = pNew1_->m_eType;
		pNew2->m_cValue = pNew1_->m_cValue;
		pNew2->m_usOptionalChar = getPaddingChar(bNoPadField_);
		pMain_ = pNew2;
		
		// Set the ParseValue with NO PAD in the other condition.
		pNew1_->m_pNext = pOther_;
		pOther_ = pNew1_;
	}
	else
	{
		pMain_ = pNew1_;
	}
}

//
//	FUNCTION private
//	Btree2::Condition::setLessThanParseValue --
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
Condition::setLessThanParseValue(ParseValue*	pNew_,
								 ParseValue*&	pMain_,
								 ParseValue*&	pOther_,
								 bool bNoPadField_,
								 bool bNoPadKey_)
{
	using namespace LogicalFile;

	if (pMain_ != 0)
	{
		if ((pMain_->m_eType == TreeNodeInterface::GreaterThan
			 || pMain_->m_eType == TreeNodeInterface::GreaterThanEquals)
			&& pMain_->m_pNext == 0)
		{
			// 上限がないので設定
			setLessThanParseValueWithSortOrder(pNew_, pMain_, pOther_,
											   bNoPadField_, bNoPadKey_);
		}
		else
		{
			// もうequalかlessがあるのでpOther_へ
			pNew_->m_pNext = pOther_;
			pOther_ = pNew_;
		}
	}
	else
	{
		// 初めて
		setLessThanParseValueWithSortOrder(pNew_, pMain_, pOther_,
										   bNoPadField_, bNoPadKey_);
	}
}

//
//	FUNCTION private
//	Btree2::Condition::setLessThanParseValueWithSortOrder --
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
Condition::setLessThanParseValueWithSortOrder(ParseValue* pNew1_,
											  ParseValue*& pMain_,
											  ParseValue*& pOther_,
											  bool bNoPadField_,
											  bool bNoPadKey_)
{
	using namespace LogicalFile;

	; _SYDNEY_ASSERT(pNew1_ != 0 &&
					 (pNew1_->m_eType == TreeNodeInterface::LessThan ||
					  pNew1_->m_eType == TreeNodeInterface::LessThanEquals));
	
	if (bNoPadField_ == false && bNoPadKey_ == true)
	{
		// The sort order of f is PAD SPACE,
		// The collation of the key is NO PAD,
		// f < 'abc\nxyz'
		// -> f <= 'abc' with PAD SPACE & f < 'abc\nxyz' with NO PAD.
		
		// Set the ParseValue with PAD SPACE in the main condition.
		ParseValue* pNew2 = new ParseValue;
		pNew2->m_eType = TreeNodeInterface::LessThanEquals;
		const ModUnicodeString& s = pNew1_->m_cValue;
		const ModUnicodeChar* p = s;
		const ModUnicodeChar* e = p + s.getLength();
		// Search the position of the character smaller than _usPaddingChar.
		while (p != e)
		{
			if (*p < _usPaddingChar)
				break;
			++p;
		}
		// Copy the string without the tail.
		(pNew2->m_cValue).allocateCopy(s,
									   static_cast<ModSize>(p - s));
		pNew2->m_usOptionalChar = getPaddingChar(bNoPadField_);
		if (pMain_ == 0)
		{
			pMain_ = pNew2;
		}
		else
		{
			; _SYDNEY_ASSERT(pMain_->m_pNext == 0);
			pMain_->m_pNext = pNew2;
		}
		
		// Set the ParseValue with NO PAD in the other condition.
		pNew1_->m_pNext = pOther_;
		pOther_ = pNew1_;
	}
	else
	{
		if (pMain_ == 0)
		{
			pMain_ = pNew1_;
		}
		else
		{
			; _SYDNEY_ASSERT(pMain_->m_pNext == 0);
			pMain_->m_pNext = pNew1_;
		}
	}
}

//
//	FUNCTION private
//	Btree2::Condition::setGreaterThanParseValue --
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
Condition::setGreaterThanParseValue(ParseValue*		pNew_,
									ParseValue*&	pMain_,
									ParseValue*&	pOther_,
									bool bNoPadField_,
									bool bNoPadKey_)
{
	using namespace LogicalFile;

	if (pMain_ != 0)
	{
		if (pMain_->m_eType == TreeNodeInterface::LessThan
			|| pMain_->m_eType == TreeNodeInterface::LessThanEquals)
		{
			setGreaterThanParseValueWithSortOrder(pNew_, pMain_, pOther_,
												  bNoPadField_, bNoPadKey_);
		}
		else
		{
			// もうequalかgreaterがあるのでpOther_へ
			pNew_->m_pNext = pOther_;
			pOther_ = pNew_;
		}
	}
	else
	{
		// 初めて
		setGreaterThanParseValueWithSortOrder(pNew_, pMain_, pOther_,
											  bNoPadField_, bNoPadKey_);
	}
}

//
//	FUNCTION private
//	Btree2::Condition::setGreaterThanParseValueWithSortOrder --
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
Condition::setGreaterThanParseValueWithSortOrder(ParseValue* pNew1_,
												 ParseValue*& pMain_,
												 ParseValue*& pOther_,
												 bool bNoPadField_,
												 bool bNoPadKey_)
{
	using namespace LogicalFile;

	; _SYDNEY_ASSERT(pNew1_ != 0 &&
					 (pNew1_->m_eType == TreeNodeInterface::GreaterThan ||
					  pNew1_->m_eType == TreeNodeInterface::GreaterThanEquals));
	
	if (bNoPadField_ == false && bNoPadKey_ == true)
	{
		// The sort order of f is PAD SPACE,
		// The collation of the key is NO PAD,
		// f > 'abc'
		// -> f > 'abb' with PAD SPACE & f > 'abc' with NO PAD.
		// f > ''
		// -> NOT able to expand the condition.
		
		int position = getPositionOfTraillingSOH(pNew1_->m_cValue);
		if (position != 0)
		{
			// Set the ParseValue with PAD SPACE in the main condition.

			ParseValue* pNew2 = new ParseValue;
			pNew2->m_eType = pNew1_->m_eType;
			pNew2->m_cValue = pNew1_->m_cValue;
			// Expand the range of the condition.
			(pNew2->m_cValue[position - 1])--;
			pNew2->m_usOptionalChar = getPaddingChar(bNoPadField_);

			pNew2->m_pNext = pMain_;
			pMain_ = pNew2;
		}
		// position = 0 means key is '', ^A, ^A^A, ...
		// In such case, the lower condition is no condition.
		
		// Set the ParseValue with NO PAD in the other condition.
		pNew1_->m_pNext = pOther_;
		pOther_ = pNew1_;
	}
	else
	{
		pNew1_->m_pNext = pMain_;
		pMain_ = pNew1_;
	}
}

//
//	FUNCTION private
//	Btree2::Condition::setPrefixMatchParseValue --
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
Condition::setPrefixMatchParseValue(ParseValue* pNew1_,
									ParseValue*& pMain_,
									ParseValue*& pOther_,
									bool bNoPadField_,
									bool bNoPadKey_)
{
	using namespace LogicalFile;
	
	if (pMain_ != 0)
	{
		if (pMain_->m_eType == TreeNodeInterface::Equals
			|| pMain_->m_eType == TreeNodeInterface::EqualsToNull)
		{
			// equal以外の検索条件はvecOther_へ
			ParseValue* p = pMain_;
			while (p->m_pNext) p = p->m_pNext;
			p->m_pNext = pOther_;
			pOther_ = pMain_;

			setPrefixMatchParseValueWithSortOrder(pNew1_, pMain_,
												  bNoPadField_, bNoPadKey_);
		}
		else
		{
			// もうequalかgreaterがあるのでvecOther_へ
			; _TRMEISTER_ASSERT(pNew1_->m_pNext != 0)
			pNew1_->m_pNext->m_pNext = pOther_;
			
			setPrefixMatchParseValueWithSortOrder(pNew1_, pOther_,
												  bNoPadField_, bNoPadKey_);
		}
	}
	else
	{
		// 初めて
		setPrefixMatchParseValueWithSortOrder(pNew1_, pMain_,
											  bNoPadField_, bNoPadKey_);
	}
}

//
//	FUNCTION private
//	Btree2::Condition::setPrefixMatchParseValue --
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
Condition::setPrefixMatchParseValueWithSortOrder(ParseValue* pNew1_,
												 ParseValue*& pParseValue_,
												 bool bNoPadField_,
												 bool bNoPadKey_)
{
	using namespace LogicalFile;
	
	if (bNoPadField_ == false && bNoPadKey_ == true)
	{
		// When the data of the field is sorted with PAD SPACE,
		// the order is here: abb < abc\n < abc < abca < abd.
		// If x >= abc is searched with NO PAD, abc\n is NOT matched.
		// So, like 'abc%' is treated as two conditons:
		// 'abb < x < abd' with PAD SPACE and like 'abc%' with NO PAD.
		
		// For a simple implementation, change the upper condition,
		// but, in fact, NOT need to change the upper condition.
		// ex1: c like 'abc%'
		// abb(PAD SPACE) <= c < abd(NO PAD) and c like abc% (NO PAD)
		// When 'abd' is compared to 'abd\n' with NO PAD,
		// the result is 'abd' < 'abd\n'.
		// It's incorrect in the view of PAD SPACE.
		// But it's no problem.
		// 'abc%' is always smaller than 'abd\t', 'abd\n' and 'abd'.
		// ex2: c like 'abc%' order by c desc
		// This is as same as ex1.
		// ex3: c like 'ab\19%'
		// ab\19(PAD SPACE) <= c < ab\20(NO PAD) and c like ab\19% (NO PAD)
		// When 'ab\20' is compared to 'ab' with NO PAD,
		// the result is 'ab\20' > 'ab'.
		// It's incorrect in the view of PAD SPACE,
		// and cause the search range wide.
		// But it' no problem.
		// The other condition, c like ab\19% (NO PAD), is used too.
		// So the result is correct.
		// ex4: c like 'ab\19%' order by c desc
		// This is as same as ex3.

		// Cheange how to compare the strings.
		pNew1_->m_usOptionalChar = getPaddingChar(bNoPadField_);
		; _TRMEISTER_ASSERT(pNew1_->m_pNext != 0);
		pNew1_->m_pNext->m_usOptionalChar = pNew1_->m_usOptionalChar;
		
		int position = getPositionOfTraillingSOH(pNew1_->m_cValue);
		if (position != 0)
		{
			// Change the lower condition.
			; _TRMEISTER_ASSERT(
				pNew1_->m_eType	= TreeNodeInterface::GreaterThanEquals);
			pNew1_->m_eType = TreeNodeInterface::GreaterThan;
			(pNew1_->m_cValue[position - 1])--;
		}
		else
		{
			// Remove the lower condition.
			// See getPositionOfTraillingSOH for details.
			ParseValue* pNew2 = pNew1_->m_pNext;
			pNew1_->m_pNext = 0;
			delete pNew1_;
			pNew1_ = pNew2;
		}
	}
	// When the data is also sorted with NO PAD,
	// it is treated as 'abc <= x < abd' simply.

	pParseValue_ = pNew1_;
}

//
//	FUNCTION private
//	Btree2::Condition::setLikeParseValue --
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
Condition::setLikeParseValue(ModUnicodeString& cstrValue_,
							 ModUnicodeChar usEscape_,
							 ParseValue*& pOther_)
{
	using namespace LogicalFile;
	
	ParseValue* pLike = new ParseValue;
	pLike->m_eType = TreeNodeInterface::Like;
	pLike->m_cValue = cstrValue_;
	pLike->m_usOptionalChar = usEscape_;
	
	pLike->m_pNext = pOther_;
	pOther_ = pLike;
}

//
//	FUNCTION private
//	Btree2::Condition::setUnknownParseValue --
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<ParseValue*>& vecMain_
//		主検索条件
//	ModVector<ParseValue*>& vecOthrer_
//		副検索条件
//
//	RETURN
//
//	EXCEPTIONS
//
void
Condition::setUnknownParseValue(ModVector<ParseValue*>& vecMain_,
								ModVector<ParseValue*>& vecOther_)
{
	ParseValue* pNew = new ParseValue;
	pNew->m_eType = LogicalFile::TreeNodeInterface::Unknown;
	_freeVector(vecMain_);
	_freeVector(vecOther_);
	vecMain_[0] = pNew;
}

//
//	FUNCTION private
//	Btree2::Condition::setToOpenOption -- OpenOptionに設定する
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<ParseValue*>& vecMain_
//		主検索条件
//	ModVector<ParseValue*>& vecOthrer_
//		副検索条件
//	int n_
//		何番目の検索条件か
//	LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	bool
//		B木で実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Condition::setToOpenOption(ModVector<ParseValue*>& vecMain_,
						   ModVector<ParseValue*>& vecOther_,
						   int n_,
						   LogicalFile::OpenOption& cOpenOption_)
{
	using namespace LogicalFile;
	
	if (vecMain_[0] == 0)
		// 先頭キーに条件がないのでエラー
		return false;

	ModUnicodeOstrStream cLower;
	ModUnicodeOstrStream cUpper;
	ModUnicodeOstrStream cOther;

	int n = 0;
	bool next = true;
	bool eq = true;
	bool seteq = false;
	while (n < static_cast<int>(vecMain_.getSize())
		   && vecMain_[n] != 0 && next == true)
	{
		ParseValue* p = vecMain_[n];
		switch (p->m_eType)
		{
		case TreeNodeInterface::Equals:
		case TreeNodeInterface::EqualsToNull:
		case TreeNodeInterface::Unknown:
			p->putStream(cLower);
			p->putStream(cUpper);
			break;
		case TreeNodeInterface::GreaterThan:
		case TreeNodeInterface::GreaterThanEquals:
			p->putStream(cLower);
			if (p->m_pNext == 0)
			{
				next = false;
			}
			else
			{
				p = p->m_pNext;
				p->putStream(cUpper);
			}
			eq = false;
			break;
		case TreeNodeInterface::LessThan:
		case TreeNodeInterface::LessThanEquals:
			p->putStream(cUpper);
			next = false;
			eq = false;
			break;
		default:
			return false;
		}

		if (eq == false && seteq == false)
		{
			// equal検索の最大フィールド数
			cOpenOption_.setInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(Key::EqualFieldNumber), n);
			seteq = true;
		}

		n++;
	}

	if (seteq == false)
	{
		// equal検索の最大フィールド数 -- すべてがeqの場合はここに来る
		cOpenOption_.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(Key::EqualFieldNumber), n);
		seteq = true;
	}

	for (int i = 0; i < static_cast<int>(m_vecKeyType.getSize()); ++i)
	{
		cOther << "%" << i << '(';
		ParseValue* p = vecOther_[i];
		while (p)
		{
			p->putStream(cOther);
			p = p->m_pNext;
		}
		if (i >= n)
		{
			p = vecMain_[i];
			while (p)
			{
				p->putStream(cOther);
				p = p->m_pNext;
			}
		}
		cOther << ')';
	}

	ModUnicodeOstrStream cStream;
	ModUnicodeString cstrLower = cLower.getString();
	ModUnicodeString cstrUpper = cUpper.getString();
	ModUnicodeString cstrOther = cOther.getString();
	if (cstrUpper == cstrLower)
	{
		// 完全一致
		cStream << "#eq(" << cstrLower << ")";
	}
	else
	{
		// 範囲指定
		if (cstrLower.getLength())
			cStream << "#ge(" << cstrLower << ")";
		if (cstrUpper.getLength())
			cStream << "#le(" << cstrUpper << ")";
	}
	// その他条件
	if (cstrOther.getLength())
		cStream << "#ot(" << cstrOther << ")";

	// OpenOptionに設定する
	cOpenOption_.setString(
		_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(Key::Condition, n_),
		cStream.getString());

	return true;
}

//
//	FUNCTION private
//	Btree2::Condition::setLowerData -- 下限条件を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar*& p_
//		検索条件文字列
//	LogicalFile::OpenOption::OpenMode::Value eMode_
//		オープンモード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Condition::setLowerData(const ModUnicodeChar*& p_,
						LogicalFile::OpenOption::OpenMode::Value eMode_)
{
	using namespace LogicalFile;

	if (*p_ == 0)
		return;
	const ModUnicodeChar* p = p_;
	p++;
	if (*p == 'e')
	{
		// #eq(....)
		Common::DataArrayData cArray;
		
		p += 3;
		int n = 0;
		ModVector<Data::Type::Value> vecType;
		while (*p != ')')
		{
			ModUnicodeString cstrValue;
			ModUnicodeChar usOptionalChar = 0;
			bool dummy = false;
			TreeNodeInterface::Type eType
				= ParseValue::getStream(p, cstrValue, usOptionalChar, dummy);
			if (eType == TreeNodeInterface::Unknown)
			{
				// 1件もヒットしない
				m_bValid = false;
				return;
			}
			Data::Type::Value eKeyType =
				getKeyType(m_vecKeyType[n], usOptionalChar);
			Common::Data::Pointer pData = createCommonData(
					eKeyType, m_vecKeyPosition[n], cstrValue, eType);
			if (eMode_ == LogicalFile::OpenOption::OpenMode::Search)
			{
				// Fetchモードなので、オープンオプションの条件はすべて
				// その他条件に格納する
				m_vecOtherCondition.pushBack(
					makeCond(eType, *pData, n, usOptionalChar));
			}
			else
			{
				// 通常の検索なので、下限に設定する
				cArray.pushBack(pData);
				vecType.pushBack(eKeyType);
			}
			n++;
		}
		p++;

		if (eMode_ == LogicalFile::OpenOption::OpenMode::Read)
		{
			// メモリーイメージを得る
			Data cData;
			cData.setType(vecType, 0, m_bHeader);
			m_cLowerData.m_nullBitmap = 0;
			ModSize size = cData.getSize(cArray);
			m_cLowerData.allocate(size * sizeof(ModUInt32));
			if (m_bHeader)
				cData.dump(m_cLowerData.m_pBuffer, cArray);
			else
				cData.dump(m_cLowerData.m_pBuffer, cArray,
						   m_cLowerData.m_nullBitmap);

			// 比較クラス
			bool isUnique = (vecType.getSize() == m_vecKeyType.getSize());
			m_cLowerData.m_cCompare.setType(vecType, isUnique, m_bHeader);
			m_cLowerData.m_eType = TreeNodeInterface::Equals;

			// 上限にも設定する
			m_cUpperData = m_cLowerData;
		}

		p_ = p;
	}
	else if (*p == 'g')
	{
		// #ge(....)
		Common::DataArrayData cArray;
		bool isOnlyGT = true;
		
		p += 3;
		int n = 0;
		ModVector<Data::Type::Value> vecType;
		while (*p != ')')
		{
			ModUnicodeString cstrValue;
			ModUnicodeChar usOptionalChar = 0;
			bool dummy = false;
			TreeNodeInterface::Type eType
				= ParseValue::getStream(p, cstrValue, usOptionalChar, dummy);
			if (eType != TreeNodeInterface::GreaterThan)
				isOnlyGT = false;
			Data::Type::Value eKeyType =
				getKeyType(m_vecKeyType[n], usOptionalChar);
			Common::Data::Pointer pData = createCommonData(
				eKeyType, m_vecKeyPosition[n], cstrValue, eType);
			if (eMode_ == LogicalFile::OpenOption::OpenMode::Search)
			{
				// Fetchモードなので、オープンオプションの条件はすべて
				// その他条件に格納する
				m_vecOtherCondition.pushBack(
					makeCond(eType, *pData, n, usOptionalChar));
			}
			else
			{
				// 通常の検索なので、下限に設定する
				cArray.pushBack(pData);
				vecType.pushBack(eKeyType);
				if (n != 0 || eType == TreeNodeInterface::GreaterThan)
				{
					m_vecOtherCondition.pushBack(
						makeCond(eType, *pData, n, usOptionalChar));
				}
			}
			n++;
		}
		p++;

		if (eMode_ == LogicalFile::OpenOption::OpenMode::Read)
		{
			// メモリーイメージを得る
			Data cData;
			cData.setType(vecType, 0, m_bHeader);
			m_cLowerData.m_nullBitmap = 0;
			ModSize size = cData.getSize(cArray);
			m_cLowerData.allocate(size * sizeof(ModUInt32));
			if (m_bHeader)
				cData.dump(m_cLowerData.m_pBuffer, cArray);
			else
				cData.dump(m_cLowerData.m_pBuffer, cArray,
						   m_cLowerData.m_nullBitmap);

			// 比較クラス
			m_cLowerData.m_cCompare.setType(vecType, false, m_bHeader);

			if (isOnlyGT == true)
				m_cLowerData.m_eType = TreeNodeInterface::GreaterThan;
			else
				m_cLowerData.m_eType = TreeNodeInterface::GreaterThanEquals;
		}

		p_ = p;
	}
}

//
//	FUNCTION private
//	Btree2::Condition::setUpperData -- 上限条件を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar*& p_
//		検索条件文字列
//	LogicalFile::OpenOption::OpenMode::Value eMode_
//		オープンモード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Condition::setUpperData(const ModUnicodeChar*& p_,
						LogicalFile::OpenOption::OpenMode::Value eMode_)
{
	using namespace LogicalFile;

	if (*p_ == 0 || m_bValid == false)
		return;
	const ModUnicodeChar* p = p_;
	p++;
	if (*p == 'l')
	{
		// #le(....)
		Common::DataArrayData cArray;
		bool isOnlyLT = true;
		
		p += 3;
		int n = 0;
		ModVector<Data::Type::Value> vecType;
		while (*p != ')')
		{
			ModUnicodeString cstrValue;
			ModUnicodeChar usOptionalChar = 0;
			bool bNormalized = false;
			TreeNodeInterface::Type eType
				= ParseValue::getStream(p, cstrValue,
										usOptionalChar, bNormalized);
			if (eType != TreeNodeInterface::LessThan)
				isOnlyLT = false;
			Data::Type::Value eKeyType =
				getKeyType(m_vecKeyType[n], usOptionalChar);
			Common::Data::Pointer pData = createCommonData(
				eKeyType, m_vecKeyPosition[n], cstrValue, eType, bNormalized);
			if (eMode_ == LogicalFile::OpenOption::OpenMode::Search)
			{
				// Fetchモードなので、オープンオプションの条件はすべて
				// その他条件に格納する
				m_vecOtherCondition.pushBack(
					makeCond(eType, *pData, n, usOptionalChar));
			}
			else
			{
				// 通常の検索なので、上限に設定する
				cArray.pushBack(pData);
				vecType.pushBack(eKeyType);
				
				if (n != 0 || eType == TreeNodeInterface::LessThan ||
					(m_cFileID.isTopNull() && m_cLowerData.m_pBuffer == 0))
				{
					// If lower condition does not exist,
					// the data of null will be taken.
					// Because the data is orderd in the top.
					// The third condition is used to remove the data of null.
					m_vecOtherCondition.pushBack(
						makeCond(eType, *pData, n, usOptionalChar));
				}
			}
			n++;
		}
		p++;

		if (eMode_ == LogicalFile::OpenOption::OpenMode::Read)
		{
			// メモリーイメージを得る
			Data cData;
			cData.setType(vecType, 0, m_bHeader);
			m_cUpperData.m_nullBitmap = 0;
			ModSize size = cData.getSize(cArray);
			m_cUpperData.allocate(size * sizeof(ModUInt32));
			if (m_bHeader)
				cData.dump(m_cUpperData.m_pBuffer, cArray);
			else
				cData.dump(m_cUpperData.m_pBuffer, cArray,
						   m_cUpperData.m_nullBitmap);

			// 比較クラス
			m_cUpperData.m_cCompare.setType(vecType, false, m_bHeader);

			if (isOnlyLT == true)
				m_cUpperData.m_eType = TreeNodeInterface::LessThan;
			else
				m_cUpperData.m_eType = TreeNodeInterface::LessThanEquals;
		}
		
		p_ = p;
	}
}

//
//	FUNCTION private
//	Btree2::Condition::setOtherData -- その他条件を設定する
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
			// %n(...)
			p++;
			int n = static_cast<int>(*p) - '0';
			p += 2;

			while (*p != ')')
			{
				ModUnicodeString cstrValue;
				ModUnicodeChar usOptionalChar = 0;
				bool bNormalized = false;
				TreeNodeInterface::Type eType
					= ParseValue::getStream(p, cstrValue,
											usOptionalChar, bNormalized);
				//cstrValue may be null
				Data::Type::Value eKeyType =
					getKeyType(m_vecKeyType[n], usOptionalChar);
				Common::Data::Pointer pData	= createCommonData(
					eKeyType, m_vecKeyPosition[n],
					cstrValue, eType, bNormalized);
				m_vecOtherCondition.pushBack(
					makeCond(eType, *pData, n, usOptionalChar));
			}

			p++;
		}
		
		p_ = p;
	}
}

//
//	FUNCTION private
//	Btree2::Condition::createCommonData -- Common::Dataを作成する
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::Data::Type::Value eType_
//		データ型
//	int iPosition_
//		The position of the key (for decimal type)
//	ModUnicodeString& cstrValue_
//		文字列データ
//	LogicalFile::TreeNodeInterface::Type eMatch_
//		Comparison operator
//	bool bAlreadyNormalized_
//		True means that the data has already been normlaized.
//
//	RETURN
//	Common::Data::Pointer
//		データオブジェクト
//
//	EXCEPTIONS
//
Common::Data::Pointer
Condition::createCommonData(Data::Type::Value eType_,
							int iPosition_, 
							ModUnicodeString& cstrValue_,
							LogicalFile::TreeNodeInterface::Type eMatch_,
							bool bAlreadyNormalized_)
{
	Common::Data::Pointer pData;

	if (eMatch_ == LogicalFile::TreeNodeInterface::EqualsToNull)
	{
		// Not need to check NotNull, because it is not supported in Btree2.
		pData = Common::NullData::getInstance();
		return pData;
	}

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
			ModSize iPrecision = m_cFileID.getInteger(
				_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
					FileCommon::FileOption::FieldLength::Key, iPosition_));
			ModSize iScale = m_cFileID.getInteger(
				_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
					FileCommon::FileOption::FieldFraction::Key, iPosition_));
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
			if (m_isNormalized == true && bAlreadyNormalized_ == false)
			{
				// 正規化する
				ModUnicodeString temp;
				Utility::CharTrait::normalize(
					cstrValue_, m_cFileID.getNormalizingMethod(), temp);
				pData = new Common::StringData(temp);
			}
			else
			{
				pData = new Common::StringData(cstrValue_);
			}
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
//	Btree2::Condition::makeCond -- Cond構造体を作成する
//
//	NOTES
//
//	ARGUMENTS
//	LogicalFile::TreeNodeInterface::Type eType_
//		比較タイプ
//	const Common::Data& cData_
//		データ
//	int n_
//		フィールド番号
//	ModUnicodeChar usOptionalChar_
//		Optional character
//
//	RETURN
//	Btree2::Condition::Cond
//		Cond構造体
//
//	EXCEPTIONS
//
Condition::Cond
Condition::makeCond(LogicalFile::TreeNodeInterface::Type eType_,
					const Common::Data& cData_, int n_,
					ModUnicodeChar usOptionalChar_)
{
	Cond cCond;
	cCond.m_eType = eType_;
	cCond.m_iFieldID = n_;
	if (eType_ != LogicalFile::TreeNodeInterface::EqualsToNull)
	{
		// Not need to check NotNull, because it is not supported in Btree2.
		cCond.m_usOptionalChar = usOptionalChar_;
		Data::Type::Value eKeyType =
			getKeyType(m_vecKeyType[n_], usOptionalChar_);
		ModSize size = Data::getSize(cData_, eKeyType);
		cCond.allocate(size * sizeof(ModUInt32));
		ModUInt32* p = cCond.m_pBuffer;
		Data::dump(p, cData_, eKeyType);
	}
	return cCond;
}

//
//	FUNCTION private
//	Btree2::Condition::getPaddingChar --
//
//	NOTES
//	The returnd character may be casted to the type of unsigned char
//	in Data::NoPadCharString::like().
//	So the character MUST belongs to the ascii code.
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModUnicodeChar
Condition::getPaddingChar(bool bNoPad_) const
{
	return (bNoPad_ == true) ? 0 : _usPaddingChar;
}

//
//	FUNCTION private
//	Btree2::Condition::getKeyType --
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
//	FUNCTION private
//	Btree2::Condition::getPositionOfTraillingSOH --
//
//	NOTES
//	SOH: Start of Heading, and its hexadecimal value is 0x01.
//
//		cstrString_	int
//		-----------------
//		\0			0
//		\1\0		0
//		\1\1\0		0
//
//		a\0			1
//		a\1\0		1
//		a\1\1\0		1
//
//		a\1a\1\0	3
//		abc\0		3
//
//	This function is used in the lower condition.
//	The convert result is here:
//	ex1: x >= abc(NO PAD) -> x > abb(PAD SPACE)
//	ex2: x >= abc\19(NO PAD) -> x > abc\18(PAD SPACE)
//	ex3: x >= abc\1(NO PAD) -> x > abb(PAD SPACE)
//	If x >= abc\0(PAD SPACE) is used instead of abb(PAD SPACE),
//	abc\2 is not included in the result.
//	ex4: x >= abc\1\1(NO PAD) -> x > abb(PAD SPACE)
//	If x >= abc\1\0(PAD SPACE) is used instead of abb(PAD SPACE),
//	abc\1\2 is not included in the result.
//	ex5: x >= \1(NO PAD) -> no condition
//	If x >= \0(PAD SPACE) is used instead of no condition,
//	\2 is not included in the result.
//
//	ARGUMENTS
//	ModUnicodeString& cstrString_
//
//	RETURN
//	int [0-base]
//		When a trailling SOH does NOT exist, return last position.
//		
//	EXCEPTIONS
//
int
Condition::getPositionOfTraillingSOH(const ModUnicodeString& cstrString_) const
{
	const ModUnicodeChar* s = cstrString_;
	const ModUnicodeChar* p = s + cstrString_.getLength();
	while (s != p)
	{
		--p;
		if (*p != _SOHChar)
		{
			++p;
			break;
		}
	}
	return static_cast<int>(p - s);
}

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
