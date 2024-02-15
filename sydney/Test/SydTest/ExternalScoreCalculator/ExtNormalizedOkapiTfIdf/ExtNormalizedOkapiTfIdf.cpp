//
// FullText2用の外部スコア計算器サンプルソースコード
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

#if defined(WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#include <stdlib.h>
#include <math.h>
#define SYD_EXTERNAL_CALCULATOR
#include "FullText2/ScoreCalculator.h"

#include "ModUnicodeChar.h"
#include "ModUnicodeCharTrait.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//////////////////////////////
	// 外部スコア計算器 DLL 本体
	//

	class ExtNormalizedOkapiTfIdf : public ScoreCalculator
	{
	public:
		// コンストラクタ
		ExtNormalizedOkapiTfIdf(const ModUnicodeChar* param_ = 0);
		// デストラクタ
		virtual ~ExtNormalizedOkapiTfIdf();
	
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

		// IDF項を計算する
		double secondStep(int y_,
						  double x_, double a_,
						  double s_, double q_,
						  double df_, double N_);
		// パラメータのポインタを次に進める
		const ModUnicodeChar* nextPos(const ModUnicodeChar* p_);
		
		// パラメータ
		int y;
		double x;
		double a;
		double s;
		double q;
		double k;
		double lambda;
	
		// 予備計算済みのパラメータ
		double pre1;
		double pre2;

	};
	

	// この計算器で必要な引数

	ScoreCalculator::Argument::Type _argType[] =
	{
		//【注意】
		// NormalizedOkapiTfScoreCalculator と上の３つは同じ順番であること
		
		ScoreCalculator::Argument::TermFrequency,			// 文書内頻度
		ScoreCalculator::Argument::DocumentLength,			// 文書長
		ScoreCalculator::Argument::AverageDocumentLength,	// 平均文書長

		ScoreCalculator::Argument::DocumentFrequency,		// 文書頻度
		ScoreCalculator::Argument::TotalDocumentFrequency	// 総文書数
	};
}

//
//	FUNCTION public
//	FullText2::ExtNormalizedOkapiTfIdf::
//			ExtNormalizedOkapiTfIdf
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
ExtNormalizedOkapiTfIdf::
ExtNormalizedOkapiTfIdf(const ModUnicodeChar* param_)
	: y(1), x(0.2), a(0.0), s(0.0), q(0.0), k(1.0), lambda(0.25)
{
	if (param_) setParameter(param_);
}

//
//	FUNCTION public
//	FullText2::ExtNormalizedOkapiTfIdf::
//			~ExtNormalizedOkapiTfIdf
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
ExtNormalizedOkapiTfIdf::~ExtNormalizedOkapiTfIdf()
{
}

//
//	FUNCTION public
//	FullText2::ExtNormalizedOkapiTfIdf::initialize
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
ExtNormalizedOkapiTfIdf::initialize(ModVector<Argument>& arg_)
{
	int s = sizeof(_argType) / sizeof(Argument::Type);
	arg_.erase(arg_.begin(), arg_.end());
	arg_.reserve(s);
	for (int i = 0; i < s; ++i)
		arg_.pushBack(Argument(_argType[i]));
}

//
//	FUNCTION public
//	FullText2::ExtNormalizedOkapiTfIdf::prepare
//		-- TF項の計算のうち、文書単位に変化しない部分を事前に計算しておく
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<FullText2::ScoreCalculator::Argument>& arg_
//		引数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ExtNormalizedOkapiTfIdf::prepare(const ModVector<Argument>& arg_)
{
	double L = arg_[2].m_dblValue;	// 平均文書長
	pre1 = k * (1.0 - lambda);
	pre2 = k * lambda / L;
}

//
//	FUNCTION public
//	FullText2::ExtNormalizedOkapiTfIdf::firstStep
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
ExtNormalizedOkapiTfIdf::firstStep(const ModVector<Argument>& arg_)
{
	ModVector<Argument>::ConstIterator i = arg_.begin();
	double tf = (*i).m_dblValue;	// 文書内頻度
	++i;
	double ld = (*i).m_dblValue;	// 文書長

	return tf / (pre1 + pre2 * ld + tf);
}

//
//	FUNCTION public
//	FullText2::ExtNormalizedOkapiTfIdf::secondStep
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
ExtNormalizedOkapiTfIdf::secondStep(const ModVector<Argument>& arg_)
{
	ModVector<Argument>::ConstIterator i = arg_.begin();
	i += 3;
	double df = (*i).m_dblValue;	// 文書頻度(ヒット数)
	++i;
	double N = (*i).m_dblValue;		// 総文書数

	return secondStep(y, x, a, s, q, df, N);
}

//
//	FUNCTION public
//	FullText2::ExtNormalizedOkapiTfIdf::copy
//		-- 自身のコピーを取得する
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
ExtNormalizedOkapiTfIdf::copy()
{
	return new ExtNormalizedOkapiTfIdf(*this);
}

//
//	FUNCTION private
//	FullText2::ExtNormalizedOkapiTfIdf::setParameter
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
ExtNormalizedOkapiTfIdf::setParameter(const ModUnicodeChar* param_)
{
	const ModUnicodeChar* p = param_;

	// パラメータは k, x, lambda, y, q, a, s の 7 つ

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
			lambda = ModUnicodeCharTrait::toDouble(p);
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
		throw 0;
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
double
ExtNormalizedOkapiTfIdf::secondStep(int y_,
									double x_, double a_,
									double s_, double q_,
									double df_, double N_)
{
	if (y_ == 1) {
		// Ogawa 式
		if (x_ == 0.0) {
			return 1.0;
		}
		return log(1.0 + x_ * N_ / df_) /
			log(1.0 + x_ * N_);
	} else if (y_ == 4) {
		// Ogawa 式
		return log(1.0 + x_ / (1.0 - x_) * N_ / df_);
	} if (y_ == 6) {
		// Ogawa2 式
		return log((x_ * N_ + df_) / (q_ * N_ + df_)) /
			log((x_ * N_ + 1.0) / (q_ * N_ + 1.0));
	} else if (y_ == 7) {
		// Ogawa2 式
		return log((x_ * N_ + df_) / (q_ * N_ + df_));
	} else if (y_ == 0) {
		// Robertson 式
		return (x_ + log(N_ / df_)) / (x_ + log(N_));
	} else if (y_ == 3) {
		// Robertson 式
		return log(x_ / (1.0 - x_) * N_ / df_);
	} else if (y_ == 2) {
		// Haprper/Croft 式
		return (x_ + log((N_ - df_) / df_)) /
			(x_ + log(N_ - 1.0));
	} else if (y_ == 5) {
		// Haprper/Croft 式
		return log(x_ / (1.0 - x_) * (N_ - df_) / df_);
	} else if (y_ == 8) {
		// s == 1 の場合
		double T = df_ / (N_ - df_);
		if (x_ == 1.0) {
			return 0;
		}
		return log(((x_ + (1.0 + a_) * T) / (1.0 - x_)) /
				   ((q_ + T) / (1.0 - q_)));
	} else if (y_ == 9) {
		// まともに計算する
		double tmp1 = N_ / df_;
		double tmp2 = pow(tmp1, s_);
		return log((x_ + (1.0 + a_) / (1.0 - tmp2)) / (1.0 - x_) /
				   (q_ + 1.0 / (tmp1 - 1)) * (1.0 - q_));
	} else {
		throw 0;
	}
}

//
//	FUNCTION private
//	FullText2::ExtNormalizedOkapiTdIdf::nextPos
//		-- パラメータのポインターを次に進める
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar* p_
//		パラメータ文字列へのポインタ
//
//	RETURN
//	const ModUnicodeChar*
//	   	次のパラメータ文字列へのポインタ
//
//	EXCEPTIONS
//
const ModUnicodeChar*
ExtNormalizedOkapiTfIdf::nextPos(const ModUnicodeChar* p_)
{
	if (p_ == 0)
		return p_;
	
	// ScoreCalculatorに渡されるパラメータは、数値がコロン(:)で
	// 区切られている文字列である。
	
	while (*p_ != 0)
	{
		if (*p_ == ':')
		{
			++p_;
			break;
		}

		++p_;
	}

	return p_;
}

//
//	FUNCTION global
//	DBGetScoreCalculator -- 外部スコア計算器を得る
//
//	NOTES
//	外部スコア計算器を得るエントリ関数
//	外部スコア計算器を提供する場合は、このエントリ関数を実装する必要がある
//
//	ARGUMENTS
// 	const ModUnicodeChar* parameter_
//		パラメータ
//
//	RETURN
//	FullText2::ScoreCalculator*
//		スコア計算器へのポインタ
//
//	EXCEPTIONS
//
ScoreCalculator*
DBGetScoreCalculator(const ModUnicodeChar* parameter_)
{
	return new ExtNormalizedOkapiTfIdf(parameter_);
}

//
//	FUNCTION global
//	DBReleaseScoreCalculator -- 外部スコア計算器を解放する
//
//	NOTES
//	外部スコア計算器を提供する場合には、このエントリ関数を実装する必要がある
//
//	ARGUMENTS
//	FullText2::ScoreCalculator* pCalculator_
//		解放する外部スコア計算器
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DBReleaseScoreCalculator(ScoreCalculator* pCalculator_)
{
	delete pCalculator_;
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
