// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// XA_RecoverStatement.h --
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

#ifndef __SYDNEY_STATEMENT_XA_RECOVERSTATEMENT_H
#define __SYDNEY_STATEMENT_XA_RECOVERSTATEMENT_H

#include "Statement/Module.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

//	CLASS
//	Statement::XA_RecoverStatement --
//
//	NOTES

class XA_RecoverStatement
	: public	Statement::Object
{
public:
	// コンストラクタ
	SYD_STATEMENT_FUNCTION
	XA_RecoverStatement();
	// デストラクタ
	virtual
	~XA_RecoverStatement();

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
	XA_RecoverStatement
	operator =(const XA_RecoverStatement& r);
};

//	FUNCTION public
//	Statement::XA_RecoverStatement::~XA_RecoverStatement --
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
XA_RecoverStatement::~XA_RecoverStatement()
{}

//	FUNCTION public
//	Statement::XA_RecoverStatement::copy -- 自分自身をコピーする
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
XA_RecoverStatement::copy() const
{
	return new XA_RecoverStatement(*this);
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_XA_RECOVERSTATEMENT_H

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
