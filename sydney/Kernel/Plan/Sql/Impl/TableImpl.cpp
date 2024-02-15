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

#include "Plan/Sql/Impl/TableImpl.h"
#include "Common/Assert.h"
#include "Plan/Sql/Argument.h"
#include "Exception/NotSupported.h"
#include "DExecution/Action/StatementConstruction.h"
#include "Opt/Environment.h"


_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SQL_BEGIN


namespace {
	enum Operator
	{
		SimpleJoin,
		LeftOuterJoin,
		RightOuterJoin,
		FullOuterJoin
	};
	
	const char* const _pszOperatorName[] = 
	{
		" JOIN ",
		" LEFT OUTER JOIN ",
		" RIGHT OUTER JOIN ",
		" FULL OUTER JOIN "
	};
	// FUNCTION local
	//	$$$::_getOperatorName -- get operator name for explain
	//
	// NOTES
	//
	// ARGUMENTS
	//	Tree::Node::Type eType_
	//	
	// RETURN
	//	const char*
	//
	// EXCEPTIONS

	const char*
	_getOperatorName(Tree::Node::Type eOperator_)
	{
		switch (eOperator_)
		{
		case Tree::Node::SimpleJoin:		return _pszOperatorName[SimpleJoin];
		case Tree::Node::LeftOuterJoin:		return _pszOperatorName[LeftOuterJoin];
		case Tree::Node::RightOuterJoin:	return _pszOperatorName[RightOuterJoin];
		case Tree::Node::FullOuterJoin:		return _pszOperatorName[FullOuterJoin];
		default:
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
}
/////////////////////////////////////
// Sql::TableImpl::SimpleTable

// FUNCTION public
//	Sql::TableImpl::SimpleTable::toSQLStatement
//
// NOTES
//
// ARGUMENTS
//	
//	
// RETURN
//	STRING
//
// EXCEPTIONS
STRING
TableImpl::SimpleTable::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	cStream << m_cstrTable;
	if (m_cstrCorrelationName.getLength() > 0
		&& m_cstrTable != m_cstrCorrelationName)
		cStream << " " << m_cstrCorrelationName;
	
	return cStream.getString();
}



/////////////////////////////////////
// Sql::TableImpl::DyadicJoin

// FUNCTION public
//	Sql::TableImpl::DyadicJoin::toSQLStatement
//
// NOTES
//
// ARGUMENTS
//	
//	
// RETURN
//	STRING
//
// EXCEPTIONS
STRING
TableImpl::DyadicJoin::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
		
	OSTRSTREAM cStream;
	if (m_pPredicate) {
		cStream << m_pLeft->toSQLStatement(cEnvironment_, cArgument_) <<
			_getOperatorName(m_eOperator) << m_pRight->toSQLStatement(cEnvironment_, cArgument_);
		cStream << " ON " << m_pPredicate->toSQLStatement(cEnvironment_, cArgument_);
	} else {
		cStream << m_pLeft->toSQLStatement(cEnvironment_, cArgument_) << " , "
				<< m_pRight->toSQLStatement(cEnvironment_, cArgument_);
	}
	return cStream.getString();
}


// FUNCTION public
//	Sql::TableImpl::DyadicJoin::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec_
//	
// RETURN
//	void
//
// EXCEPTIONS

// virtual
void
TableImpl::DyadicJoin::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	Plan::Sql::QueryArgument cMyArgument(cArgument_);
	cMyArgument.m_bSubQuery = true;
	m_pLeft->setParameter(cEnvironment_,
						  cProgram_,
						  pIterator_,
						  cExec_,
						  cMyArgument);

	if (m_pPredicate ==0 &&
		m_eOperator == Tree::Node::SimpleJoin) {
		cExec_.append(", ");
	} else {
		cExec_.append(_getOperatorName(m_eOperator));
	}
	
	m_pRight->setParameter(cEnvironment_,
						   cProgram_,
						   pIterator_,
						   cExec_,
						   cMyArgument);
	if (m_pPredicate) {
		cExec_.append(" ON ");
		m_pPredicate->setParameter(cEnvironment_,
								   cProgram_,
								   pIterator_,
								   cExec_,
								   cMyArgument);
	}
}


/////////////////////////////////////
// Sql::TableImpl::NadicJoin

// FUNCTION public
//	Sql::TableImpl::NadicJoin::toSQLStatement
//
// NOTES
//
// ARGUMENTS
//	
//	
// RETURN
//	STRING
//
// EXCEPTIONS
STRING
TableImpl::NadicJoin::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	VECTOR<Plan::Interface::ISqlNode*>::ConstIterator iterator = m_vecTable.begin();
	VECTOR<Plan::Interface::ISqlNode*>::ConstIterator last = m_vecTable.end();
	char c = ' ';	
	for (; iterator != last; ++iterator, c = ',') {
		cStream << c << (*iterator)->toSQLStatement(cEnvironment_, cArgument_);
	}
	
	return cStream.getString();
}


// FUNCTION public
//	Sql::TableImpl::NadicJoin::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	
// RETURN
//	void
//
// EXCEPTIONS

// virtual
void
TableImpl::NadicJoin::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	Plan::Sql::QueryArgument cMyArgument(cArgument_);
	cMyArgument.m_bSubQuery = true;
	VECTOR<Plan::Interface::ISqlNode*>::ConstIterator ite = m_vecTable.begin();
	VECTOR<Plan::Interface::ISqlNode*>::ConstIterator last = m_vecTable.end();
	char c = ' ';	
	for (; ite != last; ++ite, c = ',') {
		cExec_.append(c);
		(*ite)->setParameter(cEnvironment_,
							 cProgram_,
							 pIterator_,
							 cExec_,
							 cMyArgument);
		
	}
}
	
_SYDNEY_PLAN_SQL_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

