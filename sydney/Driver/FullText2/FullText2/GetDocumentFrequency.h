// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// GetDocumentFrequency.h --
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

#ifndef __SYDNEY_FULLTEXT2_GETDOCUMENTFREQUENCY_H
#define __SYDNEY_FULLTEXT2_GETDOCUMENTFREQUENCY_H

#include "FullText2/Module.h"
#include "FullText2/DoSearch.h"
#include "FullText2/OperatorTermNode.h"

#include "Os/CriticalSection.h"

#include "ModPair.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class SearchInformation;

//
//	CLASS
//	FullText2::GetDocumentFrequency -- 
//
//	NOTES
//	OpenMPで並列処理を行いDF値等を取得する
//
class GetDocumentFrequency : public DoSearch
{
public:
	//
	//	CLASS
	//	FullText2::GetDocumentFrequency::Target
	//
	class Target
	{
	public:
		// コンストラクタ(1)
		Target()
			: m_pTermNode(0) {}
		// コンストラクタ(2)
		Target(OperatorTermNode* pTermNode_)
			: m_pTermNode(pTermNode_) {}

		OperatorTermNode::Frequency	m_cFrequency;	// 頻度情報
		OperatorTermNode*			m_pTermNode;	// 検索ノード
	};
	
	// コンストラクタ
	GetDocumentFrequency(SearchInformation& cSearchInfo_,
						 bool bGetTotalTermFrequency_);
	// デストラクタ
	virtual ~GetDocumentFrequency();

	// 前処理
	void prepare();
	// マルチスレッドで実行するメソッド
	void parallel();
	// 後処理
	void dispose();

private:
	// メモリの解放
	void clear();
	
	// 検索情報クラス
	SearchInformation& m_cSearchInfo;
	// スレッド数分のコピー
	ModVector<SearchInformation*> m_vecpSearchInfo;

	// 範囲
	ModVector<ModPair<DocumentID, DocumentID> > m_vecRange;
	// スレッドごと格納領域
	ModVector<ModVector<Target> > m_vecTargetVector;

	// 取得する値
	bool m_bGetTotalTermFrequency;		// 総文書内頻度
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_GETDOCUMENTFREQUENCY_H

//
//  Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
