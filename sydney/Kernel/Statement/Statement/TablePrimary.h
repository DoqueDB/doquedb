// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::TablePrimary -- TablePrimary
// 
// Copyright (c) 1999, 2002, 2003, 2004, 2006, 2008, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_TABLEPRIMARY_H
#define __SYDNEY_STATEMENT_TABLEPRIMARY_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class ColumnNameList;
class Identifier;
class QueryExpression;
class ValueExpression;

//
//	CLASS
//		TablePrimary -- TablePrimary
//
//	NOTES
//		TablePrimary
//
class SYD_STATEMENT_FUNCTION TablePrimary : public Statement::Object
{
public:
	typedef Object Super;
	
	struct Type
	{
		enum Value {
			Unknown = 0,
			Table,
			DerivedTable,
			JoinedTable,
			Bulk,
			UnnestTable,
			ValueNum
		};
	};

	// コンストラクタ (1)
	TablePrimary();
	// コンストラクタ (2)
	TablePrimary(Identifier* pReference_, Identifier* pCorrelationName_, ColumnNameList* pDerivedColumnList_);
	// コンストラクタ (3)
	TablePrimary(QueryExpression* pQuery_, Identifier* pCorrelationName_, ColumnNameList* pDerivedColumnList_);
	// コンストラクタ (4)
	TablePrimary(Type::Value eType_, Object* pJoinedTable_);
	// コンストラクタ (5)
	TablePrimary(ValueExpression* pValueExpression_,
				 Identifier* pCorrelationName_,
				 ColumnNameList* pDerivedColumnList_);
	// コピーコンストラクタ
	TablePrimary(const TablePrimary& cOther_);

	// アクセサ
	// TablePrimaryの種別
	Type::Value getTablePrimaryType() const;

	// Reference を得る
	Identifier* getReference() const;
	// Reference を設定する
	void setReference(Identifier* pReference_);
	// Reference を ModUnicodeString で得る
	const ModUnicodeString* getReferenceString() const;

	// CorrelationName を得る
	Identifier* getCorrelationName() const;
	// CorrelationName を設定する
	void setCorrelationName(Identifier* pCorrelationName_);
	// CorrelationName を ModUnicodeString で得る
	const ModUnicodeString* getCorrelationNameString() const;

	// DerivedColumnList を得る
	ColumnNameList* getDerivedColumnList() const;
	// DerivedColumnList を設定する
	void setDerivedColumnList(ColumnNameList* pDerivedColumnList_);

	// Query を得る
	Object* getQuery() const;
	// Query を設定する
	void setQuery(Object* pQuery_);

	// Collection を得る
	ValueExpression* getCollection() const;
	// Collection を設定する
	void setCollection(ValueExpression* pCollection_);

	//自身をコピーする
	Object* copy() const;

	// Analyzerを得る
#ifdef USE_OLDER_VERSION
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

protected:
#ifdef OBSOLETE // 現在のところStatement::Objectをマップに登録することはない
	// ハッシュコードを計算する
	virtual ModSize getHashCode();

	// 同じ型のオブジェクト同士でless比較する
	virtual bool compare(const Object& cObj_) const;
#endif

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

private:
	// 代入オペレーターは使わない
	TablePrimary& operator=(const TablePrimary& cOther_);

	Type::Value m_eType;
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_TABLEPRIMARY_H

//
// Copyright (c) 1999, 2002, 2003, 2004, 2006, 2008, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
