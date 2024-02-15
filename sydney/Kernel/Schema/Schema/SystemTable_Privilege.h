// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemTable_Privilege.h -- Privilege system table
// 
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_SYSTEMTABLE_PRIVILEGE_H
#define	__SYDNEY_SCHEMA_SYSTEMTABLE_PRIVILEGE_H

#include "Schema/SystemTable.h"
#include "Schema/Privilege.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

namespace SystemTable
{
	//	CLASS
	//	Schema::SystemTable::Privilege --
	//		Privilege system table
	//
	//	NOTES

	class Privilege
		: public	Base<Schema::Privilege, Schema::Privilege::Pointer, Schema::Database>
	{
	public:
		typedef Base<Schema::Privilege, Schema::Privilege::Pointer, Schema::Database> Super;

		// constructor
		SYD_SCHEMA_FUNCTION
		explicit Privilege(Schema::Database& cDatabase_);

		// load from system table
		SYD_SCHEMA_FUNCTION
		void				load(Trans::Transaction& cTrans_, bool bRecovery_ = false);

		// store to system table
		SYD_SCHEMA_FUNCTION
		void				store(Trans::Transaction& cTrans_,
								  const Schema::PrivilegePointer& pPrivilege_,
								  bool continuously = false);
		SYD_SCHEMA_FUNCTION
		void				store(Trans::Transaction& cTrans_,
								  const Schema::Privilege& cPrivilege_,
								  bool continuously = false);
	protected:
	private:
	};
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_SYSTEMTABLE_PRIVILEGE_H

//
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
