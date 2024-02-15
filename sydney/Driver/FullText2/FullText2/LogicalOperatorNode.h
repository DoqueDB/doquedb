// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalOperatorNode.h --
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

#ifndef __SYDNEY_FULLTEXT2_LOGICALOPERATORNODE_H
#define __SYDNEY_FULLTEXT2_LOGICALOPERATORNODE_H

#include "FullText2/OperatorNode.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class ScoreCombiner;

//
//	CLASS
//	FullText2::LogicalOperatorNode -- 論理演算を実行するクラスの基底クラス
//
//	NOTES
//
class LogicalOperatorNode : public OperatorNode
{
public:
	// コンストラクタ
	LogicalOperatorNode();
	// デストラクタ
	virtual ~LogicalOperatorNode();
	// コピーコンストラクタ
	LogicalOperatorNode(const LogicalOperatorNode& src_);
	
	// スコア合成器を設定する
	void setScoreCombiner(ScoreCombiner* pCombiner_);

	// 領域を確保する
	virtual void reserve(ModSize size_) = 0;
	// 子ノードを追加する
	virtual void pushBack(OperatorNode* child_) = 0;

protected:
	// スコア合成器
	ScoreCombiner* m_pCombiner;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_LOGICALOPERATORNODE_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
