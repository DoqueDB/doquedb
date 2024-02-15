// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// GetCount.h --
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

#ifndef __SYDNEY_FULLTEXT2_GETCOUNT_H
#define __SYDNEY_FULLTEXT2_GETCOUNT_H

#include "FullText2/Module.h"
#include "FullText2/DoSearch.h"

#include "FullText2/Executor.h"
#include "FullText2/SimpleResultSet.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class OperatorNode;
class SearchInformation;

//
//	CLASS
//	FullText2::GetCount -- 
//
//	NOTES
//	OpenMPで並列処理を行い検索を行う
//
class GetCount : public DoSearch
{
public:
	// コンストラクタ
	GetCount(OperatorNode* pNode_,
			 SearchInformation* pSearchInfo_);
	// デストラクタ
	virtual ~GetCount();

	// 実行前の準備
	void prepare();
	// マルチスレッドで実行するメソッド
	void parallel();
	// 終了処理
	void dispose();

	// 検索結果件数を得る
	ModSize getCount() { return m_uiCount; }

private:
	// Executorの配列
	ModVector<Executor> m_vecExecutor;
	// 検索結果件数
	ModSize m_uiCount;

	// 検索ノード
	OperatorNode* m_pNode;
	// 検索情報クラス
	SearchInformation* m_pSearchInfo;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_GETCOUNT_H

//
//  Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
