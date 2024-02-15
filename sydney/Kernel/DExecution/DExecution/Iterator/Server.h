// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DExecution/Iterator/Server.h --
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

#ifndef __SYDNEY_DEXECUTION_ITERATOR_SERVER_H
#define __SYDNEY_DEXECUTION_ITERATOR_SERVER_H

#include "DExecution/Iterator/Module.h"
#include "Execution/Iterator/Base.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Cascade;
}

namespace Plan
{
	namespace Sql
	{
		class Query;		
	}
}
	
	

_SYDNEY_DEXECUTION_BEGIN
_SYDNEY_DEXECUTION_ITERATOR_BEGIN

////////////////////////////////////
//	CLASS
//	DExecution::Iterator::Server -- Iterator class for server access
//
//	NOTES
//		This class is not constructed directly
class Server
	: public Execution::Iterator::Base
{
public:
	typedef Execution::Iterator::Base Super;
	typedef Server This;

	struct Query
	{
		// constructor
		static This* create(Execution::Interface::IProgram& cProgram_,
							Schema::Cascade* pSchemaCascade_,
							const STRING& cstrSQL_);
		
		// 動的にsql文を生成する場合のQuery
		static This* create(Execution::Interface::IProgram& cProgram_,
							Schema::Cascade* pSchemaCascade_,
							int iInputDataID_);

		// Server::AccessをIDにセットする場合
		static This* create(Execution::Interface::IProgram& cProgram_,
							int iAccessID__);
	};

	struct PreparedQuery
	{
		// constructor
		static This* create(Execution::Interface::IProgram& cProgram_,
							Schema::Cascade* pSchemaCascade_,
							const STRING& cstrSQL_,
							int iDataID_);
	}; 
	struct Execute
	{
		// constructor
		static This* create(Execution::Interface::IProgram& cProgram_,
							Schema::Cascade* pSchemaCascade_,
							const STRING& cstrSQL_);
	};
	// destructor
	virtual ~Server() {}

	// for serialize
	static This* getInstance(int iCategory_);

protected:
	// constructor
	Server() : Super() {}

private:
};

_SYDNEY_DEXECUTION_ITERATOR_END
_SYDNEY_DEXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_DEXECUTION_ITERATOR_SERVER_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
