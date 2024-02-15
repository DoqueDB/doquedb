// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DExecution/Iterator/Expand.h --
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

#ifndef __SYDNEY_DEXECUTION_ITERATOR_EXPAND_H
#define __SYDNEY_DEXECUTION_ITERATOR_EXPAND_H

#include "DExecution/Iterator/Module.h"
#include "Execution/Interface/IIterator.h"

_SYDNEY_BEGIN



_SYDNEY_DEXECUTION_BEGIN
_SYDNEY_DEXECUTION_ITERATOR_BEGIN

////////////////////////////////////
//	CLASS
//	DExecution::Iterator::Expand -- Iterator class for server access
//
//	NOTES
//		This class is not constructed directly
class Expand
	:public Execution::Interface::IIterator
{
public:
	typedef Execution::Interface::IIterator Super;
	typedef Expand This;
	
	// constructor
	static Expand* create(Execution::Interface::IProgram& cProgram_,
						  int iCollectionID_,
						  int iTermID_,
						  int iDocNumDataID_,
						  int iAvgLengthID_,
						  bool bSimple);


	// destructor
	virtual ~Expand() {}
	
	// for serialize
	static Expand* getInstance(int iCategory_);

protected:
	
	// constructor
	Expand() : Super() {}
	

private:

};

_SYDNEY_DEXECUTION_ITERATOR_END
_SYDNEY_DEXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_DEXECUTION_ITERATOR_EXPAND_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
