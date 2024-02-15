// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedFileCapsule.cpp -- 転置ファイルカプセルの実装
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2005, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "ModInvertedFileCapsule.h"
#include "ModInvertedQuery.h"

//
// CONST
// ModInvertedFileCapsuleBooleanSearch::defaultValidateMode
//		-- デフォルト有効化モード
//
// NOTES
// ブーリアン検索のためのデフォルト有効化モード。
//
/*static*/
const ModInvertedQueryValidateMode 
ModInvertedFileCapsuleBooleanSearch::defaultValidateMode(
	// ModInvertedQuery::eraseTermLeafNode |
	ModInvertedQuery::flattenChildren |
	ModInvertedQuery::sortChildren |
	ModInvertedQuery::orStanderdStyle |
	ModInvertedQuery::makeRough |
	// ModInvertedQuery::makeRoughOfSimpleNode |
	ModInvertedQuery::sharedNode |
	// ModInvertedQuery::sharedRoughNode |
	ModInvertedQuery::toSimpleWindow |
	ModInvertedQuery::tokenMapToRough |
	ModInvertedQuery::bestPathToRough |
	ModInvertedQuery::tokenizeQuery
	);

//
// CONST
// ModInvertedFileCapsuleBooleanSearch::defaultEvaluateMode
//		-- デフォルト評価モード
//
// NOTES
// ブーリアン検索のためのデフォルト評価モード。
//
/*static*/
const ModInvertedQueryEvaluateMode 
ModInvertedFileCapsuleBooleanSearch::defaultEvaluateMode(0);

//
// CONST
// ModInvertedFileCapsuleRankingSearch::defaultValidateMode
//		-- デフォルト有効化モード
//
// NOTES
// ランキング検索のためのデフォルト有効化モード。
//
/*static*/
const ModInvertedQueryValidateMode 
ModInvertedFileCapsuleRankingSearch::defaultValidateMode(
//	ModInvertedQuery::eraseTermLeafNode |
	ModInvertedQuery::flattenChildren |
	ModInvertedQuery::sortChildren |
	ModInvertedQuery::orStanderdStyle |
	ModInvertedQuery::makeRough |
	// ModInvertedQuery::makeRoughOfSimpleNode |
	ModInvertedQuery::sharedNode |
	// ModInvertedQuery::sharedRoughNode |
	ModInvertedQuery::toSimpleWindow |
	ModInvertedQuery::tokenMapToRough |
	ModInvertedQuery::bestPathToRough |
	ModInvertedQuery::tokenizeQuery |
	ModInvertedQuery::rankingMode
	);

//
// CONST
// ModInvertedFileCapsuleRankingSearch::defaultEvaluateMode
//		-- デフォルト評価モード
//
// NOTES
// ランキング検索のためのデフォルト評価モード。
//
/*static*/
const ModInvertedQueryEvaluateMode 
ModInvertedFileCapsuleRankingSearch::defaultEvaluateMode(
	ModInvertedQuery::retrieveTF
	);

//
// CONST
// ModInvertedFileCapsuleGetMatchLocations::defaultValidateMode
//		-- デフォルト有効化モード
//
// NOTES
// 照合位置取得のためのデフォルト有効化モード。
//
/*static*/
const ModInvertedQueryValidateMode 
ModInvertedFileCapsuleGetMatchLocations::defaultValidateMode(
//	ModInvertedQuery::eraseTermLeafNode |
	ModInvertedQuery::flattenChildren |
	ModInvertedQuery::sortChildren |
	ModInvertedQuery::orStanderdStyle |
	ModInvertedQuery::makeRough |
	// ModInvertedQuery::makeRoughOfSimpleNode |
	ModInvertedQuery::sharedNode |
	// ModInvertedQuery::sharedRoughNode |
	ModInvertedQuery::toSimpleWindow |
	ModInvertedQuery::tokenMapToRough |
	ModInvertedQuery::bestPathToRough |
	ModInvertedQuery::tokenizeQuery
	);

//
// CONST
// ModInvertedFileCapsuleGetMatchLocations::defaultEvaluateMode
//		-- デフォルト評価モード
//
// NOTES
// 照合位置取得のためのデフォルト評価モード。
//
/*static*/
const ModInvertedQueryEvaluateMode 
ModInvertedFileCapsuleGetMatchLocations::defaultEvaluateMode(0);

//
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved
//
