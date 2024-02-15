// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::ColumnConstraintDefinition -- ColumnConstraintDefinition
// 
// Copyright (c) 2003, 2004, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_COLUMNCONSTRAINTDEFINITION_H
#define __SYDNEY_STATEMENT_COLUMNCONSTRAINTDEFINITION_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN
	
//
//	CLASS
//		ColumnConstraintDefinition -- ColumnConstraintDefinition
//
//	NOTES
//		ColumnConstraintDefinition
//
class SYD_STATEMENT_FUNCTION ColumnConstraintDefinition : public Statement::Object
{
public:
	//constructor
	ColumnConstraintDefinition()
		: Object(ObjectType::ColumnConstraintDefinition)
	{}
	// コンストラクタ (2)
	ColumnConstraintDefinition(int iConstraintType_);

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
		NotNull,
		PrimaryKey,
		Unique
		// ForeignKey,
	};

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
	ColumnConstraintDefinition& operator=(const ColumnConstraintDefinition& cOther_);

	// メンバ変数
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_COLUMNCONSTRAINTDEFINITION_H

//
// Copyright (c) 2003, 2004, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
