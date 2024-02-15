// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TransactAccMode.cpp --
// 
// Copyright (c) 2000, 2002, 2003, 2023 Ricoh Company, Ltd.
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
#include "Statement/TransactAccMode.h"
#include "Statement/IntegerValue.h"
#include "Statement/TransactionMode.h"
#if 0
#include "Analysis/TransactAccMode.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_AccMode,
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::TransactAccMode::TransactAccMode -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const IntegerValue* pcAccMode_
//		アクセスモード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
TransactAccMode::TransactAccMode(const IntegerValue* pcAccMode_)
	: Object(ObjectType::TransactAccMode, f__end_index)
{
	setAccMode(pcAccMode_);
}

//
//	FUNCTION public
//	Statement::TransactAccMode::~TransactAccMode -- デストラクタ
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
TransactAccMode::~TransactAccMode()
{
}

//
//	FUNCTION public
//	Statement::TransactAccMode::
//		-- アクセスモード
//
//	NOTES
//	アクセスモードへのアクセサ
//
//	ARGUMENTS
//	const IntegerValue* pcAccMode_
//
//	RETURN
//
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
TransactAccMode::setAccMode(const IntegerValue* pcAccMode_)
{
	m_vecpElements[f_AccMode] = const_cast<IntegerValue*>(pcAccMode_);
}

//
//	FUNCTION public
//	Statement::TransactAccMode::
//		-- アクセスモード
//
//	NOTES
//	アクセスモードへのアクセサ
//
//	ARGUMENTS
//
//
//	RETURN
//	Statement::IntegerValue*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
IntegerValue*
TransactAccMode::getAccMode() const
{
	IntegerValue* pResult = 0;
	Object* pObj = m_vecpElements[f_AccMode];
	if ( pObj && ObjectType::IntegerValue == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pObj);
	return pResult;
}

#if 0
namespace
{
	Analysis::TransactAccMode _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
TransactAccMode::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
