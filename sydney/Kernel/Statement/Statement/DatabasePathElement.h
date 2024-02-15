// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DatabasePathElement.h --
// 
// Copyright (c) 2000, 2002, 2003, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_DBPATHELEMENT_H
#define __SYDNEY_STATEMENT_DBPATHELEMENT_H

#include "Common/Common.h"
#include "Literal.h"
#include "Statement/ObjectSelection.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class Literal;

//	CLASS
//	Statement::DatabasePathElement --
//
//	NOTES

class DatabasePathElement : public Statement::ObjectSelection
{
public:
	//
	//	動作識別子
	//
	enum PathType
	{
		Unknown = 0,
		Database,
		LogicalLog,
		System,
		NumOfPathType
	};
public:
	//constructor
	DatabasePathElement()
		: ObjectSelection(ObjectType::DatabasePathElement)
	{}
	//コンストラクタ(2)
	SYD_STATEMENT_FUNCTION
	DatabasePathElement(PathType iPathType, Literal* pcPath_);
	//デストラクタ
	SYD_STATEMENT_FUNCTION
	virtual ~DatabasePathElement();

	//
	//	アクセサ
	//
	//動作識別子アクセサ
	int getPathType() const
		{ return ObjectSelection::getObjectType(); }
	void setPathType(PathType iPathType_)
		{ ObjectSelection::setObjectType(iPathType_); }

	//PathName アクセサ
	Statement::Literal* getPathName() const
		{ return dynamic_cast<Statement::Literal*>(ObjectSelection::getObject()); }
	void setPathName(Statement::Literal* pcPath_)
		{ ObjectSelection::setObject(pcPath_); }

	//自身をコピーする
	SYD_STATEMENT_FUNCTION
	Object* copy() const;

	// SQL文で値を得る
	ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;
	
#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

private:
	//代入オペレータは使用しない
	DatabasePathElement& operator= (const DatabasePathElement& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_DBPATHELEMENT_H

//
//	Copyright (c) 2000, 2002, 2003, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
