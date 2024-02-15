// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenMPExecutor.cpp -- Base class for reorganization
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
const char moduleName[] = "DServer";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "DServer/OpenMPExecutor.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "DServer/Cascade.h"
#include "DServer/Session.h"

#include "Opt/Algorithm.h"

#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"

_SYDNEY_BEGIN
_SYDNEY_DSERVER_BEGIN

////////////////////////////////////////////////////////////////////
// DServer::OpenMPExecutor::
////////////////////////////////////////////////////////////////////

// FUNCTION public
//	DServer::OpenMPExecutor::~OpenMPExecutor -- 
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
OpenMPExecutor::
~OpenMPExecutor()
{
}

// FUNCTION public
//	DServer::OpenMPExecutor::prepare -- 
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
OpenMPExecutor::
prepare()
{
	unsigned int n = m_vecSession.getSize();

	// prepare status and resultset
	m_vecStatus.assign(n, DServer::OpenMPExecutor::ExecuteStatus::None);
	m_vecResultSet.reserve(n);
	; _SYDNEY_ASSERT(m_vecResultSet.isEmpty());

	// execute statement for all the cascades
	for (unsigned int i = 0; i < n; ++i) {
		m_vecStatus[i] = ExecuteStatus::Started;
		m_vecResultSet.pushBack(m_vecSession[i]->executeStatement(m_cstrSQL));
	}
}

// FUNCTION public
//	DServer::OpenMPExecutor::parallel -- 
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
OpenMPExecutor::
parallel()
{
	int iIndex = -1;
	while ((iIndex = getWorkingSession()) >= 0) {
		if (isException()) break;

		; _SYDNEY_ASSERT(static_cast<int>(m_vecStatus.getSize()) > iIndex);
		; _SYDNEY_ASSERT(static_cast<int>(m_vecResultSet.getSize()) > iIndex);

		ExecuteStatus::Value& iStatus = m_vecStatus[iIndex];

		DServer::ResultSet* pResultSet = m_vecResultSet[iIndex];
		if (pResultSet == 0) continue;

		DServer::ResultSet::Status::Value eStatus;
		
		try {
			if (isAbort()) {
				pResultSet->cancel();
				iStatus = ExecuteStatus::Cancelling;
				continue;
			}
			
			do {
				eStatus = pResultSet->getNextTuple(0);
			} while (eStatus == DServer::ResultSet::Status::MetaData
					 || eStatus == DServer::ResultSet::Status::Data
					 || eStatus == DServer::ResultSet::Status::EndOfData
					 || eStatus == DServer::ResultSet::Status::HasMoreData);
		} catch (...) {
			iStatus = ExecuteStatus::Failed;
			SydInfoMessage << "Exception from cascade:#" << iIndex << ModEndl;
			_SYDNEY_RETHROW;
		}

		iStatus = (eStatus == DServer::ResultSet::Status::Success) ?
			ExecuteStatus::Succeeded : ExecuteStatus::Failed;
	}
}

// FUNCTION public
//	DServer::OpenMPExecutor::dispose -- 
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
OpenMPExecutor::
dispose()
{
	unsigned int n = m_vecResultSet.getSize();
	for (unsigned int i = 0; i < n; ++i) {
		DServer::ResultSet*& pResultSet = m_vecResultSet[i];
		if (m_vecStatus[i] == ExecuteStatus::Started ||
			m_vecStatus[i] == ExecuteStatus::Cancelling) {
			
			DServer::ResultSet::Status::Value eStatus;
		
			try {
				if (isAbort() &&
					m_vecStatus[i] == ExecuteStatus::Started)
				{
					pResultSet->cancel();
				}
				
				eStatus = pResultSet->getStatus();
				
			} catch (...) {
				eStatus = DServer::ResultSet::Status::Error;
				// ignore exception
			}

			m_vecStatus[i] = (eStatus == DServer::ResultSet::Status::Success) ?
				ExecuteStatus::Succeeded : ExecuteStatus::Failed;
		}
		DServer::ResultSet::erase(pResultSet);
	}
	m_vecResultSet.clear();
	// m_vecStatus is not cleared because status value is used by canceler
}

// FUNCTION protected
//	DServer::OpenMPExecutor::isAbort --
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//	bool
//		中断する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS

bool
OpenMPExecutor::
isAbort()
{
	return isException() && m_bForce == false;
}

// FUNCTION protected
//	DServer::OpenMPExecutor::getWorkingSession -- 
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
OpenMPExecutor::
getWorkingSession()
{
	int iIndex = -1;
	{
		Os::AutoCriticalSection l(m_cLatch);
		iIndex = ++m_iIndex;
	}
	if (iIndex < static_cast<int>(m_vecSession.getSize())) {
		return iIndex;
	}
	return -1;
}

_SYDNEY_DSERVER_END
_SYDNEY_END

//
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
