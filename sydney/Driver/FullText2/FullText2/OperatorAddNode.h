// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorAddNode.h --
// 
// Copyright (c) 2011, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_OPERATORADDNODE_H
#define __SYDNEY_FULLTEXT2_OPERATORADDNODE_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"
#include "FullText2/OperatorNode.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class ScoreCombiner;

//
//	CLASS
//	FullText2::OperatorAddNode -- スコア値を加える二項演算子
//
//	NOTES
//
class OperatorAddNode : public OperatorNode
{
public:
	// コンストラクタ
	OperatorAddNode();
	// デストラクタ
	virtual ~OperatorAddNode();
	// コピーコンストラクタ
	OperatorAddNode(const OperatorAddNode& src_);

	// スコア合成器を設定する
	void setScoreCombiner(ScoreCombiner* pCombiner_);

	// 子ノードを追加する
	void setNode(OperatorNode* first_, OperatorNode* second_)
		{ m_pFirst = first_; m_pSecond = second_; }

	// リセットする
	void reset();
	// 文書IDを検索する
	DocumentID lowerBound(SearchInformation& cSearchInfo_,
						  DocumentID id_, bool isRough_);

	// スコアを得る
	DocumentScore getScore(SearchInformation& cSearchInfo_);

	// コピーを得る
	OperatorNode* copy() const;

	// 見積もり検索件数を得る
	ModSize getEstimateCountLevel1(SearchInformation& cSearchInfo_)
		{ return m_pFirst->getEstimateCountLevel1(cSearchInfo_); }

private:
	// スコア合成器
	ScoreCombiner* m_pCombiner;
	
	// 子ノード
	OperatorNode* m_pFirst;
	OperatorNode* m_pSecond;

	// 現在の文書ID
	DocumentID m_uiCurrentID;
	DocumentID m_uiSecondCurrentID;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_OPERATORADDNODE_H

//
//	Copyright (c) 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
