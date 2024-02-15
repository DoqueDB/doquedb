// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/Locker.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2015, 2016, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Execution::Action";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Action/Locker.h"
#include "Execution/Action/Class.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Action/FileAccess.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"
#include "Execution/Utility/FakeError.h"
#include "Execution/Utility/Transaction.h"

#include "Common/Assert.h"
#include "Common/BitSet.h"

#include "Exception/NotSupported.h"
#include "Exception/TableNotFound.h"
#include "Exception/Unexpected.h"

#include "Lock/Name.h"

#include "LogicalFile/AutoLogicalFile.h"

#include "Opt/Configuration.h"

#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ACTION_BEGIN

namespace LockerImpl
{
	////////////////////////////////////
	// CLASS
	//	Action::LockerImpl::Base --
	//
	// NOTES
	class Base
		: public Locker
	{
	public:
		typedef Locker Super;
		typedef Base This;

		// constructor
		Base()
			: Super(),
			  m_pFileAccess(0),
			  m_cArgument(),
			  m_pTransaction(0)
		{}
		Base(FileAccess* pFileAccess_,
			 const Super::Argument& cArgument_)
			: Super(),
			  m_pFileAccess(pFileAccess_),
			  m_cArgument(cArgument_),
			  m_pTransaction(0)
		{}
		// destructor
		virtual ~Base() {}

	/////////////////////
	// Action::Locker::
		virtual void setFileAccess(FileAccess* pFileAccess_) {m_pFileAccess = pFileAccess_;}
		virtual void unlock(const Common::BitSet& cBitSet_);
		virtual bool isNeedLock() {return m_cArgument.m_eMode != Lock::Mode::N;}
		virtual bool isPrepare() {return m_cArgument.m_bIsPrepare;}

	/////////////////////
	// ModSerializer::
	//	void serialize(ModArchive& archiver_);

	protected:
		// initialize
		void initializeBase(Interface::IProgram& cProgram_);
		// terminate
		void terminateBase(Interface::IProgram& cProgram_);
		// serialize
		void serializeBase(ModArchive& archiver_);

		// lock
		bool tupleTryLock(unsigned int iRowID_);
		void tupleWaitLock(unsigned int iRowID_);
		void tupleUnlock(unsigned int iRowID_);
		void unlockAll(const VECTOR<unsigned int>& vecRowID_);
		void unlockAll(const Common::BitSet::ConstIterator& cBegin_,
					   const Common::BitSet::ConstIterator& cEnd_);

		// accessor
		FileAccess* getFileAccess() {return m_pFileAccess;}
		LogicalFile::AutoLogicalFile& getLogicalFile() {return m_pFileAccess->getLogicalFile();}
		const Super::Argument& getArgument() {return m_cArgument;}

		bool isCursorLock() {return m_cArgument.m_eDuration == Lock::Duration::Cursor;}
		bool isStatementLock() {return m_cArgument.m_eDuration == Lock::Duration::Statement;}

	private:
		Lock::TupleName getLockName(unsigned int iRowID_);

		FileAccess* m_pFileAccess;
		Super::Argument m_cArgument;
		Trans::Transaction* m_pTransaction;
	};

	////////////////////////////////////
	// CLASS
	//	Action::LockerImpl::Normal --
	//
	// NOTES
	class Normal
		: public Base
	{
	public:
		typedef Base Super;
		typedef Normal This;

		// constructor
		Normal()
			: Super(),
			  m_cRowID(),
			  m_vecUnlockID()
		{}
		Normal(FileAccess* pFileAccess_,
			   const Super::Argument& cArgument_,
			   int iDataID_)
			: Super(pFileAccess_, cArgument_),
			  m_cRowID(iDataID_),
			  m_vecUnlockID()
		{}
		// destructor
		~Normal() {}

	//////////////////////
	// Actiont::Locker::
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual bool getData(Interface::IProgram& cProgram_,
							 Common::DataArrayData* pData_);
		virtual void fetch(const Common::DataArrayData* pOption_);
		virtual void reset();
		virtual void close();
		virtual void unlock();

	///////////////////////
	// Common::Externalizable::
		int getClassID() const;

	/////////////////////
	// ModSerializer::
		void serialize(ModArchive& archiver_);

	protected:
	private:
		// get constant accessor for rowid
		const RowIDHolder& getRowID() {return m_cRowID;}

		RowIDHolder m_cRowID;
		VECTOR<unsigned int> m_vecUnlockID;
	};

	////////////////////////////////////
	// CLASS
	//	Action::LockerImpl::GetByBitSet --
	//
	// NOTES
	class GetByBitSet
		: public Base
	{
	public:
		typedef Base Super;
		typedef GetByBitSet This;

		// constructor
		GetByBitSet()
			: Super(),
			  m_cBitSet()
		{}
		GetByBitSet(FileAccess* pFileAccess_,
					const Super::Argument& cArgument_,
					int iDataID_)
			: Super(pFileAccess_, cArgument_),
			  m_cBitSet(iDataID_)
		{}
		// destructor
		virtual ~GetByBitSet() {}

	//////////////////////
	// Actiont::Locker::
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual bool getData(Interface::IProgram& cProgram_,
							 Common::DataArrayData* pData_);
		virtual void fetch(const Common::DataArrayData* pOption_);
		virtual void reset();
		virtual void close();
		virtual void unlock();

	///////////////////////
	// Common::Externalizable::
		int getClassID() const;

	/////////////////////
	// ModSerializer::
		void serialize(ModArchive& archiver_);

	protected:
		// get constant accessor
		const BitSetHolder& getBitSet() {return m_cBitSet;}
		// set bitset data
		void setBitSet(const Common::BitSet& cBitSet_);

	private:

		BitSetHolder m_cBitSet;
	};

	////////////////////////////////////
	// CLASS
	//	Action::LockerImpl::GetByBitSetCacheAllObject --
	//
	// NOTES
	class GetByBitSetCacheAllObject
		: public GetByBitSet
	{
	public:
		typedef GetByBitSet Super;
		typedef GetByBitSetCacheAllObject This;

		// constructor
		GetByBitSetCacheAllObject()
			: Super(),
			  m_cBitSet(),
			  m_bHasData(false),
			  m_bHasNext(false)
		{}
		GetByBitSetCacheAllObject(FileAccess* pFileAccess_,
								  const Super::Argument& cArgument_,
								  int iDataID_)
			: Super(pFileAccess_, cArgument_, iDataID_),
			  m_cBitSet(),
			  m_bHasData(false),
			  m_bHasNext(false)
		{}
		// destructor
		~GetByBitSetCacheAllObject() {}

	//////////////////////
	// Actiont::Locker::
		virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);

		virtual bool getData(Interface::IProgram& cProgram_,
							 Common::DataArrayData* pData_);
		//	virtual void fetch(const Common::DataArrayData* pOption_);
		virtual void reset();
		//	virtual void close();
		//	virtual void unlock();

		///////////////////////
		// Common::Externalizable::
		int getClassID() const;

		/////////////////////
		// ModSerializer::
		//	void serialize(ModArchive& archiver_);

	protected:
	private:
		Common::BitSet m_cBitSet;
		bool m_bHasData;
		bool m_bHasNext;
	};

	////////////////////////////////////
	// CLASS
	//	Action::LockerImpl::CacheAllObject --
	//
	// NOTES
	class CacheAllObject
		: public Base
	{
	public:
		typedef Base Super;
		typedef CacheAllObject This;

		// constructor
		CacheAllObject()
			: Super(),
			  m_cRowID(),
			  m_vecRowID(),
			  m_vecData(),
			  m_iCursor(-1)
		{}
		CacheAllObject(FileAccess* pFileAccess_,
					   const Super::Argument& cArgument_,
					   int iDataID_)
			: Super(pFileAccess_, cArgument_),
			  m_cRowID(iDataID_),
			  m_vecRowID(),
			  m_vecData(),
			  m_iCursor(-1)
		{}
		// destructor
		~CacheAllObject() {}

	//////////////////////
	// Actiont::Locker::
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual bool getData(Interface::IProgram& cProgram_,
							 Common::DataArrayData* pData_);
		virtual void fetch(const Common::DataArrayData* pOption_);
		virtual void reset();
		virtual void close();
		virtual void unlock();

	///////////////////////
	// Common::Externalizable::
		int getClassID() const;

	/////////////////////
	// ModSerializer::
		void serialize(ModArchive& archiver_);

	protected:
	private:
		void storeTuples(Interface::IProgram& cProgram_,
						 Common::DataArrayData* pData_);
		// get constant accessor
		const RowIDHolder& getRowID() {return m_cRowID;}

		RowIDHolder m_cRowID;
		VECTOR<unsigned int> m_vecRowID;
		VECTOR<Common::Data::Pointer> m_vecData;
		int m_iCursor;
	};


	////////////////////////////////////
	// CLASS
	//	Action::LockerImpl::BitSetSort --
	//
	// NOTES
	class BitSetSort
		: public Base
	{
	public:
		typedef Base Super;
		typedef BitSetSort This;

		// constructor
		BitSetSort()
			: Super(),
			  m_cBitSet(),
			  m_cLockedBitSet(),
			  m_vecData(),
			  m_iCursor(-1)
		{}
		BitSetSort(FileAccess* pFileAccess_,
				   const Super::Argument& cArgument_,
				   int iDataID_)
			: Super(pFileAccess_, cArgument_),
			  m_cBitSet(iDataID_),
			  m_cLockedBitSet(),
			  m_vecData(),
			  m_iCursor(-1)
		{}
		// destructor
		~BitSetSort() {}

	//////////////////////
	// Actiont::Locker::
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual bool getData(Interface::IProgram& cProgram_,
							 Common::DataArrayData* pData_);
		virtual void fetch(const Common::DataArrayData* pOption_);
		virtual void reset();
		virtual void close();
		virtual void unlock();

	///////////////////////
	// Common::Externalizable::
		int getClassID() const;

	/////////////////////
	// ModSerializer::
		void serialize(ModArchive& archiver_);

	protected:
	private:
		void storeTuples(Interface::IProgram& cProgram_,
						 Common::DataArrayData* pData_);
		// get constant accessor
		const BitSetHolder& getBitSet() {return m_cBitSet;}

		BitSetHolder m_cBitSet;
		Common::BitSet m_cLockedBitSet;
		VECTOR<Common::Data::Pointer> m_vecData;
		int m_iCursor;
	};

	////////////////////////////////////
	// CLASS
	//	Action::LockerImpl::Unlocker --
	//
	// NOTES
	class Unlocker
		: public Base
	{
	public:
		typedef Base Super;
		typedef Unlocker This;

		// constructor
		Unlocker()
			: Super(),
			  m_cRowID()
		{}
		Unlocker(const Super::Argument& cArgument_,
				 int iDataID_)
			: Super(0, cArgument_),
			  m_cRowID(iDataID_)
		{}
		// destructor
		~Unlocker() {}

	//////////////////////
	// Actiont::Locker::
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual bool getData(Interface::IProgram& cProgram_,
							 Common::DataArrayData* pData_);
		virtual void fetch(const Common::DataArrayData* pOption_);
		virtual void reset();
		virtual void close();
		virtual void unlock();

	///////////////////////
	// Common::Externalizable::
		int getClassID() const;

	/////////////////////
	// ModSerializer::
		void serialize(ModArchive& archiver_);

	protected:
	private:
		// get constant accessor
		const RowIDHolder& getRowID() {return m_cRowID;}

		RowIDHolder m_cRowID;
	};

} // namespace LockerImpl

///////////////////////////////////////////////
// Execution::Action::LockerImpl::Base

// FUNCTION public
//	Action::LockerImpl::Base::unlock -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::BitSet& cBitSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::Base::
unlock(const Common::BitSet& cBitSet_)
{
	if (isNeedLock()) {
		unlockAll(cBitSet_.begin(), cBitSet_.end());
	}
}

// FUNCTION protected
//	Action::LockerImpl::Base::initializeBase -- initialize
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerImpl::Base::
initializeBase(Interface::IProgram& cProgram_)
{
	if (m_cArgument.m_bIsPrepare) {
		// check lock mode here
		m_cArgument.m_bIsPrepare = false;
		Utility::Transaction::getAdequateLock(
						  *cProgram_.getTransaction(),
						  Lock::Name::Category::Tuple,
						  true /* read only */,
						  cProgram_.isBatchMode(),
						  m_cArgument);
	}
	if (isNeedLock()) {
		// set lock name base value
		m_pTransaction = cProgram_.getTransaction();
		if (m_cArgument.m_iTableID == Schema::ObjectID::Invalid) {
			// get tableID from name
			Schema::Table* pSchemaTable = 
				cProgram_.getDatabase()->getTable(m_cArgument.m_cTableName,
												  *m_pTransaction);
			if (pSchemaTable == 0) {
				_SYDNEY_THROW2(Exception::TableNotFound,
							   m_cArgument.m_cTableName,
							   cProgram_.getDatabase()->getName());
			}
			m_cArgument.m_iDatabaseID = pSchemaTable->getDatabaseID();
			m_cArgument.m_iTableID = pSchemaTable->getID();
		}
	}
}

// FUNCTION protected
//	Action::LockerImpl::Base::terminateBase -- terminate
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerImpl::Base::
terminateBase(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION protected
//	Action::LockerImpl::Base::serializeBase -- serialize
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

// serialize
void
LockerImpl::Base::
serializeBase(ModArchive& archiver_)
{
	m_cArgument.serialize(archiver_);
}

// FUNCTION protected
//	Action::LockerImpl::Base::tupleTryLock -- lock
//
// NOTES
//
// ARGUMENTS
//	unsigned int iRowID_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
LockerImpl::Base::
tupleTryLock(unsigned int iRowID_)
{
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Lock)) {
		_OPT_EXECUTION_MESSAGE
			<< "tupleTryLock: " << m_cArgument.m_cTableName
			<< " " << iRowID_
			<< " " << m_cArgument.m_eMode
			<< ":" << m_cArgument.m_eDuration
			<< ModEndl;
	}
#endif
	return m_pTransaction->lock(getLockName(iRowID_),
								m_cArgument.m_eMode,
								m_cArgument.m_eDuration,
								0);
}

// FUNCTION protected
//	Action::LockerImpl::Base::tupleWaitLock -- 
//
// NOTES
//
// ARGUMENTS
//	unsigned int iRowID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerImpl::Base::
tupleWaitLock(unsigned int iRowID_)
{
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Lock)) {
		_OPT_EXECUTION_MESSAGE
			<< "tupleWaitLock: " << m_cArgument.m_cTableName
			<< " " << iRowID_
			<< " " << m_cArgument.m_eMode
			<< ":" << Lock::Duration::Instant
			<< ModEndl;
	}
#endif
	m_pTransaction->lock(getLockName(iRowID_),
						 m_cArgument.m_eMode,
						 Lock::Duration::Instant, /* unlock immediately */
						 Lock::Timeout::Unlimited);
}

// FUNCTION protected
//	Action::LockerImpl::Base::tupleUnlock -- 
//
// NOTES
//
// ARGUMENTS
//	unsigned int iRowID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerImpl::Base::
tupleUnlock(unsigned int iRowID_)
{
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Lock)) {
		_OPT_EXECUTION_MESSAGE
			<< "tupleUnlock: " << m_cArgument.m_cTableName
			<< " " << iRowID_
			<< " " << m_cArgument.m_eMode
			<< ":" << m_cArgument.m_eDuration
			<< ModEndl;
	}
#endif
	return m_pTransaction->unlock(getLockName(iRowID_),
								  m_cArgument.m_eMode,
								  m_cArgument.m_eDuration);
}

// FUNCTION protected
//	Action::LockerImpl::Base::unlockAll -- 
//
// NOTES
//
// ARGUMENTS
//	const VECTOR<unsigned int>& vecRowID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerImpl::Base::
unlockAll(const VECTOR<unsigned int>& vecRowID_)
{
	FOREACH(vecRowID_,
			boost::bind(&This::tupleUnlock,
						this,
						_1));
}

// FUNCTION protected
//	Action::LockerImpl::Base::unlockAll -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::BitSet::ConstIterator& cBegin_
//	const Common::BitSet::ConstIterator& cEnd_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerImpl::Base::
unlockAll(const Common::BitSet::ConstIterator& cBegin_,
		  const Common::BitSet::ConstIterator& cEnd_)
{
	FOREACH(cBegin_,
			cEnd_,
			boost::bind(&This::tupleUnlock,
						this,
						_1));
}

// FUNCTION private
//	Action::LockerImpl::Base::getLockName -- 
//
// NOTES
//
// ARGUMENTS
//	unsigned int iRowID_
//	
// RETURN
//	Lock::TupleName
//
// EXCEPTIONS

Lock::TupleName
LockerImpl::Base::
getLockName(unsigned int iRowID_)
{
	return Lock::TupleName(m_cArgument.m_iDatabaseID,
						   m_cArgument.m_iTableID,
						   iRowID_);
}

///////////////////////////////////////////////
// Execution::Action::LockerImpl::Normal

// FUNCTION public
//	Action::LockerImpl::Normal::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::Normal::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	m_cRowID.initialize(cProgram_);
}

// FUNCTION public
//	Action::LockerImpl::Normal::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::Normal::
terminate(Interface::IProgram& cProgram_)
{
	unlockAll(m_vecUnlockID);
	m_vecUnlockID.clear();
	terminateBase(cProgram_);
	m_cRowID.terminate(cProgram_);
}

// FUNCTION public
//	Action::LockerImpl::Normal::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Common::DataArrayData* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
LockerImpl::Normal::
getData(Interface::IProgram& cProgram_,
		Common::DataArrayData* pData_)
{
	if (isNeedLock() == false) {
		return getLogicalFile().getData(pData_);
	}

	for (;;) {
		LogicalFile::AutoLogicalFile::AutoUnlatch l = getLogicalFile().latch();
		if (getLogicalFile().getData(pData_) == false) {
			// no more data

			if (isStatementLock() == false) {
				// unlock previous tuples if needed
				unlockAll(m_vecUnlockID);
				m_vecUnlockID.clear();
			}

			return false;
		}
		// m_cRowID should have data
		unsigned int iRowID = getRowID()->getValue();

		// try lock
		if (tupleTryLock(iRowID) == false) {
			// couldn't lock
			_EXECUTION_FAKE_ERROR(Filie::rewind, Unexpected);
			getLogicalFile().rewind();
			// explicitly unlatch
			l.unlatch();

			// wait lock (duration is instant)
			tupleWaitLock(iRowID);
			continue;
		}
		// now have the lock
		_EXECUTION_FAKE_ERROR(Filie::mark, Unexpected);
		getLogicalFile().mark();

		if (isStatementLock() == false) {
			// unlock previous tuples if needed
			unlockAll(m_vecUnlockID);
			m_vecUnlockID.clear();
		}
		if (isCursorLock() || isStatementLock()) {
			// record current rowid to unlock at next tuple
			m_vecUnlockID.PUSHBACK(iRowID);
		}

		// go out of the loop
		break;
	}

	return true;
}

// FUNCTION public
//	Action::LockerImpl::Normal::fetch -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::DataArrayData* pOption_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::Normal::
fetch(const Common::DataArrayData* pOption_)
{
	getLogicalFile().fetch(pOption_);

	if (isNeedLock()) {
		// unlock previous tuples if needed
		unlockAll(m_vecUnlockID);
		m_vecUnlockID.clear();
	}
}

// FUNCTION public
//	Action::LockerImpl::Normal::unlock -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::Normal::
unlock()
{
	if (isNeedLock() && isCursorLock() == false && isStatementLock() == false) {
		m_vecUnlockID.PUSHBACK(getRowID()->getValue());
	}
}

// FUNCTION public
//	Action::LockerImpl::Normal::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::Normal::
reset()
{
	getLogicalFile().reset();

	if (isNeedLock()) {
		// unlock previous tuples if needed
		unlockAll(m_vecUnlockID);
		m_vecUnlockID.clear();
	}
}

// FUNCTION public
//	Action::LockerImpl::Normal::close -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::Normal::
close()
{
	getLogicalFile().close();

	if (isNeedLock() && isStatementLock() == false) {
		// unlock previous tuples if needed
		unlockAll(m_vecUnlockID);
		m_vecUnlockID.clear();
	}
}

// FUNCTION public
//	Action::LockerImpl::Normal::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
LockerImpl::Normal::
getClassID() const
{
	return Class::getClassID(Class::Category::LockerNormal);
}

// FUNCTION public
//	Action::LockerImpl::Normal::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerImpl::Normal::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cRowID.serialize(archiver_);
}

///////////////////////////////////////////////
// Execution::Action::LockerImpl::GetByBitSet

// FUNCTION public
//	Action::LockerImpl::GetByBitSet::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::GetByBitSet::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	m_cBitSet.initialize(cProgram_);
}

// FUNCTION public
//	Action::LockerImpl::GetByBitSet::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::GetByBitSet::
terminate(Interface::IProgram& cProgram_)
{
	// Following code can not be valid until
	// redundant unlocking is allowed
#if 0
	if (isNeedLock()
		&& (isCursorLock() || isStatementLock())) {
		if (m_cBitSet.isInitialized()) {
			unlockAll(m_cBitSet->begin(), m_cBitSet->end());
			m_cBitSet->clear();
		}
	}
#endif

	terminateBase(cProgram_);
	m_cBitSet.terminate(cProgram_);
}

// FUNCTION public
//	Action::LockerImpl::GetByBitSet::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Common::DataArrayData* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
LockerImpl::GetByBitSet::
getData(Interface::IProgram& cProgram_,
		Common::DataArrayData* pData_)
{
	if (isNeedLock() == false) {
		return getLogicalFile().getData(pData_);
	}

	for (;;) {
		LogicalFile::AutoLogicalFile::AutoUnlatch l = getLogicalFile().latch();
		if (getLogicalFile().getData(pData_) == false) {
			// no data
			return false;
		}
		// m_cBitSet should have data
		const Common::BitSet* pBitSet = m_cBitSet.getData();

		if (pBitSet->none()) {
			// empty result
			return true;
		}

		Common::BitSet::ConstIterator iterator = pBitSet->begin();
		const Common::BitSet::ConstIterator last = pBitSet->end();
		for (; iterator != last; ++iterator) {
			// try lock
			if (tupleTryLock(*iterator) == false) {
				// couldn't lock
				break;
			}
		}

		if (iterator == last) {
			// all rowids are locked
			// -> go out of the loop
			break;
		}

		// couldn't lock all
		// -> close file and explicitly unlatch
		getFileAccess()->close();
		l.unlatch();

		// unlock all locked tuples
		unlockAll(pBitSet->begin(), iterator);

		// wait lock (duration is instant)
		tupleWaitLock(*iterator);

		// open file to get results again
		getFileAccess()->open(cProgram_);
	}

	return true;
}

// FUNCTION public
//	Action::LockerImpl::GetByBitSet::fetch -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::DataArrayData* pOption_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::GetByBitSet::
fetch(const Common::DataArrayData* pOption_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Action::LockerImpl::GetByBitSet::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::GetByBitSet::
reset()
{
	getLogicalFile().reset();
}

// FUNCTION public
//	Action::LockerImpl::GetByBitSet::close -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::GetByBitSet::
close()
{
	getLogicalFile().close();
}

// FUNCTION public
//	Action::LockerImpl::GetByBitSet::unlock -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::GetByBitSet::
unlock()
{
	; // do nothing
}

// FUNCTION public
//	Action::LockerImpl::GetByBitSet::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
LockerImpl::GetByBitSet::
getClassID() const
{
	return Class::getClassID(Class::Category::LockerBitSet);
}

// FUNCTION public
//	Action::LockerImpl::GetByBitSet::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerImpl::GetByBitSet::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cBitSet.serialize(archiver_);
}

// FUNCTION protected
//	Action::LockerImpl::GetByBitSet::setBitSet -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::BitSet& cBitSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerImpl::GetByBitSet::
setBitSet(const Common::BitSet& cBitSet_)
{
	if (m_cBitSet.isInitialized()) {
		*m_cBitSet = cBitSet_;
	}
}

////////////////////////////////////////////////////////////
// Execution::Action::LockerImpl::GetByBitSetCacheAllObject

// FUNCTION public
//	Action::LockerImpl::GetByBitSetCacheAllObject::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::GetByBitSetCacheAllObject::
initialize(Interface::IProgram& cProgram_)
{
	Super::initialize(cProgram_);
	m_bHasData = m_bHasNext = false;
}

// FUNCTION public
//	Action::LockerImpl::GetByBitSetCacheAllObject::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Common::DataArrayData* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
LockerImpl::GetByBitSetCacheAllObject::
getData(Interface::IProgram& cProgram_,
		Common::DataArrayData* pData_)
{
	if (m_bHasData) {
		// already obtained data
		if (m_bHasNext) {
			// return obtained bitset again
			setBitSet(m_cBitSet);
			m_bHasNext = false;
			return true;
		}
		return false;
	} else {
		if (Super::getData(cProgram_, pData_)) {
			// save bitset data
			m_cBitSet = *getBitSet();
			m_bHasData = true;
			m_bHasNext = false;
			return true;
		} else {
			m_bHasData = m_bHasNext = false;
			return false;
		}
	}
}

// FUNCTION public
//	Action::LockerImpl::GetByBitSetCacheAllObject::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::GetByBitSetCacheAllObject::
reset()
{
	if (isNeedLock() == false) {
		getLogicalFile().reset();
	}
	m_bHasNext = m_bHasData;
}

// FUNCTION public
//	Action::LockerImpl::GetByBitSetCacheAllObject::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
LockerImpl::GetByBitSetCacheAllObject::
getClassID() const
{
	return Class::getClassID(Class::Category::LockerBitSetCacheAll);
}

///////////////////////////////////////////////
// Execution::Action::LockerImpl::CacheAllObject

// FUNCTION public
//	Action::LockerImpl::CacheAllObject::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::CacheAllObject::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	m_cRowID.initialize(cProgram_);
}

// FUNCTION public
//	Action::LockerImpl::CacheAllObject::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::CacheAllObject::
terminate(Interface::IProgram& cProgram_)
{
	// Following code can not be valid until
	// redundant unlocking is allowed
#if 0
	if (isNeedLock()
		&& (isCursorLock() || isStatementLock())) {
		// unlock obtained tuples
		unlockAll(m_vecRowID);
		m_vecRowID.clear();
	}
#endif

	terminateBase(cProgram_);
	m_cRowID.terminate(cProgram_);
}

// FUNCTION public
//	Action::LockerImpl::CacheAllObject::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Common::DataArrayData* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
LockerImpl::CacheAllObject::
getData(Interface::IProgram& cProgram_,
		Common::DataArrayData* pData_)
{
	if (isNeedLock() == false) {
		return getLogicalFile().getData(pData_);
	}

	if (m_vecRowID.ISEMPTY()) {
		// store all tuples locking
		storeTuples(cProgram_, pData_);

		if (m_vecRowID.ISEMPTY()) {
			// no data
			return false;
		}
	}

	if (++m_iCursor >= m_vecData.GETSIZE()) {
		// all data has read
		return false;
	}

	Utility::DataType::assignElements(pData_,
									  _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*,
														   m_vecData[m_iCursor].get()));
	return true;

}

// FUNCTION public
//	Action::LockerImpl::CacheAllObject::fetch -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::DataArrayData* pOption_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::CacheAllObject::
fetch(const Common::DataArrayData* pOption_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Action::LockerImpl::CacheAllObject::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::CacheAllObject::
reset()
{
	if (isNeedLock() == false) {
		return getLogicalFile().reset();
	}
	m_iCursor = -1;
}

// FUNCTION public
//	Action::LockerImpl::CacheAllObject::close -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::CacheAllObject::
close()
{
	m_iCursor = -1;
	getLogicalFile().close();
}

// FUNCTION public
//	Action::LockerImpl::CacheAllObject::unlock -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::CacheAllObject::
unlock()
{
	; // do nothing
}

// FUNCTION public
//	Action::LockerImpl::CacheAllObject::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
LockerImpl::CacheAllObject::
getClassID() const
{
	return Class::getClassID(Class::Category::LockerCacheAll);
}

// FUNCTION public
//	Action::LockerImpl::CacheAllObject::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerImpl::CacheAllObject::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cRowID.serialize(archiver_);
}

// FUNCTION private
//	Action::LockerImpl::CacheAllObject::storeTuples -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Common::DataArrayData* pData_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerImpl::CacheAllObject::
storeTuples(Interface::IProgram& cProgram_,
			Common::DataArrayData* pData_)
{
	// latch until all data is gotten and file is closed
	LogicalFile::AutoLogicalFile::AutoUnlatch l = getLogicalFile().latch();
	for (;;) {
		if (getLogicalFile().getData(pData_) == false) {
			// no more data
			break;
		}
		// m_cRowID should have data
		unsigned int iRowID = getRowID()->getValue();

		// try lock
		if (tupleTryLock(iRowID) == false) {
			// couldn't lock

			// close file and explicitly unlatch
			getFileAccess()->close();
			l.unlatch();

			// unlock obtained tuples
			unlockAll(m_vecRowID);
			m_vecRowID.clear();
			m_vecData.clear();

			// wait lock (duration is instant)
			tupleWaitLock(iRowID);

			// latch file and open again
			l.latch();
			getFileAccess()->open(cProgram_);

			continue;
		}

		// record locked tuple
		m_vecRowID.PUSHBACK(iRowID);

		// record data
		m_vecData.PUSHBACK(pData_->copy());

		// obtain next tuple
	}

	// explicitly close the file before unlatch
	getFileAccess()->close();
}



///////////////////////////////////////////////
// Execution::Action::LockerImpl::BitSetSort

// FUNCTION public
//	Action::LockerImpl::BitSetSort::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::BitSetSort::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	m_cBitSet.initialize(cProgram_);
	m_cLockedBitSet.reset();
}

// FUNCTION public
//	Action::LockerImpl::BitSetSort::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::BitSetSort::
terminate(Interface::IProgram& cProgram_)
{
	if (isNeedLock()
		&& (isCursorLock() || isStatementLock())) {
		unlockAll(m_cLockedBitSet.begin(), m_cLockedBitSet.end());
	}
	terminateBase(cProgram_);
	m_cBitSet.terminate(cProgram_);
	m_cLockedBitSet.reset();
}

// FUNCTION public
//	Action::LockerImpl::BitSetSort::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Common::DataArrayData* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
LockerImpl::BitSetSort::
getData(Interface::IProgram& cProgram_,
		Common::DataArrayData* pData_)
{
	if (isNeedLock() == false) {
		return getLogicalFile().getData(pData_);
	}

	if (m_cLockedBitSet.none()) {
		// store all tuples locking
		storeTuples(cProgram_, pData_);

		if (m_cLockedBitSet.none()) {
			// no data
			return false;
		}
	}

	if (++m_iCursor >= m_vecData.GETSIZE()) {
		// all data has read
		return false;
	}

	Utility::DataType::assignElements(pData_,
									  _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*,
														   m_vecData[m_iCursor].get()));
	return true;

}

// FUNCTION public
//	Action::LockerImpl::BitSetSort::fetch -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::DataArrayData* pOption_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::BitSetSort::
fetch(const Common::DataArrayData* pOption_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Action::LockerImpl::BitSetSort::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::BitSetSort::
reset()
{
	if (isNeedLock() == false) {
		return getLogicalFile().reset();
	}
	m_iCursor = -1;
}

// FUNCTION public
//	Action::LockerImpl::BitSetSort::close -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::BitSetSort::
close()
{
	m_iCursor = -1;
	getLogicalFile().close();
}

// FUNCTION public
//	Action::LockerImpl::BitSetSort::unlock -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::BitSetSort::
unlock()
{
	; // do nothing
}

// FUNCTION public
//	Action::LockerImpl::BitSetSort::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
LockerImpl::BitSetSort::
getClassID() const
{
	return Class::getClassID(Class::Category::LockerBitSetSort);
}

// FUNCTION public
//	Action::LockerImpl::BitSetSort::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerImpl::BitSetSort::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cBitSet.serialize(archiver_);
}

// FUNCTION private
//	Action::LockerImpl::BitSetSort::storeTuples -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Common::DataArrayData* pData_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerImpl::BitSetSort::
storeTuples(Interface::IProgram& cProgram_,
			Common::DataArrayData* pData_)
{

	for (;;) {
		// latch until all data is gotten and file is closed
		LogicalFile::AutoLogicalFile::AutoUnlatch l = getLogicalFile().latch();
		if (getLogicalFile().getData(pData_) == false) {
			// no more data
			break;
		}
		// m_cBitSet should have data
		const Common::BitSet* pBitSet = getBitSet().getData();
		Common::BitSet::ConstIterator iterator = pBitSet->begin();
		const Common::BitSet::ConstIterator last = pBitSet->end();
		for (; iterator != last; ++iterator) {
			// try lock
			if (!m_cLockedBitSet[*iterator]) {
				if (tupleTryLock(*iterator) == false) {

					// couldn't lock all
					// -> explicitly unlatch
					l.unlatch();

					// unlock obtained tuples
					unlockAll(m_cLockedBitSet.begin(), m_cLockedBitSet.end());
					m_cLockedBitSet.clear();
					m_vecData.clear();

					// wait lock (duration is instant)
					tupleWaitLock(*iterator);

					// latch file and open again
					getFileAccess()->close();
					getFileAccess()->open(cProgram_);
					break;					
				}
				// record locked tuple				
				m_cLockedBitSet[*iterator] = true;
			}
		}

		if (iterator == last) {
			// record data
			m_vecData.PUSHBACK(pData_->copy());
		}
	}

	// explicitly close the file before unlatch
	getFileAccess()->close();
}



///////////////////////////////////////////////
// Execution::Action::LockerImpl::Unlocker

// FUNCTION public
//	Action::LockerImpl::Unlocker::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::Unlocker::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	m_cRowID.initialize(cProgram_);
}

// FUNCTION public
//	Action::LockerImpl::Unlocker::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::Unlocker::
terminate(Interface::IProgram& cProgram_)
{
	terminateBase(cProgram_);
	m_cRowID.terminate(cProgram_);
}

// FUNCTION public
//	Action::LockerImpl::Unlocker::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Common::DataArrayData* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
LockerImpl::Unlocker::
getData(Interface::IProgram& cProgram_,
		Common::DataArrayData* pData_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Action::LockerImpl::Unlocker::fetch -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::DataArrayData* pOption_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::Unlocker::
fetch(const Common::DataArrayData* pOption_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Action::LockerImpl::Unlocker::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::Unlocker::
reset()
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Action::LockerImpl::Unlocker::close -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::Unlocker::
close()
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Action::LockerImpl::Unlocker::unlock -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
LockerImpl::Unlocker::
unlock()
{
	tupleUnlock(getRowID()->getValue());
}

// FUNCTION public
//	Action::LockerImpl::Unlocker::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
LockerImpl::Unlocker::
getClassID() const
{
	return Class::getClassID(Class::Category::LockerUnlocker);
}

// FUNCTION public
//	Action::LockerImpl::Unlocker::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerImpl::Unlocker::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cRowID.serialize(archiver_);
}

////////////////////////////////////////
// Execution::Action::Locker::Normal

// FUNCTION public
//	Action::Locker::Normal::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	FileAccess* pFileAccess_
//	const Argument& cArgument_
//	int iDataID_
//	
// RETURN
//	Locker*
//
// EXCEPTIONS

//static
Locker*
Locker::Normal::
create(Interface::IProgram& cProgram_,
	   FileAccess* pFileAccess_,
	   const Argument& cArgument_,
	   int iDataID_)
{
	AUTOPOINTER<This> pLocker = new LockerImpl::Normal(pFileAccess_,
													   cArgument_,
													   iDataID_);
	pLocker->registerToProgram(cProgram_);
	return pLocker.release();
}

////////////////////////////////////////
// Execution::Action::Locker::GetByBitSet

// FUNCTION public
//	Action::Locker::GetByBitSet::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	FileAccess* pFileAccess_
//	const Argument& cArgument_
//	int iDataID_
//	
// RETURN
//	Locker*
//
// EXCEPTIONS

//static
Locker*
Locker::GetByBitSet::
create(Interface::IProgram& cProgram_,
	   FileAccess* pFileAccess_,
	   const Argument& cArgument_,
	   int iDataID_)
{
	AUTOPOINTER<This> pLocker = new LockerImpl::GetByBitSet(pFileAccess_,
															cArgument_,
															iDataID_);
	pLocker->registerToProgram(cProgram_);
	return pLocker.release();
}

////////////////////////////////////////
// Execution::Action::Locker::GetByBitSetCacheAllObject

// FUNCTION public
//	Action::Locker::GetByBitSetCacheAllObject::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	FileAccess* pFileAccess_
//	const Argument& cArgument_
//	int iDataID_
//	
// RETURN
//	Locker*
//
// EXCEPTIONS

//static
Locker*
Locker::GetByBitSetCacheAllObject::
create(Interface::IProgram& cProgram_,
	   FileAccess* pFileAccess_,
	   const Argument& cArgument_,
	   int iDataID_)
{
	AUTOPOINTER<This> pLocker = new LockerImpl::GetByBitSetCacheAllObject(pFileAccess_,
																		  cArgument_,
																		  iDataID_);
	pLocker->registerToProgram(cProgram_);
	return pLocker.release();
}

/////////////////////////////////////////////////
// Execution::Action::Locker::CacheAllObject

// FUNCTION public
//	Action::Locker::CacheAllObject::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	FileAccess* pFileAccess_
//	const Argument& cArgument_
//	int iDataID_
//	
// RETURN
//	Locker*
//
// EXCEPTIONS

//static
Locker*
Locker::CacheAllObject::
create(Interface::IProgram& cProgram_,
	   FileAccess* pFileAccess_,
	   const Argument& cArgument_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new LockerImpl::CacheAllObject(pFileAccess_,
															   cArgument_,
															   iDataID_);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}


/////////////////////////////////////////////////
// Execution::Action::Locker::BitSetSort

// FUNCTION public
//	Action::Locker::BitSetSort::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	FileAccess* pFileAccess_
//	const Argument& cArgument_
//	int iDataID_
//	
// RETURN
//	Locker*
//
// EXCEPTIONS

//static
Locker*
Locker::BitSetSort::
create(Interface::IProgram& cProgram_,
	   FileAccess* pFileAccess_,
	   const Argument& cArgument_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new LockerImpl::BitSetSort(pFileAccess_,
														   cArgument_,
														   iDataID_);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}


////////////////////////////////////////////
// Execution::Action::Locker::Unlocker

// FUNCTION public
//	Action::Locker::Unlocker::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Argument& cArgument_
//	int iDataID_
//	
// RETURN
//	Locker*
//
// EXCEPTIONS

//static
Locker*
Locker::Unlocker::
create(Interface::IProgram& cProgram_,
	   const Argument& cArgument_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new LockerImpl::Unlocker(cArgument_,
														 iDataID_);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

////////////////////////////////////
// Execution::Action::Locker

// FUNCTION public
//	Action::Locker::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Locker*
//
// EXCEPTIONS

//static
Locker*
Locker::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::LockerNormal:
		{
			return new LockerImpl::Normal;
		}
	case Class::Category::LockerBitSet:
		{
			return new LockerImpl::GetByBitSet;
		}
	case Class::Category::LockerBitSetCacheAll:
		{
			return new LockerImpl::GetByBitSetCacheAllObject;
		}
	case Class::Category::LockerCacheAll:
		{
			return new LockerImpl::CacheAllObject;
		}

	case Class::Category::LockerBitSetSort:
		{
			return new LockerImpl::BitSetSort;
		}
	case Class::Category::LockerUnlocker:
		{
			return new LockerImpl::Unlocker;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

// FUNCTION protected
//	Action::Locker::registerToProgram -- register to program
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Locker::
registerToProgram(Interface::IProgram& cProgram_)
{
	// Instance ID is obtained by registerLocker method.
	setID(cProgram_.registerLocker(this));
}

//////////////////////////////////////////
// Execution::Action::LockerHolder

// FUNCTION public
//	Action::LockerHolder::initialize -- initialize Locker instance
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerHolder::
initialize(Interface::IProgram& cProgram_)
{
	if (isValid() && !isInitialized()) {
		m_pLocker = cProgram_.getLocker(m_iID);
		if (m_pLocker == 0) {
			_SYDNEY_THROW0(Exception::Unexpected);
		}
		m_pLocker->initialize(cProgram_);
	}
}

// FUNCTION public
//	Action::LockerHolder::terminate -- terminate Locker instance
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerHolder::
terminate(Interface::IProgram& cProgram_)
{
	if (m_pLocker) {
		m_pLocker->terminate(cProgram_);
		m_pLocker = 0;
	}
}

// FUNCTION public
//	Action::LockerHolder::serialize -- serializer
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LockerHolder::
serialize(ModArchive& archiver_)
{
	archiver_(m_iID);
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2015, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
