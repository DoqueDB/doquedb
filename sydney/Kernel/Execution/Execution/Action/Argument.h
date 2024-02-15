// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Action/Argument.h --
// 
// Copyright (c) 2008, 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ACTION_ARGUMENT_H
#define __SYDNEY_EXECUTION_ACTION_ARGUMENT_H

#include "Execution/Action/Module.h"
#include "Execution/Declaration.h"

#include "Common/Object.h"

#include "Lock/Duration.h"
#include "Lock/Mode.h"

#include "Opt/Algorithm.h"

#include "Schema/Table.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Action::Argument -- argument of addAction
//
//	NOTES
class Argument
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef Argument This;

	// ENUM
	//	Execution::Action::Argument::Type::Value -- type of action argument
	//
	// NOTES
	//	Value is used in Iterator/Base.cpp

	struct Type
	{
		enum Value
		{
			Calculation = 0,
			Aggregation,
			Projection,
			InData,
			OutData,
			Input,
			Output,
			Assign,
			CheckCancel,
			If,
			EndIf,
			Else,
			Unless,
			Continue,
			Break,
			UnlockTuple,
			BeginParallel,
			ParallelList,
			EndParallel,
			ReturnParallelData,
			ValueNum
		};
	};

	// ENUM
	//	Execution::Action::Argument::Target::Value -- type of action target
	//
	// NOTES
	//	Value is used in Iterator/Base and Plan::Candidate::XXX

	struct Target
	{
		enum Value
		{
			Execution,
			StartUp,
			Parallel,
			Aggregation,
			ValueNum
		};
	};

	// constructor
	Argument()
		: m_eType(Type::Calculation),
		  m_iInstanceID(-1),
		  m_iArgumentID(-1),
		  m_iOptionID(-1),
		  m_eTarget(Target::Execution)
	{}
	Argument(Type::Value eType_,
			 int iInstanceID_,
			 int iArgumentID_ = -1,
			 int iOptionID_ = -1,
			 Target::Value eTarget_ = Target::Execution)
		: m_eType(eType_),
		  m_iInstanceID(iInstanceID_),
		  m_iArgumentID(iArgumentID_),
		  m_iOptionID(iOptionID_),
		  m_eTarget(eTarget_)
	{}
	// destructor
	~Argument() {}

	// accessor
	Type::Value getType() const {return m_eType;}
	int getInstanceID() const {return m_iInstanceID;}
	int getArgumentID() const {return m_iArgumentID;}
	int getOptionID() const {return m_iOptionID;}
	Target::Value getTarget() const {return m_eTarget;}

protected:
private:
	Type::Value m_eType;
	int m_iInstanceID;
	int m_iArgumentID;
	int m_iOptionID;
	Target::Value m_eTarget;
};

///////////////////////////////
// STRUCT
//	Action::LockerArgument --
//
// NOTES
struct LockerArgument
{
	Schema::Object::Name m_cTableName;
	Schema::Object::ID::Value m_iDatabaseID;
	Schema::Object::ID::Value m_iTableID;
	Lock::Mode::Value m_eMode;
	Lock::Duration::Value m_eDuration;
	bool m_bIsPrepare;
	bool m_bIsUpdate;
	bool m_bIsCollection;
	bool m_bIsSimple;

	LockerArgument()
		: m_cTableName(),
		  m_iDatabaseID(Schema::Object::ID::Invalid),
		  m_iTableID(Schema::Object::ID::Invalid),
		  m_eMode(Lock::Mode::ValueNum),
		  m_eDuration(Lock::Duration::ValueNum),
		  m_bIsPrepare(false),
		  m_bIsUpdate(false),
		  m_bIsCollection(false),
		  m_bIsSimple(false)
	{}

	void setTable(const Schema::Table* pSchemaTable_)
	{
		m_cTableName = pSchemaTable_->getName();
		m_iDatabaseID = pSchemaTable_->getDatabaseID();
		m_iTableID = pSchemaTable_->getID();
		if (pSchemaTable_->isTemporary()) {
			m_eMode = Lock::Mode::N;
		}
	}

	void serialize(ModArchive& archiver_);
	bool isNeedLock() const;
};

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#define _ACTION_ARGUMENT3(type_, instance_, argument_, option_) \
	Execution::Action::Argument(Execution::Action::Argument::Type::type_, \
								instance_, argument_, option_)

#define _ACTION_ARGUMENT(type_, instance_, argument_) \
	_ACTION_ARGUMENT3(type_, instance_, argument_, -1)

#define _ACTION_ARGUMENT0(type_) \
	_ACTION_ARGUMENT(type_, -1, -1)
#define _ACTION_ARGUMENT1(type_, instance_) \
	_ACTION_ARGUMENT(type_, instance_, -1)
#define _ACTION_ARGUMENT2(type_, instance_, argument_) \
	_ACTION_ARGUMENT(type_, instance_, argument_)

#define _ACTION_ARGUMENT3_T(type_, instance_, argument_, option_, target_)	\
	Execution::Action::Argument(Execution::Action::Argument::Type::type_,	\
								instance_, argument_, option_,				\
								target_)
#define _ACTION_ARGUMENT_T(type_, instance_, argument_, target_)			\
	_ACTION_ARGUMENT3_T(type_, instance_, argument_, -1, target_)

#define _ACTION_ARGUMENT0_T(type_, target_) \
	_ACTION_ARGUMENT_T(type_, -1, -1, target_)
#define _ACTION_ARGUMENT1_T(type_, instance_, target_) \
	_ACTION_ARGUMENT_T(type_, instance_, -1, target_)
#define _ACTION_ARGUMENT2_T(type_, instance_, argument_, target_) \
	_ACTION_ARGUMENT_T(type_, instance_, argument_, target_)

#endif // __SYDNEY_EXECUTION_ACTION_ARGUMENT_H

//
//	Copyright (c) 2008, 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
