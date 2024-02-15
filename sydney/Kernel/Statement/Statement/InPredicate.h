// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InPredicate.h -- IN 述語関連のクラス定義、関数宣言
// 
// Copyright (c) 2004, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_INPREDICATE_H
#define __SYDNEY_STATEMENT_INPREDICATE_H

#include "Statement/Module.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class ValueExpression;

//	CLASS
//	Statement::InPredicate -- IN 述語を表すクラス
//
//	NOTES

class SYD_STATEMENT_FUNCTION InPredicate
	: public Object
{
public:
	//constructor
	InPredicate()
		: Object(ObjectType::InPredicate)
	{}
	// コンストラクタ
	InPredicate(ValueExpression* row, Object* inValue);

	// 対象のRow型を得る
	ValueExpression* getLeft() const;
	// 対象のRow型を設定する
	void setLeft(ValueExpression* row);

	// 照合対象を得る
	Object* getRight() const;
	// 照合対象を設定する
	void setRight(Object* inValue);

	// 自分をコピーする
	Object* copy() const;

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

#ifdef USE_OLDER_VERSION
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_INPREDICATE_H

//
// Copyright (c) 2004, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
