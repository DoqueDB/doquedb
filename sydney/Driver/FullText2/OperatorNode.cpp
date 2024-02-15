// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorNode.cpp --
// 
// Copyright (c) 2010, 2014, 2023 Ricoh Company, Ltd.
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
#include "FullText2/OperatorNode.h"

#include "FullText2/SearchInformation.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::OperatorNode::OperatorNode -- コンストラクタ
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
OperatorNode::OperatorNode()
{
}

//
//	FUNCTION public
//	FullText2::OperatorNode::~OperatorNode -- デストラクタ
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
OperatorNode::~OperatorNode()
{
}

//
//	FUNCTION public
//	FullText2::OperatorNode::getEstimateCount
//		-- 検索結果件数を見積もる
//
//	NOTES
//
//	ARGUNETS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//
//	RETURN
//	ModSize
//		検索結果件数
//
//	EXCPETIONS
//
ModSize
OperatorNode::getEstimateCount(SearchInformation& cSearchInfo_)
{
	ModSize count = 0;
	
	SearchInformation::EstimateLevel::Value level
		= cSearchInfo_.getEstimateLevel();
	
	if (level == SearchInformation::EstimateLevel::Level2)
	{
		// 文書IDの突合せは行う

		count = getCountForEstimate(cSearchInfo_, true);
	}
	else if (level == SearchInformation::EstimateLevel::Level3)
	{
		// 位置情報の突合せも行う

		count = getCountForEstimate(cSearchInfo_, false);
	}
	else
	{
		// 索引単位に格納されている件数のみを利用した見積り

		count = getEstimateCountLevel1(cSearchInfo_);
	}

	return count;
}

//
//	FUNCTION private
//	FullText2::OperatorNode::getCountForEstimate
//		-- 検索結果件数見積りのための検索を行う
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	bool isRough_
//		位置の突合せを行う場合はfalse、行わない場合はtrue
//
//	RETURN
//	ModSize
//		検索結果件数
//
//	EXCEPTIONS
//
ModSize
OperatorNode::getCountForEstimate(SearchInformation& cSearchInfo_,
								  bool isRough_)
{
	ModSize count = 0;

	// リセットする
	reset();
	
	DocumentID id = 1;	// 文書IDは1から
	while ((id = lowerBound(cSearchInfo_, id, isRough_))
		   != UndefinedDocumentID)
	{
		// 検索結果が得られたので、件数を増やす
		++count;

		// 次へ
		++id;
	}

	return count;
}

//
//	Copyright (c) 2010, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
