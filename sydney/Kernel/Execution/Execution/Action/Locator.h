// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Action/Locator.h --
// 
// Copyright (c) 2011, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ACTION_LOCATOR_H
#define __SYDNEY_EXECUTION_ACTION_LOCATOR_H

#include "Execution/Action/Module.h"
#include "Execution/Action/Argument.h"

#include "Execution/Interface/IObject.h"
#include "Execution/Declaration.h"

#include "Common/Externalizable.h"

_SYDNEY_BEGIN

namespace LogicalFile
{
	class AutoLogicalFile;
	class Locator;
}

_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Action::Locator -- wrapping class for locator access in various actions
//
//	NOTES
class Locator
	: public Interface::IObject
{
public:
	typedef Interface::IObject Super;
	typedef Locator This;

	static This* create(Interface::IProgram& cProgram_);

	// destructor
	virtual ~Locator() {};

	// initialize necessary members
	virtual void initialize(Interface::IProgram& cProgram_) = 0;
	// end using members
	virtual void terminate(Interface::IProgram& cProgram_) = 0;

	// get logicalfile::locator from logical file
	virtual bool getLocator(LogicalFile::AutoLogicalFile& cLogicalFile_,
							Common::DataArrayData* pKey_) = 0;

	// get length
	virtual void length(Common::UnsignedIntegerData* pResult_) = 0;
	// get data
	virtual void get(const Common::UnsignedIntegerData* pStart_,
					 const Common::UnsignedIntegerData* pLength_,
					 Common::Data* pResult_) = 0;

	// append
	virtual void append(const Common::Data* pAppendData_) = 0;
	// truncate
	virtual void truncate(const Common::UnsignedIntegerData* pTruncateLength_) = 0;
	// replace
	virtual void replace(const Common::Data* pPlaceData_,
						 const Common::UnsignedIntegerData* pStart_,
						 const Common::UnsignedIntegerData* pLength_) = 0;

	// clear locator
	virtual void clear() = 0;
	// is locator assigned?
	virtual bool isValid() = 0;

	// for serialize
	static This* getInstance(int iCategory_);

///////////////////////////////
// Common::Externalizable
//	int getClassID() const;

///////////////////////////////
// ModSerializer
//	void serialize(ModArchive& archiver_);

protected:
	// constructor
	Locator() {}

private:
	// register to program
	void registerToProgram(Interface::IProgram& cProgram_);
};

///////////////////////////////////
// CLASS
//	Action::LocatorHolder --
//
// NOTES

class LocatorHolder
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef LocatorHolder This;

	LocatorHolder()
		: Super(),
		  m_iID(-1),
		  m_pLocator(0)
	{}
	LocatorHolder(int iID_)
		: Super(),
		  m_iID(iID_),
		  m_pLocator(0)
	{}
	~LocatorHolder() {}

	int getID() {return m_iID;}
	void setID(int iID_) {m_iID = iID_;}

	// initialize Locator instance
	void initialize(Interface::IProgram& cProgram_);
	// terminate Locator instance
	void terminate(Interface::IProgram& cProgram_);

	// clear Locator instance
	void clear();

	// initialize status
	bool isInitialized() {return m_pLocator != 0;}
	bool isValid() {return m_iID >= 0;}

	// accessor
	Locator* getLocator() {return m_pLocator;}

	// -> operator
	Locator* operator->() const {return m_pLocator;}

	// serializer
	void serialize(ModArchive& archiver_);

	// explain
	void explain(Interface::IProgram& cProgram_,
				 Opt::Explain& cExplain_);

protected:
private:
	int m_iID;
	Locator* m_pLocator;
};

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ACTION_LOCATOR_H

//
//	Copyright (c) 2011, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
