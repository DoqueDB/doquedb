// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SyncStatement.cpp --
// 
// Copyright (c) 2005, 2012, 2023 Ricoh Company, Ltd.
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
#include "Statement/SyncStatement.h"
#include "Statement/Literal.h"
#include "Statement/Token.h"
#if 0
#include "Analysis/SyncStatement.h"
#endif

#include "Common/IntegerData.h"
#include "Common/StringData.h"

_SYDNEY_USING
using namespace Statement;

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::SyncStatement::SyncStatement -- コンストラクタ
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
SyncStatement::SyncStatement()
	: Object(ObjectType::SyncStatement, f__end_index),
	  m_iCount(2)
{
}

//
//	FUNCTION public
//	Statement::SyncStatement::~SyncStatement -- デストラクタ
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
SyncStatement::~SyncStatement()
{
}

//
//	FUNCTION public
//	Statement::SyncStatement::copy -- 自身をコピーする
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
SyncStatement::copy() const
{
	SyncStatement* p = new SyncStatement(*this);
	p->m_iCount = m_iCount;
	return p;
}

#if 0
namespace
{
	Analysis::SyncStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
SyncStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	FUNCTION public
//	Statement::SyncStatement::setCount -- カウントを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Literal& cLiteral_
//		設定するカウントのリテラル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SyncStatement::setCount(const Literal& cLiteral_)
{
	Common::Data::Pointer p0 = Common::StringData::getInteger(cLiteral_.getToken().getHead(),
															  cLiteral_.getToken().getTail(),
															  Common::DataType::Integer,
															  true /* for assign */);
	Common::IntegerData* p
		= _SYDNEY_DYNAMIC_CAST(Common::IntegerData*, p0.get());
	m_iCount = p->getValue();
}

// FUNCTION public
//	Statement::SyncStatement::serialize -- 
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
SyncStatement::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	cArchive_(m_iCount);
}

//
//	Copyright (c) 2005, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
