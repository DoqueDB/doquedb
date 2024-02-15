// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Session.h -- Session wrapper for distribution
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

#ifndef __SYDNEY_DSERVER_SESSION_H
#define __SYDNEY_DSERVER_SESSION_H

#include "DServer/Module.h"
#include "DServer/Declaration.h"
#include "DServer/ResultSet.h"

#include "Common/LargeVector.h"
#include "Common/Object.h"

#include "ModAutoPointer.h"

class ModUnicodeString;

_SYDNEY_BEGIN

namespace Client2
{
	class Session;
}
namespace Common
{
	class DataArrayData;
}

_SYDNEY_DSERVER_BEGIN

// CLASS
//	DServer::Session --
//
// NOTES

class SYD_DSERVER_FUNCTION Session : public Common::Object
{
public:
	// constructor
	static Session* create(DataSource* pDataSource_,
						   const ModUnicodeString& cstrDatabaseName_,
						   const ModUnicodeString& cstrCascadeName);
	static Session* create(DataSource* pDataSource_,
						   const ModUnicodeString& cstrDatabaseName_,
						   const ModUnicodeString& cstrCascadeName,
						   const ModUnicodeString& cstrUserName_,
						   const ModUnicodeString& cstrPassword_);
	// destructor
	static void erase(Session* pSession_);

	// execute sql statement
	ResultSet* executeStatement(const ModUnicodeString& cstrSQL_);
	ResultSet* executeStatement(const ModUnicodeString& cstrSQL_,
								const Common::DataArrayData& cParameter_);
	PrepareStatement* prepare(const ModUnicodeString& cstrSQL_);

	ModUnicodeString& getCascadeName() {return m_cstrCascadeName;}

protected:
private:
	friend class ModAutoPointer<Session>;
	friend class ResultSet;
	friend class PrepareStatement;

	Session(const ModUnicodeString& cstrCascadeName)
		: m_pSession(0),
		m_cstrCascadeName(cstrCascadeName)
	{}
	~Session() {destruct();}

	void destruct();
	int addResultSet(ResultSet* pResultSet_);
	void eraseResultSet(int iID_);
	void closeAllResultSet();

	int addPrepareStatement(PrepareStatement* pPrepareStatement_);
	void erasePrepareStatement(int iID_);


	Client2::Session* m_pSession;
	Common::LargeVector<ResultSet*> m_vecResultSet;
	Common::LargeVector<PrepareStatement*> m_vecPrepareStatement;
	ModUnicodeString m_cstrCascadeName;
};

_SYDNEY_DSERVER_END
_SYDNEY_END

#endif // __SYDNEY_DSERVER_SESSION_H

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
