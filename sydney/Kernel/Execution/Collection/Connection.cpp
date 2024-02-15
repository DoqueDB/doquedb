// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Collection/Connection.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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
#include "SyReinterpretCast.h"

#include "Execution/Collection/Connection.h"
#include "Execution/Collection/Class.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/ClassID.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/Externalizable.h"

#include "Communication/Connection.h"

#include "Exception/ConnectionRanOut.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Declaration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"

#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_COLLECTION_BEGIN

namespace Impl
{
	// CLASS local
	//	Execution::Collection::Impl::ConnectionImpl -- implementation class for collection::connection
	//
	// NOTES

	class ConnectionImpl
		: public Collection::Connection
	{
	public:
		typedef Collection::Connection Super;
		typedef ConnectionImpl This;

		// constructor
		ConnectionImpl()
			: Super(),
			  m_iID(-1),
			  m_pConnection(0),
			  m_bOpenByMe(false),
			  m_cPut()
		{}
		ConnectionImpl(int iConnectionID_)
			: Super(),
			  m_iID(iConnectionID_),
			  m_pConnection(0),
			  m_bOpenByMe(false),
			  m_cPut()
		{}

		// destructor
		~ConnectionImpl()
		{try{close();}catch(...){}}

		// CLASS
		//	ConnectionImpl::PutImpl -- implementation of put interface
		//
		// NOTES
		class PutImpl
			: public Super::Put
		{
		public:
			PutImpl() : m_pOuter(0), m_pConnection(0) ,m_bFinished(false) {}
			~PutImpl() {}

			void setOuter(ConnectionImpl* pOuter_)
			{
				m_pOuter = pOuter_;
				m_pConnection = m_pOuter->m_pConnection;
			}

		/////////////////////////////
		// Super::Put
			virtual void finish(Interface::IProgram& cProgram_);
			virtual void terminate(Interface::IProgram& cProgram_);
			virtual bool putData(Interface::IProgram& cProgram_,
								 const Common::Data* pData_);
			virtual bool put(Interface::IProgram& cProgram_,
							 const Common::Externalizable* pObject_);
			virtual void flush();
		protected:
		private:
			ConnectionImpl* m_pOuter;
			Communication::Connection* m_pConnection;
			bool m_bFinished;
		};

	////////////////////////
	// ICollection::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual void clear();
		virtual bool isEmpty();
	//	virtual bool isEmptyGrouping();

		virtual Put* getPutInterface() {return &m_cPut;}
		virtual Get* getGetInterface() {_SYDNEY_THROW0(Exception::NotSupported);}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		friend class PutImpl;

		void open();
		void close();

		int m_iID;
		Communication::Connection* m_pConnection;
		bool m_bOpenByMe;
		PutImpl m_cPut;
		
	};
}

///////////////////////////////////////////////////////////
// Execution::Collection::Impl::ConnectionImpl::PutImpl

// FUNCTION public
//	Collection::Impl::ConnectionImpl::PutImpl::finish -- 
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
Impl::ConnectionImpl::PutImpl::
finish(Interface::IProgram& cProgram_)
{
	if (!m_bFinished) {
		put(cProgram_, 0);
		m_bFinished = true;
	}
}

// FUNCTION public
//	Collection::Impl::ConnectionImpl::PutImpl::terminate -- 
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
Impl::ConnectionImpl::PutImpl::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::ConnectionImpl::PutImpl::putData -- 
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
Impl::ConnectionImpl::PutImpl::
putData(Interface::IProgram& cProgram_,
		const Common::Data* pData_)
{
	bool bResult = put(cProgram_, pData_);
#ifndef NO_TRACE
	if (pData_ && _OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "Collection(connection)"
			<< " putData = "
			<< Opt::Trace::toString(*pData_)
			<< ModEndl;
	}
#endif
	return bResult;
}

// FUNCTION public
//	Collection::Impl::ConnectionImpl::PutImpl::put -- 
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
Impl::ConnectionImpl::PutImpl::
put(Interface::IProgram& cProgram_,
	const Common::Externalizable* pObject_)
{
	m_pOuter->open();
	m_pConnection->writeObject(pObject_);
#ifndef NO_TRACE
	if (pObject_ == 0
		&& _OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "Collection(connection)"
			<< " put end-of-data" << ModEndl;
	}
#endif
	
	if (cProgram_.getTransaction())
		// rows sent counter
		cProgram_.getTransaction()->addSendRowCount();
	
	return true;
}

// FUNCTION public
//	Collection::Impl::ConnectionImpl::PutImpl::flush -- 
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
Impl::ConnectionImpl::PutImpl::
flush()
{
	m_pConnection->flush();
}

/////////////////////////////////////////////////
// Execution::Collection::Impl::ConnectionImpl

// FUNCTION public
//	Collection::Impl::ConnectionImpl::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ConnectionImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("<connection:#").put(m_iID).put(">");
}

// FUNCTION public
//	Collection::Impl::ConnectionImpl::initialize -- 
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
Impl::ConnectionImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_pConnection == 0) {
		m_pConnection = cProgram_.getConnection(m_iID);
		m_cPut.setOuter(this);
	}
}

// FUNCTION public
//	Collection::Impl::ConnectionImpl::terminate -- 
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
Impl::ConnectionImpl::
terminate(Interface::IProgram& cProgram_)
{
	close();
}

// FUNCTION public
//	Collection::Impl::ConnectionImpl::clear -- 
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
Impl::ConnectionImpl::
clear()
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::ConnectionImpl::isEmpty -- 
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
Impl::ConnectionImpl::
isEmpty()
{
	// regard as always not empty
	return false;
}

// FUNCTION public
//	Collection::Impl::ConnectionImpl::getClassID -- 
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
Impl::ConnectionImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Connection);
}

// FUNCTION public
//	Collection::Impl::ConnectionImpl::serialize -- 
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
Impl::ConnectionImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	archiver_(m_iID);
}

// FUNCTION private
//	Collection::Impl::ConnectionImpl::open -- 
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
Impl::ConnectionImpl::
open()
{
	if (!m_pConnection->isOpened()) {
		// open here
		m_pConnection->open();
		m_bOpenByMe = true;
	}
}

// FUNCTION private
//	Collection::Impl::ConnectionImpl::close -- 
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
Impl::ConnectionImpl::
close()
{
	if (m_pConnection && m_bOpenByMe) {
		m_pConnection->close();
		m_pConnection = 0;
		m_bOpenByMe = false;
	}
}

///////////////////////////////////////////
// Execution::Collection::Connection

// FUNCTION public
//	Collection::Connection::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iConnectionID_
//	
// RETURN
//	Connection*
//
// EXCEPTIONS

//static
Connection*
Connection::
create(Interface::IProgram& cProgram_,
	   int iConnectionID_)
{
	AUTOPOINTER<This> pResult = new Impl::ConnectionImpl(iConnectionID_);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Collection::Connection::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Connection*
//
// EXCEPTIONS

//static
Connection*
Connection::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::Connection);
	return new Impl::ConnectionImpl;
}

_SYDNEY_EXECUTION_COLLECTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
