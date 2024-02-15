// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Value.h --
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

#ifndef __SYDNEY_PLAN_SCALAR_VALUE_H
#define __SYDNEY_PLAN_SCALAR_VALUE_H

#include "Plan/Scalar/Base.h"

#include "Common/Data.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Scalar::Value -- Interface for constant value
//
//	NOTES
class Value
	: public Base
{
public:
	typedef Base Super;
	typedef Value This;

	typedef Common::Data::Pointer DataPointer;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						const STRING& cstrName_);
	static This* create(Opt::Environment& cEnvironment_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	static This* create(Opt::Environment& cEnvironment_,
						const DataPointer& pData_);
	static This* create(Opt::Environment& cEnvironment_,
						const DataPointer& pData_,
						const STRING& cstrSQL_);
	static This* create(Opt::Environment& cEnvironment_,
						const DataPointer& pData_,
						const DataType& cDataType_);
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IRelation* pRelation_,
						const STRING& cstrName_);
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IRelation* pRelation_,
						const DataType& cDataType_,
						const STRING& cstrName_);
	// special constant
	struct Null
	{
		static This* create(Opt::Environment& cEnvironment_);
	};
	struct Default
	{
		static This* create(Opt::Environment& cEnvironment_);
	};
	struct PlaceHolder
	{
		static This* create(Opt::Environment& cEnvironment_,
							int iNumber_);
	};
	struct BulkData
	{
		static This* create(Opt::Environment& cEnvironment_,
							const STRING& cstrName_);
	};
	struct SessionVariable
	{
		static This* create(Opt::Environment& cEnvironment_,
							const STRING& cstrName_);
	};
	struct Asterisk
	{
		static This* create(Opt::Environment& cEnvironment_);
	};
	

	// destructor
	virtual ~Value() {}

	// set data
	virtual void setData(const DataPointer& pData_);

protected:
	// constructor
	explicit Value(Type eType_)
		: Super(eType_)
	{}
	Value(Type eType_, const DataType& cType_)
		: Super(eType_, cType_)
	{}
private:
};

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_VALUE_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
