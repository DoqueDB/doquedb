// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Cascade.h -- Resource holder for distributed access to sub-servers
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

#ifndef __SYDNEY_DSERVER_CASCADE_H
#define __SYDNEY_DSERVER_CASCADE_H

#include "DServer/Module.h"
#include "DServer/Declaration.h"

#include "Common/LargeVector.h"

class ModUnicodeString;

_SYDNEY_BEGIN

namespace Schema
{
	class Database;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_DSERVER_BEGIN

// CLASS
//	DServer::Cascade --
//
// NOTES
//	This class is static class

class SYD_DSERVER_FUNCTION Cascade
{
public:
	// initialize
	static void initialize();
	// terminate
	static void terminate();

	// tell cascade change
	static void resetCascade(const Schema::Database& cDatabase_);

	// create new session to access the server
	static Session* createSession(const Schema::Database& cDatabase_,
								  const ModUnicodeString& cstrServerName_,
								  Trans::Transaction& cTrans_);
	// create new sessions to access all the sub-server
	static Common::LargeVector<Session*>
					getSession(const Schema::Database& cDatabase_,
							   Trans::Transaction& cTrans_);
	static Common::LargeVector<Session*>
					getSession(const Schema::Database& cDatabase_,
							   Trans::Transaction& cTrans_,
							   const ModUnicodeString& cstrUserName_,
							   const ModUnicodeString& cstrPassword_);
	// erase all sessions created by getSession
	static void eraseSession(Common::LargeVector<Session*>& vecSession_);

protected:
private:
	// never constructed
	Cascade();
	~Cascade();
};

_SYDNEY_DSERVER_END
_SYDNEY_END

#endif // __SYDNEY_DSERVER_CASCADE_H

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
