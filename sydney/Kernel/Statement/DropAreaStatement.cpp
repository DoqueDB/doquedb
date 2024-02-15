// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DropAreaStatement.cpp --
// 
// Copyright (c) 2000, 2002, 2005, 2006, 2012, 2023 Ricoh Company, Ltd.
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
#include "Statement/DropAreaStatement.h"
#include "Statement/Identifier.h"
#if 0
#include "Analysis/DropAreaStatement.h"
#endif

_SYDNEY_USING

using namespace Statement;

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Name,
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::DropAreaStatement::DropAreaStatement -- コンストラクタ
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
DropAreaStatement::DropAreaStatement( Identifier* pName_, bool bIfExists_ )
	: Object(ObjectType::DropAreaStatement, f__end_index, Object::Reorganize),
	  m_bIfExists(bIfExists_)
{
	setName(pName_);
}

// FUNCTION public
//	Statement::DropAreaStatement::DropAreaStatement -- コンストラクタ (2)
//
// NOTES
//
// ARGUMENTS
//	const DropAreaStatement& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

DropAreaStatement::
DropAreaStatement(const DropAreaStatement& cOther_)
	: Object(cOther_),
	  m_bIfExists(cOther_.m_bIfExists)
{
}

//
//	FUNCTION public
//	Statement::DropAreaStatement::~DropAreaStatement -- デストラクタ
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
DropAreaStatement::~DropAreaStatement()
{
}

//
//	FUNCTION public
//	Statement::DropAreaStatement::
//		-- Name を得る
//
//	NOTES
//	Name を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Identifier*
//
//
//	EXCEPTIONS
//	なし
//
Identifier*
DropAreaStatement::getName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Name];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::DropAreaStatement::
//		-- Name を設定する
//
//	NOTES
//	Name を設定する
//
//	ARGUMENTS
//
//
//	RETURN
//
//
//	EXCEPTIONS
//	なし
//
void
DropAreaStatement::setName(Identifier* pName_)
{
	m_vecpElements[f_Name] = pName_;
}

//
//	FUNCTION public
//	Statement::DropAreaStatement::
//		-- Name を ModString で得る
//
//	NOTES
//	Name を ModString で得る
//
//	ARGUMENTS
//
//
//	RETURN
//
//
//	EXCEPTIONS
//	なし
//
const ModUnicodeString*
DropAreaStatement::getNameString() const
{
	Identifier* pIdentifier = getName();
	return pIdentifier ? pIdentifier->getIdentifier() : 0;
}

// FUNCTION public
//	Statement::DropAreaStatement::isIfExists -- if exists?
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
DropAreaStatement::
isIfExists() const
{
	return m_bIfExists;
}

//
//	FUNCTION public
//	Statement::DropAreaStatement::copy -- 自身をコピーする
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
DropAreaStatement::copy() const
{
	return new DropAreaStatement(*this);
}

#if 0
namespace
{
	Analysis::DropAreaStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
DropAreaStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::DropAreaStatement::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
DropAreaStatement::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	cArchive_(m_bIfExists);
}

//
//	Copyright (c) 2000, 2002, 2005, 2006, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
