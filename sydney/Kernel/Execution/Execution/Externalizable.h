// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Externalizable.h --
// 
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_EXTERNALIZABLE_H
#define __SYDNEY_EXECUTION_EXTERNALIZABLE_H

#include "Execution/Module.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN

namespace Common
{
	class Externalizable;
}

_SYDNEY_EXECUTION_BEGIN

// CLASS
//	Execution::Externalizable -- static class for serialize-related functions
//
// NOTES
class Externalizable
{
public:
	// ENUM
	//	Execution::Externalizable::SubModule::Value -- submodules
	//
	// NOTES
	//	The value must be less than 1000.
	struct SubModule
	{
		enum Value
		{
			Action		=   0,
			Collection	= 100,
			Interface	= 200,
			Iterator	= 300,
			Operator	= 400,
			Predicate	= 500,
			Control		= 600,
			Function	= 700,
			Parallel	= 800,
			MaxValue	= 900
		};
	};

	// ENUM
	//	Execution::Externalizable::Category -- class category
	//
	// NOTES
	//	This category is used for backward compatibility.
	//	Value should not be changed.
	
	struct Category
	{
		enum Value
		{
			Unknown = 0,							// 未定義
			LogData
		};
	};

	// get instance from classid
	static SYD_EXECUTION_FUNCTION Common::Externalizable* getClassInstance(int iClassID_);
	static SYD_EXECUTION_FUNCTION Common::Externalizable* getClassInstance0(int iClassID_);
	static SYD_EXECUTION_FUNCTION Common::Externalizable* getClassInstance1(int iClassID_);

	// get classid
	static int getClassID(int iBase_);

	// initialize modules
	static void initialize();
	// terminate modules
	static void terminate();
protected:
private:
};

_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif //__SYDNEY_EXECUTION_EXTERNALIZABLE_H

//
//	Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
