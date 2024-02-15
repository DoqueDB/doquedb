// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PrivilegeMap.cpp -- Hashmap holding Privilege list
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Schema/PrivilegeMap.h"
#include "Schema/Recovery.h"

namespace {

	// Parameters which used in PrivilegeMap constructor
	ModSize _privilegeMapSize = 5;
	ModBoolean _privilegeMapEnableLink = ModFalse; // don't do Iteration over the map
	bool _privilegeUseView = false; // Don't create Vector unless it is required
	bool _privilegeUseCache = false; // Don't put into Cache
}

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::PrivilegeMap::PrivilegeMap -- constructor
//
//	NOTES
//
//	ARGUMENTS
//		Nothing
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

PrivilegeMap::
PrivilegeMap()
	: ObjectMap<Privilege, PrivilegePointer>(_privilegeMapSize, _privilegeMapEnableLink, _privilegeUseView, _privilegeUseCache)
{
}

//
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
