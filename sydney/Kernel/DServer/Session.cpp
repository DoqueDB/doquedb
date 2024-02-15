// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Session.cpp -- Session wrapper for distribution
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
	const char moduleName[] = "DServer";
	const char srcFile[] = __FILE__;
}

#include "boost/foreach.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "DServer/DataSource.h"
#include "DServer/PrepareStatement.h"
#include "DServer/Session.h"

#include "Client2/DataSource.h"
#include "Client2/Session.h"

#include "Common/DataArrayData.h"

#include "Exception/Unexpected.h"

#include "ModAutoPointer.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_DSERVER_USING

// FUNCTION public
//	DServer::Session::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	DataSource* pDataSource_
//	const ModUnicodeString& cstrDatabaseName_
//	
// RETURN
//	Session*
//
// EXCEPTIONS

//static
Session*
Session::
create(DataSource* pDataSource_,
	   const ModUnicodeString& cstrDatabaseName_,
	   const ModUnicodeString& cstrCascadeName_)
{
	ModAutoPointer<Session> pResult = new Session(cstrCascadeName_);

	pResult->m_pSession = pDataSource_->m_pDataSource->createSession(cstrDatabaseName_);

	return pResult.release();
}

// FUNCTION public
//	DServer::Session::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	DataSource* pDataSource_
//	const ModUnicodeString& cstrDatabaseName_
//	
// RETURN
//	Session*
//
// EXCEPTIONS

//static
Session*
Session::
create(DataSource* pDataSource_,
	   const ModUnicodeString& cstrDatabaseName_,
	   const ModUnicodeString& cstrCascadeName_,
	   const ModUnicodeString& cstrUserName_,
	   const ModUnicodeString& cstrPassword_)
{
	ModAutoPointer<Session> pResult = new Session(cstrCascadeName_);

	pResult->m_pSession
		= pDataSource_->m_pDataSource->createSession(cstrDatabaseName_,
													 cstrUserName_,
													 cstrPassword_);

	return pResult.release();
}

// FUNCTION public
//	DServer::Session::erase -- destructor
//
// NOTES
//
// ARGUMENTS
//	Session* pSession_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Session::
erase(Session* pSession_)
{
	if (pSession_) {
		if (pSession_->m_pSession) {
			pSession_->m_pSession->close();
			delete pSession_->m_pSession;
			pSession_->m_pSession = 0;
		}
		delete pSession_;
	}
}

// FUNCTION public
//	DServer::Session::executeStatement -- execute sql statement
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrSQL_
//	
// RETURN
//	ResultSet*
//
// EXCEPTIONS

ResultSet*
Session::
executeStatement(const ModUnicodeString& cstrSQL_)
{
	return ResultSet::create(this,
							 m_pSession->executeStatement(cstrSQL_));
}

// FUNCTION public
//	DServer::Session::executeStatement -- 
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrSQL_
//	const Common::DataArrayData& cParameter_
//	
// RETURN
//	ResultSet*
//
// EXCEPTIONS

ResultSet*
Session::
executeStatement(const ModUnicodeString& cstrSQL_,
				 const Common::DataArrayData& cParameter_)
{
	return ResultSet::create(this,
							 m_pSession->executeStatement(cstrSQL_,
														  cParameter_));
}

// FUNCTION public
//	DServer::Session::prepare -- 
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrSQL_
//	
// RETURN
//	PrepareStatement*
//
// EXCEPTIONS

PrepareStatement*
Session::
prepare(const ModUnicodeString& cstrSQL_)
{
	return PrepareStatement::create(this,
									m_pSession->createPrepareStatement(cstrSQL_));
}

// FUNCTION private
//	DServer::Session::destruct -- 
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
Session::
destruct()
{
	try {
		if (m_pSession) {
			closeAllResultSet();
			m_pSession->close();
			delete m_pSession, m_pSession = 0;
		}
	} catch (...) {
		// ignore
		;
	}
}

// FUNCTION private
//	DServer::Session::addResultSet -- 
//
// NOTES
//
// ARGUMENTS
//	ResultSet* pResultSet_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
Session::
addResultSet(ResultSet* pResultSet_)
{
	m_vecResultSet.pushBack(pResultSet_);
	return m_vecResultSet.getSize() - 1;
}

// FUNCTION private
//	DServer::Session::eraseResultSet -- 
//
// NOTES
//
// ARGUMENTS
//	int iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Session::
eraseResultSet(int iID_)
{
	if (iID_ < m_vecResultSet.getSize()) {
		ResultSet* pResultSet = m_vecResultSet[iID_];
		if (pResultSet) {
			m_vecResultSet[iID_] = 0;
		}
	}
}

// FUNCTION private
//	DServer::Session::closeAllResultSet -- 
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
Session::
closeAllResultSet()
{
	Common::LargeVector<ResultSet*>::Iterator iterator = m_vecResultSet.begin();
	const Common::LargeVector<ResultSet*>::Iterator last = m_vecResultSet.end();
	for (; iterator != last; ++iterator) {
		if ((*iterator)) {
			(*iterator)->close();
			ResultSet::erase(*iterator);
		}
	}
	m_vecResultSet.clear();
}

// FUNCTION private
//	DServer::Session::addPrepareStatement -- 
//
// NOTES
//
// ARGUMENTS
//	PrepareStatement* pPrepareStatement_
//	
// RETURN
//	int
//
// EXCEPTIONS

int
Session::
addPrepareStatement(PrepareStatement* pPrepareStatement_)
{
	m_vecPrepareStatement.pushBack(pPrepareStatement_);
	return m_vecPrepareStatement.getSize() - 1;
}

// FUNCTION private
//	DServer::Session::erasePrepareStatement -- 
//
// NOTES
//
// ARGUMENTS
//	int iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Session::
erasePrepareStatement(int iID_)
{
	if (iID_ < m_vecPrepareStatement.getSize()) {
		PrepareStatement* pPrepareStatement = m_vecPrepareStatement[iID_];
		if (pPrepareStatement) {
			m_vecPrepareStatement[iID_] = 0;
		}
	}
}

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
