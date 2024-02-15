// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BulkSpecification.cpp -- BulkSpecification
// 
// Copyright (c) 2006, 2007, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/BulkSpecification.h"
#include "Statement/Type.h"
#include "Statement/ValueExpression.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/BulkSpecification.h"
#endif
#include "Analysis/Query/BulkSpecification.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"

_SYDNEY_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_InputData,
		f_With,
		f_Hint,
		f__end_index
	};
}

_SYDNEY_STATEMENT_USING

// FUNCTION public
//	Statement::BulkSpecification::BulkSpecification -- コンストラクタ
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* pInputData_
//	ValueExpression* pWith_
//	ValueExpression* pHint_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

BulkSpecification::
BulkSpecification(ValueExpression* pInputData_,
				  ValueExpression* pWith_,
				  ValueExpression* pHint_)
	: Object(ObjectType::BulkSpecification, f__end_index, Object::Optimize),
	  m_bInput(false)
{
	// InputData を設定する
	setInputData(pInputData_);
	// With を設定する
	setWith(pWith_);
	// Hint を設定する
	setHint(pHint_);
}

//
//	FUNCTION public
//		Statement::BulkSpecification::getInputData -- InputData を得る
//
//	NOTES
//		InputData を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ValueExpression*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression*
BulkSpecification::getInputData() const
{
	return _SYDNEY_DYNAMIC_CAST(ValueExpression*, getElement(f_InputData, ObjectType::ValueExpression));
}

//
//	FUNCTION public
//		Statement::BulkSpecification::setInputData -- InputData を設定する
//
//	NOTES
//		InputData を設定する
//
//	ARGUMENTS
//		ValueExpression* pInputData_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
BulkSpecification::setInputData(ValueExpression* pInputData_)
{
	setElement(f_InputData, pInputData_);
}

//
//	FUNCTION public
//		Statement::BulkSpecification::getWith -- With を得る
//
//	NOTES
//		With を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ValueExpression*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression*
BulkSpecification::getWith() const
{
	return _SYDNEY_DYNAMIC_CAST(ValueExpression*, getElement(f_With, ObjectType::ValueExpression));
}

//
//	FUNCTION public
//		Statement::BulkSpecification::setWith -- With を設定する
//
//	NOTES
//		With を設定する
//
//	ARGUMENTS
//		ValueExpression* pWith_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
BulkSpecification::setWith(ValueExpression* pWith_)
{
	setElement(f_With, pWith_);
}

//
//	FUNCTION public
//		Statement::BulkSpecification::getHint -- Hint を得る
//
//	NOTES
//		Hint を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ValueExpression*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression*
BulkSpecification::getHint() const
{
	return _SYDNEY_DYNAMIC_CAST(ValueExpression*, getElement(f_Hint, ObjectType::ValueExpression));
}

//
//	FUNCTION public
//		Statement::BulkSpecification::setHint -- Hint を設定する
//
//	NOTES
//		Hint を設定する
//
//	ARGUMENTS
//		ValueExpression* pHint_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
BulkSpecification::setHint(ValueExpression* pHint_)
{
	setElement(f_Hint, pHint_);
}

// FUNCTION public
//	Statement::BulkSpecification::isInput -- 入力用かを示すフラグを得る
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
BulkSpecification::
isInput() const
{
	return m_bInput;
}

// FUNCTION public
//	Statement::BulkSpecification::setInput -- 入力用かを示すフラグを設定する
//
// NOTES
//
// ARGUMENTS
//	bool bInput_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
BulkSpecification::
setInput(bool bInput_)
{
	m_bInput = bInput_;
}

//
//	FUNCTION public
//	Statement::BulkSpecification::copy -- 自身をコピーする
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
BulkSpecification::copy() const
{
	return new BulkSpecification(*this);
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::BulkSpecification _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
BulkSpecification::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::BulkSpecification::getAnalyzer2 -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Interface::IAnalyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Interface::IAnalyzer*
BulkSpecification::
getAnalyzer2() const
{
	return Analysis::Query::BulkSpecification::create(this);
}

// FUNCTION public
//	Statement::BulkSpecification::serialize -- 
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
BulkSpecification::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	cArchive_(m_bInput);
}

//
//	Copyright (c) 2006, 2007, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
