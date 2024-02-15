// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ArrayLeafNode.h --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_ARRAYLEAFNODE_H
#define __SYDNEY_FULLTEXT2_ARRAYLEAFNODE_H

#include "FullText2/Module.h"
#include "FullText2/LeafNode.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::ArrayLeafNode -- LeafNodeの配列ノードの基底クラス
//
//	NOTES
//
class ArrayLeafNode : public LeafNode
{
public:
	// コンストラクタ
	ArrayLeafNode();
	// デストラクタ
	virtual ~ArrayLeafNode();
	// コピーコンストラクタ
	ArrayLeafNode(const ArrayLeafNode& src_);

	// 領域をリザーブする
	virtual void reserve(ModSize size_) = 0;
	// ノードを追加する
	virtual void pushBack(LeafNode* pNode_) = 0;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_ARRAYLEAFNODE_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
