// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Operation.cpp --
// 
// Copyright (c) 2011, 2016, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Scalar/Operation.h"
#include "Plan/Scalar/Value.h"
#include "Plan/Scalar/Impl/OperationImpl.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////////
//	Scalar::Operation::Append::

// FUNCTION public
//	Scalar::Operation::Append::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pOperand_
//	Interface::IScalar* pOption_
//	
// RETURN
//	Operation*
//
// EXCEPTIONS

//static
Operation*
Operation::Append::
create(Opt::Environment& cEnvironment_,
	   Interface::IScalar* pOperand_,
	   Interface::IScalar* pOption_)
{
	AUTOPOINTER<This> pResult =
		new OperationImpl::Append(STRING(), pOperand_, pOption_);
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

////////////////////////////////////////
//	Scalar::Operation::Truncate

// FUNCTION public
//	Scalar::Operation::Truncate::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pOperand_
//	Interface::IScalar* pOption_
//	
// RETURN
//	Operation*
//
// EXCEPTIONS

//static
Operation*
Operation::Truncate::
create(Opt::Environment& cEnvironment_,
	   Interface::IScalar* pOperand_,
	   Interface::IScalar* pOption_)
{
	AUTOPOINTER<This> pResult =
		new OperationImpl::Truncate(STRING(), pOperand_, pOption_);
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

///////////////////////////////////////
//	Plan::Scalar::Operation::Replace::

// FUNCTION public
//	Scalar::Operation::Replace::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pOperand_
//	const VECTOR<Interface::IScalar*>& vecOption_
//	
// RETURN
//	Operation*
//
// EXCEPTIONS

//static
Operation*
Operation::Replace::
create(Opt::Environment& cEnvironment_,
	   Interface::IScalar* pOperand_,
	   const VECTOR<Interface::IScalar*>& vecOption_)
{
	AUTOPOINTER<This> pResult =
		new OperationImpl::Replace(STRING(), pOperand_, vecOption_);
	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

////////////////////////////////////////
//	Scalar::Operation::LogData

// FUNCTION public
//	Scalar::Operation::LogData::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pOperand_
//	const Common::Data::Pointer& pArray_
//	
// RETURN
//	Operation*
//
// EXCEPTIONS

//static
Operation*
Operation::LogData::
create(Opt::Environment& cEnvironment_,
	   Interface::IScalar* pOperand_,
	   const Common::Data::Pointer& pArray_)
{
	; _SYDNEY_ASSERT(pArray_.get());
	; _SYDNEY_ASSERT(pArray_->getType() == Common::DataType::Array);
	; _SYDNEY_ASSERT(pArray_->getElementType() == Common::DataType::Data);

	const Common::DataArrayData& cArray = _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, *pArray_);
	; _SYDNEY_ASSERT(cArray.getCount() > 0);

	AUTOPOINTER<This> pResult;

	int iType = cArray.getElement(0)->getInt();
	switch (iType) {
	case Tree::Node::Append:
		{
			; _SYDNEY_ASSERT(cArray.getCount() > 1);
			Interface::IScalar* pOption = Scalar::Value::create(cEnvironment_,
																cArray.getElement(1));
			pResult =
				new OperationImpl::Append(STRING(), pOperand_, pOption);
			break;
		}
	case Tree::Node::Truncate:
		{
			; _SYDNEY_ASSERT(cArray.getCount() > 1);
			Interface::IScalar* pOption = Scalar::Value::create(cEnvironment_,
																cArray.getElement(1));
			pResult =
				new OperationImpl::Truncate(STRING(), pOperand_, pOption);
			break;
		}
	case Tree::Node::Replace:
		{
			; _SYDNEY_ASSERT(cArray.getCount() > 2);
			VECTOR<Interface::IScalar*> vecOption;
			vecOption.PUSHBACK(Scalar::Value::create(cEnvironment_,
													 cArray.getElement(1)));
			vecOption.PUSHBACK(Scalar::Value::create(cEnvironment_,
													 cArray.getElement(2)));

			pResult =
				new OperationImpl::Replace(STRING(), pOperand_, vecOption);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}

	pResult->createDataType(cEnvironment_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
