// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenOption.cpp --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "KdTree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "KdTree/OpenOption.h"
#include "KdTree/FileID.h"

#include "Common/Configuration.h"

#include "FileCommon/OpenOption.h"
#include "FileCommon/DataManager.h"

#include "Exception/SQLSyntaxError.h"
#include "Exception/WrongParameter.h"

#include "ModUnicodeString.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
	// 探索タイプのデフォルト値
	Common::Configuration::ParameterString
	_cTraceType("KdTree_TraceType","normal");

	// 最大計算回数のデフォルト値
	Common::Configuration::ParameterInteger
	_cMaxCalculateCount("KdTree_MaxCalculateCount", 5000);

	// 結果件数のデフォルト値
	Common::Configuration::ParameterInteger
	_cSelectLimit("KdTree_SelectLimit", 0);
}

//
//	FUNCTION public
//	KdTree::OpenOption::OpenOption -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OpenOption::OpenOption(LogicalFile::OpenOption& cLogicalOpenOption_)
	: m_cOpenOption(cLogicalOpenOption_)
{
}

//
//	FUNCTION public
//	KdTree::OpenOption::~OpenOption -- デストラクタ
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
OpenOption::~OpenOption()
{
}

//
//	FUNCTION public
//	KdTree::OpenOption::getSearchParameter
//		-- TreeNodeから検索構文を作成する
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::FileID& cFileID_
//		ファイルID
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//
//	RETURN
//	bool
//		実行できる検索文の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::getSearchParameter(
	const KdTree::FileID& cFileID_,
	const LogicalFile::TreeNodeInterface* pCondition_)
{
	int iOpenMode = FileCommon::OpenOption::OpenMode::Unknown;

	if (pCondition_)
	{
		// 探索タイプ
		Node::TraceType::Value type = Node::TraceType::Unknown;
		// 最大計算回数
		int count = -1;

		//【注意】	現バージョンでは以下のSQL文のみサポート

		//
		//	neighbor(x hint '...') in (array[...], array[...], ...)
		//
		//	は
		//
		//	NeighborIn -- Option -- Hint
		//		|
		//	  Operand
		//		|
		//		+----------- Field(x)
		//		|
		//		+----------- Value(array[...])
		//		|
		//		+----------- Value(array[...])
		//		|
		//		...
		//
		//	となる
		//

		if (pCondition_->getType()
			!= LogicalFile::TreeNodeInterface::NeighborIn)
		{
			_SYDNEY_THROW1(Exception::SQLSyntaxError, pCondition_->getValue());
		}

		if (pCondition_->getOperandSize() <= 1)
		{
			_SYDNEY_THROW1(Exception::SQLSyntaxError,
						   pCondition_->getValue());
		}

		// オペランドをパースする
		parseCondition(cFileID_, pCondition_);

		// オプションをパースする
		if (pCondition_->getOptionSize())
		{
			if (pCondition_->getOptionSize() != 1)
			{
				// オプションは１つだけ
				_SYDNEY_THROW1(Exception::SQLSyntaxError,
							   pCondition_->getValue());
			}
			
			// ヒントを得る
			const LogicalFile::TreeNodeInterface* pHint
				= pCondition_->getOptionAt(0);
			if (pHint->getType()
				!= LogicalFile::TreeNodeInterface::Hint)
			{
				_SYDNEY_THROW1(Exception::SQLSyntaxError,
							   pCondition_->getValue());
			}

			// ヒントのオペランドを確認する
			if (pHint->getOperandSize() != 1)
			{
				// ヒントは１つだけ
				_SYDNEY_THROW1(Exception::SQLSyntaxError,
							   pCondition_->getValue());
			}

			// ヒントのオペランドを得る
			pHint = pHint->getOperandAt(0);

			// ヒントをパースする
			ModUnicodeString cHint = pHint->getValue();
			FileID::parseHint(cHint, type, count);
		}

		if (type == Node::TraceType::Unknown)
		{
			// 索引定義時の値を得る
			type = cFileID_.getTraceType();
		}
		if (count == -1)
		{
			// 索引定義時の値を得る
			count = cFileID_.getMaxCalculateCount();
		}
							  
		if (type != Node::TraceType::Unknown)
		{
			// 探索タイプ
			m_cOpenOption.setInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(KeyID::TraceType),
				type);
		}
		if (count != -1)
		{
			// 計算回数
			m_cOpenOption.setInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(KeyID::MaxCalculateCount),
				count);
		}

		// 検索
		iOpenMode = FileCommon::OpenOption::OpenMode::Search;
	}
	else
	{
		// 全件検索
		iOpenMode = FileCommon::OpenOption::OpenMode::Read;

		// 今のところは全件検索は未実装
		return false;
	}

	// オープンモード
	m_cOpenOption.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key),
		iOpenMode);

	// オブジェクトをドライバ側で保持する
	m_cOpenOption.setBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::CacheAllObject::Key),
		true);

	return true;
}

//
//	FUNCTION public
//	KdTree::OpenOption::getProjectionParameter
//		-- TreeNodeから取得フィールドを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::FileID& cFileID_
//		ファイルID
//	const LogicalFile::TreeNodeInterface* pNode_
//		取得フィールド指定
//
//	RETURN
//	bool
//		実行できる検索文の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::getProjectionParameter(
	const KdTree::FileID& cFileID_,
	const LogicalFile::TreeNodeInterface* pNode_)
{
	if (pNode_->getType() != LogicalFile::TreeNodeInterface::List)
		return false;
	
	// プロジェクションの指定はTreeNodeInterfaceで行う
	// 引数の pNode_ は TreeNodeInterface::List であり、
	// 以下のような構造になっている
	//
	//	List -- Operand --> Field
	//				|
	//				+-----> NeighborID --> Field
	//				|
	//				+-----> NeighborDistance --> Field

	int n = pNode_->getOperandSize();
	
	for (int i = 0; i < n; ++i)
	{
		const LogicalFile::TreeNodeInterface* p
			= pNode_->getOperandAt(i);

		switch (p->getType())
		{
		case LogicalFile::TreeNodeInterface::Field:
			// フィールド番号１のフィールドしか取得できない
			if (ModUnicodeCharTrait::toInt(p->getValue()) != 1)
				return false;

			// OpenOptionに設定する
			m_cOpenOption.setInteger(
				_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
					FileCommon::OpenOption::TargetFieldIndex::Key, i),
				Projection::ROWID);
			
			break;

		case LogicalFile::TreeNodeInterface::NeighborID:
			// 検索条件ID
			if (p->getOperandSize() != 1)
				return false;
			p = p->getOperandAt(0);
			if (p->getType() != LogicalFile::TreeNodeInterface::Field)
				return false;
			// フィールド番号０のフィールドしか設定できない
			if (ModUnicodeCharTrait::toInt(p->getValue()) != 0)
				return false;

			// OpenOptionに設定する
			m_cOpenOption.setInteger(
				_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
					FileCommon::OpenOption::TargetFieldIndex::Key, i),
				Projection::NeighborID);

			break;

		case LogicalFile::TreeNodeInterface::NeighborDistance:
			// 距離
			if (p->getOperandSize() != 1)
				return false;
			p = p->getOperandAt(0);
			if (p->getType() != LogicalFile::TreeNodeInterface::Field)
				return false;
			// フィールド番号０のフィールドしか設定できない
			if (ModUnicodeCharTrait::toInt(p->getValue()) != 0)
				return false;

			// OpenOptionに設定する
			m_cOpenOption.setInteger(
				_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
					FileCommon::OpenOption::TargetFieldIndex::Key, i),
				Projection::NeighborDistance);

			break;

		default:
			return false;
		}
	}

	// プロジェクションの数を設定する

	m_cOpenOption.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(
			FileCommon::OpenOption::TargetFieldNumber::Key), n);
			
	return true;
}

//
//	FUNCTION public
//	KdTree::OpenOption::getSortParameter
//		-- TreeNodeからソートを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::FileID& cFileID_
//		ファイルID
//	const LogicalFile::TreeNodeInterface* pNode_
//		ソート指定
//
//	RETURN
//	bool
//		実行できる検索文の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::getSortParameter(
	const KdTree::FileID& cFileID_,
	const LogicalFile::TreeNodeInterface* pNode_)
{
	//	pNode_ は以下のような構造
	//
	//	OrderBy - operand --> SortKey
	// 
	//	SortKey - operand --> NeighborDistance
	//			  option --> SortDirection (0: ASC, 1: DESC)
	//
	//	NeighborDistance - operand --> Field
	
	if (pNode_->getType() != LogicalFile::TreeNodeInterface::OrderBy ||
		pNode_->getOperandSize() != 1)
		// ソートキーは１つしか指定できない
		return false;
	
	const LogicalFile::TreeNodeInterface* p = pNode_->getOperandAt(0);
	if (p->getType() != LogicalFile::TreeNodeInterface::SortKey)
		return false;

	// 昇順 or 降順
	int order = 0;	// 昇順のみ
	if (p->getOptionSize() != 0 && p->getOptionSize() != 1)
		return false;
	if (p->getOptionSize() == 1)
		order = FileCommon::DataManager::toInt(p->getOptionAt(0));
	if (order != 0)
		return false;

	if (p->getOperandSize() != 1)
		return false;
	p = p->getOperandAt(0);
	if (p->getType() != LogicalFile::TreeNodeInterface::NeighborDistance ||
		p->getOperandSize() != 1)
		// 距離以外は指定できない
		return false;

	p = p->getOperandAt(0);
	if (p->getType() != LogicalFile::TreeNodeInterface::Field ||
		ModUnicodeCharTrait::toInt(p->getValue()) != 0)
		// フィールド番号 0 以外は指定できない
		return false;

	return true;
}

//
//	FUNCTION public
//	KdTree::OpenOption::getLimitParameter -- 取得数と取得位置を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::FileID& cFileID_
//		ファイルID
//	const Common::IntergerArrayData& cSpec_
//		取得数(0)と取得位置(1)
//
//	RETURN
//	bool
//		実行できる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::getLimitParameter(const KdTree::FileID& cFileID_,
							  const Common::IntegerArrayData& cSpec_)
{
	int limit;
	int offset = 1;

	if (cSpec_.getCount() == 0 || cSpec_.getCount() > 2)
		return false;

	limit = cSpec_.getElement(0);
	if (cSpec_.getCount() == 2)
		offset = cSpec_.getElement(1);

	if (limit < 0 || offset != 1)
		return false;

	m_cOpenOption.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SelectLimit), limit);

	return true;
}

//
//	FUNCTION public
//	KdTree::OpenOption::getCondition -- 検索条件を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<ModVector<float> >& vecCondition_
//		検索条件
//
//	RETURN
//	bool
//		検索条件がある場合にはtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::getCondition(ModVector<ModVector<float> >& vecCondition_) const
{
	bool r = false;

	int count = 0;
	if (m_cOpenOption.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(OpenOption::KeyID::ConditionCount),
			count))
	{
		vecCondition_.reserve(count);
		
		for (int i = 0; i < count; ++i)
		{
			ModUnicodeString condition;
			
			if (m_cOpenOption.getString(
					_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
						OpenOption::KeyID::Condition, i), condition))
			{
				// 文字列からfloatの配列を作成する

				ModVector<float> cond;
		
				ModUnicodeChar* p = &condition[0];

				if (*p != '[')
					_SYDNEY_THROW1(Exception::WrongParameter, condition);

				++p;

				ModUnicodeChar* s = p;
		
				while (*p)
				{
					switch (*p)
					{
					case ']':
					case ',':
					{
						*p = 0;
						float f = static_cast<float>(
							ModUnicodeCharTrait::toDouble(s));
						++p;
						s = p;
						cond.pushBack(f);
					}
					break;
					default:
						++p;
						break;
					}
				}

				vecCondition_.pushBack(cond);
				r = true;	// 検索条件がある
			}
		}
	}
	
	return r;
}

//
//	FUNCTION public
//	KdTree::OpenOption::getTraceType -- 探索タイプを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	KdTree::Node::TraceType::Value
//		探索タイプ
//
//	EXCEPTIONS
//
Node::TraceType::Value
OpenOption::getTraceType() const
{
	int type = 0;
	if (m_cOpenOption.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(OpenOption::KeyID::TraceType),
			type) == false)
	{
		// ないのでデフォルトを設定する

		type = FileID::castTraceType(_cTraceType.get());
	}

	return static_cast<Node::TraceType::Value>(type);
}

//
//	FUNCTION public
//	KdTree::OpenOption::getMaxCalculateCount -- 計算回数上限を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		検索回数上限
//
//	EXCEPTIONS
//
int
OpenOption::getMaxCalculateCount() const
{
	int count = 0;
	if (m_cOpenOption.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(OpenOption::KeyID::MaxCalculateCount),
			count) == false)
	{
		// ないのでデフォルトを設定する

		count = _cMaxCalculateCount.get();
	}

	return count;
}

//
//	FUNCTION public
//	KdTree::OpenOption::getSelectLimit -- 結果取得件数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		結果取得件数
//
//	EXCEPTIONS
//
ModSize
OpenOption::getSelectLimit() const
{
	int limit = 0;
	if (m_cOpenOption.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(OpenOption::KeyID::SelectLimit),
			limit) == false)
	{
		// ないのでデフォルトを設定する

		limit = _cSelectLimit.get();
	}

	return static_cast<ModSize>(limit);
}

//
//	FUNCTION public
//	KdTree::OpenOption::getProjection -- プロジェクションを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModVector<Projection::Value>
//		プロジェクション
//
//	EXCEPTIONS
//
ModVector<OpenOption::Projection::Value>
OpenOption::getProjection() const
{
	int count = m_cOpenOption.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(
			FileCommon::OpenOption::TargetFieldNumber::Key));

	ModVector<Projection::Value> projection;

	for (int i = 0; i < count; ++i)
	{
		Projection::Value v
			= static_cast<Projection::Value>(
				m_cOpenOption.getInteger(
					_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
						FileCommon::OpenOption::TargetFieldIndex::Key, i)));

		projection.pushBack(v);
	}
				
	return projection;
}

//
//	FUNCTION private
//	KdTree::OpenOption::parseOrCondition -- 複数の条件をパースする
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::FileID& cFileID_
//		ファイルID
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OpenOption::parseCondition(const KdTree::FileID& cFileID_,
						   const LogicalFile::TreeNodeInterface* pCondition_)
{
	int count = static_cast<int>(pCondition_->getOperandSize());
	
	int n = 0;
	for (int i = 0; i < count; ++i)
	{
		const LogicalFile::TreeNodeInterface* pSub
			= pCondition_->getOperandAt(i);

		if (pSub->getType() == LogicalFile::TreeNodeInterface::Field)
		{
			// 指定できるフィールドはキーのみ(0のみ)
		}
		else
		{
			// 1つの条件をパースする
			parseOneCondition(cFileID_, pSub, n++);
		}
	}

	// 検索条件数
	m_cOpenOption.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(OpenOption::KeyID::ConditionCount),
		n);
}

//
//	FUNCTION private
//	KdTree::OpenOption::parseOneCondition -- 一つの条件をパースする
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::FileID& cFileID_
//		ファイルID
//	const LogicalFile::TreeNodeInterface* pValue_
//		検索条件
//	int iElement_
//		設定する要素番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OpenOption::parseOneCondition(const KdTree::FileID& cFileID_,
							  const LogicalFile::TreeNodeInterface* pValue_,
							  int iElement_)
{
	if (pValue_->getType() != LogicalFile::TreeNodeInterface::ArrayConstructor)
	{
		_SYDNEY_THROW1(Exception::SQLSyntaxError, pValue_->getValue());
	}

	if (pValue_->getOperandSize() != cFileID_.getDimension())
	{
		_SYDNEY_THROW1(Exception::SQLSyntaxError, pValue_->getValue());
	}

	ModUnicodeOstrStream s;
	s << ModUnicodeChar('[');
	ModSize n = pValue_->getOperandSize();
	for (ModSize i = 0; i < n; ++i)
	{
		if (i != 0)
			s << ModUnicodeChar(',');
		s << pValue_->getOperandAt(i)->getValue();
	}
	s << ModUnicodeChar(']');

	// 検索条件を設定する
	m_cOpenOption.setString(
		_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(OpenOption::KeyID::Condition,
										  iElement_),
		ModUnicodeString(s.getString()));
		
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
