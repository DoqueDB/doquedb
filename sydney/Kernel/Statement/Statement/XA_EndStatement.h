// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// XA_EndStatement.h --
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

#ifndef __SYDNEY_STATEMENT_XA_ENDSTATEMENT_H
#define __SYDNEY_STATEMENT_XA_ENDSTATEMENT_H

#include "Statement/Module.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class XA_Identifier;

//	CLASS
//	Statement::XA_EndStatement --
//
//	NOTES

class XA_EndStatement
	: public	Statement::Object
{
public:
	struct SuspensionMode
	{
		//	ENUM
		//	Statement::XA_EndStatement::SuspensionMode::Value --
		//		どのような中断をするかを表す値の型
		//
		//	NOTES

		typedef unsigned char	Value;
		enum
		{
			// 単に中断する
			Normal =		0,
			// 他セッションから再開可能なように中断する
			ForMigrate,
			// 値の数
			Count,
			// 不明
			Unknown =		Count
		};
	};

	// コンストラクタ
	SYD_STATEMENT_FUNCTION
	XA_EndStatement();
	// デストラクタ
	virtual
	~XA_EndStatement();

	// トランザクションブランチ識別子の設定
	SYD_STATEMENT_FUNCTION
	void
	setIdentifier(XA_Identifier* p);
	// トランザクションブランチ識別子を得る
	SYD_STATEMENT_FUNCTION
	XA_Identifier*
	getIdentifier() const;

	// SUSPEND (FOR MIGRATE) 指定の有無を設定する
	void
	setSuspensionMode(SuspensionMode::Value v);
	// SUSPEND (FOR MIGRATE) 指定の有無を得る
	SuspensionMode::Value
	getSuspensionMode() const;

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
	XA_EndStatement
	operator =(const XA_EndStatement& r);

	// SUSPEND (FOR MIGRATE) 指定の有無を表す
	SuspensionMode::Value	_suspensionMode;
};

//	FUNCTION public
//	Statement::XA_EndStatement::~XA_EndStatement --
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
XA_EndStatement::~XA_EndStatement()
{}

//	FUNCTION public
//	Statement::XA_EndStatement::copy -- 自分自身をコピーする
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
XA_EndStatement::copy() const
{
	return new XA_EndStatement(*this);
}

//	FUNCTION public
//	Statement::XA_EndStatement::setSuspensionMode --
//		SUSPEND (FOR MIGRATE) 指定の有無を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::XA_EndStatement::SuspensionMode::Value	v
//			設定する SUSPEND (FOR MIGRATE) 指定の有無を表す値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
XA_EndStatement::setSuspensionMode(SuspensionMode::Value v)
{
	_suspensionMode = v;
}

//	FUNCTION public
//	Statement::XA_EndStatement::getSuspensionMode --
//		SUSPEND (FOR MIGRATE) 指定の有無を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Statement::XA_EndStatement::SuspensionMode::Normal
//			SUSPEND が指定されていた
//		Statement::XA_EndStatement::SuspensionMode::ForMigrate
//			SUSPEND FOR MIGRATE が指定されていた
//		Statement::XA_EndStatement::SuspensionMode::Unknown
//			なにも指定されていなかった
//
//	EXCEPTIONS
//		なし

inline
XA_EndStatement::SuspensionMode::Value
XA_EndStatement::getSuspensionMode() const
{
	return _suspensionMode;
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_XA_ENDSTATEMENT_H

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
