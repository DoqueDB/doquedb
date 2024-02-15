// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// XA_RollbackStatement.h --
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

#ifndef __SYDNEY_STATEMENT_XA_ROLLBACKSTATEMENT_H
#define __SYDNEY_STATEMENT_XA_ROLLBACKSTATEMENT_H

#include "Statement/Module.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class XA_Identifier;

//	CLASS
//	Statement::XA_RollbackStatement --
//
//	NOTES

class XA_RollbackStatement
	: public	Statement::Object
{
public:
	// コンストラクタ
	SYD_STATEMENT_FUNCTION
	XA_RollbackStatement();
	// デストラクタ
	virtual
	~XA_RollbackStatement();

	// トランザクションブランチ識別子の設定
	SYD_STATEMENT_FUNCTION
	void
	setIdentifier(XA_Identifier* p);
	// トランザクションブランチ識別子を得る
	SYD_STATEMENT_FUNCTION
	XA_Identifier*
	getIdentifier() const;

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
	XA_RollbackStatement
	operator =(const XA_RollbackStatement& r);
};

//	FUNCTION public
//	Statement::XA_RollbackStatement::~XA_RollbackStatement --
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
XA_RollbackStatement::~XA_RollbackStatement()
{}

//	FUNCTION public
//	Statement::XA_RollbackStatement::copy -- 自分自身をコピーする
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
XA_RollbackStatement::copy() const
{
	return new XA_RollbackStatement(*this);
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_XA_ROLLBACKSTATEMENT_H

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
