// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DExecution/Operator/ServerOperation.h --
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

#ifndef __SYDNEY_DEXECUTION_OPERATOR_SERVEROPERATION_H
#define __SYDNEY_DEXECUTION_OPERATOR_SERVEROPERATION_H

#include "DExecution/Operator/Module.h"

#include "Execution/Declaration.h"
#include "Execution/Interface/IAction.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Cascade;
}

_SYDNEY_DEXECUTION_BEGIN
_SYDNEY_DEXECUTION_OPERATOR_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	CLASS
//	DExecution::Operator::ServerOperation -- operator class for operationing file
//
//	NOTES
//		This class is not constructed directly
class ServerOperation
	: public Execution::Interface::IAction
{
public:
	typedef Execution::Interface::IAction Super;
	typedef ServerOperation This;

	// constructor
	static This* create(Execution::Interface::IProgram& cProgram_,
						Execution::Interface::IIterator* pIterator_,
						Schema::Cascade* pSchemaCascade_,
						const STRING& cstrSQL_,
						int iDataID_);

	// destructor
	virtual ~ServerOperation() {}

	// for serialize
	static This* getInstance(int iCategory_);

protected:
	// constructor
	ServerOperation() : Super() {}

private:
};

_SYDNEY_DEXECUTION_OPERATOR_END
_SYDNEY_DEXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_DEXECUTION_OPERATOR_SERVEROPERATION_H

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
