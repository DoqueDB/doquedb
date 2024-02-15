// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenMPExecutor.h -- Declaration of class executing SQL with OpenMP
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

#ifndef	__SYDNEY_DSERVER_OPENMPEXECUTOR_H
#define	__SYDNEY_DSERVER_OPENMPEXECUTOR_H

#include "DServer/Module.h"

#include "Common/LargeVector.h"

#include "Os/CriticalSection.h"

#include "Utility/OpenMP.h"

_SYDNEY_BEGIN
_SYDNEY_DSERVER_BEGIN

class ResultSet;
class Session;

/////////////////////////////////
// CLASS
//	DServer::OpenMPExecutor --
//
// NOTES
class OpenMPExecutor
	: public Utility::OpenMP
{
public:
	struct ExecuteStatus
	{
		enum Value
		{
			None = 0,
			Started,
			Succeeded,
			Failed,
			Cancelling,
			ValueNum
		};
	};

	OpenMPExecutor(const Common::LargeVector<Session*>& vecSession_,
				   Common::LargeVector<ExecuteStatus::Value>& vecStatus_,
				   const ModUnicodeString& cstrSQL_,
				   bool bForce_ = false)
		: m_vecSession(vecSession_),
		  m_vecStatus(vecStatus_),
		  m_cstrSQL(cstrSQL_),
		  m_vecResultSet(),
		  m_iIndex(-1),
		  m_cLatch(),
		  m_bForce(bForce_)
	{}
	virtual ~OpenMPExecutor();

//////////////////////
// Utility::OpenMP::
	virtual void prepare();
	virtual void parallel();
	virtual void dispose();

protected:
	virtual bool isAbort();
	int getWorkingSession();
	
	const Common::LargeVector<DServer::Session*>& getSession()
	{return m_vecSession;}
	Common::LargeVector<ExecuteStatus::Value>& getStatus()
	{return m_vecStatus;}
	const ModUnicodeString& getSQL()
	{return m_cstrSQL;}

	Common::LargeVector<DServer::ResultSet*>& getResultSet()
	{return m_vecResultSet;}

private:

	int m_iIndex;
	Os::CriticalSection m_cLatch;

	const Common::LargeVector<DServer::Session*>& m_vecSession;
	Common::LargeVector<ExecuteStatus::Value>& m_vecStatus;
	const ModUnicodeString& m_cstrSQL;

	Common::LargeVector<DServer::ResultSet*> m_vecResultSet;

	bool m_bForce;
};

_SYDNEY_DSERVER_END
_SYDNEY_END

#endif	// __SYDNEY_DSERVER_OPENMPEXECUTOR_H

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
