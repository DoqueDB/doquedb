// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::ReturnStatement -- ReturnStatement
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_RETURNSTATEMENT_H
#define __SYDNEY_STATEMENT_RETURNSTATEMENT_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class ValueExpression;
	
//
//	CLASS
//		ReturnStatement -- ReturnStatement
//
//	NOTES
//		ReturnStatement
//
class SYD_STATEMENT_FUNCTION ReturnStatement : public Statement::Object
{
public:
	//constructor
	ReturnStatement()
		: Object(ObjectType::ReturnStatement)
	{}
	// コンストラクタ (2)
	ReturnStatement(ValueExpression* pValue_);

	// アクセサ
	// Return valueを得る
	ValueExpression* getValue() const;
	// Return valueを設定する
	void setValue(ValueExpression* pValue_);

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

	//自身をコピーする
	Object* copy() const;

	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

protected:
private:
	// 代入オペレーターは使わない
	ReturnStatement& operator=(const ReturnStatement& cOther_);

	// メンバ変数
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_RETURNSTATEMENT_H

//
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
