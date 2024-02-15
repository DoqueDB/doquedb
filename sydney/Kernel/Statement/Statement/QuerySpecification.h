// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::QuerySpecification -- QuerySpecification
// 
// Copyright (c) 1999, 2002, 2006, 2008, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_QUERYSPECIFICATION_H
#define __SYDNEY_STATEMENT_QUERYSPECIFICATION_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class SelectList;
class SortSpecificationList;
class TableExpression;
class BulkSpecification;
class SelectTargetList;
	
//
//	CLASS
//		QuerySpecification -- QuerySpecification
//
//	NOTES
//		QuerySpecification
//
class SYD_STATEMENT_FUNCTION QuerySpecification : public Statement::Object
{
public:
	//constructor
	QuerySpecification()
		: Object(ObjectType::QuerySpecification)
	{}
	// コンストラクタ (2)
	QuerySpecification(int iQuantifier_, SelectList* pSelectList_, TableExpression* pTable_,
					   BulkSpecification* pOutput_, SelectTargetList* pSelectTargetList_ = 0);

	// アクセサ
	// Quantifier を得る
	int getQuantifier() const;
	// Quantifier を設定する
	void setQuantifier(int iQuantifier_);

	// SelectList を得る
	SelectList* getSelectList() const;
	// SelectList を設定する
	void setSelectList(SelectList* pSelectList_);

	// Table を得る
	TableExpression* getTable() const;
	// Table を設定する
	void setTable(TableExpression* pTable_);

	// BulkSpecification を得る
	BulkSpecification* getOutput() const;
	// BulkSpecification を設定する
	void setOutput(BulkSpecification* pOutput_);

	// SelectTargetList を得る
	SelectTargetList* getSelectTargetList() const;
	// SelectTargetList を設定する
	void setSelectTargetList(SelectTargetList* pSelectTargetList_);
	
	//自身をコピーする
	Object* copy() const;

	// Analyzerを得る
#ifdef USE_OLDER_VERSION
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

protected:
private:
	// 代入オペレーターは使わない
	QuerySpecification& operator=(const QuerySpecification& cOther_);

	// メンバ変数
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_QUERYSPECIFICATION_H

//
// Copyright (c) 1999, 2002, 2006, 2008, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
