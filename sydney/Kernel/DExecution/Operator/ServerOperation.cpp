// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/ServerOperation.cpp --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DExecution::Operator";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "DExecution/Operator/Class.h"
#include "DExecution/Operator/ServerOperation.h"

#include "DExecution/Action/ServerAccess.h"

#include "DServer/ResultSet.h"

#include "Common/Message.h"

#include "Exception/Unexpected.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"

#include "Opt/Explain.h"

#include "Schema/Cascade.h"

_SYDNEY_BEGIN
_SYDNEY_DEXECUTION_BEGIN
_SYDNEY_DEXECUTION_OPERATOR_BEGIN

namespace
{
	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName = "server operation";
}

namespace Impl
{
	// CLASS local
	//	DExecution::Operator::Impl::ServerOperationImpl -- implementation class
	//
	// NOTES
	class ServerOperationImpl
		: public Operator::ServerOperation
	{
	public:
		typedef ServerOperationImpl This;
		typedef Operator::ServerOperation Super;

		// constructor
		ServerOperationImpl()
			: Super(),
			  m_cAccess(),
			  m_cData(),
			  m_bPrepared(false)
		{}
		ServerOperationImpl(int iServerAccessID_,
							int iDataID_)
			: Super(),
			  m_cAccess(iServerAccessID_),
			  m_cData(iDataID_),
			  m_bPrepared(false)
		{}

		// destructor
		virtual ~ServerOperationImpl() {}

	////////////////////////////////
	// Operator::ServerOperation::

	///////////////////////////////////
	// Execution::Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Execution::Interface::IProgram& cProgram_);
		virtual void terminate(Execution::Interface::IProgram& cProgram_);
		virtual Execution::Action::Status::Value
					execute(Execution::Interface::IProgram& cProgram_,
							Execution::Action::ActionList& cActionList_);
		virtual void finish(Execution::Interface::IProgram& cProgram_);
		virtual void reset(Execution::Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		DExecution::Action::ServerAccessHolder m_cAccess;
		Execution::Action::ArrayDataHolder m_cData;

		bool m_bPrepared;
	};
}

///////////////////////////////////////////////////
// DExecution::Operator::Impl::ServerOperationImpl

// FUNCTION public
//	Operator::Impl::ServerOperationImpl::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ServerOperationImpl::
explain(Opt::Environment* pEnvironment_,
		Execution::Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();

	cExplain_.put(_pszOperatorName).put(" ");
	m_cAccess.explain(pEnvironment_, cProgram_, cExplain_);
	cExplain_.popNoNewLine();
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.pushIndent();
		cExplain_.newLine();
		cExplain_.put("using ");
		m_cData.explain(cProgram_, cExplain_);
		cExplain_.popIndent();
	}
}

// FUNCTION public
//	Operator::Impl::ServerOperationImpl::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ServerOperationImpl::
initialize(Execution::Interface::IProgram& cProgram_)
{
	// convert ID to object
	if (m_cData.isInitialized() == false) {
		m_cAccess.initialize(cProgram_);
		m_cData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Operator::Impl::ServerOperationImpl::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ServerOperationImpl::
terminate(Execution::Interface::IProgram& cProgram_)
{
	m_cAccess.terminate(cProgram_);
	m_cData.terminate(cProgram_);
}

// FUNCTION public
//	Operator::Impl::ServerOperationImpl::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Execution::Action::ActionList& cActionList_
//
// RETURN
//	Execution::Action::Status::Value
//
// EXCEPTIONS

//virtual
Execution::Action::Status::Value
Impl::ServerOperationImpl::
execute(Execution::Interface::IProgram& cProgram_,
		Execution::Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		bool bHasData = false;

		if (m_bPrepared == false) {
			try {
				m_cAccess->open(cProgram_);
				m_bPrepared = m_cAccess->prepare(cProgram_);
			} catch (...) {
				SydInfoMessage << "Exception from " << m_cAccess->getCascadeName() << ModEndl;
				_SYDNEY_RETHROW;
			}
			if (m_bPrepared == false) {
				_SYDNEY_THROW0(Exception::Unexpected);
			}
		}
		DServer::ResultSet* pResultSet =
			m_cAccess->executePrepare(cProgram_,
									  *m_cData);
		try {
			DServer::ResultSet::Status::Value eStatus =
				pResultSet->getStatus();

		} catch (...) {
			DServer::ResultSet::erase(pResultSet);
			SydInfoMessage << "Exception from " << m_cAccess->getCascadeName() << ModEndl;
			_SYDNEY_RETHROW;
		}
		DServer::ResultSet::erase(pResultSet);

		done();
	}
	return Execution::Action::Status::Success;
}


// FUNCTION public
//	Operator::Impl::ServerOperationImpl::finish -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ServerOperationImpl::
finish(Execution::Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::Impl::ServerOperationImpl::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ServerOperationImpl::
reset(Execution::Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::Impl::ServerOperationImpl::getClassID -- 
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
Impl::ServerOperationImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::ServerOperation);
}

// FUNCTION public
//	Operator::Impl::ServerOperationImpl::serialize -- 
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
Impl::ServerOperationImpl::
serialize(ModArchive& archiver_)
{
	m_cAccess.serialize(archiver_);
	m_cData.serialize(archiver_);
}

/////////////////////////////////////
// Operator::ServerOperation::

// FUNCTION public
//	Operator::ServerOperation::create -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Schema::Cascade* pSchemaCascade_
//	const STRING& cstrSQL_
//	int iDataID_
//	
// RETURN
//	ServerOperation*
//
// EXCEPTIONS

//static
ServerOperation*
ServerOperation::
create(Execution::Interface::IProgram& cProgram_,
	   Execution::Interface::IIterator* pIterator_,
	   Schema::Cascade* pSchemaCascade_,
	   const STRING& cstrSQL_,
	   int iDataID_)
{
	Action::ServerAccess* pAccess =
		Action::ServerAccess::create(cProgram_,
									 pSchemaCascade_,
									 cstrSQL_);

	AUTOPOINTER<This> pResult =
		new Impl::ServerOperationImpl(pAccess->getID(),
									  iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::ServerOperation::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	ServerOperation*
//
// EXCEPTIONS

//static
ServerOperation*
ServerOperation::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::ServerOperation:
		{
			return new Impl::ServerOperationImpl;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

_SYDNEY_DEXECUTION_OPERATOR_END
_SYDNEY_DEXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
