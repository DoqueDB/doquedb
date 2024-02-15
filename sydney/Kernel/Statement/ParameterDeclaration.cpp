// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ParameterDeclaration.cpp -- ParameterDeclaration
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

#include "Statement/ParameterDeclaration.h"
#include "Statement/Identifier.h"
#include "Statement/Utility.h"

#include "Analysis/Procedure/ParameterDeclaration.h"

#include "Common/SQLData.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Name,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::ParameterDeclaration::ParameterDeclaration -- コンストラクタ
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Identifier* pName_
//		const Common::SQLData& cType_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ParameterDeclaration::ParameterDeclaration(Identifier* pName_,
										   const Common::SQLData& cType_)
	: Object(ObjectType::ParameterDeclaration, f__end_index)
{
	// Name を設定する
	setParameterName(pName_);
	// DataType を設定する
	setParameterType(cType_);
}

//
//	FUNCTION public
//		Statement::ParameterDeclaration::getParameterName -- ParameterName を得る
//
//	NOTES
//		ParameterName を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Identifier*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Identifier*
ParameterDeclaration::getParameterName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Name];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::ParameterDeclaration::setParameterName -- ParameterName を設定する
//
//	NOTES
//		ParameterName を設定する
//
//	ARGUMENTS
//		Identifier* pName_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ParameterDeclaration::setParameterName(Identifier* pName_)
{
	m_vecpElements[f_Name] = pName_;
}

// FUNCTION public
//	Statement::ParameterDeclaration::getParameterType -- DataType を得る
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
ParameterDeclaration::getParameterType() const
{
	return m_cType;
}

// FUNCTION public
//	Statement::ParameterDeclaration::setParameterType -- DataType を設定する
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
ParameterDeclaration::setParameterType(const Common::SQLData& cType_)
{
	m_cType = cType_;
}

// FUNCTION public
//	Statement::ParameterDeclaration::toSQLStatement -- SQL文で値を得る
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
ParameterDeclaration::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;

	stream << getParameterName()->toSQLStatement(bForCascade_)
		   << " "
		   << getParameterType().toSQLStatement();
	return stream.getString();
}

//
//	FUNCTION public
//	Statement::ParameterDeclaration::copy -- 自身をコピーする
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
ParameterDeclaration::copy() const
{
	return new ParameterDeclaration(*this);
}

// FUNCTION public
//	Statement::ParameterDeclaration::getAnalyzer2 -- 
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
ParameterDeclaration::
getAnalyzer2() const
{
	return Analysis::Procedure::ParameterDeclaration::create(this);
}

// FUNCTION public
//	Statement::ParameterDeclaration::serialize -- 
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
ParameterDeclaration::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	Utility::Serialize::SQLDataType(cArchive_, m_cType);
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
