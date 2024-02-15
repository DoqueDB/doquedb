// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::Identifier -- Identifier
// 
// Copyright (c) 1999, 2002, 2005, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_IDENTIFIER_H
#define __SYDNEY_STATEMENT_IDENTIFIER_H

#include "Statement/Object.h"
#include "Statement/Token.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

//
//	CLASS
//		Identifier -- Identifier
//
//	NOTES
//		Identifier
//
class SYD_STATEMENT_FUNCTION Identifier : public Statement::Object
{
public:
	// コンストラクタ (1)
	Identifier();
	// コンストラクタ (2)
	explicit Identifier(const Token& cToken_);
	// コピーコンストラクタ
	explicit Identifier(const Identifier& cOther_);
	// デストラクタ
	virtual ~Identifier();

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

	// Tokenを設定する
	void setToken(const Token& cToken_);
	// Tokenを得る
	const Token& getToken() const;

	// 名前を得る
	const ModUnicodeString* getIdentifier() const;

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

protected:
private:
	// 代入オペレーターは使わない
	Identifier& operator=(const Identifier& cOther_);

	void setStringFromToken(const Token& cToken_);
	void setTokenFromString();

	// メンバ変数
	Token m_cToken;
	ModUnicodeString m_cstrIdentifier;
	ModUnicodeString m_cstrCopy; // copy, serialize用
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_IDENTIFIER_H

//
// Copyright (c) 1999, 2002, 2005, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
