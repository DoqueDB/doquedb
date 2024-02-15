// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::Literal -- Literal
// 
// Copyright (c) 1999, 2002, 2004, 2005, 2007, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_LITERAL_H
#define __SYDNEY_STATEMENT_LITERAL_H

#include "Statement/Module.h"
#include "Statement/Object.h"
#include "Statement/Token.h"

#include "Common/Data.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN
	
//
//	CLASS
//		Literal --
//
//	NOTES
//		Literalを表すオブジェクト
//		SQLParserに渡されたModUnicodeStringオブジェクトが破棄されるまで有効
//
class SYD_STATEMENT_FUNCTION Literal : public Statement::Object
{
public:
	typedef Object Super;

	//constructor
	Literal()
		: Object(ObjectType::Literal)
	{}
 	// コンストラクタ
	explicit Literal(const Token& cToken_);
	// コピーコンストラクタ
	explicit Literal(const Literal& cOther_);

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

	// 文字列Literalか
	bool isStringLiteral() const;

	// アクセサ

	// Token を得る
	const Token& getToken() const;
	// Token をセットする
	void setToken(const Token& cToken_);

	// Token が表すデータを作る
	Common::Data::Pointer createData(Common::DataType::Type eType_ = Common::DataType::Undefined,
									 bool bForAssign_ = true) const;

	// set a value expression as included in parenthesis
	void setSeparated();
	// get whether a value expression as included in parenthesis
	bool isSeparated() const;

	//自身をコピーする
	Object* copy() const;

#ifdef USE_OLDER_VERSION
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

protected:
private:
	// 代入オペレーターは使わない
	Literal& operator=(const Literal& cOther_);

	// Tokenから文字列を作る
	void setLiteralFromToken(const Token& cToken_);
	// 文字列からTokenを作る
	void setTokenFromLiteral();

	// リテラルに対応するトークン
	Token m_cToken;
	// 文字列表現
	mutable ModUnicodeString m_cstrLiteral;
	// Flag whether the literal is include in parenthesis
	bool m_bSeparated;
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_LITERAL_H

//
// Copyright (c) 1999, 2002, 2004, 2005, 2007, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
