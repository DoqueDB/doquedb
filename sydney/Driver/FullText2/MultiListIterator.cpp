// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiListIterator.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
#include "FullText2/MultiListIterator.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::MultiListIterator::MultiListIterator -- コンストラクタ
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
//	なし
//
MultiListIterator::MultiListIterator()
	: ListIterator(),
	  m_uiCurrentID(0), m_uiOtherMinimumID(0),
	  m_iCurrentElement(-1), m_bFind(false)
{
}

//
//	FUNCTION public
//	FullText2::MultiListIterator::~MultiListIterator -- デストラクタ
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
//	なし
//
MultiListIterator::~MultiListIterator()
{
	// フリーリストを解放する
	clearFree();
	
	ModVector<ModPair<DocumentID, ListIterator*> >::Iterator i
		= m_vecpIterator.begin();
	for (; i != m_vecpIterator.end(); ++i)
		if ((*i).second) delete (*i).second;
	m_vecpIterator.clear();
}

//
//	FUNCTION public
//	FullText2::MultiListIterator::MultiListIterator -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::MultiListIterator& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
MultiListIterator::MultiListIterator(const MultiListIterator& src_)
	: ListIterator(src_),
	  m_uiCurrentID(0), m_uiOtherMinimumID(0),
	  m_iCurrentElement(-1), m_bFind(false)
{
	m_vecpIterator.reserve(src_.m_vecpIterator.getSize());
	ModVector<ModPair<DocumentID, ListIterator*> >::ConstIterator i
		= src_.m_vecpIterator.begin();
	for (; i != src_.m_vecpIterator.end(); ++i)
	{
		// コピーを追加する
		ListIterator * p = 0;
		if ((*i).second)
			p = (*i).second->copy();
		pushBack(p);
	}
}

//
//	FUNCTION public
//	FullText2::MultiListIterator::pushBack -- 要素を追加する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::ListIterator* iterator
//		追加する要素
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiListIterator::pushBack(ListIterator* iterator)
{
	DocumentID id = ((iterator == 0) ? UndefinedDocumentID : 0);
	m_vecpIterator.pushBack(ModPair<DocumentID, ListIterator*>(id, iterator));
	m_b = m_vecpIterator.begin();
	m_e = m_vecpIterator.end();
}

//
//	FUNCTION public
//	FullText2::MultiListIterator::getEstimateCount -- おおよその文書数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
// 	RETURN
//	ModSize
//		おおよその文書数
//
//	EXCEPTIONS
//
ModSize
MultiListIterator::getEstimateCount()
{
	ModSize count = 0;

	// 同じ文書が格納されていることはないので、足し合わせればいい
	
	ModVector<ModPair<DocumentID, ListIterator*> >::Iterator i = m_b;
	for (; i != m_e; ++i)
		if ((*i).second) count += (*i).second->getEstimateCount();

	return count;
}

//
//	FUNCTION public
//	FullText2::MultiListIterator::getLength -- 索引単位の長さを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
// 	RETURN
//	int
//		索引単位の長さ
//
//	EXCEPTIONS
//
int
MultiListIterator::getLength()
{
	// 長さはどれも同じなので、先頭のイテレータで長さを求める
	
	ModVector<ModPair<DocumentID, ListIterator*> >::Iterator i = m_b;
	while ((*i).second == 0) ++i;
	return (*i).second->getLength();
}

//
//	FUNCTION public
//	FullText2::MultiListIterator::find -- 文書IDを検索する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	FullText2::DocumentID uiDocumentID_
//		検索する文書ID
//
//	RETURN
//	bool
//		検索にヒットした場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
MultiListIterator::find(SearchInformation& cSearchInfo_,
						DocumentID uiDocumentID_)
{
	if (m_iCurrentElement == -1)
	{
		m_iCurrentElement = getElement(cSearchInfo_, uiDocumentID_);
		m_i = m_b;
		m_i += m_iCurrentElement;
	}
	
	while (m_iCurrentElement != -1)
	{
		// とりあえず、現在の要素で検索する

		if ((*m_i).second->find(cSearchInfo_, uiDocumentID_))
		{
			(*m_i).first = uiDocumentID_;
			m_uiCurrentID = uiDocumentID_;
			m_bFind = true;
			return true;
		}

		// この要素にはなかったので、格納要素番号を得る
		int e = getElement(cSearchInfo_, uiDocumentID_);
		if (e != m_iCurrentElement)
		{
			m_iCurrentElement = e;
			m_i = m_b;
			m_i += e;
		}
		else
		{
			m_iCurrentElement = -1;
		}
	}

	m_uiCurrentID = UndefinedDocumentID;
	return false;
}

//
//	FUNCTION public
//	FullText2::MultiListIterator::next -- 次へ進める
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
DocumentID
MultiListIterator::next(SearchInformation& cSearchInfo_)
{
	if (m_uiCurrentID == UndefinedDocumentID)
		return m_uiCurrentID;

	if (m_bFind == true)
	{
		// 前回findを実行して、そのままnextが呼ばれている
		// -> 次がわからないので、現在値でlowerBoundする
		resetImpl();
		lowerBoundImpl(cSearchInfo_, m_uiCurrentID);
	}

	if (m_iCurrentElement == -1)
	{
		ModVector<ModPair<DocumentID, ListIterator*> >::Iterator i = m_b;
		for (; i != m_e; ++i)
			// 初めての next なので、すべてのイテレータで next を実行する
			if ((*i).second) (*i).first = (*i).second->next(cSearchInfo_);
		
		// 最小値を設定する
		set();

		return m_uiCurrentID;
	}

	// 現時点の最小値を保持するイテレータを次に進める
	(*m_i).first = (*m_i).second->next(cSearchInfo_);

	if ((*m_i).first < m_uiOtherMinimumID)
	{
		// この要素のまま
		m_uiCurrentID = (*m_i).first;
	}
	else
	{
		// 他のイテレータも参照する必要あり
		set();
	}

	return m_uiCurrentID;
}

//
//	FUNCTION public
//	FullText2::MultiListIterator::getTermFrequency
//		-- 位置情報内の頻度情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		位置情報内の頻度情報
//
//	EXCEPTIONS
//
ModSize
MultiListIterator::getTermFrequency()
{
	return (*m_i).second->getTermFrequency();
}

//
//	FUNCTION public
//	FullText2::MultiListIterator::getLocationListIterator
//		-- 位置情報へのイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	InvertedLocationListIterator::AutoPointer
//		位置情報イテレータへのポインタ。
//		このメモリーは呼び出し側で解放する必要がある
//
//	EXCEPTIONS
//
LocationListIterator::AutoPointer
MultiListIterator::getLocationListIterator()
{
	return (*m_i).second->getLocationListIterator();
}

//
//	FUNCTION private
//	FullText2::MultiListIterator::set -- 最小の文書IDを設定する
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
MultiListIterator::set()
{
	m_bFind = false;
	m_uiCurrentID = UndefinedDocumentID;
	m_uiOtherMinimumID = UndefinedDocumentID;
	m_iCurrentElement = -1;
	int element = 0;
	ModVector<ModPair<DocumentID, ListIterator*> >::Iterator i = m_b;
	for (; i != m_e; ++i, ++element)
	{
		if ((*i).first != UndefinedDocumentID)
		{
			if (m_uiCurrentID > (*i).first)
			{
				m_uiOtherMinimumID = m_uiCurrentID;	// その他の最小値
				m_uiCurrentID = (*i).first;
				m_iCurrentElement = element;
			}
			else if (m_uiOtherMinimumID > (*i).first)
			{
				m_uiOtherMinimumID = (*i).first;
			}
		}
	}
	if (m_iCurrentElement != -1)
	{
		m_i = m_b;
		m_i += m_iCurrentElement;
	}
}

//
//	FUNCTION private
//	FullText2::MultiListIterator::resetImpl -- 先頭へ戻る
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
MultiListIterator::resetImpl()
{
	ModVector<ModPair<DocumentID, ListIterator*> >::Iterator i = m_b;
	for (; i != m_e; ++i)
	{
		if ((*i).second)
		{
			(*i).first = 0;
			(*i).second->reset();
		}
	}
	
	m_uiCurrentID = 0;
	m_uiOtherMinimumID = 0;
	m_iCurrentElement = -1;
	m_bFind = false;
}

//
//	FUNCTION private
//	FullText2::MultiListIterator::lowerBoundImpl -- 文書IDをlower_bound検索する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	FullText2::DocumentID uiDocumentID_
//		検索する文書ID
//
//	RETURN
//	DocumentID
//
//	EXCEPTIONS
//
DocumentID
MultiListIterator::lowerBoundImpl(SearchInformation& cSearchInfo_,
								  DocumentID uiDocumentID_)
{
	if (m_uiCurrentID > uiDocumentID_)
	{
		// 現在値より小さいので、リセット
		resetImpl();
	}
	
	if (m_uiCurrentID < uiDocumentID_ &&
		m_uiOtherMinimumID > uiDocumentID_)
	{
		// この要素に存在する
		(*m_i).first = (*m_i).second->lowerBound(cSearchInfo_, uiDocumentID_);
		
		if ((*m_i).first < m_uiOtherMinimumID)
		{
			// 他の要素の最小値を超えなければOK
			m_uiCurrentID = (*m_i).first;
			return m_uiCurrentID;
		}
	}

	// 全要素を lowerBound する
	int n = 0;
	ModVector<ModPair<DocumentID, ListIterator*> >::Iterator i = m_b;
	for (; i != m_e; ++i)
	{
		if ((*i).first != UndefinedDocumentID)
			// 終端に達した要素はもうlowerBoundの対象外
			// 引数のuiDocumentID_は単調増加する
			
			(*i).first = (*i).second->lowerBound(cSearchInfo_, uiDocumentID_);
	}
	set();
	return m_uiCurrentID;
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
