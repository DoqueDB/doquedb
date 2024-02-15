// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OkapiTfIdfScoreCalculator.cpp --
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
#include "SyInclude.h"
#include "FullText2/OkapiTfIdfScoreCalculator.h"

#include "Os/Math.h"

#include "Exception/NotSupported.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	// この計算器で必要な引数

	ScoreCalculator::Argument::Type _argType[] =
	{
		//【注意】
		// OkapiTfScoreCalculator と上の１つは同じ順番であること
		
		ScoreCalculator::Argument::TermFrequency,			// 文書内頻度
		
		ScoreCalculator::Argument::DocumentFrequency,		// 文書頻度
		ScoreCalculator::Argument::TotalDocumentFrequency	// 総文書数
	};
}

//
//	FUNCTION public
//	FullText2::OkapiTfIdfScoreCalculator::OkapiTfIdfScoreCalculator
//		-- コンストラクタ
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
OkapiTfIdfScoreCalculator::
OkapiTfIdfScoreCalculator(const ModUnicodeChar* param_)
	: y(1), x(0.2), a(0.0), s(0.0), q(0.0)
{
	if (param_) setParameter(param_);
}

//
//	FUNCTION public
//	FullText2::OkapiTfIdfScoreCalculator::~OkapiTfIdfScoreCalculator
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
OkapiTfIdfScoreCalculator::~OkapiTfIdfScoreCalculator()
{
}

//
//	FUNCTION public
//	FullText2::OkapiTfIdfScoreCalculator::initialize
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
OkapiTfIdfScoreCalculator::initialize(ModVector<Argument>& arg_)
{
	int s = sizeof(_argType) / sizeof(Argument::Type);
	arg_.erase(arg_.begin(), arg_.end());
	arg_.reserve(s);
	for (int i = 0; i < s; ++i)
		arg_.pushBack(Argument(_argType[i]));
}

//
//	FUNCTION public
//	FullText2::OkapiTfIdfScoreCalculator::secondStep
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
OkapiTfIdfScoreCalculator::secondStep(const ModVector<Argument>& arg_)
{
	ModVector<Argument>::ConstIterator i = arg_.begin();
	++i;
	double df = (*i).m_dblValue;	// 文書頻度(ヒット数)
	++i;
	double N = (*i).m_dblValue;		// 総文書数

	return secondStep(y, x, a, s, q, df, N);
}

//
//	FUNCTION public
//	FullText2::OkapiTfIdfScoreCalculator::copy -- 自身のコピーを取得する
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
OkapiTfIdfScoreCalculator::copy()
{
	return new OkapiTfIdfScoreCalculator(*this);
}

//
//	FUNCTION public
//	FullText2::OkapiTfIdfScoreCalculator::secondStep
//		-- IDF項を計算する
//
//	NOTES
//
//	ARGUMENTS
//	int y_
//		方式の選択
//	double x_, a_, s_, q_
//		文書頻度調整用パラメータ
//	double df_
//		文書頻度(ヒット数)
//	double N_
//		総文書数
//
//	RETURN
//	double
//		IDF項
//
//	EXCEPTIONS
//
/*static*/
double
OkapiTfIdfScoreCalculator::secondStep(int y_,
									  double x_, double a_,
									  double s_, double q_,
									  double df_, double N_)
{
	if (y_ == 1) {
		// Ogawa 式
		if (x_ == 0.0) {
			return 1.0;
		}
		return Os::Math::log(1.0 + x_ * N_ / df_) /
			Os::Math::log(1.0 + x_ * N_);
	} else if (y_ == 4) {
		// Ogawa 式
		return Os::Math::log(1.0 + x_ / (1.0 - x_) * N_ / df_);
	} if (y_ == 6) {
		// Ogawa2 式
		return Os::Math::log((x_ * N_ + df_) / (q_ * N_ + df_)) /
			Os::Math::log((x_ * N_ + 1.0) / (q_ * N_ + 1.0));
	} else if (y_ == 7) {
		// Ogawa2 式
		return Os::Math::log((x_ * N_ + df_) / (q_ * N_ + df_));
	} else if (y_ == 0) {
		// Robertson 式
		return (x_ + Os::Math::log(N_ / df_)) / (x_ + Os::Math::log(N_));
	} else if (y_ == 3) {
		// Robertson 式
		return Os::Math::log(x_ / (1.0 - x_) * N_ / df_);
	} else if (y_ == 2) {
		// Haprper/Croft 式
		return (x_ + Os::Math::log((N_ - df_) / df_)) /
			(x_ + Os::Math::log(N_ - 1.0));
	} else if (y_ == 5) {
		// Haprper/Croft 式
		return Os::Math::log(x_ / (1.0 - x_) * (N_ - df_) / df_);
	} else if (y_ == 8) {
		// s == 1 の場合
		double T = df_ / (N_ - df_);
		if (x_ == 1.0) {
			return 0;
		}
		return Os::Math::log(((x_ + (1.0 + a_) * T) / (1.0 - x_)) /
							 ((q_ + T) / (1.0 - q_)));
	} else if (y_ == 9) {
		// まともに計算する
		double tmp1 = N_ / df_;
		double tmp2 = Os::Math::pow(tmp1, s_);
		return Os::Math::log((x_ + (1.0 + a_) / (1.0 - tmp2)) / (1.0 - x_) /
							 (q_ + 1.0 / (tmp1 - 1)) * (1.0 - q_));
	} else {
		_TRMEISTER_THROW0(Exception::NotSupported);
	}
}

//
//	FUNCTION pivate
//	FullText2::OkapiTfIdfScoreCalculator::setParameter
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
OkapiTfIdfScoreCalculator::setParameter(const ModUnicodeChar* param_)
{
	const ModUnicodeChar* p = param_;
	
	// パラメータは k, x, y, q, a, s の６つ

	if (*p != 0)
	{
		if (*p != ':')
			k = ModUnicodeCharTrait::toDouble(p);
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
	{
		if (*p != ':')
			q = ModUnicodeCharTrait::toDouble(p);
		p = nextPos(p);
	}
	if (*p != 0)
	{
		if (*p != ':')
			a = ModUnicodeCharTrait::toDouble(p);
		p = nextPos(p);
	}
	if (*p != 0)
	{
		if (*p != ':')
			s = ModUnicodeCharTrait::toDouble(p);
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
