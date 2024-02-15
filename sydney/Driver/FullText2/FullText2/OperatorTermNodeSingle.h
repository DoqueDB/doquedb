// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorTermNodeSingle.h --
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

#ifndef __SYDNEY_FULLTEXT2_OPERATORTERMNODESINGLE_H
#define __SYDNEY_FULLTEXT2_OPERATORTERMNODESINGLE_H

#include "FullText2/Module.h"
#include "FullText2/OperatorTermNodeScore.h"
#include "FullText2/LeafNode.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::OperatorTermNodeSingle -- TF値を合成する末端ノード
//
//	NOTES
//
class OperatorTermNodeSingle : public OperatorTermNodeScore
{
public:
	// コンストラクタ
	OperatorTermNodeSingle(const ModUnicodeString& cString_,
						   LeafNode* node_);
	// デストラクタ
	virtual ~OperatorTermNodeSingle();
	// コピーコンストラクタ
	OperatorTermNodeSingle(const OperatorTermNodeSingle& src_);

	// 見積もり検索件数を得る
	ModSize getEstimateCountLevel1(SearchInformation& cSearchInfo_)
		{ return getEstimateCountLevel1Impl(cSearchInfo_); }
	ModSize getEstimateCountLevel1Impl(SearchInformation& cSearchInfo_);

	// リセットする
	void reset() { resetImpl(); }
	void resetImpl();
	// 文書IDを検索する
	DocumentID lowerBound(SearchInformation& cSearchInfo_,
						  DocumentID id_, bool isRough_)
		{ return lowerBoundImpl(cSearchInfo_, id_, isRough_); }
	DocumentID lowerBoundImpl(SearchInformation& cSearchInfo_,
							  DocumentID id_, bool isRough_);
	// スコアを得る
	DocumentScore getScore(SearchInformation& cSearchInfo_)
		{ return getScoreImpl(cSearchInfo_); }
	DocumentScore getScoreImpl(SearchInformation& cSearchInfo_);
	// コピーする
	OperatorNode* copy() const;

	// 文書内頻度を得る
	ModSize getTermFrequency()
		{ return getTermFrequencyImpl(); }
	ModSize getTermFrequencyImpl();

	// 文書頻度を得る
	void getDocumentFrequency(SearchInformation& cSearchInfo_,
							  DocumentID bid_,		// 開始文書ID
							  DocumentID eid_,		// 終端文書ID
							  Frequency& cValue_,
							  bool bGetTotalTermFrequency_);

private:
	// リーフノード
	LeafNode* m_pLeafNode;

	// 現在の文書ID
	DocumentID m_uiCurrentID;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_OPERATORTERMNODESINGLE_H

//
//	Copyright (c) 2010, 2011, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
