// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UnaryLeafNode.cpp --
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
#include "FullText2/UnaryLeafNode.h"

#include "Exception/BadArgument.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::UnaryLeafNode::UnaryLeafNode -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::LeafNode* pNode_
//		リーフノード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
UnaryLeafNode::UnaryLeafNode(LeafNode* pLeafNode_)
	: LeafNode(), m_pLeafNode(pLeafNode_),
	  m_uiCurrentID(0)
{
}

//
//	FUNCTION public
//	FullText2::UnaryLeafNode::~UnaryLeafNode -- デストラクタ
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
UnaryLeafNode::~UnaryLeafNode()
{
	// フリーリストを解放する
	clearFree();
	
	delete m_pLeafNode;
}

//
//	FUNCTION public
//	FullText2::UnaryLeafNode::UnaryLeafNode -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::UnaryLeafNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
UnaryLeafNode::UnaryLeafNode(const UnaryLeafNode& src_)
	: LeafNode(src_), m_pLeafNode(0), m_uiCurrentID(0)
{
	m_pLeafNode = src_.m_pLeafNode->copy();
}

//
//	FUNCTION public
//	FullText2::UnaryLeafNode::reset -- リセットする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTINONS
//
void
UnaryLeafNode::reset()
{
	m_uiCurrentID = 0;
	m_pLeafNode->reset();
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
