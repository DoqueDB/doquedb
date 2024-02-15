// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Interface/IProgramBridge.h --
// 
// Copyright (c) 2008, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_INTERFACE_IPROGRAMBRIDGE_H
#define __SYDNEY_EXECUTION_INTERFACE_IPROGRAMBRIDGE_H

#include "Execution/Interface/Module.h"
#include "Execution/Interface/Declaration.h"

#include "Common/Object.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_INTERFACE_BEGIN

class IV1Program;
class IV2Program;

// CLASS
//	Execution::Interface::IProgramBridge -- bridge interface of program class v1 and v2
//
// NOTES
class IProgramBridge
	: public Common::Object
{
public:
	typedef IProgramBridge This;
	typedef Common::Object Super;

	// ENUM
	//	Interface::IProgramBridge::Version -- version of Program class
	//
	// NOTES

	struct Version
	{
		enum Value
		{
			Unknown = 0,
			V1,					// using VECTOR<RelationInfoPointer>
			V2,					// using Interface::IIterator
			ValueNum
		};
	};

	virtual ~IProgramBridge() {}
	
	// get program version
	virtual Version::Value getVersion() = 0;

	// get v1 interface
	virtual IV1Program* getV1Interface() = 0;
	// get v2 interface
	virtual IV2Program* getV2Interface() = 0;

protected:
	IProgramBridge() : Super() {}
private:
};

_SYDNEY_EXECUTION_INTERFACE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_INTERFACE_IPROGRAMBRIDGE_H

//
//	Copyright (c) 2008, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
