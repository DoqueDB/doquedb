// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// QueryNodeAnd.cpp --
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
#include "SyDynamicCast.h"
#include "Bitmap/QueryNodeAnd.h"
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
//	Bitmap::QueryNodeAnd::QueryNodeAnd -- コンストラクタ
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
QueryNodeAnd::QueryNodeAnd(BitmapFile& cFile_, const ModUnicodeChar* p_,
						   Condition& cCondition_)
	: QueryNodeOperator(cFile_, Type::And, p_, cCondition_)
{}

//
//	FUNCTION public
//	Bitmap::QueryNodeAnd::~QueryNodeAnd -- デストラクタ
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
QueryNodeAnd::~QueryNodeAnd()
{}

//
//	FUNCTION public
//	Bitmap::QueryNodeAnd::doValidate -- 有効化する
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
//	bool bVerify_
//		[YET]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
QueryNodeAnd::doValidate(const ModUnicodeChar*& p,
						 Condition& cCondition_,
						 ModUInt32& uiIteratorCount_,
						 bool bVerify_)
{
	// #and(...)
	p += 5;
	// ...)
	QueryNodeOperator::doValidate(p, cCondition_, uiIteratorCount_, bVerify_);
}

//
//	FUNCTION public
//	Bitmap::QueryNodeAnd::next -- 次の結果を得る
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
QueryNodeAnd::next()
{
	Common::BitSet::UnitType unit;
	if (getSuspend() == false)
	{
		unit.flip();
		ModVector<QueryNode*>::Iterator i = children.begin();
		for (; i != children.end(); ++i)
		{
			; _TRMEISTER_ASSERT((*i)->getType() == Type::Term);
			; _TRMEISTER_ASSERT((*i)->getSuspend() == false);
			
			unit &= (*i)->next();
		}
	}
	return unit;
}

//
//	FUNCTION public
//	Bitmap::QueryNodeAnd::reset --
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
QueryNodeAnd::reset(ModUInt32& uiIteratorCount_)
{
	if (getSuspend() == false)
	{
		// Data is NOT able to be gotten from a suspended And node.
		// So, reset this only when it is NOT suspended.
		
		ModVector<QueryNode*>::Iterator i = children.begin();
		for (; i != children.end(); ++i)
		{
			; _TRMEISTER_ASSERT((*i)->getType() == Type::Term);
			; _TRMEISTER_ASSERT((*i)->getSuspend() == false);
			
			(*i)->reset(uiIteratorCount_);
			delete *i;
		}
		// NOT need to use an erace command, because this is NOT reused.
		children.clear();
	}
}

//
//	FUNCTION public
//	Bitmap::QueryNodeAnd::revalidate --
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
QueryNodeAnd::revalidate(const ModUnicodeChar*& p_,
						 Condition& cCondition_,
						 ModUInt32& uiIteratorCount_,
						 const Common::BitSet* pNarrowingBitSet_)
{
	; _TRMEISTER_ASSERT(getSuspend() == true);
	; _TRMEISTER_ASSERT(children.getSize() >= 1);

	ModVector<QueryNode*>::Iterator i = children.begin();

	// Convert a Term and merge them if necessary.
	if (uiIteratorCount_ == QueryNode::getMaxIteratorCount())
	{
		; _TRMEISTER_ASSERT((*i)->getType() == Type::Term);
		QueryNodeTerm* pTerm = _SYDNEY_DYNAMIC_CAST(QueryNodeTerm*, *i);
		
		if (pTerm->isConverted() == true)
		{
			; _TRMEISTER_ASSERT(children.getSize() >= 2);
		
			// Convert the next Term.
			; _TRMEISTER_ASSERT((*(i+1))->getType() == Type::Term);
			QueryNodeTerm* pNextTerm =
				_SYDNEY_DYNAMIC_CAST(QueryNodeTerm*, *(i+1));
			pNextTerm->convert(uiIteratorCount_, pNarrowingBitSet_);
			// Merge the Terms.
			pNextTerm->merge(pTerm);
			// Erase the merged Term.
			delete *i;
			children.erase(i);
		}
		else
		{
			// Convert the Term.
			pTerm->convert(uiIteratorCount_, pNarrowingBitSet_);
		}
	}

	// Restart to validate the Term which is suspended.
	i = children.end() - 1;
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
