// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IsolationLevel.h --
// 
// Copyright (c) 2000, 2002, 2003, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_ISOLATIONLEVEL_H
#define __SYDNEY_STATEMENT_ISOLATIONLEVEL_H

#include "Common/Common.h"
#include "Statement/TransactionMode.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class IntegerValue;

//	CLASS
//	Statement::IsolationLevel --
//
//	NOTES

class SYD_STATEMENT_FUNCTION IsolationLevel  : public Statement::Object
{
public:
	//constructor
	IsolationLevel()
		: Object(ObjectType::IsolationLevel)
	{}
	//コンストラクタ(2)
	IsolationLevel(const IntegerValue* pcLevel_);
	//デストラクタ
	virtual ~IsolationLevel();

	//
	//	アクセサ
	//

	//アイソレーションレベル
	void setIsoLevel(const IntegerValue* pcLevel_);
	Statement::IntegerValue* getIsoLevel() const;

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

private:
	// 代入オペレーターは使わない
	IsolationLevel& operator=(const IsolationLevel& cOther_);
};

//	FUNCTION public
//	Statement::IsolationLevel::copy -- 自身をコピーする
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
IsolationLevel::copy() const
{
	return new IsolationLevel(*this);
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_ISOLATIONLEVEL_H

//
//	Copyright (c) 2000, 2002, 2003, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
