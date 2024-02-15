// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataSource.cpp -- DataSource wrapper for distribution
// 
// Copyright (c) 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "DServer/DataSource.h"
#include "DServer/Session.h"

#include "Client2/DataSource.h"

#include "Exception/Unexpected.h"

#include "ModAutoPointer.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_DSERVER_USING

// FUNCTION public
//	DServer::DataSource::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrHost_
//	const ModUnicodeString& cstrPort_
//	const ModUnicodeString& cstrDatabaseName_
//	
// RETURN
//	DataSource*
//
// EXCEPTIONS

//static
DataSource*
DataSource::
create(const ModUnicodeString& cstrHost_,
	   const ModUnicodeString& cstrPort_,
	   const ModUnicodeString& cstrDatabaseName_)
{
	ModAutoPointer<DataSource> pResult = new DataSource;
	int iPort = ModUnicodeCharTrait::toInt(cstrPort_);

	ModAutoPointer<Client2::DataSource> pDataSource = new Client2::DataSource(cstrHost_, iPort);
	pDataSource->open();
	pResult->m_pDataSource = pDataSource.release();

	pResult->m_cstrDatabaseName = cstrDatabaseName_;

	return pResult.release();
}

// FUNCTION public
//	DServer::DataSource::erase -- destructor
//
// NOTES
//
// ARGUMENTS
//	DataSource* pDataSource_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
DataSource::
erase(DataSource* pDataSource_)
{
	if (pDataSource_) {
		if (pDataSource_->m_pDataSource) {
			pDataSource_->m_pDataSource->close();
			pDataSource_->m_pDataSource->release();
			pDataSource_->m_pDataSource = 0;
		}
		delete pDataSource_;
	}
}

// FUNCTION public
//	DServer::DataSource::createSession -- begin session
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrDatabaseName_
//	
// RETURN
//	Session*
//
// EXCEPTIONS

Session*
DataSource::
createSession(const ModUnicodeString& cstrDatabaseName_,
			  const ModUnicodeString& cstrCascadeName_)
{
	if (m_pDataSource == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return Session::create(this,
						   m_cstrDatabaseName.getLength() ? m_cstrDatabaseName : cstrDatabaseName_,
						   cstrCascadeName_);
}

// FUNCTION public
//	DServer::DataSource::createSession -- begin session
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrDatabaseName_
//	const ModUnicodeString& cstrUserName_
//	const ModUnicodeString& cstrPassword_
//	
// RETURN
//	Session*
//
// EXCEPTIONS

Session*
DataSource::
createSession(const ModUnicodeString& cstrDatabaseName_,
			  const ModUnicodeString& cstrCascadeName_,
			  const ModUnicodeString& cstrUserName_,
			  const ModUnicodeString& cstrPassword_)
{
	if (m_pDataSource == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return Session::create(this,
						   m_cstrDatabaseName.getLength() ? m_cstrDatabaseName : cstrDatabaseName_,
						   cstrCascadeName_,
						   cstrUserName_,
						   cstrPassword_);
}

// FUNCTION private
//	DServer::DataSource::destruct -- 
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
DataSource::
destruct()
{
	try {
		if (m_pDataSource) {
			m_pDataSource->close();
			m_pDataSource->release(), m_pDataSource = 0;
		}
	} catch (...) {
		// ignore
		;
	}
}

//
// Copyright (c) 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
