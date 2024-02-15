// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Opt/NameMap.h --
// 
// Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_OPT_BITSETVARIABLEMANAGER_H
#define __SYDNEY_OPT_BITSETVARIABLEMANAGER_H

#include "Opt/Module.h"
#include "Opt/Declaration.h"
#include "Opt/Environment.h"

#include "Plan/Relation/Declaration.h"
#include "Plan/Scalar/Declaration.h"

#include "Common/Object.h"

_SYDNEY_BEGIN
_SYDNEY_OPT_BEGIN

////////////////////////////////////
//	CLASS
//	Opt::NameMap -- Represents correspondence between name and scalar info
//
//	NOTES
class BitSetVariable
	: public ExecutableObject
{
public:
	typedef BitSetVariable This;
	typedef Common::Object Super;


	
	static This* create(const STRING& cstrValName_ const STRING& cstrTableName);

	
	ModUnicodeString& getName(){return m_cstrName;}
	Common::BitSet& getValue() {return m_cBitSet;}
	ModUnicodeString& getTableName();
	

	// destructor
	~BitSetVariable() {} // no subclasses


protected:
private:
	// costructor
	BitSetVariable(const ModUnicodeString& cstrValName_ const ModUnicodeString& cstrTableName);

	Common::BitSet m_cBitSet;
	ModUnicodeString m_cstrValName;
	ModUnicodeString m_cstrTableName;
};

_SYDNEY_OPT_END
_SYDNEY_END

#endif // __SYDNEY_OPT_NAMEMAP_H

//
//	Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
