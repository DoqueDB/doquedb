// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataSource.h -- DataSource wrapper for distribution
// 
// Copyright (c) 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_DSERVER_DATASOURCE_H
#define __SYDNEY_DSERVER_DATASOURCE_H

#include "DServer/Module.h"
#include "DServer/Declaration.h"

#include "Common/Object.h"

#include "ModAutoPointer.h"

class ModUnicodeString;

_SYDNEY_BEGIN

namespace Client2
{
	class DataSource;
}

_SYDNEY_DSERVER_BEGIN

// CLASS
//	DServer::DataSource --
//
// NOTES

class SYD_DSERVER_FUNCTION DataSource : public Common::Object
{
public:
	// constructor
	static DataSource* create(const ModUnicodeString& cstrHost_,
							  const ModUnicodeString& cstrPort_,
							  const ModUnicodeString& cstrDatabaseName_);
	// destructor
	static void erase(DataSource* pDataSource_);

	// begin session
	Session* createSession(const ModUnicodeString& cstrDatabaseName_,
						   const ModUnicodeString& cstrCascadeName);
	Session* createSession(const ModUnicodeString& cstrDatabaseName_,
						   const ModUnicodeString& cstrCascadeName,
						   const ModUnicodeString& cstrUserName_,
						   const ModUnicodeString& cstrPassword_);

	// destructor
	~DataSource() {destruct();}

protected:
private:
	friend class ModAutoPointer<DataSource>;
	friend class Session;

	DataSource() : m_pDataSource(0) {}

	void destruct();

	Client2::DataSource* m_pDataSource;
	ModUnicodeString m_cstrDatabaseName;
};

_SYDNEY_DSERVER_END
_SYDNEY_END

#endif // __SYDNEY_DSERVER_DATASOURCE_H

//
// Copyright (c) 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
