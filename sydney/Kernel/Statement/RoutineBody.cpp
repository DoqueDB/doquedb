// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RoutineBody.cpp -- RoutineBody
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

#include "Statement/RoutineBody.h"
#include "Statement/Identifier.h"

#include "Analysis/Procedure/RoutineBody.h"

#include "Common/SQLData.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Statement,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::RoutineBody::RoutineBody -- コンストラクタ
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Object* pStatement_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
RoutineBody::RoutineBody(Object* pStatement_)
	: Object(ObjectType::RoutineBody, f__end_index)
{
	// Statement を設定する
	setStatement(pStatement_);
}

//
//	FUNCTION public
//		Statement::RoutineBody::getStatement -- Statement を得る
//
//	NOTES
//		Statement を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Object*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Object*
RoutineBody::getStatement() const
{
	return m_vecpElements[f_Statement];
}

//
//	FUNCTION public
//		Statement::RoutineBody::setStatement -- Statement を設定する
//
//	NOTES
//		Statement を設定する
//
//	ARGUMENTS
//		Object* pStatement_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
RoutineBody::setStatement(Object* pStatement_)
{
	m_vecpElements[f_Statement] = pStatement_;
}

// FUNCTION public
//	Statement::RoutineBody::toSQLStatement -- SQL文で値を得る
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
RoutineBody::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;
	stream << "return "
		   << getStatement()->toSQLStatement(bForCascade_);
	return stream.getString();
}

//
//	FUNCTION public
//	Statement::RoutineBody::copy -- 自身をコピーする
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
RoutineBody::copy() const
{
	return new RoutineBody(*this);
}

// FUNCTION public
//	Statement::RoutineBody::getAnalyzer2 -- 
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
RoutineBody::
getAnalyzer2() const
{
	return Analysis::Procedure::RoutineBody::create(this);
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
