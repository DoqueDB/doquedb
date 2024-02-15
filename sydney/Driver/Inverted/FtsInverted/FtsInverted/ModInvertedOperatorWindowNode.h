// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOperatorWindowNode.h -- 近傍演算(末尾と先頭の位置合せ)ノードインタフェイスファイル
// 
// Copyright (c) 1998, 1999, 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedOperatorWindowNode_H__
#define __ModInvertedOperatorWindowNode_H__

#include "ModInvertedWindowNodeTemplate.h"
#include "ModInvertedOrderedOperatorWindowLocationListIterator.h"
#include "ModInvertedUnorderedOperatorWindowLocationListIterator.h"

class
ModInvertedOperatorWindowNode
	: public ModInvertedWindowNodeTemplate<12, 14, 'o', ModInvertedOrderedOperatorWindowLocationListIterator, ModInvertedUnorderedOperatorWindowLocationListIterator>
{
public:
	ModInvertedOperatorWindowNode(ModSize, ModSize,const ModUInt32 );

	// 自分のコピーを作成する
	QueryNode* duplicate(const ModInvertedQuery& query);
};

inline
ModInvertedOperatorWindowNode::ModInvertedOperatorWindowNode(
	ModSize minimalDistance_,
	ModSize maximalDistance_,
	const  ModUInt32 resultType_
	)
	: ModInvertedWindowNodeTemplate<12, 14, 'o', ModInvertedOrderedOperatorWindowLocationListIterator, ModInvertedUnorderedOperatorWindowLocationListIterator>(AtomicNode::orderedOperatorWindowNode,
		resultType_
		)
{
	minimalDistance = minimalDistance_;
	maximalDistance = maximalDistance_;
	// minimalDistanceがmaximalDistanceより大きいか?
	if (maximalDistance < minimalDistance){
		ModErrorMessage << "min is greater than max. min = "
						<< minimalDistance << ", max = "
						<<  maximalDistance << ModEndl;
		ModThrowInvertedFileError(ModInvertedErrorQueryValidateFail);
	}
}

//
//
// FUNCTION public
// ModInvertedrOperatorWindowNode::duplicate -- 自分のコピーを作成する
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
ModInvertedOperatorWindowNode::duplicate(const ModInvertedQuery& rQuery)
{
	ModInvertedOperatorWindowNode* node
		= new ModInvertedOperatorWindowNode(minimalDistance,
											maximalDistance,
											firstStepResult->getType());

	if (this->scoreCalculator != 0) {
		node->scoreCalculator = this->scoreCalculator->duplicate();
	}

	if (isOrderedType() == ModTrue) {
		node->setOrderedType();
	} else {
		node->setUnorderedType();
	}

	// totalDocumentFrequencyのコピー
	node->setTotalDocumentFrequency(totalDocumentFrequency);

	ModInvertedQueryInternalNode* node2
		= static_cast<ModInvertedQueryInternalNode*>(node);
	setChildForWindowNode(node2, rQuery);

	return node2;
}

#endif __ModInvertedOperatorWindowNode_H__

//
// Copyright (c) 1998, 1999, 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
