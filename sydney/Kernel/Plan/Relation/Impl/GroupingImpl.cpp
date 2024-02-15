// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Grouping.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "boost/bind.hpp"

#include "SyDefault.h"

#include "Plan/Relation/Impl/GroupingImpl.h"
#include "Plan/Relation/RowInfo.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Grouping.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Order/Specification.h"
#include "Plan/Order/Key.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Schema/Field.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

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
Interface::ICandidate*
GroupingImpl::Normal::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	Interface::IPredicate* pPredicate = cPlanSource_.getPredicate();
	Order::Specification* pUsedSpecification = m_pSortSpecification;

	if (cPlanSource_.getOrder()
		&& cPlanSource_.getOrder()->getKeySize() == m_pSortSpecification->getKeySize()
		&& Order::Specification::isCompatible(cPlanSource_.getOrder(),
											  m_pSortSpecification)) {
		// specified order can be replaced for grouping sort specification
		pUsedSpecification = cPlanSource_.getOrder();
	}

	AccessPlan::Source cSource(cPlanSource_,
							   pUsedSpecification);

	cSource.erasePredicate();
	cSource.eraseLimit();
	cSource.setGrouping();
	
	// create operand's access plan
	// it can be assumed operand is sorted by grouping column
	Interface::ICandidate* pOperandCandidate =
		getOperand()->createAccessPlan(cEnvironment_, cSource);

	; _SYDNEY_ASSERT(pOperandCandidate->getOrder() == 0
					 || pUsedSpecification->getKeySize() == 0
					 || (pOperandCandidate->getOrder()->getKeySize() == pUsedSpecification->getKeySize()
						 && Order::Specification::isCompatible(pOperandCandidate->getOrder(),
															   pUsedSpecification)));

	// if bitmap file is not chosen, it cannot be grouping by expand element.
	if (pOperandCandidate->getOrder()
		&& pOperandCandidate->getOrder()->hasExpandElement()
		&& !pOperandCandidate->isGetByBitSetRowID()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	
	// set grouping column as required
	pUsedSpecification->require(cEnvironment_, pOperandCandidate);
	// set aggregation operand as required
	m_cAggregationOperand.foreachElement(boost::bind(&Interface::IScalar::require,
													 _1,
													 boost::ref(cEnvironment_),
													 pOperandCandidate));

	// create candidate
	Interface::ICandidate* pResult;
	pResult = Candidate::Grouping::create(cEnvironment_,
										  pUsedSpecification,
										  m_vecAggregation,
										  m_cAggregationOperand,
										  pOperandCandidate);

	// set arguments
	if (pPredicate) pPredicate->require(cEnvironment_, pResult);
	pUsedSpecification->require(cEnvironment_, pResult);
	pResult->checkPredicate(cEnvironment_,
							cPlanSource_);
	pResult->setOrder(pUsedSpecification);

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
Interface::IRelation::InquiryResult
GroupingImpl::Normal::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	InquiryResult iResult = 0;
	if (cArgument_.m_iTarget & InquiryArgument::Target::Distinct) {
		if (m_pSortSpecification->getKeySize() == 0) {
			// if no grouping column, all results are distinct
			iResult |= InquiryArgument::Target::Distinct;
			if (cArgument_.m_pKey) {
				const_cast<Utility::RowElementSet*>(cArgument_.m_pKey)->clear();
			}
		}
	}
	if (cArgument_.m_iTarget & (InquiryArgument::Target::Depending
								| InquiryArgument::Target::Refering)) {
		InquiryArgument cMyArgument(cArgument_);
		cMyArgument.m_iTarget |= InquiryArgument::Target::Refering;

		// inquiry operand
		iResult = getOperand()->inquiry(cEnvironment_,
										cMyArgument);

		if (cArgument_.m_iTarget & InquiryArgument::Target::Depending
			&& (iResult & InquiryArgument::Target::Refering)) {
			iResult |= InquiryArgument::Target::Depending;
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
				 Interface::IRelation::Position iPosition_)
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
			 VECTOR<Interface::IScalar*>& vecScalar_,
			 Interface::IRelation::Position iPosition_)
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
				 VECTOR<Tree::Node::Type>& vecType_,
				 Interface::IRelation::Position iPosition_)
{
	if (iPosition_ >= getMaxPosition(cEnvironment_)) {
		return;
	}

	Opt::ExpandContainer(vecType_, getMaxPosition(cEnvironment_), Tree::Node::Undefined);

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
			 Interface::IRelation::Position iPosition_)
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
			   Interface::IScalar* pScalar_,
			   Interface::IScalar* pOperand_)
{
	VECTOR<Interface::IScalar*>::ITERATOR found = m_vecAggregation.find(pScalar_);
	if (found == m_vecAggregation.end()) {
		m_vecAggregation.PUSHBACK(pScalar_);
		found = m_vecAggregation.end() - 1;

		if (pOperand_
			&& pOperand_->getType() != Tree::Node::ConstantValue
			&& pOperand_->getType() != Tree::Node::Distinct) {
			m_cAggregationOperand.add(pOperand_);
		}
	}
	return found - m_vecAggregation.begin();
}

/////////////////////////////////////
// Relation::GroupingImpl::Simple

// FUNCTION public
//	Relation::GroupingImpl::Simple::createAccessPlan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
GroupingImpl::Simple::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	; _SYDNEY_ASSERT(getOperand()->getType() == Tree::Node::Table);

	VECTOR<Interface::IScalar*> vecConvert;
	if (Opt::IsAll(getAggregation(),
				   boost::bind(&This::addConvertFunction,
							   this,
							   boost::ref(cEnvironment_),
							   _1,
							   boost::ref(vecConvert))) == false) {
		// any aggregation can't convert to simple form

		// clear converted scalar
		Opt::ForEach(getAggregation(),
					 boost::bind(&Interface::IScalar::clearConvert,
								 _1,
								 boost::ref(cEnvironment_)));
		return Super::createAccessPlan(cEnvironment_, cPlanSource_);
	}

	cPlanSource_.setSimple();
	Interface::ICandidate* pResult =
		getOperand()->createAccessPlan(cEnvironment_, cPlanSource_);

	Opt::ForEach(vecConvert,
				 boost::bind(&Interface::IScalar::retrieve,
							 _1,
							 boost::ref(cEnvironment_),
							 pResult));
	return pResult;
}

// FUNCTION private
//	Relation::GroupingImpl::Simple::addConvertFunction -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pFunction_
//	VECTOR<Interface::IScalar*>& vecResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
GroupingImpl::Simple::
addConvertFunction(Opt::Environment& cEnvironment_,
				   Interface::IScalar* pFunction_,
				   VECTOR<Interface::IScalar*>& vecResult_)
{
	Interface::IScalar* pResult =
		pFunction_->convertFunction(cEnvironment_,
									getOperand(),
									pFunction_,
									Schema::Field::Function::Undefined);
	if (pResult) {
		vecResult_.PUSHBACK(pResult);
		return true;
	}
	return false;
}

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
