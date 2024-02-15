// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenOption.cpp --
// 
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "Bitmap/OpenOption.h"

#include "FileCommon/OpenOption.h"

#include "Common/Assert.h"
#include "Common/UnicodeString.h"
#include "Common/Message.h"

#include "Exception/InvalidEscape.h"

#include "ModUnicodeCharTrait.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

namespace {
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
//	Bitmap::OpenOption::OpenOption -- コンストラクタ
//
//	NOTES
//
//	ARGUMETNS
//	const Bitmap::FileID& cFileID_
//		ファイルID
//	LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	なし
//
//	EXCPETIONS
//
OpenOption::OpenOption(const FileID& cFileID_,
					   LogicalFile::OpenOption& cOpenOption_)
	: m_cFileID(cFileID_), m_cOpenOption(cOpenOption_)
{
}

//
//	FUNCTION public
//	Bitmap::OpenOption::~OpenOption -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
// 	RETURN
//	なし
//
//	EXCEPTIONS
//
OpenOption::~OpenOption()
{
}

//
//	FUNCTION public
//	Bitmap::OpenOption::getSearchParameter
//		-- TreeNodeを解析し、検索条件をOpenOptionに設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		TreeNodeの検索条件
//
//	RETURN
//	bool
//		実行できる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::getSearchParameter(
	const LogicalFile::TreeNodeInterface* pCondition_)
{
	using namespace LogicalFile;

	if (pCondition_)
	{
		ModUnicodeOstrStream cStream;

		if (parseOrNode(pCondition_, cStream) == false)
		{
			// verifyかも
			cStream.clear();
			ModUnicodeString cstrRowID;
			if (parseVerifyNode(pCondition_, cStream, cstrRowID))
			{
				// verifyだ
				m_cOpenOption.setBoolean(
					_SYDNEY_OPEN_PARAMETER_KEY(Key::Verify),
					true);
				m_cOpenOption.setString(
					_SYDNEY_OPEN_PARAMETER_KEY(Key::RowID),
					cstrRowID);
			}
			else
			{
				return false;
			}
		}

		// オープンオプションに設定する
		m_cOpenOption.setString(_SYDNEY_OPEN_PARAMETER_KEY(Key::Condition),
								cStream.getString());

	}
	
	// オブジェクトをドライバ側で保持する
	m_cOpenOption.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
								 FileCommon::OpenOption::CacheAllObject::Key),
							 true);

	// オープンモードを設定する
	m_cOpenOption.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								 FileCommon::OpenOption::OpenMode::Key),
							 FileCommon::OpenOption::OpenMode::Read);
	
	return true;
}

//
//	FUNCTION public
//	Bitmap::OpenOption::getProjectionParameter
//		-- TreeNodeを解析し、プロジェクション情報をOpenOptionに設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pNode_
//		TreeNodeのプロジェクション情報
//
//	RETURN
//	bool
//		実行できる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::getProjectionParameter(
	const LogicalFile::TreeNodeInterface* pNode_)
{
	using namespace LogicalFile;

	if (pNode_ == 0)
		return false;

	// 取得するフィールド数
	int n = 0;

	// フィールド選択指定を設定する
	if (pNode_->getType() == TreeNodeInterface::List)
	{
		// List -- Operand --> Field
		// 複数のフィールドが指定されているのは group by の場合のみ
		// 取得できるフィールドは、キーとROWID

		n = static_cast<int>(pNode_->getOperandSize());
		if (n != 2 && n != 1)
			return false;
		
		if (n == 2)
		{
			// 2つの場合は以下のチェック

			if (m_cOpenOption.getBoolean(
						_SYDNEY_OPEN_PARAMETER_KEY(
							FileCommon::OpenOption::GroupBy::Key)) == false) {
				return false;
			}

			for (int i = 0; i < n; ++i)
			{
				const TreeNodeInterface* pOperand = pNode_->getOperandAt(i);

				if (pOperand->getType() != TreeNodeInterface::Field)
					_TRMEISTER_THROW0(Exception::BadArgument);

				int field = ModUnicodeCharTrait::toInt(pOperand->getValue());
				if (field != i)
					// フィールドの指定は必ずキー(0)、ROWID(1)の順番
					return false;

				m_cOpenOption.setInteger(
					_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
						FileCommon::OpenOption::TargetFieldIndex::Key, i),
					field);
			}
		}
		else
		{
			// 1つの場合は以下のチェック
			
			const TreeNodeInterface* pOperand = pNode_->getOperandAt(0);

			if (pOperand->getType() != TreeNodeInterface::Field)
				_TRMEISTER_THROW0(Exception::BadArgument);

			int field = ModUnicodeCharTrait::toInt(pOperand->getValue());
			if (field != 1)
				// ROWIDのみ取得可能
				return false;

			m_cOpenOption.setInteger(
				_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
					FileCommon::OpenOption::TargetFieldIndex::Key, 0),
				field);
		}
	}
	else if (pNode_->getType() == TreeNodeInterface::Field)
	{
		// Field
		// １つのフィールドしか指定されていない

		n = 1;

		int field = ModUnicodeCharTrait::toInt(pNode_->getValue());
		if (field != 1)
			// ROWIDのみ取得可能
			return false;

		m_cOpenOption.setInteger(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				FileCommon::OpenOption::TargetFieldIndex::Key, 0), field);
	}
	
	// フィールド選択がされている、という事を設定する
	m_cOpenOption.setBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::FieldSelect::Key),
		true);

	// 取得するフィールド数
	m_cOpenOption.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(
			FileCommon::OpenOption::TargetFieldNumber::Key), n);
	
	// オブジェクトをドライバ側で保持する
	m_cOpenOption.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
								 FileCommon::OpenOption::CacheAllObject::Key),
							 true);

	// オープンモードを設定する
	m_cOpenOption.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								 FileCommon::OpenOption::OpenMode::Key),
							 FileCommon::OpenOption::OpenMode::Read);

	return true;
}

//
//	FUNCTION public
//	Bitmap::OpenOption::getSortParameter
//		-- TreeNodeを解析し、ソート情報をOpenOptionに設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pNode_
//		TreeNodeのソート情報
//
//	RETURN
//	bool
//		実行できる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::getSortParameter(
	const LogicalFile::TreeNodeInterface* pNode_)
{
	using namespace LogicalFile;

	if (pNode_ == 0)
		return false;

	// ビットマップ索引の getSortParameter は gorup by の指定のしかできない
	// ビットマップ索引の検索は QueryNode から検索結果集合を順次取り出す処理
	// なので、ソートすることはできない
	// そのため、通常のソート指定は未サポート
	//
	// OrderBy -- Operand --> SortKey -- Operand --> Field
	//		 |				    |------- Option ---> SortDirection
	//		 |											0: ASC
	//		 |											1: DESC
	//		 |------- Option ---> GroupBy
	
	if (pNode_->getType() != TreeNodeInterface::OrderBy ||
		pNode_->getOperandSize() != 1 || pNode_->getOptionSize() != 1)
		return false;
	
	const TreeNodeInterface* pOption = pNode_->getOptionAt(0);
	if (pOption->getType() != TreeNodeInterface::GroupBy)
		return false;

	const TreeNodeInterface* pSortKey = pNode_->getOperandAt(0);
	if (pSortKey->getType() != TreeNodeInterface::SortKey ||
		pSortKey->getOperandSize() != 1)
		return false;

	const TreeNodeInterface* pField = pSortKey->getOperandAt(0);
	if (pField->getType() != TreeNodeInterface::Field)
		return false;

	if (ModUnicodeCharTrait::toInt(pField->getValue()) != 0)
		// キー以外のフィールドでは group by できない
		return false;

	int iSortDirection = 0;
	if (pSortKey->getOptionSize() == 1)
	{
		// オプションがあるので、ソート順を調べる
		const TreeNodeInterface* pSortDirection = pSortKey->getOptionAt(0);
		if (ModUnicodeCharTrait::toInt(pSortDirection->getValue()) != 0)
			iSortDirection = 1;
	}

	// 指定された group by は実行可能なので、OpenOptionに設定する
	m_cOpenOption.setBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::GroupBy::Key), true);
	m_cOpenOption.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(Key::SortOrder), iSortDirection);

	// 検索結果をすべてためてしまう
	m_cOpenOption.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
								 FileCommon::OpenOption::CacheAllObject::Key),
							 true);
	
	// オープンモードを設定する
	m_cOpenOption.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								 FileCommon::OpenOption::OpenMode::Key),
							 FileCommon::OpenOption::OpenMode::Read);

	return true;
}

//
//	FUNCTION private
//	Bitmap::OpenOption::parseOrNode -- ORをパースし文字列にする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ModUnicodeOstrStream& cStream_
//		設定するストリーム
//
//	RETURN
//		実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::parseOrNode(const LogicalFile::TreeNodeInterface* pCondition_,
						ModUnicodeOstrStream& cStream_)
{
	using namespace LogicalFile;

	int n = 0;
	int size = 1;

	const TreeNodeInterface* pNode = pCondition_;
	if (pNode->getType() == TreeNodeInterface::Or)
	{
		cStream_ << "#or(";
		size = static_cast<int>(pNode->getOperandSize());
		pNode = pCondition_->getOperandAt(n);
	}
	
	while (n != size)
	{
		// 1つ1つパースする
		if (parseAndNode(pNode, cStream_) == false)
		{
			return false;
		}

		if (++n != size)
			pNode = pCondition_->getOperandAt(n);
	}

	if (size != 1)
	{
		cStream_ << ")";
	}

	return true;
}

//
//	FUNCTION private
//	Bitmap::OpenOption::parseAndNode -- ANDをパースし文字列にする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ModUnicodeOstrStream& cStream_
//		設定するストリーム
//
//	RETURN
//		実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::parseAndNode(const LogicalFile::TreeNodeInterface* pCondition_,
						 ModUnicodeOstrStream& cStream_)
{
	using namespace LogicalFile;

	int n = 0;
	int size = 1;

	const TreeNodeInterface* pNode = pCondition_;
	if (m_cFileID.isArray() &&
		pNode->getType() == TreeNodeInterface::And)
	{
		cStream_ << "#and(";
		size = static_cast<int>(pNode->getOperandSize());
		pNode = pCondition_->getOperandAt(n);
	}
	
	while (n != size)
	{
		// 1つ1つパースする
		if (parseTreeNode(pNode, cStream_) == false)
		{
			return false;
		}

		if (++n != size)
			pNode = pCondition_->getOperandAt(n);
	}
	if (size != 1)
	{
		cStream_ << ")";
	}

	return true;
}

//
//	FUNCTION private
//	Bitmap::OpenOption::parseTreeNode -- TreeNodeをパースし文字列にする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ModUnicodeOstrStream& cStream_
//		設定するストリーム
//
//	RETURN
//	bool
//		実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::parseTreeNode(const LogicalFile::TreeNodeInterface* pCondition_,
						  ModUnicodeOstrStream& cStream_)
{
	using namespace LogicalFile;
	
	ParseValue* pMain = 0;
	ParseValue* pOther = 0;

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
		if (parseOneNode(pNode, pMain, pOther) == false)
		{
			delete pMain;
			delete pOther;
			return false;
		}

		if (++n != size)
			pNode = pCondition_->getOperandAt(n);
	}

	// ストリームに設定する
	if (setToStream(pMain, pOther, cStream_) == false)
	{
		delete pMain;
		delete pOther;
		return false;
	}
	
	delete pMain;
	delete pOther;
	return true;
}

//
//	FUNCTION private
//	Bitmap::OpenOption::parseOneNode -- 1つのノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ParseValue*& pMain_
//		主検索条件
//	ParseValue*& pOther_
//		副検索条件
//
//	RETURN
//	bool
//		実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::parseOneNode(const LogicalFile::TreeNodeInterface* pCondition_,
						 ParseValue*& pMain_,
						 ParseValue*& pOther_)
{
	using namespace LogicalFile;

	bool result = false;
	
	switch (pCondition_->getType())
	{
	case TreeNodeInterface::Equals:
		result = parseEqualsNode(pCondition_, pMain_, pOther_);
		break;
	case TreeNodeInterface::LessThan:
	case TreeNodeInterface::LessThanEquals:
		result = parseLessThanNode(pCondition_, pMain_, pOther_);
		break;
	case TreeNodeInterface::GreaterThan:
	case TreeNodeInterface::GreaterThanEquals:
		result = parseGreaterThanNode(pCondition_, pMain_, pOther_);
		break;
	case TreeNodeInterface::NotEquals:
		result = parseNotEqualsNode(pCondition_, pMain_, pOther_);
		break;
	case TreeNodeInterface::EqualsToNull:
		result = parseEqualsToNullNode(pCondition_, pMain_, pOther_);
		break;
	case TreeNodeInterface::Like:
		result = parseLikeNode(pCondition_, pMain_, pOther_);
		break;
	default:
		;
	}

	return result;
}

//
//	FUNCTION private
//	Bitmap::OpenOption::parseEqualsNode -- Equalsノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ParseValue*& pMain_
//		主検索条件
//	ParseValue*& pOther_
//		副検索条件
//
//	RETURN
//	bool
//		実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::parseEqualsNode(const LogicalFile::TreeNodeInterface* pCondition_,
							ParseValue*& pMain_,
							ParseValue*& pOther_)
{
	using namespace LogicalFile;

	if (pMain_ != 0 && pMain_->m_eType == Data::MatchType::Unknown)
		return true;

	Data::MatchType::Value eMatch = Data::MatchType::Equals;
	ModUnicodeString cstrValue;
	bool isValid;
	bool bNoPadKey;
	if (checkTwoTerm(pCondition_, eMatch, cstrValue, isValid, bNoPadKey)
		== false)
		return false;
	bool bNoPadField = checkNoPadSortOrder(
		m_cFileID.getKeyType(), m_cFileID.isFixed(), bNoPadKey);

	if (isValid == false)
	{
		// nullとの比較はつねにunknown
		setUnknownParseValue(pMain_, pOther_);
		return true;
	}

	ParseValue* pNew = new ParseValue;
	pNew->m_eType = eMatch;
	pNew->m_cValue = cstrValue;
	pNew->m_usOptionalChar = getPaddingChar(bNoPadField || bNoPadKey);

	setEqualsParseValue(pNew, pMain_, pOther_, bNoPadField, bNoPadKey);
	
	return true;
}

//
//	FUNCTION private
//	Bitmap::OpenOption::parseLessThanNode -- LessThanノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ParseValue*& pMain_
//		主検索条件
//	ParseValue*& pOther_
//		副検索条件
//
//	RETURN
//	bool
//		実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::parseLessThanNode(const LogicalFile::TreeNodeInterface* pCondition_,
							  ParseValue*& pMain_,
							  ParseValue*& pOther_)
{
	using namespace LogicalFile;

	if (pMain_ != 0 && pMain_->m_eType == Data::MatchType::Unknown)
		return true;

	Data::MatchType::Value eMatch
		= (pCondition_->getType() == TreeNodeInterface::LessThan) ?
		Data::MatchType::LessThan : Data::MatchType::LessThanEquals;
	ModUnicodeString cstrValue;
	bool isValid;
	bool bNoPadKey;
	if (checkTwoTerm(pCondition_, eMatch, cstrValue, isValid, bNoPadKey)
		== false)
		return false;
	bool bNoPadField = checkNoPadSortOrder(
		m_cFileID.getKeyType(), m_cFileID.isFixed(), bNoPadKey);

	if (isValid == false)
	{
		// nullとの比較はつねにunknown
		setUnknownParseValue(pMain_, pOther_);
		return true;
	}

	ParseValue* pNew = new ParseValue;
	pNew->m_eType = eMatch;
	pNew->m_cValue = cstrValue;
	pNew->m_usOptionalChar = getPaddingChar(bNoPadField || bNoPadKey);

	if (eMatch == Data::MatchType::LessThan ||
		eMatch == Data::MatchType::LessThanEquals)
	{
		setLessThanParseValue(pNew, pMain_, pOther_,
							  bNoPadField, bNoPadKey);
	}
	else
	{
		; _SYDNEY_ASSERT(eMatch == Data::MatchType::GreaterThan
						 || eMatch == Data::MatchType::GreaterThanEquals);
		
		// Field and Value were interchanged in checkTwoTerm().
		setGreaterThanParseValue(pNew, pMain_, pOther_,
								 bNoPadField, bNoPadKey);
	}

	return true;
}

//
//	FUNCTION private
//	Bitmap::OpenOption::parseGreaterThanNode -- GreaterThanノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ParseValue*& pMain_
//		主検索条件
//	ParseValue*& pOther_
//		副検索条件
//
//	RETURN
//	bool
//		実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::parseGreaterThanNode(
	const LogicalFile::TreeNodeInterface* pCondition_,
	ParseValue*& pMain_,
	ParseValue*& pOther_)
{
	using namespace LogicalFile;

	if (pMain_ != 0 && pMain_->m_eType == Data::MatchType::Unknown)
		return true;

	Data::MatchType::Value eMatch
		= (pCondition_->getType() == TreeNodeInterface::GreaterThan) ?
		Data::MatchType::GreaterThan : Data::MatchType::GreaterThanEquals;
	ModUnicodeString cstrValue;
	bool isValid;
	bool bNoPadKey;
	if (checkTwoTerm(pCondition_, eMatch, cstrValue, isValid, bNoPadKey)
		== false)
		return false;
	bool bNoPadField = checkNoPadSortOrder(
		m_cFileID.getKeyType(), m_cFileID.isFixed(), bNoPadKey);

	if (isValid == false)
	{
		// nullとの比較はつねにunknown
		setUnknownParseValue(pMain_, pOther_);
		return true;
	}

	ParseValue* pNew = new ParseValue;
	pNew->m_eType = eMatch;
	pNew->m_cValue = cstrValue;
	pNew->m_usOptionalChar = getPaddingChar(bNoPadField || bNoPadKey);

	if (eMatch == Data::MatchType::GreaterThan ||
		eMatch == Data::MatchType::GreaterThanEquals)
	{
		setGreaterThanParseValue(pNew, pMain_, pOther_,
								 bNoPadField, bNoPadKey);
	}
	else
	{
		; _SYDNEY_ASSERT(eMatch == Data::MatchType::LessThan
						 || eMatch == Data::MatchType::LessThanEquals);

		// Field and Value were interchanged in checkTwoTerm().
		setLessThanParseValue(pNew, pMain_, pOther_,
							  bNoPadField, bNoPadKey);
	}

	return true;
}

//
//	FUNCTION private
//	Bitmap::OpenOption::parseNotEqualsNode -- NotEqualsノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
// 	ParseValue*& pMain_
//		主検索条件
//	ParseValue*& pOther_
//		副検索条件
//
//	RETURN
//	bool
//		実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::parseNotEqualsNode(
	const LogicalFile::TreeNodeInterface* pCondition_,
	ParseValue*& pMain_,
	ParseValue*& pOther_)
{
	using namespace LogicalFile;

	if (pMain_ != 0 && pMain_->m_eType == TreeNodeInterface::Unknown)
		return true;

	Data::MatchType::Value eMatch = Data::MatchType::NotEquals;
	ModUnicodeString cstrValue;
	bool isValid;
	bool bNoPadKey;
	if (checkTwoTerm(pCondition_, eMatch, cstrValue, isValid, bNoPadKey)
		== false)
		return false;
	bool bNoPadField = checkNoPadSortOrder(
		m_cFileID.getKeyType(), m_cFileID.isFixed(), bNoPadKey);

	if (isValid == false)
	{
		// nullとの比較はつねにunknown
		setUnknownParseValue(pMain_, pOther_);
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
	pNew->m_pNext = pOther_;
	pOther_ = pNew;

	return true;
}

//
//	FUNCTION private
//	Bitmap::OpenOption::parseEqualsToNullNode -- EqualsToNullノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ParseValue*& pMain_
//		主検索条件
//	ParseValue*& pOther_
//		副検索条件
//
//	RETURN
//	bool
//		実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::parseEqualsToNullNode(
	const LogicalFile::TreeNodeInterface* pCondition_,
	ParseValue*& pMain_,
	ParseValue*& pOther_)
{
	using namespace LogicalFile;

	if (pMain_ != 0 && pMain_->m_eType == Data::MatchType::Unknown)
		return true;

	Data::MatchType::Value eMatch = Data::MatchType::EqualsToNull;
	if (checkOneTerm(pCondition_, eMatch) == false)
		return false;

	ParseValue* pNew = new ParseValue;
	pNew->m_eType = eMatch;

	if (pMain_ != 0)
	{
		if (!pMain_->isEquals())
		{
			// equal検索が一番強いのでそれ以外の検索条件はpOther_へ
			ParseValue* p = pMain_;
			while (p->m_pNext) p = p->m_pNext;
			p->m_pNext = pOther_;
			pOther_ = pMain_;
			pMain_ = pNew;
		}
		else
		{
			// もうequalがあるのでpOther_へ
			pNew->m_pNext = pOther_;
			pOther_ = pNew;
		}
	}
	else
	{
		// 初めて
		pMain_ = pNew;
	}

	return true;
}

//
//	FUNCTION private
//	Bitmap::OpenOption::parseLikeNode -- Likeノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ParseValue*& pMain_
//		主検索条件
//  ParseValue*& pOther_
//		副検索条件
//
//	RETURN
//	bool
//		実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::parseLikeNode(
	const LogicalFile::TreeNodeInterface* pCondition_,
	ParseValue*& pMain_,
	ParseValue*& pOther_)
{
	using namespace LogicalFile;

	if (pMain_ != 0 && pMain_->m_eType == Data::MatchType::Unknown)
		return true;

	Data::MatchType::Value eMatch = Data::MatchType::Like;
	ModUnicodeString cstrValue;
	bool isValid;
	bool bNoPadKey;
	if (checkTwoTerm(pCondition_, eMatch, cstrValue, isValid, bNoPadKey)
		== false)
		return false;
	// Like is alwyas used with NO PAD.
	bNoPadKey = true;

	// Check the field.
	if (m_cFileID.getKeyType() != Data::Type::CharString &&
		m_cFileID.getKeyType() != Data::Type::UnicodeString &&
		m_cFileID.getKeyType() != Data::Type::NoPadCharString &&
		m_cFileID.getKeyType() != Data::Type::NoPadUnicodeString)
	{
		// Like supports only string type.
		return false;
	}
	bool bNoPadField = checkNoPadSortOrder(
		m_cFileID.getKeyType(), m_cFileID.isFixed(), bNoPadKey);

	// Check the condition again.
	if (isValid == false)
	{
		// nullとの比較はつねにunknown
		setUnknownParseValue(pMain_, pOther_);
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
		pNew->m_eType = Data::MatchType::Equals;
		pNew->m_cValue = cstrLower;
		pNew->m_usOptionalChar = getPaddingChar(bNoPadField || bNoPadKey);

		setEqualsParseValue(pNew, pMain_, pOther_, bNoPadField, bNoPadKey);

		// The remains of the condition does NOT exist.
		; _SYDNEY_ASSERT(*p == 0);
	}
	else
	{
		// 前方一致

		ParseValue* pNew1 = new ParseValue;
		pNew1->m_eType = Data::MatchType::GreaterThanEquals;
		pNew1->m_cValue = cstrLower;
		pNew1->m_usOptionalChar = getPaddingChar(bNoPadField || bNoPadKey);
		
		ParseValue* pNew2 = new ParseValue;
		pNew2->m_eType = Data::MatchType::LessThan;
		pNew2->m_cValue = cstrUpper;
		pNew2->m_usOptionalChar = pNew1->m_usOptionalChar;
		pNew1->m_pNext = pNew2;

		setPrefixMatchParseValue(pNew1, pMain_, pOther_,
								 bNoPadField, bNoPadKey);

		if (*p != 0 || bNoPadField == false && bNoPadKey == true)
		{
			// The reminds of the condition exist.
			setLikeParseValue(cstrValue, usEscape, pOther_);
		}
	}

	return true;
}

//
//	FUNCTION private
//	Bitmap::OpenOption::parseVerifyNode -- verifyのときの検索ノードを解析する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ModUnicodeOstrStream& cStream_
//		検索条件(キーだけ)
//	ModUnicodeString& cRowID_
//		ROWID
//
//	RETURN
//	bool
//		実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::parseVerifyNode(const LogicalFile::TreeNodeInterface* pCondition_,
							ModUnicodeOstrStream& cStream_,
							ModUnicodeString& cRowID_)
{
	using namespace LogicalFile;

	//
	//	整合性検査のためのequalの検索は以下のようになっている
	//

	// 単一文書の場合
	// AND ---- Equals -------- Field(0)
	//	 |			|
	//	 |			+---------- ConstantValue/Variable
	//	 |
	//	 +----- Equals -------- Field(1)
	//				|
	//				+---------- ConstantValue/Variable("1") ← ROWID=1の場合
	// 配列の場合
	// AND ---- Equals -------- Field(0)
	//	 |			|
	//	 |			+---------- List -------- ConstantValue/Variable
	//	 |						  |
	//	 |						  +---------- ConstantValue/Variable
	//	 |
	//	 +---- Equals -------- Field(1)
	//			   |
	//			   +---------- ConstantValue/Variable("1") ← ROWID=1の場合

	if (pCondition_->getType() != TreeNodeInterface::And
		|| pCondition_->getOperandSize() != 2)
		return false;

	for (int i = 0; i < static_cast<int>(pCondition_->getOperandSize()); ++i)
	{
		const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);

		if (pNode->getType() != TreeNodeInterface::EqualsToNull
			&& (pNode->getType() != TreeNodeInterface::Equals
				|| pNode->getOperandSize() != 2))
			return false;
		
		const TreeNodeInterface* pField = pNode->getOperandAt(0);
		const TreeNodeInterface* pValue = pNode->getOperandAt(1);

		int key = ModUnicodeCharTrait::toInt(pField->getValue());

		if (key == 0)
		{
			ModUnicodeChar usOptionalChar = 0;
			if (m_cFileID.getKeyType() == Data::Type::CharString ||
				m_cFileID.getKeyType() == Data::Type::UnicodeString)
			{
				// The collation of the field is PAD SPACE.
				usOptionalChar = getPaddingChar(false);
			}

			// キー
			if (pNode->getType() == TreeNodeInterface::EqualsToNull)
			{
				if (m_cFileID.isArray())
					cStream_ << "#eq(#na)";
				else
					cStream_ << "#eq(#nl)";
			}
			else if (m_cFileID.isArray())
			{
				if (pValue->getType() != TreeNodeInterface::List)
					return false;
				
				cStream_ << "#and(";
				int size = pValue->getOperandSize();
				for (int i = 0; i < size; ++i)
				{
					cStream_ << "#eq(";
					if (pValue->getOperandAt(i) == 0 ||
						(pValue->getOperandAt(i)->getData() &&
						 pValue->getOperandAt(i)->getData()->isNull()))
					{
						cStream_ << "#nl";
					}
					else
					{
						cStream_ << "#eq";
						ParseValue::putStreamValue(
							cStream_,
							pValue->getOperandAt(i)->getValue(),
							usOptionalChar);
					}
					cStream_ << ")";
				}
				cStream_ << ")";
			}
			else
			{
				if (pValue->getType() == TreeNodeInterface::List)
					return false;
				
				cStream_ << "#eq(#eq";
				ParseValue::putStreamValue(
					cStream_,
					pValue->getValue(),
					usOptionalChar);
				cStream_ << ")";
			}
		}
		else if (key == 1)
		{
			// ROWID
			cRowID_ = pValue->getValue();
		}
		else
			return false;
	}

	return true;
}

//
//	FUNCTION private
//	Bitmap::OpenOption::checkOneTerm -- 単項演算をチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		単項演算のノード
//
//	RETURN
//	bool
//		Bitmapで検索できないものの場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::checkOneTerm(const LogicalFile::TreeNodeInterface* pCondition_,
						 Data::MatchType::Value& eMatch_)
{
	using namespace LogicalFile;
	
	bool result = false;
	if (pCondition_->getOperandSize() == 1)
	{
		const TreeNodeInterface* pField = pCondition_->getOperandAt(0);
		
		if (pField->getType() == TreeNodeInterface::Field)
		{
			// 今は isNull の時にしかこの関数は呼ばれない
			// よって、配列で任意要素指定がなくてもエラーにしない

			if (!m_cFileID.isArray() &&
				pField->getOptionSize() &&
				pField->getOptionAt(0)->getType()
				== TreeNodeInterface::All)
				return false;

			if (m_cFileID.isArray() &&
				(pField->getOptionSize() == 0 ||
				 pField->getOptionAt(0)->getType() != TreeNodeInterface::All))
				// 配列で任意要素指定ではない
				eMatch_ = Data::MatchType::EqualsToNull_All;
			
			if (ModUnicodeCharTrait::toInt(pField->getValue()) == 0)
			{
				result = true;
			}
		}
	}
	return result;
}

//
//	FUNCTION private
//	Bitmap::OpenOption::checkTwoTerm -- 2項演算をチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		2項演算のノード
//	Bitmap::Data::MatchType::Value& eMatch_
//		一致条件
//	ModUnicodeString& cstrValue_
//		データ
//	bool& isValid_
//		有効な条件かどうか
//
//	RETURN
//	bool
//		Bitmapで検索できないものの場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::checkTwoTerm(const LogicalFile::TreeNodeInterface* pCondition_,
						 Data::MatchType::Value& eMatch_,
						 ModUnicodeString& cstrValue_,
						 bool& isValid_,
						 bool& bNoPadKey_)
{
	using namespace LogicalFile;
	
	bool result = false;
	isValid_ = false;
	bNoPadKey_ = false;
	
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
			int field = ModUnicodeCharTrait::toInt(pField->getValue());
			if (field == 0)
			{
				// キーは0だけ
				if (eMatch_ != Data::MatchType::Like)
				{
					// likeじゃないときには、
					// 配列の場合は任意要素指定が必要
					// 配列じゃない場合は任意要素指定は不可
					// となる
					
					if ((m_cFileID.isArray() &&
						(pField->getOptionSize() == 0 ||
						 pField->getOptionAt(0)->getType()
						 != TreeNodeInterface::All)) ||
						(!m_cFileID.isArray() &&
						 pField->getOptionSize() &&
						 pField->getOptionAt(0)->getType()
						 == TreeNodeInterface::All))
						return false;
				}
				else
				{
					// likeのときは、
					// 配列の場合は「要素指定なし」か「任意要素指定」
					// 配列じゃない場合は「要素指定なし」
					// となる
					if ((m_cFileID.isArray() &&
						 pField->getOptionSize() &&
						 pField->getOptionAt(0)->getType()
						 != TreeNodeInterface::All) ||
						(!m_cFileID.isArray() &&
						 pField->getOptionSize() &&
						 (pField->getOptionAt(0)->getType()
						  == TreeNodeInterface::All ||
						  pField->getOptionAt(0)->getType()
						  == TreeNodeInterface::ConstantValue)))
						return false;
				}
				
				const Common::Data* pData = pValue->getData();
				if (pData != 0 && !pData->isNull())
				{
					result = true;
					if (m_cFileID.getKeyType() == Data::Type::Integer
						&& (pData->getType() == Common::DataType::Double
							|| pData->getType() == Common::DataType::Integer64))
					{
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
					else if (m_cFileID.getKeyType() == Data::Type::Integer64
							 && pData->getType() == Common::DataType::Double)
					{
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
					else if ((m_cFileID.getKeyType()
							  == Data::Type::UnicodeString
							  || m_cFileID.getKeyType()
							  == Data::Type::CharString)
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
					else if (m_cFileID.getKeyType()
							 == Data::Type::NoPadUnicodeString ||
							 m_cFileID.getKeyType()
							 == Data::Type::NoPadCharString)
					{
						// The collation of the field is NO PAD.
						cstrValue_ = pValue->getValue();
						isValid_ = true;
						bNoPadKey_ = true;

						// BACKWARD COMPATIBILITY
						//
						// The field of varchar created by v1
						// is sorted with NO PAD.
						//
						// It is difficult to resort the data with PAD SPACE.
						// So this case is treated as a KNOWN BUG.
						// And the below procedure is for simulation of v1.
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
						// It's different from the result using v1.
						//
						if (m_cFileID.checkVersion(FileID::Version2) == false &&
							m_cFileID.isFixed() == false &&
							pData->getType() == Common::DataType::String)
						{
							if (eMatch_ == Data::MatchType::GreaterThan ||
								eMatch_ == Data::MatchType::GreaterThanEquals ||
								eMatch_ == Data::MatchType::LessThan ||
								eMatch_ == Data::MatchType::LessThanEquals)
							{
								// When using inequality in v16.3,
								// this conpatibility occurs a different result
								// between with an index and without an index.
								// It's undesirable.
								// So, in such situation, NOT use the index.
								// For your information, v16.2 does NOT create
								// a plan for an inequality using the index.
								
								SydInfoMessage
									<< "Warning: an inequality operator is"
									<< " not processed by the bitmap index"
									<< " which is created by an old module."
									<< ModEndl;
								
								return false;
							}
							else if (eMatch_ != Data::MatchType::Like)
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
					}
					else if (m_cFileID.getKeyType() == Data::Type::Decimal)
					{
						int iPrecision;
						int iScale;
						Data::Decimal::getParameter(
							m_cFileID, 0, m_cFileID.isArray(),
							iPrecision, iScale);
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
	return result;
}

//
//	FUNCTION private
//	Bitmap::OpenOption::alternateTerm -- Alternate two terms.
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pFirst_
//		First term
//	const LogicalFile::TreeNodeInterface* pSecond_
//		Second term
//	Data::MatchType::Value& eMatch_
//		Comparison operator
//
//	RETURN
//
//	EXCEPTIONS
//
void
OpenOption::alternateTerm(const LogicalFile::TreeNodeInterface*& pFirst_,
						  const LogicalFile::TreeNodeInterface*& pSecond_,
						  Data::MatchType::Value& eMatch_)
{
	using namespace LogicalFile;
	
	switch (eMatch_)
	{
	case Data::MatchType::Equals:
	case Data::MatchType::NotEquals:
		break;
	case Data::MatchType::GreaterThan:
		eMatch_ = Data::MatchType::LessThan;
		break;
	case Data::MatchType::GreaterThanEquals:
		eMatch_ = Data::MatchType::LessThanEquals;
		break;
	case Data::MatchType::LessThan:
		eMatch_ = Data::MatchType::GreaterThan;
		break;
	case Data::MatchType::LessThanEquals:
		eMatch_ = Data::MatchType::GreaterThanEquals;
		break;
	case Data::MatchType::Like:
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
//	Btree2::OpenOption::checkNoPadSortOrder --
//		Check whether the sort order of the field is NO PAD
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
OpenOption::checkNoPadSortOrder(Data::Type::Value eType_,
								bool bFixedField_,
								bool bNoPadKey_) const
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
//	Bitmap::OpenOption::setEqualsParseValue --
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
OpenOption::setEqualsParseValue(ParseValue* pNew_,
								ParseValue*& pMain_,
								ParseValue*& pOther_,
								bool bNoPadField_,
								bool bNoPadKey_)
{
	using namespace LogicalFile;
	
	if (pMain_ != 0)
	{
		if (pMain_->m_eType != Data::MatchType::Equals
			&& pMain_->m_eType != Data::MatchType::EqualsToNull)
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
//	Bitmap::OpenOption::setEqualsParseValueWithSortOrder --
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
OpenOption::setEqualsParseValueWithSortOrder(ParseValue* pNew1_,
											 ParseValue*& pMain_,
											 ParseValue*& pOther_,
											 bool bNoPadField_,
											 bool bNoPadKey_)
{
	; _SYDNEY_ASSERT(pNew1_ != 0 &&
					 pNew1_->m_eType == Data::MatchType::Equals);
	
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
//	Bitmap::OpenOption::setLessThanParseValue --
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
OpenOption::setLessThanParseValue(ParseValue* pNew_,
								  ParseValue*& pMain_,
								  ParseValue*& pOther_,
								  bool bNoPadField_,
								  bool bNoPadKey_)
{
	if (pMain_ != 0)
	{
		if ((pMain_->m_eType == Data::MatchType::GreaterThan
			|| pMain_->m_eType == Data::MatchType::GreaterThanEquals)
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
//	Bitmap::OpenOption::setLessThanParseValueWithSortOrder --
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
OpenOption::setLessThanParseValueWithSortOrder(ParseValue* pNew1_,
											   ParseValue*& pMain_,
											   ParseValue*& pOther_,
											   bool bNoPadField_,
											   bool bNoPadKey_)
{
	using namespace LogicalFile;

	; _SYDNEY_ASSERT(pNew1_ != 0 &&
					 (pNew1_->m_eType == Data::MatchType::LessThan ||
					  pNew1_->m_eType == Data::MatchType::LessThanEquals));
	
	if (bNoPadField_ == false && bNoPadKey_ == true)
	{
		// The sort order of f is PAD SPACE,
		// The collation of the key is NO PAD,
		// f < 'abc\nxyz'
		// -> f <= 'abc' with PAD SPACE & f < 'abc\nxyz' with NO PAD.
		
		// Set the ParseValue with PAD SPACE in the main condition.
		ParseValue* pNew2 = new ParseValue;
		pNew2->m_eType = Data::MatchType::LessThanEquals;
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
//	Bitmap::OpenOption::setGreaterThanParseValue --
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
OpenOption::setGreaterThanParseValue(ParseValue* pNew_,
									 ParseValue*& pMain_,
									 ParseValue*& pOther_,
									 bool bNoPadField_,
									 bool bNoPadKey_)
{
	if (pMain_ != 0)
	{
		if (pMain_->m_eType == Data::MatchType::LessThan
			|| pMain_->m_eType == Data::MatchType::LessThanEquals)
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
//	Btree2::OpenOption::setGreaterThanParseValueWithSortOrder --
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
OpenOption::setGreaterThanParseValueWithSortOrder(ParseValue* pNew1_,
												  ParseValue*& pMain_,
												  ParseValue*& pOther_,
												  bool bNoPadField_,
												  bool bNoPadKey_)
{
	using namespace LogicalFile;

	; _SYDNEY_ASSERT(pNew1_ != 0 &&
					 (pNew1_->m_eType == Data::MatchType::GreaterThan ||
					  pNew1_->m_eType == Data::MatchType::GreaterThanEquals));
	
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
//	Btree2::OpenOption::setPrefixMatchParseValue --
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
OpenOption::setPrefixMatchParseValue(ParseValue* pNew1_,
									 ParseValue*& pMain_,
									 ParseValue*& pOther_,
									 bool bNoPadField_,
									 bool bNoPadKey_)
{
	using namespace LogicalFile;
	
	if (pMain_ != 0)
	{
		if (pMain_->m_eType == Data::MatchType::Equals
			|| pMain_->m_eType == Data::MatchType::EqualsToNull)
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
//	Btree2::OpenOption::setPrefixMatchParseValueWithSortOrder --
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
OpenOption::setPrefixMatchParseValueWithSortOrder(ParseValue* pNew1_,
												  ParseValue*& pParseValue_,
												  bool bNoPadField_,
												  bool bNoPadKey_)
{
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
				pNew1_->m_eType	= Data::MatchType::GreaterThanEquals);
			pNew1_->m_eType = Data::MatchType::GreaterThan;
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
//	Btree2::OpenOption::setLikeParseValue --
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
OpenOption::setLikeParseValue(ModUnicodeString& cstrValue_,
							  ModUnicodeChar usEscape_,
							  ParseValue*& pOther_)
{
	using namespace LogicalFile;
	
	ParseValue* pLike = new ParseValue;
	pLike->m_eType = Data::MatchType::Like;
	pLike->m_cValue = cstrValue_;
	pLike->m_usOptionalChar = usEscape_;
	
	pLike->m_pNext = pOther_;
	pOther_ = pLike;
}

//
//	FUNCTION private
//	Bitmap::OpenOption::setUnknownParseValue --
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
OpenOption::setUnknownParseValue(ParseValue*& pMain_,
								 ParseValue*& pOther_)
{
	ParseValue* pNew = new ParseValue;
	pNew->m_eType = Data::MatchType::Unknown;
	delete pMain_, pMain_ = 0;
	delete pOther_, pOther_ = 0;
	pMain_ = pNew;
}

//
//	FUNCTION private
//	Bitmap::OpenOption::setToStream -- ストリームに設定する
//
//	NOTES
//
//	ARGUMENTS
//	ParseValue* pMain_
//		主検索条件
//	ParseValue* pOthrer_
//		副検索条件
//	ModUnicodeOstrStream& cStream_
//		設定する文字列
//
//	RETURN
//	bool
//		実行できる検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::setToStream(ParseValue* pMain_,
						ParseValue* pOther_,
						ModUnicodeOstrStream& cStream_)
{
	using namespace LogicalFile;

	if (pMain_)
	{
		switch (pMain_->m_eType)
		{
		case Data::MatchType::Equals:
		case Data::MatchType::EqualsToNull:
		case Data::MatchType::Unknown:
		case Data::MatchType::EqualsToNull_All:
			cStream_ << "#eq(";
			pMain_->putStream(cStream_);
			cStream_ << ')';
			break;
		case Data::MatchType::GreaterThan:
		case Data::MatchType::GreaterThanEquals:
			cStream_ << "#ge(";
			pMain_->putStream(cStream_);
			cStream_ << ')';
			if (pMain_->m_pNext)
			{
				pMain_ = pMain_->m_pNext;
				cStream_ << "#le(";
				pMain_->putStream(cStream_);
				cStream_ << ')';
			}
			break;
		case Data::MatchType::LessThan:
		case Data::MatchType::LessThanEquals:
			cStream_ << "#le(";
			pMain_->putStream(cStream_);
			cStream_ << ')';
			break;
		default:
			return false;
		}
	}

	if (pOther_)
	{
		// その他条件
		cStream_ << "#ot(";
		ParseValue* p = pOther_;
		while (p)
		{
			p->putStream(cStream_);
			p = p->m_pNext;
		}
		cStream_ << ')';
	}

	return true;
}

//
//	FUNCTION private
//	Bitmap::OpenOption::getPaddingChar --
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
OpenOption::getPaddingChar(bool bNoPad_) const
{
	return (bNoPad_ == true) ? 0 : _usPaddingChar;
}

//
//	FUNCTION private
//	Btree2::OpenOption::getPositionOfTraillingSOH --
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
OpenOption::getPositionOfTraillingSOH(const ModUnicodeString& cstrString_) const
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
//	Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
