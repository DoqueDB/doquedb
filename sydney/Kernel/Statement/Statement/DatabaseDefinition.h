// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DatabaseDefinition.h --
// 
// Copyright (c) 2000, 2002, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_DATABASEDEFINITION_H
#define __SYDNEY_STATEMENT_DATABASEDEFINITION_H

#include "Common/Common.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN

namespace Statement
{

	class Identifier;
	class Hint;
	class DatabaseCreateOptionList;
	class DatabasePathElement;
	class Literal;

//
//	CLASS
//	Statement::DatabaseDefinition --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION DatabaseDefinition  : public Statement::Object
{
public:
	//constructor
	DatabaseDefinition()
		: Object(ObjectType::DatabaseDefinition)
	{}
	//コンストラクタ(2)
	DatabaseDefinition(Identifier* pId_, Hint* pHint_, DatabaseCreateOptionList* pOption_);
	//デストラクタ
	virtual ~DatabaseDefinition();

	// アクセサ

	//データベース名を得る
	Identifier* getDatabaseName() const;
	//データベース名設定
	void setDatabaseName(Identifier* pId_);

	// Hint を得る
	Hint*	getHint() const;
	// Hint を設定する
	void setHint(Hint* pHint_);

	// OptionList を得る
	DatabaseCreateOptionList* getOptionList() const;
	// OptionList を設定する
	void setOptionList(DatabaseCreateOptionList* pAreaOpt_);

	//自身をコピーする
	Object* copy() const;

	// SQL文で得る
	ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

protected:
private:
	// 代入オペレーターは使わない
	DatabaseDefinition& operator=(const DatabaseDefinition& cOther_);

};

}

_SYDNEY_END

#endif //__SYDNEY_STATEMENT_DATABASEDEFINITION_H

//
//	Copyright (c) 2000, 2002, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
