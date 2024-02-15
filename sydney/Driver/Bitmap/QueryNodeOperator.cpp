// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// QueryNodeOperator.cpp --
// 
// Copyright (c) 2005, 2008, 2023 Ricoh Company, Ltd.
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
#include "Bitmap/QueryNodeOperator.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

namespace
{
}

//
//	FUNCTION public
//	Bitmap::QueryNodeOperator::QueryNodeOperator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::BitmapFile& cFile_
//		ファイル
//	Bitmap::QueryNode::Type::Value eType_
//		タイプ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
QueryNodeOperator::QueryNodeOperator(BitmapFile& cFile_, Type::Value eType_,
									 const ModUnicodeChar* p_,
									 Condition& cCondition_)
	: QueryNode(eType_, p_, cCondition_), m_cFile(cFile_)
{}

//
//	FUNCTION public
//	Bitmap::QueryNodeOperator::~QueryNodeOperator -- デストラクタ
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
QueryNodeOperator::~QueryNodeOperator()
{
	ModVector<QueryNode*>::Iterator i = children.begin();
	for (; i != children.end(); ++i)
	{
		delete *i;
	}
}

//
//	FUNCTION public
//	Bitmap::QueryNodeOperator::doValidate -- 有効化する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar*& p
//		検索文字列
//	Bitmap::Condition& cCondition_
//		検索条件クラス
//	ModUInt32& uiIteratorCount_
//		Number of validated iterators under the node and the children
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
QueryNodeOperator::doValidate(const ModUnicodeChar*& p,
							  Condition& cCondition_,
							  ModUInt32& uiIteratorCount_,
							  bool bVerify_)
{
	// ...)

	while (*p != ')' &&
		   (uiIteratorCount_ < QueryNode::getMaxIteratorCount() ||
			bVerify_ == true))
	{
		// インスタンスを得る
		QueryNode* pNode = QueryNode::getQueryNode(p, m_cFile, cCondition_);
		// 有効化する
		pNode->doValidate(p, cCondition_, uiIteratorCount_, bVerify_);

		// 子ノードとして追加する
		children.pushBack(pNode);
	}

	bool bSuspend = false;
	if (uiIteratorCount_ < QueryNode::getMaxIteratorCount() || bVerify_ == true)
	{
		; _TRMEISTER_ASSERT(*p == ')');
		++p;
	}
	else
	{
		bSuspend = true;
	}
	setSuspend(bSuspend);
}

//
//	FUNCTION public
//	Bitmap::QueryNodeOperator::seek -- 指定の位置に移動する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize offset_
//		Common::BitSet::UnitType単位の先頭からのオフセット
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
QueryNodeOperator::seek(ModSize offset_)
{
	ModVector<QueryNode*>::Iterator i = children.begin();
	for (; i != children.end(); ++i)
	{
		(*i)->seek(offset_);
	}
}

//
//	Copyright (c) 2005, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

