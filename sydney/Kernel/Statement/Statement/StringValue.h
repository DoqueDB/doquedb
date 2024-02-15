// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::StringValue -- 文字列値
// 
// Copyright (c) 1999, 2002, 2003, 2004, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_STRINGVALUE_H
#define __SYDNEY_STATEMENT_STRINGVALUE_H

#include "Statement/Object.h"
#include "ModUnicodeString.h"
#include "ModUnicodeOstrStream.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

//	CLASS
//	StringValue -- 文字列値
//
//	NOTES

class SYD_STATEMENT_FUNCTION StringValue : public Statement::Object
{
public:
	typedef Object Super;

	// コンストラクタ (1)
	StringValue();
	// コンストラクタ (2)
	explicit StringValue(const ModUnicodeString& cstrValue_);
#ifdef OBSOLETE
	// コンストラクタ (3)
	explicit StringValue(const ModUnicodeChar* pszValue_);
#endif
	// コピーコンストラクタ
	explicit StringValue(const StringValue& cOther_);
	// デストラクタ
	virtual ~StringValue();

#ifndef SYD_COVERAGE
	// 文字列化
	ModUnicodeString toString() const;
	// LISP形式で出力する
	void toString(ModUnicodeOstrStream& cStream_, int iIndent_ = 0) const;
#endif

	// 値を得る
	const ModUnicodeString* getValue() const;
	// 値を設定する
	void setValue(const ModUnicodeString& cstrValue_);

	//自身をコピーする
	Object* copy() const;

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

protected:
#ifdef OBSOLETE // 現在のところStatement::Objectをマップに登録することはない
	// ハッシュコードを計算する
	virtual ModSize getHashCode();

	// 同じ型のオブジェクト同士でless比較する
	virtual bool compare(const Object& cObj_) const;
#endif

private:
	// 代入オペレーターは使わない
	StringValue& operator=(const StringValue& cOther_);

	// メンバ変数
	ModUnicodeString m_cstrValue;
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_STRINGVALUE_H

//
// Copyright (c) 1999, 2002, 2003, 2004, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
