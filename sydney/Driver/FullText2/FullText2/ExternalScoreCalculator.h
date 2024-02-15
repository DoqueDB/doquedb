// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExternalScoreCalculator.h --
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

#ifndef __SYDNEY_FULLTEXT2_EXTERNALSCORECALCULATOR_H
#define __SYDNEY_FULLTEXT2_EXTERNALSCORECALCULATOR_H

#include "FullText2/Module.h"
#include "FullText2/ScoreCalculatorImpl.h"

#include "Os/Unicode.h"

#include "ModUnicodeChar.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::ExternalScoreCalculator -- スコア計算器
//
//	NOTES
//	外部スコア計算器をラップするクラス
//	外部スコア計算器のロードを行い、引数を受け渡す
//
class ExternalScoreCalculator : public ScoreCalculatorImpl
{
public:
	// コンストラクタ
	ExternalScoreCalculator(const ModUnicodeChar* param_);
	// デストラクタ
	virtual ~ExternalScoreCalculator();
	// コピーコンストラクタ
	ExternalScoreCalculator(const ExternalScoreCalculator& src_);
	
	// 必要な引数を得る
	void initialize(ModVector<Argument>& arg_);

	// TF項の前準備を行う
	void prepare(const ModVector<Argument>& arg_);
	
	// TF項を計算する
	double firstStep(const ModVector<Argument>& arg_);
	// IDF項を計算する
	double secondStep(const ModVector<Argument>& arg_);

	// コピーを取得する
	ScoreCalculator* copy();

private:
	// パラメータを設定する
	void setParameter(const ModUnicodeChar* param_);

	// 外部スコア計算器を解放する
	void releaseCalculator();

	// ライブラリ名
	Os::UnicodeString m_cLibName;
	// 外部スコア計算器
	ScoreCalculator* m_pCalculator;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_EXTERNALSCORECALCULATOR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
