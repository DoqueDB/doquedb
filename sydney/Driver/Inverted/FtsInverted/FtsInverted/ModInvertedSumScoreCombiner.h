// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedSumScoreCombiner.h -- 和スコア合成器の宣言
// 
// Copyright (c) 1999, 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedSumScoreCombiner_H__
#define __ModInvertedSumScoreCombiner_H__

#include "ModInvertedRankingScoreCombiner.h"

//
// CLASS
// ModInvertedSumScoreCombiner -- 和用スコア合成器
//
// NOTES
// ランキング検索において、スコアを(単純)和を用いて合成する。
//
class
ModInvertedSumScoreCombiner
	: public ModInvertedRankingScoreCombiner
{
	friend class ModInvertedRankingScoreCombiner;
public:
	// ランキング用スコアの合成
	DocumentScore combine(const DocumentScore x, const DocumentScore y) const;
	DocumentScore apply(const ModVector<DocumentScore>& scoreVector) const;

	// 自分自身の複製
	virtual ScoreCombiner* duplicate() const;

	// 記述文字列の取得
	virtual void getDescription(ModString&, const ModBoolean = ModTrue) const;

protected:

private:
	static const char combinerName[];
};

//
// FUNCTION public
// ModInvertedSumScoreCombiner::combine -- 代数和用スコア合成
//
// NOTES
// 代数和用スコア合成を計算しかえす
//
// ARGUMENTS
// DocumentScore x
//	  第１のランキング用スコア
// DocumentScore y
//	  第２のランキング用スコア
// 
// RETURN
// 合成結果のランキング用スコア
//
// EXCEPTIONS
// なし
//
inline ModInvertedDocumentScore
ModInvertedSumScoreCombiner::combine(const DocumentScore x,
									 const DocumentScore y) const
{
	return x + y;
}

//
// FUNCTION public
// ModInvertedSumScoreCombiner::apply -- スコアのVectorから合成
//
// NOTES
// スコアのVectorを受け取り、そのVectorに含まれている全てのスコアを代
// 数積で合成して返す。
//
// ARGUMENTS
// ModVector<DocumentScore>& scoreVector
//		スコアのVector
// 
// RETURN
// 合成結果のランキング用スコア
// 
// EXCEPTIONS
// なし
//
inline ModInvertedDocumentScore
ModInvertedSumScoreCombiner::apply(
	const ModVector<DocumentScore>& scoreVector) const
{
	ModVector<DocumentScore>::ConstIterator p = scoreVector.begin();
	ModVector<DocumentScore>::ConstIterator end = scoreVector.end();

	DocumentScore score = *p;			// 最初の要素の計算
	++p;

	while (p != end){
		score += *p;
		++p;
	}
	return score;
}

//
// FUNCTION public
// ModInvertedSumScoreCombiner::duplicate -- 自分自身の複製
//
// NOTES
// 自分自身の複製
//
// ARGUMENTS
// なし
// 
// RETURN
// ScoreCombiner*
// 
// EXCEPTIONS
// なし
//
inline ModInvertedRankingScoreCombiner*
ModInvertedSumScoreCombiner::duplicate() const
{
	return new ModInvertedSumScoreCombiner();
}

//
// FUNCTION public
// ModInvertedSumScoreCombiner::getDescription -- 記述文字列の獲得
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
ModInvertedSumScoreCombiner::getDescription(
	ModString& description_,
	const ModBoolean withParameter_) const
{
	description_ = combinerName;
}

#endif // __ModInvertedSumScoreCombiner_H__

//
// Copyright (c) 1999, 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
