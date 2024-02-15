// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AlterDatabaseAttribute.h --
// 
// Copyright (c) 2000, 2002, 2003, 2009, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_ALTERDATABASEATTRIBUTE_H
#define __SYDNEY_STATEMENT_ALTERDATABASEATTRIBUTE_H

#include "Common/Common.h"
#include "Statement/ObjectSelection.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class IntegerValue;

//	CLASS
//	Statement::AlterDatabaseAttribute --
//
//	NOTES

class AlterDatabaseAttribute : public Statement::ObjectSelection
{
public:
	//
	//	動作識別子
	//
	enum AttributeType
	{
		Unknown = 0,
		ReadWrite,
		Online,
		RecoveryFull,
		SuperUserMode,
		NumOfAttributeType
	};

public:
	//constructor
	AlterDatabaseAttribute()
		: ObjectSelection(ObjectType::AlterDatabaseAttribute)
	{}
	//コンストラクタ(2)
	SYD_STATEMENT_FUNCTION
	AlterDatabaseAttribute(AttributeType iAttributeType, int iAttribute_);
	//デストラクタ
	SYD_STATEMENT_FUNCTION
	virtual ~AlterDatabaseAttribute();

	//
	//	アクセサ
	//
	//動作識別子アクセサ
	int getAttributeType() const
		{ return ObjectSelection::getObjectType(); }
	void setAttributeType(AttributeType iAttributeType_)
		{ ObjectSelection::setObjectType(iAttributeType_); }

	//Attribute アクセサ
	int getAttribute() const
		{ return ObjectSelection::getScaler(); }
	void setAttribute(int iAttribute_)
		{ ObjectSelection::setScaler(iAttribute_); }

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
	AlterDatabaseAttribute& operator= (const AlterDatabaseAttribute& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_ALTERDATABASEATTRIBUTE_H

//
//	Copyright (c) 2000, 2002, 2003, 2009, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
