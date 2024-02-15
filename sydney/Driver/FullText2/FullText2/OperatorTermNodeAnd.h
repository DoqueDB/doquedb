// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorTermNodeAnd.h --
// 
// Copyright (c) 2010, 2011, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_OPERATORTERMNODEAND_H
#define __SYDNEY_FULLTEXT2_OPERATORTERMNODEAND_H

#include "FullText2/Module.h"
#include "FullText2/OperatorTermNode.h"
#include "FullText2/OperatorTermNodeArray.h"

#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class ScoreCombiner;

//
//	CLASS
//	FullText2::OperatorTermNodeAnd -- スコア値を合成する末端ノード(論理積用)
//
//	NOTES
//
class OperatorTermNodeAnd : public OperatorTermNode,
							public OperatorTermNodeArray
{
public:
	// コンストラクタ
	OperatorTermNodeAnd(const ModUnicodeString& cString_,
						ScoreCombiner* pCombiner_);
	// デストラクタ
	virtual ~OperatorTermNodeAnd();
	// コピーコンストラクタ
	OperatorTermNodeAnd(const OperatorTermNodeAnd& src_);

	// TermNodeを追加する
	void pushBack(int field_,
				  OperatorTermNodeSingle* node_,
				  double scoreScale_,
				  double scoreGeta_);

	// 見積もり検索件数を得る
	ModSize getEstimateCountLevel1(SearchInformation& cSearchInfo_);

	// リセットする
	void reset();
	// 文書IDを検索する
	DocumentID lowerBound(SearchInformation& cSearchInfo_,
						  DocumentID id_, bool isRough_);
	// スコアを得る
	DocumentScore getScore(SearchInformation& cSearchInfo_);
	// コピーする
	OperatorNode* copy() const;

	// 文書内頻度を得る
	ModSize getTermFrequency();
	// スコア計算器を設定する
	void setScoreCalculator(ScoreCalculator* pCalculator_);

	// 文書頻度を得る
	void getDocumentFrequency(SearchInformation& cSearchInfo_,
							  DocumentID bid_,		// 開始文書ID
							  DocumentID eid_,		// 終端文書ID
							  Frequency& cValue_,
							  bool bGetTotalTermFrequency_);

private:
	// TermNode用の構造体
	struct NodeData
	{
		NodeData()
			: m_iField(-1), m_pTermNode(0), m_dblScale(0), m_dblGeta(0) {}
		NodeData(int iField_,
				 OperatorTermNodeSingle* pTermNode_,
				 double scoreScale_, double scoreGeta_)
			: m_iField(iField_),
			  m_pTermNode(pTermNode_),
			  m_dblScale(scoreScale_),
			  m_dblGeta(scoreGeta_) {}

		int							m_iField;
		OperatorTermNodeSingle* 	m_pTermNode;
		double						m_dblScale;
		double						m_dblGeta;
	};
	
	// TermNode
	ModVector<NodeData> m_vecpTermNode;

	// 現在の文書ID
	DocumentID m_uiCurrentID;

	// スコア合成器
	ScoreCombiner* m_pCombiner;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_OPERATORTERMNODEAND_H

//
//	Copyright (c) 2010, 2011, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
