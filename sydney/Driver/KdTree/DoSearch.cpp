// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DoSearch.cpp --
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
#include "KdTree/DoSearch.h"

#include "KdTree/Entry.h"
#include "KdTree/KdTreeIndex.h"

#include "Common/Configuration.h"
#include "Common/Message.h"

#include "Os/AutoCriticalSection.h"
#include "Os/Limits.h"

#include "ModPair.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
}

//
//	FUNCTION public
//	KdTree::DoSearch::DoSearch -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::KdTreeIndex* pMain_
//	const KdTree::KdTreeIndex* pSmall1_
//	const KdTree::KdTreeIndex* pSmall2_
//		KD-Tree索引
//	const ModVector<ModVector<float> >& vecCondition_
//		検索条件
//	Node::Status* pMainStatus_
//	Node::Status* pSmall1Status_
//	Node::Status* pSmall2Status_
//		検索状態クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
DoSearch::DoSearch(const KdTreeIndex* pMain_,
				   const KdTreeIndex* pSmall1_,
				   const KdTreeIndex* pSmall2_,
				   const ModVector<ModVector<float> >& vecCondition_,
				   Node::Status* pMainStatus_,
				   Node::Status* pSmall1Status_,
				   Node::Status* pSmall2Status_)
	: m_pMain(pMain_), m_pSmall1(pSmall1_), m_pSmall2(pSmall2_),
	  m_vecCondition(vecCondition_), m_pMainStatus(pMainStatus_),
	  m_pSmall1Status(pSmall1Status_), m_pSmall2Status(pSmall2Status_),
	  m_iElement(0)
{
	m_vecResult.assign(vecCondition_.getSize());
}

//
//	FUNCTION public
//	KdTree::DoSearch::~DoSearch -- デストラクタ
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
DoSearch::~DoSearch()
{
}

//
//	FUNCTION public
//	KdTree::DoSearch::parallel -- 平行実行する
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
void
DoSearch::parallel()
{
	// 検索結果を格納する場所
	ModVector<ModVector<ModPair<ModUInt32, double> > >::Iterator result;
	
	// 検索条件を得る
	Entry* pCondition;
	while ((pCondition = getNext(result)) != 0)
	{
		try
		{
			// 複数の索引を順番に検索する
			
			double dsquare = Os::Limits<double>::getMax();
			const Entry* pEntry = 0;
			// 近いものを取って置く配列
			ModVector<ModPair<double, const Entry*> > nearList;

			if (m_pMain)
			{
				double d;
				Node::Status s(*m_pMainStatus);
				const Entry* tmp = m_pMain->nnsearch(pCondition, d, s);
				nearList = s.m_vecEntry;
				if (d < dsquare)
				{
					pEntry = tmp;
					dsquare = d;
				}
			}

			if (m_pSmall1)
			{
				double d;
				Node::Status s(*m_pSmall1Status);
				s.m_vecEntry = nearList;
				const Entry* tmp = m_pSmall1->nnsearch(pCondition, d, s);
				nearList = s.m_vecEntry;
				if (d < dsquare)
				{
					pEntry = tmp;
					dsquare = d;
				}
			}

			if (m_pSmall2)
			{
				double d;
				Node::Status s(*m_pSmall2Status);
				s.m_vecEntry = nearList;
				const Entry* tmp = m_pSmall2->nnsearch(pCondition, d, s);
				nearList = s.m_vecEntry;
				if (d < dsquare)
				{
					pEntry = tmp;
					dsquare = d;
				}
			}

			if (pEntry)
			{
				if (nearList.getSize() == 0)
				{
					(*result).pushBack(
						ModPair<ModUInt32, double>(pEntry->getID(), dsquare));
				}
				else
				{
					(*result).reserve(nearList.getSize());
					ModVector<ModPair<double, const Entry*> >::Iterator
						i = nearList.begin();
					for (; i != nearList.end(); ++i)
					{
						(*result).pushBack(
							ModPair<ModUInt32, double>((*i).second->getID(),
													   (*i).first));
					}
				}
			}
			else
			{
				// ヒットしなかった -> 索引が空？
				
				(*result).pushBack(
					ModPair<ModUInt32, double>(Os::Limits<ModUInt32>::getMax(),
											   0.0));
			}
		}
		catch (...)
		{
			void* p = pCondition;
			Os::Memory::free(p);
			_SYDNEY_RETHROW;
		}
		
		void* p = pCondition;
		Os::Memory::free(p);
	}
}

//
//	FUNCTION private
//	KdTree::DoSearch::getNext
//		-- 次に処理すべき検索条件と格納すべき結果を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<ModVector<ModPair<ModUInt32, double> > >::Iterator& i_
//		検索結果を格納する場所
//
//	RETURN
//	KdTree::Entry*
//		検索条件
//
//	EXCEPTIONS
//
Entry*
DoSearch::
getNext(ModVector<ModVector<ModPair<ModUInt32, double> > >::Iterator& i_)
{
	const ModVector<float>* pCond = 0;
	
	{
		Os::AutoCriticalSection cAuto(m_cLatch);

		if (m_iElement >= static_cast<int>(m_vecCondition.getSize()))
			return 0;

		pCond = &(m_vecCondition[m_iElement]);
		i_ = (m_vecResult.begin() + m_iElement);

		++m_iElement;
	}

	return makeEntry(*pCond);
}

//
//	FUNCTION private
//	KdTree::DoSearch::makeEntry -- エントリを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVecto<float>& vecValue_
//		検索条件
//
//	RETURN
//	KdTree::Entry*
//		エントリ。呼び出し側で解放する必要あり
//
//	EXCEPTIONS
//
Entry*
DoSearch::makeEntry(const ModVector<float>& vecValue_)
{
	Entry* pEntry = syd_reinterpret_cast<Entry*>(
		Os::Memory::allocate(Entry::calcSize(vecValue_.getSize())));

	pEntry->m_uiID = 0;
	pEntry->m_iDimensionSize = static_cast<int>(vecValue_.getSize());

	Os::Memory::copy(pEntry->m_pValue, &(*vecValue_.begin()),
					 sizeof(float) * pEntry->m_iDimensionSize);

	return pEntry;
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
