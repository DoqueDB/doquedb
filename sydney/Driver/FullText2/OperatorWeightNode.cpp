// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorWeightNode.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/OperatorWeightNode.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::OperatorWeightNode::OperatorWeightNode -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentScore dblScale_
//		スケール
//	FullText2::OperatorNode* pNode_
//		検索ノード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OperatorWeightNode::OperatorWeightNode(DocumentScore dblScale_,
									   OperatorNode* pNode_)
	: OperatorNode(), m_dblScale(dblScale_), m_pNode(pNode_)
{
}

//
//	FUNCTION public
//	FullText2::OperatorWeighNode::~OperatorWeightNode -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OperatorWeightNode::~OperatorWeightNode()
{
	delete m_pNode;
}

//
//	FUNCTION public
//	FullText2::OperatorWeightNode::OperatorWeightNode -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::OperatorWeightNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OperatorWeightNode::OperatorWeightNode(const OperatorWeightNode& src_)
	: OperatorNode(src_)
{
	m_dblScale = src_.m_dblScale;
	m_pNode = src_.m_pNode->copy();
}

//
//	FUNCTION public
//	FullText2::OperatorWeightNode::copy -- コピーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::OperatorNode*
//		自身のコピー
//
//	EXCEPTIONS
//
OperatorNode*
OperatorWeightNode::copy() const
{
	return new OperatorWeightNode(*this);
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

