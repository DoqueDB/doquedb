// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::LimitSpecification -- LimitSpecification
// 
// Copyright (c) 2004, 2010, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_LIMITSPECIFICATION_H
#define __SYDNEY_STATEMENT_LIMITSPECIFICATION_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class ValueExpression;
	
//
//	CLASS
//		LimitSpecification -- LimitSpecification
//
//	NOTES
//		LimitSpecification
//
class SYD_STATEMENT_FUNCTION LimitSpecification : public Statement::Object
{
public:
	//constructor
	LimitSpecification()
		: Object(ObjectType::LimitSpecification)
	{}
	// コンストラクタ
	LimitSpecification(ValueExpression* limit, ValueExpression* offset = 0);

	// Limit を得る
	ValueExpression* getLimit() const;
	// Limit を設定する
	void setLimit(ValueExpression* limit);

	// Offset を得る
	ValueExpression* getOffset() const;
	// Offset を設定する
	void setOffset(ValueExpression* offset);
   
	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

protected:
private:
	// 代入オペレーターは使わない
	LimitSpecification& operator=(const LimitSpecification& cOther_);

	// メンバ変数
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_LIMITSPECIFICATION_H

//
// Copyright (c) 2004, 2010, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
