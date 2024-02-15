// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// XA_Identifier.h --
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

#ifndef __SYDNEY_STATEMENT_XA_IDENTIFIER_H
#define __SYDNEY_STATEMENT_XA_IDENTIFIER_H

#include "Statement/Module.h"
#include "Statement/Object.h"

#include "Common/BinaryData.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Literal;

//	CLASS
//	Statement::XA_Identifier --
//
//	NOTES

class XA_Identifier
	: public	Statement::Object
{
public:
	// コンストラクタ
	SYD_STATEMENT_FUNCTION
	XA_Identifier();
	// デストラクタ
	virtual
	~XA_Identifier();

	// グローバルトランザクション識別子の設定
	SYD_STATEMENT_FUNCTION
	void
	setGlobalTransactionIdentifier(const Literal& v);
	// グローバルトランザクション識別子の取得
	const Common::BinaryData&
	getGlobalTransactionIdentifier() const;

	// トランザクションブランチ限定子の設定
	SYD_STATEMENT_FUNCTION
	void
	setBranchQualifier(const Literal& v);
	// トランザクションブランチ限定子の取得
	const Common::BinaryData&
	getBranchQualifier() const;

	// フォーマット識別子の設定
	SYD_STATEMENT_FUNCTION
	void
	setFormatIdentifier(const Literal& v);
	// フォーマット識別子を得る
	int
	getFormatIdentifier() const;

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
	XA_Identifier
	operator =(const XA_Identifier& r);

	// グローバルトランザクション識別子
	Common::BinaryData		_gtrID;
	// トランザクションブランチ限定子
	Common::BinaryData		_bqual;
	// フォーマット識別子
	int						_formatID;
};

//	FUNCTION public
//	Statement::XA_Identifier::~XA_Identifier --
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
XA_Identifier::~XA_Identifier()
{}

//	FUNCTION public
//	Statement::XA_Identifier::copy -- 自分自身をコピーする
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
XA_Identifier::copy() const
{
	return new XA_Identifier(*this);
}

//	FUNCTION public
//	Statement::XA_Identifier::getGlobalTransactionIdentifier --
//		グローバルトランザクション識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		グローバルトランザクション識別子を表すバイナリ値
//
//	EXCEPTIONS
//		なし

inline
const Common::BinaryData&
XA_Identifier::getGlobalTransactionIdentifier() const
{
	return _gtrID;
}

//	FUNCTION public
//	Statement::XA_Identifier::getBranchQualifier --
//		トランザクションブランチ限定子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		トランザクションブランチ限定子を表すバイナリ値
//
//	EXCEPTIONS
//		なし

inline
const Common::BinaryData&
XA_Identifier::getBranchQualifier() const
{
	return _bqual;
}

//	FUNCTION public
//	Statement::XA_Identifier::getFormatIdentifier -- フォーマット識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		フォーマット識別子を表す整数値
//
//	EXCEPTIONS
//		なし

inline
int
XA_Identifier::getFormatIdentifier() const
{
	return _formatID;
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_XA_IDENTIFIER_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
