// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::DropTableStatement -- DropTableStatement
// 
// Copyright (c) 1999, 2002, 2003, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_DROPTABLESTATEMENT_H
#define __SYDNEY_STATEMENT_DROPTABLESTATEMENT_H

#include "Statement/Object.h"

class ModUnicodeString;

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class Identifier;
	
//	CLASS
//	DropTableStatement --
//
//	NOTES

class SYD_STATEMENT_FUNCTION DropTableStatement : public Statement::Object
{
public:
	//constructor
	DropTableStatement()
		: Object(ObjectType::DropTableStatement)
	{}
	// コンストラクタ
	DropTableStatement( Identifier* pName_, bool bIfExists_ );
	// コンストラクタ (2)
	explicit DropTableStatement(const DropTableStatement& cOther_);

	// アクセサ
	// Name を得る
	Identifier* getName() const;
	// Name を設定する
	void setName(Identifier* pName_);
#ifdef OBSOLETE
	// Name を ModString で得る
	const ModUnicodeString* getNameString() const;
#endif

	// If exists?
	bool isIfExists() const;

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

private:
	// 代入オペレーターは使わない
	DropTableStatement& operator=(const DropTableStatement& cOther_);

	// true means drop only if the table exists
	bool m_bIfExists;
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_DROPTABLESTATEMENT_H

//
// Copyright (c) 1999, 2002, 2003, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
