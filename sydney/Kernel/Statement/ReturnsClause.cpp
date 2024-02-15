// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReturnsClause.cpp -- ReturnsClause
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/ReturnsClause.h"
#include "Statement/Utility.h"

#include "Analysis/Procedure/ReturnsClause.h"

#include "Common/SQLData.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::ReturnsClause::ReturnsClause -- コンストラクタ
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		const Common::SQLData& cType_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ReturnsClause::ReturnsClause(const Common::SQLData& cType_)
	: Object(ObjectType::ReturnsClause, f__end_index)
{
	// DataType を設定する
	setDataType(cType_);
}

// FUNCTION public
//	Statement::ReturnsClause::getDataType -- DataType を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::SQLData&
//
// EXCEPTIONS

const Common::SQLData&
ReturnsClause::getDataType() const
{
	return m_cType;
}

// FUNCTION public
//	Statement::ReturnsClause::setDataType -- DataType を設定する
//
// NOTES
//
// ARGUMENTS
//	const Common::SQLData& cType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ReturnsClause::setDataType(const Common::SQLData& cType_)
{
	m_cType = cType_;
}

// FUNCTION public
//	Statement::ReturnsClause::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
ReturnsClause::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;
	stream << "returns "
		   << getDataType().toSQLStatement();
	return stream.getString();
}

//
//	FUNCTION public
//	Statement::ReturnsClause::copy -- 自身をコピーする
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
ReturnsClause::copy() const
{
	return new ReturnsClause(*this);
}

// FUNCTION public
//	Statement::ReturnsClause::getAnalyzer2 -- 
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
ReturnsClause::
getAnalyzer2() const
{
	return Analysis::Procedure::ReturnsClause::create(this);
}

// FUNCTION public
//	Statement::ReturnsClause::serialize -- 
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
ReturnsClause::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	Utility::Serialize::SQLDataType(cArchive_, m_cType);
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
