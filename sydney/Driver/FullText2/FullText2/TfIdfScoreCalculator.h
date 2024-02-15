// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TfIdfScoreCalculator.h --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_TFIDFSCORECALCULATOR_H
#define __SYDNEY_FULLTEXT2_TFIDFSCORECALCULATOR_H

#include "FullText2/Module.h"
#include "FullText2/ScoreCalculatorImpl.h"

#include "ModUnicodeChar.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class NormalizedTfIdfScoreCalculator;

//
//	CLASS
//	FullText2::TfIdfScoreCalculator -- スコア計算器
//
//	NOTES
// 以下の計算式に基づいてスコアを計算する。
//	tf: その文書におけるそのトークンの出現頻度
//	N: データーベースに登録されている全登録文書数
//	df: そのトークンを含む文書数
//
// * First Step (TF 項)
//
//		k1 + k2 * tf
//
//	k1: 文書内頻度調整用パラメータ1
//	k2: 文書内頻度調整用パラメータ2
//
// * Second Step (DF項)
//
// y == 0
//
//			 	 N
//		x + log(---)
//				 df
//		---------------
//		   x + log(N)
//
//	x:	文書頻度調整用パラメータ 
//
// y != 0
//
//	 				 N
//		log(1 + x * ---)
//					 df
//		---------------
//		log(1 + x * N)
//
//	x:	文書頻度調整用パラメータ 
//
class TfIdfScoreCalculator : public ScoreCalculatorImpl
{
	friend class NormalizedTfIdfScoreCalculator;
	
public:
	// コンストラクタ
	TfIdfScoreCalculator(const ModUnicodeChar* param_ = 0);
	// デストラクタ
	virtual ~TfIdfScoreCalculator();
	
	// 必要な引数を得る
	void initialize(ModVector<Argument>& arg_);

	// TF項を計算する
	double firstStep(const ModVector<Argument>& arg_);
	// IDF項を計算する
	double secondStep(const ModVector<Argument>& arg_);

	// コピーを取得する
	ScoreCalculator* copy();

protected:
	// IDF項を計算する
	static double secondStep(int y,		// どの方式にするか
							 double x,
							 double df, double N);
	
	// パラメータ
	double k1;					// 文書内頻度調整用パラメータ1。デフォルトは0。
	double k2;					// 文書内頻度調整用パラメータ2。デフォルトは1。
	double x;					// 文書頻度調整用パラメータ。デフォルト0。
	int y;						// きりかえ用

private:
	// パラメータを設定する
	void setParameter(const ModUnicodeChar* param_);
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_TFIDFSCORECALCULATOR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
