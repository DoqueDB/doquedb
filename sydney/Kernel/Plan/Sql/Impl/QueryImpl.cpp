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
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Sql/Impl/QueryImpl.h"
#include "Plan/Sql/Argument.h"

#include "Common/Assert.h"
#include "Common/DataInstance.h"

#include "DExecution/Action/StatementConstruction.h"
#include "DExecution/Iterator/Server.h"
#include "DExecution/Action/ServerAccess.h"


#include "Execution/Collection/Queue.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/Input.h"
#include "Execution/Operator/Output.h"
#include "Execution/Operator/Iterate.h"
#include "Execution/Parallel/Thread.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Interface/IScalar.h"
#include "Plan/Utility/ObjectSet.h"
#include "Plan/Order/Specification.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Row.h"

#include "Schema/Database.h"


_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SQL_BEGIN

namespace {
	unsigned int _iCascadeQueueSize = 50;
}

/////////////////////////////////////
// Sql::QueryImpl::Base


// FUNCTION public
//	Sql::QueryImpl::Base::adoptQuery -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	
// RETURN
//	STRING
//
// EXCEPTIONS
Execution::Interface::IIterator*
QueryImpl::Base::
adoptQuery(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Plan::Candidate::AdoptArgument& cArgument_,
		   bool bNoUndone_)
{
	Execution::Interface::IIterator* pResult = 0;
	if (cArgument_.m_pCascade) {
		DExecution::Action::ServerAccess* pAccess =
			DExecution::Action::ServerAccess::create(cProgram_,
													 cArgument_.m_pCascade,
													 getSqlID(cProgram_));
		pResult = DExecution::Iterator::Server::Query::create(cProgram_,
															  pAccess->getID());
		// 分散の場合は上位のNadicなIteratorでSQL文を生成する
		if (cArgument_.m_bReplicate) { 
			addSqlGenerator(cEnvironment_,
							cProgram_,
							pResult,
							cArgument_.m_bConcatinateIterable);
		}
		
		Execution::Parallel::Thread* pThread =
			Execution::Parallel::Thread::create(cProgram_, pResult);
	
		// add parallel by thread to start up list
		pResult->addAction(cProgram_,
						   _ACTION_ARGUMENT1_T(BeginParallel,
											   pThread->getID(),
											   Execution::Action::Argument::Target::StartUp));
		pResult->addAction(cProgram_,
						   _ACTION_ARGUMENT0_T(ParallelList,
											   Plan::Candidate::AdoptArgument::Target::Parallel));
		pResult->addCalculation(cProgram_,
								pAccess,
								Execution::Action::Argument::Target::Parallel);
		pResult->addAction(cProgram_,
						   _ACTION_ARGUMENT0_T(EndParallel,
											   Execution::Action::Argument::Target::StartUp));
		pResult->addAction(cProgram_,
						   _ACTION_ARGUMENT0_T(CheckCancel,
											   Execution::Action::Argument::Target::Execution));
		int iDataID = createOutput(cEnvironment_, cProgram_, pResult, cArgument_);
		pResult->addAction(cProgram_,
						   _ACTION_ARGUMENT1(OutData,
											 iDataID));
	} else {
		pResult = Execution::Iterator::Input::create(cProgram_);
		pResult->addAction(cProgram_,
						   _ACTION_ARGUMENT0_T(CheckCancel,
											   Execution::Action::Argument::Target::Execution));
		int iDataID = createOutput(cEnvironment_, cProgram_, pResult, cArgument_);
		Execution::Interface::ICollection* pQueue =
			getQueue(cEnvironment_, cProgram_, pResult, cArgument_, bNoUndone_);
		
		pResult->addAction(cProgram_,
						   _ACTION_ARGUMENT2(Input,
											 pQueue->getID(),
											 iDataID));
	}
	return pResult;
}
	

// FUNCTION public
//	Sql::QueryImpl::SelectImpl::addSqlGenerator -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	
// RETURN
//	STRING
//
// EXCEPTIONS
void
QueryImpl::Base::
addSqlGenerator(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_,
				bool bConcatinate_)
{

	if (m_pStatement != 0) return;
	
	m_pStatement = DExecution::Action::StatementConstruction::create(cProgram_,
																	 pIterator_,
																	 getSqlID(cProgram_),
																	 bConcatinate_);

	QueryArgument cArgument;
	setParameter(cEnvironment_, cProgram_, pIterator_, *m_pStatement, cArgument);
	m_pStatement->readyToSet();
	pIterator_->addCalculation(cProgram_,
							   m_pStatement,
							   isIterable() ? Execution::Action::Argument::Target::Execution
							   : Execution::Action::Argument::Target::StartUp);
	return;
}


// FUNCTION public
//	Base::createOutput
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
int
QueryImpl::Base::
createOutput(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Plan::Candidate::AdoptArgument& cArgument_)
{
	VECTOR<int> vecData;
	m_cFieldSet.mapElement(vecData,
								boost::bind(&Plan::Interface::IScalar::generateFromType,
											_1,
											boost::ref(cEnvironment_),
											boost::ref(cProgram_),
											pIterator_,
											boost::ref(cArgument_)));
	return cProgram_.addVariable(vecData);	
}


// FUNCTION public
//	Base::createRow
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
Candidate::Row*
QueryImpl::Base::
createRow(Opt::Environment& cEnvironment_)
{
	Candidate::Row* pRow = Candidate::Row::create(cEnvironment_);
	m_cFieldSet.foreachElement(boost::bind(&Candidate::Row::addScalar,
										   pRow,
										   _1));
	return pRow;
}


// FUNCTION public
//	Base::getSqlID
//
// NOTES
//
// ARGUMENTS
//	
//	
// RETURN
//	int
//
// EXCEPTIONS
int
QueryImpl::Base::
getSqlID(Execution::Interface::IProgram& cProgram_)
{
	if (m_iSqlID == -1) {
		VECTOR<int> vecID;
		vecID.PUSHBACK(cProgram_.addVariable(Common::DataInstance::create(Common::DataType::String)));
		vecID.PUSHBACK(cProgram_.addVariable(Common::DataInstance::create(Common::DataType::Array)));
		m_iSqlID = cProgram_.addVariable(vecID);
	}
	
	return m_iSqlID;
}

// FUNCTION private
//	Sql::QueryImpl::Base::createQueue -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Plan::Candidate::AdoptArgument& cArgument_
//	bool bNoUndone_
//	
// RETURN
//	Execution::Interface::ICollection*
//
// EXCEPTIONS

// virtual
Execution::Interface::ICollection*
QueryImpl::Base::
createQueue(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_	,
			Plan::Candidate::AdoptArgument& cArgument_,
			bool bNoUndone_)
{
	// get all the cascade for the database	
	ModVector<Schema::Cascade*> vecSchemaCascade =
		cEnvironment_.getDatabase()->getCascade(cEnvironment_.getTransaction());
	; _SYDNEY_ASSERT(vecSchemaCascade.isEmpty() == false);

	int iQueueSize =
		cArgument_.m_pCascade == 0 ?
		_iCascadeQueueSize * vecSchemaCascade.getSize()
		: _iCascadeQueueSize;
		
	Execution::Interface::ICollection* pQueue =
		Execution::Collection::Queue::Safe::create(cProgram_, iQueueSize);

	addSqlGenerator(cEnvironment_,
					cProgram_,
					pIterator_,
					cArgument_.m_bConcatinateIterable);

	// StartUpの場合はundoneをしない.
	Execution::Parallel::Thread* pThread =
		Execution::Parallel::Thread::create(cProgram_,
											cArgument_.m_eTarget == Execution::Action::Argument::Target::StartUp ? 0 : pIterator_);
	
	// add parallel by thread to start up list
	pIterator_->addAction(cProgram_,
					   _ACTION_ARGUMENT1_T(BeginParallel,
										   pThread->getID(),
										   cArgument_.m_eTarget));
	
	if (cArgument_.m_pCascade != 0) {
		addIterator(cEnvironment_,
					cProgram_,
					pIterator_,
					cArgument_,
					pQueue->getID(),
					bNoUndone_,
					cArgument_.m_pCascade);
	} else {
		Opt::ForEach(vecSchemaCascade,
					 boost::bind(&This::addIterator,
								 this,
								 boost::ref(cEnvironment_),
								 boost::ref(cProgram_),
								 pIterator_,
								 boost::ref(cArgument_),							 
								 pQueue->getID(),
								 bNoUndone_,
								 _1));
	}

	pIterator_->addAction(cProgram_,
					   _ACTION_ARGUMENT0_T(EndParallel,
										   cArgument_.m_eTarget));
	return pQueue;
	
}



// FUNCTION private
//	Sql::QueryImpl::Base::addIterator -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Plan::Candidate::AdoptArgument& cArgument_
//	int iQueueID_
//	bool bNoUndone_
//	Schema::Cascade* pCascade_
//
// RETURN
//	void
//
// EXCEPTIONS
void
QueryImpl::Base::
addIterator(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Plan::Candidate::AdoptArgument& cArgument_,
			int iQueueID_,
			bool bNoUndone_,
			Schema::Cascade* pCascade_)
			
{
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT0_T(ParallelList,
											  Plan::Candidate::AdoptArgument::Target::Parallel));

	// create server input action
	QueryArgument cQueryArgument;
	Execution::Interface::IIterator* pInput =
		DExecution::Iterator::Server::Query::create(cProgram_,
													pCascade_,
													getSqlID(cProgram_));
	
	int iDataID = createOutput(cEnvironment_,
							   cProgram_,
							   pInput,
							   cArgument_);
	
	pInput->addAction(cProgram_,
					  _ACTION_ARGUMENT1(OutData,
										iDataID));
	pInput->addCalculation(cProgram_,
						   Execution::Operator::Output::create(cProgram_,
															   pInput,
															   iQueueID_,
															   iDataID));
	// add loop action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::Iterate::NestedAll::create(cProgram_,
																			   pIterator_,
																			   pInput->getID(),
																			   bNoUndone_),
							   Plan::Candidate::AdoptArgument::Target::Parallel);
}


/////////////////////////////////////
// Sql::QueryImpl::SelectImpl

// FUNCTION public
//	Sql::QueryImpl::SelectImpl::toSQLStatement -- 
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
QueryImpl::SelectImpl::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	char c = ' ';	
	{
		cStream << "select";
		if (m_bDistinct) {
			cStream << " distinct ";
		}
		VECTOR<Plan::Interface::IScalar*>::ConstIterator iterator = m_cFieldSet.begin();
		VECTOR<Plan::Interface::IScalar*>::ConstIterator last = m_cFieldSet.end();
		for (; iterator != last; ++iterator, c = ',') {
			cStream << c << (*iterator)->toSQLStatement(cEnvironment_, cArgument_);
		}
	}
	
	cStream << " from " << m_pTable->toSQLStatement(cEnvironment_, cArgument_);

	if (m_pPredicate) {
		cStream << " where ";
		cStream << m_pPredicate->toSQLStatement(cEnvironment_, cArgument_);
	}

	if (m_pGrouping
		&& m_pGrouping->getKeySize() != 0) {
		cStream << " group by";		
		cStream << m_pGrouping->toSQLStatement(cEnvironment_, cArgument_);
		if (m_pHaving) {
			cStream << "having";
			cStream << m_pHaving->toSQLStatement(cEnvironment_, cArgument_);
		}
	}

	if (m_pOrder) {
		cStream << " order by";		
		cStream << m_pOrder->toSQLStatement(cEnvironment_, cArgument_);
	}

	if (m_pLimit) {
		cStream << m_pLimit->toSQLStatement(cEnvironment_, cArgument_);
	}


	return cStream.getString();
}

// FUNCTION public
//	Sql::QueryImpl::SelectImpl::setParameter -- 
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
QueryImpl::SelectImpl::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	

	Plan::Sql::QueryArgument cMyArgument(cArgument_);
	cMyArgument.m_bNeedRelationForColumn = true;
	cMyArgument.m_bDistribution = m_bDistribution;
	
	if (cMyArgument.m_bSubQuery) {
		cExec_.append("(");
	}
	if (m_cstrCorrelationName.getLength() > 0)
		m_cFieldSet.foreachElement(boost::bind(&Plan::Interface::ISqlNode::nextScope,
											   _1));
	
	char c = ' ';
	{
		Plan::Sql::QueryArgument cMyArgument2(cMyArgument);
		cMyArgument2.m_bProjectionColumn = true;
		cExec_.append("select");

		if (m_bDistinct)
			cExec_.append(" distinct ");
		
		VECTOR<Plan::Interface::IScalar*>::ConstIterator iterator = m_cFieldSet.begin();
		VECTOR<Plan::Interface::IScalar*>::ConstIterator last = m_cFieldSet.end();
		if (iterator == last) {
			cExec_.append(" * ");
		} else {
			for (; iterator != last; ++iterator, c = ',') {
				cExec_.append(c);
				(*iterator)->setParameter(cEnvironment_,
										  cProgram_,
										  pIterator_,
										  cExec_,
										  cMyArgument2);
			}
		}
	}
	
	cExec_.append(" from ");
	m_pTable->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cMyArgument);

	if (m_pPredicate) {
		cExec_.append(" where ");
		m_pPredicate->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cMyArgument);
	}

	if (m_pGrouping
		&& m_pGrouping->getKeySize() != 0) {
		cExec_.append(" group by");
		m_pGrouping->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cMyArgument);
		if (m_pHaving) {
			cExec_.append(" having ");
			m_pHaving->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cMyArgument);
		}
	}

	if (m_pOrder) {
		cExec_.append(" order by");		
		m_pOrder->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cMyArgument);		
	}

	if (m_pLimit) {
		cExec_.append(m_pLimit->toSQLStatement(cEnvironment_, cMyArgument));
	}

	if (m_cstrCorrelationName.getLength() > 0)
			m_cFieldSet.foreachElement(boost::bind(&Plan::Interface::ISqlNode::reverseScope,
										   _1));

	if (cMyArgument.m_bSubQuery) {
		cExec_.append(")");
	}

	if (m_cstrCorrelationName.getLength() > 0)
		cExec_.append(" ").append(m_cstrCorrelationName);
}





/////////////////////////////////////
// Sql::QueryImpl::InsertImpl

// FUNCTION public
//	Plan::InsertImpl::toSQLStatement -- 
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
QueryImpl::InsertImpl::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	cStream << "insert into ";
	cStream << m_pTable->toSQLStatement(cEnvironment_, cArgument_);
	char c = '(';
	VECTOR<Plan::Interface::IScalar*>::ConstIterator iterator = m_cFieldSet.begin();
	VECTOR<Plan::Interface::IScalar*>::ConstIterator last = m_cFieldSet.end();	
	for (; iterator != last; ++iterator, c = ',') {
		cStream << c << (*iterator)->getName();
	}
	cStream << ") values ";
	iterator = m_cFieldSet.begin();
	c = '(';
	for (; iterator != last; ++iterator, c = ',') {
		cStream << c << "?";
	}
	cStream << ")";

	return cStream.getString();
}



/////////////////////////////////////
// Sql::QueryImpl::DeleteImpl

// FUNCTION public
//	Plan::DeleteImpl::toSQLStatement -- 
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
QueryImpl::UpdateImpl::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}


// FUNCTION public
//	Sql::QueryImpl::UpdateImpl::setParameter
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	
//	
// RETURN
//	STRING
//
// EXCEPTIONS
void
QueryImpl::UpdateImpl::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	Plan::Sql::QueryArgument cArgument;
	cArgument.m_bNeedRelationForColumn = false;
	cArgument.m_bDistribution = m_bDistribution;
	OSTRSTREAM cStream;
	cExec_.append("update ");
	m_pTable->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cArgument);
	cExec_.append(" set ");
	
	VECTOR<Plan::Interface::IScalar*>::ConstIterator iterator = m_cUpdateSet.begin();
	VECTOR<Plan::Interface::IScalar*>::ConstIterator last = m_cUpdateSet.end();
	char c = ' ';
	Plan::Sql::QueryArgument cMyArgument(cArgument_);
	cMyArgument.m_pUpdateSet = &m_cUpdateSet;
	for (; iterator != last; ++iterator, c = ',') {
		cExec_.append(c);
		(*iterator)->setParameter(cEnvironment_,
								  cProgram_,
								  pIterator_,
								  cExec_,
								  cMyArgument);
	}
	

	if (m_pPredicate) {
		cExec_.append(" where ");
		m_pPredicate->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cArgument);
	}
}



/////////////////////////////////////
// Sql::QueryImpl::DeleteImpl

// FUNCTION private
//	Sql::QueryImpl::DeleteImpl::setParameter
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	
//	
// RETURN
//	STRING
//
// EXCEPTIONS
void
QueryImpl::DeleteImpl::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	QueryArgument cArgument;
	cArgument.m_bDistribution = m_bDistribution;
	OSTRSTREAM cStream;
	char c = ' ';
	cExec_.append("delete");
	
	cExec_.append(" from ");
	m_pTable->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cArgument);

	if (m_pPredicate) {
		cExec_.append(" where ");
		m_pPredicate->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cArgument);
	}
}


/////////////////////////////////////
// Sql::QueryImpl::InsertImpl

// FUNCTION public
//	Plan::InsertImpl::toSQLStatement -- 
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
QueryImpl::DeleteImpl::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}



_SYDNEY_PLAN_SQL_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
