// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InPredicate.cpp -- IN 述語関連の関数定義
// 
// Copyright (c) 2002, 2005, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/InPredicate.h"
#include "Statement/Type.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/InPredicate.h"
#include "Analysis/InPredicate_Value.h"
#endif

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Left,
		f_Right,
		f__end_index
	};
}

//	FUNCTION public
//	Statement::InPredicate::InPredicate --
//		IN 述語を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	row
//			対象のRow型式
//		Statement::Object* inValue
//			照合対象のRow型リストまたは副問い合わせ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

InPredicate::InPredicate(
	ValueExpression* row, Object* inValue)
	: Object(ObjectType::InPredicate, f__end_index)
{
	; _SYDNEY_ASSERT(row);
	; _SYDNEY_ASSERT(inValue);

	// 対象のRow型式を設定する
	setLeft(row);
	// 照合対象を設定する
	setRight(inValue);
};

//	FUNCTION public
//	Statement::InPredicate::getLeft -- 対象のRow型式を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた対象のRow型式をが格納されている領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

ValueExpression*
InPredicate::getLeft() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(f_Left,
				   ObjectType::ValueExpression));
}

//	FUNCTION public
//	Statement::InPredicate::setString -- 対象のRow型式を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	row
//			対象のRow型式
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
InPredicate::setLeft(ValueExpression* row)
{
	setElement(f_Left, row);
}

//	FUNCTION public
//	Statement::InPredicate::getRight -- 照合対象を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた照合対象が格納されている領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

Statement::Object*
InPredicate::getRight() const
{
	return m_vecpElements[f_Right];
}

//	FUNCTION public
//	Statement::InPredicate::setRight -- 照合対象を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Object*	inPredicateValue
//			設定する照合対象が格納されている領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
InPredicate::setRight(Object* inValue)
{
	m_vecpElements[f_Right] = inValue;
}

//	FUNCTION public
//	Statement::InPredicate::copy -- 自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		自身をコピーして生成された IN 述語を表すクラスを
//		格納する領域の先頭アドレス
//
//	EXCEPTIONS

Object*
InPredicate::copy() const
{
	return new InPredicate(*this);
}

// FUNCTION public
//	Statement::InPredicate::toSQLStatement -- SQL文で値を得る
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
InPredicate::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;
	stream << getLeft()->toSQLStatement(bForCascade_)
		   << " in ("
		   << getRight()->toSQLStatement(bForCascade_)
		   << ")";
	return stream.getString();
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::InPredicate _analyzer_subquery;
	Analysis::InPredicate_Value _analyzer_value;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
InPredicate::
getAnalyzer() const
{
	if (ValueExpression* pVERight = _SYDNEY_DYNAMIC_CAST(
										 ValueExpression*,
										 getElement(f_Right,
													ObjectType::ValueExpression))) {
		if (pVERight->getOperator() == ValueExpression::op_Tblconst)
			return &_analyzer_value;
	}
	return &_analyzer_subquery;
}
#endif

//
//	Copyright (c) 2002, 2005, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
