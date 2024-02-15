// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TransactionMode.h --
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

#ifndef __SYDNEY_STATEMENT_TRANSACTIONMODE_H
#define __SYDNEY_STATEMENT_TRANSACTIONMODE_H

#include "Common/Common.h"
#include "Statement/Object.h"


_SYDNEY_BEGIN

namespace Statement
{

class IntegerValue;

//
//	CLASS
//	Statement::TransactionMode --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION TransactionMode : public Statement::Object
{
public:
	//constructor
	TransactionMode()
		: Object(ObjectType::TransactionMode)
	{}
	//コンストラクタ(2)
	TransactionMode(const IntegerValue* pcAccMode_,
					const IntegerValue* pcIsoLevel_,
					bool  bUsingSnapshot_);
	//デストラクタ
	virtual ~TransactionMode();

	//アクセスモード
	enum AccessMode
	{
		AccUnknown,
		ReadOnly,
		ReadWrite
	};

	//アイソレーションレベル
	enum IsolationLevel
	{
		IsoUnknown,
		ReadUncommitted,
		ReadCommitted,
		RepeatableRead,
		Serializable
	};

	//
	//	アクセサ
	//

	//アクセスモード
	void setAccMode(const IntegerValue* pcAccMode_);
	int getAccMode() const;

	//アイソレーションレベル
	void setIsoLevel(const IntegerValue* pcAccMode_);
	int getIsoLevel() const;

	// 版管理使用フラグ
	void setUsingSnapshot(bool bUsingSnapshot_);
	bool isUsingSnapshot() const;

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

private:
	// 代入オペレーターは使わない
	TransactionMode& operator=(const TransactionMode& cOther_);

};

}

_SYDNEY_END

#endif //__SYDNEY_STATEMENT_TRANSACTIONMODE_H

//
//	Copyright (c) 2000, 2002, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
