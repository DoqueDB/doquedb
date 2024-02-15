// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::TableConstraintDefinition -- TableConstraintDefinition
// 
// Copyright (c) 1999, 2002, 2004, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_TABLECONSTRAINTDEFINITION_H
#define __SYDNEY_STATEMENT_TABLECONSTRAINTDEFINITION_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class ColumnNameList;
class Hint;
class Identifier;
	
//
//	CLASS
//		TableConstraintDefinition -- TableConstraintDefinition
//
//	NOTES
//		TableConstraintDefinition
//
class SYD_STATEMENT_FUNCTION TableConstraintDefinition : public Statement::Object
{
public:
	//constructor
	TableConstraintDefinition()
		: Object(ObjectType::TableConstraintDefinition)
	{}
	// コンストラクタ (2)
	TableConstraintDefinition(int iConstraintType_, ColumnNameList* pColumnNameList_, int iClustered_,
							  Hint* pHint_,
							  Identifier* pReferedTableName_ = 0,
							  ColumnNameList* pReferedColumnName_ = 0);

	// アクセサ
	// ConstraintType を得る
	int getConstraintType() const;
	// ConstraintType を設定する
	void setConstraintType(int iConstraintType_);
	//
	//  Enum global
	//  ConstraintType -- ConstraintTypeの値
	//
	//  NOTES
	//  ConstraintTypeの値
	//
	enum ConstraintType {
		None = 0,
		PrimaryKey,
		Unique,
		ForeignKey
		// References,
	};

	// ColumnNameList を得る
	ColumnNameList* getColumnNameList() const;
	// ColumnNameList を設定する
	void setColumnNameList(ColumnNameList* pColumnNameList_);

	// Clustered を得る
	int getClustered() const;
	// Clustered を設定する
	void setClustered(int iClustered_);

	// Hint を得る
	Hint* getHint() const;
	// Hint を設定する
	void setHint(Hint* pHint_);

	// ReferedTableName を得る
	Identifier* getReferedTableName() const;
	// ReferedTableName を設定する
	void setReferedTableName(Identifier* pTableName_);

	// ReferedColumnName を得る
	ColumnNameList* getReferedColumnName() const;
	// ReferedColumnName を設定する
	void setReferedColumnName(ColumnNameList* pColumnName_);

	//自身をコピーする
	Object* copy() const;

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

protected:
private:
	// 代入オペレーターは使わない
	TableConstraintDefinition& operator=(const TableConstraintDefinition& cOther_);

	// メンバ変数
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_TABLECONSTRAINTDEFINITION_H

//
// Copyright (c) 1999, 2002, 2004, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
