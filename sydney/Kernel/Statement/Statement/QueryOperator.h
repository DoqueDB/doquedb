// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::QueryOperator -- QueryOperator
// 
// Copyright (c) 1999, 2002, 2003, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_QUERYOPERATOR_H
#define __SYDNEY_STATEMENT_QUERYOPERATOR_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class ColumnNameList;
	
//
//	CLASS
//		QueryOperator -- QueryOperator
//
//	NOTES
//		QueryOperator
//
class SYD_STATEMENT_FUNCTION QueryOperator : public Statement::Object
{
public:
	//constructor
	QueryOperator()
		: Object(ObjectType::QueryOperator)
	{}
	// コンストラクタ (2)
	explicit QueryOperator(int iSetOperatorType_, int iAll_, ColumnNameList* pCorrespondingSpec_);

	// アクセサ
	// SetOperatorType を得る
	int getSetOperatorType() const;
	// SetOperatorType を設定する
	void setSetOperatorType(int iSetOperatorType_);
	//
	enum SetOperatorType {
		None = 0,
		Union,
		Except,
		Intersect
	};

	// All を得る
	int getAll() const;
	// All を設定する
	void setAll(int iAll_);

	// CorrespondingSpec を得る
	ColumnNameList* getCorrespondingSpec() const;
	// CorrespondingSpec を設定する
	void setCorrespondingSpec(ColumnNameList* pCorrespondingSpec_);

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

private:
	// 代入オペレーターは使わない
	QueryOperator& operator=(const QueryOperator& cOther_);

	// メンバ変数
};

//	FUNCTION public
//	Statement::QueryOperator::copy -- 自身をコピーする
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
QueryOperator::copy() const
{
	return new QueryOperator(*this);
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_QUERYOPERATOR_H

//
// Copyright (c) 1999, 2002, 2003, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
