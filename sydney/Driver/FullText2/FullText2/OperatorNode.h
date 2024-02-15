// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorNode.h --
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

#ifndef __SYDNEY_FULLTEXT2_OPERATORNODE_H
#define __SYDNEY_FULLTEXT2_OPERATORNODE_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class SearchInformation;

//
//	CLASS
//	FullText2::OperatorNode -- 検索ノードの基底クラス
//
//	NOTES
//
class OperatorNode
{
public:
	// コンストラクタ
	OperatorNode();
	// デストラクタ
	virtual ~OperatorNode();

	// 検索件数を見積る
	ModSize getEstimateCount(SearchInformation& cSearchInfo_);

	// リセットする
	virtual void reset() = 0;
	
	// 文書IDを検索する
	virtual DocumentID lowerBound(SearchInformation& cSearchInfo_,
								  DocumentID id_, bool isRough_) = 0;

	// スコアを得る
	virtual DocumentScore getScore(SearchInformation& cSearchInfo_) = 0;

	// コピーを得る
	virtual OperatorNode* copy() const = 0;

	// 見積もり検索件数を得る
	virtual ModSize getEstimateCountLevel1(SearchInformation& cSearchInfo_) = 0;

private:
	// 検索結果件数を得る
	ModSize getCountForEstimate(SearchInformation& cSearchInfo_, bool isRough_);
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_OPERATORNODE_H

//
//	Copyright (c) 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
