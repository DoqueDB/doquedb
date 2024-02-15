// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalizedOkapiTfIdfScoreCalculator.h --
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

#ifndef __SYDNEY_FULLTEXT2_NORMALIZEDOKAPITFIDFSCORECALCULATOR_H
#define __SYDNEY_FULLTEXT2_NORMALIZEDOKAPITFIDFSCORECALCULATOR_H

#include "FullText2/Module.h"
#include "FullText2/NormalizedOkapiTfScoreCalculator.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::NormalizedOkapiTfIdfScoreCalculator -- スコア計算器
//
//	NOTES
// 以下の計算式に基づいてスコアを計算する。
//  N: データーベースに登録されている全登録文書数
//  df: そのトークンを含む文書数
//  tf: その文書におけるそのトークンの出現頻度
//  ld: 文書長
//  L:  登録済み全文書の文書長平均値
//
// * First Step (TF 項)
//
//		                tf
//		-------------------------------------
//		k*((1 - lambda) + lambda*(ld/L)) + tf
//
//  k:  文書内頻度調整用パラメータ
//  lambda: 文書長による正規化の調整用パラメータ
//
// * Second Step (DF 項) -- OkapiTfIdf と同じ
//
class NormalizedOkapiTfIdfScoreCalculator
	: public NormalizedOkapiTfScoreCalculator
{
public:
	// コンストラクタ
	NormalizedOkapiTfIdfScoreCalculator(const ModUnicodeChar* param_ = 0);
	// デストラクタ
	virtual ~NormalizedOkapiTfIdfScoreCalculator();
	
	// 必要な引数を得る
	void initialize(ModVector<Argument>& arg_);

	// TF項を計算する
	double secondStep(const ModVector<Argument>& arg_);

	// コピーを取得する
	ScoreCalculator* copy();

protected:
	// パラメータ
	int y;
	double x;					// 文書頻度調整用パラメータ
	double a;					// 文書頻度調整用パラメータ
	double s;					// 文書頻度調整用パラメータ
	double q;					// 文書頻度調整用パラメータ

private:
	// パラメータを設定する
	void setParameter(const ModUnicodeChar* param_);
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_NORMALIZEDOKAPITFIDFSCORECALCULATOR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
