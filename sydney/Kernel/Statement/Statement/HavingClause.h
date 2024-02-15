// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::HavingClause -- HavingClause
// 
// Copyright (c) 1999, 2002, 2003, 2010, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_HAVINGCLAUSE_H
#define __SYDNEY_STATEMENT_HAVINGCLAUSE_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class ValueExpression;
	
//	CLASS
//		HavingClause -- HavingClause
//
//	NOTES

class SYD_STATEMENT_FUNCTION HavingClause : public Statement::Object
{
public:
	//constructor
	HavingClause()
		: Object(ObjectType::HavingClause)
	{}
	// コンストラクタ (2)
	explicit HavingClause(ValueExpression* pCondition_);

	// アクセサ
	// Condition を得る
	ValueExpression* getCondition() const;
	// Condition を設定する
	void setCondition(ValueExpression* pCondition_);

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

private:
	// 代入オペレーターは使わない
	HavingClause& operator=(const HavingClause& cOther_);

	// メンバ変数
};

//	FUNCTION public
//	Statement::HavingClause::copy -- 自身をコピーする
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
HavingClause::copy() const
{
	return new HavingClause(*this);
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_HAVINGCLAUSE_H

//
// Copyright (c) 1999, 2002, 2003, 2010, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
