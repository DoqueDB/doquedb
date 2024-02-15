// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Monadic.h --
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

#ifndef __SYDNEY_PLAN_CANDIDATE_MONADIC_H
#define __SYDNEY_PLAN_CANDIDATE_MONADIC_H

#include "Plan/Interface/ICandidate.h"
#include "Plan/Candidate/Row.h"
#include "Plan/Scalar/Argument.h"
#include "Plan/Scalar/Field.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

////////////////////////////////////////////////////
// TEMPLATE CLASS
//	Plan::Candidate::Monadic -- implementation class of Interface::ICandidate
//
// TEMPLATE ARGUMENT
//	class Base_
//
// NOTES
template <class Base_>
class Monadic
	: public Base_
{
public:
	typedef Monadic<Base_> This;
	typedef Base_ Super;

	// destructor
	virtual ~Monadic() {}

/////////////////////////////
// Interface::ICandidate::
//	virtual void createCost(Opt::Environment& cEnvironment_,
//							const AccessPlan::Source& cPlanSource_);
//	virtual const AccessPlan::Cost& getCost();

	virtual void require(Opt::Environment& cEnvironment_,
						 Scalar::Field* pField_);
	virtual void retrieve(Opt::Environment& cEnvironment_,
						  Scalar::Field* pField_);
	virtual void use(Opt::Environment& cEnvironment_,
					 Scalar::Field* pField_);
	virtual bool delay(Opt::Environment& cEnvironment_,
					   Scalar::Field* pField_,
					   Scalar::DelayArgument& cArgument_);

	virtual Interface::IPredicate* getNotCheckedPredicate()
	{return Super::getNotCheckedPredicate() == 0 && m_pOperand ?
			m_pOperand->getNotCheckedPredicate() : Super::getNotCheckedPredicate();}

	virtual Order::Specification* getOrder()
	{return Super::getOrder() == 0 && m_pOperand ?
			m_pOperand->getOrder() : Super::getOrder();}
	virtual const AccessPlan::Limit& getLimit()
	{return Super::getLimit().isSpecified() == false && m_pOperand ?
			m_pOperand->getLimit() : Super::getLimit();}

 	virtual bool isReferingRelation(Interface::IRelation* pRelation_)
	{return m_pOperand && m_pOperand->isReferingRelation(pRelation_);}

	virtual bool isMergeSortAvailable()
	{return m_pOperand && m_pOperand->isMergeSortAvailable();}
	
 	virtual void createReferingRelation(Utility::RelationSet& cRelationSet_)
	{
		if (m_pOperand) {
			m_pOperand->createReferingRelation(cRelationSet_);
		}
	}
	virtual Candidate::Table* getCandidate(Opt::Environment& cEnvironment_,
										   Interface::IRelation* pRelation_)
	{return m_pOperand ? m_pOperand->getCandidate(cEnvironment_, pRelation_) : 0;}
	virtual void generateDelayed(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_)
	{
		if (m_pOperand) {
			m_pOperand->generateDelayed(cEnvironment_, cProgram_, pIterator_);
		}
	}
	virtual Scalar::Field* getUsedField(Opt::Environment& cEnvironment_,
										Scalar::Field* pField_)
	{
		return (m_pOperand ? m_pOperand->getUsedField(cEnvironment_,
													  pField_)
				: Super::getUsedField(cEnvironment_,
									  pField_));
	}
	virtual Interface::ICandidate::InquiryResult
						inquiry(Opt::Environment& cEnvironment_,
								const InquiryArgument& cArgument_)
	{return m_pOperand ? m_pOperand->inquiry(cEnvironment_, cArgument_) : 0;}

	virtual Sql::Query* generateSQL(Opt::Environment& cEnvironment_)
	{return m_pOperand ? m_pOperand->generateSQL(cEnvironment_): 0;}




protected:
	// constructor
	Monadic(Interface::ICandidate* pOperand_)
		: Super(),
		  m_pOperand(pOperand_)
	{}
	template <class A_>
	Monadic(A_ a_, Interface::ICandidate* pOperand_)
		: Super(a_),
		  m_pOperand(pOperand_)
	{}
	template <class A1_, class A2_>
	Monadic(A1_ a1_, A2_ a2_, Interface::ICandidate* pOperand_)
		: Super(a1_, a2_),
		  m_pOperand(pOperand_)
	{}

	Interface::ICandidate* getOperand() {return m_pOperand;}

private:
/////////////////////////////
// Candidate::Base::
	virtual void createCost(Opt::Environment& cEnvironment_,
							const AccessPlan::Source& cPlanSource_,
							AccessPlan::Cost& cCost_)
	{
		if (m_pOperand) {
			m_pOperand->createCost(cEnvironment_, cPlanSource_);
			cCost_ = m_pOperand->getCost();
		}
	}
	virtual Candidate::Row* createRow(Opt::Environment& cEnvironment_)
	{return m_pOperand ? m_pOperand->getRow(cEnvironment_) : Candidate::Row::create(cEnvironment_);}
	virtual Candidate::Row* createKey(Opt::Environment& cEnvironment_)
	{return m_pOperand ? m_pOperand->getKey(cEnvironment_) : Candidate::Row::create(cEnvironment_);}

	Interface::ICandidate* m_pOperand;
	Utility::FieldSet m_cRetrievedColumn;
	Utility::FieldSet m_cRequiredColumn;
	Utility::FieldSet m_cDelayedColumn;
};

// TEMPLATE FUNCTION public
//	Candidate::Monadic<Base_>::require -- 
//
// TEMPLATE ARGUMENTS
//	class Base_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
template <class Base_>
void
Monadic<Base_>::
require(Opt::Environment& cEnvironment_,
		Scalar::Field* pField_)
{
	if (m_pOperand) {
		m_pOperand->require(cEnvironment_,
							pField_);
	}
	m_cRequiredColumn.add(pField_);
}

// TEMPLATE FUNCTION public
//	Candidate::Monadic<Base_>::retrieve -- 
//
// TEMPLATE ARGUMENTS
//	class Base_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
template <class Base_>
void
Monadic<Base_>::
retrieve(Opt::Environment& cEnvironment_,
		 Scalar::Field* pField_)
{
	if (m_pOperand) {
		m_pOperand->retrieve(cEnvironment_,
							 pField_);
	}
}

// TEMPLATE FUNCTION public
//	Candidate::Monadic<Base_>::use -- 
//
// TEMPLATE ARGUMENTS
//	class Base_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
template <class Base_>
void
Monadic<Base_>::
use(Opt::Environment& cEnvironment_,
	Scalar::Field* pField_)
{
	if (m_pOperand) {
		m_pOperand->use(cEnvironment_,
						pField_);
	}
}

// TEMPLATE FUNCTION public
//	Candidate::Monadic<Base_>::delay -- 
//
// TEMPLATE ARGUMENTS
//	class Base_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	Scalar::DelayArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
template <class Base_>
bool
Monadic<Base_>::
delay(Opt::Environment& cEnvironment_,
	  Scalar::Field* pField_,
	  Scalar::DelayArgument& cArgument_)
{
	// check according to requirement for this candidate
	if (m_pOperand && m_cRequiredColumn.isContaining(pField_) == false) {
		// this field is not required -> delay
		// [NOTES]
		//	delay is called only when the field can be delayed
		bool bMinimumSave = cArgument_.m_bMinimum;		
		cArgument_.m_bMinimum = true;
		bool result = m_pOperand->delay(cEnvironment_,
											   pField_,
											   cArgument_);
		cArgument_.m_bMinimum = bMinimumSave;
		return result;
	} else {
		// required
		// [NOTES]
		//	 require method should have been called for operand
		return false;
	}
}

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_MONADIC_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
