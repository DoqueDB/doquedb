// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/OperationImpl.cpp --
// 
// Copyright (c) 2016, 2023 Ricoh Company, Ltd.
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

#include "Plan/Scalar/Impl/OperationImpl.h"
#include "Plan/Scalar/Operation.h"

#include "Common/Assert.h"
#include "Common/IntegerData.h"

#include "Execution/Function/Factory.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Operator/Locator.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

/////////////////////////////////////////
// Scalar::OperationImpl::Append::

// FUNCTION private
//	Scalar::OperationImpl::Append::generateSkipCheck -- 
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
OperationImpl::Append::
generateSkipCheck(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_,
				  Candidate::AdoptArgument& cArgument_)
{
	// generate for skip check -> use only option
	return getOption()->generate(cEnvironment_,
								 cProgram_,
								 pIterator_,
								 cArgument_);
}

// FUNCTION private
//	Scalar::OperationImpl::Append::generateLogData -- 
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
OperationImpl::Append::
generateLogData(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_,
				Candidate::AdoptArgument& cArgument_)
{
	// generate data array data to express the operation
	VECTOR<int> vecData;
	vecData.PUSHBACK(cProgram_.addVariable(new Common::IntegerData(_eType)));
	vecData.PUSHBACK(getOption()->generate(cEnvironment_,
										   cProgram_,
										   pIterator_,
										   cArgument_));
	return cProgram_.addVariable(vecData);
}

// FUNCTION private
//	Scalar::OperationImpl::Append::generateOperation -- 
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
//	Nothing
//
// EXCEPTIONS

//virtual
void
OperationImpl::Append::
generateOperation(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_,
				  Candidate::AdoptArgument& cArgument_)
{
	Candidate::AdoptArgument cTmpArgument(cArgument_);
	cTmpArgument.m_bLocator = true;

	int iOperandID = getOperand()->generate(cEnvironment_,
											cProgram_,
											pIterator_,
											cTmpArgument);
	int iOptionID = getOption()->generate(cEnvironment_,
										  cProgram_,
										  pIterator_,
										  cArgument_);

	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::Locator::Append::create(
										   cProgram_,
										   pIterator_,
										   iOperandID,
										   iOptionID),
							   cArgument_.m_eTarget);
}

/////////////////////////////////////////
// Scalar::OperationImpl::Truncate::

// FUNCTION private
//	Scalar::OperationImpl::Truncate::generateSkipCheck -- 
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
OperationImpl::Truncate::
generateSkipCheck(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_,
				  Candidate::AdoptArgument& cArgument_)
{
	// generate for skip check -> use only option
	return getOption()->generate(cEnvironment_,
								 cProgram_,
								 pIterator_,
								 cArgument_);
}

// FUNCTION private
//	Scalar::OperationImpl::Truncate::generateLogData -- 
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
OperationImpl::Truncate::
generateLogData(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_,
				Candidate::AdoptArgument& cArgument_)
{
	// generate data array data to express the operation
	VECTOR<int> vecData;
	vecData.PUSHBACK(cProgram_.addVariable(new Common::IntegerData(_eType)));
	vecData.PUSHBACK(getOption()->generate(cEnvironment_,
										   cProgram_,
										   pIterator_,
										   cArgument_));
	return cProgram_.addVariable(vecData);
}

// FUNCTION private
//	Scalar::OperationImpl::Truncate::generateOperation -- 
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
//	Nothing
//
// EXCEPTIONS

//virtual
void
OperationImpl::Truncate::
generateOperation(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_,
				  Candidate::AdoptArgument& cArgument_)
{
	Candidate::AdoptArgument cTmpArgument(cArgument_);
	cTmpArgument.m_bLocator = true;

	int iOperandID = getOperand()->generate(cEnvironment_,
											cProgram_,
											pIterator_,
											cTmpArgument);
	int iOptionID = getOption()->generate(cEnvironment_,
										  cProgram_,
										  pIterator_,
										  cArgument_);

	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::Locator::Truncate::create(
										   cProgram_,
										   pIterator_,
										   iOperandID,
										   iOptionID),
							   cArgument_.m_eTarget);
}

/////////////////////////////////////////
// Scalar::OperationImpl::Replace::

// FUNCTION private
//	Scalar::OperationImpl::Replace::generateSkipCheck -- 
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
OperationImpl::Replace::
generateSkipCheck(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_,
				  Candidate::AdoptArgument& cArgument_)
{
	// generate for skip check -> generate options
	VECTOR<int> vecData;
	foreachOption(boost::bind(&VECTOR<int>::PUSHBACK,
							  boost::ref(vecData),
							  boost::bind(&Option::generate,
										  _1,
										  boost::ref(cEnvironment_),
										  boost::ref(cProgram_),
										  pIterator_,
										  boost::ref(cArgument_))));
	return cProgram_.addVariable(vecData);
}

// FUNCTION private
//	Scalar::OperationImpl::Replace::generateLogData -- 
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
OperationImpl::Replace::
generateLogData(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_,
				Candidate::AdoptArgument& cArgument_)
{
	// generate data array data to express the operation
	VECTOR<int> vecData;
	vecData.PUSHBACK(cProgram_.addVariable(new Common::IntegerData(_eType)));
	foreachOption(boost::bind(&VECTOR<int>::PUSHBACK,
							  boost::ref(vecData),
							  boost::bind(&Option::generate,
										  _1,
										  boost::ref(cEnvironment_),
										  boost::ref(cProgram_),
										  pIterator_,
										  boost::ref(cArgument_))));
	return cProgram_.addVariable(vecData);
}

// FUNCTION private
//	Scalar::OperationImpl::Replace::generateOperation -- 
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
//	Nothing
//
// EXCEPTIONS

//virtual
void
OperationImpl::Replace::
generateOperation(Opt::Environment& cEnvironment_,
				  Execution::Interface::IProgram& cProgram_,
				  Execution::Interface::IIterator* pIterator_,
				  Candidate::AdoptArgument& cArgument_)
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
							   Execution::Operator::Locator::Replace::create(
										   cProgram_,
										   pIterator_,
										   iOperandID,
										   vecOptionID),
							   cArgument_.m_eTarget);
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
