// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ResultSet.h -- ResultSet wrapper for distribution
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

#ifndef __SYDNEY_DSERVER_RESULTSET_H
#define __SYDNEY_DSERVER_RESULTSET_H

#include "DServer/Module.h"
#include "DServer/Declaration.h"
#include "DServer/Session.h"

#include "Client2/ResultSet.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN

namespace Client2
{
	class Session;
}

_SYDNEY_DSERVER_BEGIN

// CLASS
//	DServer::ResultSet --
//
// NOTES

class SYD_DSERVER_FUNCTION ResultSet : public Common::Object
{
public:
	typedef Client2::ResultSet::Status Status;

	// constructor
	static ResultSet* create(Session* pSession_,
							 Client2::ResultSet* pResultSet_);
	// destructor
	static void erase(ResultSet* pResultSet_);

	// get next tuple
	Status::Value getNextTuple(Common::DataArrayData* pTuple_);
	// read until final status is read
	Status::Value getStatus();
	// close resultset
	void close();
	// cancel execution
	void cancel();

protected:
private:
	friend class ModAutoPointer<ResultSet>;

	ResultSet() : m_pSession(0), m_pResultSet(0), m_iID(-1) {}
	ResultSet(Session* pSession_,
			  Client2::ResultSet* pResultSet_)
		: m_pSession(pSession_),
		  m_pResultSet(pResultSet_),
		  m_iID(-1)
	{}
	~ResultSet() {destruct();}

	void destruct();

	Session* m_pSession;
	Client2::ResultSet* m_pResultSet;
	int m_iID;
};

_SYDNEY_DSERVER_END
_SYDNEY_END

#endif // __SYDNEY_DSERVER_RESULTSET_H

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
