// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/ServerAccess.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "DExecution::Action";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "DExecution/Action/Class.h"
#include "DExecution/Action/ServerAccess.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/StringData.h"

#include "DServer/Cascade.h"
#include "DServer/Manager.h"
#include "DServer/PrepareStatement.h"
#include "DServer/ResultSet.h"
#include "DServer/Session.h"

#include "Exception/Unexpected.h"
#include "Exception/CascadeNotFound.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"

#include "Schema/Cascade.h"
#include "Schema/Database.h"
#include "Server/Session.h"


_SYDNEY_BEGIN
_SYDNEY_DEXECUTION_BEGIN
_SYDNEY_DEXECUTION_ACTION_BEGIN


///////////////////////////////////////
// DExecution::Action::StaticServerAccess
class StaticServerAccess
	: public ServerAccess
{
public:
	StaticServerAccess(Schema::Cascade* pSchemaCascade_,
					   const STRING& cstrSQL_) 
		:ServerAccess(pSchemaCascade_),
		 m_cstrSQL(cstrSQL_),
		 m_cParam()
	{}

	StaticServerAccess() 
		:ServerAccess(),
		 m_cstrSQL(),
		 m_cParam()
	{}

	virtual ~ StaticServerAccess()
	{};

///////////////////////////////
// Common::Externalizable
	int getClassID() const
	{return Class::getClassID(Class::Category::StaticServerAccess);}

///////////////////////////////
// ModSerializer
	void serialize(ModArchive& archiver_)
	{
		ServerAccess::serialize(archiver_);
		archiver_(m_cstrSQL);
	}

protected:
	virtual const STRING& getSQL() {return m_cstrSQL;}

	virtual void clearSQL() {return m_cstrSQL.clear();}
	
	virtual const Common::DataArrayData& getParameter() {return m_cParam;}

	virtual void explainThis(Opt::Environment* pEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_)
	{
		cExplain_.put(m_cstrSQL);
	}
	
private:
	STRING m_cstrSQL;
	Common::DataArrayData m_cParam;
};


///////////////////////////////////////
// DExecution::Action::DynamicServerAccess
class DynamicServerAccess
	: public ServerAccess
{
public:
	DynamicServerAccess(Schema::Cascade* pSchemaCascade_,
						int iDataID_)
		:ServerAccess(pSchemaCascade_),
		 m_cSqlData(iDataID_)
	{}

	DynamicServerAccess() 
		:ServerAccess(),
		 m_cSqlData()
	{}

	virtual ~ DynamicServerAccess()
	{};	

	
	void initialize(Execution::Interface::IProgram& cProgram_)
	{
		ServerAccess::initialize(cProgram_);
		if (!m_cSqlData.isInitialized()) m_cSqlData.initialize(cProgram_);
	}

	void terminate(Execution::Interface::IProgram& cProgram_)
	{
		m_cSqlData.terminate(cProgram_);
	}
	
    ///////////////////////////////
    // Common::Externalizable
	int getClassID() const
	{return Class::getClassID(Class::Category::DynamicServerAccess);}

    ///////////////////////////////
    // ModSerializer
	void serialize(ModArchive& archiver_)
	{
		ServerAccess::serialize(archiver_);
		m_cSqlData.serialize(archiver_);
	}

protected:
	virtual const STRING& getSQL()
	{
		;_SYDNEY_ASSERT(m_cSqlData->getCount() == 2);
		const Common::Data::Pointer pSql = m_cSqlData->getElement(0);
		;_SYDNEY_ASSERT(pSql->getType() == Common::DataType::String);
		return _SYDNEY_DYNAMIC_CAST(const Common::StringData*, pSql.get())->getValue();
	}

	virtual void clearSQL()
	{
		const Common::Data::Pointer pSql = m_cSqlData->getElement(0);
		;_SYDNEY_ASSERT(pSql->getType() == Common::DataType::String);
		_SYDNEY_DYNAMIC_CAST(Common::StringData*, pSql.get())->setValue("", 0);
	}
	
	virtual const Common::DataArrayData& getParameter()
	{
		;_SYDNEY_ASSERT(m_cSqlData->getCount() == 2);
		const Common::Data::Pointer pParam = m_cSqlData->getElement(1);
		;_SYDNEY_ASSERT(pParam->getType() == Common::DataType::Array);
		return *(_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pParam.get()));
	}
	

	virtual void explainThis(Opt::Environment* pEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_)
	{
		cExplain_.put("Dynamic SQL data by ");
		m_cSqlData.explain(cProgram_, cExplain_);
	}
	
private:
	Execution::Action::ArrayDataHolder m_cSqlData;
};

	
// FUNCTION public
//	Action::ServerAccess::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Schema::Cascade* pSchemaCascade_
//	const STRING& cstrSQL_
//	
// RETURN
//	ServerAccess*
//
// EXCEPTIONS

//static
ServerAccess*
ServerAccess::
create(Execution::Interface::IProgram& cProgram_,
	   Schema::Cascade* pSchemaCascade_,
	   const STRING& cstrSQL_)
{
	AUTOPOINTER<This> pServerAccess = new StaticServerAccess(pSchemaCascade_,
															 cstrSQL_);
	pServerAccess->registerToProgram(cProgram_);
	return pServerAccess.release();
}


// FUNCTION public
//	Action::ServerAccess::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Schema::Cascade* pSchemaCascade_
//	int iDataID_
//	
// RETURN
//	ServerAccess*
//
// EXCEPTIONS

//static
ServerAccess*
ServerAccess::
create(Execution::Interface::IProgram& cProgram_,
	   Schema::Cascade* pSchemaCascade_,
	   int iDataID_)
{
	AUTOPOINTER<This> pServerAccess = new DynamicServerAccess(pSchemaCascade_,
															  iDataID_);

	pServerAccess->registerToProgram(cProgram_);
	return pServerAccess.release();
}

// FUNCTION public
//	Action::ServerAccess::executeQuery -- execute query
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	DServer::ResultSet*
//
// EXCEPTIONS

DServer::ResultSet*
ServerAccess::
executeQuery(Execution::Interface::IProgram& cProgram_)
{
	if (m_pSession) {
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			_OPT_EXECUTION_MESSAGE
				<< "Server [" << getCascadeName()
				<< "] execute: "
				<< Opt::Trace::toString(Common::StringData(getSQL()))
				<< ModEndl;
		}
#endif
		DServer::ResultSet* pResult =
			m_pSession->executeStatement(getSQL(), getParameter());

		return pResult;
	}
	return 0;
}

// FUNCTION public
//	Action::ServerAccess::executeQuery -- execute query
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	DServer::ResultSet*
//
// EXCEPTIONS

DServer::ResultSet*
ServerAccess::
getResultSet(Execution::Interface::IProgram& cProgram_)
{
	if (m_pResultSet == 0) execute(cProgram_);
	_SYDNEY_ASSERT(m_pResultSet);
	return m_pResultSet;
}

// FUNCTION public
//	Action::ServerAccess::open -- open
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

void
ServerAccess::
open(Execution::Interface::IProgram& cProgram_)
{
	if (m_pSession == 0) {
		Common::LargeVector<DServer::Session*> vecDSession = 
			Server::Session::getSession(cProgram_.getTransaction()->getSessionID())->getCascadeSession();
		Common::LargeVector<DServer::Session*>::ConstIterator ite =vecDSession.begin();
		Common::LargeVector<DServer::Session*>::ConstIterator end =vecDSession.end();
		
		for (; ite != end; ite++ ) {
			if (m_cCascadeName == (*ite)->getCascadeName()) {
				m_pSession = (*ite);
				break;
			}
		}
		if (m_pSession == 0)
			_SYDNEY_THROW2(Exception::CascadeNotFound, m_cCascadeName, cProgram_.getDatabase()->getName());
	}
}

// FUNCTION public
//	Action::ServerAccess::prepare -- prepare
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
ServerAccess::
prepare(Execution::Interface::IProgram& cProgram_)
{
	if (m_pSession) {
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			_OPT_EXECUTION_MESSAGE
				<< "Server [" << getCascadeName()
				<< "] prepare: "
				<< Opt::Trace::toString(Common::StringData(getSQL()))
				<< ModEndl;
		}
#endif
		m_pPrepare = m_pSession->prepare(getSQL());
	}
	return m_pPrepare != 0;
}

// FUNCTION public
//	Action::ServerAccess::executePrepare -- execute prepared
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	const Common::DataArrayData& cData_
//	
// RETURN
//	DServer::ResultSet*
//
// EXCEPTIONS

DServer::ResultSet*
ServerAccess::
executePrepare(Execution::Interface::IProgram& cProgram_,
			   const Common::DataArrayData& cData_)
{
	return m_pPrepare->execute(cData_);
}

// FUNCTION public
//	Action::ServerAccess::explain -- explain serveraccess
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ServerAccess::
explain(Opt::Environment* pEnvironment_,
		Execution::Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();

	Schema::Cascade* pSchemaCascade = cProgram_.getDatabase()->getCascade(m_cCascadeName,
																		  *cProgram_.getTransaction());	
	cExplain_.put(m_cCascadeName);
	cExplain_.put("(").put(pSchemaCascade->getTarget().m_cstrHost);
	cExplain_.put(":").put(pSchemaCascade->getTarget().m_cstrPort).put(")");
	cExplain_.popNoNewLine();

	cExplain_.pushIndent();
	cExplain_.newLine();
	cExplain_.put("by [");
	explainThis(pEnvironment_, cProgram_, cExplain_);
	cExplain_.put("]");
	cExplain_.popIndent();
}

// FUNCTION public
//	Action::ServerAccess::initialize -- 
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
ServerAccess::
initialize(Execution::Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Action::ServerAccess::terminate -- 
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
ServerAccess::
terminate(Execution::Interface::IProgram& cProgram_)
{
	if (m_pSession) {
		close(cProgram_);
		m_pSession = 0;
	}
}

// FUNCTION public
//	Action::ServerAccess::execute -- 
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
ServerAccess::
execute(Execution::Interface::IProgram& cProgram_)
{
	try {
		open(cProgram_);
		m_pResultSet = executeQuery(cProgram_);
	} catch (...) {
		SydInfoMessage << "Exceptiion from " << getCascadeName() << ModEndl;
		_SYDNEY_RETHROW;
	}
}



// FUNCTION public
//	Action::ServerAccess::execute -- 
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
ServerAccess::
execute(Execution::Interface::IProgram& cProgram_,
		Execution::Action::ActionList& cActionList_)
{
	if (m_pResultSet == 0) execute(cProgram_);
	return Execution::Action::Status::Success;
}


// FUNCTION public
//	Action::ServerAccess::finish -- 
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
ServerAccess::
finish(Execution::Interface::IProgram& cProgram_)
{
	
}


// FUNCTION public
//	Action::ServerAccess::reset -- 
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
ServerAccess::
reset(Execution::Interface::IProgram& cProgram_)
{

}




// FUNCTION public
//	Action::ServerAccess::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	ServerAccess*
//
// EXCEPTIONS

//static
ServerAccess*
ServerAccess::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::StaticServerAccess:
	{
		return new StaticServerAccess;
	}
	case Class::Category::DynamicServerAccess:
	{
		return new DynamicServerAccess;
	}
	default:
	{
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	}
}


// FUNCTION public
//	Action::ServerAccess::serialize -- 
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
ServerAccess::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	archiver_(m_cCascadeName);
}

// FUNCTION private
//	Action::ServerAccess::close -- close
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

void
ServerAccess::
close(Execution::Interface::IProgram& cProgram_)
{
	if (m_pPrepare) {
		DServer::PrepareStatement::erase(m_pPrepare);
		m_pPrepare = 0;
	}
}

// FUNCTION private
//	Action::ServerAccess::registerToProgram -- register to program
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

void
ServerAccess::
registerToProgram(Execution::Interface::IProgram& cProgram_)
{
	Super::registerToProgram(cProgram_, 0);
}

//////////////////////////////////////////
// DExecution::Action::ServerAccessHolder

// FUNCTION public
//	Action::ServerAccessHolder::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ServerAccessHolder::
explain(Opt::Environment* pEnvironment_,
		Execution::Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	if (m_pServerAccess) {
		m_pServerAccess->explain(pEnvironment_, cProgram_, cExplain_);
	} else {
		cProgram_.getAction(m_iID)->explain(pEnvironment_, cProgram_, cExplain_);
	}
}

// FUNCTION public
//	Action::ServerAccessHolder::initialize -- initialize ServerAccess instance
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

void
ServerAccessHolder::
initialize(Execution::Interface::IProgram& cProgram_)
{
	if (m_pServerAccess == 0) {
		m_pServerAccess = _SYDNEY_DYNAMIC_CAST(ServerAccess*, cProgram_.getAction(m_iID));
		if (m_pServerAccess == 0) {
			_SYDNEY_THROW0(Exception::Unexpected);
		}
		m_pServerAccess->initialize(cProgram_);
	}
}

// FUNCTION public
//	Action::ServerAccessHolder::terminate -- terminate ServerAccess instance
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

void
ServerAccessHolder::
terminate(Execution::Interface::IProgram& cProgram_)
{
	if (m_pServerAccess) {
		m_pServerAccess->terminate(cProgram_);
		m_pServerAccess = 0;
	}
}

// FUNCTION public
//	Action::ServerAccessHolder::serialize -- serializer
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
ServerAccessHolder::
serialize(ModArchive& archiver_)
{
	archiver_(m_iID);
}

_SYDNEY_DEXECUTION_ACTION_END
_SYDNEY_DEXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
