// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedRankingResultLeafNode.h -- ランキング検索結果ノードインターフェースファイル
// 
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedRankingResultLeafNode_H__
#define __ModInvertedRankingResultLeafNode_H__

#include "ModOstrStream.h"
#include "ModInvertedQueryLeafNode.h"
#include "ModInvertedSearchResult.h"

class ModInvertedBooleanResultLeafNode;

// ModInvertedBooleanResultLeafNode,ModInvertedRankingResultLeafNode
// 従来は、別のクラス扱いになっていたが、よく見ると、継承関係にあることがわかったため、
// そのように改めた。
//

class
ModInvertedRankingResultLeafNode : public ModInvertedBooleanResultLeafNode
{
public:
	 // コンストラクタ
	ModInvertedRankingResultLeafNode(
	const ModInvertedRankingResultLeafNode* originalNode,const  ModUInt32 resultType_);
	ModInvertedRankingResultLeafNode(const ModInvertedSearchResult* result,const  ModUInt32 resultType_);

	virtual void contentString(ModUnicodeString& content) const;

#if 0
	virtual void removeFromFirstStepResult(
		const ModInvertedSearchResult* bresult);
#endif
	virtual void checkQueryNode(
				ModInvertedQuery* query_,
				const ModBoolean setStringInChildren_,
				const ModBoolean needDF_);

	virtual ModInvertedQueryNode*	duplicate(const ModInvertedQuery& rQuery)
	{
		return new ModInvertedRankingResultLeafNode(*this);
	}
	ModInvertedSearchResult* getRankingResult() {
		return searchResult;
	}
};

#endif // __ModInvertedRankingResultLeafNode_H__

//
// Copyright (c) 1997, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
