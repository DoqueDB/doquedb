// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// QueryNodeOr.cpp --
// 
// Copyright (c) 2005, 2008, 2011, 2023 Ricoh Company, Ltd.
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
#include "Bitmap/QueryNodeOr.h"
#include "Bitmap/QueryNodeOr.h"
#include "Bitmap/QueryNodeTerm.h"
#include "Bitmap/Condition.h"
#include "Bitmap/BitmapFile.h"
#include "Bitmap/BitmapIterator.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

namespace
{
}

//
//	FUNCTION public
//	Bitmap::QueryNodeOr::QueryNodeOr -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::BitmapFile& cFile_
//		ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
QueryNodeOr::QueryNodeOr(BitmapFile& cFile_, const ModUnicodeChar* p_,
						 Condition& cCondition_)
	: QueryNodeOperator(cFile_, Type::Or, p_, cCondition_)
{}

//
//	FUNCTION public
//	Bitmap::QueryNodeOr::~QueryNodeOr -- デストラクタ
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
QueryNodeOr::~QueryNodeOr()
{}

//
//	FUNCTION public
//	Bitmap::QueryNodeOr::doValidate -- 有効化する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar*& p
//		検索文字列
//	Bitmap::Condition& cCondition_
//		検索条件クラス
//	ModUInt32& uiIteratorCount_
//		[YET]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
QueryNodeOr::doValidate(const ModUnicodeChar*& p,
						Condition& cCondition_,
						ModUInt32& uiIteratorCount_,
						bool bVerify_)
{
	// #or(...)
	p += 4;
	// ...)
	QueryNodeOperator::doValidate(p, cCondition_, uiIteratorCount_, bVerify_);
}

//
//	FUNCTION public
//	Bitmap::QueryNodeOr::next -- 次の結果を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::BitSet::UnitType
//		次の結果
//
//	EXCEPTIONS
//
Common::BitSet::UnitType
QueryNodeOr::next()
{
	Common::BitSet::UnitType unit;

	ModVector<QueryNode*>::Iterator i = children.begin();
	for (; i != children.end(); ++i)
	{
		if ((*i)->getSuspend() == true)
		{
			if ((*i)->getType() == Type::Term)
			{
				unit |= (*i)->next();
			}
			break;
		}

		unit |= (*i)->next();
	}
	return unit;
}

//
//	FUNCTION public
//	Bitmap::QueryNodeOr::reset --
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32& uiIteratorCount_
//		[YET]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
QueryNodeOr::reset(ModUInt32& uiIteratorCount_)
{
	ModVector<QueryNode*>::Iterator i = children.begin();
	for (; i != children.end(); ++i)
	{
		; _TRMEISTER_ASSERT((*i)->getType() == Type::Term
							|| (*i)->getType() == Type::And);
		
		if ((*i)->getSuspend() == true)
		{
			if ((*i)->getType() == Type::Term)
			{
				// Data is able to be gotten from Term,
				// even if the Term is suspended.
				(*i)->reset(uiIteratorCount_);
				
				// After revalidating, Data is gotten from the Term again.
				// So, NOT increment the iterator.
			}
			break;
		}
		
		(*i)->reset(uiIteratorCount_);
		delete *i;
	}
	// Erase the iterators just before the suspended one.
	children.erase(children.begin(), i);
}

//
//	FUNCTION public
//	Bitmap::QueryNodeOr::revalidate --
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
QueryNodeOr::revalidate(const ModUnicodeChar*& p_,
						Condition& cCondition_,
						ModUInt32& uiIteratorCount_,
						const Common::BitSet* pNarrowingBitSet_)
{
	; _TRMEISTER_ASSERT(getSuspend() == true);

	// インスタンスを得る
	; _TRMEISTER_ASSERT(children.getSize() == 1);
	ModVector<QueryNode*>::Iterator i = children.begin();
	// 有効化する
	(*i)->revalidate(p_, cCondition_, uiIteratorCount_, pNarrowingBitSet_);

	// Not need to check the suspension, it will be checked in doValidate().

	// This function is not called in a verification.
	// See QueryNode::validate for detail.

	QueryNodeOperator::doValidate(p_, cCondition_, uiIteratorCount_, false);
}

//
//	Copyright (c) 2005, 2008, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
