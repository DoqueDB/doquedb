// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaDefinition.cpp --
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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
#include "Statement/AreaDefinition.h"
#include "Statement/Identifier.h"
#include "Statement/AreaElementList.h"
#if 0
#include "Analysis/AreaDefinition.h"
#endif

_SYDNEY_USING

using namespace Statement;

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Name,							// Area 名
		f_Elements,						// ElementList
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::AreaDefinition::AreaDefinition -- コンストラクタ(2)
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
AreaDefinition::AreaDefinition(Identifier* pName_,
							   AreaElementList* pAreaElements_)
	: Object(ObjectType::AreaDefinition, f__end_index, Object::Reorganize)
{
	setName(pName_);
	setElements(pAreaElements_);
}

//
//	FUNCTION public
//	Statement::AreaDefinition::~AreaDefinition -- デストラクタ
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
AreaDefinition::~AreaDefinition()
{
}

//
//	FUNCTION public
//	Statement::AreaDefinition::setName -- Area 名を取得する
//
//	NOTES
//	Area 名を設定する
//
//	ARGUMENTS
//	Identifier* pName_
//		Area 名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
Identifier*
AreaDefinition::getName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Name];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::AreaDefinition::setName -- Area 名を設定する
//
//	NOTES
//	Area 名を設定する
//
//	ARGUMENTS
//	Identifier* pName_
//		Area 名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
AreaDefinition::setName(Identifier* pName_)
{
	m_vecpElements[f_Name] = pName_;
	return;
}

//
//	FUNCTION public
//	Statement::AreaDefinition::getElements -- Elemets を取得する
//
//	NOTES
//	Elemets を取得する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	AreaElementList*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
AreaElementList*
AreaDefinition::getElements() const
{
	AreaElementList* pResult = 0;
	Object* pObj = m_vecpElements[f_Elements];
	if ( pObj && ObjectType::AreaElementList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(AreaElementList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::AreaDefinition::setElements -- Elements を設定する
//
//	NOTES
//	Elements を設定する
//
//	ARGUMENTS
//	AreaElementList*
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
AreaDefinition::setElements(AreaElementList* pElements_)
{
	m_vecpElements[f_Elements] = pElements_;
	return;
}

//
//	FUNCTION public
//	Statement::AreaDefinition::copy -- 自身をコピーする
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
AreaDefinition::copy() const
{
	return new AreaDefinition(*this);
}

#if 0
namespace
{
	Analysis::AreaDefinition _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
AreaDefinition::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
