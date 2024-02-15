// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenOption.h --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_OPENOPTION_H
#define __SYDNEY_KDTREE_OPENOPTION_H

#include "KdTree/Module.h"
#include "KdTree/FileID.h"

#include "LogicalFile/OpenOption.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "Common/IntegerArrayData.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

//
//	CLASS
//	KdTree::OpenOption -- 転置ファイルドライバーのOpenOption
//
//	NOTES
//
class OpenOption
{
public:
	struct KeyID
	{
		enum Value
		{
			// 検索文(String)
			Condition = LogicalFile::OpenOption::DriverNumber::KdTree,
			ConditionCount,
			TraceType,
			SelectLimit,
			MaxCalculateCount
		};
	};

	struct Projection
	{
		enum Value
		{
			ROWID,				// ROWID
			NeighborID,			// 検索条件ID
			NeighborDistance	// 距離
		};
	};

	// コンストラクタ
	OpenOption(LogicalFile::OpenOption& cLogicalOpenOption_);
	// デストラクタ
	virtual ~OpenOption();

	// 検索条件を設定する
	bool getSearchParameter(const FileID& cFileID_,
							const LogicalFile::TreeNodeInterface* pCondition_);
	// プロジェクションを設定する
	bool getProjectionParameter(const FileID& cFileID_,
								const LogicalFile::TreeNodeInterface* pNode_);
	// ソートを設定する
	bool getSortParameter(const FileID& cFileID_,
						  const LogicalFile::TreeNodeInterface* pNode_);
	// 取得数と取得位置を設定する
	bool getLimitParameter(const FileID& cFileID_,
						   const Common::IntegerArrayData& cSpec_);


	// 検索条件を得る
	bool getCondition(ModVector<ModVector<float> >& vecCondition_) const;
	// 探索タイプを得る
	Node::TraceType::Value getTraceType() const;
	// 計算回数上限を得る
	int getMaxCalculateCount() const;
	// 結果取得件数を得る
	ModSize getSelectLimit() const;
	// プロジェクションを得る
	ModVector<Projection::Value> getProjection() const;
	
private:
	// 1つの検索条件をパースする
	void parseOneCondition(
		const FileID& cFileID_,		
		const LogicalFile::TreeNodeInterface* pValue_,
		int iElement_);
	// 検索条件をパースする
	void parseCondition(
		const FileID& cFileID_,
		const LogicalFile::TreeNodeInterface* pValue_);
	
	// LogicalFile::OpenOption
	LogicalFile::OpenOption& m_cOpenOption;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_OPENOPTION_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
