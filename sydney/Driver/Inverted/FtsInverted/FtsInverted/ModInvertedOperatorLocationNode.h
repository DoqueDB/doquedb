// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOperatorLocationNode.h -- Loation ノード
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

#ifndef __ModInvertedOperatorLocationNode_H__
#define __ModInvertedOperatorLocationNode_H__

#include "ModInvertedLocationNodeTemplate.h"
#include "ModInvertedLocationNodeLocationListIterator.h"

//
// CLASS ModInvertedOperatorLocationNode -- Locationノード
//
// NOTES
// 検索式内部表現中間ノードクラスの派生クラスで、Locationノードを表す。
// locationノードはこの_locationで指定された位置に検索語が存在する場合ヒット
// となる。
//
class ModInvertedOperatorLocationNode :
	public ModInvertedLocationNodeTemplate<ModInvertedLocationNodeLocationListIterator, ModInvertedAtomicNode::operatorLocationNode>
{
public:
	// コンストラクタ
	ModInvertedOperatorLocationNode(const ModSize,const  ModUInt32 resultType_);
};

//
// FUNCTION public
// ModInvertedOperatorLocationNode::ModInvertedOperatorLocationNode -- コンストラクタ
//
// NOTES
// コンストラクタ
//
// ARGUMENTS
// const ModSize location_
//		出現位置
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
// 
inline
ModInvertedOperatorLocationNode::ModInvertedOperatorLocationNode(
	const ModSize location_,const  ModUInt32 resultType_) 
	: ModInvertedLocationNodeTemplate<ModInvertedLocationNodeLocationListIterator, AtomicNode::operatorLocationNode>
		(location_, AtomicNode::operatorLocationNode,resultType_)
{
}

#endif //__ModInvertedOperatorLocationNode_H__

//
// Copyright (c) 1999, 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
