// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorOrNode.h --
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

#ifndef __SYDNEY_FULLTEXT2_OPERATORORNODE_H
#define __SYDNEY_FULLTEXT2_OPERATORORNODE_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"
#include "FullText2/LogicalOperatorNode.h"

#include "ModPair.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::OperatorOrNode -- 論理和を処理する多項演算子
//
//	NOTES
//
class OperatorOrNode : public LogicalOperatorNode
{
public:
	// コンストラクタ
	OperatorOrNode();
	// デストラクタ
	virtual ~OperatorOrNode();
	// コピーコンストラクタ
	OperatorOrNode(const OperatorOrNode& src_);

	// 領域を確保する
	void reserve(ModSize size_) { m_vecChildren.reserve(size_); }
	// 子ノードを追加する
	void pushBack(OperatorNode* child_);

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
	ModSize getEstimateCountLevel1(SearchInformation& cSearchInfo_);

private:
	typedef ModPair<DocumentID, OperatorNode*>	Child;
	typedef ModVector<Child>					ChildVector;

	// 子ノード
	ChildVector m_vecChildren;
	// 現在の文書ID
	DocumentID m_uiCurrentID;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_OPERATORORNODE_H

//
//	Copyright (c) 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
