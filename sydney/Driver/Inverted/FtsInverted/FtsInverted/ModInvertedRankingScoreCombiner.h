// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedRankingScoreCombiner.h -- ランキングスコア合成器の宣言
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedRankingScoreCombiner_H__
#define __ModInvertedRankingScoreCombiner_H__

#include "ModTypes.h"
#include "ModString.h"
#include "ModVector.h"

#include "ModInvertedTypes.h"
#include "ModInvertedManager.h"

class ModUnicodeString;


//
// CLASS
// ModInvertedRankingScoreCombiner -- ランキング用スコア合成器
//
// NOTES
// ランキング検索において、スコアを合成する。
//
class
ModInvertedRankingScoreCombiner : public ModInvertedObject
{
public:
	// クラス定義内の行を短くするための typedef
	typedef ModInvertedRankingScoreCombiner	ScoreCombiner;
	typedef ModInvertedDocumentScore		DocumentScore;

	// スコア合成器の生成
	static ScoreCombiner* create();
	static ScoreCombiner* create(const ModString& s);
	static ScoreCombiner* create(const ModUnicodeString& s);

	virtual ~ModInvertedRankingScoreCombiner();

	// ランキング用スコアの合成
	DocumentScore operator()(const DocumentScore x, const DocumentScore y) const;
	virtual DocumentScore combine(const DocumentScore x,
								  const DocumentScore y) const = 0;
	virtual DocumentScore apply(const ModVector<DocumentScore>& scoreVector)
		const = 0;

	// 交換律は成り立つか？
	virtual ModBoolean isCommutative() const;

	// 結合律は成り立つか？
	virtual ModBoolean isAssociative() const;

	// 自分自身の複製
	virtual ScoreCombiner* duplicate() const = 0;

	// 記述文字列の取得
	// virtual void getCombinerNameString(ModString &name) {
	// 	; ModAssert(0); /*UNICODE コンパイルできるようにするために残してある*/ }
	virtual void getDescription(ModString&,
								const ModBoolean = ModTrue) const = 0;
	void getDescription(ModUnicodeString&, const ModBoolean = ModTrue) const;

protected:

private:

};


// 
// FUNCTION public
// ModInvertedRankingScoreCombiner::~ModInvertedRankingScoreCombiner -- デストラクタ
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
ModInvertedRankingScoreCombiner::~ModInvertedRankingScoreCombiner()
{}

//
// FUNCTION public
// ModInvertedRankingScoreCombiner::operator() -- ランキング用スコアの合成
//
// NOTES
// 与えられた２つのランキング用スコア x, y を合成したランキング用
// スコアを求めて、それを返す。
// 実際には combine() を呼び出すフォワーディング関数である。
//
// ARGUMENTS
// const ModInvertedDocumentScore x
//		第１のランキング用スコア
// const ModInvertedDocumentScore y
//		第２のランキング用スコア
// 
// RETURN
// 合成結果のランキング用スコア
// 
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModInvertedDocumentScore
ModInvertedRankingScoreCombiner::operator()(const DocumentScore x,
											const DocumentScore y) const
{
	return this->combine(x, y);
}

//
// FUNCTION public
// ModInvertedRankingScoreCombiner::isCommutative() -- 交換律が成り立つか？
//
// NOTES
// 交換律が成り立つような合成演算であるかどうかを返す。
// ModInvertedRankingScoreCombiner ではデフォルトの挙動を定めるので、
// ここでは必ず ModTrue を返す。
//
// ARGUMENTS
// なし
// 
// RETURN
// 必ず ModTrue
// 
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedRankingScoreCombiner::isCommutative() const
{
	return ModTrue;
}

//
// FUNCTION public
// ModInvertedRankingScoreCombiner::isAssociative() -- 結合律が成り立つか？
//
// NOTES
// 結合律が成り立つような合成演算であるかどうかを返す。
// ModInvertedRankingScoreCombiner ではデフォルトの挙動を定めるので、
// ここでは必ず ModTrue を返す。
//
// ARGUMENTS
// なし
// 
// RETURN
// 必ず ModTrue
// 
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedRankingScoreCombiner::isAssociative() const
{
	return ModTrue;
}

#endif //__ModInvertedRankingScoreCombiner_H__

//
// Copyright (c) 1997, 1998, 1999, 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
