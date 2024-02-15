// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FieldMask.cpp -- 指定されたフィールドの組み合わせが正しいか確認するクラス
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/FieldMask.h"

#include "FullText2/FileID.h"
#include "FullText2/OpenOption.h"

#include "FileCommon/DataManager.h"

#include "Exception/BadArgument.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace {
}

//
//	全文索引のフィールドは以下のようになっている。
//
//	0-: String or StringArray		文字列(複合索引の場合にはその数分)
//	1: Language or LanguageArray	言語情報
//	2: Double						スコア調整
//	3: UnsignedInteger				ROWID
//
//	文字列フィールドに対してのみ関数が利用できるが、
//	同時に指定できる関数は制限される
//
//	NORMAL
//		OpenOption::Function::Score
//		OpenOption::Function::Section
//		OpenOption::Function::Tf
//		OpenOption::Function::Existence
//		OpenOption::Function::ClusterID
//		OpenOption::Function::FeatureValue
//		OpenOption::Function::RoughKwicPosition
//
//	WORD
//		OpenOption::Function::Word
//		OpenOption::Function::WordDf
//		OpenOption::Function::WordScale
//
//	LENGTH
//		OpenOption::Function::AverageLength
//		OpenOption::Function::AverageCharLength
//		OpenOption::Function::AverageWordCount
//		OpenOption::Function::Count
//

//
//	FUNCTION public
//	FullText2::FieldMask::FieldMask -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::FileID& cFileID_
//		ファイルID
//
//	RETURN
//
//	EXCEPTIONS
//
FieldMask::FieldMask(const FileID& cFileID_)
	: m_cFileID(cFileID_),
	  m_rowid(FieldType::RowID), m_score(-1),
	  m_eGetType(GetType::None),
	  m_bScan(false), m_bOnlyRowID(false)
{
	// フィールドをチェックするためのマスクを生成
	// プロジェクションパラメータの確認には、OpenOptionを引数に持つものを
	// 利用すること

	m_iKeyCount = cFileID_.getKeyCount();
	int k = m_iKeyCount;
	while (--k != 0)
		shift();

	if (cFileID_.isLanguage())
		shift();
	
	if (cFileID_.isScoreField())
	{
		m_score = m_rowid;
		shift();
	}
}

//
//	FUNCTION public
//	FullText2::FieldMask::FieldMask -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::FileID& cFileID_
//		ファイルID
//	LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
FieldMask::FieldMask(const FileID& cFileID_,
					 LogicalFile::OpenOption& cOpenOption_)
	: m_cFileID(cFileID_),
	  m_rowid(FieldType::RowID), m_score(-1),
	  m_eGetType(GetType::None),
	  m_bScan(true), m_bOnlyRowID(false)
{
	//
	// データ取得方法、Bitsetかどうかを設定
	//
	
	if (cOpenOption_.getBoolean(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::GetByBitSet::Key)) == true)
	{
		m_bOnlyRowID = true;
	}

	//
	// 検索方法、SCANかどうかを設定
	//
	
	int tmp;
	if (cOpenOption_.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::OpenMode::Key), tmp) == true)
	{
		if (tmp == FileCommon::OpenOption::OpenMode::Search)
		{
			m_bScan = false;
		}
	}

	//
	// 検索タイプ
	//

	if (m_bScan == false)
	{
		if (cOpenOption_.getInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(OpenOption::KeyID::SearchType),
				tmp) == true)
		{
			if (tmp == OpenOption::Type::Equal)
			{
				// 整合性検査のための検索なので、取得できるのはROWIDのみ
				m_bOnlyRowID = true;
			}
		}
	}

	//
	// 非固定フィールドに応じて、固定フィールドの位置を移動
	//

	m_iKeyCount = cFileID_.getKeyCount();
	int k = m_iKeyCount;
	while (--k != 0)
		shift();

	if (cFileID_.isLanguage())
		shift();
	
	if (cFileID_.isScoreField())
	{
		m_score = m_rowid;
		shift();
	}
}

//
//	FUNCTION public
//	FullText2::FieldMask::check
//		-- 同時に取得できるかどうかを確認し、可能ならOpenOptionに設定する
//
//	NOTES
//
//	ARGUMENTS
//	LogicalFile::TreeNodeInterface* pNode_
//		取得する情報を表すノード
//	const ModVector<int>& vecField_
//		検索対象のフィールド
//	int& iField_
//	   	プロジェクション対象のフィールド番号をビット位置にした場合の論理和
//		フィールド指定できない関数の場合は -1 を設定
//	FullText2::OpenOption::Function::Value& eFunc_
//		関数
//	ModUnicodeString& cParam_
//		関数のオプション引数
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
FieldMask::check(const LogicalFile::TreeNodeInterface* pNode_,
				 const ModVector<int>& vecField_,
				 int& iField_,
				 OpenOption::Function::Value& eFunc_,
				 ModUnicodeString& cParam_)
{
	// まずは、同時に取得できる組み合わせかどうかを確認する
	
	switch (pNode_->getType())
	{
	case LogicalFile::TreeNodeInterface::Field:
	case LogicalFile::TreeNodeInterface::Score:
	case LogicalFile::TreeNodeInterface::Tf:
	case LogicalFile::TreeNodeInterface::Existence:
	case LogicalFile::TreeNodeInterface::ClusterID:
	case LogicalFile::TreeNodeInterface::RoughKwicPosition:
	case LogicalFile::TreeNodeInterface::Section:
	case LogicalFile::TreeNodeInterface::FeatureValue:
		// 通常
		
		if (m_eGetType == GetType::None)
			m_eGetType = GetType::Normal;
		else if (m_eGetType != GetType::Normal)
			// できない組み合わせ
			return false;
		
		break;
		
	case LogicalFile::TreeNodeInterface::Word:
	case LogicalFile::TreeNodeInterface::WordDf:
	case LogicalFile::TreeNodeInterface::WordScale:
		// ワード
		
		if (m_eGetType == GetType::None)
			m_eGetType = GetType::Word;
		else if (m_eGetType != GetType::Word)
			// できない組み合わせ
			return false;
		
		break;
		
	case LogicalFile::TreeNodeInterface::Avg:
	case LogicalFile::TreeNodeInterface::Count:
		// 固定値
		
		if (m_eGetType == GetType::None)
			m_eGetType = GetType::Length;
		else if (m_eGetType != GetType::Length)
			// できない組み合わせ
			return false;
		
		break;

	default:
		// その他は無理
		return false;
	}

	// 次に、それぞれの指定方法が正しいか確認する
	
	switch (pNode_->getType())
	{
	case LogicalFile::TreeNodeInterface::Field:
		{
			// 取得できるフィールドはROWIDのみ
			int f = FileCommon::DataManager::toInt(pNode_);
			if (f != m_rowid)
				return false;

			iField_ = 1 << f;
			eFunc_ = OpenOption::Function::RowID;
			cParam_.clear();
		}
		break;
		
	case LogicalFile::TreeNodeInterface::Score:
		{
			// フィールド指定は、vecField_と一致している必要あり
			if (checkFieldExact(pNode_, vecField_) == false)
				return false;

			iField_ = -1;
			eFunc_ = OpenOption::Function::Score;
			cParam_.clear();
		}
		break;
	case LogicalFile::TreeNodeInterface::Tf:
		{
			// フィールド指定は、vecField_と一致している必要あり
			if (checkFieldExact(pNode_, vecField_) == false)
				return false;

			iField_ = -1;
			eFunc_ = OpenOption::Function::Tf;
			cParam_.clear();
		}
		break;
	case LogicalFile::TreeNodeInterface::Existence:
		{
			// フィールド指定は、vecField_と一致している必要あり
			if (checkFieldExact(pNode_, vecField_) == false)
				return false;

			iField_ = -1;
			eFunc_ = OpenOption::Function::Existence;
			cParam_.clear();
		}
		break;
	case LogicalFile::TreeNodeInterface::ClusterID:
		{
			// フィールド指定は、vecField_と一致している必要あり
			if (checkFieldExact(pNode_, vecField_) == false)
				return false;

			iField_ = -1;
			eFunc_ = OpenOption::Function::ClusterID;
			cParam_.clear();
		}
		break;
		
	case LogicalFile::TreeNodeInterface::RoughKwicPosition:
		{
			// フィールド指定は、vecField_のどれか１つと一致している
			int f = 0;
			if (checkFieldOne(pNode_, vecField_, f) == false)
				return false;

			iField_ = 1 << f;
			eFunc_ = OpenOption::Function::RoughKwicPosition;

			cParam_.clear();
			if (pNode_->getOptionSize())
			{
				// 引数があるので加える
				
				if (pNode_->getOptionSize() != 1)
					return false;
				const LogicalFile::TreeNodeInterface* p
					= pNode_->getOptionAt(0);
				if (p->getType() != LogicalFile::TreeNodeInterface::KwicSize)
					return false;
				
				cParam_ = p->getValue();
			}
		}
		break;
	case LogicalFile::TreeNodeInterface::Section:
		{
			// フィールド指定は、vecField_のどれか１つと一致している
			int f = 0;
			if (checkFieldOne(pNode_, vecField_, f) == false)
				return false;

			iField_ = 1 << f;
			eFunc_ = OpenOption::Function::Section;
			cParam_.clear();
		}
		break;
		
	case LogicalFile::TreeNodeInterface::FeatureValue:
		{
			// 任意のキー(1つのみ)
			int f = 0;
			if (checkFieldAny(pNode_, f) == false)
				return false;

			iField_ = 1 << f;
			eFunc_ = OpenOption::Function::FeatureValue;
			cParam_.clear();
		}
		break;
		
	case LogicalFile::TreeNodeInterface::Word:
		{
			// フィールド指定は、vecField_と一致している必要あり
			int f = 0;
			if (checkFieldExact(pNode_, vecField_) == false)
				return false;

			iField_ = -1;
			eFunc_ = OpenOption::Function::Word;
			cParam_.clear();
		}
		break;
	case LogicalFile::TreeNodeInterface::WordDf:
		{
			// フィールド指定は、vecField_と一致している必要あり
			int f = 0;
			if (checkFieldExact(pNode_, vecField_) == false)
				return false;

			iField_ = -1;
			eFunc_ = OpenOption::Function::WordDf;
			cParam_.clear();
		}
		break;
	case LogicalFile::TreeNodeInterface::WordScale:
		{
			// フィールド指定は、vecField_と一致している必要あり
			int f = 0;
			if (checkFieldExact(pNode_, vecField_) == false)
				return false;

			iField_ = -1;
			eFunc_ = OpenOption::Function::WordScale;
			cParam_.clear();
		}
		break;
		
	case LogicalFile::TreeNodeInterface::Avg:
		{
			if (pNode_->getOperandSize() != 1)
				return false;
			const LogicalFile::TreeNodeInterface* n = pNode_->getOperandAt(0);

			switch (n->getType())
			{
			case LogicalFile::TreeNodeInterface::FullTextLength:
				// 単語単位索引なら平均単語数、そうでないなら平均文書長
				eFunc_ = OpenOption::Function::AverageLength;
				break;
			case LogicalFile::TreeNodeInterface::CharLength:
				// 平均文書長
				if (m_cFileID.getIndexingType() == IndexingType::Word)
					// 単語単位索引では平均文書長は取得できない
					return false;
				eFunc_ = OpenOption::Function::AverageCharLength;
				break;
			case LogicalFile::TreeNodeInterface::WordCount:
				// 平均単語数
				if (m_cFileID.getIndexingType() != IndexingType::Word)
					// 単語単位索引じゃないと平均単語数は取得できない
					return false;
				eFunc_ = OpenOption::Function::AverageWordCount;
				break;
			default:
				return false;
			}
			
			// 任意のキー
			ModVector<int> vecField;
			if (getField(n, vecField) == false)
				return false;
			int f = 0;
			ModVector<int>::Iterator i = vecField.begin();
			for (; i != vecField.end(); ++i)
				f |= (1 << (*i));

			iField_ = f;
			cParam_.clear();
		}
		break;
		
	case LogicalFile::TreeNodeInterface::Count:
		{
			// 任意のキー
			ModVector<int> vecField;
			if (getField(pNode_, vecField) == false)
				return false;
			int f = 0;
			ModVector<int>::Iterator i = vecField.begin();
			for (; i != vecField.end(); ++i)
				f |= (1 << (*i));

			iField_ = f;
			eFunc_ = OpenOption::Function::Count;
			cParam_.clear();
		}
		break;
		
	}

	return true;
}

//
//	FUNCTION public
//	FullText2::FieldMask::checkValueRangeValidity
//		-- 更新対象のフィールド番号は適切な範囲内か？
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		調べるフィールド番号
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
FieldMask::checkValueRangeValidity(int n_) const
{
	// 更新できるのは ROWID より小さいフィールドのみ

	return (n_ < m_rowid) ? true : false;
}

//
//	FUNCTION private
//	FullText2::FieldMask::shift -- 仮想フィールドの位置を移動
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
void
FieldMask::shift()
{
	// 範囲を移動
	m_rowid++;
}

//
//	FUNCTION private
//	FullText2::FieldMask::checkFieldExact
//		-- 検索対象のフィールドと同じ指定がされているかチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pNode_
//		関数ノード
//	const ModVector<int>& vecField_
//		検索対象のフィールド
//
//	RETURN
//	bool
//		正しい指定がされている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FieldMask::checkFieldExact(const LogicalFile::TreeNodeInterface* pNode_,
						   const ModVector<int>& vecField_)
{
	//【注意】vecField_ は昇順にソートされていること
	
	ModVector<int> v;
	if (getField(pNode_, v) == false)
		return false;
	ModSort(v.begin(), v.end(), ModLess<int>());	// 昇順にソートする

	if (v.getSize() != vecField_.getSize())
		return false;

	bool r = true;
	
	ModVector<int>::ConstIterator i0 = vecField_.begin();
	ModVector<int>::Iterator i1 = v.begin();

	for (; i0 != vecField_.end(); ++i0, ++i1)
	{
		if ((*i0) != (*i1))
		{
			r = false;
			break;
		}
	}

	return r;
}

//
//	FUNCTION private
//	FullText2::FieldMask::checkFieldOne
//		-- 検索対象のフィールドのどれかが指定されているかチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pNode_
//		関数ノード
//	const ModVector<int>& vecField_
//		検索対象のフィールド
//	int& iField_
//		指定されているフィールド番号
//
//	RETURN
//	bool
//		正しい指定がされている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FieldMask::checkFieldOne(const LogicalFile::TreeNodeInterface* pNode_,
						 const ModVector<int>& vecField_,
						 int& iField_)
{
	ModVector<int> v;
	if (getField(pNode_, v) == false)
		return false;

	if (v.getSize() != 1)
		return false;

	bool r = false;

	iField_ = (*v.begin());
	ModVector<int>::ConstIterator i = vecField_.begin();
	for (; i != vecField_.end(); ++i)
	{
		if (iField_ == (*i))
		{
			r = true;
			break;
		}
	}

	return r;
}

//
//	FUNCTION private
//	FullText2::FieldMask::checkFieldAny
//		-- 任意の1つのキーが指定されているかチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pNode_
//		関数ノード
//	int& iField_
//		指定されているフィールド番号
//
//	RETURN
//	bool
//		正しい指定がされている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FieldMask::checkFieldAny(const LogicalFile::TreeNodeInterface* pNode_,
						 int& iField_)
{
	ModVector<int> v;
	if (getField(pNode_, v) == false)
		return false;

	if (v.getSize() != 1)
		return false;

	iField_ = (*v.begin());

	return true;
}

//
//	FUNCTION private
//	FullText2::FieldMask::getField -- フィールド指定を得る
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pNode_
//		検索ノード (関数ノードを想定)
//	ModVector<int>& vecField_
//		指定されているフィールド
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
FieldMask::getField(const LogicalFile::TreeNodeInterface* pNode_,
					ModVector<int>& vecField_)
{
	int n = static_cast<int>(pNode_->getOperandSize());
	for (int i = 0; i < n; ++i)
	{
		const LogicalFile::TreeNodeInterface* pField = pNode_->getOperandAt(i);
		
		if (pField->getType() != LogicalFile::TreeNodeInterface::Field)
			return false;

		int f = FileCommon::DataManager::toInt(pField);
		if (f >= m_iKeyCount)
			return false;
		
		vecField_.pushBack(f);
	}
	return true;
}

//
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
