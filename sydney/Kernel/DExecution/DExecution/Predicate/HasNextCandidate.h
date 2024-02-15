// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DExecution/Predicate/HasNextCandidate.h --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_DEXECUTION_PREDICATE_HASNEXTCANDIDATE_H
#define __SYDNEY_DEXECUTION_PREDICATE_HASNEXTCANDIDATE_H_H

#include "DExecution/Predicate/Module.h"

#include "Execution/Predicate/Base.h"

#include "Opt/Algorithm.h"

_SYDNEY_BEGIN



_SYDNEY_DEXECUTION_BEGIN
_SYDNEY_DEXECUTION_PREDICATE_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	CLASS
//	DExecution::Predicate::HasNextCandidate -- predicate class for isValid
//
//	NOTES
//		This class is not constructed directly
class HasNextCandidate
	: public Execution::Predicate::Base
{
public:
	typedef Execution::Predicate::Base Super;
	typedef HasNextCandidate This;

	// constructor
	static This* create(Execution::Interface::IProgram& cProgram_,
						Execution::Interface::IIterator* pIterator_,
						int iDataID_);

	// destructor
	virtual ~HasNextCandidate() {}

	// for serialize
	static This* getInstance(int iCategory_);

protected:
	// constructor
	HasNextCandidate() : Super() {}

private:
};

_SYDNEY_DEXECUTION_PREDICATE_END
_SYDNEY_DEXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_DEXECUTION_PREDICATE_ISVALID_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
