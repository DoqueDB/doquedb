// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalizedOkapiTfIdfScoreCalculator.cpp --
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
#include "FullText2/NormalizedOkapiTfIdfScoreCalculator.h"
#include "FullText2/OkapiTfIdfScoreCalculator.h"

#include "Exception/NotSupported.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
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
//	FullText2::NormalizedOkapiTfIdfScoreCalculator::
//			NormalizedOkapiTfIdfScoreCalculator
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
NormalizedOkapiTfIdfScoreCalculator::
NormalizedOkapiTfIdfScoreCalculator(const ModUnicodeChar* param_)
	: y(1), x(0.2), a(0.0), s(0.0), q(0.0)
{
	if (param_) setParameter(param_);
}

//
//	FUNCTION public
//	FullText2::NormalizedOkapiTfIdfScoreCalculator::
//			~NormalizedOkapiTfIdfScoreCalculator
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
NormalizedOkapiTfIdfScoreCalculator::~NormalizedOkapiTfIdfScoreCalculator()
{
}

//
//	FUNCTION public
//	FullText2::NormalizedOkapiTfIdfScoreCalculator::initialize
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
NormalizedOkapiTfIdfScoreCalculator::initialize(ModVector<Argument>& arg_)
{
	int s = sizeof(_argType) / sizeof(Argument::Type);
	arg_.erase(arg_.begin(), arg_.end());
	arg_.reserve(s);
	for (int i = 0; i < s; ++i)
		arg_.pushBack(Argument(_argType[i]));
}

//
//	FUNCTION public
//	FullText2::NormalizedOkapiTfIdfScoreCalculator::secondStep
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
NormalizedOkapiTfIdfScoreCalculator::secondStep(const ModVector<Argument>& arg_)
{
	ModVector<Argument>::ConstIterator i = arg_.begin();
	i += 3;
	double df = (*i).m_dblValue;	// 文書頻度(ヒット数)
	++i;
	double N = (*i).m_dblValue;		// 総文書数

	return OkapiTfIdfScoreCalculator::secondStep(y, x, a, s, q, df, N);
}

//
//	FUNCTION public
//	FullText2::NormalizedOkapiTfIdfScoreCalculator::copy
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
NormalizedOkapiTfIdfScoreCalculator::copy()
{
	return new NormalizedOkapiTfIdfScoreCalculator(*this);
}

//
//	FUNCTION private
//	FullText2::NormalizedOkapiTfIdfScoreCalculator::setParameter
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
NormalizedOkapiTfIdfScoreCalculator::setParameter(const ModUnicodeChar* param_)
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
		_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
