// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedNormalizedOkapiTfScoreCalculator.h -- NormalizedOkapiTfランキングスコア計算器インターフェース
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

#ifndef __ModInvertedNormalizedOkapiTfScoreCalculator_h_
#define __ModInvertedNormalizedOkapiTfScoreCalculator_h_

#include "ModInvertedRankingScoreCalculator.h"
#include "ModInvertedOkapiTfScoreCalculator.h"

//
// CLASS
// ModInvertedNormalizedOkapiTfScoreCalculator -- NormalizedOkapiTfスコア計算器
//
// NOTES
// スコア計算式 :
//		tf / (k * ((1-lambda) + lambda * (ld / L) ))
//	tf: その文書におけるそのトークンの出現頻度
//	k:	文書内頻度調整用パラメータ
//  lambda:	文書長による正規化の調整用パラメータ
//	ld:	文書長
//	L:	登録済み全文書の文書長平均値
//
class
ModInvertedNormalizedOkapiTfScoreCalculator
	: public ModInvertedOkapiTfScoreCalculator
{
	friend class ModInvertedRankingScoreCalculator;
public:
	// コンストラクタ
	ModInvertedNormalizedOkapiTfScoreCalculator();
	ModInvertedNormalizedOkapiTfScoreCalculator(
		const ModInvertedNormalizedOkapiTfScoreCalculator&);

	// デストラクタ
	virtual ~ModInvertedNormalizedOkapiTfScoreCalculator() {}

	// 自分自身の複製
	virtual ScoreCalculator* duplicate() const;

	virtual DocumentScore firstStep(const ModSize tf,
									const DocumentID ID,
									ModBoolean& exist) const;


	// アクセサ関数 K:文書内頻度調整用パラメータ。
	virtual void setParameterLambda(const double lambda_);
	double getParameterLambda() const;

	// calculatorにパラメータをセット。
	virtual void setParameter(const ModString&);

	// 記述文字列の取得
	virtual void getDescription(ModString&, const ModBoolean = ModTrue) const;
	
protected:
	double lambda;					// 文書長による正規化の調整用パラメータ
	double pre1;					// = k*(1-lambda)
	double pre2;					// = k*lambda/averageDocumentLength

	void precalculate();

private:
	static const char calculatorName[];
};

#endif // __ModInvertedNormalizedOkapiTfScoreCalculator_h_

//
// Copyright (c) 1999, 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
