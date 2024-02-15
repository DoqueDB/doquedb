// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedWordOrderedDistanceNode.cpp -- wordHead,wordTail用のOrderedDistanceNodeの実装
// 
// Copyright (c) 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifdef SYD_INVERTED // SYDNEY 対応
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#endif

#include "ModAssert.h"
#include "ModOstrStream.h"
#include "ModInvertedException.h"

#include "ModInvertedWordOrderedDistanceNode.h"
#include "ModInvertedOrderedDistanceLocationListIterator.h"

//
//
// FUNCTION protected
// ModInvertedWordOrderedDistanceNode::reevaluate -- 正確な再 evaluate
//
// NOTES
// 粗い evaluate 後の、正確な再 evaluate。
// 出現位置の検査のみを行なう。
// wordHead,wordTailの場合は2番目の子ノードがemptyNodeなので、
// それに対してはevaluate()を呼ぶ。
//
// ARGUMENTS
// ModInvertedDocumentID documentID
//		文書ID
//
// RETURN
// 出現位置検査を満足すれば ModTrue、しなければ ModFalse
//
// EXCEPTIONS
// ModInvertedErrorRetrieveFail
//
ModBoolean
ModInvertedWordOrderedDistanceNode::reevaluate(ModInvertedDocumentID documentID)
{
	ModInvertedOrderedDistanceLocationListIterator* iterator
		= static_cast<ModInvertedOrderedDistanceLocationListIterator*>(getFreeList());
	if (iterator == 0)
	{
		iterator = new ModInvertedOrderedDistanceLocationListIterator(this);
		iterator->reserve(children.getSize());
	}
	LocationIterator::AutoPointer p = iterator;

#ifdef DEBUG
	++countLocCheck;
#endif

	// 常に document-at-a-time
	// ２つのノードそれぞれについて正確に再 evaluate すると共に、
	// 両者の出現位置も得る。

	// 少なくとも一方の子ノードで再 evaluate が ModFalse なら ModFalse
	ModInvertedLocationListIterator* i = 0;
	if (children[0]->reevaluate(documentID, i)
		== ModFalse) {
		return ModFalse;
	}
	iterator->pushIterator(pathPosition[0], i);
	// 2番目のノードはemptyNodeなのでevaluateを呼ぶ
	if (children[1]->evaluate(documentID, i,
							  ModInvertedQueryInternalNode::defaultEMode, 0)
		== ModFalse) {
		return ModFalse;
	}
	iterator->pushIterator(pathPosition[1], i);

	// 位置を比較
	iterator->initialize();
	ModBoolean result = (iterator->isEnd() == ModFalse) ? ModTrue : ModFalse;

	return result;
}

//
// FUNCTION protected
// ModInvertedWordOrderedDistanceNode::reevaluate -- 正確な再 evaluate と位置情報の獲得
//
// NOTES
// 粗い evaluate を前提として、正確な再 evaluate を行ない、満足の場合、
// 与えられた出現位置情報オブジェクトに出現位置情報を格納する。
// wordHead,wordTailの場合は2番目の子ノードがemptyNodeなので、
// それに対してはevaluate()を呼ぶ。
//
// ARGUMENTS
// ModInvertedLocationListIterator*& locations
//		出現位置反復子へのポインタ (結果格納用)
// ModInvertedQueryNode* givenEndNode
//		OrderedDistanceが作成される前のTermLeafNodeの最終端simpleTokenLeafNode
//		へのポインタ。自分の子ノードがこのgivenEndNodeと一致した場合は、子ノード
//		から作成したlocationListIteratorを自分のlocation->endにセットする。
//
// RETURN
// 出現位置検査を満足すれば ModTrue、しなければ ModFalse
//
// EXCEPTIONS
// ModInvertedErrorRetrieveFail
//
ModBoolean
ModInvertedWordOrderedDistanceNode::reevaluate(DocumentID documentID,
											   LocationIterator*& locations,
											   ModInvertedQueryNode* givenEndNode)
{
	ModInvertedOrderedDistanceLocationListIterator* iterator
		= static_cast<ModInvertedOrderedDistanceLocationListIterator*>(getFreeList());
	if (iterator == 0)
	{
		iterator = new ModInvertedOrderedDistanceLocationListIterator(this);
		iterator->reserve(children.getSize());
	}
	LocationIterator::AutoPointer p = iterator;

#ifdef DEBUG
	++countLocCheck;
#endif

	ModInvertedLocationListIterator* max = 0;
	// 常に document-at-a-time
	// ２つのノードそれぞれについて正確に再 evaluate すると共に、
	// 両者の出現位置も得る。

	// 少なくとも一方の子ノードで再 evaluate が ModFalse なら ModFalse
	ModInvertedLocationListIterator* i = 0;
	if (children[0]->reevaluate(documentID, i,
								givenEndNode) == ModFalse) {
		return ModFalse;
	}
	iterator->pushIterator(pathPosition[0], i);
	if (maxElement == 0) max = i;
	// 2番目のノードはemptyNodeなのでevaluateを呼ぶ
	if (children[1]->evaluate(documentID, i,
							  ModInvertedQueryInternalNode::defaultEMode,
							  givenEndNode) == ModFalse) {
		return ModFalse;
	}
	iterator->pushIterator(pathPosition[1], i);
	if (maxElement == 1) max = i;

	// 位置を比較
	iterator->initialize();

	if (iterator->isEnd() == ModFalse) {
		// OrderedDistanceLocationListIteratorのendのセット
		iterator->setEnd(max);
		locations = p.release();
		return ModTrue;
	}

	return ModFalse;
}

//
// Copyright (c) 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
