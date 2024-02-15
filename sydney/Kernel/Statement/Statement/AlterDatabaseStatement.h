// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterDatabaseStatement.h --
// 
// Copyright (c) 2000, 2002, 2009, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_ALTERDATABASESTATEMENT_H
#define __SYDNEY_STATEMENT_ALTERDATABASESTATEMENT_H

#include "Common/Common.h"

#include "Statement/Object.h"
#include "Statement/AlterDatabaseAttribute.h"

_SYDNEY_BEGIN

namespace Statement
{
class Identifier;
class AlterDatabaseAction;
class AlterDatabaseAttributeList;

//
//	CLASS
//	Statement::AlterDatabaseStatement --
//
//	NOTES
//
//
class SYD_STATEMENT_FUNCTION AlterDatabaseStatement : public Statement::Object
{
public:
	//
	//	レプリケーション動作種別
	//
	enum ReplicationType
	{
		Unknown = 0,
		
		StartSlave,
		StopSlave,
		SetToMaster,
		
		NumOfAttributeType
	};

public:
	//constructor
	AlterDatabaseStatement()
		: Object(ObjectType::AlterDatabaseStatement)
	{}
	//コンストラクタ(2)
	AlterDatabaseStatement(Identifier* pcIdent_,
						   AlterDatabaseAttributeList* pcAttribute_);
	//デストラクタ
	virtual ~AlterDatabaseStatement();

	//
	//	アクセサ
	//

	//データベース名
	Identifier* getDatabaseName() const;
	void setDatabaseName(Identifier* pcIdent_);

	//AlterDatabaseOption アクセサ
	AlterDatabaseAttributeList* getAlterDatabaseOption() const;
	void setAlterDatabaseOption(AlterDatabaseAttributeList* pcAttribute_);

	Boolean::Value
	getAlterDatabaseOption(AlterDatabaseAttribute::AttributeType type) const;

	// WITH DISCARD LOGICALLOG があるか
	bool isDiscardLogicalLog() const;

	// レプリケーション動作種別
	ReplicationType getReplicationType() const;
	void setReplicationType(ReplicationType eType_);

	//自身をコピーする
	Object* copy() const;

	// SQL文で得る
	ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

private:
	//代入オペレータは使用しない
	AlterDatabaseStatement& operator= (const AlterDatabaseStatement& cOther_);

};

} // namespace Statement

_SYDNEY_END

#endif // __SYDNEY_STATEMENT_ALTERDATABASESTATEMENT_H

//
//	Copyright (c) 2000, 2002, 2009, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
