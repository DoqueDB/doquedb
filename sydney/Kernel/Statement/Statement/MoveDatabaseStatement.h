// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MoveDatabaseStatement.h --
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

#ifndef __SYDNEY_STATEMENT_MOVEDATABASESTATEMENT_H
#define __SYDNEY_STATEMENT_MOVEDATABASESTATEMENT_H

#include "Statement/Identifier.h"
#include "Statement/DatabasePathElementList.h"
#include "Statement/ObjectConnection.h"

_SYDNEY_BEGIN

namespace Statement
{

	class Identifier;
	class DatabasePathElementList;

//
//	CLASS
//	Statement::MoveDatabaseStatement --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION MoveDatabaseStatement  : public Statement::ObjectConnection
{
private:
	// メンバのm_vecpElements内でのindex
	enum { f_Name, f_Type, f_Path, f__end_index };
public:
	//constructor
	MoveDatabaseStatement()
		: ObjectConnection(ObjectType::MoveDatabaseStatement)
	{}
	//コンストラクタ(2)
	MoveDatabaseStatement(Identifier* pId_ ,DatabasePathElementList* pPath_ ,int iType_);

	//デストラクタ
	virtual ~MoveDatabaseStatement();

	// アクセサ
	enum MoveType {
		Unknown = 0
		,Set
		,Drop
	};

	//データベース名を得る
	Identifier* getDatabaseName() const
		{ return dynamic_cast<Identifier*>(ObjectConnection::getObject(f_Name)); }
	//データベース名設定
	void setDatabaseName(Identifier* pId_)
		{ ObjectConnection::setObject(f_Name ,pId_); }

	// PathList を得る
	DatabasePathElementList* getPathList() const
		{ return dynamic_cast<DatabasePathElementList*>(ObjectConnection::getObject(f_Path)); }
	// Path を設定する
	void setPathList(DatabasePathElementList* pPath_)
		{ ObjectConnection::setObject(f_Path ,pPath_); }

	// Type を得る
	int	getType() const
		{ return ObjectConnection::getScaler(f_Type); }
	// Type を設定する
	void setType(int iValue_)
		{ ObjectConnection::setScaler(f_Type ,iValue_); }

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
	MoveDatabaseStatement& operator=(const MoveDatabaseStatement& cOther_);

};

}

_SYDNEY_END

#endif //__SYDNEY_STATEMENT_MOVEDATABASESTATEMENT_H

//
//	Copyright (c) 2000, 2002, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
