// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File/IndexMap.h --
// 
// Copyright (c) 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_FILE_INDEXMAP_H
#define __SYDNEY_PLAN_FILE_INDEXMAP_H

#include "Plan/File/Declaration.h"
#include "Plan/Predicate/Declaration.h"

#include "Plan/Utility/Algorithm.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_FILE_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::File::IndexMap -- 
//
//	NOTES
class IndexMap
	: public MAP<Interface::IFile*,
				 Interface::IPredicate*,
				 Utility::ReferingLess<Interface::IFile> >
{
public:
	typedef MAP<Interface::IFile*,
				Interface::IPredicate*,
				Utility::ReferingLess<Interface::IFile> > Super;
	typedef IndexMap This;

	// constructor
	IndexMap() : Super() {}
	IndexMap(const IndexMap& cOther_) : Super(cOther_) {}
	// destructor
	~IndexMap() {}
	
protected:
private:
};

_SYDNEY_PLAN_FILE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_FILE_INDEXMAP_H

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
