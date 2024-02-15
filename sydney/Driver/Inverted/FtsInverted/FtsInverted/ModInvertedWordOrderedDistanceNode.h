// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedWordOrderedDistanceNode.h -- wordHead,wordTail用のOrderedDistanceNodインタフェイスファイル
// 
// Copyright (c) 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedWordOrderedDistanceNode_H__
#define __ModInvertedWordOrderedDistanceNode_H__

#include "ModInvertedOrderedDistanceNode.h"

class ModInvertedOrderedDistanceLocationListIterator;

//
// CLASS
// ModInvertedWordOrderedDistanceNode -- 間隔演算(正確な距離を指定した距離演算)を表すノード
//
// NOTES
// wordHead,wordTail用に作成されるOrderedDistanceNodeは、
// 2番目の子ノードが常にempryNodeになる。その場合、
// reevaluate処理のときにemptyNodeに対してはevaluateを呼ぶ必要がある。
// そのために、OrderedDistanceNodeからrevaluate()だけをオーバーライドした。
//
class
ModInvertedWordOrderedDistanceNode 
	: public ModInvertedOrderedDistanceNode
{
public:
	// コンストラクタ
	ModInvertedWordOrderedDistanceNode(ModSize freq,
			const ModUInt32 resultType_);

protected:
	// 粗い evaluate 満足を前提とした、正確な再評価
	// 二番目の形式は、位置情報が必要な場合に用いられる。
	virtual ModBoolean reevaluate(DocumentID documentID);
	virtual ModBoolean reevaluate(DocumentID documentID,
								  LocationIterator*& locations,
								  ModInvertedQueryNode* givenEndNode = 0);
};

//
// FUNCTION public
// ModInvertedOrderedWordDistanceNode::ModInvertedWordOrderedDistanceNode -- 間隔演算ノードの生成
//
// NOTES
// 間隔演算(正確な距離を指定した距離演算)ノードを生成する。
//
// ARGUMENTS
// ModSize distance_
//	間隔
//
inline
ModInvertedWordOrderedDistanceNode::ModInvertedWordOrderedDistanceNode(
	ModSize freq,const ModUInt32 resultType_)
	: ModInvertedOrderedDistanceNode(freq,resultType_)
{
}

#endif //__ModInvertedWordOrderedDistanceNode_H__

//
// Copyright (c) 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
