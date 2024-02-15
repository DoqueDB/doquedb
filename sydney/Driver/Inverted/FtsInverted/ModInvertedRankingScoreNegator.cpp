// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedRankingScoreNegator.h -- ランキングスコア否定器の宣言
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2002, 2023 Ricoh Company, Ltd.
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
#include "ModInvertedRankingScoreNegator.h"
#include "ModInvertedAndNotScoreNegator.h"

//
// CONST
// ModInvertedAndNotScoreNegator::negatorName -- 否定器名
//
// NOTES
// スコア否定器の名称
//
/*static*/ const char
ModInvertedAndNotScoreNegator::negatorName[] = "AndNot";


// 
// FUNCTION public
// ModInvertedRankingScoreNegator::create -- スコア否定器の生成
// 
// NOTES
// スコア否定器記述に対応するスコア否定器を生成して返す。
//
// 現在、以下の否定器をサポートしている。
//	+ ModInvertedAndNotScoreCombiner
//
// ARGUMENTS
// const ModString& negatorName
//		スコア否定器記述
// 
// RETURN
// 生成したスコア否定器
// 
// EXCEPTIONS
// ModInvertedErrorInvalidScoreNegator -- 否定器の名称が正しくない
//
/*static*/ ModInvertedRankingScoreNegator*
ModInvertedRankingScoreNegator::create()
{
	ModString negatorName = ModInvertedAndNotScoreNegator::negatorName;
	return create(negatorName);
}

/*static*/ ModInvertedRankingScoreNegator*
ModInvertedRankingScoreNegator::create(const ModString& negatorName)
{
	if (negatorName.compare(
		ModInvertedAndNotScoreNegator::negatorName, ModFalse) == 0) {
		return new ModInvertedAndNotScoreNegator();
	}

	ModErrorMessage << "Create failed: unknown ScoreNegator: "
					<< negatorName << ModEndl;
	ModThrowInvertedFileError(ModInvertedErrorInvalidScoreNegator);
}

/*static*/ ModInvertedRankingScoreNegator*
ModInvertedRankingScoreNegator::create(const ModUnicodeString& description)
{
	ModUnicodeString tmp(description);
	return create(ModString(tmp.getString()));
}

//
// FUNCTION public
// ModInvertedRankingScoreNegator::getDescription -- 記述文字列の獲得
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
ModInvertedRankingScoreNegator::getDescription(
	ModUnicodeString& description_,
	const ModBoolean withParameter_) const
{
	ModString tmp;
	getDescription(tmp, withParameter_);
	description_ = ModUnicodeString(tmp);
}

//
// Copyright (c) 1997, 1998, 1999, 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
