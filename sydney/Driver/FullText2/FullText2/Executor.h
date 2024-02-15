// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Executor.h --
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

#ifndef __SYDNEY_FULLTEXT2_EXECUTOR_H
#define __SYDNEY_FULLTEXT2_EXECUTOR_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"
#include "FullText2/SimpleResultSet.h"

#include "Common/LargeVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class OperatorNode;
class SearchInformation;

//
//	CLASS
//	FullText2::Executor
//		-- 検索を実行するクラス
//		   スレッドごとに１つのインスタンス
//
//	NOTES
//
class Executor
{
public:
	// コンストラクタ
	Executor();
	// デストラクタ
	virtual ~Executor();

	// 検索結果集合を得る
	SimpleResultSet& getResultSet();
	// 検索結果件数を得る
	ModSize getCount();

	// 検索情報クラスを設定する
	void setSearchInfo(SearchInformation* pSearchInfo_, bool bOwner_)
		{ m_pSearchInfo = pSearchInfo_; m_bSearchInfoOwner = bOwner_; }
	// 検索ノードを設定する
	void setOperatorNode(OperatorNode* pNode_, bool bOwner_)
		{ m_pNode = pNode_; m_bNodeOwner = bOwner_; }
	// 検索範囲を設定する
	void setRange(DocumentID uiLower_, DocumentID uiUpper_)
		{ m_uiLower = uiLower_; m_uiUpper = uiUpper_; m_bNarrowing = false; }
	void setRange(Common::LargeVector<DocumentID>::ConstIterator b_,
				  Common::LargeVector<DocumentID>::ConstIterator e_)
		{
			m_ite = b_;
			m_endIte = e_;
			--e_;
			m_uiUpper = *e_;
			++m_uiUpper;
			m_bNarrowing = true;
		}
	// スコアを取得するかどうかを設定する
	void setScore(bool bScore_)
		{ m_bScore = bScore_; }
	// ソートパラメータを設定する
	void setSortParameter(SortKey::Value eKey_,
						  Order::Value eOrder_)
		{ m_eKey = eKey_; m_eOrder = eOrder_; }

private:
	// 検索を実行する
	ModSize search(bool bOnlyCount_);

	// 先頭の文書IDを得る
	DocumentID beginID();
	// 次の文書IDを得る
	DocumentID nextID(DocumentID currentID_);
	// 条件を満たしているか
	bool checkID(DocumentID currentID_);
	
	// 検索結果集合
	SimpleResultSet m_cResultSet;
	
	// 検索情報クラス
	SearchInformation* m_pSearchInfo;
	bool m_bSearchInfoOwner;
	// 検索ノード
	OperatorNode* m_pNode;
	bool m_bNodeOwner;

	// スコアが必要かどうか
	bool m_bScore;
	// 検索を実行したかどうか
	bool m_bSearched;

	// 検索用のパラメータ m_uiLower 以上かつ m_uiUpper 未満の文書を検索する
	DocumentID	m_uiLower;		// 下限の文書ID
	DocumentID	m_uiUpper;		// 上限の文書ID

	// 絞り込み検索かどうか
	bool m_bNarrowing;
	// 絞り込み検索のデータ
	Common::LargeVector<DocumentID>::ConstIterator m_ite;
	Common::LargeVector<DocumentID>::ConstIterator m_endIte;
	// 現在値
	DocumentID m_uiCurrentID;

	// ソート条件
	SortKey::Value m_eKey;
	Order::Value m_eOrder;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_EXECUTOR_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
