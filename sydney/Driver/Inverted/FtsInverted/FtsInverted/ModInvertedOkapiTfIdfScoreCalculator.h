// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOkapiTfIdfScoreCalculator.h -- OkapiTfIdfランキングスコア計算器インターフェース
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

#ifndef __ModInvertedOkapiTfIdfScoreCalculator_h_
#define __ModInvertedOkapiTfIdfScoreCalculator_h_

#include "ModInvertedRankingScoreCalculator.h"
#include "ModInvertedOkapiTfScoreCalculator.h"

//
// CLASS
// ModInvertedOkapiTfIdfScoreCalculator -- スコア計算器
//
// NOTES
// 以下の計算式に基づいてスコアを計算する。
//
//  N: データーベースに登録されている全登録文書数
//	  （totalDocumentFrequency_)
//  df: そのトークンを含む文書数
//  tf: その文書におけるそのトークンの出現頻度
//
// * First Step (TF 項)
//
//		  tf
//		------
//		k + tf
//
//  k:  文書内頻度調整用パラメータ
//
// * Second Step (DF 項)
//
//		       p0   N^s         a      (N/n) - 1
//		log((------*--- + 1 + ------)*-----------)
//		     1 - p0 n^s       1 - p0  (N/n)^s - 1
//
//  p0: 文書頻度調整用パラメータ
//  a:  文書頻度調整用パラメータ
//  s:  文書頻度調整用パラメータ
//  q0: 文書頻度調整用パラメータ
//
// ここで簡単にした計算式として以下のものを用意する
// 
// * Ogawa (a=0, s=1, q0=0)
//   OkapiTfIdf:x:k:1 <- default
//		      p0    N                 N
//		log(------*--- + 1) -> log(x*--- + 1)
//		    1 - p0  n                 n
//
//		x のデフォルト値 = 0.2	(p0 = 0.1666...)
//		k のデフォルト値 = 1	(OkapTf のデフォルト値)
//
//   OkapiTfIdf:x:k:4	:- x = p0, MAX=1 となるように正規化しない
//
// * Robertson (a=p0-1, s=1, q0=0)
//   OkapiTfIdf:x:k:0
//		      p0           N               N
//		log(------) + log(---) -> x + log(---)
//		    1 - p0         n               n
//
//   OkapiTfIdf:x:k:3	:- x = p0, MAX=1 となるように正規化しない
//
// * Harper/Croft (a=-1, s=1, q0=0)
//   OkapiTfIdf:x:k:2
//		      p0   N - n             N - n
//		log(------*-----) -> x + log(-----)
//		    1 - p0   n                 n
//
//   OkapiTfIdf:x:k:5	:- x = p0, MAX=1 となるように正規化しない
//
// * Ogawa2 (a=0, s=1)
//   OkapiTfIdf:x:k:6:q
//		       p0             q0                  x*N + n  
//		log((------*N + n)/(------*N + n)) -> log(-------)
//		     1 - p0         1 - q0                q*N + n
//
//		x のデフォルト値 = 0.2	(p0 = 0.1666...)
//		k のデフォルト値 = 1	(OkapTf のデフォルト値)
//		q のデフォルト値 = 0.0	(p0 = 0.0)
//
//   OkapiTfIdf:x:k:7:q	:- x = p0, q = q0, MAX=1 となるように正規化しない
//
// * Original
//	 OkapiTfIdf:x:k:8:q:q		:- x=p0, q=q0, MAX=1 となるように正規化しない
//	 OkapiTfIdf:x:k:9:q:a:s		:- x=p0, q=q0, MAX=1 となるように正規化しない
//		a のデフォルト値 = 0.0
//		q のデフォルト値 = 0.0
//		s のデフォルト値 = 1.0
//
class
ModInvertedOkapiTfIdfScoreCalculator
	: public ModInvertedOkapiTfScoreCalculator
{
	friend class ModInvertedRankingScoreCalculator;
public:
	// コンストラクタ
	ModInvertedOkapiTfIdfScoreCalculator();
	ModInvertedOkapiTfIdfScoreCalculator(
		const ModInvertedOkapiTfIdfScoreCalculator&);

	// デストラクタ
	virtual ~ModInvertedOkapiTfIdfScoreCalculator();

	// 自分自身の複製
	virtual ScoreCalculator* duplicate() const;

	// 重みの計算
	virtual DocumentScore secondStep(const ModSize df,
									 const ModSize totalDocument) const;

	// パラメータのアクセサ関数
	void setParameterX(const double x_);
	double getParameterX() const;
	void setParameterY(const int y_);
	int getParameterY() const;
	void setParameterA(const double a_);
	double getParameterA() const;
	void setParameterS(const double s_);
	double getParameterS() const;
	void setParameterQ(const double q_);
	double getParameterQ() const;

	// パラメータをセット
	virtual void setParameter(const ModString&);

	// 記述文字列の取得
	virtual void getDescription(ModString&, const ModBoolean = ModTrue) const;
	
protected:
	int y;
	double x;					// 文書頻度調整用パラメータ
	double a;					// 文書頻度調整用パラメータ
	double s;					// 文書頻度調整用パラメータ
	double q;					// 文書頻度調整用パラメータ

private:
	static const char calculatorName[];
};

#endif // __ModInvertedOkapiTfIdfScoreCalculator_h_

//
// Copyright (c) 1997, 1999, 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
