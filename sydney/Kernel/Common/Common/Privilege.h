// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Privilege.h --
// 
// Copyright (c) 2007, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_PRIVILEGE_H
#define __TRMEISTER_COMMON_PRIVILEGE_H

#include "Common/Module.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

namespace Privilege
{
	//	TYPEDEF
	//	Common::Privilege::Value -- bitmap which represents privilege list
	//
	//	NOTES
	typedef unsigned int Value;

	//	CONSTANT
	//	Common::Privilege::None -- constant value which denotes no privileges for a category
	//
	//	NOTES

	//	CONSTANT
	//	Common::Privilege::All -- constant value which denotes all privileges for a category
	//
	//	NOTES
	enum {
		None = static_cast<Value>(0),
		All = ~static_cast<Value>(0)
	};

	namespace Category
	{
		//	ENUM
		//	Common::Privilege::Category::Value -- category of privilege
		//
		//	NOTES
		//	This value is used in system_privilege system table.
		//	Value must not be changed.
		enum Value
		{
			System = 0,
			Database,
			Data,
			Reference,
			ValueNum,
			SuperUser = 99				// special category
		};
	}
	namespace SuperUser
	{
		//	ENUM
		//	Common::Privilege::SuperUser::Value -- privileges for the category 'superuser'
		//
		//	NOTES
		//	This value is used in system_privilege system table.
		//	Value must not be changed.
		enum
		{
			CreateDatabase =	1,
			Mount =				1<<1,
			CreateUser =		1<<2,
			DropUser =			1<<3,
			SetPassword =		1<<4,
			ValueNum
		};
	}
	namespace System
	{
		//	ENUM
		//	Common::Privilege::System::Value -- privileges for the category 'system'
		//
		//	NOTES
		//	This value is used in system_privilege system table.
		//	Value must not be changed.
		enum
		{
			DropDatabase =		1,
			AlterDatabase =		1<<1,
			Unmount =			1<<2,
			Backup =			1<<3,
			Grant =				1<<4,
			Revoke =			1<<5,
			ValueNum
		};
	}
	namespace Database
	{
		//	ENUM
		//	Common::Privilege::Database::Value -- privileges for the category 'database'
		//
		//	NOTES
		//	This value is used in system_privilege system table.
		//	Value must not be changed.
		enum
		{
			CreateTable =		1,
			DropTable =			1<<1,
			AlterTable =		1<<2,
			AlterIndex =		1<<3,
			CreateArea =		1<<4,
			DropArea =			1<<5,
			AlterArea =			1<<6,
			Verify =			1<<7,
			CreateCascade =		1<<8,
			AlterCascade =		1<<9,
			DropCascade =		1<<10,
			CreatePartition =	1<<11,
			AlterPartition =	1<<12,
			DropPartition =		1<<13,
			ValueNum
		};
	}
	namespace Data
	{
		//	ENUM
		//	Common::Privilege::Data::Value -- privileges for the category 'data'
		//
		//	NOTES
		//	This value is used in system_privilege system table.
		//	Value must not be changed.
		enum
		{
			Delete =			1,
			Insert =			1<<1,
			Update =			1<<2,
			CreateIndex =		1<<3,
			DropIndex =			1<<4,
			CreateFunction =	1<<5,
			DropFunction =		1<<6,
			ValueNum
		};
	}
	namespace Reference
	{
		//	ENUM
		//	Common::Privilege::Select::Value -- privileges for the category 'select'
		//
		//	NOTES
		//	This value is used in system_privilege system table.
		//	Value must not be changed.
		typedef unsigned int Value;
		enum
		{
			Select =			1,
			Transaction =		1<<1,
			TemporaryTable =	1<<2,
			ValueNum
		};
	}

	namespace Object
	{
		//	ENUM
		//	Common::Privilege::Object::Value -- object category of privilege targets
		//
		//	NOTES
		//	This value is not used now
		typedef int Value;
		enum
		{
			Unknown,
			ValueNum
		};
	}
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif //__TRMEISTER_COMMON_PRIVILEGE_H

//
//	Copyright (c) 2007, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
