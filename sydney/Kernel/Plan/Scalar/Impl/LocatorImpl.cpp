// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/LocatorImpl.cpp --
// 
// Copyright (c) 2011, 2016, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Scalar::Impl";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Scalar/Impl/LocatorImpl.h"
#include "Plan/Scalar/Operation.h"

#include "Common/Assert.h"

#include "Execution/Function/Factory.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Operator/Locator.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

/////////////////////////////////////////
// Scalar::LocatorImpl::CharLength::

// FUNCTION protected
//	Scalar::LocatorImpl::CharLength::generateThis -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	int iDataID_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
LocatorImpl::CharLength::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	Candidate::AdoptArgument cTmpArgument(cArgument_);
	cTmpArgument.m_bLocator = true;

	int iOperandID =
		getOperand()->generate(cEnvironment_,
							   cProgram_,
							   pIterator_,
							   cTmpArgument);

	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::Locator::Length::create(
										   cProgram_,
										   pIterator_,
										   iOperandID,
										   iDataID_),
							   cArgument_.m_eTarget);
	return iDataID_;
}

/////////////////////////////////////////
// Scalar::LocatorImpl::StringConcatenate::

// FUNCTION public
//	Scalar::LocatorImpl::StringConcatenate::checkOperation -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pOperand_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
LocatorImpl::StringConcatenate::
checkOperation(Opt::Environment& cEnvironment_,
			   Interface::IScalar* pOperand_)
{
	// first operand is target column
	// -> can be converted into append operation
	return (pOperand_ == getOperand0());
}

// FUNCTION public
//	Scalar::LocatorImpl::StringConcatenate::createOperation -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pOperand_
//	
// RETURN
//	PAIR<Interface::IScalar*, Interface::IScalar*>
//
// EXCEPTIONS

//virtual
PAIR<Interface::IScalar*, Interface::IScalar*>
LocatorImpl::StringConcatenate::
createOperation(Opt::Environment& cEnvironment_,
				Interface::IScalar* pOperand_)
{
	Interface::IScalar* pOperation = 0;
	Interface::IScalar* pRevert = 0;

	if (pOperand_ == getOperand0()) {
		// first operand is target column
		// -> can be converted into append operation
		pOperation = Operation::Append::create(cEnvironment_,
											   getOperand0(),
											   getOperand1());
		// truncate by appended length to revert 
		Interface::IScalar* truncateLength =
			Function::create(cEnvironment_,
							 Tree::Node::CharLength,
							 getOperand1(),
							 STRING());
		pRevert = Operation::Truncate::create(cEnvironment_,
											  getOperand0(),
											  truncateLength);
	}

	return MAKEPAIR(pOperation, pRevert);
}

// FUNCTION protected
//	Scalar::LocatorImpl::StringConcatenate::generateThis -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	int iDataID_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
LocatorImpl::StringConcatenate::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	return Super::generateThis(cEnvironment_,
							   cProgram_,
							   pIterator_,
							   cArgument_,
							   iDataID_);
}

/////////////////////////////////////////
// Scalar::LocatorImpl::SubString::

// FUNCTION protected
//	Scalar::LocatorImpl::SubString::generateThis -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	int iDataID_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
LocatorImpl::SubString::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	Candidate::AdoptArgument cTmpArgument(cArgument_);
	cTmpArgument.m_bLocator = true;

	int iOperandID =
		getOperand()->generate(cEnvironment_,
							   cProgram_,
							   pIterator_,
							   cTmpArgument);
	OptionMapIntResult vecOptionID;
	mapOption(vecOptionID,
			  boost::bind(&Option::generate,
						  _1,
						  boost::ref(cEnvironment_),
						  boost::ref(cProgram_),
						  pIterator_,
						  boost::ref(cArgument_)));

	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::Locator::Get::create(
										   cProgram_,
										   pIterator_,
										   iOperandID,
										   vecOptionID,
										   iDataID_),
							   cArgument_.m_eTarget);
	return iDataID_;
}

/////////////////////////////////////////
// Scalar::LocatorImpl::Overlay::

// FUNCTION public
//	Scalar::LocatorImpl::Overlay::checkOperation -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pOperand_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
LocatorImpl::Overlay::
checkOperation(Opt::Environment& cEnvironment_,
			   Interface::IScalar* pOperand_)
{
	// no length is specified and first operand is target column
	// -> can be converted into replace operation
	return (getOptionSize() == 1 && pOperand_ == getOperand0());
}

// FUNCTION public
//	Scalar::LocatorImpl::Overlay::createOperation -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pOperand_
//	
// RETURN
//	PAIR<Interface::IScalar*, Interface::IScalar*>
//
// EXCEPTIONS

//virtual
PAIR<Interface::IScalar*, Interface::IScalar*>
LocatorImpl::Overlay::
createOperation(Opt::Environment& cEnvironment_,
				Interface::IScalar* pOperand_)
{
	Interface::IScalar* pOperation = 0;
	Interface::IScalar* pRevert = 0;

	if (getOptionSize() == 1 && pOperand_ == getOperand0()) {
		// no length is specified and first operand is target column
		// -> can be converted into replace operation
		VECTOR<Interface::IScalar*> vecOption1;
		vecOption1.PUSHBACK(getOperand1()); // placing
		vecOption1.PUSHBACK(getOptioni(0)); // from
		pOperation = Operation::Replace::create(cEnvironment_,
												getOperand0(),
												vecOption1);
		// replace by original substring to revert 
		Interface::IScalar* placingLength =
			Function::create(cEnvironment_,
							 Tree::Node::CharLength,
							 getOperand1(),
							 STRING());
		VECTOR<Interface::IScalar*> vecOption2;
		vecOption2.PUSHBACK(getOptioni(0)); // from
		vecOption2.PUSHBACK(placingLength); // length

		// get original part by substring
		Interface::IScalar* originalPart =
			Function::create(cEnvironment_,
							 Tree::Node::SubString,
							 pOperand_,
							 vecOption2,
							 STRING());

		VECTOR<Interface::IScalar*> vecOption3;
		vecOption3.PUSHBACK(originalPart); // placing
		vecOption3.PUSHBACK(getOptioni(0)); // from

		pRevert = Operation::Replace::create(cEnvironment_,
											 getOperand0(),
											 vecOption3);
	}

	return MAKEPAIR(pOperation, pRevert);
}

// FUNCTION protected
//	Scalar::LocatorImpl::Overlay::generateThis -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	int iDataID_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
LocatorImpl::Overlay::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	return Super::generateThis(cEnvironment_,
							   cProgram_,
							   pIterator_,
							   cArgument_,
							   iDataID_);
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
