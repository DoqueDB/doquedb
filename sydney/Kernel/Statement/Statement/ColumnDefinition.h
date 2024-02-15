// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::ColumnDefinition -- ColumnDefinition
// 
// Copyright (c) 1999, 2002, 2003, 2004, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_COLUMNDEFINITION_H
#define __SYDNEY_STATEMENT_COLUMNDEFINITION_H

#include "Statement/Object.h"
#include "Common/SQLData.h"

class ModUnicodeString;

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class Hint;
	class Identifier;
	class ValueExpression;
	class ColumnConstraintDefinitionList;
	
//
//	CLASS
//		ColumnDefinition -- ColumnDefinition
//
//	NOTES
//		ColumnDefinition
//
class SYD_STATEMENT_FUNCTION ColumnDefinition : public Statement::Object
{
public:
	//constructor
	ColumnDefinition()
		: Object(ObjectType::ColumnDefinition)
	{}
	// コンストラクタ (2)
	ColumnDefinition(Identifier* pName_, Common::SQLData* pDataType_,
					 ValueExpression* pDefaultValue_, ColumnConstraintDefinitionList* pConstraints_,
					 ValueExpression* pConstValue_, Hint* pHint_, bool bUseOnUpdate_);
	// コピーコンストラクタ
	ColumnDefinition(const ColumnDefinition& cOther_);

	// アクセサ
	// Name を得る
	Identifier* getName() const;
	// Name を設定する
	void setName(Identifier* pName_);
#ifdef OBSOLETE
	// Name を ModUnicodeString で得る
	const ModUnicodeString* getNameString() const;
#endif

	// DataType を得る
	const Common::SQLData& getDataType() const;
	// DataType を設定する
	void setDataType(const Common::SQLData& cDataType_);

	// DefaultValue を得る
	ValueExpression* getDefaultValue() const;
	// DefaultValue を設定する
	void setDefaultValue(ValueExpression* pDefaultValue_);

	// ConstValue を得る
	ValueExpression* getConstValue() const;
	// ConstValue を設定する
	void setConstValue(ValueExpression* pConstValue_);

	// Hint を得る
	Hint* getHint() const;
	// Hint を設定する
	void setHint(Hint* pHint_);

	// Constraint を得る
	ColumnConstraintDefinitionList* getConstraints() const;
	// Constraint を設定する
	void setConstraints(ColumnConstraintDefinitionList* pConstraints_);

	// UseOnUpdateを得る
	bool isUseOnUpdate() const;
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
	ColumnDefinition& operator=(const ColumnDefinition& cOther_);

	Common::SQLData m_cDataType;
	bool m_bUseOnUpdate;
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_COLUMNDEFINITION_H

//
// Copyright (c) 1999, 2002, 2003, 2004, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
