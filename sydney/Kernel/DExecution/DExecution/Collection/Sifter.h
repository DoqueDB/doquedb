// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Collection/Sifter.h --
// 
// Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_DEXECUTION_COLLECTION_SHIFTER_H
#define __SYDNEY_DEXECUTION_COLLECTION_SHIFTER_H

#include "DExecution/Collection/Module.h"

#include "Execution/Interface/ICollection.h"

#include "Opt/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_DEXECUTION_BEGIN
_SYDNEY_DEXECUTION_COLLECTION_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Collection::Sifter -- Collection class for grouping
//
//	NOTES
//		This class is not constructed directly
class Sifter
	: public Execution::Interface::ICollection
{
public:
	typedef Execution::Interface::ICollection Super;
	typedef Sifter This;

	// constructor
	static This* create(Execution::Interface::IProgram& cProgram_,
						int iDataID_);

	// destructor
	virtual ~Sifter() {}

	// for serialize
	static This* getInstance(int iCategory_);

protected:
	// constructor
	Sifter() : Super() {}

private:
};

_SYDNEY_DEXECUTION_COLLECTION_END
_SYDNEY_DEXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_DEXECUTION_COLLECTION_GROUPING_H

//
//	Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
