// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// XA_CommitStatement.h --
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_XA_COMMITSTATEMENT_H
#define __SYDNEY_STATEMENT_XA_COMMITSTATEMENT_H

#include "Statement/Module.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class XA_Identifier;

//	CLASS
//	Statement::XA_CommitStatement --
//
//	NOTES

class XA_CommitStatement
	: public	Statement::Object
{
public:
	// コンストラクタ
	SYD_STATEMENT_FUNCTION
	XA_CommitStatement();
	// デストラクタ
	virtual
	~XA_CommitStatement();

	// トランザクションブランチ識別子の設定
	SYD_STATEMENT_FUNCTION
	void
	setIdentifier(XA_Identifier* p);
	// トランザクションブランチ識別子を得る
	SYD_STATEMENT_FUNCTION
	XA_Identifier*
	getIdentifier() const;

	// ONE PHASE 指定の有無を設定する
	void
	setOnePhase(bool v);
	// ONE PHASE 指定の有無を得る
	bool
	getOnePhase() const;

	// 自身をコピーする
	Object*
	copy() const;

#if 0
	// Analyzerを得る
	virtual
	const Analysis::Analyzer*
	getAnalyzer() const;
#endif

private:
	// 代入オペレータは使用しない
	XA_CommitStatement
	operator =(const XA_CommitStatement& r);

	// ONE PHASE 指定の有無を表す
	bool					_onePhase;
};

//	FUNCTION public
//	Statement::XA_CommitStatement::~XA_CommitStatement --
//		デストラクタ
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
//		なし

inline
XA_CommitStatement::~XA_CommitStatement()
{}

//	FUNCTION public
//	Statement::XA_CommitStatement::copy -- 自分自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		コピーされた自分自身
//
//	EXCEPTIONS

inline
Object*
XA_CommitStatement::copy() const
{
	return new XA_CommitStatement(*this);
}

//	FUNCTION public
//	Statement::XA_CommitStatement::setOnePhase --
//		ONE PHASE 指定の有無を設定する
//
//	NOTES
//
//	ARGUMENTS
//		bool				v
//			設定する ONE PHASE 指定の有無を表す値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
XA_CommitStatement::setOnePhase(bool v)
{
	_onePhase = v;
}

//	FUNCTION public
//	Statement::XA_CommitStatement::getOnePhase --
//		ONE PHASE 指定の有無を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			ONE PHASE が指定されていた
//		false
//			ONE PHASE が指定されていなかった
//
//	EXCEPTIONS
//		なし

inline
bool
XA_CommitStatement::getOnePhase() const
{
	return _onePhase;
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_XA_COMMITSTATEMENT_H

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
