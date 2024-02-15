// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorWeightNode.h --
// 
// Copyright (c) 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_OPERATORWEIGHTNODE_H
#define __SYDNEY_FULLTEXT2_OPERATORWEIGHTNODE_H

#include "FullText2/Module.h"
#include "FullText2/OperatorNode.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::OperatorWeightNode -- 重みを処理する検索ノード
//
//	NOTES
//
class OperatorWeightNode : public OperatorNode
{
public:
	// コンストラクタ
	OperatorWeightNode(DocumentScore dblScale_,
					   OperatorNode* pNode_);
	// デストラクタ
	virtual ~OperatorWeightNode();
	// コピーコンストラクタ
	OperatorWeightNode(const OperatorWeightNode& src_);

	// リセットする
	void reset() { m_pNode->reset(); }
	// 文書IDを検索する
	DocumentID lowerBound(SearchInformation& cSearchInfo_,
						  DocumentID id_, bool isRough_)
		{
			return m_pNode->lowerBound(cSearchInfo_, id_, isRough_);
		}

	// スコアを得る
	DocumentScore getScore(SearchInformation& cSearchInfo_)
		{
			return m_pNode->getScore(cSearchInfo_) * m_dblScale;
		}

	// コピーを得る
	OperatorNode* copy() const;

	// 見積もり検索件数を得る
	ModSize getEstimateCountLevel1(SearchInformation& cSearchInfo_)
		{ return m_pNode->getEstimateCountLevel1(cSearchInfo_); }
	
private:
	// スケール
	DocumentScore m_dblScale;
	// 検索ノード
	OperatorNode* m_pNode;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_OPERATORWEIGHTNODE_H

//
//	Copyright (c) 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
