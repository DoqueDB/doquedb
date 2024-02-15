// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ResultSet.cpp -- ResultSet wrapper for distribution
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
#include "DServer/ResultSet.h"
#include "DServer/Session.h"

#include "Client2/DataSource.h"
#include "Client2/ResultSet.h"
#include "Client2/Session.h"

#include "Exception/Unexpected.h"

#include "Execution/Utility/DataType.h"

#include "ModAutoPointer.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_DSERVER_USING

// FUNCTION public
//	DServer::ResultSet::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Session* pSession_
//	Client2::ResultSet* pResultSet_
//	
// RETURN
//	ResultSet*
//
// EXCEPTIONS

//static
ResultSet*
ResultSet::
create(Session* pSession_,
	   Client2::ResultSet* pResultSet_)
{
	ModAutoPointer<ResultSet> pResult = new ResultSet(pSession_,
													  pResultSet_);
	pResult->m_iID = pSession_->addResultSet(pResult.get());

	return pResult.release();
}

// FUNCTION public
//	DServer::ResultSet::erase -- destructor
//
// NOTES
//
// ARGUMENTS
//	ResultSet* pResultSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
ResultSet::
erase(ResultSet* pResultSet_)
{
	if (pResultSet_) {
		pResultSet_->m_pSession->eraseResultSet(pResultSet_->m_iID);
		delete pResultSet_;
	}
}

// FUNCTION public
//	DServer::ResultSet::getNextTuple -- get next tuple
//
// NOTES
//
// ARGUMENTS
//	Common::DataArrayData* pTuple_
//	
// RETURN
//	ResultSet::Status::Value
//
// EXCEPTIONS

ResultSet::Status::Value
ResultSet::
getNextTuple(Common::DataArrayData* pTuple_)
{
	Status::Value eResult;
	if (pTuple_ == 0) {
		eResult = m_pResultSet->getNextTuple(0);
	} else if (pTuple_->getCount() == 0) {
		eResult = m_pResultSet->getNextTuple(pTuple_);
	} else {
		Common::DataArrayData cData;
		eResult = m_pResultSet->getNextTuple(&cData);
		if (eResult == Status::Data) {
			Execution::Utility::DataType::assignElements(pTuple_, &cData);
		}
	}
	return eResult;
}

// FUNCTION public
//	DServer::ResultSet::getStatus -- read until final status is read
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ResultSet::Status::Value
//
// EXCEPTIONS

ResultSet::Status::Value
ResultSet::
getStatus()
{
	return m_pResultSet->getStatus(true);
}

// FUNCTION public
//	DServer::ResultSet::close -- close resultset
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
ResultSet::
close()
{
	if (m_pResultSet) {
		m_pResultSet->close();
	}
}

// FUNCTION public
//	DServer::ResultSet::cancel -- cancel execution
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
ResultSet::
cancel()
{
	if (m_pResultSet) {
		m_pResultSet->cancel();
	}
}

// FUNCTION private
//	DServer::ResultSet::destruct -- 
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
ResultSet::
destruct()
{
	try {
		if (m_pResultSet) {
			m_pResultSet->close();
			delete m_pResultSet, m_pResultSet = 0;
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
