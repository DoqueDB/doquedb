// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Nadic.h --
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

#ifndef __SYDNEY_PLAN_CANDIDATE_NADIC_H
#define __SYDNEY_PLAN_CANDIDATE_NADIC_H

#include "boost/bind.hpp"

#include "Plan/Interface/ICandidate.h"
#include "Plan/Scalar/Argument.h"
#include "Plan/Scalar/Field.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

////////////////////////////////////////////////////
// TEMPLATE CLASS
//	Plan::Candidate::Nadic -- implementation class of Interface::ICandidate
//
// TEMPLATE ARGUMENT
//	class Base_
//
// NOTES
template <class Base_>
class Nadic
	: public Base_
{
public:
	typedef Nadic<Base_> This;
	typedef Base_ Super;
	typedef Interface::ICandidate Operand;

	// destructor
	virtual ~Nadic() {}

/////////////////////////////
// Interface::ICandidate::
	virtual void require(Opt::Environment& cEnvironment_,
						 Scalar::Field* pField_);
	virtual void retrieve(Opt::Environment& cEnvironment_,
						  Scalar::Field* pField_);
	virtual void use(Opt::Environment& cEnvironment_,
					 Scalar::Field* pField_);
	virtual bool delay(Opt::Environment& cEnvironment_,
					   Scalar::Field* pField_,
					   Scalar::DelayArgument& cArgument_);

//	virtual Order::Specification* getOrder();
 	virtual bool isReferingRelation(Interface::IRelation* pRelation_)
	{
		return isAny(boost::bind(&Operand::isReferingRelation,
								 _1,
								 pRelation_));
	}

	virtual bool isMergeSortAvailable()
	{
		return isAll(boost::bind(&Operand::isMergeSortAvailable,
								 _1));
	}
	
 	virtual void createReferingRelation(Utility::RelationSet& cRelationSet_)
	{
		foreachOperand(boost::bind(&Operand::createReferingRelation,
								   _1,
								   boost::ref(cRelationSet_)));
	}
//	virtual Candidate::Table* getCandidate(Opt::Environment& cEnvironment_,
//										   Interface::IRelation* pRelation_);
	virtual void generateDelayed(Opt::Environment& cEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Execution::Interface::IIterator* pIterator_)
	{
		foreachOperand(boost::bind(&Operand::generateDelayed,
								   _1,
								   boost::ref(cEnvironment_),
								   boost::ref(cProgram_),
								   pIterator_));
	}
	virtual Interface::ICandidate::InquiryResult
						inquiry(Opt::Environment& cEnvironment_,
								const InquiryArgument& cArgument_)
	{return 0;}

protected:
	// constructor
	Nadic(const VECTOR<Operand*>& vecOperand_)
		: Super(),
		  m_vecOperand(vecOperand_)
	{}
	template <class A_>
	Nadic(A_ a_, const VECTOR<Operand*>& vecOperand_)
		: Super(a_),
		  m_vecOperand(vecOperand_)
	{}
	template <class A1_, class A2_>
	Nadic(A1_ a1_, A2_ a2_, const VECTOR<Operand*>& vecOperand_)
		: Super(a1_, a2_),
		  m_vecOperand(vecOperand_)
	{}

	// accesor
	Operand* getOperandi(int iPos_) const {return m_vecOperand[iPos_];}	

	// TEMPLATE FUNCTION public
	//	Candidate::Nadic::foreachOperand -- 
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function_
	//	
	// RETURN
	//	Function_
	//
	// EXCEPTIONS

	template <class Function_>
	Function_ foreachOperand(Function_ function_)
	{
		return FOREACH(m_vecOperand, function_);
	}
	template <class Function_>
	Function_ foreachOperand_i(Function_ function_)
	{
		return FOREACH_i(m_vecOperand, function_);
	}

	// TEMPLATE FUNCTION public
	//	Candidate::Nadic::isAll -- 
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function_
	//	
	// RETURN
	//	bool
	//
	// EXCEPTIONS

	template <class Function_>
	bool isAll(Function_ function_)
	{
		return Opt::IsAll(m_vecOperand, function_);
	}

	// TEMPLATE FUNCTION public
	//	Candidate::bool Nadic::isAny -- 
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function_
	//	
	// RETURN
	//	bool
	//
	// EXCEPTIONS

	template <class Function_>
	bool isAny(Function_ function_)
	{
		return Opt::IsAny(m_vecOperand, function_);
	}

private:
/////////////////////////////
// Candidate::Base::
//	virtual void createCost(Opt::Environment& cEnvironment_,
//							const AccessPlan::Source& cPlanSource_,
//							AccessPlan::Cost& cCost_);
//	virtual Candidate::Row* createRow(Opt::Environment& cEnvironment_);
//	virtual Candidate::Row* createKey(Opt::Environment& cEnvironment_);

	VECTOR<Operand*> m_vecOperand;
	Utility::FieldSet m_cRetrievedColumn;
	Utility::FieldSet m_cRequiredColumn;
	Utility::FieldSet m_cDelayedColumn;
};

// TEMPLATE FUNCTION public
//	Candidate::Nadic<Base_>::require -- 
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
Nadic<Base_>::
require(Opt::Environment& cEnvironment_,
		Scalar::Field* pField_)
{
	foreachOperand(boost::bind(&Operand::require,
							   _1,
							   boost::ref(cEnvironment_),
							   pField_));
	m_cRequiredColumn.add(pField_);
}

// TEMPLATE FUNCTION public
//	Candidate::Nadic<Base_>::retrieve -- 
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
Nadic<Base_>::
retrieve(Opt::Environment& cEnvironment_,
		 Scalar::Field* pField_)
{
	foreachOperand(boost::bind(&Operand::retrieve,
							   _1,
							   boost::ref(cEnvironment_),
							   pField_));
}

// TEMPLATE FUNCTION public
//	Candidate::Nadic<Base_>::use -- 
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
Nadic<Base_>::
use(Opt::Environment& cEnvironment_,
	Scalar::Field* pField_)
{
	foreachOperand(boost::bind(&Operand::use,
							   _1,
							   boost::ref(cEnvironment_),
							   pField_));
}

// TEMPLATE FUNCTION public
//	Candidate::Nadic<Base_>::delay -- 
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
Nadic<Base_>::
delay(Opt::Environment& cEnvironment_,
	  Scalar::Field* pField_,
	  Scalar::DelayArgument& cArgument_)
{
	// check according to requirement for this candidate
	if (m_cRequiredColumn.isContaining(pField_) == false) {
		// this field is not required -> delay
		// [NOTES]
		//	delay is called only when the field can be delayed
		bool bMinimumSave = cArgument_.m_bMinimum;
		cArgument_.m_bMinimum = true;
		foreachOperand(boost::bind(&Operand::delay,
								   _1,
								   boost::ref(cEnvironment_),
								   pField_,
								   boost::ref(cArgument_)));
		cArgument_.m_bMinimum = bMinimumSave;
		return true;

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

#endif // __SYDNEY_PLAN_CANDIDATE_NADIC_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
