// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalizedOkapiTfScoreCalculator.cpp --
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
#include "FullText2/NormalizedOkapiTfScoreCalculator.h"

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
		ScoreCalculator::Argument::AverageDocumentLength	// 平均文書長
	};
}

//
//	FUNCTION public
//	FullText2::NormalizedOkapiTfScoreCalculator::
//			NormalizedOkapiTfScoreCalculator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
// 	const ModUnicodeChar* param_
//		パラメータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
NormalizedOkapiTfScoreCalculator::
NormalizedOkapiTfScoreCalculator(const ModUnicodeChar* param_)
	: k(1.0), lambda(0.25)
{
	if (param_) setParameter(param_);
}

//
//	FUNCTION public
//	FullText2::NormalizedOkapiTfScoreCalculator::
//			~NormalizedOkapiTfScoreCalculator
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
NormalizedOkapiTfScoreCalculator::~NormalizedOkapiTfScoreCalculator()
{
}

//
//	FUNCTION public
//	FullText2::NormalizedOkapiTfScoreCalculator::initialize
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
NormalizedOkapiTfScoreCalculator::initialize(ModVector<Argument>& arg_)
{
	int s = sizeof(_argType) / sizeof(Argument::Type);
	arg_.erase(arg_.begin(), arg_.end());
	arg_.reserve(s);
	for (int i = 0; i < s; ++i)
		arg_.pushBack(Argument(_argType[i]));
}

//
//	FUNCTION public
//	FullText2::NormalizedOkapiTfScoreCalculator::prepare
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
NormalizedOkapiTfScoreCalculator::prepare(const ModVector<Argument>& arg_)
{
	double L = arg_[2].m_dblValue;	// 平均文書長
	pre1 = k * (1.0 - lambda);
	pre2 = k * lambda / L;
}

//
//	FUNCTION public
//	FullText2::NormalizedOkapiTfScoreCalculator::firstStep
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
NormalizedOkapiTfScoreCalculator::firstStep(const ModVector<Argument>& arg_)
{
	ModVector<Argument>::ConstIterator i = arg_.begin();
	double tf = (*i).m_dblValue;	// 文書内頻度
	++i;
	double ld = (*i).m_dblValue;	// 文書長

	return tf / (pre1 + pre2 * ld + tf);
}

//
//	FUNCTION public
//	FullText2::NormalizedOkapiTfScoreCalculator::copy -- 自身のコピーを取得する
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
NormalizedOkapiTfScoreCalculator::copy()
{
	return new NormalizedOkapiTfScoreCalculator(*this);
}

//
//	FUNCTION private
//	FullText2::NormalizedOkapiTfScoreCalculator::setParameter
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
NormalizedOkapiTfScoreCalculator::setParameter(const ModUnicodeChar* param_)
{
	const ModUnicodeChar* p = param_;
	
	// パラメータは k:lambda の２つ

	if (*p != 0)
	{
		if (*p != ':')
			k = ModUnicodeCharTrait::toDouble(p);
		p = nextPos(p);
	}
	if (*p != 0)
	{
		if (*p != ':')
			lambda = ModUnicodeCharTrait::toDouble(p);
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
