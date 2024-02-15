// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::TableDefinition -- TableDefinition
// 
// Copyright (c) 1999, 2002, 2003, 2008, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_TABLEDEFINITION_H
#define __SYDNEY_STATEMENT_TABLEDEFINITION_H

#include "Statement/Object.h"


_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class Hint;
	class Identifier;
	class TableElementList;
	class DataValue;
	class AreaOption;

//	CLASS
//	TableDefinition -- TableDefinition
//
//	NOTES

class SYD_STATEMENT_FUNCTION TableDefinition : public Statement::Object
{
public:
	//constructor
	TableDefinition()
		: Object(ObjectType::TableDefinition)
	{}
	// コンストラクタ (2)
	TableDefinition(Identifier* pName_, int iScope_, int iConstUpdate_, 
					TableElementList* pElements_, DataValue* pInitialValue_, 
					Hint* pHint_, AreaOption* pAreaOpt_);

	// アクセサ
	// Name を得る
	Identifier* getName() const;
	// Name を設定する
	void setName(Identifier* pName_);
#ifdef OBSOLETE
	// Name を ModUnicodeString で得る
	const ModUnicodeString* getNameString() const;
#endif

	// Scopeを得る
	int getScope() const;
	void setScope(int iScope_);
	//
	//  Enum global
	//  Scope -- Scopeの値
	//
	//  NOTES
	//  Scopeの値
	//
	enum Scope {
		Permanent = 0,			// 永続的
		LocalTemporary,			// セッション内一時テーブル
		GlobalTemporary			// 大域的一時テーブル
	};

#ifdef OBSOLETE
	// ConstUpdate を得る
	int getConstUpdate() const;
#endif
	// ConstUpdate を設定する
	void setConstUpdate(int iConstUpdate_);
	//
	//  Enum global
	//  ConstUpdate -- ConstUpdateの値
	//
	//  NOTES
	//  ConstUpdateの値
	//
	enum ConstUpdate {
		Updatable = 0,
		Constant
	};

	// Elements を得る
	TableElementList* getElements() const;
	// Elements を設定する
	void setElements(TableElementList* pElements_);

#ifdef OBSOLETE
	// InitialValueを得る
	DataValue* getInitialValue() const;
#endif
	// InitialValueを設定する
	void setInitialValue(DataValue* pInitialValue_);

	// Hint を得る
	Hint* getHint() const;
	// Hint を設定する
	void setHint(Hint* pHint_);

	// AreaOption を得る
	AreaOption* getAreaOption() const;
	// AreaOption を設定する
	void setAreaOption(AreaOption* pAreaOpt_);

	//自身をコピーする
	Object* copy() const;

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getOldAnalyzer() const;
#endif

private:
	// 代入オペレーターは使わない
	TableDefinition& operator=(const TableDefinition& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_TABLEDEFINITION_H

//
// Copyright (c) 1999, 2002, 2003, 2008, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
