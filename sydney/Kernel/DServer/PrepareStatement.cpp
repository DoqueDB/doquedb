// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PrepareStatement.cpp -- PrepareStatement wrapper for distribution
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

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "DServer/DataSource.h"
#include "DServer/PrepareStatement.h"
#include "DServer/Session.h"

#include "Client2/DataSource.h"
#include "Client2/PrepareStatement.h"
#include "Client2/Session.h"

#include "Exception/Unexpected.h"

#include "ModAutoPointer.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_DSERVER_USING

// FUNCTION public
//	DServer::PrepareStatement::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Session* pSession_
//	Client2::PrepareStatement* pPrepareStatement_
//	
// RETURN
//	PrepareStatement*
//
// EXCEPTIONS

//static
PrepareStatement*
PrepareStatement::
create(Session* pSession_,
	   Client2::PrepareStatement* pPrepareStatement_)
{
	ModAutoPointer<PrepareStatement> pResult = new PrepareStatement(pSession_,
																	pPrepareStatement_);

	pResult->m_iID = pSession_->addPrepareStatement(pResult.get());

	return pResult.release();
}

// FUNCTION public
//	DServer::PrepareStatement::erase -- destructor
//
// NOTES
//
// ARGUMENTS
//	PrepareStatement* pPrepareStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
PrepareStatement::
erase(PrepareStatement* pPrepareStatement_)
{
	if (pPrepareStatement_) {
		pPrepareStatement_->m_pSession->erasePrepareStatement(pPrepareStatement_->m_iID);
		delete pPrepareStatement_;
	}
}

// FUNCTION public
//	DServer::PrepareStatement::execute -- execute
//
// NOTES
//
// ARGUMENTS
//	const Common::DataArrayData& cParameter_
//	
// RETURN
//	ResultSet*
//
// EXCEPTIONS

// execute
ResultSet*
PrepareStatement::
execute(const Common::DataArrayData& cParameter_)
{
	return ResultSet::create(m_pSession,
							 m_pSession->m_pSession->executePrepareStatement(*m_pPrepareStatement,
																			 cParameter_));
}

// FUNCTION public
//	DServer::PrepareStatement::close -- close prepareStatement
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
PrepareStatement::
close()
{
	m_pSession->m_pSession->erasePrepareStatement(m_pPrepareStatement->getPrepareID());
}

// FUNCTION private
//	DServer::PrepareStatement::destruct -- 
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
PrepareStatement::
destruct()
{
	try {
		if (m_pPrepareStatement) {
			m_pPrepareStatement->close();
			delete m_pPrepareStatement, m_pPrepareStatement = 0;
		}
	} catch (...) {
		// ignore
		;
	}
}

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
