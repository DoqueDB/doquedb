// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalizedTfIdfScoreCalculator.cpp --
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
#include "FullText2/NormalizedTfIdfScoreCalculator.h"

#include "Exception/NotSupported.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	// この計算器で必要な引数

	ScoreCalculator::Argument::Type _argType[] =
	{
		ScoreCalculator::Argument::TermFrequency,			// 文書内頻度
		ScoreCalculator::Argument::DocumentLength,			// 文書長
		ScoreCalculator::Argument::AverageDocumentLength,	// 平均文書長
		
		ScoreCalculator::Argument::DocumentFrequency,		// 文書頻度
		ScoreCalculator::Argument::TotalDocumentFrequency	// 総文書数
	};
}

//
//	FUNCTION public
//	FullText2::NormalizedTfIdfScoreCalculator::NormalizedTfIdfScoreCalculator
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
NormalizedTfIdfScoreCalculator::
NormalizedTfIdfScoreCalculator(const ModUnicodeChar* param_)
	: lambda(0.25)
{
	if (param_) setParameter(param_);
}

//
//	FUNCTION public
//	FullText2::NormalizedTfIdfScoreCalculator::~NormalizedTfIdfScoreCalculator
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
NormalizedTfIdfScoreCalculator::~NormalizedTfIdfScoreCalculator()
{
}

//
//	FUNCTION public
//	FullText2::NormalizedTfIdfScoreCalculator::initialize
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
NormalizedTfIdfScoreCalculator::initialize(ModVector<Argument>& arg_)
{
	int s = sizeof(_argType) / sizeof(Argument::Type);
	arg_.erase(arg_.begin(), arg_.end());
	arg_.reserve(s);
	for (int i = 0; i < s; ++i)
		arg_.pushBack(Argument(_argType[i]));
}

//
//	FUNCTION public
//	FullText2::NormalizedTfIdfScoreCalculator::firstStep
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
NormalizedTfIdfScoreCalculator::firstStep(const ModVector<Argument>& arg_)
{
	ModVector<Argument>::ConstIterator i = arg_.begin();
	double tf = (*i).m_dblValue;	// 文書内頻度
	++i;
	double ld = (*i).m_dblValue;	// 文書長
	++i;
	double L = (*i).m_dblValue;		// 平均文書長

	return (k1 + k2 * tf / ((1 - lambda) * L + lambda * ld));
}

//
//	FUNCTION public
//	FullText2::NormalizedTfIdfScoreCalculator::secondStep
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
NormalizedTfIdfScoreCalculator::secondStep(const ModVector<Argument>& arg_)
{
	ModVector<Argument>::ConstIterator i = arg_.begin();
	i += 3;
	double df = (*i).m_dblValue;	// 文書頻度(ヒット数)
	++i;
	double N =(*i).m_dblValue;		// 総文書数

	return TfIdfScoreCalculator::secondStep(y, x, df, N);
}

//
//	FUNCTION public
//	FullText2::NormalizedTfIdfScoreCalculator::copy -- 自身のコピーを取得する
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
NormalizedTfIdfScoreCalculator::copy()
{
	return new NormalizedTfIdfScoreCalculator(*this);
}

//
//	FUNCTION private
//	FullText2::NormalizedTfIdfScoreCalculator::setParameter
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
NormalizedTfIdfScoreCalculator::setParameter(const ModUnicodeChar* param_)
{
	const ModUnicodeChar* p = param_;
	
	// パラメータは k1, k2, x, lambda, y の５つ

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
		// パラメータが多いのでエラーとする
		_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
