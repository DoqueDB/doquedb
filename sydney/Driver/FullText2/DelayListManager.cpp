// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DelayListManager.cpp --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "FullText2/DelayListManager.h"
#include "FullText2/DelayListIterator.h"
#include "FullText2/ListIteratorWithMax.h"
#include "FullText2/ListIteratorWithExpungeList.h"
#include "FullText2/InvertedSection.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//  FUNCTION public
//  FullText2::DelayListManager::DelayListManager -- コンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::FullTextFile& cFile_
//		全文索引ファイル
//  FullText2::InvertedSection& cSection_
//		転置ファイルセクション
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
DelayListManager::DelayListManager(FullTextFile& cFile_,
								   InvertedSection& cSection_)
	: MultiListManager(cFile_, cSection_)
{
}

//
//  FUNCTION public
//  FullText2::DelayListManager::~DelayListManager -- デストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
DelayListManager::~DelayListManager()
{
}

//
//  FUNCTION public
//  FullText2::DelayListManager::DelayListManager -- コピーコンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  const FullText2::DelayListManager& cSrc_
//		コピー元
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
DelayListManager::DelayListManager(const DelayListManager& cSrc_)
	: MultiListManager(cSrc_), m_vecMaxDocumentID(cSrc_.m_vecMaxDocumentID)
{
}

//
//	FUNCTION public
//	FullText2::DelayListManager::copy -- コピーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::ListManager*
//		コピー
//
//	EXCEPTIONS
//
ListManager*
DelayListManager::copy() const
{
	return new DelayListManager(*this);
}

//
//	FUNCTION public
//	FullText2::DelayListManager::reserve -- 要素を格納する領域を確保する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize size_
//		要素数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DelayListManager::reserve(ModSize size_)
{
	m_vecMaxDocumentID.reserve(size_);
	MultiListManager::reserve(size_);
}

//
//	FUNCTION public
//	FullText2::DelayListManager::pushBack -- 要素を追加する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::ListManager* pListManager_
//		追加する要素
//	FullText2::DocumentID uiMaxDocumentID_
//		最大文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DelayListManager::pushBack(ListManager* pListManager_,
						   DocumentID uiMaxDocumentID_)
{
	m_vecMaxDocumentID.pushBack(uiMaxDocumentID_);
	MultiListManager::pushBack(pListManager_);
}

//
//  FUNCTION public
//  FullText2::DelayListManager::getIterator -- 転置リストイテレータを得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ListIterator*
//	  転置リストイテレータ
//
//  EXCEPTIONS
//
ListIterator*
DelayListManager::getIterator()
{
	DelayListIterator* iterator = new DelayListIterator();
	iterator->reserve(m_vecpListManager.getSize());
	ModVector<ModPair<ListManager*, bool> >::ConstIterator
		i = m_vecpListManager.begin();
	ModVector<DocumentID>::ConstIterator j = m_vecMaxDocumentID.begin();
	for (; i != m_vecpListManager.end(); ++i, ++j)
	{
		// ヒットしていないところは 0 を pushBack する
		
		ListIterator* ite = 0;
		if ((*i).second == true && (*i).first->getKey() == m_cstrKey)
		{
			ite = new ListIteratorWithMax((*i).first->getIterator(), *j);
		}
		iterator->pushBack(ite, *j);
	}

	return new ListIteratorWithExpungeList(iterator,
										   m_cSection.getExpungeList());
}

//
//  Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
