// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Predicate/IsNull.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_PREDICATE_ISNULL_H
#define __SYDNEY_EXECUTION_PREDICATE_ISNULL_H

#include "Execution/Predicate/Base.h"
#include "Opt/Algorithm.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_PREDICATE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	CLASS
//	Execution::Predicate::IsNull -- predicate class for isNull
//
//	NOTES
//		This class is not constructed directly
class IsNull
	: public Base
{
public:
	typedef Base Super;
	typedef IsNull This;

	// constructor
	static This* create(Interface::IProgram& cProgram_,
						Interface::IIterator* pIterator_,
						int iDataID_);
	struct All
	{
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							const VECTOR<int>& vecDataID_);
	};
	struct Any
	{
		static This* create(Interface::IProgram& cProgram_,
							Interface::IIterator* pIterator_,
							const VECTOR<int>& vecDataID_);
	};

	// destructor
	virtual ~IsNull() {}

	// for serialize
	static This* getInstance(int iCategory_);

protected:
	// constructor
	IsNull() : Super() {}

private:
};

_SYDNEY_EXECUTION_PREDICATE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_PREDICATE_ISNULL_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
