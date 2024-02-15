// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExternalScoreCalculator.cpp --
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
#include "FullText2/ExternalScoreCalculator.h"

#include "Os/Library.h"
#include "Os/Unicode.h"

#include "Exception/FunctionNotFound.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace {

	// 外部スコア計算器を取得するための関数の関数名
	// この関数は ScoreCalculator.h で宣言されている
	
	Os::UnicodeString _cGetFunctionName("DBGetScoreCalculator");
	Os::UnicodeString _cReleaseFunctionName("DBReleaseScoreCalculator");
}

//
//	FUNCTION public
//	FullText2::ExternalScoreCalculator::ExternalScoreCalculator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//  const ModUnicodeChar* param_
//		パラメータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ExternalScoreCalculator::ExternalScoreCalculator(const ModUnicodeChar* param_)
	: m_pCalculator(0)
{
	setParameter(param_);
}

//
//	FUNCTION public
//	FullText2::ExternalScoreCalculator::~ExternalScoreCalculator
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
ExternalScoreCalculator::~ExternalScoreCalculator()
{
	releaseCalculator();
}

//
//	FUNCTION public
//	FullText2::ExternalScoreCalculator::ExternalScoreCalculator
//		-- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::ExternalScoreCalculator& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ExternalScoreCalculator::
ExternalScoreCalculator(const ExternalScoreCalculator& src_)
	: m_cLibName(src_.m_cLibName)
{
	m_pCalculator = src_.m_pCalculator->copy();
}

//
//	FUNCTION public
//	FullText2::ExternalScoreCalculator::initialize
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
ExternalScoreCalculator::initialize(ModVector<Argument>& arg_)
{
	m_pCalculator->initialize(arg_);
}

//
//	FUNCTION public
//	FullText2::ExternalScoreCalculator::prepare
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
ExternalScoreCalculator::prepare(const ModVector<Argument>& arg_)
{
	m_pCalculator->prepare(arg_);
}

//
//	FUNCTION public
//	FullText2::ExternalScoreCalculator::firstStep
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
ExternalScoreCalculator::firstStep(const ModVector<Argument>& arg_)
{
	return m_pCalculator->firstStep(arg_);
}

//
//	FUNCTION public
//	FullText2::ExternalScoreCalculator::secondStep
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
//		TF項
//
//	EXCEPTIONS
//
double
ExternalScoreCalculator::secondStep(const ModVector<Argument>& arg_)
{
	return m_pCalculator->secondStep(arg_);
}

//
//	FUNCTION public
//	FullText2::ExternalScoreCalculator::copy -- 自身のコピーを取得する
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
ExternalScoreCalculator::copy()
{
	return new ExternalScoreCalculator(*this);
}

//
//	FUNCTION private
//	FullText2::ExternalScoreCalculator::setParameter
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
ExternalScoreCalculator::setParameter(const ModUnicodeChar* param_)
{
	// パラメータは <DLL名>:<外部スコア計算器に渡すパラメータ>...

	// ライブラリ名を切り出す
	const ModUnicodeChar* p = param_;
	while (*p != 0)
	{
		if (*p == ':')
			break;
		++p;
	}
	m_cLibName.allocateCopy(param_, ModSize(p - param_));
	if (*p == ':') ++p;

	// ライブラリをロードする
	Os::Library::load(m_cLibName);

	// ライブラリを取得する
	ScoreCalculator* (*f)(const ModUnicodeChar*);
	f = (ScoreCalculator*(*)(const ModUnicodeChar*))
		Os::Library::getFunction(m_cLibName, _cGetFunctionName);

	// ScoreCalculator を取得する
	m_pCalculator = (*f)(p);
	if (m_pCalculator == 0)
	{
		// 見つからなかったものとみなす
		_TRMEISTER_THROW2(Exception::FunctionNotFound,
						  _cGetFunctionName, m_cLibName);
	}
}

//
//	FUNCTION private
//	FullText2::ExternalScoreCalculator::releaseCalculator
//		-- 取得した外部スコア計算器を解放する
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
void
ExternalScoreCalculator::releaseCalculator()
{
	if (m_pCalculator)
	{
		// すでに取得されているので、ライブラリはロード済み
		// よって、解放関数のエントリだけを取得する

		void (*f)(ScoreCalculator*);
		f = (void (*)(ScoreCalculator*))
			Os::Library::getFunction(m_cLibName, _cReleaseFunctionName);

		// 解放する

		(*f)(m_pCalculator);
		m_pCalculator = 0;
	}
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
