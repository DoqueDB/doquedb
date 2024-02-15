// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/IScalar.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Interface";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Interface/IScalar.h"
#include "Plan/Candidate/Argument.h"

#include "Plan/Sql/Query.h"

#include "Common/Object.h"
#include "Common/Assert.h"

#include "DExecution/Action/StatementConstruction.h"

#include "Exception/NotSupported.h"

#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_INTERFACE_USING

namespace
{
	// CLASS local
	// SimpleNode
	class SimpleNode
		:public ISqlNode,
		 public Common::Object
	{
	public:
		SimpleNode(const STRING& cstrSql, int iDataID_)
			:ISqlNode(),
			 m_cstrSql(cstrSql),
			 m_iDataID(iDataID_)
			{}
		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const
		{return m_cstrSql;}
		
		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_)
			{
				cExec.append(toSQLStatement(cEnvironment_, cArgument_));
				cExec.append(m_iDataID, 0);
			}		


		void registerToEnvironment(Opt::Environment& cEnvironment_) {cEnvironment_.addObject(this);}

	private:
		STRING m_cstrSql;
		int m_iDataID;

	};
}




// FUNCTION public
//	Plan::Interface::ISqlNode::setParameter
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	const Plan::Sql::QueryArgument& cArgument_
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
ISqlNode::setParameter(Opt::Environment& cEnvironment_,
					   Execution::Interface::IProgram& cProgram_,
					   Execution::Interface::IIterator* pIterator_,
					   DExecution::Action::StatementConstruction& cExec,
					   const Plan::Sql::QueryArgument& cArgument_)
{
	cExec.append(toSQLStatement(cEnvironment_, cArgument_));
}



// FUNCTION public
//	Plan::Interface::ISqlNode::pushNameScope
//
// NOTES
//
// ARGUMENTS
//	Plan::Sql::Query* pQuery_
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
ISqlNode::pushNameScope(Plan::Sql::Query* pQuery_)
{
	if (m_vecNameScope.GETSIZE() != 0
		&& m_vecNameScope.GETBACK() == pQuery_) {
		return;
	}
	m_vecNameScope.PUSHBACK(pQuery_);
	m_iPosition++;
}


// FUNCTION public
//	Plan::Interface::ISqlNode::nextScope
//
// NOTES
//
// ARGUMENTS
//	Plan::Sql::Query* pQuery_
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
ISqlNode::nextScope()
{
	m_iPosition--;
}

// FUNCTION public
//	Plan::Interface::ISqlNode::reverseScope
//
// NOTES
//
// ARGUMENTS
//	Plan::Sql::Query* pQuery_
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
ISqlNode::reverseScope()
{
	m_iPosition++;
}




// FUNCTION public
//	Plan::Interface::ISqlNode::getCorrelationName
//
// NOTES
//
// ARGUMENTS
//	Plan::Sql::Query* pQuery_
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
const STRING&
ISqlNode::getCorrelationName() const
{

	if (m_iPosition == -1) {
		const static STRING cstrEmypt = "";
		return cstrEmypt;
	}
	
	;_SYDNEY_ASSERT(m_iPosition < m_vecNameScope.GETSIZE());
	return m_vecNameScope[m_iPosition]->getCorrelationName();
}


	


// FUNCTION public
//	Sql::Query::createSimpleNode -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
//	
// RETURN
//	ISqlNode*
//
// EXCEPTIONS

//static
ISqlNode*
ISqlNode::
createSimpleNode(Opt::Environment& cEnvironment_, const STRING& cstrSql_, int iDataID_)
{
	AUTOPOINTER<SimpleNode> pResult = new SimpleNode(cstrSql_, iDataID_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
