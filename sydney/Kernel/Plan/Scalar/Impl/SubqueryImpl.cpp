// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/SubqueryImpl.cpp --
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
const char moduleName[] = "Plan::Scalar";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Scalar/Impl/SubqueryImpl.h"
#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Table.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IRelation.h"
#include "Plan/Relation/RowInfo.h"

#include "Common/Assert.h"
#include "Common/ColumnMetaData.h"
#include "Common/DefaultData.h"
#include "Common/DoubleData.h"
#include "Common/NullData.h"
#include "Common/UnicodeString.h"

#include "Exception/Unexpected.h"

#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Operator/Iterate.h"

#include "DExecution/Action/StatementConstruction.h"

#include "Opt/Environment.h"
#include "Opt/Explain.h"

#include "ModUnicodeCharTrait.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_SCALAR_USING

////////////////////////////////////
// Impl::SubqueryImpl::
////////////////////////////////////

// FUNCTION public
//	Scalar::Impl::SubqueryImpl::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::SubqueryImpl::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("<subquery>");
}

// FUNCTION public
//	Scalar::Impl::SubqueryImpl::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const STRING&
//
// EXCEPTIONS

//virtual
const STRING&
Impl::SubqueryImpl::
getName()
{
	return m_cstrName;
}

// FUNCTION public
//	Scalar::Impl::SubqueryImpl::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	Impl::SubqueryImpl::Check::Value
//
// EXCEPTIONS

//virtual
Impl::SubqueryImpl::Check::Value
Impl::SubqueryImpl::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	Check::Value cResult = Check::Constant;
	m_cOuterRelation.foreachElement(boost::bind(&This::checkOuterRelation,
												this,
												_1,
												boost::cref(cArgument_),
												&cResult));
	return cResult;
}

// FUNCTION public
//	Scalar::Impl::SubqueryImpl::isRefering -- 
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
Impl::SubqueryImpl::
isRefering(Interface::IRelation* pRelation_)
{
	return pRelation_ == 0;
}

// FUNCTION public
//	Scalar::Impl::SubqueryImpl::getUsedTable -- check used tables
//
// NOTES
//
// ARGUMENTS
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::SubqueryImpl::
getUsedTable(Utility::RelationSet& cResult_)
{
	cResult_.merge(m_cOuterRelation);
}

// FUNCTION public
//	Scalar::Impl::SubqueryImpl::getUsedField -- 
//
// NOTES
//
// ARGUMENTS
//	Utility::FieldSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::SubqueryImpl::
getUsedField(Utility::FieldSet& cResult_)
{
	;
}

// FUNCTION public
//	Scalar::Impl::SubqueryImpl::getUnknownKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::CheckUnknownArgument& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::SubqueryImpl::
getUnknownKey(Opt::Environment& cEnvironment_,
			  Predicate::CheckUnknownArgument& cResult_)
{
	;
}

// FUNCTION public
//	Scalar::Impl::SubqueryImpl::require -- extract refered scalar
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::SubqueryImpl::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	m_pSubRelation->require(cEnvironment_,
							pCandidate_);
}

// FUNCTION public
//	Scalar::Impl::SubqueryImpl::retrieve -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::SubqueryImpl::
retrieve(Opt::Environment& cEnvironment_)
{
	// do nothing
}

// FUNCTION public
//	Scalar::Impl::SubqueryImpl::retrieve -- extract refered scalar
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::SubqueryImpl::
retrieve(Opt::Environment& cEnvironment_,
		 Interface::ICandidate* pCandidate_)
{
	// set required
	m_pSubRelation->require(cEnvironment_,
							pCandidate_);
}

// FUNCTION public
//	Scalar::Impl::SubqueryImpl::delay -- extract refered scalar
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	Scalar::DelayArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::SubqueryImpl::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  Scalar::DelayArgument& cArgument_)
{
	// subquery cannot delay
	return false;
}

// FUNCTION public
//	Scalar::Impl::SubqueryImpl::setMetaData -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Common::ColumnMetaData& cMetaData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::SubqueryImpl::
setMetaData(Opt::Environment& cEnvironment_,
			Common::ColumnMetaData& cMetaData_)
{
	cMetaData_.setNotSearchable();
	cMetaData_.setReadOnly();
}


// FUNCTION public
//	Scalar::Impl::SubqueryImpl::toSQLStatement -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const STRING
//
// EXCEPTIONS

//virtual
STRING
Impl::SubqueryImpl::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Scalar::Impl::SubqueryImpl::toSQLStatement -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const STRING
//
// EXCEPTIONS

//virtual
void
Impl::SubqueryImpl::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	if (!m_cOuterRelation.isEmpty()) { // 空でない場合は、Joinなので、ローカルで行う
		cExec_.append("(");
		m_pSubRelation->generateSQL(cEnvironment_)->setParameter(cEnvironment_,
																 cProgram_,
																 pIterator_,
																 cExec_,
																 cArgument_);
		cExec_.append(")");
		return;
	}
	
	Plan::AccessPlan::Source cPlanSource;
	Plan::Interface::ICandidate* pCandidate =
		m_pSubRelation->createAccessPlan(cEnvironment_,
										 cPlanSource);
	Plan::Candidate::AdoptArgument cAdoptArgument;
	cAdoptArgument.m_pQuery = pCandidate->generateSQL(cEnvironment_);

	Execution::Interface::IIterator* pOperandIterator =
		pCandidate->adopt(cEnvironment_, cProgram_, cAdoptArgument);
	Relation::RowInfo* pRowInfo = m_pSubRelation->getRowInfo(cEnvironment_);
	if (pRowInfo->getSize() != 1) {
		// this situation should be checked before
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	int iDataID = pRowInfo->getScalar(cEnvironment_, 0)->generate(cEnvironment_,
																  cProgram_,
																  pOperandIterator,
																  cAdoptArgument);
	Execution::Interface::IAction* pAction =
		Execution::Operator::Iterate::All::create(cProgram_,
												  pIterator_,
												  pOperandIterator->getID());
	cExec_.appendPlaceHolder(iDataID, pAction);
}


// FUNCTION protected
//	Scalar::Impl::SubqueryImpl::generateData -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::SubqueryImpl::
generateData(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	// get corresponding scalar data
	Interface::IScalar* pScalar = m_pSubRelation->getScalar(cEnvironment_,
															m_iPosition);
	/////////////
	// check field's data
	/////////////
	int iDataID = pIterator_->getNodeVariable(pScalar->getID());
	if (iDataID >= 0) {
		// subquery has been already generated -> use it
		return iDataID;
	}

	/////////////
	// generate subquery
	/////////////

	AccessPlan::Source cSource(cArgument_.m_vecPrecedingCandidate, 0, 0);
	if (cArgument_.m_pCandidate) {
		cSource.addPrecedingCandidate(cArgument_.m_pCandidate);
	}

	// mark subquery result row info as retrieved
	Relation::RowInfo* pRowInfo = m_pSubRelation->getRowInfo(cEnvironment_);
	if (pRowInfo) {
		pRowInfo->retrieveAll(cEnvironment_);
	}

	Interface::ICandidate* pCandidate =
		m_pSubRelation->createAccessPlan(cEnvironment_, 
										 cSource);
	if (pCandidate == 0) {
		// can't optimize
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_pInput = pIterator_;
	cMyArgument.setCandidate(pCandidate);

	Execution::Interface::IIterator* pIterator =
		pCandidate->adopt(cEnvironment_, cProgram_, cMyArgument);
	if (pIterator == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	pCandidate->generateDelayed(cEnvironment_, cProgram_, pIterator);

	int iResult = -1;
	// get corresponding scalar data
	int n = m_pSubRelation->getDegree(cEnvironment_);
	for (int i = 0; i < n; ++i) {
		Interface::IScalar* pTmpScalar = m_pSubRelation->getScalar(cEnvironment_, i);
		int iTmpDataID = pTmpScalar->generate(cEnvironment_,
											  cProgram_,
											  pIterator,
											  cMyArgument);
		if (i == m_iPosition) iResult = iTmpDataID;

		// record node->id relationships on outer iterator too
		pIterator_->setNodeVariable(pTmpScalar->getID(), iTmpDataID, pIterator);
		// register result id as used
		pIterator->useVariable(iTmpDataID);
	}
	; _SYDNEY_ASSERT(iResult >= 0);

	// create iterate action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::Iterate::Once::create(cProgram_,
																		  pIterator_,
																		  pIterator->getID(),
																		  m_cOuterRelation.isEmpty()),
							   cArgument_.m_eTarget);

	return iResult;
}

// FUNCTION private
//	Scalar::Impl::SubqueryImpl::checkOuterRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	const CheckArgument& cArgument_
//	Check::Value* pValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::SubqueryImpl::
checkOuterRelation(Interface::IRelation* pRelation_,
				   const CheckArgument& cArgument_,
				   Check::Value* pValue_)
{
	if (cArgument_.m_pCandidate
		&& cArgument_.m_pCandidate->isReferingRelation(pRelation_)) {
		Check::mergeValue(pValue_, Check::Referred);

	} else if (cArgument_.m_vecPrecedingCandidate.ISEMPTY() == false
			   && Opt::IsAny(cArgument_.m_vecPrecedingCandidate,
							 boost::bind(&Interface::ICandidate::isReferingRelation,
										 _1,
										 pRelation_))) {
		Check::mergeValue(pValue_, Check::Preceding);

	} else {
		Check::mergeValue(pValue_, Check::NotYet);
	}
}

//
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
