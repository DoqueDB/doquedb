// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Aggregation.cpp --
// 
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Plan/Scalar/Aggregation.h"
#include "Plan/Scalar/Impl/AggregationImpl.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

// FUNCTION public
//	Scalar::Aggregation::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	const STRING& cstrName_
//	
// RETURN
//	Aggregation*
//
// EXCEPTIONS

//static
Aggregation*
Aggregation::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult =
		new AggregationImpl::Monadic(eOperator_,
									 cstrName_,
									 pOperand_);
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}


// FUNCTION public
//	Scalar::Aggregation::Distribution::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	const STRING& cstrName_
//	
// RETURN
//	Aggregation*
//
// EXCEPTIONS

//static
Aggregation*
Aggregation::Distribution::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult =
		new AggregationImpl::MonadicDistribution(eOperator_,
												 cstrName_,
												 pOperand_);
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}


// FUNCTION public
//	Scalar::Aggregation::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	Interface::IScalar* pOption_
//	const STRING& cstrName_
//	
// RETURN
//	Aggregation*
//
// EXCEPTIONS

//static
Aggregation*
Aggregation::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   Interface::IScalar* pOption_,
	   const STRING& cstrName_)
{
	if (pOption_ == 0) {
		return create(cEnvironment_,
					  eOperator_,
					  pOperand_,
					  cstrName_);
	}

	AUTOPOINTER<This> pResult =
		new AggregationImpl::MonadicWithMonadicOption(eOperator_,
													  cstrName_,
													  pOperand_,
													  pOption_);
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Aggregaiton::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Aggregation*
//
// EXCEPTIONS

//static
Aggregation*
Aggregation::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	This* pResult = create(cEnvironment_,
						   eOperator_,
						   pOperand_,
						   cstrName_);
	pResult->setDataType(cDataType_);
	return pResult;
}

// FUNCTION public
//	Scalar::Aggregation::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	Interface::IScalar* pOption_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Aggregation*
//
// EXCEPTIONS

//static
Aggregation*
Aggregation::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   Interface::IScalar* pOption_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	This* pResult = create(cEnvironment_,
						   eOperator_,
						   pOperand_,
						   pOption_,
						   cstrName_);
	pResult->setDataType(cDataType_);
	return pResult;
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
