// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeDistribute.cpp -- Base class for reorganization
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
const char moduleName[] = "DSchema";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "DSchema/ReorganizeDistribute.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "DServer/Cascade.h"
#include "DServer/OpenMPExecutor.h"
#include "DServer/Session.h"

#include "Opt/Algorithm.h"

#include "Schema/Database.h"

_SYDNEY_BEGIN
_SYDNEY_DSCHEMA_BEGIN

namespace Manager
{
namespace SystemTable
{

namespace
{
	//////////////////////////////////////////////////////////
	// CLASS local
	//	_CancelExecutor -- OpenMPExecutor for cancellation
	//
	// NOTES
	class _CancelExecutor
		: public DServer::OpenMPExecutor
	{
	public:
		typedef _CancelExecutor This;
		typedef DServer::OpenMPExecutor Super;

		_CancelExecutor(const Common::LargeVector<DServer::Session*>& vecSession_,
					   Common::LargeVector<ExecuteStatus::Value>& vecStatus_,
					   const ModUnicodeString& cstrSQL_)
			: Super(vecSession_, vecStatus_, cstrSQL_)
		{}
		~_CancelExecutor() {}

	//////////////////////
	// Utility::OpenMP::
		virtual void prepare();
		virtual void parallel();

protected:
private:
};

}

/////////////////////////////////
// $$$::_CancelExecutor::
/////////////////////////////////

// FUNCTION public
//	$$$::_CancelExecutor::prepare -- 
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
_CancelExecutor::
prepare()
{
	unsigned int n = getSession().getSize();
	; _SYDNEY_ASSERT(n == getStatus().getSize());

	getResultSet().reserve(n);
	for (unsigned int i = 0; i < n; ++i) {
		switch (getStatus()[i]) {
		case DServer::OpenMPExecutor::ExecuteStatus::None:
		case DServer::OpenMPExecutor::ExecuteStatus::Started:
		case DServer::OpenMPExecutor::ExecuteStatus::Failed:
		default:
			{
				// do nothing
				getResultSet().pushBack(0);
				break;
			}
		case DServer::OpenMPExecutor::ExecuteStatus::Succeeded:
			{
				// cancel execution
				DServer::Session* pSession = getSession()[i];
				getResultSet().pushBack(pSession->executeStatement(getSQL()));
				break;
			}
		}
	}
}

// FUNCTION public
//	$$$::_CancelExecutor::parallel -- 
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
_CancelExecutor::
parallel()
{
	int iIndex = -1;
	while ((iIndex = getWorkingSession()) >= 0) {
		DServer::ResultSet* pResultSet = getResultSet()[iIndex];
		if (pResultSet) {
			try {
				(void)pResultSet->getStatus();
			} catch (...) {
				SydInfoMessage << "Exception from cascade:#" << iIndex << ModEndl;
				_SYDNEY_RETHROW;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////
// DSchema::Manager::SystemTable::ReorganizeDistribute::
////////////////////////////////////////////////////////////////////

// FUNCTION protected
//	DSchema::Manager::SystemTable::ReorganizeDistribute::open -- start session
//
// NOTES
//
// ARGUMENTS
//	Schema::Database* pDatabase_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ReorganizeDistribute::
open(Schema::Database* pDatabase_,
	 Trans::Transaction& cTrans_)
{
	setSession(pDatabase_,
			   cTrans_);
}

// FUNCTION protected
//	DSchema::Manager::SystemTable::ReorganizeDistribute::close -- end session
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
ReorganizeDistribute::
close()
{
	clearSession();
	clearStatus();
}

// FUNCTION protected
//	DSchema::Manager::SystemTable::ReorganizeDistribute::executeStatement -- execute statement
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrSQL_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ReorganizeDistribute::
executeStatement(const ModUnicodeString& cstrSQL_)
{
	DServer::OpenMPExecutor(m_vecSession, m_vecStatus, cstrSQL_).run();
}

// FUNCTION protected
//	DSchema::Manager::SystemTable::ReorganizeDistribute::cancelStatement -- calcel statement
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrSQL_
//	Schema::Database* pDatabase_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ReorganizeDistribute::
cancelStatement(const ModUnicodeString& cstrSQL_,
				Schema::Database* pDatabase_,
				Trans::Transaction& cTrans_)
{
	if (m_vecStatus.isEmpty()) {
		// setSession has failed -> do nothing
		return;
	}

	// reopen connection
	clearSession();
	setSession(pDatabase_, cTrans_);

	_CancelExecutor(m_vecSession, m_vecStatus, cstrSQL_).run();
}

// FUNCTION private
//	DSchema::Manager::SystemTable::ReorganizeDistribute::setSession -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::Database* pDatabase_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ReorganizeDistribute::
setSession(Schema::Database* pDatabase_,
		   Trans::Transaction& cTrans_)
{
	m_vecSession =
		DServer::Cascade::getSession(*pDatabase_,
									 cTrans_);
}

// FUNCTION private
//	DSchema::Manager::SystemTable::ReorganizeDistribute::clearSession -- 
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
ReorganizeDistribute::
clearSession()
{
	DServer::Cascade::eraseSession(m_vecSession);
	m_vecSession.clear();
}

// FUNCTION private
//	DSchema::Manager::SystemTable::ReorganizeDistribute::clearStatus -- 
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
ReorganizeDistribute::
clearStatus()
{
	m_vecStatus.clear();
}

} // namespace SystemTable
} // namespace Manager
_SYDNEY_DSCHEMA_END
_SYDNEY_END

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
