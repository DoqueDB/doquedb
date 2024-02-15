// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Procedure.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Plan::Relation";
}

#include "SyDefault.h"

#include "Plan/Relation/Procedure.h"
#include "Plan/Relation/Impl/ProcedureImpl.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_RELATION_USING

////////////////////////////////////
// Relation::Procedure::

// FUNCTION public
//	Relation::Procedure::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR<Interface::IScalar*>& vecParames_
//	Interface::IScalar* pReturnValue_
//	
// RETURN
//	Procedure*
//
// EXCEPTIONS

//static
Procedure*
Procedure::
create(Opt::Environment& cEnvironment_,
	   const VECTOR<Interface::IScalar*>& vecParams_,
	   Interface::IScalar* pReturnValue_)
{
	if (pReturnValue_ == 0) {
		// procedure is not supported yet
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	AUTOPOINTER<This> pResult =
		new ProcedureImpl::Function(vecParams_, pReturnValue_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION protected
//	Relation::Procedure::Procedure -- constructor
//
// NOTES
//
// ARGUMENTS
//	Execution::Procedure* pProcedure_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Procedure::
Procedure()
	: Super(Node::Procedure)
{}

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
