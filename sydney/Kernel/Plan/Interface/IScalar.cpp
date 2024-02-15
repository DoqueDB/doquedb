// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/IScalar.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2016, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Interface";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Interface/IScalar.h"
#include "Plan/Candidate/Argument.h"

#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_INTERFACE_USING

//////////////////////////////////////
//	Plan::Interface::IScalar::Check

// FUNCTION public
//	Interface::IScalar::Check::mergeValue -- 
//
// NOTES
//
// ARGUMENTS
//	Value* pValue1_
//	Value eValue2_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
IScalar::Check::
mergeValue(Value* pValue1_, Value eValue2_)
{
	*pValue1_ |= eValue2_;
}

////////////////////////////////////
//	Plan::Interface::IScalar

// FUNCTION public
//	Interface::IScalar::getDataType -- accessor
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Scalar::DataType&
//
// EXCEPTIONS

const Scalar::DataType&
IScalar::
getDataType() const
{
	return m_cType;
}

// FUNCTION public
//	Interface::IScalar::convertFunction -- create virtual column for simple aggregation
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
IScalar::
convertFunction(Opt::Environment& cEnvironment_,
				Interface::IRelation* pRelation_,
				Interface::IScalar* pFunction_,
				Schema::Field::Function::Value eFunction_)
{
	// default: can't convert
	return 0;
}

// FUNCTION public
//	Interface::IScalar::clearConvert -- clear converted virtual column
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
IScalar::
clearConvert(Opt::Environment& cEnvironment_)
{
	// default: do nothing
	;
}

// FUNCTION public
//	Interface::IScalar::getPosition -- get position in the relation
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
IScalar::
getPosition(Interface::IRelation* pRelation_)
{
	// default: can't find
	return -1;
}

// FUNCTION public
//	Interface::IScalar::generate -- generate variable
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
IScalar::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	// get variable ID corresponding to the field
	int iDataID = getNodeVariable(pIterator_,
								  cArgument_);

	if (iDataID < 0) {
		// if field value can be obtained from input data, use it
		if (cArgument_.m_pInput) {
			iDataID = pIterator_->copyNodeVariable(cArgument_.m_pInput,
												   getID(),
												   cArgument_.m_bCollecting);
		}
		if (iDataID < 0) {
			// add new variable
			iDataID = generateData(cEnvironment_,
								   cProgram_,
								   pIterator_,
								   cArgument_);

			if (iDataID >= 0) {
				// register field <-> data relationship before generating
				setNodeVariable(pIterator_,
								cArgument_,
								iDataID);
			}

			// generate extra action if necessary
			iDataID = generateThis(cEnvironment_,
								   cProgram_,
								   pIterator_,
								   cArgument_,
								   iDataID);
		}
	}
	return iDataID;
}

// FUNCTION public
//	Interface::IScalar::generateFromType -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
IScalar::
generateFromType(Opt::Environment& cEnvironment_,
				 Execution::Interface::IProgram& cProgram_,
				 Execution::Interface::IIterator* pIterator_,
				 Candidate::AdoptArgument& cArgument_)
{
	// get variable ID corresponding to the field
	int iDataID = pIterator_->getNodeVariable(getID());
	if (iDataID < 0) {
		// add new variable
		iDataID = cProgram_.addVariable(getDataType().createData());
		// register field <-> data relationship
		pIterator_->setNodeVariable(getID(), iDataID, pIterator_);
	}
	return iDataID;
}

// FUNCTION public
//	Interface::IScalar::checkOperation -- can create equivalent operation
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
IScalar::
checkOperation(Opt::Environment& cEnvironment_,
			   Interface::IScalar* pOperand_)
{
	// default: cannot convert into operation
	return false;
}

// FUNCTION public
//	Interface::IScalar::createOperation -- create equivalent operation if available
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pOperand
//	
// RETURN
//	PAIR<IScalar*, IScalar*>
//
// EXCEPTIONS

//virtual
PAIR<IScalar*, IScalar*>
IScalar::
createOperation(Opt::Environment& cEnvironment_,
				Interface::IScalar* pOperand)
{
	// default: no operation available
	return MAKEPAIR((IScalar*)0, (IScalar*)0);
}

// FUNCTION protected
//	Interface::IScalar::IScalar -- constructor
//
// NOTES
//
// ARGUMENTS
//	IScalar::Type eType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

IScalar::
IScalar(IScalar::Type eType_)
	: Super(eType_), m_cType()
{}

// FUNCTION protected
//	Interface::IScalar::IScalar -- constructor
//
// NOTES
//
// ARGUMENTS
//	IScalar::Type eType_
//	const Scalar::DataType& cType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

IScalar::
IScalar(IScalar::Type eType_, const Scalar::DataType& cType_)
  : Super(eType_), m_cType(cType_)
{}

// FUNCTION protected
//	Interface::IScalar::generateData -- generate data
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
IScalar::
generateData(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_)
{
	// default: add new data
	return cProgram_.addVariable(getDataType().createData());
}

// FUNCTION protected
//	Interface::IScalar::generateThis -- generate main
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
IScalar::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	// default: do nothing
	return iDataID_;
}

// FUNCTION protected
//	Interface::IScalar::getNodeVariable -- get data if cached
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
IScalar::
getNodeVariable(Execution::Interface::IIterator* pIterator_,
				Candidate::AdoptArgument& cArgument_)
{
	int iDataID = pIterator_->getNodeVariable(getID());
	if (iDataID < 0) {
		// try to get from if-else scope
		iDataID = cArgument_.getNodeVariable(getID());
	}
	return iDataID;
}

// FUNCTION protected
//	Interface::IScalar::setNodeVariable -- set data cache
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	int iDataID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
IScalar::
setNodeVariable(Execution::Interface::IIterator* pIterator_,
				Candidate::AdoptArgument& cArgument_,
				int iDataID_)
{
	if (cArgument_.setNodeVariable(getID(), iDataID_) == false) {
		// no if-else scope -> add to iterator
		pIterator_->setNodeVariable(getID(), iDataID_, pIterator_);
	}
}

// FUNCTION protected
//	Interface::IScalar::setDataType -- accessor
//
// NOTES
//
// ARGUMENTS
//	const Scalar::DataType& cType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
IScalar::
setDataType(const Scalar::DataType& cType_)
{
	m_cType = cType_;
}




//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
