// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TfIdfScoreCalculator.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/TfIdfScoreCalculator.h"

#include "Os/Math.h"

#include "Exception/NotSupported.h"

#include "ModUnicodeCharTrait.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	// この計算器で必要な引数

	ScoreCalculator::Argument::Type _argType[] =
	{
		ScoreCalculator::Argument::TermFrequency,			// 文書内頻度
		ScoreCalculator::Argument::DocumentFrequency,		// 文書頻度
		ScoreCalculator::Argument::TotalDocumentFrequency	// 総文書数
	};
}

//
//	FUNCTION public
//	FullText2::TfIdfScoreCalculator::TfIdfScoreCalculator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const ModUncodeChar* param_
//		パラメータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
TfIdfScoreCalculator::TfIdfScoreCalculator(const ModUnicodeChar* param_)
	: k1(0.0), k2(1.0), x(0.0), y(0)
{
	if (param_) setParameter(param_);
}

//
//	FUNCTION public
//	FullText2::TfIdfScoreCalculator::~TfIdfScoreCalculator
//		-- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
TfIdfScoreCalculator::~TfIdfScoreCalculator()
{
}

//
//	FUNCTION public
//	FullText2::TfIdfScoreCalculator::initialize
//		-- 必要な引数を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<Argument>& arg_
//		必要な引数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
TfIdfScoreCalculator::initialize(ModVector<Argument>& arg_)
{
	int s = sizeof(_argType) / sizeof(Argument::Type);
	arg_.erase(arg_.begin(), arg_.end());
	arg_.reserve(s);
	for (int i = 0; i < s; ++i)
		arg_.pushBack(Argument(_argType[i]));
}

//
//	FUNCTION public
//	FullText2::TfIdfScoreCalculator::firstStep
//		-- TF項を計算する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<FullText2::ScoreCalculator::Argument>& arg_
//		引数
//
//	RETURN
//	double
//		TF項
//
//	EXCEPTIONS
//
double
TfIdfScoreCalculator::firstStep(const ModVector<Argument>& arg_)
{
	ModVector<Argument>::ConstIterator i = arg_.begin();
	double tf = (*i).m_dblValue;	// 文書内頻度

	return (k1 + k2 * tf);
}

//
//	FUNCTION public
//	FullText2::TfIdfScoreCalculator::secondStep
//		-- IDF項を計算する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<FullText2::ScoreCalculator::Argument>& arg_
//		引数
//
//	RETURN
//	double
//		IDF項
//
//	EXCEPTIONS
//
double
TfIdfScoreCalculator::secondStep(const ModVector<Argument>& arg_)
{
	ModVector<Argument>::ConstIterator i = arg_.begin();
	++i;
	double df = (*i).m_dblValue;	// 文書頻度(ヒット数)
	++i;
	double N =(*i).m_dblValue;		// 総文書数

	return secondStep(y, x, df, N);
}

//
//	FUNCTION public
//	FullText2::TfIdfScoreCalculator::copy -- 自身のコピーを取得する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::ScoreCalculator*
//		コピー
//
//	EXCEPTIONS
//
ScoreCalculator*
TfIdfScoreCalculator::copy()
{
	return new TfIdfScoreCalculator(*this);
}

//
//	FUNCTION protected
//	FullText2::TfIdfScoreCalculator::secondStep -- IDF項を計算する
//
//	NOTES
//
//	ARGUMENTS
//	int y_
//		どの方式にするか
//	double x_
//		文書頻度調整用パラメータ
//	double df_
//		文書頻度(ヒット数)
//	double N_
//		総文書数
//
//	RERURN
//	double
//		IDF項
//
//	EXCEPTIONS
//
/*static*/
double
TfIdfScoreCalculator::secondStep(int y_, double x_,
								 double df_, double N_)
{
	if (y_ == 0) {
		// Robertson オリジナルの計算式
		return (x_ + Os::Math::log(N_ / df_)) / (x_ + Os::Math::log(N_));
	} else if (x_ == 0.0) {
		// 改良計算式
		return 1.0;
	} else {
		// 改良計算式
		return Os::Math::log(1.0 + x_ * N_ / df_) /
			Os::Math::log(1.0 + x_ * N_);
	}
}

//
//	FUNCTION private
//	FullText2::TfIdfScoreCalculator::setParameter
//		-- パラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar* param_
//		パラメータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
TfIdfScoreCalculator::setParameter(const ModUnicodeChar* param_)
{
	const ModUnicodeChar* p = param_;
	
	// パラメータは k1, k2, x, y の４つ

	if (*p != 0)
	{
		if (*p != ':')
			k1 = ModUnicodeCharTrait::toDouble(p);
		p = nextPos(p);
	}
	if (*p != 0)
	{
		if (*p != ':')
			k2 = ModUnicodeCharTrait::toDouble(p);
		p = nextPos(p);
	}
	if (*p != 0)
	{
		if (*p != ':')
			x = ModUnicodeCharTrait::toDouble(p);
		p = nextPos(p);
	}
	if (*p != 0)
	{
		if (*p != ':')
			y = ModUnicodeCharTrait::toInt(p);
		p = nextPos(p);
	}
	if (*p != 0)
		// パラメータが多いのでエラーとする
		_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
