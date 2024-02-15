// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// GetResult.cpp --
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
#include "FullText2/GetResult.h"

#include "FullText2/OperatorNode.h"
#include "FullText2/SearchInformation.h"

#include "Os/AutoCriticalSection.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace {
}

//
//	FUNCTION public
//	FullText2::GetResult::GetResult -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::OperatorNode* pNode_
//		検索ノード
//	FullText2::SearchInformation* pSearchInfo_
//		検索情報ファイル
//	bool bScore_
//		スコアを取得するかどうか
//	FullText2::SortKey::Value eKey_
//		ソートキー
//	FullText2::Order::Value eOrder_
//		昇順 or 降順
//	Common::LargeVector<FullText2::DocumentID>& vecNarrowing_
//		絞り込みのための文書IDの配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
GetResult::GetResult(OperatorNode* pNode_,
					 SearchInformation* pSearchInfo_,
					 bool bScore_,
					 SortKey::Value eKey_,
					 Order::Value eOrder_,
					 const Common::LargeVector<DocumentID>* pNarrowing_)
	: DoSearch(),
	  m_pNode(pNode_), m_pSearchInfo(pSearchInfo_),
	  m_bScore(bScore_), m_eKey(eKey_), m_eOrder(eOrder_),
	  m_pNarrowing(pNarrowing_)
{
}

//
//	FUNCTION public
//	FullText2::GetResult::~GetResult -- デストラクタ
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
GetResult::~GetResult()
{
}

//
//	FUNCTION
//	FullText2::GetResult::prepare -- 準備する
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
GetResult::prepare()
{
	if (m_pNarrowing)
	{
		//
		//	絞り込み
		//

		// 件数を得る
		ModSize count = m_pNarrowing->getSize();
		if (count == 0)
		{
			// 絞り込む前にすでに 0 件なので、検索しない
			return;
		}
		
		// 実行スレッド数を得る
		ModSize size = getExecuteThreadSize(count);
	
		// スレッド数と同じ数だけの Executor を用意する
		m_vecExecutor.assign(size);

		// 各エグゼキュータにパラメータを設定する
		ModVector<Executor>::Iterator i = m_vecExecutor.begin();
		Common::LargeVector<DocumentID>::ConstIterator b
			= m_pNarrowing->begin();
		Common::LargeVector<DocumentID>::ConstIterator e = b;
		e += (count / size);
		int n = 1;
	
		// 先頭は特別
		(*i).setSearchInfo(m_pSearchInfo, false);
		(*i).setOperatorNode(m_pNode, false);
		(*i).setRange(b, e);
		(*i).setScore(m_bScore);
		(*i).setSortParameter(m_eKey, m_eOrder);

		++i;
		++n;
		for (; i != m_vecExecutor.end(); ++i, ++n)
		{
			b = e;
			if (n == size)
			{
				e = m_pNarrowing->end();
			}
			else
			{
				e += (count / size);
			}
		
			// 二番目以降
			(*i).setSearchInfo(m_pSearchInfo->copy(), true);
			(*i).setOperatorNode(m_pNode->copy(), true);
			(*i).setRange(b, e);
			(*i).setScore(m_bScore);
			(*i).setSortParameter(m_eKey, m_eOrder);
		}
	}
	else
	{
		//
		//	全体
		//

		//【注意】
		//	マルチスレッドで実行するには、文書IDで範囲を区切る必要があるが、
		//	文書が削除されたりすると、その範囲を最大文書IDの数から
		//	等間隔に区切るのは、非効率な場合がある
		//	よって、何らかの統計情報が必要になると思われる

		// 最大文書IDを得る
		DocumentID maxID = m_pSearchInfo->getMaxDocumentID();
		// 実行スレッド数を得る
		ModSize size = getExecuteThreadSize(maxID);
	
		// 最大文書IDのデータは格納されているので、終端は1を足す必要がある
		maxID += 1;
		// スレッド数と同じ数だけの Executor を用意する
		m_vecExecutor.assign(size);

		// 各エグゼキュータにパラメータを設定する
		ModVector<Executor>::Iterator i = m_vecExecutor.begin();
		DocumentID b = 1;
		DocumentID e = maxID / size;
		int n = 1;
	
		// 先頭は特別
		(*i).setSearchInfo(m_pSearchInfo, false);
		(*i).setOperatorNode(m_pNode, false);
		(*i).setRange(b, e);
		(*i).setScore(m_bScore);
		(*i).setSortParameter(m_eKey, m_eOrder);

		++i;
		++n;
		for (; i != m_vecExecutor.end(); ++i, ++n)
		{
			b = e;
			e = (n == size) ? maxID : (maxID / size * n);
		
			// 二番目以降
			(*i).setSearchInfo(m_pSearchInfo->copy(), true);
			(*i).setOperatorNode(m_pNode->copy(), true);
			(*i).setRange(b, e);
			(*i).setScore(m_bScore);
			(*i).setSortParameter(m_eKey, m_eOrder);
		}
	}
}

//
//	FUNCTION public
//	FullText2::GetResult::parallel -- マルチスレッドで実行する
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
GetResult::parallel()
{
	// スレッド番号を得る
	int n = getThreadNum();

	if (static_cast<ModSize>(n) >= m_vecExecutor.getSize())
		// このスレッドは実行する必要なし
		return;

	// 検索実行クラスを得る
	Executor& e = m_vecExecutor[n];

	// 検索を実行する
	SimpleResultSet& r = e.getResultSet();

	{
		// 検索結果をメンバー変数にマージする
		//
		// dispose() でやるとすべてのスレッドが終了するのを待ってしまうので、
		// 検索が終わったスレッドから順次マージした方が効率的

		Os::AutoCriticalSection cAuto(m_cLatch);
		m_cResultSet.merge(r, m_eKey, m_eOrder);
	}
}

//
//	FUNCTION public
//	FullText2::GetResult::dispose -- 終了処理を行う
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
GetResult::dispose()
{
	// prepare() で確保した作業領域を解放する
	m_vecExecutor.clear();
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
