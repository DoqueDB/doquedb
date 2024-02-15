// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::InsertColumnsAndSource -- InsertColumnsAndSource
// 
// Copyright (c) 1999, 2002, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_INSERTCOLUMNSANDSOURCE_H
#define __SYDNEY_STATEMENT_INSERTCOLUMNSANDSOURCE_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class ColumnNameList;
	class QueryExpression;
	
//
//	CLASS
//		InsertColumnsAndSource -- InsertColumnsAndSource
//
//	NOTES
//		InsertColumnsAndSource
//
class SYD_STATEMENT_FUNCTION InsertColumnsAndSource : public Statement::Object
{
public:
	// コンストラクタ (1)
	InsertColumnsAndSource();
	// コンストラクタ (2)
	explicit InsertColumnsAndSource(QueryExpression* pQuery_, ColumnNameList* pColumnList_);

	// アクセサ
	// Query を得る
	QueryExpression* getQuery() const;
	// Query を設定する
	void setQuery(QueryExpression* pQuery_);

	// ColumnList を得る
	ColumnNameList* getColumnList() const;
	// ColumnList を設定する
	void setColumnList(ColumnNameList* pColumnList_);

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

protected:
private:
	// 代入オペレーターは使わない
	InsertColumnsAndSource& operator=(const InsertColumnsAndSource& cOther_);

	// メンバ変数
};

//	FUNCTION public
//	Statement::InsertColumnsAndSource::copy -- 自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Statement::Object*
//
//	EXCEPTIONS

inline
Object*
InsertColumnsAndSource::copy() const
{
	return new InsertColumnsAndSource(*this);
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_INSERTCOLUMNSANDSOURCE_H

//
// Copyright (c) 1999, 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
