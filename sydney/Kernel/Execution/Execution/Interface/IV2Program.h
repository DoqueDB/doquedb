// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Interface/IV2Program.h --
// 
// Copyright (c) 2008, 2009, 2010, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_INTERFACE_IV2PROGRAM_H
#define __SYDNEY_EXECUTION_INTERFACE_IV2PROGRAM_H

#include "Execution/Interface/Module.h"
#include "Execution/Program.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_INTERFACE_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Interface::IV2Program -- Interface for new executor program
//
//	NOTES
class IV2Program
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef IV2Program This;

	// destructor
	virtual ~IV2Program() {}

	// get program interface
	virtual Interface::IProgram* getProgram() = 0;
	// set program interface
	virtual void setProgram(Interface::IProgram* pProgram_) = 0;
	// release program interface
	virtual Interface::IProgram* releaseProgram() = 0;

protected:
	// constructor
	IV2Program() : Super() {}

private:
};

_SYDNEY_EXECUTION_INTERFACE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_INTERFACE_IV2PROGRAM_H

//
//	Copyright (c) 2008, 2009, 2010, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
