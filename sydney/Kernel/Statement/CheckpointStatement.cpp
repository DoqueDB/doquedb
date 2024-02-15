// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CheckpointStatement.cpp --
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
#include "Statement/CheckpointStatement.h"
#include "Statement/Literal.h"
#include "Statement/Token.h"
#if 0
#include "Analysis/CheckpointStatement.h"
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
//	Statement::CheckpointStatement::CheckpointStatement -- コンストラクタ
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
CheckpointStatement::CheckpointStatement()
	: Object(ObjectType::CheckpointStatement, f__end_index),
	  m_iCount(2)
{
}

//
//	FUNCTION public
//	Statement::CheckpointStatement::~CheckpointStatement -- デストラクタ
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
CheckpointStatement::~CheckpointStatement()
{
}

//
//	FUNCTION public
//	Statement::CheckpointStatement::copy -- 自身をコピーする
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
CheckpointStatement::copy() const
{
	CheckpointStatement* p = new CheckpointStatement(*this);
	p->m_iCount = m_iCount;
	return p;
}

#if 0
namespace
{
	Analysis::CheckpointStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
CheckpointStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	FUNCTION public
//	Statement::CheckpointStatement::setCount -- カウントを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Literal& cLiteral_
//		設定するカウントを表すリテラル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
CheckpointStatement::setCount(const Literal& cLiteral_)
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
//	Statement::CheckpointStatement::serialize -- 
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
CheckpointStatement::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	cArchive_(m_iCount);
}

//
//	Copyright (c) 2005, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
