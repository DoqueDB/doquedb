// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// GetDocumentFrequency.cpp
// 
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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
#include "FullText2/GetDocumentFrequency.h"
#include "FullText2/OperatorTermNode.h"
#include "FullText2/SearchInformation.h"
#include "FullText2/Query.h"

#include "Os/AutoCriticalSection.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::GetDocumentFrequency::GetDocumentFrequency -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	bool bGetTotalTermFrequency_
//		総文書内頻度を得るかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
GetDocumentFrequency::GetDocumentFrequency(SearchInformation& cSearchInfo_,
										   bool bGetTotalTermFrequency_)
	: DoSearch(),
	  m_cSearchInfo(cSearchInfo_),
	  m_bGetTotalTermFrequency(bGetTotalTermFrequency_)
{
}

//
//	FUNCTION public
//	FullText2::GetDocumentFrequency::~GetDocumentFrequency -- デストラクタ
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
GetDocumentFrequency::~GetDocumentFrequency()
{
	clear();
}

//
//	Function public
//	FullText2::GetDocumentFrequency::prepare
//		-- 前処理
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
GetDocumentFrequency::prepare()
{
	// 最大文書IDを得る
	DocumentID maxID = m_cSearchInfo.getMaxDocumentID();
	// スレッド数を得る
	ModSize size = getExecuteThreadSize(maxID);

	// 検索情報クラスをスレッド数分コピーする
	m_vecpSearchInfo.pushBack(&m_cSearchInfo);
	for (ModSize n = 1; n < size; ++n)
		m_vecpSearchInfo.pushBack(m_cSearchInfo.copy());
	
	// 最大文書IDのデータは格納されているので、終端は1を足す必要がある
	maxID += 1;
	
	// レンジを作成
	m_vecRange.reserve(size);
	DocumentID b = 1;
	DocumentID e = maxID / size;
	m_vecRange.pushBack(ModPair<DocumentID, DocumentID>(b, e));
	
	for (ModSize i = 1; i < size; ++i)
	{
		ModSize n = i + 1;
		
		b = e;
		e = (n == size) ? maxID : (maxID / size * n);
		
		m_vecRange.pushBack(ModPair<DocumentID, DocumentID>(b, e));
	}

	// スレッドごとの検索ノードを用意する
	ModVector<OperatorTermNode*> vecNode;
	m_cSearchInfo.getTermNodeForDocumentFrequency(vecNode);
	m_vecTargetVector.assign(size);
	ModVector<ModVector<Target> >::Iterator k = m_vecTargetVector.begin();
	for (ModSize i = 0; i < size; ++i, ++k)
	{
		(*k).reserve(vecNode.getSize());
		
		ModVector<OperatorTermNode*>::Iterator j = vecNode.begin();
		for (; j != vecNode.end(); ++j)
		{
			if (i == 0)
			{
				// 先頭スレッドはコピーしない
				(*k).pushBack(Target(*j));
			}
			else
			{
				// その他はコピー
				OperatorTermNode* node = _SYDNEY_DYNAMIC_CAST(OperatorTermNode*,
															  (*j)->copy());
				(*k).pushBack(Target(node));
			}
		}
	}
}

//
//	FUNCTION public
//	FullText2::GetDocumentFrequency::parallel
//		-- マルチスレッドで実行するメソッド
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
GetDocumentFrequency::parallel()
{
	// スレッド番号を得る
	int n = getThreadNum();

	if (static_cast<ModSize>(n) >= m_vecRange.getSize())
		// このスレッドは実行する必要なし
		return;

	// 探索範囲
	DocumentID s_id = m_vecRange[n].first;
	DocumentID e_id = m_vecRange[n].second;

	// 検索情報クラス
	SearchInformation* pSearchInfo = m_vecpSearchInfo[n];

	// このスレッド用のノードを得る
	ModVector<Target>& vecTarget = m_vecTargetVector[n];

	ModVector<Target>::Iterator i = vecTarget.begin();
	ModVector<Target>::Iterator e = vecTarget.end();

	for (; i != e; ++i)
	{
		// 検索ノード
		OperatorTermNode* pTerm = (*i).m_pTermNode;

		// 文書頻度を求める
		pTerm->getDocumentFrequency(*pSearchInfo,
									s_id, e_id,
									(*i).m_cFrequency,
									m_bGetTotalTermFrequency);
	}
}

//
//	FUNCTION public
//	FullText2::GetDocumentFrequency::dispose
//		-- 後処理を行う
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
GetDocumentFrequency::dispose()
{
	if (isException() == false)
	{
		// 正常に検索できたので、結果をマージする

		// 検索語の数
		ModSize size = m_vecTargetVector[0].getSize();

		ModVector<ModVector<Target> >::Iterator b = m_vecTargetVector.begin();
		ModVector<ModVector<Target> >::Iterator e = m_vecTargetVector.end();

		for (ModSize n = 0; n < size; ++n)
		{
			// 先頭要素の Target にまとめる
			Target* p = 0;

			ModVector<ModVector<Target> >::Iterator i = b;
			for (; i != e; ++i)
			{
				if (i == b)
				{
					// 先頭要素なので、Target を得る
					p = &((*i).operator[](n));
				}
				else
				{
					// 先頭以外なので、マージする
					p->m_cFrequency.merge((*i).operator[](n).m_cFrequency);
				}
			}
		}

		// マージした結果を検索情報クラスに設定する
		ModVector<Target>::Iterator j = (*b).begin();
		for (; j != (*b).end(); ++j)
		{
			const ModUnicodeString& tea = (*j).m_pTermNode->getString();
			m_cSearchInfo.setDocumentFrequency(tea, (*j).m_cFrequency);
		}
	}
	
	clear();
}

//
//	FUNCTION private
//	FullText2::GetDocumentFrequency::clear -- メモリの解放
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
GetDocumentFrequency::clear()
{
	// どちらも先頭要素は削除してはいけない

	if (m_vecpSearchInfo.getSize())
	{
		// 検索情報
		ModVector<SearchInformation*>::Iterator i = m_vecpSearchInfo.begin();
		for (++i; i < m_vecpSearchInfo.end(); ++i)
		{
			delete *i;
		}
		m_vecpSearchInfo.clear();
	}

	if (m_vecTargetVector.getSize())
	{
		// 頻度情報
		ModVector<ModVector<Target> >::Iterator k = m_vecTargetVector.begin();
		for (++k; k < m_vecTargetVector.end(); ++k)
		{
			ModVector<Target>::Iterator j = (*k).begin();
			for (; j != (*k).end(); ++j)
			{
				delete (*j).m_pTermNode;
			}
		}
		m_vecTargetVector.clear();
	}
}

//
//
//	Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
