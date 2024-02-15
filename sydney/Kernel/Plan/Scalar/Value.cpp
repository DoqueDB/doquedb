// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Value.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#include "Plan/Scalar/Value.h"
#include "Plan/Scalar/Impl/ValueImpl.h"

#include "Common/DefaultData.h"
#include "Common/NullData.h"

#include "Exception/Unexpected.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_SCALAR_USING

//////////////////////////////////
//	Plan::Scalar::Value::Null

// FUNCTION public
//	Scalar::Value::Null::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Value*
//
// EXCEPTIONS

//static
Value*
Value::Null::
create(Opt::Environment& cEnvironment_)
{
	This* pResult = cEnvironment_.getNullConstant();
	if (pResult == 0) {
		pResult = Value::create(cEnvironment_,
								Common::NullData::getInstance());
		cEnvironment_.setNullConstant(pResult);
	}
	return pResult;
}

/////////////////////////////////////
//	Plan::Scalar::Value::Default

// FUNCTION public
//	Scalar::Value::Default::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Value*
//
// EXCEPTIONS

//static
Value*
Value::Default::
create(Opt::Environment& cEnvironment_)
{
	This* pResult = cEnvironment_.getDefaultConstant();
	if (pResult == 0) {
		pResult = Value::create(cEnvironment_,
								Common::DefaultData::getInstance());
		cEnvironment_.setDefaultConstant(pResult);
	}
	return pResult;
}

/////////////////////////////////////////
//	Plan::Scalar::Value::PlaceHolder

// FUNCTION public
//	Scalar::Value::PlaceHolder::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	int iNumber_
//	
// RETURN
//	Value*
//
// EXCEPTIONS

//static
Value*
Value::PlaceHolder::
create(Opt::Environment& cEnvironment_,
	   int iNumber_)
{
	This* pResult = cEnvironment_.getPlaceHolder(iNumber_);
	if (pResult == 0) {
		// name
		OSTRSTREAM stream;
		stream << "?" << iNumber_;

		AUTOPOINTER<This> pPlaceHolder = new ValueImpl::PlaceHolder(stream.getString(),
																	iNumber_);
		pPlaceHolder->registerToEnvironment(cEnvironment_);
		pResult = pPlaceHolder.release();

		cEnvironment_.setPlaceHolder(iNumber_, pResult);
	}
	return pResult;
}

/////////////////////////////////////////
//	Plan::Scalar::Value::BulkData

// FUNCTION public
//	Scalar::Value::BulkData::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const STRING& cstrName_
//	
// RETURN
//	Value*
//
// EXCEPTIONS

//static
Value*
Value::BulkData::
create(Opt::Environment& cEnvironment_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult = new ValueImpl::BulkVariable(cstrName_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

/////////////////////////////////////////
//	Plan::Scalar::Value::SessionVariable

// FUNCTION public
//	Scalar::Value::SessionVariable::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const STRING& cstrName_
//	
// RETURN
//	Value*
//
// EXCEPTIONS

//static
Value*
Value::SessionVariable::
create(Opt::Environment& cEnvironment_,
	   const STRING& cstrName_)
{
	// get datatype for the variable
	//----------
	// for now, session variable holds Common::BitSet

	Value* pResult = cEnvironment_.getSessionVariable(cstrName_);
	if (pResult == 0) {
		DataType cDataType(Common::DataType::BitSet);
		AUTOPOINTER<This> pVariable = new ValueImpl::SessionVariable(cstrName_,
																	 cDataType);
		pVariable->registerToEnvironment(cEnvironment_);
		pResult = pVariable.release();

		cEnvironment_.setSessionVariable(cstrName_, pResult);
	}
	return pResult;
}

/////////////////////////////////////////
//	Plan::Scalar::Value::SessionVariable

// FUNCTION public
//	Scalar::Value::SessionVariable::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const STRING& cstrName_
//	
// RETURN
//	Value*
//
// EXCEPTIONS

//static
Value*
Value::Asterisk::
create(Opt::Environment& cEnvironment_)

{
	AUTOPOINTER<This> pResult = new ValueImpl::Asterisk();
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}




//////////////////////////////
//	Plan::Scalar::Value

// FUNCTION public
//	Scalar::Value::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const STRING& cstrName_
//	
// RETURN
//	Value*
//
// EXCEPTIONS

//static
Value*
Value::
create(Opt::Environment& cEnvironment_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult = new ValueImpl::Variable(cstrName_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}




// FUNCTION public
//	Scalar::Value::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Value*
//
// EXCEPTIONS

//static
Value*
Value::
create(Opt::Environment& cEnvironment_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult = new ValueImpl::Variable(cstrName_,
														cDataType_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Value::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const DataPointer& pData_
//	
// RETURN
//	Value*
//
// EXCEPTIONS

//static
Value*
Value::
create(Opt::Environment& cEnvironment_,
	   const DataPointer& pData_)
{
	AUTOPOINTER<This> pResult = new ValueImpl::Constant(pData_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}




// FUNCTION public
//	Scalar::Value::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const DataPointer& pData_
//	
// RETURN
//	Value*
//
// EXCEPTIONS

//static
Value*
Value::
create(Opt::Environment& cEnvironment_,
	   const DataPointer& pData_,
	   const STRING& cstrSQL_)
{
	AUTOPOINTER<This> pResult = new ValueImpl::Constant(pData_, cstrSQL_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Value::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const DataPointer& pData_
//	const DataType& cDataType_
//	
// RETURN
//	Value*
//
// EXCEPTIONS

//static
Value*
Value::
create(Opt::Environment& cEnvironment_,
	   const DataPointer& pData_,
	   const DataType& cDataType_)
{
	This* pResult = 0;

	DataType cType(pData_->getType());
	if (DataType::isAssignable(cType, cDataType_) == false) {
		DataPointer pTarget = cDataType_.createData();
		cDataType_.cast(cType, pData_, pTarget);
		pResult = create(cEnvironment_, pTarget);
	} else {
		pResult = create(cEnvironment_, pData_);
	}
	pResult->setDataType(cDataType_);
	return pResult;
}

// FUNCTION public
//	Scalar::Value::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation_
//	const STRING& cstrName_
//	
// RETURN
//	Value*
//
// EXCEPTIONS

//static
Value*
Value::
create(Opt::Environment& cEnvironment_,
	   Interface::IRelation* pRelation_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult = new ValueImpl::RelationVariable(pRelation_,
																cstrName_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Value::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation_
//	const DataType& cDataType_
//	const STRING& cstrName_
//	
// RETURN
//	Value*
//
// EXCEPTIONS

//static
Value*
Value::
create(Opt::Environment& cEnvironment_,
	   Interface::IRelation* pRelation_,
	   const DataType& cDataType_,
	   const STRING& cstrName_)
{
	AUTOPOINTER<This> pResult = new ValueImpl::RelationVariable(pRelation_,
																cstrName_,
																cDataType_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Scalar::Value::setData -- set data
//
// NOTES
//
// ARGUMENTS
//	const DataPointer& pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Value::
setData(const DataPointer& pData_)
{
	// default: never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
