// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IsolationLevel.cpp --
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
#include "Statement/IsolationLevel.h"
#include "Statement/IntegerValue.h"
#if 0
#include "Analysis/IsolationLevel.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_IsoLevel,
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::IsolationLevel::IsolationLevel -- コンストラクタ
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
IsolationLevel::IsolationLevel(const IntegerValue* pcLevel_)
	: Object(ObjectType::IsolationLevel, f__end_index)
{
	setIsoLevel(pcLevel_);
}

//
//	FUNCTION public
//	Statement::IsolationLevel::~IsolationLevel -- デストラクタ
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
IsolationLevel::~IsolationLevel()
{
}

//
//	FUNCTION public
//	Statement::IsolationLevel::
//		-- アイソレーションレベルへのアクセサ
//
//	NOTES
//	アイソレーションレベルへのアクセサ
//
//	ARGUMENTS
//	アイソレーションレベル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
IsolationLevel::setIsoLevel(const IntegerValue* pcLevel_)
{
	m_vecpElements[f_IsoLevel] = const_cast<IntegerValue*>(pcLevel_);
}

//
//	FUNCTION public
//	Statement::IsolationLevel::
//		-- アイソレーションレベルへのアクセサ
//
//	NOTES
//	アイソレーションレベルへのアクセサ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Statement::IntegerValue*
//		アイソレーションレベル
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
IntegerValue*
IsolationLevel::getIsoLevel() const
{
	IntegerValue* pResult = 0;
	Object* pObj = m_vecpElements[f_IsoLevel];
	if ( pObj && ObjectType::IntegerValue == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pObj);
	return pResult;
}

#if 0
namespace
{
	Analysis::IsolationLevel _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
IsolationLevel::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
