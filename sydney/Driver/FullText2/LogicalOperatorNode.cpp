// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalOperatorNode.cpp --
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
#include "FullText2/LogicalOperatorNode.h"
#include "FullText2/ScoreCombiner.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::LogicalOperatorNode::LogicalOperatorNode -- コンストラクタ
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
LogicalOperatorNode::LogicalOperatorNode()
	: OperatorNode(), m_pCombiner(0)
{
}

//
//	FUNCTION public
//	FullText2::LogicalOperatorNode::~LogicalOperatorNode -- デストラクタ
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
LogicalOperatorNode::~LogicalOperatorNode()
{
	delete m_pCombiner;
}

//
//	FUNCTION public
//	FullText2::LogicalOperatorNode::LogicalOperatorNode
//		-- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::LogicalOperatorNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
LogicalOperatorNode::LogicalOperatorNode(const LogicalOperatorNode& src_)
	: OperatorNode(src_), m_pCombiner(0)
{
	// スコア合成器をコピーする
	if (src_.m_pCombiner)
		m_pCombiner = src_.m_pCombiner->copy();
}

//
//	FUNCTION public
//	FullText2::LogicalOperatorNode::setScoreCombiner
//		-- スコア合成器を設定する
//
//	NOTES
//	与えられたスコア合成器のインスタンスはデストラクト時に解放される
//
//	ARGUMENTS
//	FullText2::ScoreCombiner*
//		スコア合成器
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LogicalOperatorNode::setScoreCombiner(ScoreCombiner* pCombiner_)
{
	if (m_pCombiner) delete m_pCombiner;
	m_pCombiner = pCombiner_;
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

