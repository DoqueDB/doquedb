// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ScoreCalculatorImpl.h --
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

#ifndef __SYDNEY_FULLTEXT2_SCORECALCULATORIMPL_H
#define __SYDNEY_FULLTEXT2_SCORECALCULATORIMPL_H

#include "FullText2/Module.h"
#include "FullText2/ScoreCalculator.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::ScoreCalculatorImpl -- 内部実装用の基底クラス
//
//	NOTES
//
class ScoreCalculatorImpl : public ScoreCalculator
{
public:
	// コンストラクタ
	ScoreCalculatorImpl();
	// デストラクタ
	virtual ~ScoreCalculatorImpl();
	
	// 前準備を行う
	// TF項の計算部分のうち、文書単位で変化しない部分を事前に計算しておく
	// 注意: 空の関数
	virtual void prepare(const ModVector<Argument>& arg_);

	// IDF項を計算する
	// 注意: 常に 1.0 を返す
	virtual double secondStep(const ModVector<Argument>& arg_);

protected:
	// パラメータのポインターを次に進める
	const ModUnicodeChar* nextPos(const ModUnicodeChar* p_);
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_SCORECALCULATORIMPL_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
