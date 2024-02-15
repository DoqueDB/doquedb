// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// XA_Identifier.cpp --
// 
// Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
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

#include "Statement/Literal.h"
#include "Statement/XA_Identifier.h"

#if 0
#include "Analysis/XA_Identifier.h"
#endif
#include "Common/Assert.h"
#include "Common/IntegerData.h"
#include "Exception/BadArgument.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace
{

namespace _XA_Identifier
{
	namespace _FormatID
	{
		const int			Default = 1;
	}
}

}

//	FUNCTION public
//	Statement::XA_Identifier::XA_Identifier --
//		コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

XA_Identifier::XA_Identifier()
	: Object(ObjectType::XA_Identifier),
	  _formatID(_XA_Identifier::_FormatID::Default)
{}

//	FUNCTION public
//	Statement::XA_Identifier::setGlobalTransactionIdentifier --
//		グローバルトランザクション識別子の設定
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Literal&	v
//			設定するグローバルトランザクション識別子を表すバイナリ列リテラル
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
XA_Identifier::setGlobalTransactionIdentifier(const Literal& v)
{
	Common::Data::Pointer p(v.createData());
	; _SYDNEY_ASSERT(p.get());
	if (p->getType() != Common::DataType::Binary)
		_SYDNEY_THROW0(Exception::BadArgument);

	_gtrID = *(_SYDNEY_DYNAMIC_CAST(Common::BinaryData*, p.get()));
}

//	FUNCTION public
//	Statement::XA_Identifier::setBranchQualifier --
//		トランザクションブランチ限定子の設定
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Literal&	v
//			設定するトランザクションブランチ限定子を表すバイナリ列リテラル
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
XA_Identifier::setBranchQualifier(const Literal& v)
{
	Common::Data::Pointer p(v.createData());
	; _SYDNEY_ASSERT(p.get());
	if (p->getType() != Common::DataType::Binary)
		_SYDNEY_THROW0(Exception::BadArgument);

	_bqual = *(_SYDNEY_DYNAMIC_CAST(Common::BinaryData*, p.get()));
}

//	FUNCTION public
//	Statement::XA_Identifier::setFormatIdentifier --
//		フォーマット識別子の設定
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Literal&	v
//			設定するフォーマット識別子を表すバイナリ列リテラル
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
XA_Identifier::setFormatIdentifier(const Literal& v)
{
	Common::Data::Pointer p(v.createData(Common::DataType::Integer));
	; _SYDNEY_ASSERT(p.get());
	; _SYDNEY_ASSERT(p->getType() == Common::DataType::Integer);

	_formatID = (_SYDNEY_DYNAMIC_CAST(
					 Common::IntegerData*, p.get()))->getValue();
}

#if 0
namespace
{
	Analysis::XA_Identifier _analyzer;
}

// Analyzerを得る
// virtual
const Analysis::Analyzer*
XA_Identifier::getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
