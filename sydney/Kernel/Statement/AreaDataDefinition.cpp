// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaDataDefinition.cpp --
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

#include "Statement/AreaDataDefinition.h"
#include "Statement/Object.h"
#include "Statement/Type.h"
#include "Statement/Literal.h"
#include "Statement/Hint.h"
#if 0
#include "Analysis/AreaDataDefinition.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Path,
		f_Hint,
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::AreaDataDefinition::AreaDataDefinition -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Identifier* pName_
//		エリア名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
AreaDataDefinition::AreaDataDefinition(Literal* pPath_, Hint* pHint_)
	: Object(ObjectType::AreaDataDefinition, f__end_index)
{
	setAreaData(pPath_);
	setHint(pHint_);
}

//	FUNCTION public
//	Statement::AreaDataDefinition::~AreaDataDefinition -- デストラクタ
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
//
AreaDataDefinition::~AreaDataDefinition()
{
}

//
//	FUNCTION public
//	Statement::AreaDataDefinition::getAreaData
//		-- Area データを取得する
//
//	NOTES
//	Area データを取得する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Literal* pPath_
//		Area データ
//
//	EXCEPTIONS
//	なし
//
Literal*
AreaDataDefinition::getAreaData() const
{
	Literal* pResult = 0;
	Object* pObj = m_vecpElements[f_Path];
	if ( pObj && ObjectType::Literal == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Literal*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::AreaDataDefinition::setAreaData
//		-- Area データを設定する
//
//	NOTES
//	Area データを設定する
//
//	ARGUMENTS
//	Literal* pPath_
//		Area データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaDataDefinition::setAreaData(Literal* pPath_)
{
	m_vecpElements[f_Path] = pPath_;
}

#ifdef OBSOLETE
//	FUNCTION public
//	Statement::AreaDataDefinition::getHint
//		-- Hint を取得する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//
//	EXCEPTIONS
//	なし

Hint*
AreaDataDefinition::getHint() const
{
	Hint* pResult = 0;
	Object* pObj = m_vecpElements[f_Hint];
	if ( pObj && ObjectType::Hint == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Hint*, pObj);
	return pResult;
}
#endif

//
//	FUNCTION public
//	Statement::AreaDataDefinition::
//		-- Hint を設定する
//
//	NOTES
//	Hint を設定する
//
//	ARGUMENTS
//	Hint* pHint_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaDataDefinition::setHint(Hint* pHint_)
{
	m_vecpElements[f_Hint] = pHint_;
}

//
//	FUNCTION public
//	Statement::AreaDataDefinition::copy -- 自身をコピーする
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
AreaDataDefinition::copy() const
{
	return new AreaDataDefinition(*this);
}

#if 0
namespace
{
	Analysis::AreaDataDefinition _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
AreaDataDefinition::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
