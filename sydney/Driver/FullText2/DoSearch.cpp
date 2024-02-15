// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DoSearch.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
#include "FullText2/DoSearch.h"
#include "FullText2/Parameter.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace {
	// 1スレッドあたりの処理件数
	ParameterInteger _cMinimumNumberForEachThread(
		"FullText2_MinimumNumberForEachThread", 100000);
}

//
//	FUNCTION public
//	FullText2::DoSearch::DoSearch -- コンストラクタ
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
DoSearch::DoSearch()
{
}

//
//	FUNCTION public
//	FullText2::DoSearch::~DoSearch -- デストラクタ
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
DoSearch::~DoSearch()
{
}

//
//	FUNCTION protected
//	FullText2::DoSearch::getExecuteThreadSize -- 実行スレッド数を得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentID uiMaxID_
//		格納されている最大文書ID
//
//	RETURN
//	ModSize
//		実行スレッド数
//
//	EXCEPTIONS
//
ModSize
DoSearch::getExecuteThreadSize(DocumentID uiMaxID_)
{
	// スレッド数を得る
	ModSize size = static_cast<ModSize>(getThreadSize());
	
	// 1スレッドあたり10万件以上になるように調整する
	ModSize min = static_cast<ModSize>(_cMinimumNumberForEachThread.get());
	if (size * min >= uiMaxID_)
	{
		do
		{
			--size;
		}
		while (size * min >= uiMaxID_);
		
		++size;
	}

	return size;
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
