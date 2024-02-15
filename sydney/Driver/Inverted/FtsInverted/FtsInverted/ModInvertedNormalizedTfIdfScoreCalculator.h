// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedNormalizedTfIdfScoreCalculator.h -- NormalizedTfIdf ランキングスコア計算器インターフェース
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

#ifndef __ModInvertedNormalizedTfIdfScoreCalculator_h_
#define __ModInvertedNormalizedTfIdfScoreCalculator_h_

#include "ModInvertedRankingScoreCalculator.h"
#include "ModInvertedTfIdfScoreCalculator.h"

//
// CLASS
// ModInvertedNormalizedTfIdfScoreCalculator -- スコア計算器
//
// NOTES
// スコア計算式 :
//		(k1 + k2 * (tf / ((1 - lambda) * L + lambda * ld))) * log(x + N / df ) / log(x + N)
//	N: データーベースに登録されている全登録文書数
//		（totalDocumentFrequency_)
//	df: そのトークンを含む文書数
//	tf: その文書におけるそのトークンの出現頻度
//	k1:	文書内頻度調整用パラメータ1
//	k2:	文書内頻度調整用パラメータ2
//  lambda:	文書長による正規化の調整用パラメータ
//	ld:	文書長
//	L:	登録済み全文書の文書長平均値
//	x:	文書頻度調整用パラメータ 
//
class
ModInvertedNormalizedTfIdfScoreCalculator
	: public ModInvertedTfIdfScoreCalculator
{
	friend class ModInvertedRankingScoreCalculator;
public:
	// コンストラクタ
	ModInvertedNormalizedTfIdfScoreCalculator();
	ModInvertedNormalizedTfIdfScoreCalculator(const ModInvertedNormalizedTfIdfScoreCalculator&);

	// デストラクタ
	virtual ~ModInvertedNormalizedTfIdfScoreCalculator();

	// 自分自身の複製
	virtual ScoreCalculator* duplicate() const;

	virtual DocumentScore firstStep(const ModSize tf,
									const DocumentID ID,
									ModBoolean& exist) const;

	// アクセサ関数 K:文書内頻度調整用パラメータ。
	virtual void setParameterLambda(const double lambda_);
	double getParameterLambda() const;

	virtual void setParameter(const ModString&);

	// 記述文字列の取得
	virtual void getDescription(ModString&, const ModBoolean = ModTrue) const;
	
protected:
	double lambda;					// 文書長による正規化の調整用パラメータ

private:
	static const char calculatorName[];
};

#endif // __ModInvertedNormalizedTfIdfScoreCalculator_h_

//
// Copyright (c) 1999, 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
