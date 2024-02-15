// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOperatorEndNode.h -- End ノード
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedOperatorEndNode_H__
#define __ModInvertedOperatorEndNode_H__

#include "ModInvertedLocationNodeTemplate.h"
#include "ModInvertedEndNodeLocationListIterator.h"

//
// CLASS ModInvertedOperatorEndNode -- Endノード
//
// NOTES
// 検索式内部表現中間ノードクラスの派生クラスで、Locationノードを表す。
//
class
ModInvertedOperatorEndNode : 
	public ModInvertedLocationNodeTemplate<ModInvertedEndNodeLocationListIterator, ModInvertedAtomicNode::operatorEndNode>
{
public:
	// コンストラクタ
	ModInvertedOperatorEndNode(const ModSize,const  ModUInt32 resultType_);
};

//
// FUNCTION public
// ModInvertedOperatorEndNode::ModInvertedOperatorEndNode -- コンストラクタ
//
// NOTES
// コンストラクタ
//
// ARGUMENTS
// ModInvertedDocumentLengthFile* documentLengthFile_ 
//		文書長ファイル
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
// 
inline
ModInvertedOperatorEndNode::ModInvertedOperatorEndNode(
	const ModSize distance_,const  ModUInt32 resultType_)
	: ModInvertedLocationNodeTemplate<ModInvertedEndNodeLocationListIterator, AtomicNode::operatorEndNode>
	(distance_, AtomicNode::operatorEndNode,resultType_)
{
}

#endif //__ModInvertedOperatorEndNode_H__

//
// Copyright (c) 1999, 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
