// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Procedure.cpp --
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
const char moduleName[] = "Plan::Candidate";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Candidate/Impl/ProcedureImpl.h"
#include "Plan/Candidate/Procedure.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Row.h"

#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IRelation.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Relation/Procedure.h"

#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/Loop.h"

#include "LogicalFile/Estimate.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

//////////////////////////////////////////////
// Candidate::Impl::ProcedureImpl

// FUNCTION public
//	Candidate::Impl::ProcedureImpl::isReferingRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::ProcedureImpl::
isReferingRelation(Interface::IRelation* pRelation_)
{
	return pRelation_ == m_pRelation;
}

// FUNCTION public
//	Candidate::Impl::ProcedureImpl::createReferingRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Utility::RelationSet& cRelationSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ProcedureImpl::
createReferingRelation(Utility::RelationSet& cRelationSet_)
{
	cRelationSet_.add(m_pRelation);
}

// FUNCTION public
//	Candidate::Impl::ProcedureImpl::adopt -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
Impl::ProcedureImpl::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	// create tuple iterator
	Execution::Interface::IIterator* pResult =
		Execution::Iterator::Loop::Once::create(cProgram_);

	VECTOR<Interface::IScalar*>& vecParam = m_pRelation->getParam();
	Opt::ForEach_i(vecParam,
				   boost::bind(&Execution::Interface::IProgram::setPlaceHolder,
							   boost::ref(cProgram_),
							   _2,
							   boost::bind(&Interface::IScalar::generate,
										   _1,
										   boost::ref(cEnvironment_),
										   boost::ref(cProgram_),
										   pResult,
										   boost::ref(cArgument_))));
	int iCalculateID = m_pRelation->getRoutine()->generate(cEnvironment_,
														   cProgram_,
														   pResult,
														   cArgument_);
	int iResultID = m_pRelation->getReturnValue()->generate(cEnvironment_,
															cProgram_,
															pResult,
															cArgument_);
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT(Assign,
										iCalculateID,
										iResultID));
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(OutData,
										 iResultID));

	return pResult;
}

// FUNCTION private
//	Candidate::Procedure::createCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const AccessPlan::Source& cPlanSource_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ProcedureImpl::
createCost(Opt::Environment& cEnvironment_,
		   const AccessPlan::Source& cPlanSource_,
		   AccessPlan::Cost& cCost_)
{
	cCost_.setOverhead(0);
	AccessPlan::Cost::Value cSizeCost(getTupleSize(cEnvironment_));
	cSizeCost /= static_cast<double>(LogicalFile::Estimate::getTransferSpeed(
													 LogicalFile::Estimate::Memory));
	cCost_.setTotalCost(getTupleCount(cEnvironment_) * cSizeCost);
}

// FUNCTION private
//	Candidate::Impl::ProcedureImpl::createRow -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Candidate::Row*
//
// EXCEPTIONS

//virtual
Candidate::Row*
Impl::ProcedureImpl::
createRow(Opt::Environment& cEnvironment_)
{
	Candidate::Row* pResult = Candidate::Row::create(cEnvironment_);
	int n = m_pRelation->getDegree(cEnvironment_);
	for (int i = 0; i < n; ++i) {
		pResult->addScalar(m_pRelation->getScalar(cEnvironment_,
												  i));
	}
	return pResult;
}

// FUNCTION private
//	Candidate::Impl::ProcedureImpl::createKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Candidate::Row*
//
// EXCEPTIONS

//virtual
Candidate::Row*
Impl::ProcedureImpl::
createKey(Opt::Environment& cEnvironment_)
{
	return Candidate::Row::create(cEnvironment_);
}

// FUNCTION private
//	Candidate::Impl::ProcedureImpl::getTupleSize -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//
// RETURN
//	AccessPlan::Cost::Value
//
// EXCEPTIONS

AccessPlan::Cost::Value
Impl::ProcedureImpl::
getTupleSize(Opt::Environment& cEnvironment_)
{
	return m_pRelation->getDegree(cEnvironment_) * 8;
}

// FUNCTION private
//	Candidate::Impl::ProcedureImpl::getTupleCount -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//
// RETURN
//	AccessPlan::Cost::Value
//
// EXCEPTIONS

//virtual
AccessPlan::Cost::Value
Impl::ProcedureImpl::
getTupleCount(Opt::Environment& cEnvironment_)
{
	return m_pRelation->getCardinality(cEnvironment_);
}

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
