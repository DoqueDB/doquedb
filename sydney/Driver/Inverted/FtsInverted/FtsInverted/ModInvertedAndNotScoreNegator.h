// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedAndNotScoreNegator.h -- AndNot 用スコア否定器インタフェイス
// 
// Copyright (c) 1998, 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedAndNotScoreNegator_H__
#define __ModInvertedAndNotScoreNegator_H__

#include "ModInvertedRankingScoreNegator.h"

//
// CLASS
// ModInvertedAndNotScoreNegator -- ランキング用スコア否定器
//
// NOTES
// ランキング検索用 AndNot 演算子のスコア計算において、第2オペランドの
// スコア否定に使われるクラス。
//
class
ModInvertedAndNotScoreNegator
	: public ModInvertedRankingScoreNegator
{
	friend class ModInvertedRankingScoreNegator;
public:
	// 否定を表すランキング用スコアの計算
	virtual DocumentScore negate(DocumentScore x) const;

	// 自分自身の複製
	virtual ScoreNegator* duplicate() const;

	// 記述文字列の取得
	virtual void getDescription(ModString&, const ModBoolean = ModTrue) const;

protected:

private:
	static const char negatorName[];
};

//
// FUNCTION public
// ModInvertedAndNotScoreNegator::negate() -- ランキング用スコアの否定
//
// NOTES
// 与えられたランキング用スコア x の否定を表すランキング用スコアを求めて、
// それを返す。計算式は次の通り。
//			1 - x
//
// ARGUMENTS
// ModInvertedDocumentScore x
//		元のランキング用スコア
// 
// RETURN
// 得られたランキング用スコア
// 
// EXCEPTIONS
// なし
//
inline ModInvertedDocumentScore
ModInvertedAndNotScoreNegator::negate(DocumentScore x) const
{
	return 1.0 - x;
}

//
// FUNCTION public
// ModInvertedAndNotScoreNegator::duplicate() -- 自分自身の複製を作りそれを返す
//
// NOTES
// 自分自身の複製を作りそれを返す
//
// ARGUMENTS
// なし
// 
// RETURN
// 複製したスコア否定器
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModInvertedRankingScoreNegator*
ModInvertedAndNotScoreNegator::duplicate() const
{
	return new ModInvertedAndNotScoreNegator;
}

//
// FUNCTION public
// ModInvertedAndNotScoreNegator::getDescription -- 記述文字列の獲得
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
inline void
ModInvertedAndNotScoreNegator::getDescription(
	ModString& description_,
	const ModBoolean withParameter_) const
{
	description_ = negatorName;
}

#endif // __ModInvertedAndNotScoreNegator_H__

//
// Copyright (c) 1998, 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
