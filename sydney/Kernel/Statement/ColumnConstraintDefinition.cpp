// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ColumnConstraintDefinition.cpp -- ColumnConstraintDefinition
// 
// Copyright (c) 2003, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
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
	const char moduleName[] = "Statement";
	const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Statement/ColumnConstraintDefinition.h"
#include "Statement/Type.h"
#include "Statement/ColumnNameList.h"
#include "Statement/IntegerValue.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#if 0
#include "Analysis/ColumnConstraintDefinition.h"
#endif

_SYDNEY_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_ConstraintType,
		f__end_index
	};
}

using namespace Statement;

//
//	FUNCTION public
//		Statement::ColumnConstraintDefinition::ColumnConstraintDefinition -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		int iType_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ColumnConstraintDefinition::ColumnConstraintDefinition(int iConstraintType_)
	: Object(ObjectType::ColumnConstraintDefinition, f__end_index, Object::Reorganize)
{
	// ConstraintType を設定する
	setConstraintType(iConstraintType_);
}

//
//	FUNCTION public
//		Statement::ColumnConstraintDefinition::getConstraintType -- ConstraintType を得る
//
//	NOTES
//		ConstraintType を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
int
ColumnConstraintDefinition::getConstraintType() const
{
	int iResult = 0;
	Object* pObj = m_vecpElements[f_ConstraintType];
	if ( pObj && ObjectType::IntegerValue == pObj->getType() )
		iResult = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pObj)->getValue();
	return iResult;
}

//
//	FUNCTION public
//		Statement::ColumnConstraintDefinition::setConstraintType -- ConstraintType を設定する
//
//	NOTES
//		ConstraintType を設定する
//
//	ARGUMENTS
//		int iConstraintType_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ColumnConstraintDefinition::setConstraintType(int iConstraintType_)
{
	IntegerValue* pIntVal = _SYDNEY_DYNAMIC_CAST(IntegerValue*, m_vecpElements[f_ConstraintType]);

	if (pIntVal == 0)
	{
		pIntVal = new IntegerValue;
		m_vecpElements[f_ConstraintType] = pIntVal;
	}
	pIntVal->setValue(iConstraintType_);
}

//
//	FUNCTION public
//	Statement::ColumnConstraintDefinition::copy -- 自身をコピーする
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
ColumnConstraintDefinition::copy() const
{
	return new ColumnConstraintDefinition(*this);
}

namespace
{
	const char* const _pszConstraintName[] =
	{
		"",
		"not null",
		"primary key",
		"unique",
	};
}

// SQL文で値を得る
//virtual
ModUnicodeString
ColumnConstraintDefinition::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	return ModUnicodeString(_pszConstraintName[getConstraintType()]);
}

#if 0
namespace
{
	Analysis::ColumnConstraintDefinition _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
ColumnConstraintDefinition::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2003, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
