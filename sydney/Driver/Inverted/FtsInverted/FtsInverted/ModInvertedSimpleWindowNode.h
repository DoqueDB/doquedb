// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedSimpleWindowNode.h -- 近傍演算(先頭文字同士の位置合せ)ノードインタフェイスファイル
// 
// Copyright (c) 1998, 1999, 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedSimpleWindowNode_H__
#define __ModInvertedSimpleWindowNode_H__

#include "ModInvertedWindowNodeTemplate.h"
#include "ModInvertedOrderedSimpleWindowLocationListIterator.h"
#include "ModInvertedUnorderedSimpleWindowLocationListIterator.h"

class
ModInvertedSimpleWindowNode
	: public ModInvertedWindowNodeTemplate<11, 12, 's', ModInvertedOrderedSimpleWindowLocationListIterator, ModInvertedUnorderedSimpleWindowLocationListIterator>
{
public:
	ModInvertedSimpleWindowNode(ModSize, ModSize, ModSize,const ModUInt32 );

	// 自分のコピーを作成する
	QueryNode* duplicate(const ModInvertedQuery& query);
};

inline
ModInvertedSimpleWindowNode::ModInvertedSimpleWindowNode(
	ModSize minimalDistance_,
	ModSize maximalDistance_,
	ModSize freq_,
	const ModUInt32 resultType_
	)
	: ModInvertedWindowNodeTemplate<11, 12, 's', ModInvertedOrderedSimpleWindowLocationListIterator, ModInvertedUnorderedSimpleWindowLocationListIterator>(AtomicNode::orderedSimpleWindowNode,
				resultType_)
{
	minimalDistance = minimalDistance_;
	maximalDistance = maximalDistance_;
	// minimalDistanceがmaximalDistanceより多きいか?
	if (maximalDistance < minimalDistance){
		ModErrorMessage << "min is greater than max. min = "
						<< minimalDistance << ", max = "
						<<  maximalDistance << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
	}
	setTotalDocumentFrequency(freq_);
}

//
//
// FUNCTION public
// ModInvertedrSimpleWindowNode::duplicate -- 自分のコピーを作成する
//
// NOTES
//
// ARGUMENTS
// const ModInvertedQuery&
//      クエリ。
//
// RETURN
// 生成したコピーのノードのポインタ
//
// EXCEPTIONS
// ModInvertedErrorQueryValidateFail
//
inline ModInvertedQueryNode*
ModInvertedSimpleWindowNode::duplicate(const ModInvertedQuery& rQuery)
{
	ModInvertedSimpleWindowNode* node
		= new ModInvertedSimpleWindowNode(minimalDistance,
										  maximalDistance,
										  getTotalDocumentFrequency(),
											firstStepResult->getType());

	if (this->scoreCalculator != 0) {
		node->scoreCalculator = this->scoreCalculator->duplicate();
	}

	if (isOrderedType() == ModTrue) {
		node->setOrderedType();
	} else {
		node->setUnorderedType();
	}

	ModInvertedQueryInternalNode* node2
		= static_cast<ModInvertedQueryInternalNode*>(node);
	setChildForWindowNode(node2, rQuery);

	return node2;
}

#endif //__ModInvertedSimpleWindowNode_H__

//
// Copyright (c) 1998, 1999, 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
