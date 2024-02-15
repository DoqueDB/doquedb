// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedIDScorePair.h -- 文書IDとランキング用スコアの対インタフェイス
// 
// Copyright (c) 1997, 1998, 1999, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedIDScorePair_H__
#define __ModInvertedIDScorePair_H__

#include "ModAssert.h"
#include "ModInvertedTypes.h"
#include "ModInvertedManager.h"

class ModInvertedRankingScoreCombiner;
class ModInvertedRankingScoreNegator;


//
// 以下の define を有効にすると、operator+ においてスコアの合成を代数和
// （x + y - xy）となる。無効の場合は算術和（x + y）となる。
// デフォルトでは定義しない。
//
// #define MOD_INV_ALGEBRAIC_SUM


//
// CLASS
// ModInvertedIDScorePair -- 文書IDとランキング用スコアの対
//
// NOTES
// 文書IDとランキング用スコアの対である。
//
class
ModInvertedIDScorePair : public ModInvertedObject {
public:
	// クラス定義内の行を短くするための typedef
	typedef ModInvertedIDScorePair		IDScorePair;
	typedef ModInvertedDocumentID		DocumentID;
	typedef ModInvertedDocumentScore	DocumentScore;
	typedef ModInvertedRankingScoreCombiner	ScoreCombiner;
	typedef ModInvertedRankingScoreNegator	ScoreNegator;

	// コンストラクタ
	ModInvertedIDScorePair();
	ModInvertedIDScorePair(DocumentID id_, DocumentScore score_);
	ModInvertedIDScorePair(const IDScorePair& original);

	// デストラクタ
	~ModInvertedIDScorePair() {}

	// 代入演算
	IDScorePair& operator=(const IDScorePair& original);

	// 順序比較 (文書IDの大小による比較)
	ModBoolean operator<(const IDScorePair& another) const;
	ModBoolean operator>(const IDScorePair& another) const;
	ModBoolean operator==(const IDScorePair& another) const;

	// 順序比較 (スコアの大小による比較)
	static ModBoolean scoreCompare(const IDScorePair& x,
								   const IDScorePair& y);

	// スコアの否定
	static ModStatus fuzzyNot(const IDScorePair& x,
							  const ScoreNegator& negator);

	// スコアの否定 (1 - x による否定)
	IDScorePair operator~() const;

	// スコアの論理積
	static ModStatus fuzzyAnd(const IDScorePair& x,
							  const IDScorePair& y,
							  IDScorePair& z,	// 結果格納用
							  const ScoreCombiner& combiner);

	// スコアの論理積 (最小値による合成)
	IDScorePair operator&(const IDScorePair& another) const;
	IDScorePair& operator&=(const IDScorePair& another);

	// スコアの論理積 (算術積=代数積による合成)
	IDScorePair operator*(const IDScorePair& another) const;
	IDScorePair& operator*=(const IDScorePair& another);

	// スコアの論理和
	static ModStatus fuzzyOr(const IDScorePair& x,
							 const IDScorePair& y,
							 IDScorePair& z,	// 結果格納用
							 const ScoreCombiner& combiner);

	// スコアの論理和 (最大値による合成)
	IDScorePair operator|(const IDScorePair& another) const;
	IDScorePair& operator|=(const IDScorePair& another);

	// スコアの論理和 (算術和による合成)
	IDScorePair operator+(const IDScorePair& another) const;
	IDScorePair& operator+=(const IDScorePair& another);

	// スコアの論理差
	static ModStatus fuzzyAndNot(const IDScorePair& x,
								 const IDScorePair& y,
								 IDScorePair& z,	// 結果格納用
								 const ScoreCombiner& combiner,
								 const ScoreNegator& negator);

	// スコアの論理差 (最小値による合成, 1 - x による否定)
//	IDScorePair operator~(const IDScorePair& another) const;
//	IDScorePair& operator~=(const IDScorePair& another);

	// スコアの論理差 (代数積による合成, 1 - x による否定)
	IDScorePair operator-(const IDScorePair& another) const;
	IDScorePair& operator-=(const IDScorePair& another);

	// 文書ID
	DocumentID id;
	// スコア
	DocumentScore score;

protected:

private:

};

//
// FUNCTION public
// ModInvertedIDScorePair::ModInvertedIDScorePair -- 文書IDとスコアの対の生成
//
// NOTES
// 文書IDとランキング用スコアの対を新しく生成する。
//
// ARGUMENTS
// ModInvertedDocumentID id_
//		文書ID;
// ModInvertedDocumentScore score_
//		ランキング用スコア
//
// const ModInvertedIDScorePair& original
//		元の文書IDとランキング用スコアの対
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModInvertedIDScorePair::ModInvertedIDScorePair()
//	: id(0), score(0.0)
{}

inline
ModInvertedIDScorePair::ModInvertedIDScorePair(ModInvertedDocumentID id_,
											   ModInvertedDocumentScore score_)
	: id(id_), score(score_)
{}

inline
ModInvertedIDScorePair:: ModInvertedIDScorePair(
	const ModInvertedIDScorePair& original)
	: id(original.id), score(original.score)
{}

//
// FUNCTION public
// ModInvertedIDScorePair::operator= -- 文書IDとスコアの対の代入
//
// NOTES
// 文書IDとランキング用スコアの対を代入する。
//
// ARGUMENTS
// const ModInvertedIDScorePair& original
//		元の文書IDとランキング用スコアの対
//
// RETURN
// *this (変更後のもの)
//
// EXCEPTIONS
// なし
//
inline ModInvertedIDScorePair&
ModInvertedIDScorePair::operator=(const ModInvertedIDScorePair& original)
{
	this->id = original.id;
	this->score = original.score;
	return *this;
}

// 
// FUNCTION public
// ModInvertedIDScorePair::operator< -- 順序比較 (文書IDの大小による順序)
// 
// NOTES
// 文書IDとスコアの対の文書IDの大小による順序で、*this の文書IDの方が
// another の文書IDより小さいかどうか調べる。
// 
// ARGUMENTS
// const ModInvertedIDScorePair& another
//		もう一方の文書IDとスコアの対
// 
// RETURN
// *this の文書IDの方が another の文書IDより小さい場合 ModTrue、
// それ以外の場合 ModFalse
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedIDScorePair::operator<(const ModInvertedIDScorePair& another) const
{
	return this->id < another.id ? ModTrue : ModFalse;
}

// 
// FUNCTION public
// ModInvertedIDScorePair::operator> -- 順序比較 (文書IDの大小による順序)
// 
// NOTES
// 文書IDとスコアの対の文書IDの大小による順序で、*this の文書IDの方が
// another の文書IDより大きいかどうか調べる。
// 
// ARGUMENTS
// const ModInvertedIDScorePair& another
//		もう一方の文書IDとスコアの対
// 
// RETURN
// *this の文書IDの方が another の文書IDより大きい場合 ModTrue、
// それ以外の場合 ModFalse
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedIDScorePair::operator>(const ModInvertedIDScorePair& another) const
{
	return this->id > another.id ? ModTrue : ModFalse;
}

// 
// FUNCTION public
// ModInvertedIDScorePair::operator== -- 順序比較 (文書IDの大小による順序)
// 
// NOTES
// 文書IDとスコアの対の文書IDの大小による順序で、*this の文書IDが
// another の文書IDと等しいかどうか調べる。
// 
// ARGUMENTS
// const ModInvertedIDScorePair& another
//		もう一方の文書IDとスコアの対
// 
// RETURN
// *this の文書IDが another の文書IDと等しい場合 ModTrue、
// それ以外の場合 ModFalse
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedIDScorePair::operator==(const ModInvertedIDScorePair& another) const
{
	return this->id == another.id ? ModTrue : ModFalse;		// スコアは無視する
}

// 
// FUNCTION public static
// ModInvertedIDScorePair::scoreCompare -- 順序比較 (スコアの大小による順序)
// 
// NOTES
// ２つの文書IDとスコアの対を、スコアの大小で比較する。
// 
// ARGUMENTS
// const ModInvertedIDScorePair& x
//		第１の文書IDとスコアの対
// const ModInvertedIDScorePair& y
//		第２の文書IDとスコアの対
// 
// RETURN
// x のスコアの方が y のスコアより大きい場合、又は両者のスコアが等しくて
// x の文書IDの方が y の文書IDよりも小さい場合 ModTrue、
// それ以外の場合 ModFalse
// 
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedIDScorePair::scoreCompare(const ModInvertedIDScorePair& x,
									 const ModInvertedIDScorePair& y)
{
	return (x.score > y.score) || ((x.score == y.score) && (x.id < y.id)) ?
	  ModTrue : ModFalse;
}

// 
// FUNCTION public
// ModInvertedIDScorePair::operator~ -- スコアの否定 (1 - x)
// 
// NOTES
// *this と同じ文書IDを持ち、スコアは 1 から *this のスコアを減じたものに
// なるような、文書IDとスコアの対を作って、それを返す。
// 
// ARGUMENTS
// なし。
// 
// RETURN
// 新しく作った、文書IDとスコアの対
// 
// EXCEPTIONS
// なし
//
inline ModInvertedIDScorePair
ModInvertedIDScorePair::operator~() const
{
	return ModInvertedIDScorePair(this->id,
								  (this->score > 1.0) ? 0.0 : 1.0 - this->score);
}

// 
// FUNCTION public
// ModInvertedIDScorePair::operator& -- スコアの論理積 (最小値)
// 
// NOTES
// *this の文書IDと another の文書IDが等しいことを前提に、スコアが
// *this のスコアと another のスコアの最小値になるような、文書IDとスコアの対を
// 作って、それを返す。
// 
// ARGUMENTS
// const ModInvertedIDScorePair& another
//	もう一方の文書IDとスコアの対
// 
// RETURN
// 新しく作った、文書IDとスコアの対
// 
// EXCEPTIONS
// なし
//
inline ModInvertedIDScorePair
ModInvertedIDScorePair::operator&(const ModInvertedIDScorePair& another) const
{
	; ModAssert(this->id == another.id);
	return ModInvertedIDScorePair(this->id,
								  (this->score < another.score) ?
								  this->score : another.id);
}

// 
// FUNCTION public
// ModInvertedIDScorePair::operator&= -- スコアの論理積代入 (最小値)
// 
// NOTES
// *this の文書IDと another の文書IDが等しいことを前提に、*this のスコアを
// *this のスコアと another のスコアの最小値に変更する。
// 
// ARGUMENTS
// const ModInvertedIDScorePair& another
//		もう一方の文書IDとスコアの対
// 
// RETURN
// *this (変更後のもの)
// 
// EXCEPTIONS
// なし
//
inline ModInvertedIDScorePair&
ModInvertedIDScorePair::operator&=(const ModInvertedIDScorePair& another)
{
	; ModAssert(this->id == another.id);
	if (this->score > another.score) {
		this->score = another.score;
	}
	return *this;
}

// 
// FUNCTION public
// ModInvertedIDScorePair::operator* -- スコアの論理積 (代数積)
// 
// NOTES
// *this の文書IDと another の文書IDが等しいことを前提に、スコアが
// *this のスコアと another のスコアの積になるような、文書IDとスコアの対を
// 作って、それを返す。
// 
// ARGUMENTS
// const ModInvertedIDScorePair& another
//		もう一方の文書IDとスコアの対
// 
// RETURN
// 新しく作った、文書IDとスコアの対
// 
// EXCEPTIONS
// なし
//
inline ModInvertedIDScorePair
ModInvertedIDScorePair::operator*(const ModInvertedIDScorePair& another) const
{
	; ModAssert(this->id == another.id);
	return ModInvertedIDScorePair(this->id,
								  this->score * another.score);
}

// 
// FUNCTION public
// ModInvertedIDScorePair::operator*= -- スコアの論理積代入 (代数積)
// 
// NOTES
// *this の文書IDと another の文書IDが等しいことを前提に、*this のスコアを
// *this のスコアと another のスコアの積に変更する。
// 
// ARGUMENTS
// const ModInvertedIDScorePair& another
//		もう一方の文書IDとスコアの対
// 
// RETURN
// *this (変更後のもの)
// 
// EXCEPTIONS
// なし
//
inline ModInvertedIDScorePair&
ModInvertedIDScorePair::operator*=(const ModInvertedIDScorePair& another)
{
	; ModAssert(this->id == another.id);
	this->score *= another.score;
	return *this;
}

// 
// FUNCTION public
// ModInvertedIDScorePair::operator| -- スコアの論理和 (最大値)
// 
// NOTES
// *this の文書IDと another の文書IDが等しいことを前提に、スコアが
// *this のスコアと another のスコアの最大値になるような、文書IDとスコアの対を
// 作って、それを返す。
// 
// ARGUMENTS
// const ModInvertedIDScorePair& another
//		もう一方の文書IDとスコアの対
// 
// RETURN
// 新しく作った、文書IDとスコアの対
// 
// EXCEPTIONS
// なし
//
inline ModInvertedIDScorePair
ModInvertedIDScorePair::operator|(const ModInvertedIDScorePair& another) const
{
	; ModAssert(this->id == another.id);
	return ModInvertedIDScorePair(this->id,
								  (this->score < another.score) ?
								  another.score : this->score);
}

// 
// FUNCTION public
// ModInvertedIDScorePair::operator|= -- スコアの論理和代入 (最大値)
// 
// NOTES
// *this の文書IDと another の文書IDが等しいことを前提に、*this のスコアを
// *this のスコアと another のスコアの最大値に変更する。
// 
// ARGUMENTS
// const ModInvertedIDScorePair& another
//		もう一方の文書IDとスコアの対
// 
// RETURN
// *this (変更後のもの)
// 
// EXCEPTIONS
// なし
//
inline ModInvertedIDScorePair&
ModInvertedIDScorePair::operator|=(const ModInvertedIDScorePair& another)
{
	; ModAssert(id == another.id);
	if (this->score < another.score) {
		this->score = another.score;
	}
	return *this;
}

// 
// FUNCTION public
// ModInvertedIDScorePair::operator+ -- スコアの論理和 (算術和)
// 
// NOTES
// *this の文書IDと another の文書IDが等しいことを前提に、スコアが
// *this のスコアと another のスコアの代数和になるような、文書IDとスコアの対を
// 作って、それを返す。
// 
// ARGUMENTS
// const ModInvertedIDScorePair& another
//		もう一方の文書IDとスコアの対
// 
// RETURN
// 新しく作った、文書IDとスコアの対
// 
// EXCEPTIONS
// なし
//
inline ModInvertedIDScorePair
ModInvertedIDScorePair::operator+(const ModInvertedIDScorePair& another) const
{
	; ModAssert(this->id == another.id);
#ifdef MOD_INV_ALGEBRAIC_SUM
	return ModInvertedIDScorePair(this->id,
								  (this->score > 1.0 || another.score > 1.0) ?
								  1.0 :
								  this->score + another.score -
								  this->score * another.score);
#else
	return ModInvertedIDScorePair(this->id,
								  this->score + another.score);
#endif
}

// 
// FUNCTION public
// ModInvertedIDScorePair::operator+= -- スコアの論理和代入 (代数和)
// 
// NOTES
// *this の文書IDと another の文書IDが等しいことを前提に、*this のスコアを
// *this のスコアと another のスコアの代数和に変更する。
// 
// ARGUMENTS
// const ModInvertedIDScorePair& another
//		もう一方の文書IDとスコアの対
// 
// RETURN
// *this (変更後のもの)
// 
// EXCEPTIONS
// なし
//
inline ModInvertedIDScorePair&
ModInvertedIDScorePair::operator+=(const ModInvertedIDScorePair& another)
{
	; ModAssert(this->id == another.id);
#ifdef MOD_INV_ALGEBRAIC_SUM
	; ModAssert(this->score >= 0 && this->score <= 1);
	; ModAssert(another.score >= 0 && another.score <= 1);
	this->score += another.score * (1 - this->score);
#else
	this->score += another.score;
#endif
	return *this;
}


#if 0
// 
// FUNCTION public
// ModInvertedIDScorePair::operator~ -- スコアの論理差 (最小値, 1 - x)
// 
// NOTES
// *this の文書IDと another の文書IDが等しいことを前提に、スコアが
// *this のスコアと another のスコアの論理差になるような、文書IDとスコアの対を
// 作って、それを返す。
// 論理差のスコアは、min{x, 1 - y} で求める。
// 
// ARGUMENTS
// const ModInvertedIDScorePair& another
//		もう一方の文書IDとスコアの対
// 
// RETURN
// 新しく作った、文書IDとスコアの対
// 
// EXCEPTIONS
// なし
//
inline ModInvertedIDScorePair
ModInvertedIDScorePair::operator~(const ModInvertedIDScorePair& another) const
{
	ModInvertedIDScorePair tmp(*this);
	; ModAssert(this->id == another.id);
	; ModAssert(this->score >= 0 && this->score <= 1);
	; ModAssert(another.score >= 0 && another.score <= 1);
	if (this->score > 1 - another.score) {
		tmp.score = 1 - another.score;
	}
	return tmp;
}

// 
// FUNCTION public
// ModInvertedIDScorePair::operator~= -- スコアの論理差代入 (最小値, 1 - x)
// 
// NOTES
// *this の文書IDと another の文書IDが等しいことを前提に、*this のスコアを
// *this のスコアと another のスコアの論理差に変更する。
// 論理差のスコアは、min{x, 1 - y} で求める。
// 
// ARGUMENTS
// const ModInvertedIDScorePair& another
//		もう一方の文書IDとスコアの対
// 
// RETURN
// *this (変更後のもの)
// 
// EXCEPTIONS
// なし
//
inline ModInvertedIDScorePair&
ModInvertedIDScorePair::operator~=(const ModInvertedIDScorePair& another)
{
	; ModAssert(this->id == another.id);
	; ModAssert(this->score >= 0 && this->score <= 1);
	; ModAssert(another.score >= 0 && another.score <= 1);
	if (this->score > 1 - another.score) {
		this->score = 1 - another.score;
	}
	return *this;
}
#endif

// 
// FUNCTION public
// ModInvertedIDScorePair::operator- -- スコアの論理差 (代数積, 1 - x)
// 
// NOTES
// *this の文書IDと another の文書IDが等しいことを前提に、スコアが
// *this のスコアと another のスコアの論理差になるような、文書IDとスコアの対を
// 作って、それを返す。
// 論理差のスコアは、x * (1 - y) で求める。
// 
// ARGUMENTS
// const ModInvertedIDScorePair& another
//		もう一方の文書IDとスコアの対
// 
// RETURN
// 新しく作った、文書IDとスコアの対
// 
// EXCEPTIONS
// なし
//
inline ModInvertedIDScorePair
ModInvertedIDScorePair::operator-(const ModInvertedIDScorePair& another) const
{
	; ModAssert(this->id == another.id);
	; ModAssert(this->score >= 0 && this->score <= 1);
	; ModAssert(another.score >= 0 && another.score <= 1);
	return ModInvertedIDScorePair(this->id,
								  this->score*(1 - another.score));
}

// 
// FUNCTION public
// ModInvertedIDScorePair::operator-= -- スコアの論理差代入 (代数積, 1 - x)
// 
// NOTES
// *this の文書IDと another の文書IDが等しいことを前提に、*this のスコアを
// *this のスコアと another のスコアの論理差に変更する。
// 論理差のスコアは、x * (1 - y) で求める。
// 
// ARGUMENTS
// const ModInvertedIDScorePair& another
//		もう一方の文書IDとスコアの対
// 
// RETURN
// *this (変更後のもの)
// 
// EXCEPTIONS
// なし
//
inline ModInvertedIDScorePair&
ModInvertedIDScorePair::
operator-=(const ModInvertedIDScorePair& another)
{
	; ModAssert(this->id == another.id);
	; ModAssert(this->score >= 0 && this->score <= 1);
	; ModAssert(another.score >= 0 && another.score <= 1);
	this->score *= 1 - another.score;
	return *this;
}

#endif //__ModInvertedIDScorePair_H__

//
// Copyright (c) 1997, 1998, 1999, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
