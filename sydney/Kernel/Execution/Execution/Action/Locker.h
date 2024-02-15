// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Action/Locker.h --
// 
// Copyright (c) 2010, 2011, 2012, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ACTION_LOCKER_H
#define __SYDNEY_EXECUTION_ACTION_LOCKER_H

#include "Execution/Action/Module.h"
#include "Execution/Action/Argument.h"

#include "Execution/Interface/IObject.h"
#include "Execution/Declaration.h"

#include "Common/Externalizable.h"

_SYDNEY_BEGIN

namespace Common
{
	class BitSet;
	class DataArrayData;
}

_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Action::Locker -- wrapping class for locker access in various actions
//
//	NOTES
class Locker
	: public Interface::IObject
{
public:
	typedef Interface::IObject Super;
	typedef Locker This;
	typedef LockerArgument Argument;

	// constructor
	struct Normal
	{
		static This* create(Interface::IProgram& cProgram_,
							FileAccess* pFileAccess_,
							const Argument& cArgument_,
							int iDataID_);
	};
	struct GetByBitSet
	{
		static This* create(Interface::IProgram& cProgram_,
							FileAccess* pFileAccess_,
							const Argument& cArgument_,
							int iDataID_);
	};
	struct GetByBitSetCacheAllObject
	{
		static This* create(Interface::IProgram& cProgram_,
							FileAccess* pFileAccess_,
							const Argument& cArgument_,
							int iDataID_);
	};
	struct CacheAllObject
	{
		static This* create(Interface::IProgram& cProgram_,
							FileAccess* pFileAccess_,
							const Argument& cArgument_,
							int iDataID_);
	};

	struct BitSetSort
	{
		static This* create(Interface::IProgram& cProgram_,
							FileAccess* pFileAccess_,
							const Argument& cArgument_,
							int iDataID_);
	};
	struct Unlocker
	{
		static This* create(Interface::IProgram& cProgram_,
							const Argument& cArgument_,
							int iDataID_);
	};

	// destructor
	virtual ~Locker() {};

	// set fileaccess instance
	virtual void setFileAccess(FileAccess* pFileAccess_) = 0;

	// initialize necessary members
	virtual void initialize(Interface::IProgram& cProgram_) = 0;
	// end using members
	virtual void terminate(Interface::IProgram& cProgram_) = 0;

	// main action
	virtual bool getData(Interface::IProgram& cProgram_,
						 Common::DataArrayData* pData_) = 0;
	// fetch related action
	virtual void fetch(const Common::DataArrayData* pOption_) = 0;
	// reset
	virtual void reset() = 0;
	// close
	virtual void close() = 0;
	// unlock tuple
	virtual void unlock() = 0;
	virtual void unlock(const Common::BitSet& cBitSet_) = 0;

	// lock is needed?
	virtual bool isNeedLock() = 0;
	// is prepared?
	virtual bool isPrepare() = 0;

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
	Locker() {}

	// register to program
	void registerToProgram(Interface::IProgram& cProgram_);

private:
};

///////////////////////////////////
// CLASS
//	Action::LockerHolder --
//
// NOTES

class LockerHolder
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef LockerHolder This;

	LockerHolder()
		: Super(),
		  m_iID(-1),
		  m_pLocker(0)
	{}
	LockerHolder(int iID_)
		: Super(),
		  m_iID(iID_),
		  m_pLocker(0)
	{}
	~LockerHolder() {}

	int getID() {return m_iID;}
	void setID(int iID_) {m_iID = iID_;}

	// initialize Locker instance
	void initialize(Interface::IProgram& cProgram_);
	// terminate Locker instance
	void terminate(Interface::IProgram& cProgram_);

	// initialize status
	bool isInitialized() {return m_pLocker != 0;}
	bool isValid() {return m_iID >= 0;}

	// -> operator
	Locker* operator->() const {return m_pLocker;}

	// serializer
	void serialize(ModArchive& archiver_);

protected:
private:
	int m_iID;
	Locker* m_pLocker;
};

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ACTION_LOCKER_H

//
//	Copyright (c) 2010, 2011, 2012, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
