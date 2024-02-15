// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// GetDocumentFrequencyForEstimate.cpp
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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
#include "FullText2/GetDocumentFrequencyForEstimate.h"
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
//	FullText2::GetDocumentFrequencyForEstimate::GetDocumentFrequencyForEstimate
//		-- コンストラクタ
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
GetDocumentFrequencyForEstimate::
GetDocumentFrequencyForEstimate(SearchInformation& cSearchInfo_)
	: m_cSearchInfo(cSearchInfo_)
{
}

//
//	FUNCTION public
//	FullText2::GetDocumentFrequencyForEstimate::~GetDocumentFrequencyForEstimate
//		-- デストラクタ
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
GetDocumentFrequencyForEstimate::~GetDocumentFrequencyForEstimate()
{
	ModVector<SearchInformation*>::Iterator i = m_vecpSearchInfo.begin();
	for (++i; i < m_vecpSearchInfo.end(); ++i)
	{
		delete *i;
	}
}

//
//	FUNCTION public
//	FullText2::GetDocumentFrequencyForEstimate::prepare
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
GetDocumentFrequencyForEstimate::prepare()
{
	// スレッド数を得る
	ModSize size = static_cast<ModSize>(getThreadSize());

	// 検索情報クラスをスレッド数分コピーする
	m_vecpSearchInfo.pushBack(&m_cSearchInfo);
	for (ModSize n = 1; n < size; ++n)
		m_vecpSearchInfo.pushBack(m_cSearchInfo.copy());

	m_ite = m_cSearchInfo.getTermMap().begin();
	m_end = m_cSearchInfo.getTermMap().end();
}

//
//	FUNCTION public
//	FullText2::GetDocumentFrequencyForEstimate::parallel
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
GetDocumentFrequencyForEstimate::parallel()
{
	// 検索情報クラス
	SearchInformation* pSearchInfo = m_vecpSearchInfo[getThreadNum()];
	
	for (;;)
	{
		SearchInformation::MapValue* pMapValue = getNextMapValue();
		if (pMapValue == 0)
			// 終了
			return;

		// 検索結果件数を見積もる
		pMapValue->m_dblDocumentFrequency
			= pMapValue->m_pTermNode->getEstimateCount(*pSearchInfo);

		// 取得済みにする
		pMapValue->m_bDone = true;
	}
}

//
//	FUNCTION public
//	FullText2::GetDocumentFrequencyForEstimate::dispose
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
GetDocumentFrequencyForEstimate::dispose()
{
}

//
//	FUNCTION private
//	FullText2::GetDocumentFrequencyForEstimate::getNextMapValue
//		-- 次に検索結果件数を見積もるノードを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::SearchInformation::MapValue*
//		検索結果件数を見積もるノード
//
//	EXCEPTIONS
//
SearchInformation::MapValue*
GetDocumentFrequencyForEstimate::getNextMapValue()
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	
	SearchInformation::MapValue* ret = 0;
		
	while (m_ite != m_end)
	{
		if ((*m_ite).second.m_bDone)
		{
			++m_ite;
			continue;
		}

		ret = &((*m_ite).second);
		++m_ite;

		break;
	}

	return ret;
}

//
//	Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
