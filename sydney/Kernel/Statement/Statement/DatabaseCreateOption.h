// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DatabaseCreateOption.h --
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

#ifndef __SYDNEY_STATEMENT_DATABASECREATEOPTION_H
#define __SYDNEY_STATEMENT_DATABASECREATEOPTION_H

#include "Common/Common.h"
#include "Statement/ObjectSelection.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

	class Object;

//	CLASS
//	Statement::DatabaseCreateOption --
//
//	NOTES

class DatabaseCreateOption	: public Statement::ObjectSelection
{
public:
	//
	//	動作識別子
	//
	enum OptionType
	{
		Unknown = 0,
		PathOption,
		ReadWriteOption,
		OnlineOption,
		RecoveryOption,
		UserModeOption,
		NumOfOptionType
	};
	enum ReadWriteMode
	{
		ReadWriteModeUnknown = 0,
		ReadOnly,
		ReadWrite
	};
	enum OnlineMode
	{
		OnlineModeUnknown = 0,
		Online,
		Offline,
		OnlineWithDiscardLogicalLog	// ALTER DATABASE にしか現れない
	};
	enum RecoveryMode
	{
		RecoveryModeUnknown = 0,
		RecoveryFull,
		RecoveryCheckpoint
	};

	enum SuperUserMode
	{
		SuperUserModeUnknown = 0,
		SuperUser,
		AllUser
	};
	
public:
	//constructor
	DatabaseCreateOption()
		: ObjectSelection(ObjectType::DatabaseCreateOption)
	{}
	//コンストラクタ(2)
	SYD_STATEMENT_FUNCTION
	DatabaseCreateOption(OptionType iOptionType, Object* pcOption_);
	//デストラクタ
	SYD_STATEMENT_FUNCTION
	virtual ~DatabaseCreateOption();

	//
	//	アクセサ
	//
	//動作識別子アクセサ
	int getOptionType() const
		{ return ObjectSelection::getObjectType(); }
	void setOptionType(OptionType iOptionType_)
		{ ObjectSelection::setObjectType(iOptionType_); }

	//CreateOption アクセサ
	Object* getOption() const
		{ return ObjectSelection::getObject(); }
	void setOption(Object* pcOption_)
		{ ObjectSelection::setObject(pcOption_); }

	//自身をコピーする
	SYD_STATEMENT_FUNCTION
	Object* copy() const;

	// SQL文で値を得る
	ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;
	//文字列表記を得る
	static ModUnicodeString getOptionName(OptionType iOptionType_,
										  int iValue_);

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

private:
	//代入オペレータは使用しない
	DatabaseCreateOption& operator= (const DatabaseCreateOption& cOther_);
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif //__SYDNEY_STATEMENT_DATABASECREATEOPTION_H

//
//	Copyright (c) 2000, 2002, 2003, 2009, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
