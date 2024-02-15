// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Predicate/Comparison.h --
// 
// Copyright (c) 2009, 2010, 2011, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_PREDICATE_COMPARISON_H
#define __SYDNEY_EXECUTION_PREDICATE_COMPARISON_H

#include "Execution/Predicate/Base.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_PREDICATE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Predicate::Comparison -- predicate class for comparison
//
//	NOTES
//		This class is not constructed directly
class Comparison
	: public Base
{
public:
	typedef Base Super;
	typedef Comparison This;

	// ENUM
	//	Execution::Predicate::Comparison::Type::Value --
	//
	// NOTE
	struct Type
	{
		enum Value
		{
			Equals = 0,
			LessThanEquals,
			GreaterThanEquals,
			LessThan,
			GreaterThan,
			NotEquals,
			IsNull,
			IsNotNull,
			IsDistinct,

			Undefined,
			ValueNum
		};
	};

	// constructor
	static This* create(Interface::IProgram& cProgram_,
						Interface::IIterator* pIterator_,
						int iData0_,
						int iData1_,
						Type::Value eType_);
	static This* create(Interface::IProgram& cProgram_,
						Interface::IIterator* pIterator_,
						int iID_,
						Type::Value eType_);
	struct Row
	{
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							const VECTOR<int>& vecData0_,
							const VECTOR<int>& vecData1_,
							Type::Value eType_);
	};
	struct AnyElement
	{
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iData0_,
							int iData1_,
							Type::Value eType_);
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							const VECTOR<int>& vecData0_,
							int iData1_,
							Type::Value eType_);
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iID_,
							Type::Value eType_,
							bool bCascade_ = false,
							bool bCheckIsArray_ = false);
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							const VECTOR<int>& vecID_,
							Type::Value eType_,
							bool bCascade_ = false,
							bool bCheckIsArray_ = false);
	};
	struct AllElement
	{
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iData0_,
							int iData1_,
							Type::Value eType_);
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							const VECTOR<int>& vecData0_,
							int iData1_,
							Type::Value eType_);
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							int iID_,
							Type::Value eType_,
							bool bCascade_ = false);
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							const VECTOR<int>& vecID_,
							Type::Value eType_,
							bool bCascade_ = false);
	};

	// destructor
	virtual ~Comparison() {}

	// for serialize
	static This* getInstance(int iCategory_);

protected:
	// constructor
	Comparison() : Super() {}

private:
};

_SYDNEY_EXECUTION_PREDICATE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_PREDICATE_COMPARISON_H

//
//	Copyright (c) 2009, 2010, 2011, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
