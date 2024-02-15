// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/GeneratorImpl.cpp --
// 
// Copyright (c) 2010, 2011, 2015, 2023 Ricoh Company, Ltd.
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

#include "Plan/Scalar/Impl/GeneratorImpl.h"
#include "Plan/Candidate/Argument.h"

#include "Execution/Action/Argument.h"
#include "Execution/Function/Factory.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Operator/Generator.h"

#include "Exception/NotSupported.h"

#include "Schema/Column.h"
#include "Schema/Table.h"

#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////////
//	Scalar::GeneratorImpl::RowID

// FUNCTION private
//	Scalar::GeneratorImpl::RowID::generateThis -- 
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
GeneratorImpl::RowID::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	if (!cArgument_.m_bForLock) {
		// add generator action
		pIterator_->addCalculation(cProgram_,
								   Execution::Operator::Generator::RowID::create(
										 cProgram_,
										 pIterator_,
										 m_pSchemaTable,
										 iDataID_),
								   cArgument_.m_eTarget);
	}
	return iDataID_;
}

////////////////////////////////////////
//	Scalar::GeneratorImpl::Identity

// FUNCTION private
//	Scalar::GeneratorImpl::Identity::generateThis -- 
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
GeneratorImpl::Identity::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	Execution::Interface::IAction* pGenerator = 0;
	if (m_pInput) {
		// generate input data
		int iInputID = m_pInput->generate(cEnvironment_,
										  cProgram_,
										  pIterator_,
										  cArgument_);
		pGenerator = Execution::Operator::Generator::Identity::create(
									cProgram_,
									pIterator_,
									m_pSchemaColumn,
									iInputID,
									iDataID_);
	} else {
		pGenerator = Execution::Operator::Generator::Identity::create(
									cProgram_,
									pIterator_,
									m_pSchemaColumn,
									iDataID_);
	}
	// add generator action
	pIterator_->addCalculation(cProgram_,
							   pGenerator,
							   cArgument_.m_eTarget);
	return iDataID_;
}

////////////////////////////////////////
//	Scalar::GeneratorImpl::Function

// FUNCTION private
//	Scalar::GeneratorImpl::Function::generateThis -- 
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
GeneratorImpl::Function::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	const Schema::Default& cDefault = m_pSchemaColumn->getDefault();
	switch (cDefault.getFunction()) {
	case Statement::ValueExpression::func_Current_Timestamp:
		{
			// add generator action
			pIterator_->addCalculation(cProgram_,
									   Execution::Function::Factory::create(
											 cProgram_,
											 pIterator_,
											 Tree::Node::CurrentTimestamp,
											 iDataID_),
									   cArgument_.m_eTarget);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	return iDataID_;
}

////////////////////////////////////////
//	Scalar::GeneratorImpl::RecoveryRowID

// FUNCTION private
//	Scalar::GeneratorImpl::RecoveryRowID::generateData -- 
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
GeneratorImpl::RecoveryRowID::
generateData(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	return m_pInput->generate(cEnvironment_,
							  cProgram_,
							  pIterator_,
							  cArgument_);
}

// FUNCTION private
//	Scalar::GeneratorImpl::RecoveryRowID::generateThis -- 
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
GeneratorImpl::RecoveryRowID::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	// add generator action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::Generator::RowID::Recovery::create(
									cProgram_,
									pIterator_,
									m_pSchemaTable,
									iDataID_),
							   cArgument_.m_eTarget);
	return iDataID_;
}

////////////////////////////////////////
//	Scalar::GeneratorImpl::RecoveryIdentity

// FUNCTION private
//	Scalar::GeneratorImpl::RecoveryIdentity::generateData -- 
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
GeneratorImpl::RecoveryIdentity::
generateData(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	return m_pInput->generate(cEnvironment_,
							  cProgram_,
							  pIterator_,
							  cArgument_);
}

// FUNCTION private
//	Scalar::GeneratorImpl::RecoveryIdentity::generateThis -- 
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
GeneratorImpl::RecoveryIdentity::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	// add generator action
	pIterator_->addCalculation(cProgram_,
							   Execution::Operator::Generator::Identity::Recovery::create(
									cProgram_,
									pIterator_,
									m_pSchemaColumn,
									iDataID_),
							   cArgument_.m_eTarget);
	return iDataID_;
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
