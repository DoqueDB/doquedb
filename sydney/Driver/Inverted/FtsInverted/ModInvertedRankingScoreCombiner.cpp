// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedRankingScoreCombiner.cpp -- ModInvertedRankingScoreCombinerの実装
// 
// Copyright (c) 1998, 1999, 2000, 2002, 2006, 2023 Ricoh Company, Ltd.
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

#ifdef SYD_INVERTED // SYDNEY 対応
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#endif

#include "ModUnicodeString.h"

#include "ModInvertedException.h"
#include "ModInvertedRankingScoreCombiner.h"
#include "ModInvertedProdScoreCombiner.h"
#include "ModInvertedASumScoreCombiner.h"
#include "ModInvertedSumScoreCombiner.h"
#include "ModInvertedMaxScoreCombiner.h"
#include "ModInvertedMinScoreCombiner.h"

//
// CONST
// ModInvertedSumScoreCombiner::combinerName -- 合成器名
//
// NOTES
// スコア合成器の名称
//
/*static*/ const char
ModInvertedSumScoreCombiner::combinerName[] = "Sum";

//
// CONST
// ModInvertedASumScoreCombiner::combinerName -- 合成器名
//
// NOTES
// スコア合成器の名称
//
/*static*/ const char
ModInvertedASumScoreCombiner::combinerName[] = "ASum";

//
// CONST
// ModInvertedProdScoreCombiner::combinerName -- 合成器名
//
// NOTES
// スコア合成器の名称
//
/*static*/ const char
ModInvertedProdScoreCombiner::combinerName[] = "Prod";

//
// CONST
// ModInvertedMaxScoreCombiner::combinerName -- 合成器名
//
// NOTES
// スコア合成器の名称
//
/*static*/ const char
ModInvertedMaxScoreCombiner::combinerName[] = "Max";

//
// CONST
// ModInvertedMinScoreCombiner::combinerName -- 合成器名
//
// NOTES
// スコア合成器の名称
//
/*static*/ const char
ModInvertedMinScoreCombiner::combinerName[] = "Min";


//
// FUNCTION public
// ModInvertedRankingScoreCombiner::create -- スコア合成器の生成
//
// NOTES
// スコア合成器記述に対応するスコア合成器を生成して返す。
//
// 現在、以下の合成器をサポートしている。
//	+ ModInvertedProdScoreCombiner
//	+ ModInvertedASumScoreCombiner
//	+ ModInvertedSumScoreCombiner
//
// ARGUMENTS
// const ModString& description
//		スコア合成器記述
// 
// RETURN
// 生成した合成器
//
// EXCEPTIONS
// ModInvertedErrorInvalidScoreCombiner -- 合成器の名称が正しくない
//
/*static*/ ModInvertedRankingScoreCombiner*
ModInvertedRankingScoreCombiner::create()
{
	ModString combinerName = ModInvertedSumScoreCombiner::combinerName;
	return create(combinerName);
}

/*static*/ ModInvertedRankingScoreCombiner*
ModInvertedRankingScoreCombiner::create(const ModString& description)
{
	if (description.compare(
		ModInvertedSumScoreCombiner::combinerName, ModFalse) == 0) {
		return new ModInvertedSumScoreCombiner();
	}
	else if (description.compare(
		ModInvertedASumScoreCombiner::combinerName, ModFalse) == 0) {
		return new ModInvertedASumScoreCombiner();
	}
	else if (description.compare(
		ModInvertedProdScoreCombiner::combinerName, ModFalse) == 0) {
		return new ModInvertedProdScoreCombiner();
	}
	else if (description.compare(
		ModInvertedMaxScoreCombiner::combinerName, ModFalse) == 0) {
		return new ModInvertedMaxScoreCombiner();
	}
	else if (description.compare(
		ModInvertedMinScoreCombiner::combinerName, ModFalse) == 0) {
		return new ModInvertedMinScoreCombiner();
	}

	ModErrorMessage << "Create failed: unknown ScoreCombiner: "
					<< description << ModEndl;
	ModThrowInvertedFileError(ModInvertedErrorInvalidScoreCombiner);
}

/*static*/ ModInvertedRankingScoreCombiner*
ModInvertedRankingScoreCombiner::create(const ModUnicodeString& description)
{
	ModUnicodeString tmp(description);
	return create(ModString(tmp.getString()));
}

//
// FUNCTION public
// ModInvertedRankingScoreCombiner::getDescription -- 記述文字列の獲得
//
// NOTES
// 自分を記述する文字列を獲得する。
//
// ARGUMENTS
// ModUnicodeString& description_
//		文字列表現
// const ModBoolean withParameter_
//		パラメータ出力指示
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedRankingScoreCombiner::getDescription(
	ModUnicodeString& description_,
	const ModBoolean withParameter_) const
{
	ModString tmp;
	getDescription(tmp, withParameter_);
	description_ = ModUnicodeString(tmp);
}

//
// Copyright (c) 1998, 1999, 2000, 2002, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
