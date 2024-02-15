// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DExecution/Externalizable.h --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_DEXECUTION_EXTERNALIZABLE_H
#define __SYDNEY_DEXECUTION_EXTERNALIZABLE_H

#include "DExecution/Module.h"

#include "Execution/Externalizable.h"

_SYDNEY_BEGIN

namespace Common
{
	class Externalizable;
}

_SYDNEY_DEXECUTION_BEGIN

// CLASS
//	DExecution::Externalizable -- static class for serialize-related functions
//
// NOTES
class Externalizable
{
public:
	// ENUM
	//	DExecution::Externalizable::SubModule::Value -- submodules
	//
	// NOTES
	//	The value must be less than 1000.
	typedef Execution::Externalizable::SubModule SubModule;

	// get instance from classid
	static SYD_DEXECUTION_FUNCTION Common::Externalizable* getClassInstance(int iClassID_);

	// get classid
	static int getClassID(int iBase_);

	// initialize modules
	static void initialize();
	// terminate modules
	static void terminate();
protected:
private:
};

_SYDNEY_DEXECUTION_END
_SYDNEY_END

#endif //__SYDNEY_DEXECUTION_EXTERNALIZABLE_H

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
