// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Generator.cpp --
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
const char moduleName[] = "Plan::Scalar";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Scalar/Generator.h"
#include "Plan/Scalar/Impl/AggregationImpl.h"
#include "Plan/Scalar/Impl/ArithmeticImpl.h"
#include "Plan/Scalar/Impl/ArrayImpl.h"
#include "Plan/Scalar/Impl/GeneratorImpl.h"

#include "Common/Assert.h"
#include "Common/ColumnMetaData.h"

#include "Exception/NotSupported.h"

#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Opt/Environment.h"

#include "Schema/Column.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

/////////////////////////////////////////
//	Plan::Scalar::Generator::RowID::

// FUNCTION public
//	Scalar::Generator::RowID::create -- constructor of rowid generator
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	
// RETURN
//	Generator*
//
// EXCEPTIONS

//static
Generator*
Generator::RowID::
create(Opt::Environment& cEnvironment_,
	   Schema::Column* pSchemaColumn_)
{
	AUTOPOINTER<This> pResult =
		new GeneratorImpl::RowID(pSchemaColumn_->getTable(cEnvironment_.getTransaction()),
								 DataType(*pSchemaColumn_));
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//////////////////////////////////////////////////
//	Plan::Scalar::Generator::RowID::Recovery::

// FUNCTION public
//	Scalar::Generator::RowID::Recovery::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	Interface::IScalar* pInput_
//	
// RETURN
//	Generator*
//
// EXCEPTIONS

//static
Generator*
Generator::RowID::Recovery::
create(Opt::Environment& cEnvironment_,
	   Schema::Column* pSchemaColumn_,
	   Interface::IScalar* pInput_)
{
	AUTOPOINTER<This> pResult =
		new GeneratorImpl::RecoveryRowID(pSchemaColumn_->getTable(cEnvironment_.getTransaction()),
										 DataType(*pSchemaColumn_),
										 pInput_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

///////////////////////////////////////////
//	Plan::Scalar::Generator::Identity::

// FUNCTION public
//	Scalar::Generator::Identity::create -- constructor of identity generator
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	
// RETURN
//	Generator*
//
// EXCEPTIONS

//static
Generator*
Generator::Identity::
create(Opt::Environment& cEnvironment_,
	   Schema::Column* pSchemaColumn_)
{
	AUTOPOINTER<This> pResult = new GeneratorImpl::Identity(pSchemaColumn_,
															DataType(*pSchemaColumn_),
															0 /* no input */);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Generator::Identity::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	Interface::IScalar* pInput_
//	
// RETURN
//	Generator*
//
// EXCEPTIONS

//static
Generator*
Generator::Identity::
create(Opt::Environment& cEnvironment_,
	   Schema::Column* pSchemaColumn_,
	   Interface::IScalar* pInput_)
{
	AUTOPOINTER<This> pResult = new GeneratorImpl::Identity(pSchemaColumn_,
															DataType(*pSchemaColumn_),
															pInput_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

/////////////////////////////////////////////////////
//	Plan::Scalar::Generator::Identity::Recovery::

// FUNCTION public
//	Scalar::Generator::Identity::Recovery::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	Interface::IScalar* pInput_
//	
// RETURN
//	Generator*
//
// EXCEPTIONS

//static
Generator*
Generator::Identity::Recovery::
create(Opt::Environment& cEnvironment_,
	   Schema::Column* pSchemaColumn_,
	   Interface::IScalar* pInput_)
{
	AUTOPOINTER<This> pResult = new GeneratorImpl::RecoveryIdentity(pSchemaColumn_,
																	DataType(*pSchemaColumn_),
																	pInput_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

////////////////////////////////////////////
//	Plan::Scalar::Generator::Function::

// FUNCTION public
//	Scalar::Generator::Function::create -- constructor of function generator
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Schema::Column* pSchemaColumn_
//	
// RETURN
//	Generator*
//
// EXCEPTIONS

//static
Generator*
Generator::Function::
create(Opt::Environment& cEnvironment_,
	   Schema::Column* pSchemaColumn_)
{
	AUTOPOINTER<This> pResult = new GeneratorImpl::Function(pSchemaColumn_,
															DataType(*pSchemaColumn_));
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

////////////////////////////////////
//	Plan::Scalar::Generator

// FUNCTION public
//	Scalar::Generator::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	Generator::Check::Value
//
// EXCEPTIONS

//virtual
Generator::Check::Value
Generator::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	// generator can be regarged as constant
	return Check::Constant;
}

// FUNCTION public
//	Scalar::Generator::isRefering -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Generator::
isRefering(Interface::IRelation* pRelation_)
{
	// generator can not be regarded as a member of a relation
	return pRelation_ == 0;
}

// FUNCTION public
//	Scalar::Generator::getUsedTable -- check used tables
//
// NOTES
//
// ARGUMENTS
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Generator::
getUsedTable(Utility::RelationSet& cResult_)
{
	// generator does not include any tables
}

// FUNCTION public
//	Scalar::Generator::getUsedField -- 
//
// NOTES
//
// ARGUMENTS
//	Utility::FieldSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Generator::
getUsedField(Utility::FieldSet& cResult_)
{
	// generator does not include any fields
}

// FUNCTION public
//	Scalar::Generator::getUnknownKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::CheckUnknownArgument& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Generator::
getUnknownKey(Opt::Environment& cEnvironment_,
			  Predicate::CheckUnknownArgument& cResult_)
{
	// generator does not become null
}

// FUNCTION public
//	Scalar::Generator::require -- create refered scalar
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Generator::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	// generator is always unnecessary to read from a relation
	;
}

// FUNCTION public
//	Scalar::Generator::retrieve -- 
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
Generator::
retrieve(Opt::Environment& cEnvironment_)
{
	// generator is always unnecessary to read from a relation
	;
}

// FUNCTION public
//	Scalar::Generator::retrieve -- create refered scalar
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Generator::
retrieve(Opt::Environment& cEnvironment_,
		 Interface::ICandidate* pCandidate_)
{
	// generator is always unnecessary to read from a relation
	;
}


// FUNCTION public
//	Scalar::Generator::delay -- create refered scalar
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
Generator::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  Scalar::DelayArgument& cArgument_)
{
	// generator is always unnecessary to read from a relation
	// -> can be regarded as delayable
	return true;
}

// FUNCTION public
//	Scalar::Generator::setMetaData -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Common::ColumnMetaData& cMetaData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Generator::
setMetaData(Opt::Environment& cEnvironment_,
			Common::ColumnMetaData& cMetaData_)
{
	cMetaData_.setNotSearchable();
	cMetaData_.setReadOnly();
}

// FUNCTION protected
//	Scalar::Generator::registerToEnvironment -- register to environment
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

void
Generator::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	Super::registerToEnvironment(cEnvironment_);
	// store as known-not-null
	cEnvironment_.addKnownNotNull(this);
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
