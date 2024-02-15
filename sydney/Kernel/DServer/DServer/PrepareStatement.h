// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PrepareStatement.h -- PrepareStatement wrapper for distribution
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

#ifndef __SYDNEY_DSERVER_PREPARESTATEMENT_H
#define __SYDNEY_DSERVER_PREPARESTATEMENT_H

#include "DServer/Module.h"
#include "DServer/Declaration.h"
#include "DServer/Session.h"

#include "Client2/PrepareStatement.h"

_SYDNEY_BEGIN

namespace Client2
{
	class Session;
}

_SYDNEY_DSERVER_BEGIN

// CLASS
//	DServer::PrepareStatement --
//
// NOTES

class SYD_DSERVER_FUNCTION PrepareStatement : public Common::Object
{
public:
	// constructor
	static PrepareStatement* create(Session* pSession_,
									Client2::PrepareStatement* pPrepareStatement_);
	// destructor
	static void erase(PrepareStatement* pPrepareStatement_);

	// execute
	ResultSet* execute(const Common::DataArrayData& cParameter_);
	// close prepareStatement
	void close();

protected:
private:
	friend class ModAutoPointer<PrepareStatement>;

	PrepareStatement() : m_pSession(0), m_pPrepareStatement(0), m_iID(-1) {}
	PrepareStatement(Session* pSession_,
					 Client2::PrepareStatement* pPrepareStatement_)
		: m_pSession(pSession_),
		  m_pPrepareStatement(pPrepareStatement_),
		  m_iID(-1)
	{}
	~PrepareStatement() {destruct();}

	void destruct();

	Session* m_pSession;
	Client2::PrepareStatement* m_pPrepareStatement;
	int m_iID;
};

_SYDNEY_DSERVER_END
_SYDNEY_END

#endif // __SYDNEY_DSERVER_PREPARESTATEMENT_H

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
