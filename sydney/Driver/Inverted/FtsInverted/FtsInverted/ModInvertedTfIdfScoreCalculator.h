// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedTfIdfScoreCalculator.h -- ランキングスコア計算器インターフェース
// 
// Copyright (c) 1999, 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedTfIdfScoreCalculator_h_
#define __ModInvertedTfIdfScoreCalculator_h_

#include "ModInvertedRankingScoreCalculator.h"

//
// CLASS
// ModInvertedTfIdfScoreCalculator -- スコア計算器
//
// NOTES
// スコア計算式 :
//		(k1 + k2 * tf) * (x + log(N / df)) / (x + log(N))	: y == 0
//		(k1 + k2 * tf) * log(1 + x*(N / df)) / log(1 + x*N)	: y != 0
//
//	N: データーベースに登録されている全登録文書数
//		（totalDocumentFrequency_)
//	df: そのトークンを含む文書数
//	tf: その文書におけるそのトークンの出現頻度
//	k1: 文書内頻度調整用パラメータ1
//	k2: 文書内頻度調整用パラメータ2
//
class
ModInvertedTfIdfScoreCalculator
	: public ModInvertedRankingScoreCalculator
{
	friend class ModInvertedRankingScoreCalculator;
public:
	// コンストラクタ
	ModInvertedTfIdfScoreCalculator();
	ModInvertedTfIdfScoreCalculator(const ModInvertedTfIdfScoreCalculator&);

	// デストラクタ
	virtual ~ModInvertedTfIdfScoreCalculator();

	// 自分自身の複製
	virtual ScoreCalculator* duplicate() const;

	// 重みの計算
	virtual DocumentScore firstStep(const ModSize tf,
									const DocumentID ID,
									ModBoolean& exist) const;
	virtual DocumentScore secondStep(const ModSize df,
									 const ModSize totalDocument) const;

	void setParameterK1(const double k1_);
	double getParameterK1() const;

	void setParameterK2(const double k2_);
	double getParameterK2() const;

	void setParameterX(const double x_);
	double getParameterX() const;

	void setParameterY(const int y_);
	int getParameterY() const;

	// calculatorにパラメータをセット。
	virtual void setParameter(const ModString&);

	// 記述文字列の取得
	virtual void getDescription(ModString&, const ModBoolean = ModTrue) const;
	
protected:
	double k1;					// 文書内頻度調整用パラメータ1。デフォルトは0。
	double k2;					// 文書内頻度調整用パラメータ2。デフォルトは1。
	double x;					// 文書頻度調整用パラメータ。デフォルト0。
	int y;						// きりかえ用

private:
	static const char calculatorName[];
};

#endif // __ModInvertedTfIdfScoreCalculator_h_

//
// Copyright (c) 1999, 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
