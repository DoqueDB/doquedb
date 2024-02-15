// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// XA_StartStatement.cpp --
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

#include "Statement/IntegerValue.h"
#include "Statement/IsolationLevel.h"
#include "Statement/TransactAccMode.h"
#include "Statement/TransactionModeList.h"
#include "Statement/XA_Identifier.h"
#include "Statement/XA_StartStatement.h"

#if 0
#include "Analysis/XA_StartStatement.h"
#endif
#include "Common/Assert.h"
#include "Common/UnicodeString.h"
#include "Exception/SQLSyntaxError.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace
{
	// メンバの m_vecpElements 内でのインデックス

	enum
	{
		f_Identifier,
		f_TransactionMode,
		f__end_index
	};
}

#define _SYDNEY_THROW_SQL_SYNTAX_ERROR(msg)	_SYDNEY_THROW1(Exception::SQLSyntaxError, _TRMEISTER_U_STRING(msg))

//	FUNCTION public
//	Statement::XA_StartStatement::XA_StartStatement --
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

XA_StartStatement::XA_StartStatement()
	: Object(ObjectType::XA_StartStatement,
			 f__end_index, Object::Undefine, true),
	  _join(false),
	  _resume(false)
{}

//	FUNCTION public
//	Statement::XA_StartStatement::setIdentifier --
//		トランザクションブランチ識別子を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::XA_Identifier*	p
//			トランザクションブランチ識別子が格納された領域の先頭アドレス。
//			本関数内で複製されずそのまま使われるので、
//			呼び出し側で解放してはいけない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
XA_StartStatement::setIdentifier(XA_Identifier* p)
{
	m_vecpElements[f_Identifier] = p;
}

//	FUNCTION public
//	Statement::XA_StartStatement::getIdentifier --
//		トランザクションブランチ識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		0 以外の値
//			得られたトランザクションブランチ識別子を格納する領域の先頭アドレス
//		0
//			トランザクションブランチ識別子が指定されていなかった
//
//	EXCEPTIONS
//		なし

XA_Identifier*
XA_StartStatement::getIdentifier() const
{
	if (Object* obj = m_vecpElements[f_Identifier]) {
		; _SYDNEY_ASSERT(obj->getType() == ObjectType::XA_Identifier);
		return _SYDNEY_DYNAMIC_CAST(XA_Identifier*, obj);
	}
	return 0;
}

//	FUNCTION public
//	Statement::XA_StartStatement::setTransactionMode --
//		トランザクションモードの並びを設定する
//
//	NOTES
//
//	ARGUMENTS
//		TransactionModeList&	list
//			トランザクションモードの並び
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
XA_StartStatement::setTransactionMode(const TransactionModeList& list)
{
	ModAutoPointer<IntegerValue> mode, level;
	Boolean::Value usingSnapshot = Boolean::Unknown;

	const int n = list.getCount();
	for (int i = 0; i < n; ++i)
		if (const Object* obj = list.getAt(i))
			switch (obj->getType()) {
			case ObjectType::TransactAccMode:
			{
				if (mode.get()) {
					; _SYDNEY_THROW_SQL_SYNTAX_ERROR(
						"More than one <transaction access mode> is specified");
				}

				const TransactAccMode* p =
					_SYDNEY_DYNAMIC_CAST(const TransactAccMode*, obj);
				; _SYDNEY_ASSERT(p);
				const IntegerValue* src = p->getAccMode();
				; _SYDNEY_ASSERT(src);

				mode = new IntegerValue(*src);
				break;
			}
			case ObjectType::IsolationLevel:
			{
				if (level.get()) {
					; _SYDNEY_THROW_SQL_SYNTAX_ERROR(
						"More than one <isolation level> is specified");
				}

				const IsolationLevel* p =
					_SYDNEY_DYNAMIC_CAST(const IsolationLevel*, obj);
				; _SYDNEY_ASSERT(p);
				const IntegerValue* src = p->getIsoLevel();
				; _SYDNEY_ASSERT(src);

				level = new IntegerValue(*src);
				break;
			}
			case ObjectType::IntegerValue:
			{
				if (usingSnapshot != Boolean::Unknown) {
					; _SYDNEY_THROW_SQL_SYNTAX_ERROR(
						"More than one <using snapshot> is specified");
				}

				const IntegerValue* p =
					_SYDNEY_DYNAMIC_CAST(const IntegerValue*, obj);
				; _SYDNEY_ASSERT(p);

				usingSnapshot = (p->getValue()) ?
					Boolean::True : Boolean::False;
				break;
			}
			default:
				; _SYDNEY_ASSERT(false);
			}

	if (!mode.get())
		mode = new IntegerValue(TransactionMode::AccUnknown);
	if (!level.get())
		level = new IntegerValue(TransactionMode::IsoUnknown);

	m_vecpElements[f_TransactionMode] =
		new TransactionMode(mode.release(), level.release(),
							usingSnapshot == Boolean::True);
}

//	FUNCTION public
//	Statement::XA_StartStatement::getTransactionMode --
//		トランザクションモードを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		0 以外の値
//			得られたトランザクションモードが格納された領域の先頭アドレス
//		0
//			トランザクションモードが設定されていなかった
//
//	EXCEPTIONS
//		なし

TransactionMode*
XA_StartStatement::getTransactionMode() const
{
	if (Object* obj = m_vecpElements[f_TransactionMode]) {
		; _SYDNEY_ASSERT(obj->getType() == ObjectType::TransactionMode);
		return _SYDNEY_DYNAMIC_CAST(TransactionMode*, obj);
	}
	return 0;
}

#if 0
namespace
{
	Analysis::XA_StartStatement _analyzer;
}

// Analyzerを得る
// virtual
const Analysis::Analyzer*
XA_StartStatement::getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
