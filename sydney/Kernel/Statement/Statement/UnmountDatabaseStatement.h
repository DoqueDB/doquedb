// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UnmountDatabaseStatement.h --
// 
// Copyright (c) 2000, 2002, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_UNMOUNTDATABASESTATEMENT_H
#define __SYDNEY_STATEMENT_UNMOUNTDATABASESTATEMENT_H

#include "Statement/ObjectConnection.h"
#include "Statement/Identifier.h"

_SYDNEY_BEGIN

namespace Statement
{

	class Identifier;

//
//	CLASS
//	Statement::UnmountDatabaseStatement --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION UnmountDatabaseStatement  : public Statement::ObjectConnection
{
private:
	// メンバのm_vecpElements内でのindex
	enum { f_Name, f__end_index };
public:
	//constructor
	UnmountDatabaseStatement()
		: ObjectConnection(ObjectType::UnmountDatabaseStatement)
	{}
	//コンストラクタ(2)
	UnmountDatabaseStatement(Identifier* pId_);

	//デストラクタ
	virtual ~UnmountDatabaseStatement();

	// アクセサ

	//データベース名を得る
	Identifier* getDatabaseName() const
		{ return dynamic_cast<Identifier*>(ObjectConnection::getObject(f_Name)); }
	//データベース名設定
	void setDatabaseName(Identifier* pId_)
		{ ObjectConnection::setObject(f_Name ,pId_); }

	//自身をコピーする
	Object* copy() const;

	// SQL文で得る
	ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;
	
#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

protected:
private:
	// 代入オペレーターは使わない
	UnmountDatabaseStatement& operator=(const UnmountDatabaseStatement& cOther_);

};

}

_SYDNEY_END

#endif //__SYDNEY_STATEMENT_UNMOUNTDATABASESTATEMENT_H

//
//	Copyright (c) 2000, 2002, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
