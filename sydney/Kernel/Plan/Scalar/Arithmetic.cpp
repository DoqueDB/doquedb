// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Arithmetic.cpp --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#include "Plan/Scalar/Arithmetic.h"
#include "Plan/Scalar/Impl/ArithmeticImpl.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataOperation.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////////
//	Scalar::Arithmetic

// FUNCTION public
//	Scalar::Arithmetic::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const VECTOR<Interface::IScalar*>& vecOperand_
//	const STRING& cstrName_
//	
// RETURN
//	Arithmetic*
//
// EXCEPTIONS

//static
Arithmetic*
Arithmetic::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const VECTOR<Interface::IScalar*>& vecOperand_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult =
		new ArithmeticImpl::Nadic(eOperator_,
								  cstrName_,
								  vecOperand_);
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Arithmetic::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR<Interface::IScalar*
//	Interface::IScalar*>& cOperand_
//	const STRING& cstrName_
//	
// RETURN
//	Arithmetic*
//
// EXCEPTIONS

//static
Arithmetic*
Arithmetic::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult =
		new ArithmeticImpl::Dyadic(eOperator_,
								   cstrName_,
								   cOperand_);
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Arithmetic::create -- constructor
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
//	Arithmetic*
//
// EXCEPTIONS

//static
Arithmetic*
Arithmetic::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult =
		new ArithmeticImpl::Monadic(eOperator_,
									cstrName_,
									pOperand_);
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Arithmetic::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const VECTOR<Interface::IScalar*>& vecOperand_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Arithmetic*
//
// EXCEPTIONS

//static
Arithmetic*
Arithmetic::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const VECTOR<Interface::IScalar*>& vecOperand_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	This* pResult = create(cEnvironment_,
						   eOperator_,
						   vecOperand_,
						   cstrName_);
	pResult->setDataType(cDataType_);
	return pResult;
}

// FUNCTION public
//	Scalar::Arithmetic::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR<Interface::IScalar*
//	Interface::IScalar*>& cOperand_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Arithmetic*
//
// EXCEPTIONS

//static
Arithmetic*
Arithmetic::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	This* pResult = create(cEnvironment_,
						   eOperator_,
						   cOperand_,
						   cstrName_);
	pResult->setDataType(cDataType_);
	return pResult;
}

// FUNCTION public
//	Scalar::Arithmetic::create -- constructor
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
//	Arithmetic*
//
// EXCEPTIONS

//static
Arithmetic*
Arithmetic::
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

// FUNCTION protected
//	Scalar::Arithmetic::castOperand -- convert operand to expected type
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pOperand_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

Interface::IScalar*
Arithmetic::
castOperand(Opt::Environment& cEnvironment_,
			Interface::IScalar* pOperand_)
{
	if (getDataType().isDecimalType()) {
		const DataType& cDataType = pOperand_->getDataType();
		if (cDataType.isDecimalType() == false) {
			DataType cNewType(getDataType());
			// set length and scale from original type
			cNewType.setLength(cDataType.getLength());
			cNewType.setScale(cDataType.getScale());

			return pOperand_->createCast(cEnvironment_,
										 cNewType,
										 false /* not for comparison */,
										 getType());
		}
	}
	return pOperand_->createCast(cEnvironment_,
								 getDataType(),
								 false /* not for comparison */,
								 getType());
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
