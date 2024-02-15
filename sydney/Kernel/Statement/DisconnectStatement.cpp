// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DisconnectStatement.cpp --
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
#include "Statement/DisconnectStatement.h"
#include "Statement/Literal.h"
#include "Statement/Token.h"
#include "Statement/Utility.h"

#include "Common/IntegerData.h"
#include "Common/StringData.h"


#include "Exception/BadArgument.h"
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
//	Statement::DisconnectStatement::DisconnectStatement -- コンストラクタ
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
DisconnectStatement::DisconnectStatement(Mode::Value iMode_)
	: Object(ObjectType::DisconnectStatement, f__end_index),
	  m_iMode(iMode_),
	  m_iSessionId(0),
	  m_iClientId(0)
{
}

//
//	FUNCTION public
//	Statement::DisconnectStatement::~DisconnectStatement -- デストラクタ
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
DisconnectStatement::~DisconnectStatement()
{
}

//
//	FUNCTION public
//	Statement::DisconnectStatement::copy -- 自身をコピーする
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
DisconnectStatement::copy() const
{
	DisconnectStatement* p = new DisconnectStatement(*this);
	p->m_iMode = m_iMode;
	p->m_iClientId = m_iClientId;
	p->m_iSessionId = m_iSessionId;
	return p;
}

#if 0
namespace
{
	Analysis::DisconnectStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
DisconnectStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	FUNCTION public
//	Statement::DisconnectStatement::setSessionId -- セッションIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Literal& cLiteral_
//		設定するセッションIDのリテラル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DisconnectStatement::setSessionId(const Literal& cLiteral_)
{

	if(!Common::StringData::getInteger(m_iSessionId,
									   cLiteral_.getToken().getHead(),
									   cLiteral_.getToken().getTail())){
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	

}

//
//	FUNCTION public
//	Statement::DisconnectStatement::setClientId -- クライアントIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Literal& cLiteral_
//		設定するクライアントIDのリテラル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DisconnectStatement::setClientId(const Literal& cLiteral_)
{
	if(!Common::StringData::getInteger(m_iClientId,
									   cLiteral_.getToken().getHead(),
									   cLiteral_.getToken().getTail())){
		_SYDNEY_THROW0(Exception::BadArgument);
	}

}

// FUNCTION public
//	Statement::DisconnectStatement::serialize -- 
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
DisconnectStatement::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	Utility::Serialize::EnumValue(cArchive_, m_iMode);
	cArchive_(m_iClientId);
	cArchive_(m_iSessionId);
}

//
//	Copyright (c) 2005, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
