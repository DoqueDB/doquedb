// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedOkapiTfScoreCalculator.h -- OkapiTfランキングスコア計算器インターフェース
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

#ifndef __ModInvertedOkapiTfScoreCalculator_h_
#define __ModInvertedOkapiTfScoreCalculator_h_

#include "ModInvertedRankingScoreCalculator.h"

//
// CLASS
// ModInvertedOkapiTfScoreCalculator -- OkapiTfスコア計算器
//
// NOTES
// スコア計算式 :
//		tf / ( 1 + tf )
//	tf: その文書におけるそのトークンの出現頻度
//
class
ModInvertedOkapiTfScoreCalculator
	: public ModInvertedRankingScoreCalculator
{
	friend class ModInvertedRankingScoreCalculator;
public:
	// コンストラクタ
	ModInvertedOkapiTfScoreCalculator();
	ModInvertedOkapiTfScoreCalculator(const ModInvertedOkapiTfScoreCalculator&);

	// デストラクタ
	virtual ~ModInvertedOkapiTfScoreCalculator() {}

	// 自分自身の複製
	virtual ScoreCalculator* duplicate() const;

	// 重みの計算
	virtual DocumentScore firstStep(const ModSize tf,
									const DocumentID ID,
									ModBoolean& exist) const;
	virtual DocumentScore secondStep(const ModSize df,
									 const ModSize totalDocument) const;

	// アクセサ関数 K:文書内頻度調整用パラメータ。
	virtual void setParameterK(const double k_);
	double getParameterK() const;

	// calculatorにパラメータをセット。
	virtual void setParameter(const ModString&);

	// 記述文字列の取得
	virtual void getDescription(ModString&, const ModBoolean = ModTrue) const;
	
protected:
	double k;					// 文書内頻度の調整用パラメータ。デフォルトは1

private:
	static const char calculatorName[];
};


// 
// FUNCTION public
// ModInvertedOkapiTfScoreCalculator::firstStep -- tf / ( k + tf ) の計算
// 
// NOTES
// tf / ( k + tf ) の計算
//
// ARGUMENTS
// const ModSize tf
//		文書内出現頻度
// 
// RETURN
// tf / ( k + tf ) の結果
//
// EXCEPTIONS
// なし
//
inline ModInvertedDocumentScore
ModInvertedOkapiTfScoreCalculator::firstStep(const ModSize tf,
											 const DocumentID ID,
											 ModBoolean& exist) const
{
#ifdef DEBUG
	++countFirstStep;
#endif

	exist = ModTrue;
	return DocumentScore(tf / (k + tf));
}


// 
// FUNCTION public
// ModInvertedOkapiTfScoreCalculator::secondStep -- IDFの計算
// 
// NOTES
// IDFを計算する。ただし、このクラスでは、定数を使用する。
//
// ARGUMENTS
// const ModSize df
//		文書頻度
// const ModSize total
//		総文書数
// 
// RETURN
// 計算結果
//
// EXCEPTIONS
// なし
//
inline ModInvertedDocumentScore
ModInvertedOkapiTfScoreCalculator::secondStep(const ModSize df,
											  const ModSize total) const
{
#ifdef DEBUG
	++countSecondStep;
#endif

	return 1.0;
}

#endif // __ModInvertedOkapiTfScoreCalculator_h_

//
// Copyright (c) 1999, 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
