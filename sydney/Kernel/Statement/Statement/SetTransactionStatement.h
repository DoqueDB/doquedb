// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SetTransactionStatement.h --
// 
// Copyright (c) 2000, 2002, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_SETTRANSACTIONSTATEMENT_H
#define __SYDNEY_STATEMENT_SETTRANSACTIONSTATEMENT_H

#include "Common/Common.h"
#include "Statement/Object.h"


_SYDNEY_BEGIN

namespace Statement
{

class TransactionModeList;
class TransactionMode;

//
//	CLASS
//	Statement::SetTransactionStatement --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION SetTransactionStatement	: public Statement::Object
{
public:
	//constructor
	SetTransactionStatement()
		: Object(ObjectType::SetTransactionStatement)
	{}
	//コンストラクタ(2)
	SetTransactionStatement(const TransactionModeList* pcTransMode_);
	//デストラクタ
	virtual ~SetTransactionStatement();

	//
	//	アクセサ
	//

	void setTransactMode(
		const TransactionModeList* pcTransModeList_);

	//トランザクションモード
	void setTransactMode(const TransactionMode* pcTransMode_);
	TransactionMode* getTransactMode() const;

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

private:
	// 代入オペレーターは使わない
	SetTransactionStatement& operator=(const SetTransactionStatement& cOther_);

};

}

_SYDNEY_END

#endif //__SYDNEY_STATEMENT_SETTRANSACTIONSTATEMENT_H

//
//	Copyright (c) 2000, 2002, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
