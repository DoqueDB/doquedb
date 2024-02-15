// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Table.cpp -- Implementation of classes concerning with table reorganization
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

#include "DSchema/Table.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "DServer/Cascade.h"
#include "DServer/OpenMPExecutor.h"
#include "DServer/Session.h"

#include "Opt/Algorithm.h"

#include "Schema/Database.h"
#include "Schema/Table.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_BEGIN
_SYDNEY_DSCHEMA_BEGIN

namespace
{
	class _CountChecker
	{
	public:
		_CountChecker(Trans::Transaction& cTrans_,
					  const Schema::Table& cTable_)
			: m_cTrans(cTrans_),
			  m_cTable(cTable_),
			  m_vecSession(),
			  m_vecStatus()
		{}
		~_CountChecker();

		// check whether table is empty
		bool isEmpty();

	protected:
	private:
		// start session
		void open();
		// end session 
		void close();

		Trans::Transaction& m_cTrans;
		const Schema::Table& m_cTable;

		Common::LargeVector<DServer::Session*> m_vecSession;
		Common::LargeVector<DServer::OpenMPExecutor::ExecuteStatus::Value> m_vecStatus;

		bool m_bResult;
	};

	class _IsEmptyExecutor
		: public DServer::OpenMPExecutor
	{
	public:
		_IsEmptyExecutor(const Common::LargeVector<DServer::Session*>& vecSession_,
						 Common::LargeVector<ExecuteStatus::Value>& vecStatus_,
						 const ModUnicodeString& cstrSQL_)
			: DServer::OpenMPExecutor(vecSession_, vecStatus_, cstrSQL_),
			  m_bResult(true)
		{}
		~_IsEmptyExecutor() {}

		bool getResult();

	//////////////////////
	// Utility::OpenMP::
	//	virtual void prepare();
		virtual void parallel();
	protected:
	private:
		bool m_bResult;
	};
}

////////////////////////////////
// $$$::_CountChecker::

// FUNCTION public
//	$$$::_CountChecker::~CountChecker -- 
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

_CountChecker::
~_CountChecker()
{
	try {close();} catch (...) {}
}

// FUNCTION public
//	$$$::_CountChecker::isEmpty -- check whether table is empty
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

bool
_CountChecker::
isEmpty()
{
	open();

	ModUnicodeOstrStream stream;
	stream << "select count(*) from " << m_cTable.getName();

	return _IsEmptyExecutor(m_vecSession, m_vecStatus, stream.getString()).getResult();
}

// FUNCTION private
//	$$$::_CountChecker::open -- start session
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
_CountChecker::
open()
{
	Schema::Database* pDatabase = m_cTable.getDatabase(m_cTrans);
	; _SYDNEY_ASSERT(pDatabase);
	m_vecSession = DServer::Cascade::getSession(*pDatabase, m_cTrans);
}

// FUNCTION private
//	$$$::_CountChecker::close -- end session 
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
_CountChecker::
close()
{
	DServer::Cascade::eraseSession(m_vecSession);
	m_vecSession.clear();
	m_vecStatus.clear();
}

////////////////////////////////
// $$$::_IsEmptyExecutor::

bool
_IsEmptyExecutor::
getResult()
{
	run();
	return m_bResult;
}

// FUNCTION public
//	$$$::_IsEmptyExecutor::parallel -- 
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
_IsEmptyExecutor::
parallel()
{
	int iIndex = -1;
	while ((iIndex = getWorkingSession()) >= 0) {
		if (isException()) break;
		if (m_bResult == false) break;

		Common::DataArrayData cTuple;
		Common::UnsignedIntegerData cCount;
		cTuple.pushBack(Common::Data::Pointer(reinterpret_cast<const Common::Data*>(&cCount)));

		DServer::ResultSet* pResultSet = getResultSet()[iIndex];
		if (pResultSet == 0) continue;

		try {
			DServer::ResultSet::Status::Value eStatus;
			do {
				if (isException()) {
					pResultSet->cancel();
					break;
				}
				eStatus = pResultSet->getNextTuple(&cTuple);

				if (eStatus == DServer::ResultSet::Status::Data
					&& cCount.getValue() > 0) {
					m_bResult = false;
				}

			} while (eStatus == DServer::ResultSet::Status::MetaData
					 || eStatus == DServer::ResultSet::Status::Data
					 || eStatus == DServer::ResultSet::Status::EndOfData
					 || eStatus == DServer::ResultSet::Status::HasMoreData);
		} catch (...) {
			SydInfoMessage << "Exception from cascade:#" << iIndex << ModEndl;
			_SYDNEY_RETHROW;
		}
	}
}

////////////////////////////////
// DSchema::Table::

// FUNCTION public
//	DSchema::Table::isEmpty -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::Table& cTable_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Table::
isEmpty(Trans::Transaction& cTrans_,
		const Schema::Table& cTable_)
{
	return _CountChecker(cTrans_, cTable_).isEmpty();
}

_SYDNEY_DSCHEMA_END
_SYDNEY_END

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
