// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OkapiTfScoreCalculator.h --
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

#ifndef __SYDNEY_FULLTEXT2_OKAPITFSCORECALCULATOR_H
#define __SYDNEY_FULLTEXT2_OKAPITFSCORECALCULATOR_H

#include "FullText2/Module.h"
#include "FullText2/ScoreCalculatorImpl.h"

#include "ModUnicodeChar.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::OkapiTfScoreCalculator -- スコア計算器
//
//	NOTES
//	スコア計算式 :
//		tf / (k + tf)
//
//		tf	 	文書内頻度
//		k		文書内頻度調整用パラメータ
//
class OkapiTfScoreCalculator : public ScoreCalculatorImpl
{
public:
	// コンストラクタ
	OkapiTfScoreCalculator(const ModUnicodeChar* param_ = 0);
	// デストラクタ
	virtual ~OkapiTfScoreCalculator();
	
	// 必要な引数を得る
	void initialize(ModVector<Argument>& arg_);

	// TF項を計算する
	double firstStep(const ModVector<Argument>& arg_);

	// コピーを取得する
	ScoreCalculator* copy();

protected:
	// パラメータ
	double	k;

private:
	// パラメータを設定する
	void setParameter(const ModUnicodeChar* param_);
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_OKAPITFSCORECALCULATOR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
