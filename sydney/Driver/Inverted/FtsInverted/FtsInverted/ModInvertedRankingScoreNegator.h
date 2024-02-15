// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedRankingScoreNegator.h -- ランキング用スコア否定器インタフェイス
// 
// Copyright (c) 1997, 1999, 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedRankingScoreNegator_H__
#define __ModInvertedRankingScoreNegator_H__

#include "ModInvertedTypes.h"
#include "ModInvertedManager.h"
#include "ModString.h"

class ModUnicodeString;

//
// CLASS
// ModInvertedRankingScoreNegator -- ランキング用スコア否定器
//
// NOTES
// ランキング検索において、否定を表すスコアを求める。
//
class
ModInvertedRankingScoreNegator : public ModInvertedObject
{
public:
	// クラス定義内の行を短くするための typedef
	typedef ModInvertedDocumentScore			DocumentScore;
	typedef ModInvertedRankingScoreNegator		ScoreNegator;

	// スコア否定器の生成
	static ScoreNegator* create();
	static ScoreNegator* create(const ModString& s);
	static ScoreNegator* create(const ModUnicodeString& s);

	virtual ~ModInvertedRankingScoreNegator();

	// 否定を表すランキング用スコアの計算
	virtual DocumentScore negate(DocumentScore x) const = 0;
	DocumentScore operator()(DocumentScore x) const;

	// 自分自身の複製
	virtual ScoreNegator* duplicate() const = 0;

	// 記述文字列の取得
	virtual void getDescription(ModString&,
								const ModBoolean = ModTrue) const = 0;
	void getDescription(ModUnicodeString&, const ModBoolean = ModTrue) const;

protected:

private:
};


// 
// FUNCTION public
// ModInvertedRankingScoreNegator::~ModInvertedRankingScoreNegator -- デストラクタ
// 
// NOTES
// デストラクタ。
// 
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModInvertedRankingScoreNegator::~ModInvertedRankingScoreNegator()
{}

//
// FUNCTION public
// ModInvertedRankingScoreNegator::operator() -- ランキング用スコアの否定
//
// NOTES
// 与えられたランキング用スコア x の否定を表すランキング用スコアを求めて、
// それを返す。
// 実際には negate() を呼び出すフォワーディング関数である。
//
// ARGUMENTS
// ModInvertedDocumentScore x
//		元のランキング用スコア
// 
// RETURN
// 得られたランキング用スコア
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModInvertedDocumentScore
ModInvertedRankingScoreNegator::operator()(DocumentScore x) const
{
	return this->negate(x);
}

#endif	// __ModInvertedRankingScoreNegator_H__

//
// Copyright (c) 1997, 1999, 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
