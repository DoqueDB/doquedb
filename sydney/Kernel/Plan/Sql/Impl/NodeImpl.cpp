// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Projection.cpp --
// 
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Sql::Impl";
}

#include "SyDefault.h"

#include "Plan/Sql/Impl/NodeImpl.h"
#include "Common/Assert.h"
#include "Plan/Sql/Argument.h"
#include "DExecution/Action/StatementConstruction.h"
#include "Opt/Environment.h"


_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SQL_BEGIN


namespace {

}
/////////////////////////////////////
// Sql::NodeImpl::SimpleStringNode

// FUNCTION public
//	Sql::NodeImpl::SimpleStringNode::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec_
// const Plan::Sql::QueryArgument& cArgument_
//
// RETURN
//	void
//
// EXCEPTIONS

// virtual
void
NodeImpl::SimpleStringNode::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	cExec_.append(m_cstrName);
}

/////////////////////////////////////
// Sql::NodeImpl::SimpleDataNode

// FUNCTION public
//	Sql::NodeImpl::SimpleDataNode::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec_
// const Plan::Sql::QueryArgument& cArgument_
//
// RETURN
//	void
//
// EXCEPTIONS

// virtual
void
NodeImpl::SimpleDataNode::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	cExec_.append(m_iDataID, 0);
}


/////////////////////////////////////
// Sql::NodeImpl::ArrayNode

// FUNCTION public
//	Sql::NodeImpl::ArrayNode::setParameter --
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec_
// const Plan::Sql::QueryArgument& cArgument_
//
// RETURN
//	void
//
// EXCEPTIONS

// virtual
void
NodeImpl::ArrayNode::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	VECTOR<Plan::Interface::ISqlNode*>::ConstIterator ite = m_vecNode.begin();
	for (; ite != m_vecNode.end(); ++ite)
		(*ite)->setParameter(cEnvironment_,
							 cProgram_,
							 pIterator_,
							 cExec_,
							 cArgument_);
}



/////////////////////////////////////
// Sql::NodeImpl::ArrayPlaceHoderNode

// FUNCTION public
//	Sql::NodeImpl::ArrayPlaceHoderNode::setParameter --
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec_
// const Plan::Sql::QueryArgument& cArgument_
//
// RETURN
//	void
//
// EXCEPTIONS

// virtual
void
NodeImpl::ArrayPlaceHolderNode::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	cExec_.appendArrayPlaceHolder(m_iDataID);
}

	
_SYDNEY_PLAN_SQL_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

