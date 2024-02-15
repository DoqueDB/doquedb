// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::ColumnName -- ColumnName
// 
// Copyright (c) 1999, 2002, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_COLUMNNAME_H
#define __SYDNEY_STATEMENT_COLUMNNAME_H

#include "Statement/Object.h"

class ModUnicodeString;

_SYDNEY_BEGIN

namespace Statement
{
	class Identifier;
	
//
//	CLASS
//		ColumnName -- ColumnName
//
//	NOTES
//		ColumnName
//
class SYD_STATEMENT_FUNCTION ColumnName : public Statement::Object
{
public:
	//constructor
	ColumnName()
		: Object(ObjectType::ColumnName)
	{}
	// コンストラクタ (2)
	explicit ColumnName(Identifier* pIdentifier_);

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

	// アクセサ
	// Identifier を得る
	Identifier* getIdentifier() const;
	// Identifier を設定する
	void setIdentifier(Identifier* pIdentifier_);
	// Identifier を ModString で得る
	const ModUnicodeString* getIdentifierString() const;

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

protected:
private:
	// 代入オペレーターは使わない
	ColumnName& operator=(const ColumnName& cOther_);

	// メンバ変数
};

} // namescape Statement

_SYDNEY_END

#endif // __SYDNEY_STATEMENT_COLUMNNAME_H

//
// Copyright (c) 1999, 2002, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
