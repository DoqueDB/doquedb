// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Grouping.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DPlan::Relation";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "DPlan/Relation/Impl/GroupingImpl.h"
#include "Plan/Relation/RowInfo.h"

#include "Plan/AccessPlan/Source.h"
#include "DPlan/Candidate/Grouping.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Order/Specification.h"
#include "Plan/Order/Key.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Plan/Tree/Node.h"	

#include "Opt/Environment.h"

#include "Schema/Field.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_RELATION_BEGIN

/////////////////////////////////////
// Relation::GroupingImpl::Normal

// FUNCTION public
//	Relation::GroupingImpl::Normal::createAccessPlan -- create access plan candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Plan::Interface::ICandidate*
GroupingImpl::Normal::
createAccessPlan(Opt::Environment& cEnvironment_,
				 Plan::AccessPlan::Source& cPlanSource_)
{
	Plan::Interface::IPredicate* pPredicate = cPlanSource_.getPredicate();
	Plan::AccessPlan::Source cSource(cPlanSource_);
	cSource.erasePredicate();
		
	Plan::Interface::ICandidate* pOperandCandidate =
		getOperand()->createAccessPlan(cEnvironment_, cSource);

	Plan::Relation::InquiryArgument cArgument = 0;
	cArgument.m_iTarget |= Plan::Relation::InquiryArgument::Target::Distributed;
	Plan::Interface::IRelation::InquiryResult iResult = getOperand()->inquiry(cEnvironment_, cArgument);
	Plan::Interface::ICandidate* pResult = 0;
	if (!(iResult & 
		  Plan::Relation::InquiryArgument::Target::Distributed)) {
		pResult = DPlan::Candidate::Grouping::Replicate::create(cEnvironment_,
																m_pSortSpecification,
																m_vecAggregation,
																m_cAggregationOperand,
																pOperandCandidate);
	} else if (m_pSortSpecification->getKeySize() == 0) {
		pResult = DPlan::Candidate::Grouping::Simple::create(cEnvironment_,
															 m_vecAggregation,
															 m_cAggregationOperand,
															 pOperandCandidate);
	} else {
		pResult = DPlan::Candidate::Grouping::Normal::create(cEnvironment_,
															 m_pSortSpecification,
															 m_vecAggregation,
															 m_cAggregationOperand,
															 pOperandCandidate);
	}
	
	if (pPredicate) {
		pPredicate->require(cEnvironment_, pResult);
		pResult->setPredicate(pPredicate);
	}

	return pResult;
}

// FUNCTION public
//	Relation::GroupingImpl::Normal::inquiry -- inquiry about relation's attributes
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const InquiryArgument& cArgument_
//	
// RETURN
//	Interface::IRelation::InquiryResult
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation::InquiryResult
GroupingImpl::Normal::
inquiry(Opt::Environment& cEnvironment_,
		const Plan::Relation::InquiryArgument& cArgument_)
{
	Plan::Interface::IRelation::InquiryResult iResult = 0;
	if (cArgument_.m_iTarget & Plan::Relation::InquiryArgument::Target::Distinct) {
		if (m_pSortSpecification->getKeySize() == 0) {
			// if no grouping column, all results are distinct
			iResult |= Plan::Relation::InquiryArgument::Target::Distinct;
			if (cArgument_.m_pKey) {
				const_cast<Plan::Utility::RowElementSet*>(cArgument_.m_pKey)->clear();
			}
		}
	}
	if (cArgument_.m_iTarget & (Plan::Relation::InquiryArgument::Target::Depending
								| Plan::Relation::InquiryArgument::Target::Refering
								| Plan::Relation::InquiryArgument::Target::Distributed)) {
		Plan::Relation::InquiryArgument cMyArgument(cArgument_);
		if (cArgument_.m_iTarget & (Plan::Relation::InquiryArgument::Target::Depending))
			cMyArgument.m_iTarget |= Plan::Relation::InquiryArgument::Target::Refering;

		// inquiry operand
		iResult = getOperand()->inquiry(cEnvironment_,
										cMyArgument);

		if (cArgument_.m_iTarget & Plan::Relation::InquiryArgument::Target::Depending
			&& (iResult & Plan::Relation::InquiryArgument::Target::Refering)) {
			iResult |= Plan::Relation::InquiryArgument::Target::Depending;
		}
	}


	return (iResult & cArgument_.m_iTarget);
}

// FUNCTION private
//	Relation::GroupingImpl::Normal::setDegree -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
GroupingImpl::Normal::
setDegree(Opt::Environment& cEnvironment_)
{
	return m_vecAggregation.GETSIZE();
}

// FUNCTION private
//	Relation::GroupingImpl::Normal::setMaxPosition -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
GroupingImpl::Normal::
setMaxPosition(Opt::Environment& cEnvironment_)
{
	return getDegree(cEnvironment_);
}

// FUNCTION private
//	Relation::GroupingImpl::Normal::createScalarName -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	VECTOR<STRING>& vecName_
//	Interface::IRelation::Position iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
GroupingImpl::Normal::
createScalarName(Opt::Environment& cEnvironment_,
				 VECTOR<STRING>& vecName_,
				 Plan::Interface::IRelation::Position iPosition_)
{
	if (iPosition_ >= getMaxPosition(cEnvironment_)) {
		return;
	}

	Opt::ExpandContainer(vecName_, getMaxPosition(cEnvironment_));

	vecName_[iPosition_] = m_vecAggregation[iPosition_]->getName();
}

// FUNCTION private
//	Relation::GroupingImpl::Normal::createScalar -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	VECTOR<Interface::IScalar*>& vecScalar_
//	Interface::IRelation::Position iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
GroupingImpl::Normal::
createScalar(Opt::Environment& cEnvironment_,
			 VECTOR<Plan::Interface::IScalar*>& vecScalar_,
			 Plan::Interface::IRelation::Position iPosition_)
{
	if (iPosition_ >= getMaxPosition(cEnvironment_)) {
		return;
	}

	// copy all the scalar
	vecScalar_ = m_vecAggregation;
}

// FUNCTION private
//	Relation::GroupingImpl::Normal::createScalarType -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	VECTOR<Tree::Node::Type>& vecType_
//	Interface::IRelation::Position iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
GroupingImpl::Normal::
createScalarType(Opt::Environment& cEnvironment_,
				 VECTOR<Plan::Tree::Node::Type>& vecType_,
				 Plan::Interface::IRelation::Position iPosition_)
{
	if (iPosition_ >= getMaxPosition(cEnvironment_)) {
		return;
	}

	Opt::ExpandContainer(vecType_, getMaxPosition(cEnvironment_), Plan::Tree::Node::Undefined);

	vecType_[iPosition_] = m_vecAggregation[iPosition_]->getType();
}

// FUNCTION private
//	Relation::GroupingImpl::Normal::setRetrieved -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation::Position iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
GroupingImpl::Normal::
setRetrieved(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation::Position iPosition_)
{
	; // do nothing
}

// FUNCTION private
//	Relation::GroupingImpl::Normal::addAggregation -- add aggregation
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pScalar_
//	Interface::IScalar* pOperand_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
GroupingImpl::Normal::
addAggregation(Opt::Environment& cEnvironment_,
			   Plan::Interface::IScalar* pScalar_,
			   Plan::Interface::IScalar* pOperand_)
{
	VECTOR<Plan::Interface::IScalar*>::ITERATOR found = m_vecAggregation.find(pScalar_);
	if (found == m_vecAggregation.end()) {
		m_vecAggregation.PUSHBACK(pScalar_);
		found = m_vecAggregation.end() - 1;

		// AVG(FulltextLength)の場合のみオペランドをgenerateする
		if (pScalar_->getType() == Plan::Tree::Node::Divide
			&& pOperand_
			&& (pOperand_->getType() == Plan::Tree::Node::FullTextLength
				|| pOperand_->getType() == Plan::Tree::Node::WordCount)) {
			;_SYDNEY_ASSERT(pScalar_->getOperandSize() > 0);
			const Plan::Interface::IScalar* pLengthOperand =
				_SYDNEY_DYNAMIC_CAST(const Plan::Interface::IScalar*,
									 pScalar_->getOperandAt(0)->getOperandAt(0));
			m_cAggregationOperand.add(const_cast<Plan::Interface::IScalar*>(pLengthOperand));
		}
	}
	return found - m_vecAggregation.begin();
}


_SYDNEY_DPLAN_RELATION_END
_SYDNEY_DPLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
