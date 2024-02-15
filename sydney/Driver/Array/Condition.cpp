// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Condition.cpp --
// 
// Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"

#include "Array/Condition.h"
#include "Array/FileID.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/DateTimeData.h"
#include "Common/DecimalData.h"
#include "Common/LanguageData.h"
#include "Common/NullData.h"
#include "Common/StringData.h"
#include "Exception/BadArgument.h"
#include "Exception/InvalidEscape.h"
#include "FileCommon/FileOption.h"
#include "FileCommon/OpenOption.h"
#include "Utility/CharTrait.h"

#include "ModUnicodeString.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_ARRAY_USING

namespace
{
	//
	//	FUNCTION local
	//	_$$::_paddingChar
	//
	ModUnicodeChar _paddingChar = 0x20;
	
	//
	//	FUNCTION local
	//	_$$::_SOHChar
	//
	ModUnicodeChar _SOHChar = 0x01;
}

//
//	FUNCTION public
//	Array::Condition::Condition -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Array::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Condition::Condition(const FileID& cFileID_)
	: m_cFileID(cFileID_), m_eKeyType(cFileID_.getKeyType()),
	  m_isNormalized(cFileID_.isNormalized()), m_bValid(true),
	  m_bFetch(false), m_bFirstFetch(true),
	  m_bNoPadFieldForFetch(false), m_bNoPadKeyForFetch(false),
	  m_eTreeType(Tree::Type::Undefined)
{
}

//
//	FUNCTION public
//	Array::Condition::~Condition -- デストラクタ
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
//	Array::Condition::getSearchParameter
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

	int iOpenMode = FileCommon::OpenOption::OpenMode::Unknown;
	if (pCondition_)
	{
		if (pCondition_->getType() == TreeNodeInterface::Fetch)
		{
			// Set the fetch condition.
			if (setFetchCondition(pCondition_, cOpenOption_) == false)
			{
				return false;
			}
			iOpenMode = FileCommon::OpenOption::OpenMode::Search;
		}
		else
		{
			// Set the condition
			if (setTreeCondition(pCondition_, cOpenOption_) == false)
			{
				if (setVerifyTreeCondition(pCondition_, cOpenOption_) == true)
				{
					cOpenOption_.setBoolean(
						_SYDNEY_OPEN_PARAMETER_KEY(Key::Verify),
						true);
				}
				else
				{
					return false;
				}
			}
			iOpenMode = FileCommon::OpenOption::OpenMode::Read;
		}
	}
	else
	{
		// pCondition_==0 means scan and Array does NOT support it.
		return false;
	}

	// Set OpenMode.
	cOpenOption_.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key),
		iOpenMode);

	// オブジェクトをドライバ側で保持しない
	cOpenOption_.setBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::CacheAllObject::Key),
		false);

	return true;
}

//
//	FUNCTION public
//	Array::Condition::setOpenOption -- オープンオプションを設定する
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
	m_cLowerData.clear();
	m_cUpperData.clear();
	m_vecOtherData.clear();
	m_bValid = true;

	// Get OpenMode.
	LogicalFile::OpenOption::OpenMode::Value eMode
		= static_cast<LogicalFile::OpenOption::OpenMode::Value>(
			cOpenOption_.getInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(
					FileCommon::OpenOption::OpenMode::Key)));

	// Get the string of condition.
	ModUnicodeString cstrString = cOpenOption_.getString(
		_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(Key::Condition, iNumber_));
	const ModUnicodeChar* p = cstrString;
	if (*p == 0)
	{
		m_bValid = false;
		return;
	}
	
	// Get tree type. The format: %Tree::Type::Value(...)
	; _SYDNEY_ASSERT(*p == '%');
	int iTreeType = 0;
	while (*++p != '(')
	{
		iTreeType = iTreeType * 10 + static_cast<int>(*p) - '0';
	}
	; _SYDNEY_ASSERT(*p == '(');
	++p;
	m_eTreeType = static_cast<Tree::Type::Value>(iTreeType);
	if (m_eTreeType == Tree::Type::Undefined)
	{
		// The type of condition is unknown.
		m_bValid = false;
		return;
	}

	// Set the conditions.
	if (eMode != LogicalFile::OpenOption::OpenMode::Search)
	{
		// Search is for the case of fetch.
		// fetch's condition is set in m_cLowerData and m_cUpperData.
		// So these condition is set in m_vecOtherData.

		setCond(m_cLowerData, p);
		setCond(m_cUpperData, p);
	}

	// Get the other conditions.
	while (*p != ')')
	{
		Cond cCond;
		if (setCond(cCond, p) == true)
		{
			m_vecOtherData.pushBack(cCond);
		}
	}

	// Only ')' is reminded in cstrValue.
}

//
//	FUNCTION public
//	Array::Condition::setFetchKey -- fetchのキーを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cKey_
//		fetchのキー
//		Key consists of one Value and not include the other fields.
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Condition::setFetchKey(const Common::DataArrayData& cKey_)
{
	using namespace LogicalFile;

	// Check the count of the key.
	if (cKey_.getCount() != 1)
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// Check the collations.
	if (m_bFirstFetch == true)
	{
		// First time to fetch.
		m_bFirstFetch = false;

		; _SYDNEY_ASSERT(m_bNoPadKeyForFetch == false);
		; _SYDNEY_ASSERT(m_bNoPadFieldForFetch == false);
		const Common::Data& cTemp = *cKey_.getElement(0).get();
		if (cTemp.isNull() == false &&
			cTemp.getType() == Common::DataType::String)
		{
			// Check the collation in the case of the string type.
			const Common::StringData& cStringData
				= _SYDNEY_DYNAMIC_CAST(const Common::StringData&, cTemp);
			if (cStringData.getCollation() == Common::Collation::Type::NoPad)
			{
				m_bNoPadKeyForFetch = true;
			}
		}
		m_bNoPadFieldForFetch = checkNoPadSortOrder(
			m_eKeyType, m_cFileID.isFixed(), m_bNoPadKeyForFetch);
	}
	else
	{
		// Second time or later.
		
		// This instance is reused in some cases.
		// So remove the conditions of the previous fetch.
		if (m_bNoPadFieldForFetch == false && m_bNoPadKeyForFetch == true &&
			m_bValid == true)
		{
			// [NOTE] PAD SPACE 列を NO PAD キーでfetchしており、
			//  m_bValid==trueなので、前回はnullではなかったので、
			//  その他条件が追加されている。
			; _SYDNEY_ASSERT(m_vecOtherData.getSize() >= 1);
			m_vecOtherData.popBack();
		}
	}

	// Set the conditions. See setEqualsStream.
	
	Common::Data::Pointer pData = cKey_.getElement(0);
	if (pData->isNull() == false)
	{
		// Set tree type.
		m_eTreeType = Tree::Type::Data;
		
		// First, set Upper condition.
		// The reason is why it is reused to initialze Lower and Other conditions.
		if (m_cFileID.isNormalized() == true)
		{
			; _SYDNEY_ASSERT(pData->getType() == Common::DataType::String);
			m_cFileID.normalizeOneData(pData);
		}
		ModUnicodeChar usPadding =
			getPaddingChar(m_bNoPadFieldForFetch || m_bNoPadKeyForFetch);
		Data::Type::Value eKeyType = getKeyType(m_eKeyType, usPadding);
		m_cUpperData.clear();
		dumpCommonData(m_cUpperData.m_pBuffer, eKeyType, *pData);
		m_cUpperData.m_eMatch = TreeNodeInterface::LessThanEquals;
		m_cUpperData.m_OptionalChar = usPadding;

		// Second, set Lower and Other conditions.
		m_cLowerData.clear();
		if (m_bNoPadFieldForFetch == false && m_bNoPadKeyForFetch == true)
		{
			// ソート順と比較方法が不一致の場合

			// ここに来るのは、キーがNO PADになりうる文字列型の時だけ。
			; _TRMEISTER_ASSERT(pData->getType() == Common::DataType::String);
			
			// Lower condition with PAD SPACE.
			ModUnicodeChar usExpand = getPaddingChar(m_bNoPadFieldForFetch);
			Data::Type::Value eKeyType = getKeyType(m_eKeyType, usExpand);
			dumpCommonData(m_cLowerData.m_pBuffer, eKeyType, *pData);
			m_cLowerData.m_OptionalChar = usExpand;
			m_cLowerData.m_eMatch = TreeNodeInterface::GreaterThanEquals;

			// Other condition.
			Cond cCond = m_cUpperData;
			cCond.m_eMatch = TreeNodeInterface::Equals;
			m_vecOtherData.pushBack(cCond);
		}
		else
		{
			// Lower condition
			m_cLowerData = m_cUpperData;
			m_cLowerData.m_eMatch = TreeNodeInterface::GreaterThanEquals;
		}

		// [NOTE] 前回のfetchでfalseに設定されているかもしれないのでtrueにする。
		m_bValid = true;
	}
	else
	{
		// [NOTE] 現在のArrayのfetchは与えられたキーと等しいデータを返すだけ。
		//  したがって、キーがNULLだった場合は等しいデータが存在しないので、
		//  1件も返せない。
		//  ArrayFileで無駄な検索をしないために、条件をinvalidにしておく。
		m_bValid = false;
	}
}

//
//	FUNCTION public
//	Array::Condition::isUpperConditionSatisfied
//		-- Check whether the key satisfies the upper condition.
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pBuffer_
//		比較するバッファ
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
Condition::isUpperConditionSatisfied(const ModUInt32* pBuffer_)
{
	using namespace LogicalFile;

	bool result = false;
	
	if (m_cUpperData.m_eMatch == TreeNodeInterface::Undefined)
	{
		// Return true when Upper condition is NOT specified.
		result = true;
	}
	else
	{
		; _SYDNEY_ASSERT(
			m_cUpperData.m_eMatch == TreeNodeInterface::LessThan ||
			m_cUpperData.m_eMatch == TreeNodeInterface::LessThanEquals);

		Data::Type::Value eKeyType = getKeyType(
			m_eKeyType, m_cUpperData.m_OptionalChar);
		const ModUInt32* pBuffer2 = m_cUpperData.m_pBuffer;
		int tmp = Compare::compare(pBuffer_, pBuffer2, eKeyType);
		if (tmp < 0 ||
			(tmp == 0 &&
			 m_cUpperData.m_eMatch == TreeNodeInterface::LessThanEquals))
		{
			result = true;
		}
	}

	return result;
}

//
//	FUNCTION public
//	Array::Condition::isOtherConditionMatch
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

	bool result =  true;

	if (m_vecOtherData.getSize())
	{
		ModVector<Cond>::Iterator i = m_vecOtherData.begin();
		for (; i != m_vecOtherData.end(); ++i)
		{
			const ModUInt32* p1 = pBuffer_;
			const ModUInt32* p2 = (*i).m_pBuffer.get();

			Data::Type::Value eKeyType =
				getKeyType(m_eKeyType, (*i).m_OptionalChar);
			if ((*i).m_eMatch == TreeNodeInterface::Like)
			{
				result = Compare::like(
					p1, p2, eKeyType, (*i).m_OptionalChar);
			}
			else
			{
				int r = Compare::compare(p1, p2, eKeyType);
				
				switch ((*i).m_eMatch)
				{
				case TreeNodeInterface::Equals:
					if (r != 0) result = false;
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
				default:
					; _SYDNEY_ASSERT(false);
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
//	Array::Condition::setFetchField --
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
Condition::setFetchField(int n)
{
	if (n == 1)
	{
		// fetchは先頭フィールドのみ使う。
		//  参照 setFetchCondition()
		m_bFetch = true;

		// fetch以外の条件が設定されていることはない。
		// 設定されていたら、以下の条件を満たさなくなる。
		//  参照: setOpenOption()
		// そもそもgetSearchParameter()の時点で、fetchとfetch以外の
		// 条件が渡されたらfalseを返している。
		//  参照: setFetchCondition()
		; _TRMEISTER_ASSERT(
			m_bValid == true && m_eTreeType == Tree::Type::Undefined);
		
		// とりあえず非NULL値との比較であると仮定する。
		// こうしないと件数見積もりできない。
		//  参照: ArrayFile::getEstimateCountForFetch()
		// また、この設定をしたとしてもキーの設定時に影響はない
		//  参照: setFetchKey()
		m_eTreeType = Tree::Type::Data;
	}
	else
	{
		m_bFetch = false;
	}
}

//
//	FUNCTION private
//	Array::Condition::setFetchCondition --
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//
//	RETURN
//	bool
//		B木で実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Condition::setFetchCondition(const LogicalFile::TreeNodeInterface* pCondition_,
							 LogicalFile::OpenOption& cOpenOption_)
{
	using namespace LogicalFile;

	// Get the Fetch fields.
	const TreeNodeInterface* pFetchFields = pCondition_->getOptionAt(0);
	if (pFetchFields->getOperandSize() != 1)
	{
		// The number of the operand is equal to the number of the key.
		// And Array module supports only one key.
		return false;
	}

	// Check the field in which the value is set.
	const TreeNodeInterface* pField = pFetchFields->getOperandAt(0);
	if (pField->getType() != TreeNodeInterface::Field ||
		ModUnicodeCharTrait::toInt(pField->getValue())
		!= FileID::FieldPosition::Array)
	{
		return false;
	}
	// Array supports the specification of arbitrary element for the column.
	// NOT supports the specification of an element.
	if (pField->getOptionSize() != 1 ||
		pField->getOptionAt(0)->getType() != TreeNodeInterface::All)
	{
		return false;
	}

	// Array use only the top field.

	// Fetchに利用するフィールド数
	cOpenOption_.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(Key::FetchFieldNumber), 1);
	
	return true;
}

//
//	FUNCTION private
//	Array::Condition::setTreeCondition --
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
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
Condition::setTreeCondition(
	const LogicalFile::TreeNodeInterface* pCondition_,
	LogicalFile::OpenOption& cOpenOption_)
{
	using namespace LogicalFile;

	// Get the number of nodes.
	int size = 1;
	if (pCondition_->getType() == TreeNodeInterface::Or)
	{
		size = static_cast<int>(pCondition_->getOperandSize());
		; _SYDNEY_ASSERT(size > 1);
	}

	// Set the conditions.
	int num = 0;
	for (int i = 0; i < size; ++i)
	{
		// Get a node.
		const TreeNodeInterface* pNode = pCondition_;
		if (size != 1)
		{
			// The type of pCondition_ is 'or'.
			pNode = pCondition_->getOperandAt(i);
		}

		// Set the condition of the node.
		if (setNodeCondition(pNode, cOpenOption_, num) == false)
		{
			return false;
		}
	}
	if (num == 0)
	{
		// All the condition is unknown or useless.
		
		ModUnicodeOstrStream cStream;
		setUnknownStream(cStream);
		setCondition(cOpenOption_, num++, cStream);
	}

	// Set the number of conditions
	// which may be greater than the number of nodes.
	cOpenOption_.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(Key::ConditionCount),
		num);

	return true;
}

//
//	FUNCTION private
//	Array::Condition::setVerifyTreeCondition --
//
//	NOTES
//	AND ---- Equals -------- Field(0)
//	  |			|
//	  |			+----------- List -------- ConstantValue/Variable
//	  |						  |
//	  |						  +----------- ConstantValue/Variable
//	  |						  |
//	  |						  ...
//	  |
//	  +----- Equals -------- Field(1)
//			    |
//				+----------- ConstantValue/Variable("1") ← ROWID=1の場合
//
//	AND ---- EqualsToNull
//	  |
//	  +----- Equals -------- Field(1)
//			    |
//				+----------- ConstantValue/Variable("1") ← ROWID=1の場合
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
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
Condition::setVerifyTreeCondition(
	const LogicalFile::TreeNodeInterface* pCondition_,
	LogicalFile::OpenOption& cOpenOption_)
{
	using namespace LogicalFile;

	// Get the number of nodes.
	if (pCondition_->getType() != TreeNodeInterface::And ||
		pCondition_->getOperandSize() != 2)
	{
		return false;
	}

	// Set the Value conditions.
	// And num is equls to the Index condition.
	const TreeNodeInterface* pNode = pCondition_->getOperandAt(0);
	int num = 0;
	if (pNode->getType() == TreeNodeInterface::EqualsToNull)
	{
		// f IS NULL
		ModUnicodeOstrStream cStream;
		setScanStream(cStream, Tree::Type::NullArray);
		setCondition(cOpenOption_, num++, cStream);
	}
	else if (pNode->getType() == TreeNodeInterface::Equals
			 && pNode->getOperandSize() == 2)
	{
		const TreeNodeInterface* pList = pNode->getOperandAt(1);
		if (pList->getType() != TreeNodeInterface::List)
		{
			return false;
		}

		int size = pList->getOperandSize();
		for (int i = 0; i < size; ++i)
		{
			const TreeNodeInterface* pValue = pList->getOperandAt(i);
			if (pValue == 0 ||
				(pValue->getData() && pValue->getData()->isNull() == true))
			{
				// f[num] is NULL
				ModUnicodeOstrStream cStream;
				setScanStream(cStream, Tree::Type::NullData);
				setCondition(cOpenOption_, num++, cStream);
			}
			else
			{
				// f[num] = Value
				TreeNodeInterface* pDummy = 0;
				if (setEqualsNodeCondition(
						pNode->getType(), pDummy, pValue, cOpenOption_, num)
					== false)
				{
					return false;
				}
			}
		}
	}
	else
	{
		return false;
	}
	// Set the number of the Value conditions.
	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(Key::ConditionCount),
							num);

	// Set the RowID condition.
	pNode = pCondition_->getOperandAt(1);
	if (pNode == 0 || pNode->getType() != TreeNodeInterface::Equals)
	{
		return false;
	}
	const TreeNodeInterface* pValue = pNode->getOperandAt(1);
	if (pValue == 0 ||
		(pValue->getType() != TreeNodeInterface::ConstantValue &&
		 pValue->getType() != TreeNodeInterface::Variable))
	{
		return false;
	}
	const ModUnicodeString cstrRowID = pValue->getValue();
	cOpenOption_.setString(_SYDNEY_OPEN_PARAMETER_KEY(Key::RowID),
						   cstrRowID);

	return true;
}

//
//	FUNCTION private
//	Array::Condition::setNodeCondition --
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//
//	RETURN
//	bool
//		B木で実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Condition::setNodeCondition(const LogicalFile::TreeNodeInterface* pCondition_,
							LogicalFile::OpenOption& cOpenOption_,
							int& iNum_)
{
	using namespace LogicalFile;

	bool result = false;
	
	// Check term.
	const TreeNodeInterface* pField = 0;
	const TreeNodeInterface* pValue = 0;
	const TreeNodeInterface* pValue2 = 0;
	TreeNodeInterface::Type eMatch = pCondition_->getType();
	switch (eMatch)
	{
	case TreeNodeInterface::EqualsToNull:
	case TreeNodeInterface::NotNull:
		result = checkOneTerm(pCondition_, pField);
		break;
	case TreeNodeInterface::Between:
		result = checkThreeTerm(pCondition_, pField, pValue, pValue2);
		break;
	default:
		result = checkTwoTerm(pCondition_, eMatch, pField, pValue);
		break;
	}

	if (result == true)
	{
		// Set conditions.
		switch (eMatch)
		{
		case TreeNodeInterface::Equals:
			result = setEqualsNodeCondition(eMatch, pField, pValue,
											cOpenOption_, iNum_);
			break;
		case TreeNodeInterface::LessThan:
		case TreeNodeInterface::LessThanEquals:
			result = setLessThanNodeCondition(eMatch, pField, pValue,
											  cOpenOption_, iNum_);
			break;
		case TreeNodeInterface::GreaterThan:
		case TreeNodeInterface::GreaterThanEquals:
			result = setGreaterThanNodeCondition(eMatch, pField, pValue,
												 cOpenOption_, iNum_);
			break;
		case TreeNodeInterface::EqualsToNull:
			result = setEqualsToNullNodeCondition(eMatch, pField, pValue,
												  cOpenOption_, iNum_);
			break;
		case TreeNodeInterface::NotNull:
			result = setNotNullNodeCondition(eMatch, pField, pValue,
											 cOpenOption_, iNum_);
			break;
		case TreeNodeInterface::Between:
			result = setBetweenNodeCondition(eMatch, pField, pValue, pValue2,
											 cOpenOption_, iNum_);
			break;
		case TreeNodeInterface::Like:
			result = setLikeNodeCondition(eMatch, pField, pValue,
										  getEscapeChar(pCondition_),
										  cOpenOption_, iNum_);
			break;
		default:
			result = false;
			break;
		}
	}
	return result;
}

//
//	FUNCTION private
//	Array::Condition::setEqualsNodeCondition -- Equalsノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//
//	RETURN
//	bool
//		B木で実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Condition::setEqualsNodeCondition(LogicalFile::TreeNodeInterface::Type eMatch_,
								  const LogicalFile::TreeNodeInterface* pField_,
								  const LogicalFile::TreeNodeInterface* pValue_,
								  LogicalFile::OpenOption& cOpenOption_,
								  int& iNum_)
{
	using namespace LogicalFile;

	; _SYDNEY_ASSERT(eMatch_ == TreeNodeInterface::Equals);

	ModUnicodeString cstrValue;
	bool bNoPadKey = false;
	if (getValue(eMatch_, pValue_, cstrValue, bNoPadKey) == true)
	{
		bool bNoPadField = checkNoPadSortOrder(
			m_eKeyType, m_cFileID.isFixed(), bNoPadKey);

		ModUnicodeOstrStream cStream;
		setEqualsStream(cStream, eMatch_, cstrValue, bNoPadField, bNoPadKey);
		setCondition(cOpenOption_, iNum_++, cStream);
	}

	return true;
}

//
//	FUNCTION private
//	Array::Condition::setLessThanNodeCondition --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
bool
Condition::setLessThanNodeCondition(
	LogicalFile::TreeNodeInterface::Type eMatch_,
	const LogicalFile::TreeNodeInterface* pField_,
	const LogicalFile::TreeNodeInterface* pValue_,
	LogicalFile::OpenOption& cOpenOption_,
	int& iNum_)
{
	using namespace LogicalFile;

	ModUnicodeString cstrValue;
	bool bNoPadKey = false;
	if (getValue(eMatch_, pValue_, cstrValue, bNoPadKey) == true)
	{
		bool bNoPadField = checkNoPadSortOrder(
			m_eKeyType, m_cFileID.isFixed(), bNoPadKey);

		ModUnicodeOstrStream cStream;
		setInequalityStream(
			cStream,
			TreeNodeInterface::Undefined, cstrValue, bNoPadField, bNoPadKey,
			eMatch_, cstrValue, bNoPadField, bNoPadKey);
		setCondition(cOpenOption_, iNum_++, cStream);
	}
	
	return true;
}

//
//	FUNCTION private
//	Array::Condition::setGreaterThanNodeCondition --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
bool
Condition::setGreaterThanNodeCondition(
	LogicalFile::TreeNodeInterface::Type eMatch_,
	const LogicalFile::TreeNodeInterface* pField_,
	const LogicalFile::TreeNodeInterface* pValue_,
	LogicalFile::OpenOption& cOpenOption_,
	int& iNum_)
{
	using namespace LogicalFile;

	ModUnicodeString cstrValue;
	bool bNoPadKey = false;
	if (getValue(eMatch_, pValue_, cstrValue, bNoPadKey) == true)
	{
		bool bNoPadField = checkNoPadSortOrder(
			m_eKeyType, m_cFileID.isFixed(), bNoPadKey);

		ModUnicodeOstrStream cStream;
		setInequalityStream(
			cStream, eMatch_, cstrValue, bNoPadField, bNoPadKey,
			TreeNodeInterface::Undefined, cstrValue, bNoPadField, bNoPadKey);
		setCondition(cOpenOption_, iNum_++, cStream);
	}
	
	return true;
}

//
//	FUNCTION private
//	Array::Condition::setEqualsToNullNodeCondition --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
bool
Condition::setEqualsToNullNodeCondition(
	LogicalFile::TreeNodeInterface::Type eMatch_,
	const LogicalFile::TreeNodeInterface* pField_,
	const LogicalFile::TreeNodeInterface* pValue_,
	LogicalFile::OpenOption& cOpenOption_,
	int& iNum_)
{
	using namespace LogicalFile;
	
	if (pField_->getOptionSize() == 0)
	{
		// f IS NULL => NullArray scan
		ModUnicodeOstrStream cStream;
		setScanStream(cStream, Tree::Type::NullArray);
		setCondition(cOpenOption_, iNum_++, cStream);
	}
	else if (pField_->getOptionAt(0)->getType() == TreeNodeInterface::All)
	{
		// f[] IS NULL => NullData scan
		ModUnicodeOstrStream cStream;
		setScanStream(cStream, Tree::Type::NullData);
		setCondition(cOpenOption_, iNum_++, cStream);
	}
	else
	{
		return false;
	}

	return true;
}

//
//	FUNCTION private
//	Array::Condition::setNotNullNodeCondition --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
bool
Condition::setNotNullNodeCondition(
	LogicalFile::TreeNodeInterface::Type eMatch_,
	const LogicalFile::TreeNodeInterface* pField_,
	const LogicalFile::TreeNodeInterface* pValue_,
	LogicalFile::OpenOption& cOpenOption_,
	int& iNum_)
{
	using namespace LogicalFile;
	
	if (pField_->getOptionSize() == 0)
	{
		// f IS NOT NULL => Data scan + NullData scan

		// Set NullData scan.
		ModUnicodeOstrStream cStream;
		setScanStream(cStream, Tree::Type::NullData);
		setCondition(cOpenOption_, iNum_++, cStream);
	}
	else if (pField_->getOptionAt(0)->getType() != TreeNodeInterface::All)
	{
		return false;
	}
	// else => f[] IS NOT NULL => Data scan

	// Set Data scan.
	ModUnicodeOstrStream cStream;
	setScanStream(cStream, Tree::Type::Data);
	setCondition(cOpenOption_, iNum_++, cStream);

	return true;
}

//
//	FUNCTION private
//	Array::Condition::setBetweenNodeCondition --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
bool
Condition::setBetweenNodeCondition(
	LogicalFile::TreeNodeInterface::Type eMatch_,
	const LogicalFile::TreeNodeInterface* pField_,
	const LogicalFile::TreeNodeInterface* pValue_,
	const LogicalFile::TreeNodeInterface* pValue2_,
	LogicalFile::OpenOption& cOpenOption_,
	int& iNum_)
{
	using namespace LogicalFile;

#ifdef BETWEEN_SYMMETRIC
	// Sort the two values.
	sortValueOrder(pValue_, pValue2_);
#endif

	ModUnicodeString cstrLower;
	ModUnicodeString cstrUpper;
	bool bLowerNoPadKey = false;
	bool bUpperNoPadKey = false;
	if (getValue(eMatch_, pValue_, cstrLower, bLowerNoPadKey) == true &&
		getValue(eMatch_, pValue2_, cstrUpper, bUpperNoPadKey) == true)
	{
		bool bLowerNoPadField = checkNoPadSortOrder(
			m_eKeyType, m_cFileID.isFixed(), bLowerNoPadKey);
		bool bUpperNoPadField = checkNoPadSortOrder(
			m_eKeyType, m_cFileID.isFixed(), bUpperNoPadKey);
		
		ModUnicodeOstrStream cStream;
		setInequalityStream(
			cStream,
			TreeNodeInterface::GreaterThanEquals, cstrLower,
			bLowerNoPadField, bLowerNoPadKey,
			TreeNodeInterface::LessThanEquals, cstrUpper,
			bUpperNoPadField, bUpperNoPadKey);
		setCondition(cOpenOption_, iNum_++, cStream);
	}

	return true;
}

//
//	FUNCTION private
//	Array::Condition::setLikeNodeCondition --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
bool
Condition::setLikeNodeCondition(
	LogicalFile::TreeNodeInterface::Type eMatch_,
	const LogicalFile::TreeNodeInterface* pField_,
	const LogicalFile::TreeNodeInterface* pValue_,
	ModUnicodeChar escape_,
	LogicalFile::OpenOption& cOpenOption_,
	int& iNum_)
{
	using namespace LogicalFile;

	// Check the column.
	if (m_eKeyType != Data::Type::UnicodeString &&
		m_eKeyType != Data::Type::CharString &&
		m_eKeyType != Data::Type::NoPadUnicodeString &&
		m_eKeyType != Data::Type::NoPadCharString)
	{
		// 'like' is used for olny the string type column.
		return false;
	}

	// Get string.
	ModUnicodeString cstrValue;
	bool bNoPadKey = false;
	if (getValue(eMatch_, pValue_, cstrValue, bNoPadKey) == true)
	{
		bool bNoPadField = checkNoPadSortOrder(
			m_eKeyType, m_cFileID.isFixed(), bNoPadKey);
		// Like is always used with NO PAD.
		bool bNoPadKey = true;

		// 前方一致を上限下限に変換する
		ModUnicodeString cstrLower;
		ModUnicodeString cstrUpper;
		// Prefix search has one '%' in the end of the pattern. ex: 'ab%'
		bool isPrefixMatch;
		if (expandPattern(cstrValue, escape_, cstrLower, cstrUpper,
						  isPrefixMatch) == false)
		{
			return false;
		}

		ModUnicodeOstrStream cStream;
		if (cstrLower == cstrUpper)
		{
			// Exact match  ex: 'abc'
			setEqualsStream(
				cStream, TreeNodeInterface::Equals, cstrValue,
				bNoPadField, bNoPadKey);
		}
		else if (isPrefixMatch == true)
		{
			// Prefix match  ex: 'abc%'
			setPrefixMatchStream(
				cStream, cstrLower, cstrUpper, bNoPadField, bNoPadKey);
		}
		else
		{
			// The other cases  ex: 'ab_', 'a%c'
			setLikeStream(
				cStream, cstrLower, cstrUpper, cstrValue, escape_,
				bNoPadField, bNoPadKey);
		}
		setCondition(cOpenOption_, iNum_++, cStream);
	}

	return true;
}

//
//	FUNCTION private
//	Array::Condition::checkOneTerm -- 単項演算をチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		単項演算のノード
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
Condition::checkOneTerm(const LogicalFile::TreeNodeInterface* pCondition_,
						const LogicalFile::TreeNodeInterface*& pField_)
{
	using namespace LogicalFile;
	
	// Check the number of terms.
	if (pCondition_->getOperandSize() != 1)
	{
		return false;
	}
	
	// Check the term.
	pField_ = pCondition_->getOperandAt(0);
	if (pField_->getType() != TreeNodeInterface::Field)
	{
		return false;
	}
	
	// Check the field term.
	if (ModUnicodeCharTrait::toInt(pField_->getValue())
		!= FileID::FieldPosition::Array)
	{
		return false;
	}

	// Not check the field's option.

	return true;
}

//
//	FUNCTION private
//	Array::Condition::checkTwoTerm -- 2項演算をチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		2項演算のノード
//	LogicalFile::TreeNodeInterface::Type& eMatch_
//		一致条件
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
Condition::checkTwoTerm(const LogicalFile::TreeNodeInterface* pCondition_,
						LogicalFile::TreeNodeInterface::Type& eMatch_,
						const LogicalFile::TreeNodeInterface*& pField_,
						const LogicalFile::TreeNodeInterface*& pValue_)
{
	using namespace LogicalFile;
	
	// Check the number of terms.
	if (pCondition_->getOperandSize() != 2)
	{
		return false;
	}

	// Check the order of the terms.
	// These are set in order of SQL's string.
	pField_ = pCondition_->getOperandAt(0);
	pValue_ = pCondition_->getOperandAt(1);
	if (pValue_->getType() == TreeNodeInterface::Field
		&& (pField_->getType() == TreeNodeInterface::ConstantValue
			|| pField_->getType() == TreeNodeInterface::Variable))
	{
		alternateTerm(pValue_, pField_, eMatch_);
	}

	// Check the terms.
	if (pField_->getType() != TreeNodeInterface::Field
		|| (pValue_->getType() != TreeNodeInterface::ConstantValue
			&& pValue_->getType() != TreeNodeInterface::Variable))
	{
		return false;
	}

	// Check the field term.
	if (ModUnicodeCharTrait::toInt(pField_->getValue())
		!= FileID::FieldPosition::Array)
	{
		return false;
	}

	// Check the field's option.
	if (eMatch_ != TreeNodeInterface::Like &&
		(pField_->getOptionSize() != 1 ||
		 pField_->getOptionAt(0)->getType() != TreeNodeInterface::All))
	{
		return false;
	}
	// The operator of Like does NOT need
	// the spesification of the arbitrary element for the column.
	// ex: "c like 'abc'" is equals to "c[] like 'abc'".
	// See Bitmap and FullText modules.

	// Not check the value term.

	return true;
}

//
//	FUNCTION private
//	Array::Condition::checkThreeTerm -- 3項演算をチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		3項演算のノード
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
Condition::checkThreeTerm(const LogicalFile::TreeNodeInterface* pCondition_,
						  const LogicalFile::TreeNodeInterface*& pField_,
						  const LogicalFile::TreeNodeInterface*& pValue_,
						  const LogicalFile::TreeNodeInterface*& pValue2_)
{
	using namespace LogicalFile;
	
	// For Between operator
	
	// Check the number of terms.
	if (pCondition_->getOperandSize() != 3)
	{
		return false;
	}

	// Check the order of the terms.
	pField_ = pCondition_->getOperandAt(0);
	pValue_ = pCondition_->getOperandAt(1);
	pValue2_ = pCondition_->getOperandAt(2);

	// Check the terms.
	if (pField_->getType() != TreeNodeInterface::Field
		|| (pValue_->getType() != TreeNodeInterface::ConstantValue
			&& pValue_->getType() != TreeNodeInterface::Variable)
		|| (pValue2_->getType() != TreeNodeInterface::ConstantValue
			&& pValue2_->getType() != TreeNodeInterface::Variable))
	{
		return false;
	}

	// Check the field term.
	if (ModUnicodeCharTrait::toInt(pField_->getValue())
		!= FileID::FieldPosition::Array)
	{
		return false;
	}

	// Check the field's option.
	if (pField_->getOptionSize() != 1 ||
		pField_->getOptionAt(0)->getType() != TreeNodeInterface::All)
	{
		return false;
	}

	// Not check the value term.

	return true;
}

//
//	FUNCTION private
//	Array::Condition::getValue -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	bool
//			'false' means the data is null or the condition is useless.
//
//	EXCEPTIONS
//
bool
Condition::getValue(LogicalFile::TreeNodeInterface::Type eMatch_,
					const LogicalFile::TreeNodeInterface* pValue_,
					ModUnicodeString& cstrValue_,
					bool& bNoPadKey_)
{
	using namespace LogicalFile;

	; _SYDNEY_ASSERT(pValue_ != 0);
	const Common::Data* pData = pValue_->getData();
	if (pData == 0 || pData->isNull())
	{
		return false;
	}
		
	if (m_eKeyType == Data::Type::Integer
		&& (pData->getType() == Common::DataType::Double
			|| pData->getType() == Common::DataType::Integer64))
	{
		// フィールドがintで、比較対象がdoubleかint64だったら
		int value;
		if (Data::Integer::round(*pData, eMatch_, value) == false)
		{
			return false;
		}
		ModUnicodeOstrStream s;
		s << value;
		cstrValue_ = s.getString();
	}
	else if (m_eKeyType == Data::Type::Integer64
			 && pData->getType() == Common::DataType::Double)
	{
		// フィールドがint64で、比較対象がdoubleだったら
		ModInt64 value;
		if (Data::Integer64::round(*pData, eMatch_, value) == false)
		{
			return false;
		}
		ModUnicodeOstrStream s;
		s << value;
		cstrValue_ = s.getString();
	}
	else if ((m_eKeyType == Data::Type::UnicodeString
			  || m_eKeyType == Data::Type::CharString)
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
		// from the condtion of the key string.
		// 
		cstrValue_ = pStringData->getValue();
	}
	else if (m_eKeyType == Data::Type::NoPadUnicodeString ||
			 m_eKeyType == Data::Type::NoPadCharString)
	{
		// The collation of the field is NO PAD.
		cstrValue_ = pValue_->getValue();
		bNoPadKey_ = true;
	}
	else if (m_eKeyType == Data::Type::Decimal)
	{
		int precision;
		int scale;
		Data::Decimal::getParameter(
			m_cFileID, FileID::FieldPosition::Array, true, precision, scale);
		if (Data::Decimal::round(*pData, eMatch_,
								 precision, scale, cstrValue_) == false)
		{
			return false;
		}
	}
	else
	{
		cstrValue_ = pValue_->getValue();
	}

	return true;
}

//
//	FUNCTION private
//	Array::Condition::expandPattern -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
bool
Condition::expandPattern(const ModUnicodeString& cstrValue_,
						 ModUnicodeChar escape_,
						 ModUnicodeString& cstrLower_,
						 ModUnicodeString& cstrUpper_,
						 bool& isPrefixMatch_)
{
	isPrefixMatch_ = false;
	const ModUnicodeChar* s = cstrValue_;
	const ModUnicodeChar* p = s;
	
	while (*p != 0)
	{
		if (*p == escape_)
		{
			p++;
			if (*p == 0)
			{
				break;
			}
			cstrLower_ += *p;
			cstrUpper_ += *p;
			p++;
			continue;
		}

		if (*p == Common::UnicodeChar::usPercent ||
			*p == Common::UnicodeChar::usLowLine)
		{
			if (cstrLower_.getLength() == 0)
			{
				// like '%' or like '_' is NOT supported.
				return false;
			}

			if (m_isNormalized == true)
			{
				// [BUG] 以下の方法だと検索結果が不正になる場合がある。
				//  Bug report 1324
				
				// ++する前に正規化しないといけない
				// createCommonData()でも正規化するので２重にやることになるが、
				// しょうがないかな。
				Common::StringData cStringData(cstrUpper_);
				Utility::CharTrait::normalize(
					cStringData.getValue(),
					m_cFileID.getNormalizingMethod(),
					cstrUpper_);
			}

			// Set upper condition.
			(cstrUpper_[cstrUpper_.getLength() - 1])++;

			// Set the other condition.
			if (*p == Common::UnicodeChar::usPercent)
			{
				p++;
				if (cstrValue_.getLength() == static_cast<ModSize>(p - s))
				{
					// The end of the pattern is '%'.	ex: 'ab%'
					isPrefixMatch_ = true;
				}
			}
			break;
		}
		else
		{
			cstrLower_ += *p;
			cstrUpper_ += *p;
		}

		p++;
	}

	return true;
}

//
//	FUNCTION private
//	Array::Condition::getEscapeChar -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModUnicodeChar
Condition::getEscapeChar(
	const LogicalFile::TreeNodeInterface*& pCondition_) const
{
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
	return usEscape;
}


//
//	FUNCTION private
//	Array::Condition::createCommonData -- Common::Dataを作成する
//
//	NOTES
//
//	ARGUMENTS
//	Array::Data::Type::Value
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
Condition::createCommonData(Data::Type::Value eKeyType_,
							ModUnicodeString& cstrValue_)
{
	Common::Data::Pointer pData;

	switch (eKeyType_)
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
	case Data::Type::Double:
	{
		Common::StringData cSrc(cstrValue_);
		pData = cSrc.cast(Common::DataType::Double, true);
	}
	break;
	case Data::Type::Decimal:
	{
		int precision;
		int scale;
		Data::Decimal::getParameter(
			m_cFileID, FileID::FieldPosition::Array, true, precision, scale);
		Common::DecimalData cDec(precision, scale);
		Common::StringData cSrc(cstrValue_);
		if ("" == cstrValue_)
		{
			pData = Common::NullData::getInstance();
		}
		else
		{
			pData = cSrc.cast(cDec, true);
		}
	}
	break;
	case Data::Type::CharString:
	case Data::Type::UnicodeString:
	case Data::Type::NoPadCharString:
	case Data::Type::NoPadUnicodeString:
	{
		if (m_isNormalized == true)
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
	case Data::Type::DateTime:
		pData = new Common::DateTimeData(cstrValue_);
		break;
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
//	Array::Condition::dumpCommonData --
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
Condition::dumpCommonData(AutoPointer<ModUInt32>& pBuffer_,
						  Data::Type::Value eKeyType_,
						  const Common::Data& cData_)
{
	ModSize size = Data::getSize(cData_, eKeyType_) * sizeof(ModUInt32);
	pBuffer_ = syd_reinterpret_cast<ModUInt32*>(Os::Memory::allocate(size));
	Data::dumpOneData(pBuffer_, cData_, eKeyType_);
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
	case TreeNodeInterface::EqualsToNull:
	case TreeNodeInterface::NotNull:
	case TreeNodeInterface::Between:
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
//	Array::Condition::setEqualsStream --
//
//	NOTES
//	Example of the case where the field is PAD SPACE.
//	Assume that the next of \t is \n, the next of \n is \r, and
//	these are smaller than white space.
//	
//	Input:
//		f = 'abc'
//		f = 'ab\n'
//		f = 'ab '
//	Expand Condition:
//		abc(PAD SPACE) <= f <= abc(NO PAD) and f = abc(NO PAD)
//		ab\n(PAD SPACE) <= f <= ab\n(NO PAD) and f = ab\n(NO PAD)
//		ab (PAD SPACE) <= f <= ab (NO PAD) and f = ab (NO PAD)
//
//	1st example does NOT need Other condition.
//	But, 2nd and 3rd one NEED it.
//	Assume that the data consists of ab\n < ab < aba,
//	and using 'ab\n'(PAD SPACE) <= f <= 'ab\n'(NO PAD), 'ab' will be gotten.
//	If using 'ab '(PAD SPACE) <= f <= 'ab '(NO PAD), 'ab' will be gotten.
//
//	Upper condition does NOT care about the collation of the comparison.
//	NO PAD is better than PAD SPACE in the point of comparison cost,
//	when the length of the data in the index is defferent
//	from the length of the key.
//	NO PAD is worse, in the 2nd example,
//	assume that the data consists of ab\n < ab <= ab <= ab <= ... < aba,
//	this condition NEED to compare with all 'ab' which is smaller than
//	'ab\n' with NO PAD.
//	The latter case may be a rare case, so it's ignored.
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Condition::setEqualsStream(ModUnicodeOstrStream& cStream_,
						   LogicalFile::TreeNodeInterface::Type eMatch_,
						   ModUnicodeString& cstrValue_,
						   bool bNoPadField_,
						   bool bNoPadKey_)
{
	using namespace LogicalFile;

	cStream_ << '%' << Tree::Type::Data << '(';

	TreeNodeInterface::Type eLowerType = TreeNodeInterface::GreaterThanEquals;
	TreeNodeInterface::Type eUpperType = TreeNodeInterface::LessThanEquals;
	ModUnicodeChar usPadding = getPaddingChar(bNoPadField_ || bNoPadKey_);

	if (bNoPadField_ == false && bNoPadKey_ == true)
	{
		// Lower condition with PAD SPACE
		ModUnicodeChar usExpand = getPaddingChar(bNoPadField_);
		ParseValue::putStream(cStream_, eLowerType, cstrValue_, usExpand);
		// Upper condition with NO PAD
		ParseValue::putStream(cStream_, eUpperType, cstrValue_, usPadding);
		// Other condition(Equals condition) with NO PAD
		;_SYDNEY_ASSERT(eMatch_ == TreeNodeInterface::Equals);
		ParseValue::putStream(cStream_, eMatch_, cstrValue_, usPadding);
	}
	else
	{
		// Lower condition
		ParseValue::putStream(cStream_, eLowerType, cstrValue_, usPadding);
		// Upper condition
		ParseValue::putStream(cStream_, eUpperType, cstrValue_, usPadding);
	}
	cStream_ << ')';
}

//
//	FUNCTION private
//	Array::Condition::setInequalityStream --
//
//	NOTES
//	Example of the case where the field is PAD SPACE.
//	Assume that the next of \t is \n, the next of \n is \r, and
//	these are smaller than white space.
//	
//	Lower Input 1:
//		f > 'abc'
//		f >= 'abc'
//	Lower Expand Condition 1:
//		f > abb(PAD SPACE) and f > abc(NO PAD)
//		f > abb(PAD SPACE) and f >= abc(NO PAD)
//
//	Lower condition must be expanded.
//	The data in the field is sorted as the folowing.
//		abb < abba < abc\n < abc < abca
//	If using f > abc(PAD SPACE), abc\n will be missed.
//	And NOT need to use GreaterThan"Equals".
//	Because abb is smaller than abc sufficiently.
//
//	Lower Input 2:
//		f > ''
//		f >= ''
//	Lower Expand Condition 2:
//		f > UNDEFINED and f > ''(NO PAD)
//		f > UNDEFINED
//
//	Empty string need a special handling.
//	The data in the field is sorted as the folowing.
//		\n < '' < a < b
//	If using f > ''(PAD SPACE), \n will be missed.
//	And NOT need to set f >= ''(NO PAD) to Other condition.
//	Because, the condiiton returns true always.
//
//	Lower Input 3:
//		f > abc\1
//		f >= abc\1
//	Lower Expand Condition 3:
//		f > abb(PAD SPACE) and f > abc\1(NO PAD)
//		f > abb(PAD SPACE) and f >= abc\1(NO PAD)
//
//	If using f > abc\0(PAD SPACE) and f > abc\1(NO PAD), abc\n will be missed.
//
//	Lower Input 4:
//		f > \1
//		f >= \1
//	Lower Expand Condition 4:
//		f > UNDEFINED and f > \1(NO PAD)
//		f > UNDEFINED and f >= \1(NO PAD)
//
//	\1 needs special handling as well as an empty string.
//
//
//	Upper Input 1:
//		f < 'abc'
//		f <= 'abc'
//	Upper Expand Condition 1:
//		f < abc(NO PAD)
//		f <= abc(PAD SPACE) and f <= abc(NO PAD)
//
//	In the case where
//	the key does NOT include any character smaller than _padding char.
//
//	NOT need to expand the condition when the operator is LessThan.
//	The data in the field is sorted as the folowing.
//		abb < abba < abc\n < abc < abca
//	If using f < abc(PAD SPACE), abc\n will be gotten.
//	And Upper condition is used only for a comparison, NOT for a search.
//	Comparison does NOT care about the order of the field.
//
//	BUT, need to expand it when the operator is LessThanEquals.
//	If using f <= abc(NO PAD), abc may be missed.
//	Because abc\n is greater than abc comparing with NO PAD.
//
//	Upper Input 2:
//		f < '\n'
//		f <= '\n'
//		f < 'abc\n'
//		f <= 'abc\n'
//		f < 'abc\nxyz'
//		f <= 'abc\nxyz'
//	Upper Expand Condition 2:
//		f <= ''(PAD SPACE) and f < \n(NO PAD)
//		f <= ''(PAD SPACE) and f <= \n(NO PAD)
//		f <= abc(PAD SPACE) and f < abc\n(NO PAD)
//		f <= abc(PAD SPACE) and f <= abc\n(NO PAD)
//		f <= abc(PAD SPACE) and f < abc\nxyz(NO PAD)
//		f <= abc(PAD SPACE) and f <= abc\nxyz(NO PAD)
//
//	In the case where the key include any character smaller than _padding char.
//
//	The data in the field is sorted as the folowing.
//		abb < abc\n < abc\nxyz < abc
//	If using f < abc\n(NO PAD), abc will be missed.
//	If using f < abc\nxyz(NO PAD), abc will be missed too.
//
//	Upper Input 3:
//		f < ' '
//		f <= ' '
//		f < 'abc '
//		f <= 'abc '
//	Upper Expand Condition 3:
//		f < ' '(NO PAD)
//		f <= ' '(NO PAD)
//		f < abc (NO PAD)
//		f <= abc (NO PAD)
//
//	NOT need to expand these conditions includeing white spaces.
//
//	Upper Input 4:
//		f < ''
//		f <= ''
//	Upper Expand Condition 4:
//		f < ''(NO PAD)
//		f <= ''(PAD SPACE) and f <= ''(NO PAD)
//
//	f < ''(NO PAD) is meaningless.
//	But it's a rare case, so ignored.
//
//	NOT need to expand these conditions including white spaces.
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Condition::setInequalityStream(ModUnicodeOstrStream& cStream_,
							   LogicalFile::TreeNodeInterface::Type eLower_,
							   ModUnicodeString& cstrLower_,
							   bool bLowerNoPadField_,
							   bool bLowerNoPadKey_,
							   LogicalFile::TreeNodeInterface::Type eUpper_,
							   ModUnicodeString& cstrUpper_,
							   bool bUpperNoPadField_,
							   bool bUpperNoPadKey_)
{
	using namespace LogicalFile;
	TreeNodeInterface::Type eUndefType = TreeNodeInterface::Undefined;

	cStream_ << '%' << Tree::Type::Data << '(';

	ModUnicodeChar usLower =
		getPaddingChar(bLowerNoPadField_ || bLowerNoPadKey_);
	ModUnicodeChar usUpper =
		getPaddingChar(bUpperNoPadField_ || bUpperNoPadKey_);

	// Set Lower condition.
	bool bLowerOtherCondition = false;
	if (bLowerNoPadField_ == false && bLowerNoPadKey_ == true
		&& eLower_ != eUndefType)
	{
		int position = getPositionOfTraillingSOH(cstrLower_);
		if (position != 0)
		{
			// Expand the range of the condition.
			ModUnicodeString cstrExpand = cstrLower_;
			cstrExpand[position - 1]--;
			ModUnicodeChar usExpand = getPaddingChar(bLowerNoPadField_);
			ParseValue::putStream(cStream_, TreeNodeInterface::GreaterThan,
								  cstrExpand, usExpand);
			bLowerOtherCondition = true;
		}
		else
		{
			// Not able to expand the range, so set an undefined condition.
			ParseValue::putStream(cStream_, eUndefType, cstrLower_, usLower);

			if (eLower_ == TreeNodeInterface::GreaterThan
				|| cstrLower_.getLength() != 0)
			{
				bLowerOtherCondition = true;
			}
			// When f >= '', NOT need to set Other condition. See NOTES.
		}
	}
	else
	{
		ParseValue::putStream(cStream_, eLower_, cstrLower_, usLower);
	}

	// Set Upper condition.
	bool bUpperOtherCondition = false;
	if (bUpperNoPadField_ == false && bUpperNoPadKey_ == true
		&& eUpper_ != eUndefType)
	{
		ModUnicodeChar usExpand = getPaddingChar(bUpperNoPadField_);

		int position = getPositionOfFirstSmallCharacter(cstrUpper_);
		int length = cstrUpper_.getLength();
		if (position != length)
		{
			// Exist the character smaller than _paddingChar so expand it.

			ModUnicodeString cstrExpand;
			const ModUnicodeChar* p = cstrUpper_;
			cstrExpand.allocateCopy(p, position);
			ParseValue::putStream(cStream_, TreeNodeInterface::LessThanEquals,
								  cstrExpand, usExpand);
			bUpperOtherCondition = true;
		}
		else
		{
			// NOT exist the character smaller than _paddingChar.

			if (eUpper_ == TreeNodeInterface::LessThanEquals
				&& (length == 0 || cstrUpper_[length - 1] != _paddingChar))
			{
				// Expand it. ex: f <= '', f <= 'abc'
				ParseValue::putStream(
					cStream_, TreeNodeInterface::LessThanEquals,
					cstrUpper_,	usExpand);
				bUpperOtherCondition = true;
			}
			else
			{
				// NOT need to expand it. ex: f < 'abc', f <= 'abc '
				ParseValue::putStream(cStream_, eUpper_, cstrUpper_, usUpper);
			}
		}
	}
	else
	{
		ParseValue::putStream(cStream_, eUpper_, cstrUpper_, usUpper);
	}

	// Set Other condition.
	if (bLowerOtherCondition == true)
		ParseValue::putStream(cStream_, eLower_, cstrLower_, usLower);
	if (bUpperOtherCondition == true)
		ParseValue::putStream(cStream_, eUpper_, cstrUpper_, usUpper);

	cStream_ << ')';
}

//
//	FUNCTION private
//	Array::Condition::setPrefixMatchStream --
//
//	NOTES
//	Example of the case where the field is PAD SPACE.
//	Assume that the next of \t is \n, the next of \n is \r, and
//	these are smaller than white space.
//	
//	Input:
//		f like 'abc%'
//		f like 'ab\n%'
//		f like '\1%'
//		f like 'ab\1%'
//	Expand Pattern:
//		abc <= f < abd
//		ab\n <= f < ab\r
//		\1 <= f < \2
//		ab\1 <= f < ab\2
//	Expand Condition:
//		abb(PAD SPACE) < f < abd(NO PAD) and f >= abc(NO PAD)
//		ab\t(PAD SPACE) < f < ab\r(NO PAD) and f >= ab\n(NO PAD)
//		UNDEFINED < f < \2(NO PAD) and f >= \1(NO PAD)
//		aa(PAD SPACE) < f < ab\2(NO PAD) and f >= ab\1(NO PAD)
//
//	Lower condition must be expanded.
//	The data in the field is sorted as the folowing.
//		abb < abba < abc\n < abc < abca < abd\n < abd
//	If using abc(PAD SPACE) <= f < abd(NO PAD) and abc(NO PAD) <= f,
//	abc\n will be missed.
//	If using ab\n(PAD SPACE) <= f < ab\r(NO PAD) and ab\n(NO PAD) <= f,
//	ab\n\n will be missed.
//	If using abb(PAD SPACE) < f < abd(NO PAD), abba will be gotten.
//	If using ab\t(PAD SPACE) < f < ab\r(NO PAD), ab\txyz will be gotten.
//
//	Upper condition must use NO PAD.
//	Because this is used for a comparison, not a searching.
//	Comparison does NOT care about the order of the field.
//	And abd is larger than abc% regardless of the comparison of the collation.
//	ab\r is larger than ab\n% too.
//	If using abb(PAD SPACE) < f < abd(PAD SPACE) and abc(NO PAD) <= f,
//	abd\n will be gotton.
//	If using ab\t(PAD SPACE) < f < ab\r(PAD SPACE) and ab\n(NO PAD) <= f
//	ab\r\n will be gotton.
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Condition::setPrefixMatchStream(ModUnicodeOstrStream& cStream_,
								const ModUnicodeString& cstrLower_,
								const ModUnicodeString& cstrUpper_,
								bool bNoPadField_,
								bool bNoPadKey_)
{
	using namespace LogicalFile;
	TreeNodeInterface::Type eUndefType = TreeNodeInterface::Undefined;

	; _SYDNEY_ASSERT(cstrLower_.getLength() > 0);
	; _SYDNEY_ASSERT(cstrUpper_.getLength() > 0);
	// The collation of key is treated as NO PAD in Like.
	; _SYDNEY_ASSERT(bNoPadKey_ == true);

	cStream_ << '%' << Tree::Type::Data << '(';

	ModUnicodeChar usPadding = getPaddingChar(bNoPadField_ || bNoPadKey_);
	
	if (bNoPadField_ == false && bNoPadKey_ == true)
	{
		// Lower expanded conditon 
		int position = getPositionOfTraillingSOH(cstrLower_);
		if (position != 0)
		{
			// Expand the range of the condition.
			ModUnicodeString cstrExpand = cstrLower_;
			cstrExpand[position - 1]--;
			ModUnicodeChar usExpand = getPaddingChar(bNoPadField_);
			ParseValue::putStream(cStream_, TreeNodeInterface::GreaterThan,
								  cstrExpand, usExpand);
		}
		else
		{
			// Not able to expand the range, so set an undefined condition.
			ParseValue::putStream(cStream_, eUndefType, cstrLower_, usPadding);
		}

		// Upper condition
		ParseValue::putStream(cStream_, TreeNodeInterface::LessThan,
							  cstrUpper_, usPadding);

		// Other conditon(Lower original condition)
		ParseValue::putStream(cStream_, TreeNodeInterface::GreaterThanEquals,
							  cstrLower_, usPadding);
	}
	else
	{
		// Lower condition
		ParseValue::putStream(cStream_, TreeNodeInterface::GreaterThanEquals,
							  cstrLower_, usPadding);
		// Upper condition
		ParseValue::putStream(cStream_, TreeNodeInterface::LessThan,
							  cstrUpper_, usPadding);
	}
	cStream_ << ')';
}

//
//	FUNCTION private
//	Array::Condition::setLikeStream --
//
//	NOTES
//	Example of the case where the field is PAD SPACE.
//	Assume that the next of \t is \n, the next of \n is \r, and
//	these are smaller than white space.
//	
//	Input:
//		f like 'abc%xyz'
//		f like 'ab\n%xyz'
//		f like '\1%xyz'
//		f like 'ab\1%xyz'
//	Expand Pattern:
//		abc <= f < abd and f like 'abc%xyz'
//		ab\n <= f < ab\r and f like 'ab\n%xyz'
//		\1 <= f < \2 and f like '\1%xyz'
//		ab\1 <= f < ab\2 and f like 'ab\1%xyz'
//	Expand Condition:
//		abb(PAD SPACE) < f < abd(NO PAD) and f like 'abc%xyz'
//		ab\t(PAD SPACE) < f < ab\r(NO PAD) and f like 'ab\n%xyz'
//		UNDEFINED < f < \2(NO PAD) and f like '\1%xyz'
//		aa(PAD SPACE) < f < ab\2(NO PAD) and f like 'ab\1%xyz'
//
//	See setPrefixMatch for details.
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
Condition::setLikeStream(ModUnicodeOstrStream& cStream_,
						 const ModUnicodeString& cstrLower_,
						 const ModUnicodeString& cstrUpper_,
						 const ModUnicodeString& cstrLike_,
						 ModUnicodeChar	escape_,
						 bool bNoPadField_,
						 bool bNoPadKey_)
{
	using namespace LogicalFile;
	TreeNodeInterface::Type eUndefType = TreeNodeInterface::Undefined;

	; _SYDNEY_ASSERT(cstrLower_.getLength() > 0);
	; _SYDNEY_ASSERT(cstrUpper_.getLength() > 0);
	; _SYDNEY_ASSERT(cstrLike_.getLength() > 0);
	// The collation of key is treated as NO PAD in Like.
	; _SYDNEY_ASSERT(bNoPadKey_ == true);

	cStream_ << '%' << Tree::Type::Data << '(';

	ModUnicodeChar usPadding = getPaddingChar(bNoPadField_ || bNoPadKey_);
	
	// Lower condition
	if (bNoPadField_ == false && bNoPadKey_ == true)
	{
		// Expand the lower condition.
		int position = getPositionOfTraillingSOH(cstrLower_);
		if (position != 0)
		{
			// Expand the range of the condition.
			ModUnicodeString cstrExpand = cstrLower_;
			cstrExpand[position - 1]--;
			ModUnicodeChar usExpand = getPaddingChar(bNoPadField_);
			ParseValue::putStream(cStream_, TreeNodeInterface::GreaterThan,
								  cstrExpand, usExpand);
		}
		else
		{
			// Not able to expand the range, so set an undefined condition.
			ParseValue::putStream(cStream_, eUndefType, cstrLower_, usPadding);
		}
	}
	else
	{
		ParseValue::putStream(cStream_, TreeNodeInterface::GreaterThanEquals,
							  cstrLower_, usPadding);
	}

	// Upeer condition
	ParseValue::putStream(cStream_, TreeNodeInterface::LessThanEquals,
						  cstrUpper_, usPadding);

	// Other condition(Like condition)
	ParseValue::putStream(cStream_, TreeNodeInterface::Like,
						  cstrLike_, escape_);

	cStream_ << ')';
}

//
//	FUNCTION private
//	Array::Condition::setScanStream --
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
Condition::setScanStream(ModUnicodeOstrStream& cStream_,
						 Tree::Type::Value eTreeType_)
{
	using namespace LogicalFile;

	ModUnicodeString s;
	ModUnicodeChar c = 0;
	TreeNodeInterface::Type eUndefType = TreeNodeInterface::Undefined;
	
	cStream_ << '%' << eTreeType_ << '(';
	ParseValue::putStream(cStream_, eUndefType, s, c);
	ParseValue::putStream(cStream_, eUndefType, s, c);
	cStream_ << ')';
}

//
//	FUNCTION private
//	Array::Condition::setUnknownStream --
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
Condition::setUnknownStream(ModUnicodeOstrStream& cStream_)
{
	using namespace LogicalFile;

	ModUnicodeString s;
	ModUnicodeChar c = 0;

	cStream_ << '%' << Tree::Type::Undefined << '(';
	ParseValue::putStream(cStream_, TreeNodeInterface::Unknown, s, c);
	cStream_ << ')';
}

#ifdef BETWEEN_SYMMETRIC
//
//	FUNCTION private
//	Array::Condition::sortValueOrder --
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
Condition::sortValueOrder(const LogicalFile::TreeNodeInterface*& pValue_,
						  const LogicalFile::TreeNodeInterface*& pValue2_)
{
	using namespace LogicalFile;
	
	int compare = 0;
	const Common::Data* pData = pValue_->getData();
	const Common::Data* pData2 = pValue2_->getData();
	if (m_eKeyType == Data::Type::UnicodeString ||
		m_eKeyType == Data::Type::CharString)
	{
		// Compare with PAD SPACE.

		// [OPTIMIZE]
		// NOT need to create new strings
		// if the collaiton of pData and pData2 is PAD SPACE.
		ModUnicodeString cstrValue;
		ModUnicodeString cstrValue2;
		if (pData != 0)
		{
			cstrValue = pData->getString();
		}
		if (pData2 != 0)
		{
			cstrValue2 = pData2->getString();
		}
		// Collation is Implicit.
		Common::StringData str(cstrValue);
		Common::StringData str2(cstrValue2);

		compare = str.compareTo(&str2);
	}
	else
	{
		compare = pData->compareTo(pData2);
	}
	if (compare > 0)
	{
		const TreeNodeInterface* p = pValue2_;
		pValue2_ = pValue_;
		pValue_ = p;
	}
}
#endif

//
//	FUNCTION private
//	Array::Condition::checkNoPadSortOrder --
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
Condition::checkNoPadSortOrder(Data::Type::Value eFieldType_,
							   bool bFixedField_, bool bNoPadKey_) const
{
	bool result = false;

	switch (eFieldType_)
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
//	Array::Condition::setCondition --
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
Condition::setCondition(LogicalFile::OpenOption& cOpenOption_,
						int iNum_,
						ModUnicodeOstrStream& cStream_)
{
	// Set to OpenOption.
	cOpenOption_.setString(
		_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(Key::Condition, iNum_),
		cStream_.getString());
}

//
//	FUNCTION private
//	Array::Condition::setCond --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
bool
Condition::setCond(Condition::Cond& cCond_,
				   const ModUnicodeChar*& p)
{
	using namespace LogicalFile;

	bool result = false;
	
	ModUnicodeString cstrValue;
	ModUnicodeChar optionalChar;
	
	TreeNodeInterface::Type eMatch =
		ParseValue::getStream(p, cstrValue, optionalChar);
	if (eMatch != TreeNodeInterface::Undefined)
	{
		result = true;
		
		cCond_.m_eMatch = eMatch;
		Data::Type::Value eKeyType = getKeyType(m_eKeyType, optionalChar);
		Common::Data::Pointer pData = createCommonData(eKeyType, cstrValue);
		dumpCommonData(cCond_.m_pBuffer, eKeyType, *pData);
		cCond_.m_OptionalChar = optionalChar;
	}
	return result;
}

//
//	FUNCTION private
//	Array::Condition::getPaddingChar --
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
	return (bNoPad_ == true) ? 0 : _paddingChar;
}

//
//	FUNCTION private
//	Array::Condition::getKeyType --
//
//	NOTES
//
//	ARGUMENTS
//	Data::Type::Value eKeyType_
//	ModUnicodeChar optionalChar_
//
//	RETURN
//
//	EXCEPTIONS
//
Data::Type::Value
Condition::getKeyType(Data::Type::Value eKeyType_,
					  ModUnicodeChar optionalChar_) const
{
	Data::Type::Value result = eKeyType_;
	// usOptionalChar_ has TWO applications.
	// One is an escape character for LIKE,
	// the other is a padding character for other than LIKE.
	// But the former situation is NOT considered in the folowing code.
	// In a current implementation,
	// LIKE does NOT care whether the type with NO PAD or without NO PAD.
	// So the code is NOT wrong. But it is NOT easily understandable.
	if (optionalChar_ != _paddingChar)
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
//	Array::Condition::getPositionOfTraillingSOH --
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
//	FUNCTION private
//	Array::Condition::getPositionOfFirstSmallCharacter --
//
//	NOTES
//
//		cstrString_	int
//		-----------------
//		\0			0
//		\n\0		0
//		\n\n\0		0
//
//		a\0			1
//		a\n\0		1
//		a\n\n\0		1
//
//		a\na\n\0	1
//		abc\0		3
//
//	ARGUMENTS
//	const ModUnicodeString& cstrString_
//
//	RETURN
//	int [0-base]
//		When the smaller character does NOT exist, return last position.
//		
//	EXCEPTIONS
//
int
Condition::getPositionOfFirstSmallCharacter(
	const ModUnicodeString& cstrString_) const
{
	const ModUnicodeChar* p = cstrString_;
	const ModUnicodeChar* s = p;
	const ModUnicodeChar* e = p + cstrString_.getLength();
	while (p != e)
	{
		if (*p < _paddingChar)
		{
			break;
		}
		++p;
	}

	return static_cast<int>(p - s);
}

//
//	Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
