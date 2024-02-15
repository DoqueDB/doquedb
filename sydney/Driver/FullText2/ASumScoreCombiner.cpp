// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ASumScoreCombiner.cpp --
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
#include "FullText2/ASumScoreCombiner.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::ASumScoreCombiner::ASumScoreCombiner -- コンストラクタ
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
ASumScoreCombiner::ASumScoreCombiner()
{
}

//
//	FUNCTION public
//	FullText2::ASumScoreCombiner::~ASumScoreCombiner -- デストラクタ
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
ASumScoreCombiner::~ASumScoreCombiner()
{
}

//
//	FUNCTION public
//	FullText2::ASumScoreCombiner::combine -- スコアを合成する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentScore x
//		スコア
//	FullText2::DocumentScore y
//		スコア
//
//	RETURN
//	FullText2::DocumentScore
//		合成後のスコア
//
//	EXCEPTIONS
//
DocumentScore
ASumScoreCombiner::combine(DocumentScore x, DocumentScore y)
{
	return (x + y) - (x * y);
}

//
//	FUNCTION public
//	FullText2::ASumScoreCombiner::apply -- スコアを合成する
//
//	NOTES
//	スコアのVectorを受け取り、そのVectorに含まれている全てのスコアを代
//	数積で合成して返す。
//
//	1 - (1 - X1)(1 - X2)...(1 - Xn)
//
//	ARGUMENTS
//	const ModVector<FullText2::DocumentScore>& scores
//		スコア
//
//	RETURN
//	FullText2::DocumentScore
//		合成後のスコア
//
//	EXCEPTIONS
//
DocumentScore
ASumScoreCombiner::apply(const ModVector<DocumentScore>& scores)
{
	ModVector<DocumentScore>::ConstIterator p = scores.begin();
	ModVector<DocumentScore>::ConstIterator end = scores.end();

	DocumentScore score;

	// 最初の要素の計算
	if (*p > 1) {		// *p > 1 の場合は *p=1 として計算
		return 1.0;
	} else { 
		score = 1 - *p;			
	}

	++p;

	while (p != end) {
		if (*p > 1) {
			return 1.0;
		} else {
			score *= (1 - *p);
			++p;
		}
	}
	return (1 - score);
}

//
//	FUNCTION public
//	FullText2::ASumScoreCombiner::copy -- コピーを取得する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::ScoreCombiner*
//		コピー
//
//	EXCEPTIONS
//
ScoreCombiner*
ASumScoreCombiner::copy()
{
	return new ASumScoreCombiner(*this);
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
