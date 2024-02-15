// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/ArrayImpl.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Scalar/Impl/ArrayImpl.h"
#include "Plan/Scalar/Function.h"

#include "Common/Assert.h"

#include "Execution/Action/Argument.h"
#include "Execution/Function/Cast.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

/////////////////////////////////////////////
//	Scalar::ArrayImpl::Constructor::Base

// FUNCTION public
//	Scalar::ArrayImpl::Constructor::Base::generate -- 
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
ArrayImpl::Constructor::Base::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	// get variable ID corresponding to the field
	int iDataID = getNodeVariable(pIterator_, cArgument_);
	if (iDataID < 0) {
		VECTOR<int> vecOperandData;
		DataType cElementType = DataType::getElementType(getDataType());

		// generate operand data
		generateElements(cEnvironment_,
						 cProgram_,
						 pIterator_,
						 cArgument_,
						 cElementType,
						 vecOperandData);

		// result is array data constructed from element data
		iDataID = cProgram_.addVariable(vecOperandData);
		// register field <-> data relationship
		setNodeVariable(pIterator_, cArgument_, iDataID);
	}
	return iDataID;
}

// FUNCTION protected
//	Scalar::ArrayImpl::Constructor::Base::getCompatibleType -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IScalar* pOperand_
//	DataType& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
ArrayImpl::Constructor::Base::
getCompatibleType(Interface::IScalar* pOperand_,
				  DataType& cResult_)
{
	DataType cTmp(cResult_);
	return DataType::getCompatibleType(pOperand_->getDataType(),
									   cTmp,
									   cResult_,
									   false);
}

// FUNCTION protected
//	Scalar::ArrayImpl::Constructor::Base::generateElement -- generate element data
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	const DataType& cElementType_
//	Interface::IScalar* pElement_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
ArrayImpl::Constructor::Base::
generateElement(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_,
				Candidate::AdoptArgument& cArgument_,
				const DataType& cElementType_,
				Interface::IScalar* pElement_)
{
	int iResult = pElement_->generate(cEnvironment_,
									  cProgram_,
									  pIterator_,
									  cArgument_);
	if (DataType::isAssignable(pElement_->getDataType(),
							   cElementType_) == false) {
		// add cast operation
		int iCastTarget = cProgram_.addVariable(cElementType_.createData());
		pIterator_->addCalculation(cProgram_,
								   Execution::Function::Cast::create(
										 cProgram_,
										 pIterator_,
										 pElement_->getDataType(),
										 cElementType_,
										 iResult,
										 iCastTarget),
								   cArgument_.m_eTarget);
		iResult = iCastTarget;
	}
	return iResult;
}

/////////////////////////////////////////////
//	Scalar::ArrayImpl::Element::Base

// FUNCTION private
//	Scalar::ArrayImpl::Element::Base::createDataType -- 
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
ArrayImpl::Element::Base::
createDataType(Opt::Environment& cEnvironment_)
{
	; _SYDNEY_ASSERT(getOperand()->getDataType().isArrayType());
	setDataType(DataType::getElementType(getOperand()->getDataType()));
}

/////////////////////////////////////////////
//	Scalar::ArrayImpl::Element::Reference

// FUNCTION protected
//	Scalar::ArrayImpl::Element::Reference::generateThis -- 
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
ArrayImpl::Element::Reference::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	int iOperandDataID = getOperand()->generate(cEnvironment_,
												cProgram_,
												pIterator_,
												cArgument_);
	int iOptionDataID = getOption()->generate(cEnvironment_,
											  cProgram_,
											  pIterator_,
											  cArgument_);
	pIterator_->addCalculation(cProgram_,
							   Execution::Function::Factory::create(cProgram_,
																	pIterator_,
																	getType(),
																	iOperandDataID,
																	iOptionDataID,
																	iDataID_),
							   cArgument_.m_eTarget);
	return iDataID_;
}

/////////////////////////////////////////////
//	Scalar::ArrayImpl::Element::Arbitrary

// FUNCTION public
//	Scalar::ArrayImpl::Element::Arbitrary::isArbitraryElement -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ArrayImpl::Element::Arbitrary::
isArbitraryElement()
{
	return true;
}

// FUNCTION public
//	Scalar::ArrayImpl::Element::Arbitrary::generate -- 
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
ArrayImpl::Element::Arbitrary::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	// use operand data
	return getOperand()->generate(cEnvironment_,
								  cProgram_,
								  pIterator_,
								  cArgument_);
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
