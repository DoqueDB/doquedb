// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// QueryNodeTerm.cpp --
// 
// Copyright (c) 2005, 2007, 2008, 2011, 2023 Ricoh Company, Ltd.
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
//	Bitmap::QueryNodeTerm::QueryNodeTerm -- コンストラクタ
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
QueryNodeTerm::QueryNodeTerm(BitmapFile& cFile_, const ModUnicodeChar* p_,
							 Condition& cCondition_)
	: QueryNode(Type::Term, p_, cCondition_), m_cFile(cFile_),
	  m_bConvert(false), m_uiPos(0)
{}

//
//	FUNCTION public
//	Bitmap::QueryNodeTerm::~QueryNodeTerm -- デストラクタ
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
QueryNodeTerm::~QueryNodeTerm()
{
	ModVector<BitmapIterator*>::Iterator i = m_vecpIterator.begin();
	for (; i != m_vecpIterator.end(); ++i)
	{
		delete *i;
	}
}

//
//	FUNCTION public
//	Bitmap::QueryNodeTerm::doValidate -- 有効化する
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
QueryNodeTerm::doValidate(const ModUnicodeChar*& p,
						  Condition& cCondition_,
						  ModUInt32& uiIteratorCount_,
						  bool bVerify_)
{
	// 検索条件を有効化する
	cCondition_.setQueryString(p);

	// B木を検索する
	m_cFile.search(&cCondition_);

	// ビットマップイテレータを得る
	setIterator(uiIteratorCount_, bVerify_);
}

//
//	FUNCTION public
//	Bitmap::QueryNodeTerm::next -- 次の結果を得る
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
QueryNodeTerm::next()
{
	Common::BitSet::UnitType unit;
	if (m_bConvert == true)
	{
		unit = m_cBitSet.getUnitType(m_uiPos);
		// m_uiPos has to be equals to pos in QueryNode::convert().
		++m_uiPos;
	}
	else
	{
		int n = Common::BitSet::UNIT_SIZE/sizeof(ModUInt32);
		ModVector<BitmapIterator*>::Iterator i = m_vecpIterator.begin();
		for (; i != m_vecpIterator.end(); ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				unit[j] |= (*i)->getNext();
			}
		}
	}
	return unit;
}

//
//	FUNCTION public
//	BitSet::QueryNodeTerm::seek -- 該当するUnitTypeまで移動する
//
//	NOTES
//
//	ARGUMENTS
// 	ModSize offset_
//		Common::BitSet::UnitType単位でのオフセット
//
// 	RETURN
//	なし
//
//	EXCEPTIONS
//
void
QueryNodeTerm::seek(ModSize offset_)
{
	// This is called only during a verification,
	// in such case, a conversion does not occur.
	; _TRMEISTER_ASSERT(m_bConvert == false);
	
	ModVector<BitmapIterator*>::Iterator i = m_vecpIterator.begin();
	for (; i != m_vecpIterator.end(); ++i)
	{
		// ここはModUInt32単位
		(*i)->seek(offset_ * (Common::BitSet::UNIT_SIZE / sizeof(ModUInt32)));
	}
}

//
//	FUNCTION public
//	Bitmap::QueryNodeTerm::reset --
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32&
//		[YET]
//
//	RETURN
//
//	EXCEPTIONS
//
void
QueryNodeTerm::reset(ModUInt32& uiIteratorCount_)
{
	ModVector<BitmapIterator*>::Iterator i = m_vecpIterator.begin();
	for (; i != m_vecpIterator.end(); ++i)
	{
		delete *i;
		; _TRMEISTER_ASSERT(uiIteratorCount_ > 0);
		--uiIteratorCount_;
	}
	// Using an clear command is NOT appropriate, because this may be reused.
	m_vecpIterator.erase(m_vecpIterator.begin(), m_vecpIterator.end());
}

//
//	FUNCTION public
//	Bitmap::QueryNodeTerm::convert --
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
QueryNodeTerm::convert(ModUInt32& uiIteratorCount_,
					   const Common::BitSet* pNarrowingBitSet_)
{
	; _TRMEISTER_ASSERT(m_bConvert == false);
	; _TRMEISTER_ASSERT(uiIteratorCount_ == QueryNode::getMaxIteratorCount());

	ModUInt32 uiMaxRowID = m_cFile.getHeaderPage().getMaxRowID();
	QueryNode::convert(&m_cBitSet, uiMaxRowID, uiIteratorCount_,
					   pNarrowingBitSet_);
	m_bConvert = true;
}

//
//	FUNCTION public
//	Bitmap::QueryNodeTerm::merge --
//
//	NOTES
//
//	ARGUMENTS
//	QueryNodeTerm* pTerm_
//		[YET]
//
//	RETURN
//
//	EXCEPTIONS
//
void
QueryNodeTerm::merge(const QueryNodeTerm* pOther_)
{
	; _TRMEISTER_ASSERT(m_bConvert == true);
	; _TRMEISTER_ASSERT(pOther_->m_bConvert == true);

	m_cBitSet &= pOther_->m_cBitSet;
}

//
//	FUNCTION private
//	Bitmap::QueryNodeTerm::setIterator --
//
//	NOTES
//
//	ARGUMENTS
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
QueryNodeTerm::setIterator(ModUInt32& uiIteratorCount_, bool bVerify_)
{
	m_uiPos = 0;
	bool bSuspend = false;
	
	ModUInt32 uiMaxIteratorCount = QueryNode::getMaxIteratorCount();
	if (uiIteratorCount_ == uiMaxIteratorCount && bVerify_ == false)
	{
		// After a convert, usually, uiIteratorCount is less than the maximum.
		// Becuase some iterators are freed in the convert.
		// But when the converted node has no iterator,
		// uiIteratorCount is equal to the maximum.
		// In such case, do NOT get any iterators.
		bSuspend = true;
	}
	else
	{
		BitmapIterator* i = 0;
		while ((i = m_cFile.getIterator()) != 0)
		{
			m_vecpIterator.pushBack(i);
			++uiIteratorCount_;
			
			if (uiIteratorCount_ == uiMaxIteratorCount && bVerify_ == false)
			{
				bSuspend = true;
				break;
			}
		}
	}
	setSuspend(bSuspend);
}

//
//	Copyright (c) 2005, 2007, 2008, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
