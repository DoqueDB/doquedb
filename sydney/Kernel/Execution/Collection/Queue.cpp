// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Collection/Queue.cpp --
// 
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Execution/Collection/Queue.h"
#include "Execution/Collection/Class.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"
#include "Execution/Externalizable.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"
#include "Exception/Cancel.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"

#include "Os/AutoCriticalSection.h"
#include "Os/Event.h"
#include "ModList.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_COLLECTION_BEGIN

namespace QueueImpl
{
	class Base
		: public Collection::Queue
	{
	public:
		typedef Collection::Queue Super;
		typedef Base This;

		// constructor
		Base()
			: Super(),
			  m_iMaxSize(0),
			  m_vecStorage(),
			  m_vecData()
		{}
		Base(SIZE iMaxSize_)
			: Super(),
			  m_iMaxSize(iMaxSize_),
			  m_vecStorage(),
			  m_vecData()
		{}

		// destructor
		virtual ~Base()
		{}

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
	//	virtual Put* getPutInterface();
	//	virtual Get* getGetInterface();

	///////////////////////////////
	// Common::Externalizable
	//	int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
		bool addData(const Common::Data* pData_);
		bool getData(Common::Data* pData_);
		void clearData();
		bool isFull() { return m_vecStorage.GETSIZE() >= m_iMaxSize; }

	private:
		SIZE m_iMaxSize;
		ModList<Common::Data::Pointer> m_vecStorage;// holding pointer itself (not queueed)
		ModList<Common::DataArrayData*> m_vecData;// casted data pointer (queueed)
	};

	// CLASS
	//	Execution::Collection::QueueImpl::Normal -- implementation class for collection::queue
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
			  m_cGet(),
			  m_cPut()
		{}
		Normal(SIZE iMaxSize_)
			: Super(iMaxSize_),
			  m_cGet(),
			  m_cPut()
		{}

		// destructor
		~Normal()
		{}

		// CLASS
		//	QueueImpl::PutImpl -- implementation of put interface
		//
		// NOTES
		class PutImpl
			: public Super::Put
		{
		public:
			PutImpl() : m_pOuter(0) {}
			~PutImpl() {}

			void setOuter(This* pOuter_) {m_pOuter = pOuter_;}

		/////////////////////////////
		// Super::Put
			virtual void finish(Interface::IProgram& cProgram_);
			virtual void terminate(Interface::IProgram& cProgram_);
			virtual bool putData(Interface::IProgram& cProgram_,
								 const Common::Data* pData_);
			virtual bool put(Interface::IProgram& cProgram_,
							 const Common::Externalizable* pObject_);
		protected:
		private:
			This* m_pOuter;
		};

		// CLASS
		//	QueueImpl::GetImpl -- implementation of get interface
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
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Put* getPutInterface() {return &m_cPut;}
		virtual Get* getGetInterface() {return &m_cGet;}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
		friend class PutImpl;
		friend class GetImpl;

		PutImpl m_cPut;
		GetImpl m_cGet;
	};

	// CLASS
	//	Execution::Collection::QueueImpl::Safe -- implementation class for collection::queue
	//
	// NOTES

	class Safe
		: public Base
	{
	public:
		typedef Base Super;
		typedef Safe This;

		// constructor
		Safe()
			: Super(),
			  m_cGet(),
			  m_cPut(),
			  m_cLatch(),
			  m_cReadEvent(),
			  m_cWriteEvent(),
			  m_iInitialized(0),
			  m_iFinished(0),
			  m_iTerminated(0),
			  m_bLast(false)
		{}
		Safe(SIZE iMaxSize_)
			: Super(iMaxSize_),
			  m_cGet(),
			  m_cPut(),
			  m_cLatch(),
			  m_cReadEvent(),
			  m_cWriteEvent(),
			  m_iInitialized(0),
			  m_iFinished(0),
			  m_iTerminated(0),
			  m_bLast(false)
		{}

		// destructor
		~Safe()
		{}

		// CLASS
		//	QueueImpl::PutImpl -- implementation of put interface
		//
		// NOTES
		class PutImpl
			: public Super::Put
		{
		public:
			PutImpl() : m_pOuter(0) {}
			~PutImpl() {}

			void setOuter(This* pOuter_) {m_pOuter = pOuter_;}

		/////////////////////////////
		// Super::Put
			virtual void finish(Interface::IProgram& cProgram_);
			virtual void terminate(Interface::IProgram& cProgram_);
			virtual bool putData(Interface::IProgram& cProgram_,
								 const Common::Data* pData_);
			virtual bool put(Interface::IProgram& cProgram_,
							 const Common::Externalizable* pObject_);
		protected:
		private:
			This* m_pOuter;
		};

		// CLASS
		//	QueueImpl::GetImpl -- implementation of get interface
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
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Put* getPutInterface() {return &m_cPut;}
		virtual Get* getGetInterface() {return &m_cGet;}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;


		bool setWasLast() {return m_bLast = true;}
		bool isLast() {return m_bLast;}
		
	protected:
	private:
		friend class PutImpl;
		friend class GetImpl;

		PutImpl m_cPut;
		GetImpl m_cGet;
		Os::CriticalSection m_cLatch;
		Os::Event m_cReadEvent;
		Os::Event m_cWriteEvent;

		int m_iInitialized;
		int m_iTerminated;
		int m_iFinished;
		bool m_bLast;
	};
}

/////////////////////////////////////////////////
// Execution::Collection::QueueImpl::Base

// FUNCTION public
//	Collection::QueueImpl::Base::initialize -- 
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
QueueImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::QueueImpl::Base::terminate -- 
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
QueueImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	clearData();
}

// FUNCTION public
//	Collection::QueueImpl::Base::clear -- 
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
QueueImpl::Base::
clear()
{
	clearData();
}

// FUNCTION public
//	Collection::QueueImpl::Base::isEmpty -- 
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
QueueImpl::Base::
isEmpty()
{
	return m_vecStorage.ISEMPTY();
}

// FUNCTION public
//	Collection::QueueImpl::Base::serialize -- 
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
QueueImpl::Base::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	archiver_(m_iMaxSize);
}

// FUNCTION private
//	Collection::QueueImpl::Base::addData -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
QueueImpl::Base::
addData(const Common::Data* pData_)
{
	if (pData_->getType() == Common::DataType::Array
		&& pData_->getElementType() == Common::DataType::Data) {
		Common::Data::Pointer pCopy = pData_->copy();
		Common::DataArrayData* pData = _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pCopy.get());
		m_vecStorage.PUSHBACK(pCopy);
		m_vecData.PUSHBACK(pData);

	} else {
		// single data -> create dataarray data
		AUTOPOINTER<Common::DataArrayData> pData = new Common::DataArrayData;
		pData->pushBack(pData_->copy());
		m_vecStorage.PUSHBACK(Common::Data::Pointer(pData.release()));
		m_vecData.PUSHBACK(pData.get());
	}
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "Collection(queue) "
			<< " putData = "
			<< Opt::Trace::toString(*(m_vecData.GETBACK()))
			<< ModEndl;
	}
#endif
	// until storage size reaches max, return false
	return m_iMaxSize == -1 ? true : m_vecStorage.GETSIZE() >= m_iMaxSize;
}

// FUNCTION private
//	Collection::QueueImpl::Base::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Common::Data* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
QueueImpl::Base::
getData(Common::Data* pData_)
{
	if (m_vecData.GETSIZE()) {
		if (pData_ == 0) { // 読み捨てる
			m_vecData.POPFRONT();
			m_vecStorage.POPFRONT();
			return true;
		} else if (pData_->getType() != Common::DataType::Array
				   || pData_->getElementType() != Common::DataType::Data) {
			_SYDNEY_THROW0(Exception::NotSupported);
		}

		// take first element
		Utility::DataType::assignElements(_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pData_),
										  m_vecData.getFront());
		m_vecData.POPFRONT();
		m_vecStorage.POPFRONT();

#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			_OPT_EXECUTION_MESSAGE
				<< "Collection(queue) "
				<< " getData = "
				<< Opt::Trace::toString(*pData_)
				<< ModEndl;
		}
#endif
		return true;
	}
	return false;
}

// FUNCTION private
//	Collection::QueueImpl::Base::clearData -- 
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
QueueImpl::Base::
clearData()
{
	m_vecData.clear();
	m_vecStorage.clear();
}

///////////////////////////////////////////////////////////
// Execution::Collection::QueueImpl::Normal::PutImpl

// FUNCTION public
//	Collection::QueueImpl::Normal::PutImpl::finish -- 
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
QueueImpl::Normal::PutImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::QueueImpl::Normal::PutImpl::terminate -- 
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
QueueImpl::Normal::PutImpl::
terminate(Interface::IProgram& cProgram_)
{
	m_pOuter = 0;
}

// FUNCTION public
//	Collection::QueueImpl::Normal::PutImpl::putData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
QueueImpl::Normal::PutImpl::
putData(Interface::IProgram& cProgram_,
		const Common::Data* pData_)
{
	return m_pOuter->addData(pData_);
}

// FUNCTION public
//	Collection::QueueImpl::Normal::PutImpl::put -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Externalizable* pObject_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
QueueImpl::Normal::PutImpl::
put(Interface::IProgram& cProgram_,
	const Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

///////////////////////////////////////////////////////////
// Execution::Collection::QueueImpl::Normal::GetImpl

// FUNCTION public
//	Collection::QueueImpl::Normal::GetImpl::finish -- 
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
QueueImpl::Normal::GetImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::QueueImpl::Normal::GetImpl::terminate -- 
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
QueueImpl::Normal::GetImpl::
terminate(Interface::IProgram& cProgram_)
{
	m_pOuter = 0;
}

// FUNCTION public
//	Collection::QueueImpl::Normal::GetImpl::getData -- 
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
QueueImpl::Normal::GetImpl::
getData(Interface::IProgram& cProgram_,
		Common::Data* pData_)
{
	return m_pOuter->getData(pData_);
}

// FUNCTION public
//	Collection::QueueImpl::Normal::GetImpl::get -- 
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
QueueImpl::Normal::GetImpl::
get(Interface::IProgram& cProgram_,
	Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::QueueImpl::Normal::GetImpl::reset -- 
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
QueueImpl::Normal::GetImpl::
reset()
{
	m_pOuter->clearData();
}

///////////////////////////////////////////
// Execution::Collection::QueueImpl::Normal

// FUNCTION public
//	Collection::QueueImpl::Normal::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
QueueImpl::Normal::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("queue");
}

// FUNCTION public
//	Collection::QueueImpl::Normal::initialize -- 
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
QueueImpl::Normal::
initialize(Interface::IProgram& cProgram_)
{
	Super::initialize(cProgram_);
	m_cPut.setOuter(this);
	m_cGet.setOuter(this);
}

// FUNCTION public
//	Collection::QueueImpl::Normal::terminate -- 
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
QueueImpl::Normal::
terminate(Interface::IProgram& cProgram_)
{
	m_cPut.terminate(cProgram_);
	m_cGet.terminate(cProgram_);
	Super::terminate(cProgram_);
}

// FUNCTION public
//	Collection::QueueImpl::Normal::getClassID -- 
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
QueueImpl::Normal::
getClassID() const
{
	return Class::getClassID(Class::Category::Queue);
}

/////////////////////////////////////////////////////
// Execution::Collection::QueueImpl::Safe::PutImpl

// FUNCTION public
//	Collection::QueueImpl::Safe::PutImpl::finish -- 
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
QueueImpl::Safe::PutImpl::
finish(Interface::IProgram& cProgram_)
{
	Os::AutoCriticalSection l(m_pOuter->m_cLatch);
	m_pOuter->m_iFinished++; // do nothing
	if (m_pOuter->m_iFinished + m_pOuter->m_iTerminated + 1 == m_pOuter->m_iInitialized) {
		m_pOuter->m_cWriteEvent.set();
	}
}

// FUNCTION public
//	Collection::QueueImpl::Safe::PutImpl::terminate -- 
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
QueueImpl::Safe::PutImpl::
terminate(Interface::IProgram& cProgram_)
{
	m_pOuter = 0;
}

// FUNCTION public
//	Collection::QueueImpl::Safe::PutImpl::putData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
QueueImpl::Safe::PutImpl::
putData(Interface::IProgram& cProgram_,
		const Common::Data* pData_)
{
	Os::AutoTryCriticalSection l(m_pOuter->m_cLatch);
retry:
	l.lock();
	if (m_pOuter->isLast()) _SYDNEY_THROW0(Exception::Cancel);
	if (m_pOuter->isFull()) {
		// queue is full
		// wait for reading
		l.unlock();
		m_pOuter->m_cReadEvent.wait(100);
		goto retry;
	}
	m_pOuter->addData(pData_);
	l.unlock();
	m_pOuter->m_cWriteEvent.set();
	return false;
}

// FUNCTION public
//	Collection::QueueImpl::Safe::PutImpl::put -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Externalizable* pObject_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
QueueImpl::Safe::PutImpl::
put(Interface::IProgram& cProgram_,
	const Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

///////////////////////////////////////////////////////////
// Execution::Collection::QueueImpl::Safe::GetImpl

// FUNCTION public
//	Collection::QueueImpl::Safe::GetImpl::finish -- 
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
QueueImpl::Safe::GetImpl::
finish(Interface::IProgram& cProgram_)
{
	Os::AutoCriticalSection l(m_pOuter->m_cLatch);
	m_pOuter->setWasLast();
	while(m_pOuter->getData(0));
	m_pOuter->m_iFinished = 0;
	m_pOuter->m_cReadEvent.set();
}

// FUNCTION public
//	Collection::QueueImpl::Safe::GetImpl::terminate -- 
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
QueueImpl::Safe::GetImpl::
terminate(Interface::IProgram& cProgram_)
{
	m_pOuter = 0;
}

// FUNCTION public
//	Collection::QueueImpl::Safe::GetImpl::getData -- 
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
QueueImpl::Safe::GetImpl::
getData(Interface::IProgram& cProgram_,
		Common::Data* pData_)
{
	Os::AutoTryCriticalSection l(m_pOuter->m_cLatch);
 retry:
	l.lock();
	if (m_pOuter->getData(pData_) == false) {
		if (m_pOuter->m_iFinished + m_pOuter->m_iTerminated  == 0
			|| m_pOuter->m_iInitialized >
			m_pOuter->m_iFinished + m_pOuter->m_iTerminated + 1) {
			// any one might put data
			l.unlock();
			m_pOuter->m_cWriteEvent.wait(100);
			goto retry;
		}
		return false;
	}
	m_pOuter->m_cReadEvent.set();
	return true;
}

// FUNCTION public
//	Collection::QueueImpl::Safe::GetImpl::get -- 
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
QueueImpl::Safe::GetImpl::
get(Interface::IProgram& cProgram_,
	Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::QueueImpl::Safe::GetImpl::reset -- 
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
QueueImpl::Safe::GetImpl::
reset()
{
	Os::AutoCriticalSection l(m_pOuter->m_cLatch);
	m_pOuter->m_bLast = false;
}

///////////////////////////////////////////
// Execution::Collection::QueueImpl::Safe

// FUNCTION public
//	Collection::QueueImpl::Safe::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
QueueImpl::Safe::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("safe queue");
}

// FUNCTION public
//	Collection::QueueImpl::Safe::initialize -- 
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
QueueImpl::Safe::
initialize(Interface::IProgram& cProgram_)
{
	Os::AutoCriticalSection l(m_cLatch);
	if (m_iInitialized++ == 0) {
		Super::initialize(cProgram_);
		m_cPut.setOuter(this);
		m_cGet.setOuter(this);
	}
}

// FUNCTION public
//	Collection::QueueImpl::Safe::terminate -- 
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
QueueImpl::Safe::
terminate(Interface::IProgram& cProgram_)
{
	Os::AutoCriticalSection l(m_cLatch);
	if (!isLast()) setWasLast();
	if (++m_iTerminated == m_iInitialized) {
		m_cPut.terminate(cProgram_);
		m_cGet.terminate(cProgram_);
		Super::terminate(cProgram_);
	}
}

// FUNCTION public
//	Collection::QueueImpl::Safe::getClassID -- 
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
QueueImpl::Safe::
getClassID() const
{
	return Class::getClassID(Class::Category::QueueSafe);
}

///////////////////////////////////////////
// Execution::Collection::Queue::Safe

// FUNCTION public
//	Collection::Queue::Safe::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	SIZE iMaxSize_
//	
// RETURN
//	Queue*
//
// EXCEPTIONS

//static
Queue*
Queue::Safe::
create(Interface::IProgram& cProgram_,
	   SIZE iMaxSize_)
{
	AUTOPOINTER<This> pResult = new QueueImpl::Safe(iMaxSize_);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

///////////////////////////////////////////
// Execution::Collection::Queue

// FUNCTION public
//	Collection::Queue::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	SIZE iMaxSize_
//	
// RETURN
//	Queue*
//
// EXCEPTIONS

//static
Queue*
Queue::
create(Interface::IProgram& cProgram_,
	   SIZE iMaxSize_)
{
	AUTOPOINTER<This> pResult = new QueueImpl::Normal(iMaxSize_);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Collection::Queue::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Queue*
//
// EXCEPTIONS

//static
Queue*
Queue::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::Queue:
		{
			return new QueueImpl::Normal;
		}
	case Class::Category::QueueSafe:
		{
			return new QueueImpl::Safe;
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
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
