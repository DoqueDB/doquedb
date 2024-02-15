// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TransactionMode.cpp --
// 
// Copyright (c) 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Statement";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Statement/Type.h"
#include "Statement/TransactionMode.h"
#include "Statement/IntegerValue.h"

#include "Common/Assert.h"
#if 0
#include "Analysis/TransactionMode.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_AccMode,
		f_IsoLevel,
		f_UsingSnapshot,
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::TransactionMode::TransactionMode -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
TransactionMode::TransactionMode(const IntegerValue* pcAccMode_,
								 const IntegerValue* pcIsoLevel_,
								 bool  bUsingSnapshot_)
	: Object(ObjectType::TransactionMode, f__end_index)
{
	setAccMode(pcAccMode_);
	setIsoLevel(pcIsoLevel_);
	setUsingSnapshot(bUsingSnapshot_);
}

//
//	FUNCTION public
//	Statement::TransactionMode::~TransactionMode -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
TransactionMode::~TransactionMode()
{
}

//
//	FUNCTION public
//	Statement::TransactionMode::
//		-- アクセスモードへのアクセサ
//
//	NOTES
//	アクセスモードのアクセサ
//
//	ARGUMENTS
//	const IntegerValue* pcAccMode_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
TransactionMode::setAccMode(const IntegerValue* pcAccMode_)
{
	m_vecpElements[f_AccMode] = const_cast<IntegerValue*>(pcAccMode_);
}

//	FUNCTION public
//	Statement::TransactionMode::
//		-- アクセスモードへのアクセサ
//
//	NOTES
//	アクセスモードのアクセサ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		AccessMode
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出

int
TransactionMode::getAccMode() const
{
	const IntegerValue* pcIntVal =
		_SYDNEY_DYNAMIC_CAST(const IntegerValue*, m_vecpElements[f_AccMode]);

	return (pcIntVal) ? pcIntVal->getValue() : AccUnknown;
}

//
//	FUNCTION public
//	Statement::TransactionMode::
//		-- アイソレーションレベルへのアクセサ
//
//	NOTES
//	アイソレーションレベルのアクセサ
//
//	ARGUMENTS
//	const IntegerValue* pcAccMode_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
TransactionMode::setIsoLevel(const IntegerValue* pcAccMode_)
{
	m_vecpElements[f_IsoLevel] = const_cast<IntegerValue*>(pcAccMode_);
}

//	FUNCTION public
//	Statement::TransactionMode::
//		-- アイソレーションレベルへのアクセサ
//
//	NOTES
//	アイソレーションレベルのアクセサ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		IsolationLevel
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出

int
TransactionMode::getIsoLevel() const
{
	const IntegerValue* pcIntVal =
		_SYDNEY_DYNAMIC_CAST(const IntegerValue*, m_vecpElements[f_IsoLevel]);

	return (pcIntVal) ? pcIntVal->getValue() : IsoUnknown;
}

//
//	FUNCTION public
//	Statement::TransactionMode::
//		-- 
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
TransactionMode::setUsingSnapshot(bool bUsingSnapshot_)
{
	int i = bUsingSnapshot_ ? 1 : 0;
	m_vecpElements[f_UsingSnapshot] = new IntegerValue(i);
}

//	FUNCTION public
//	Statement::TransactionMode::isUsingSnapshot
//		-- 版管理を使用するか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool	true  : 可能な限り版管理を使用する
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出

bool
TransactionMode::isUsingSnapshot() const
{
	const IntegerValue* pcIntVal = _SYDNEY_DYNAMIC_CAST(
		const IntegerValue*, m_vecpElements[f_UsingSnapshot]);
	; _SYDNEY_ASSERT(pcIntVal);

	return pcIntVal->getValue();
}

//
//	FUNCTION public
//	Statement::TransactionMode::copy -- 自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Statement::Object*
//
//	EXCEPTIONS
//	なし
//
Object*
TransactionMode::copy() const
{
	return new TransactionMode(*this);
}

#if 0
namespace
{
	Analysis::TransactionMode _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
TransactionMode::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
