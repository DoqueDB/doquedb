// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaOption.cpp --
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
#include "Statement/AreaOption.h"
#include "Statement/Identifier.h"
#include "Statement/Hint.h"
#if 0
#include "Analysis/AreaOption.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_default		= AreaOption::Default,
		f_table			= AreaOption::Table,
		f_heap			= AreaOption::Heap,
		f_index			= AreaOption::Index,
		f_fulltext		= AreaOption::FullText,
		f_logicallog	= AreaOption::LogicalLog,
		f_physicallog	= AreaOption::PhysicalLog,
		f_hint			= AreaOption::HintArea,
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::AreaOption::AreaOption -- コンストラクタ
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
AreaOption::AreaOption(Identifier* Default_, Identifier* Table_,
					   Identifier* Heap_, Identifier* Index_,
					   Identifier* FullText_, Identifier* LogicalLog_,
					   Identifier* PhysicalLog_, Hint* HintArea_)
	: Object(ObjectType::AreaOption, f__end_index)
{
	setAreaName(Default, Default_);
	setAreaName(Table, Table_);
	setAreaName(Heap, Heap_);
	setAreaName(Index, Index_);
	setAreaName(FullText, FullText_);
	setAreaName(LogicalLog, LogicalLog_);
	setAreaName(PhysicalLog, PhysicalLog_);
	setHintArea(HintArea_);
}

//
//	FUNCTION public
//	Statement::AreaOption::~AreaOption -- デストラクタ
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
AreaOption::~AreaOption()
{
}

//
//	FUNCTION public
//	Statement::AreaOption::getAreaName -- エリア名を取得する
//
//	NOTES
//	エリア名を取得する
//
//	ARGUMENTS
//	const AreaType& eAreaType_
//		エリア種別
//
//	RETURN
//	Statement::Identifier*
//		エリア名オブジェクト
//
//	EXCEPTIONS
//	なし
//
Identifier*
AreaOption::getAreaName(const AreaType& eAreaType_) const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[eAreaType_];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::AreaOption::setAreaName
//		-- エリア名を設定する
//
//	NOTES
//	エリア名を設定する
//
//	ARGUMENTS
//	const AreaType& eAreaType_
//		エリア種別
//
//
//	RETURN
//	Statement::Identifier*
//		エリア名オブジェクト
//
//	EXCEPTIONS
//	なし
//
void
AreaOption::setAreaName(const AreaType& eAreaType_, Identifier* pId_)
{
	m_vecpElements[eAreaType_] = pId_;
	return;
}

#ifdef OBSOLETE
//	FUNCTION public
//	Statement::AreaOption::getHintArea -- Hint を得る
//
//	NOTES
//
//	ARGUMENTS
//	Hint* HintArea_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

Hint*
AreaOption::getHintArea() const
{
	Hint* pResult = 0;
	Object* pObj = m_vecpElements[f_hint];
	if ( pObj && ObjectType::Hint == pObj->getType() )
		pResult =_SYDNEY_DYNAMIC_CAST(Hint*, pObj);
	return pResult;
}
#endif

//
//	FUNCTION public
//	Statement::AreaOption::setHintArea -- Hint を設定する
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
AreaOption::setHintArea(Hint* pHint_)
{
	m_vecpElements[f_hint] = pHint_;
}

//
//	FUNCTION public
//	Statement::AreaOption::copy -- 自身をコピーする
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
AreaOption::copy() const
{
	return new AreaOption(*this);
}

#if 0
namespace
{
	Analysis::AreaOption _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
AreaOption::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
