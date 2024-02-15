// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Interface/IPredicate.h --
// 
// Copyright (c) 2008, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_INTERFACE_IPREDICATE_H
#define __SYDNEY_EXECUTION_INTERFACE_IPREDICATE_H

#include "Execution/Interface/IAction.h"

_SYDNEY_BEGIN

namespace Common
{
	class Data;
}

_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_INTERFACE_BEGIN

//////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Interface::IPredicate -- Base class for the classes which represents action
//
//	NOTES
//		This class is not constructed directly
class IPredicate
	: public IAction
{
public:
	typedef IAction Super;
	typedef IPredicate This;

	// CLASS
	//	Execution::Interface::IPredicate::Boolean -- tri-value boolean
	//
	// NOTES
	class Boolean
	{
	public:
		enum Value
		{
			False = 0,
			True,
			Unknown,
			NeverTrue,
			ValueNum
		};
		static Value boolAnd(Value v1_, Value v2_);
		static Value boolOr(Value v1_, Value v2_);
		static Value boolNot(Value v_);
		static bool isTrue(Value v_) {return v_ == True;}
		static bool isFalse(Value v_) {return v_ == False || v_ == NeverTrue;}
		static bool isUnknown(Value v_) {return v_ == Unknown;}
	protected:
	private:
		Boolean() {}					// never instantiated
	};

	// destructor
	virtual ~IPredicate() {}

	// get predicate result after calling execute
	virtual Boolean::Value check(Interface::IProgram& cProgram_) = 0;
	// get predicate result for a specified data
	virtual Boolean::Value checkByData(Interface::IProgram& cProgram_,
									   const Common::Data* pData_);

protected:
	// constructor
	IPredicate() : Super() {}

private:
};

_SYDNEY_EXECUTION_INTERFACE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_INTERFACE_IPREDICATE_H

//
//	Copyright (c) 2008, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
