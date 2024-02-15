// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/AggregationImpl.cpp --
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
const char moduleName[] = "Plan::Scalar::Impl";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Scalar/Impl/AggregationImpl.h"
#include "Plan/Scalar/Function.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Interface/IRelation.h"
#include "Plan/Interface/ICandidate.h"


#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Execution/Action/Argument.h"
#include "Execution/Function/Aggregation.h"
#include "Execution/Interface/IIterator.h"

#include "Opt/Environment.h"

#include "Schema/Field.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////////////////////
// AggregationImpl::Monadic::

// FUNCTION public
//	Scalar::AggregationImpl::Monadic::convertFunction -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation_
//	Interface::IScalar* pFunction_
//	Schema::Field::Function::Value eFunction_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//virtual
Interface::IScalar*
AggregationImpl::Monadic::
convertFunction(Opt::Environment& cEnvironment_,
				Interface::IRelation* pRelation_,
				Interface::IScalar* pFunction_,
				Schema::Field::Function::Value eFunction_)
{
	if (eFunction_ == Schema::Field::Function::Undefined) {
		Schema::Field::Function::Value eFunction = eFunction_;
		Interface::IScalar* pOperand = getOperand();

		switch (getType()) {
		case Tree::Node::Count:
			{
				eFunction = Schema::Field::Function::Count;
				if (isCountAll(cEnvironment_)) {
					// rowid
					pOperand = pRelation_->getScalar(cEnvironment_, 0);
				}
				break;
			}
		case Tree::Node::Max:
			{
				eFunction = Schema::Field::Function::MaxKey;
				break;
			}
		case Tree::Node::Min:
			{
				eFunction = Schema::Field::Function::MinKey;
				break;
			}
		case Tree::Node::Avg:
			{
				break;
			}
		default:
			{
				return false;
			}
		}
		return m_pConverted = pOperand->convertFunction(cEnvironment_,
														pRelation_,
														pFunction_,
														eFunction);
	}
	return 0;
}




// FUNCTION public
//	Scalar::AggregationImpl::Monadic::clearConvert -- 
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
AggregationImpl::Monadic::
clearConvert(Opt::Environment& cEnvironment_)
{
	if (m_pConverted) {
		m_pConverted = 0;
	}
}

// FUNCTION public
//	Scalar::AggregationImpl::Monadic::generate -- 
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
AggregationImpl::Monadic::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	if (m_pConverted) {
		return m_pConverted->generate(cEnvironment_,
									  cProgram_,
									  pIterator_,
									  cArgument_);
	} else if (m_pConvertedForDist) {
		return m_pConvertedForDist->generate(cEnvironment_,
											 cProgram_,
											 pIterator_,
											 cArgument_);
	}
		
	return Super::generate(cEnvironment_,
						   cProgram_,
						   pIterator_,
						   cArgument_);
}

// FUNCTION public
//	Scalar::AggregationImpl::Monadic::delay -- 
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
AggregationImpl::Monadic::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  Scalar::DelayArgument& cArgument_)
{
	// aggregation can't be delayed
	return false;
}

// FUNCTION protected
//	Scalar::AggregationImpl::Monadic::generateThis -- generate
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
AggregationImpl::Monadic::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_eTarget = Candidate::AdoptArgument::Target::Aggregation;
	return generateFunction(cEnvironment_,
							cProgram_,
							pIterator_,
							cMyArgument,
							iDataID_);
}

// FUNCTION protected
//	Scalar::AggregationImpl::Monadic::generateFunction -- generate main
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

int
AggregationImpl::Monadic::
generateFunction(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_,
				 int iDataID_)
{

	Execution::Interface::IAction* pResult = 0;
	int iOperandID = getOperand()->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);

	switch (getType()) {
	case Tree::Node::Count:
		{
			
			Scalar::Field* pField = getBitSetField(cEnvironment_, cArgument_);
			if (pField) {
				iOperandID = pField->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);
				pResult = Execution::Function::Aggregation::BitSetCount::create(cProgram_,
																				pIterator_,
																				iOperandID,
																				iDataID_);
			} else {
				pResult = Execution::Function::Aggregation::Count::create(cProgram_,
																		  pIterator_,
																		  iOperandID,
																		  iDataID_);
			}
			break;
		}
	case Tree::Node::Avg:
		{
			pResult = Execution::Function::Aggregation::Avg::create(cProgram_,
																	pIterator_,
																	iOperandID,
																	iDataID_);
			break;
		}
	case Tree::Node::Max:
		{
			pResult = Execution::Function::Aggregation::Max::create(cProgram_,
																	pIterator_,
																	iOperandID,
																	iDataID_);
			break;
		}
	case Tree::Node::Min:
		{
			pResult = Execution::Function::Aggregation::Min::create(cProgram_,
																	pIterator_,
																	iOperandID,
																	iDataID_);
			break;
		}
	case Tree::Node::Sum:
		{
			pResult = Execution::Function::Aggregation::Sum::create(cProgram_,
																	pIterator_,
																	iOperandID,
																	iDataID_);
			break;
		}
	case Tree::Node::Distinct:
		{
			pResult = Execution::Function::Aggregation::Distinct::create(cProgram_,
																		 pIterator_,
																		 iOperandID,
																		 iDataID_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}

	// add aggregate action
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT1(Aggregation,
											pResult->getID()));

	return iDataID_;
}

// FUNCTION protected
//	Scalar::AggregationImpl::Monadic::createDataType -- 
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
AggregationImpl::Monadic::
createDataType(Opt::Environment& cEnvironment_)
{
	switch (getType()) {
	case Tree::Node::Count:
		{
			// If this result type is changed, logical file I/F has to be changed
			setDataType(DataType::getUnsignedIntegerType());
			break;
		}
	case Tree::Node::Sum:
	case Tree::Node::Avg:
	case Tree::Node::Max:
	case Tree::Node::Min:
	case Tree::Node::Distinct:
	case Tree::Node::Word:
		{
			// same as operand
			setDataType(getOperand()->getDataType());
			break;
		}
	}
}

// FUNCTION private
//	Scalar::AggregationImpl::Monadic::isCountAll -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
AggregationImpl::Monadic::
isCountAll(Opt::Environment& cEnvironment_)
{
	if (getOperand()->getType() == Tree::Node::ConstantValue) {
		Common::Data::Pointer pData = getOperand()->preCalculate(cEnvironment_);
		return pData.get() && pData->isNull() == false;
	} else {
		return cEnvironment_.isKnownNotNull(getOperand());
	}
}


// FUNCTION private
//	Scalar::AggregationImpl::Monadic::isBitSetCount -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

Scalar::Field*
AggregationImpl::Monadic::
getBitSetField(Opt::Environment& cEnvironment_,
			   Candidate::AdoptArgument& cArgument_)
{
	if (cArgument_.m_pCandidate
		&& isCountAll(cEnvironment_)
		&& cArgument_.m_bForceRowIDSet == false) {
		Utility::RelationSet cRelationSet;
		cArgument_.m_pCandidate->createReferingRelation(cRelationSet);
		if (cRelationSet.getSize() == 1) {
			Interface::IRelation* pRelation = (cRelationSet.getVector())[0];
			if (pRelation->getType() == Tree::Node::Table) {
				Interface::IScalar* pScalar = pRelation->getScalar(cEnvironment_, 0);
				if (pScalar
					&& pScalar->isField()) {
					Scalar::Field* pField = pScalar->getField();
					pField = cArgument_.m_pCandidate->getUsedField(cEnvironment_, pField);
					if (pField
						&& pField->isBitSetRowID()) {
						return pField;
					}
				}
			}
		}
	}
	return 0;
}


// FUNCTION protected
//	Scalar::AggregationImpl::MonadicDistribution::generateThis -- generate
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
AggregationImpl::MonadicDistribution::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	Execution::Interface::IAction* pResult = 0;
	int iOperandID = getOperand()->generateFromType(cEnvironment_, cProgram_, pIterator_, cArgument_);

	switch (getType()) {
	case Tree::Node::Count:
		{
			pResult = Execution::Function::Aggregation::Count::create(cProgram_,
																	  pIterator_,
																	  iOperandID,
																	  iDataID_);
			break;
		}
	case Tree::Node::Max:
		{
			pResult = Execution::Function::Aggregation::Max::create(cProgram_,
																	pIterator_,
																	iOperandID,
																	iDataID_);
			break;
		}
	case Tree::Node::Min:
		{
			pResult = Execution::Function::Aggregation::Min::create(cProgram_,
																	pIterator_,
																	iOperandID,
																	iDataID_);
			break;
		}
	case Tree::Node::Sum:
		{
			pResult = Execution::Function::Aggregation::Sum::create(cProgram_,
																	pIterator_,
																	iOperandID,
																	iDataID_);
			break;
		}
	case Tree::Node::Word:
		{
			pResult = Execution::Function::Aggregation::Word::create(cProgram_,
																	 pIterator_,
																	 iOperandID,
																	 iDataID_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}

	// add aggregate action
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT1(Aggregation,
											pResult->getID()));

	return iDataID_;
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
