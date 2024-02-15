// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Locator.cpp --
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

#include "Plan/Scalar/Locator.h"
#include "Plan/Scalar/Impl/LocatorImpl.h"

#include "Common/Assert.h"

#include "Exception/Unexpected.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////////
//	Scalar::Locator

//static
Locator*
Locator::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   const STRING& cstrName_)
{
	; _SYDNEY_ASSERT(pOperand_->isField());
	; _SYDNEY_ASSERT(pOperand_->getField()->isLob());

	AUTOPOINTER<This> pResult;
	switch (eOperator_) {
	case Tree::Node::CharLength:
		{
			pResult = new LocatorImpl::CharLength(cstrName_,
												  pOperand_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Locator::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_
//	const STRING& cstrName_
//	
// RETURN
//	Locator*
//
// EXCEPTIONS

//static
Locator*
Locator::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
	   const STRING& cstrName_)
{
	; _SYDNEY_ASSERT(cOperand_.first->isField());
	; _SYDNEY_ASSERT(cOperand_.first->getField()->isLob());

	AUTOPOINTER<This> pResult;
	switch (eOperator_) {
	case Tree::Node::StringConcatenate:
		{
			pResult = new LocatorImpl::StringConcatenate(cstrName_,
														 cOperand_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Locator::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	const VECTOR<Interface::IScalar*>& vecOption_
//	const STRING& cstrName_
//	
// RETURN
//	Locator*
//
// EXCEPTIONS

//static
Locator*
Locator::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   const VECTOR<Interface::IScalar*>& vecOption_,
	   const STRING& cstrName_)
{
	; _SYDNEY_ASSERT(pOperand_->isField());
	; _SYDNEY_ASSERT(pOperand_->getField()->isLob());

	AUTOPOINTER<This> pResult;
	switch (eOperator_) {
	case Tree::Node::SubString:
		{
			pResult =
				new LocatorImpl::SubString(cstrName_,
										   pOperand_,
										   vecOption_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Locator::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_
//	const VECTOR<Interface::IScalar*>& vecOption_
//	const STRING& cstrName_
//	
// RETURN
//	Locator*
//
// EXCEPTIONS

//static
Locator*
Locator::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
	   const VECTOR<Interface::IScalar*>& vecOption_,
	   const STRING& cstrName_)
{
	; _SYDNEY_ASSERT(cOperand_.first->isField());
	; _SYDNEY_ASSERT(cOperand_.first->getField()->isLob());

	AUTOPOINTER<This> pResult;
	switch (eOperator_) {
	case Tree::Node::Overlay:
		{
			pResult =
				new LocatorImpl::Overlay(cstrName_,
										 cOperand_,
										 vecOption_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Locator::create -- constructor
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
//	Locator*
//
// EXCEPTIONS

//static
Locator*
Locator::
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
//	Scalar::Locator::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Locator*
//
// EXCEPTIONS

//static
Locator*
Locator::
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
//	Scalar::Locator::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	const VECTOR<Interface::IScalar*>& vecOption_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Locator*
//
// EXCEPTIONS

//static
Locator*
Locator::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_,
	   const VECTOR<Interface::IScalar*>& vecOption_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	This* pResult = create(cEnvironment_,
						   eOperator_,
						   pOperand_,
						   vecOption_,
						   cstrName_);
	pResult->setDataType(cDataType_);
	return pResult;
}

// FUNCTION public
//	Scalar::Locator::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_
//	const VECTOR<Interface::IScalar*>& vecOption_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Locator*
//
// EXCEPTIONS

//static
Locator*
Locator::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
	   const VECTOR<Interface::IScalar*>& vecOption_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	This* pResult = create(cEnvironment_,
						   eOperator_,
						   cOperand_,
						   vecOption_,
						   cstrName_);
	pResult->setDataType(cDataType_);
	return pResult;
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
