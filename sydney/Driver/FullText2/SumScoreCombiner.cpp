// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SumScoreCombiner.cpp --
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
#include "FullText2/SumScoreCombiner.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::SumScoreCombiner::SumScoreCombiner -- コンストラクタ
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
SumScoreCombiner::SumScoreCombiner()
{
}

//
//	FUNCTION public
//	FullText2::SumScoreCombiner::~SumScoreCombiner -- デストラクタ
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
SumScoreCombiner::~SumScoreCombiner()
{
}

//
//	FUNCTION public
//	FullText2::SumScoreCombiner::combine -- スコアを合成する
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
SumScoreCombiner::combine(DocumentScore x, DocumentScore y)
{
	return (x + y);
}

//
//	FUNCTION public
//	FullText2::SumScoreCombiner::apply -- スコアを合成する
//
//	NOTES
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
SumScoreCombiner::apply(const ModVector<DocumentScore>& scores)
{
	ModVector<DocumentScore>::ConstIterator i = scores.begin();
	ModVector<DocumentScore>::ConstIterator e = scores.end();
	
	DocumentScore s = (*i);
	for (++i; i < e; ++i)
		s += (*i);

	return s;
}

//
//	FUNCTION public
//	FullText2::SumScoreCombiner::copy -- コピーを取得する
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
SumScoreCombiner::copy()
{
	return new SumScoreCombiner(*this);
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
