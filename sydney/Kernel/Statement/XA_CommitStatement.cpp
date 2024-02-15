// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// XA_CommitStatement.cpp --
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

#include "Statement/XA_CommitStatement.h"
#include "Statement/XA_Identifier.h"

#if 0
#include "Analysis/XA_CommitStatement.h"
#endif
#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace
{
	// メンバの m_vecpElements 内でのインデックス

	enum
	{
		f_Identifier,
		f__end_index
	};
}

//	FUNCTION public
//	Statement::XA_CommitStatement::XA_CommitStatement --
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

XA_CommitStatement::XA_CommitStatement()
	: Object(ObjectType::XA_CommitStatement,
			 f__end_index, Object::Undefine, true),
	  _onePhase(false)
{}

//	FUNCTION public
//	Statement::XA_CommitStatement::setIdentifier --
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
XA_CommitStatement::setIdentifier(XA_Identifier* p)
{
	m_vecpElements[f_Identifier] = p;
}

//	FUNCTION public
//	Statement::XA_CommitStatement::getIdentifier --
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
XA_CommitStatement::getIdentifier() const
{
	if (Object* obj = m_vecpElements[f_Identifier]) {
		; _SYDNEY_ASSERT(obj->getType() == ObjectType::XA_Identifier);
		return _SYDNEY_DYNAMIC_CAST(XA_Identifier*, obj);
	}
	return 0;
}

#if 0
namespace
{
	Analysis::XA_CommitStatement _analyzer;
}

// Analyzerを得る
// virtual
const Analysis::Analyzer*
XA_CommitStatement::getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
