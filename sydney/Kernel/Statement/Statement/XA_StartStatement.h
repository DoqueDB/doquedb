// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// XA_StartStatement.h --
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

#ifndef __SYDNEY_STATEMENT_XA_STARTSTATEMENT_H
#define __SYDNEY_STATEMENT_XA_STARTSTATEMENT_H

#include "Statement/Module.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class TransactionMode;
class TransactionModeList;
class XA_Identifier;

//	CLASS
//	Statement::XA_StartStatement --
//
//	NOTES

class XA_StartStatement
	: public	Statement::Object
{
public:
	// コンストラクタ
	SYD_STATEMENT_FUNCTION
	XA_StartStatement();
	// デストラクタ
	virtual
	~XA_StartStatement();

	// トランザクションブランチ識別子の設定
	SYD_STATEMENT_FUNCTION
	void
	setIdentifier(XA_Identifier* p);
	// トランザクションブランチ識別子を得る
	SYD_STATEMENT_FUNCTION
	XA_Identifier*
	getIdentifier() const;

	// トランザクションモードの並びを設定する
	SYD_STATEMENT_FUNCTION
	void
	setTransactionMode(const TransactionModeList& list);
	// トランザクションモードを得る
	SYD_STATEMENT_FUNCTION
	TransactionMode*
	getTransactionMode() const;

	// JOIN 指定の有無を設定する
	void
	setJoin(bool v);
	// JOIN 指定の有無を得る
	bool
	getJoin() const;

	// RESUME 指定の有無を設定する
	void
	setResume(bool v);
	// RESUME 指定の有無を得る
	bool
	getResume() const;

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
	XA_StartStatement
	operator =(const XA_StartStatement& r);

	// JOIN 指定の有無を表す
	bool					_join;
	// RESUME 指定の有無を表す
	bool					_resume;
};

//	FUNCTION public
//	Statement::XA_StartStatement::~XA_StartStatement --
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
XA_StartStatement::~XA_StartStatement()
{}

//	FUNCTION public
//	Statement::XA_StartStatement::copy -- 自分自身をコピーする
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
XA_StartStatement::copy() const
{
	return new XA_StartStatement(*this);
}

//	FUNCTION public
//	Statement::XA_StartStatement::setJoin -- JOIN 指定の有無を設定する
//
//	NOTES
//
//	ARGUMENTS
//		bool				v
//			JOIN 指定の有無
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
XA_StartStatement::setJoin(bool v)
{
	_join = v;
}

//	FUNCTION public
//	Statement::XA_StartStatement::getJoin -- JOIN 指定の有無を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			JOIN 指定がされていた
//		false
//			JOIN 指定がされていなかった
//
//	EXCEPTIONS
//		なし

inline
bool
XA_StartStatement::getJoin() const
{
	return _join;
}

//	FUNCTION public
//	Statement::XA_StartStatement::setResume -- RESUME 指定の有無を設定する
//
//	NOTES
//
//	ARGUMENTS
//		bool				v
//			RESUME 指定の有無
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
XA_StartStatement::setResume(bool v)
{
	_resume = v;
}

//	FUNCTION public
//	Statement::XA_StartStatement::getResume -- RESUME 指定の有無を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			RESUME 指定がされていた
//		false
//			RESUME 指定がされていなかった
//
//	EXCEPTIONS
//		なし

inline
bool
XA_StartStatement::getResume() const
{
	return _resume;
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_XA_STARTSTATEMENT_H

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
