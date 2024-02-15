// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Collection/VirtualTable.cpp --
// 
// Copyright (c) 2011, 2012, 2017, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Collection";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Collection/VirtualTable.h"
#include "Execution/Collection/Class.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerData.h"
#include "Common/StringData.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

#include "Os/Limits.h"

#include "Schema/VirtualTable.h"

#include "Server/InstanceManager.h"
#include "Server/Manager.h"
#include "Server/Session.h"
#include "Server/UserList.h"
#include "Server/Transaction.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_COLLECTION_BEGIN

namespace
{
	struct _Type
	{
		enum Value {
			User,
			Session,
			ValueNum
		};
	};
	const char* const _pszTableName[] =
	{
		"system_user",
		"system_session"
	};
}

namespace VirtualTableImpl
{
	// CLASS
	//	Execution::Collection::VirtualTableImpl::Base -- base class of virtualtable implementation class
	//
	// NOTES

	class Base
		: public Collection::VirtualTable
	{
	public:
		typedef Collection::VirtualTable Super;
		typedef Base This;

		// destructor
		virtual ~Base() {}

		// CLASS
		//	VirtualTableImpl::GetImpl -- implementation of get interface
		//
		// NOTES
		class GetImpl
			: public Super::Get
		{
		public:
			GetImpl() : m_pOuter(0) {}
			~GetImpl() {}

			void setOuter(This* pOuter_) {m_pOuter = pOuter_;}

		/////////////////////////////
		// Super::Get
			virtual void finish(Interface::IProgram& cProgram_);
			virtual void terminate(Interface::IProgram& cProgram_);
			virtual bool getData(Interface::IProgram& cProgram_,
								 Common::Data* pData_);
			virtual bool get(Interface::IProgram& cProgram_,
							 Common::Externalizable* pObject_);
			virtual void reset();
		protected:
		private:
			This* m_pOuter;
		};

	////////////////////////
	// ICollection::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual void clear();
		virtual bool isEmpty();
	//	virtual bool isEmptyGrouping();

		virtual Put* getPutInterface() {return 0;}
		virtual Get* getGetInterface() {return &m_cGet;}

	///////////////////////////////
	// Common::Externalizable
	//	int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
		// constructor
		Base()
			: Super(),
			  m_vecPosition(),
			  m_cGet()
		{}
		Base(const VECTOR<int>& vecPosition_)
			: Super(),
			  m_vecPosition(vecPosition_),
			  m_cGet()
		{}
	private:
		friend class GetImpl;

		// get data
		bool getData(Interface::IProgram& cProgram_,
					 Common::Data* pData_);
		// get next entry
		virtual bool getNextData(Interface::IProgram& cProgram_,
								 Common::DataArrayData* pData_,
								 const VECTOR<int>& vecPosition_) = 0;
		// do at last
		virtual void finish(Interface::IProgram& cProgram_) = 0;
		// go to first entry
		virtual void reset() = 0;

		VECTOR<int> m_vecPosition;
		GetImpl m_cGet;
	}; // class Base

	// CLASS
	//	Execution::Collection::VirtualTableImpl::User -- implementation class of user virtual table
	//
	// NOTES
	class User
		: public Base
	{
	public:
		typedef Base Super;
		typedef User This;

		// constructor
		User()
			: Super(),
			  m_iLastID(-1),
			  m_pUserList(0)
		{}
		User(const VECTOR<int>& vecPosition_)
			: Super(vecPosition_),
			  m_iLastID(-1),
			  m_pUserList(0)
		{}
		// destructor
		~User() {}

	////////////////////////
	// ICollection::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
		// prepare userlist
		void prepare();
		// set element of one entry
		void setElement(Common::DataArrayData* pData_,
						Server::UserList::Entry* pEntry_,
						const STRING& cstrName_,
						int iPosition_,
						int iElement_);

	//////////////////////////////
	// VirtualTableImpl::Base::
		virtual bool getNextData(Interface::IProgram& cProgram_,
								 Common::DataArrayData* pData_,
								 const VECTOR<int>& vecPosition_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset();

		Server::UserList::Entry::ID m_iLastID;
		Server::UserList* m_pUserList;
	}; // class User

	// CLASS
	//	Execution::Collection::VirtualTableImpl::Session -- implementation class of session virtual table
	//
	// NOTES
	class Session
		: public Base
	{
	public:
		typedef Base Super;
		typedef Session This;

		// constructor
		Session()
			: Super(),
			  m_iClientID(0),
			  m_iSessionID(0),
			  m_uiTupleID(0)
		{}
		Session(const VECTOR<int>& vecPosition_)
			: Super(vecPosition_),
			  m_iClientID(0),
			  m_iSessionID(0),
			  m_uiTupleID(0)
		{}
		// destructor
		~Session() {}

	////////////////////////
	// ICollection::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
		class AutoLockMap
		{
		public:
			AutoLockMap() { Server::InstanceManager::lockMap(); }
			~AutoLockMap() { Server::InstanceManager::unlockMap(); }
		};

		class AutoLock
		{
		public:
			AutoLock(Server::InstanceManager* p) : manager(p)
				{ manager->lock(); }
			~AutoLock()
				{ manager->unlock(); }
		private:
			Server::InstanceManager* manager;
		};

		// set element of one entry
		void setElement(Common::DataArrayData* pData_,
						Server::InstanceManager* pEntry_,
						Server::Session* pSession_,
						int iPosition_,
						int iElement_);

	//////////////////////////////
	// VirtualTableImpl::Base::
		virtual bool getNextData(Interface::IProgram& cProgram_,
								 Common::DataArrayData* pData_,
								 const VECTOR<int>& vecPosition_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset();

		Server::ID m_iClientID;
		Server::ID m_iSessionID;
		unsigned int m_uiTupleID;
	}; // class Session
}

///////////////////////////////////////////////////////////
// Execution::Collection::VirtualTableImpl::Base::GetImpl

// FUNCTION public
//	Collection::VirtualTableImpl::Base::GetImpl::finish -- 
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
VirtualTableImpl::Base::GetImpl::
finish(Interface::IProgram& cProgram_)
{
	m_pOuter->finish(cProgram_);
}

// FUNCTION public
//	Collection::VirtualTableImpl::Base::GetImpl::terminate -- 
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
VirtualTableImpl::Base::GetImpl::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::VirtualTableImpl::Base::GetImpl::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
bool
VirtualTableImpl::Base::GetImpl::
getData(Interface::IProgram& cProgram_,
		Common::Data* pData_)
{
	return m_pOuter->getData(cProgram_, pData_);
}

// FUNCTION public
//	Collection::VirtualTableImpl::Base::GetImpl::get -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Externalizable* pObject_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
bool
VirtualTableImpl::Base::GetImpl::
get(Interface::IProgram& cProgram_,
	Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::VirtualTableImpl::Base::GetImpl::reset -- 
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
VirtualTableImpl::Base::GetImpl::
reset()
{
	m_pOuter->reset();
}

/////////////////////////////////////////////////
// Execution::Collection::VirtualTableImpl::Base

// FUNCTION public
//	Collection::VirtualTableImpl::Base::initialize -- 
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
VirtualTableImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	m_cGet.setOuter(this);
}

// FUNCTION public
//	Collection::VirtualTableImpl::Base::terminate -- 
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
VirtualTableImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::VirtualTableImpl::Base::clear -- 
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
VirtualTableImpl::Base::
clear()
{
	; // do nothing
}

// FUNCTION public
//	Collection::VirtualTableImpl::Base::isEmpty -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
VirtualTableImpl::Base::
isEmpty()
{
	// regard as always not empty
	return false;
}

// FUNCTION public
//	Collection::VirtualTableImpl::Base::serialize -- 
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
VirtualTableImpl::Base::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	Utility::SerializeValue(archiver_,
							m_vecPosition);
}

// FUNCTION private
//	Collection::VirtualTableImpl::Base::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Common::Data* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
VirtualTableImpl::Base::
getData(Interface::IProgram& cProgram_,
		Common::Data* pData_)
{
	; _SYDNEY_ASSERT(pData_->getType() == Common::DataType::Array
					 && pData_->getElementType() == Common::DataType::Data);
	Common::DataArrayData* pArrayData = _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pData_);
	; _SYDNEY_ASSERT(pArrayData);

	int n = pArrayData->getCount();
	if (n != static_cast<int>(m_vecPosition.GETSIZE())) {
		// array size should be same as position list
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	return getNextData(cProgram_, pArrayData, m_vecPosition);
}

/////////////////////////////////////////////////
// Execution::Collection::VirtualTableImpl::User

// FUNCTION public
//	Collection::VirtualTableImpl::User::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
VirtualTableImpl::User::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszTableName[_Type::User]);
}

// FUNCTION public
//	Collection::VirtualTableImpl::User::terminate -- 
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
VirtualTableImpl::User::
terminate(Interface::IProgram& cProgram_)
{
	if (m_pUserList) {
		// finish has not been called because of error or some reason
		finish(cProgram_);
	}
}

// FUNCTION public
//	Collection::VirtualTableImpl::User::getClassID -- 
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
VirtualTableImpl::User::
getClassID() const
{
	return Class::getClassID(Class::Category::VirtualTableUser);
}

// FUNCTION private
//	Collection::VirtualTableImpl::User::prepare -- prepare userlist
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

void
VirtualTableImpl::User::
prepare()
{
	m_pUserList = Server::Manager::getUserList();
	if (m_pUserList == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	// lock user list
	m_pUserList->getLock().lock(Os::RWLock::Mode::Read);
}

// FUNCTION private
//	Collection::VirtualTableImpl::User::setElement -- 
//
// NOTES
//
// ARGUMENTS
//	Common::DataArrayData* pData_
//	Server::UserList::Entry* pEntry_
//	const STRING& cstrName_
//	int iPosition_
//	int iElement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
VirtualTableImpl::User::
setElement(Common::DataArrayData* pData_,
		   Server::UserList::Entry* pEntry_,
		   const STRING& cstrName_,
		   int iPosition_,
		   int iElement_)
{
	const Common::Data::Pointer pElement = pData_->getElement(iElement_);
	; _SYDNEY_ASSERT(pElement.get());

	switch (iPosition_) {
	case 0: // RowID == UserID
	case 1: // UserID
		{
			Common::UnsignedIntegerData cID(pEntry_->getID());
			pElement->assign(&cID);
			break;
		}
	case 2: // UserName
		{
			Common::StringData cName(cstrName_);
			pElement->assign(&cName);
			break;
		}
	case 3: // Type
		{
			Common::IntegerData cType(pEntry_->getCategory());
			pElement->assign(&cType);
			break;
		}
	default:
		{
			pElement->setNull();
			break;
		}
	}
}

// FUNCTION private
//	Collection::VirtualTableImpl::User::getNextData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Common::DataArrayData* pData_
//	const VECTOR<int>& vecPosition_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
VirtualTableImpl::User::
getNextData(Interface::IProgram& cProgram_,
			Common::DataArrayData* pData_,
			const VECTOR<int>& vecPosition_)
{
	if (m_pUserList == 0) prepare();
	; _SYDNEY_ASSERT(m_pUserList);

	STRING cstrName;
	Server::UserList::Entry::Pointer pEntry;

	if (m_iLastID < Os::Limits<Server::UserList::Entry::ID>::getMax()
		&& m_pUserList->getNext(++m_iLastID, cstrName, pEntry)) {
		m_iLastID = pEntry->getID();

		// set element data
		Opt::ForEach_i(vecPosition_,
					   boost::bind(&This::setElement,
								   this,
								   pData_,
								   pEntry.get(),
								   boost::cref(cstrName),
								   _1,
								   _2));
		return true;
	}
	// no more data
	m_iLastID = Os::Limits<Server::UserList::Entry::ID>::getMax();
	return false;
}

// FUNCTION private
//	Collection::VirtualTableImpl::User::finish -- 
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
VirtualTableImpl::User::
finish(Interface::IProgram& cProgram_)
{
	if (m_pUserList) {
		m_pUserList->getLock().unlock(Os::RWLock::Mode::Read);
		m_pUserList = 0;
	}
}

// FUNCTION private
//	Collection::VirtualTableImpl::User::reset -- 
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
VirtualTableImpl::User::
reset()
{
	m_iLastID = -1;
}

//////////////////////////////////////////////////////
// Execution::Collection::VirtualTableImpl::Session

// FUNCTION public
//	Collection::VirtualTableImpl::Session::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
VirtualTableImpl::Session::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszTableName[_Type::Session]);
}

// FUNCTION public
//	Collection::VirtualTableImpl::Session::getClassID -- 
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
VirtualTableImpl::Session::
getClassID() const
{
	return Class::getClassID(Class::Category::VirtualTableSession);
}

// FUNCTION private
//	Collection::VirtualTableImpl::Session::setElement -- 
//
// NOTES
//
// ARGUMENTS
//	Common::DataArrayData* pData_
//	Server::InstanceManager* pEntry_
//	Server::Session* pSession_
//	int iPosition_
//	int iElement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
VirtualTableImpl::Session::
setElement(Common::DataArrayData* pData_,
		   Server::InstanceManager* pEntry_,
		   Server::Session* pSession_,
		   int iPosition_,
		   int iElement_)
{
	const Common::Data::Pointer pElement = pData_->getElement(iElement_);
	; _SYDNEY_ASSERT(pElement.get());

	switch (iPosition_) {
	case 0: // RowID
		{
			// generate here
			Common::UnsignedIntegerData cID(m_uiTupleID++);
			pElement->assign(&cID);
			break;
		}
	case 1: // ClientID
		{
			Common::IntegerData cID(pEntry_->getID());
			pElement->assign(&cID);
			break;
		}
	case 2: // HostName
		{
			Common::StringData cName(pEntry_->getHostName());
			pElement->assign(&cName);
			break;
		}
	case 3: // ConnectedTime
		{
			const Common::DateTimeData& cTime = pEntry_->getConnectedTime();
			pElement->assign(&cTime);
			break;
		}
	case 4: // ProtocolVersion
		{
			Common::IntegerData cVersion(pEntry_->getProtocolVersion());
			pElement->assign(&cVersion);
			break;
		}
	case 5: // CryptMode
		{
			if (pEntry_->getCryptMode()) {
				Common::StringData cMode(*pEntry_->getCryptMode());
				pElement->assign(&cMode);
			} else {
				pElement->setNull();
			}
			break;
		}
	case 6: // SessionID
		{
			if (pSession_) {
				Common::IntegerData cID(pSession_->getID());
				pElement->assign(&cID);
			} else {
				pElement->setNull();
			}
			break;
		}
	case 7: // DatabaseName
		{
			if (pSession_) {
				Common::StringData cName(pSession_->getDatabaseName());
				pElement->assign(&cName);
			} else {
				pElement->setNull();
			}
			break;
		}
	case 8: // UserName
		{
			if (pSession_ && pSession_->getUserID() != -1) {
				Common::StringData cName(pSession_->getUserName());
				pElement->assign(&cName);
			} else {
				pElement->setNull();
			}
			break;
		}
	case 9: // SessionStartTime
		{
			if (pSession_) {
				const Common::DateTimeData& cTime = pSession_->getStartTime();
				pElement->assign(&cTime);
			} else {
				pElement->setNull();
			}
			break;
		}
	case 10: // StatementType
		{
			if (pSession_ && pSession_->getStatementType() != Statement::ObjectType::Object) {
				Common::StringData cName(STRING(Statement::getTypeName(pSession_->getStatementType())));
				pElement->assign(&cName);
			} else {
				pElement->setNull();
			}
			break;
		}
	case 11: // TransactionState
		{
			
			if (pSession_) {
				Common::StringData cState(pSession_->getTransaction().getState());
				pElement->assign(&cState);
			} else {
				pElement->setNull();
			}
			break;
		}
	case 12: // TransactionStartTime
		{
			if (pSession_ && !pSession_->getTransaction().getStartTime().isNull()) {
				const Common::DateTimeData& cTime = pSession_->getTransaction().getStartTime();
				pElement->assign(&cTime);
			} else {
				pElement->setNull();
			}
			break;
		}
	case 13: // SQL Statement
		{
			ModAutoPointer<ModUnicodeString> sql;
			if (pSession_) {
				sql = pSession_->getCurrentSQL();
			}
			if (sql.get()) {
				Common::StringData cSQL(*sql);
				pElement->assign(&cSQL);
			} else {
				pElement->setNull();
			}
			break;
		}
	default:
		{
			pElement->setNull();
			break;
		}
	}
}

// FUNCTION private
//	Collection::VirtualTableImpl::Session::getNextData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Common::DataArrayData* pData_
//	const VECTOR<int>& vecPosition_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
VirtualTableImpl::Session::
getNextData(Interface::IProgram& cProgram_,
			Common::DataArrayData* pData_,
			const VECTOR<int>& vecPosition_)
{
	bool bResult = false;

	// Lock instanceManager map
	// During lock, new client connection will be blocked

	AutoLockMap cLock1;

	while (bResult == false) {
		// get instance manager
		Server::InstanceManagerPointer pEntry =
//		Server::InstanceManager* pEntry =
			Server::InstanceManager::lowerBound(m_iClientID);
		if (pEntry.get()) {
			// lock instance manager
			AutoLock cLock2(pEntry.get());

			Server::Session* pSession =
				pEntry->lowerBoundSession(m_iSessionID);

			if (pSession) {
				m_iSessionID = pSession->getID();
			} else if (m_iClientID == pEntry->getID()) {
				// no more session for the instance manager
				++m_iClientID; // go to next client
				m_iSessionID = 0;
				continue;
			}

			// set element data
			Opt::ForEach_i(vecPosition_,
						   boost::bind(&This::setElement,
									   this,
									   pData_,
									   pEntry.get(),
									   pSession,
									   _1,
									   _2));

			m_iClientID = pEntry->getID();
			++m_iSessionID;
			bResult = true;
		} else {
			// no more client
			m_iClientID = Os::Limits<Server::ID>::getMax();
			break;
		}
	}
	return bResult;
}

// FUNCTION private
//	Collection::VirtualTableImpl::Session::finish -- 
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
VirtualTableImpl::Session::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION private
//	Collection::VirtualTableImpl::Session::reset -- 
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
VirtualTableImpl::Session::
reset()
{
	m_iClientID = 0;
	m_iSessionID = 0;
	m_uiTupleID = 0;
}

///////////////////////////////////////////
// Execution::Collection::VirtualTable

// FUNCTION public
//	Collection::VirtualTable::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Schema::Table& cSchemaTable_
//	const VECTOR<int>& vecPosition_
//	
// RETURN
//	VirtualTable*
//
// EXCEPTIONS

//static
VirtualTable*
VirtualTable::
create(Interface::IProgram& cProgram_,
	   const Schema::Table& cSchemaTable_,
	   const VECTOR<int>& vecPosition_)
{
	; _SYDNEY_ASSERT(cSchemaTable_.isVirtual());

	const Schema::VirtualTable& cVirtualTable =
		_SYDNEY_DYNAMIC_CAST(const Schema::VirtualTable&, cSchemaTable_);

	AUTOPOINTER<This> pResult;
	switch (cVirtualTable.getCategory()) {
	case Schema::VirtualTable::Category::User:
		{
			pResult = new VirtualTableImpl::User(vecPosition_);
			break;
		}
	case Schema::VirtualTable::Category::Session:
		{
			pResult = new VirtualTableImpl::Session(vecPosition_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Collection::VirtualTable::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	VirtualTable*
//
// EXCEPTIONS

//static
VirtualTable*
VirtualTable::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::VirtualTableUser:
		{
			return new VirtualTableImpl::User;
		}
	case Class::Category::VirtualTableSession:
		{
			return new VirtualTableImpl::Session;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

_SYDNEY_EXECUTION_COLLECTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2017, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
