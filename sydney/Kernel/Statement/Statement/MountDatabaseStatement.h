// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MountDatabaseStatement.h --
// 
// Copyright (c) 2000, 2002, 2004, 2009, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_MOUNTDATABASESTATEMENT_H
#define __SYDNEY_STATEMENT_MOUNTDATABASESTATEMENT_H

#include "Statement/Module.h"
#include "Statement/ObjectConnection.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class DatabaseCreateOptionList;
class Identifier;
class Literal;
class OptionalAreaParameter;

//	CLASS
//	Statement::MountDatabaseStatement --
//
//	NOTES

class MountDatabaseStatement
	: public ObjectConnection
{
public:
	struct Option
	{
		
	};

	//constructor
	MountDatabaseStatement()
		: ObjectConnection(ObjectType::MountDatabaseStatement)
	{}
	// コンストラクター
	SYD_STATEMENT_FUNCTION
	MountDatabaseStatement(
		Identifier* identifer, DatabaseCreateOptionList* optionList,
		OptionalAreaParameter* areaParameter,
		bool usingSnapshot, bool withRecovery, bool discardLogicalLog);
	// デストラクター
	virtual
	~MountDatabaseStatement();

	// データベース名を得る
	SYD_STATEMENT_FUNCTION
	Identifier*
	getDatabaseName() const;
	// 生成オプションを得る
	SYD_STATEMENT_FUNCTION
	DatabaseCreateOptionList*
	getOptionList() const;
	// エリア指定を得る
	SYD_STATEMENT_FUNCTION
	OptionalAreaParameter*
	getAreaParameter() const;
	// USING SNAPSHOT 指定の有無を得る
	bool
	SYD_STATEMENT_FUNCTION
	isUsingSnapshot() const;
	// WITH RECOVERY 指定の有無を得る
	SYD_STATEMENT_FUNCTION
	bool
	isWithRecovery() const;
	// DISCARD LOGICALLOG 指定の有無を得る
	SYD_STATEMENT_FUNCTION
	bool
	isDiscardLogicalLog() const;
	// MASTERのURLを得る
	SYD_STATEMENT_FUNCTION
	Literal*
	getMasterUrl() const;

	// 自分自身をコピーする
	Object*
	copy() const;

	// SQL文で得る
	ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;
	
#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

	// マスターURLを設定する
	void setMasterUrl(Literal* url_);

protected:
private:
	// データベース名を設定する
	void
	setDatabaseName(Identifier* p);
	// 生成オプションを設定する
	void
	setOptionList(DatabaseCreateOptionList* p);
	// エリア指定を設定する
	void
	setAreaParameter(OptionalAreaParameter* p);
	// USING SNAPSHOT 指定の有無を設定する
	void
	setUsingSnapshot(bool v);
	// WITH RECOVERY 指定の有無を設定する
	void
	setWithRecovery(bool v);
	// DISCARD LOGICALLOG 指定の有無を設定する
	void
	setDiscardLogicalLog(bool v);
};

//	FUNCTION public
//	Statement::MountDatabaseStatement::~MountDatabaseStatement --
//		デストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
MountDatabaseStatement::~MountDatabaseStatement()
{}

//	FUNCTION public
//	Statement::MountDatabaseStatement::copy -- 自分自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		コピーされた自分自身
//
//	EXCEPTIONS

inline
Object*
MountDatabaseStatement::copy() const
{
	return new MountDatabaseStatement(*this);
}

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_MOUNTDATABASESTATEMENT_H

//
//	Copyright (c) 2000, 2002, 2004, 2009, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
