// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/UpdateFieldImpl.cpp --
// 
// Copyright (c) 2015, 2016, 2023 Ricoh Company, Ltd.
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

#include "Plan/Scalar/Impl/UpdateFieldImpl.h"
#include "Plan/Scalar/Argument.h"
#include "Plan/Scalar/CheckedField.h"

#include "Plan/Sql/Argument.h"

#include "Plan/Candidate/Argument.h"
#include "Plan/Relation/Table.h"

#include "Common/Assert.h"

#include "DExecution/Action/StatementConstruction.h"

#include "Exception/Unexpected.h"

#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

// FUNCTION public
//	Scalar::UpdateFieldImpl::Base::getField -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IFile* pFile_
//	
// RETURN
//	Field*
//
// EXCEPTIONS

//virtual
Field*
UpdateFieldImpl::Base::
getField(Interface::IFile* pFile_)
{
	return getFile() == pFile_ ? this : getTable()->getField(pFile_, this);
}

// FUNCTION public
//	Scalar::UpdateFieldImpl::Base::checkPut -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Field*
//
// EXCEPTIONS

//virtual
Field*
UpdateFieldImpl::Base::
checkPut(Opt::Environment& cEnvironment_)
{
	if (m_pChecked == 0) {
		Utility::FileSet cFileSet;
		(void) Field::getPutFile(cEnvironment_,
								 GetFileArgument(this,
												 0,
												 cFileSet));
		m_pChecked = CheckedField::create(cEnvironment_,
										  this,
										  cFileSet);
	}
	return m_pChecked;
}

// FUNCTION protected
//	Scalar::UpdateFieldImpl::Base::generateOperand -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	Interface::IScalar* pOperand_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
UpdateFieldImpl::Base::
generateOperand(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_,
				Candidate::AdoptArgument& cArgument_,
				Interface::IScalar* pOperand_)
{
	if (DataType::isAssignable(pOperand_->getDataType(),
							   getDataType()) == false) {
		// need cast
		pOperand_ = pOperand_->createCast(cEnvironment_,
										  getDataType(),
										  false /* not for comparison */);
	}
	return pOperand_->generate(cEnvironment_,
							   cProgram_,
							   pIterator_,
							   cArgument_);
}

///////////////////////////////////
// UpdateFieldImpl::Insert

// FUNCTION public
//	Scalar::UpdateFieldImpl::Insert::generate -- 
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
UpdateFieldImpl::Insert::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	if (cArgument_.m_bOriginal == false) {
		if (cArgument_.m_bSkipCheck == true
			&& m_pInput->isOperation()) {
			// generate operand apartedly
			return generateOperand(cEnvironment_,
								   cProgram_,
								   pIterator_,
								   cArgument_,
								   m_pInput);
		}

		// get variable ID corresponding to the field
		int iDataID = pIterator_->getNodeVariable(getWrappedField()->getID());

		if (iDataID < 0) {
			iDataID = generateOperand(cEnvironment_,
									  cProgram_,
									  pIterator_,
									  cArgument_,
									  m_pInput);
			// register field <-> data relationship before generating
			pIterator_->setNodeVariable(getWrappedField()->getID(), iDataID, pIterator_);
		}
		return iDataID;

	} else {
		// do nothing
		return -1;
	}
	// never reach
}

// FUNCTION public
//	Scalar::UpdateFieldImpl::Insert::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
UpdateFieldImpl::Insert::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	if (cArgument_.isFirstUpdateField(this)
		&& m_pInput->isSubquery()) {
		cExec.append(" (");
	}
	if (m_pInput->isSubquery()
		&& cArgument_.isLastUpdateField(this)) {
		cExec.append(") =");
	} else if(!m_pInput->isSubquery()) {
		cExec.append(" = ");
	}
	if (!m_pInput->isSubquery() ||
		cArgument_.isLastUpdateField(this)) {
		m_pInput->setParameter(cEnvironment_,
							   cProgram_,
							   pIterator_,
							   cExec,
							   cArgument_);
	}
}

///////////////////////////////////
// UpdateFieldImpl::Delete

// FUNCTION public
//	Scalar::UpdateFieldImpl::Delete::generate -- 
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
UpdateFieldImpl::Delete::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	if (cArgument_.m_bOriginal == false) {
		// get variable ID corresponding to the field
		int iDataID = pIterator_->getNodeVariable(getWrappedField()->getID());

		if (iDataID < 0) {
			iDataID = cProgram_.addVariable(getDataType().createData());

			// register field <-> data relationship before generating
			pIterator_->setNodeVariable(getWrappedField()->getID(), iDataID, pIterator_);
		}
		return iDataID;

	} else {
		if (m_pOriginal) {
			return generateOperand(cEnvironment_,
								   cProgram_,
								   pIterator_,
								   cArgument_,
								   m_pOriginal);
		} else {
			// do nothing
			return -1;
		}
	}
	// never reach
}

// FUNCTION public
//	Scalar::UpdateFieldImpl::Delete::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
UpdateFieldImpl::Delete::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	Super::setParameter(cEnvironment_,
						cProgram_,
						pIterator_,
						cExec,
						cArgument_);
}

///////////////////////////////////
// UpdateFieldImpl::Update

// FUNCTION public
//	Scalar::UpdateFieldImpl::Update::generate -- 
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
UpdateFieldImpl::Update::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	if (cArgument_.m_bOriginal == false) {

		if (cArgument_.m_bSkipCheck == true
			&& m_pInput->isOperation()) {
			// generate operand apartedly
			return generateOperand(cEnvironment_,
								   cProgram_,
								   pIterator_,
								   cArgument_,
								   m_pInput);
		}

		// get variable ID corresponding to the field
		int iDataID = pIterator_->getNodeVariable(getWrappedField()->getID());

		if (iDataID < 0) {
			if (m_pInput) {
				iDataID = generateOperand(cEnvironment_,
										  cProgram_,
										  pIterator_,
										  cArgument_,
										  m_pInput);
			} else {
				iDataID = cProgram_.addVariable(getDataType().createData());
			}
			// register field <-> data relationship before generating
			pIterator_->setNodeVariable(getWrappedField()->getID(), iDataID, pIterator_);
		}
		return iDataID;

	} else {
		if (m_pOriginal) {
			return generateOperand(cEnvironment_,
								   cProgram_,
								   pIterator_,
								   cArgument_,
								   m_pOriginal);
		} else {
			// do nothing
			return -1;
		}
	}
	// never reach
}

// FUNCTION public
//	Scalar::UpdateFieldImpl::Update::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
UpdateFieldImpl::Update::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	if (m_pInput) {
		if (cArgument_.isFirstUpdateField(this)
			&& m_pInput->isSubquery()) {
			cExec.append(" (");
		}

		if (m_pOriginal) {
			m_pOriginal->setParameter(cEnvironment_,
									  cProgram_,
									  pIterator_,
									  cExec,
									  cArgument_);
		}
		if (m_pInput->isSubquery()
			&& cArgument_.isLastUpdateField(this)) {
			cExec.append(") =");
		} else if(!m_pInput->isSubquery()) {
			cExec.append(" = ");
		}
		if (!m_pInput->isSubquery() ||
			cArgument_.isLastUpdateField(this)) {
			m_pInput->setParameter(cEnvironment_,
								   cProgram_,
								   pIterator_,
								   cExec,
								   cArgument_);
		}
	} else {
		Super::setParameter(cEnvironment_,
							cProgram_,
							pIterator_,
							cExec,
							cArgument_);
	}
}

////////////////////////////////////////////
// UpdateFieldImpl::UpdateByOperation

// FUNCTION public
//	Scalar::UpdateFieldImpl::UpdateByOperation::setOperation -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	UpdateFieldImpl::UpdateByOperation*
//
// EXCEPTIONS

UpdateFieldImpl::UpdateByOperation*
UpdateFieldImpl::UpdateByOperation::
setOperation(Opt::Environment& cEnvironment_)
{
	if (m_pOperation == 0) {
		Interface::IScalar* pInput = Super::getInput();
		Interface::IScalar* pOriginal = Super::getOriginal();

		if (pInput
			&& pInput->checkOperation(cEnvironment_, pOriginal)) {
			// generate operation node
			PAIR<Interface::IScalar*, Interface::IScalar*> cPair =
				pInput->createOperation(cEnvironment_, pOriginal);
			m_pOperation = cPair.first;
			m_pRevert = cPair.second;
		}
	}
	return this;
}

// FUNCTION public
//	Scalar::UpdateFieldImpl::UpdateByOperation::getInput -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
UpdateFieldImpl::UpdateByOperation::
getInput()
{
	; _SYDNEY_ASSERT(m_pOperation);

	return m_pOperation;
}

// FUNCTION public
//	Scalar::UpdateFieldImpl::UpdateByOperation::getOriginal -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
UpdateFieldImpl::UpdateByOperation::
getOriginal()
{
	; _SYDNEY_ASSERT(m_pRevert);

	return m_pRevert;
}

// FUNCTION public
//	Scalar::UpdateFieldImpl::UpdateByOperation::generate -- 
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
UpdateFieldImpl::UpdateByOperation::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	if (cArgument_.m_bOriginal == false) {
		; _SYDNEY_ASSERT(m_pOperation);

		if (cArgument_.m_bSkipCheck == true
			|| cArgument_.m_bOperation == true) {
			// generate operand without cache
			return generateOperand(cEnvironment_,
								   cProgram_,
								   pIterator_,
								   cArgument_,
								   m_pOperation);
		}

		// get variable ID corresponding to the field
		int iDataID = pIterator_->getNodeVariable(getWrappedField()->getID());

		if (iDataID < 0) {
			iDataID = generateOperand(cEnvironment_,
									  cProgram_,
									  pIterator_,
									  cArgument_,
									  m_pOperation);
			// register field <-> data relationship before generating
			pIterator_->setNodeVariable(getWrappedField()->getID(), iDataID, pIterator_);
		}
		return iDataID;

	} else {
		; _SYDNEY_ASSERT(m_pRevert);

		return generateOperand(cEnvironment_,
							   cProgram_,
							   pIterator_,
							   cArgument_,
							   m_pRevert);
	}
	// never reach
}

// FUNCTION public
//	Scalar::UpdateFieldImpl::UpdateByOperation::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
UpdateFieldImpl::UpdateByOperation::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Scalar::UpdateFieldImpl::UpdateByOperation::isOperation -- 
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
UpdateFieldImpl::UpdateByOperation::
isOperation()
{
	return true;
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2015, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
