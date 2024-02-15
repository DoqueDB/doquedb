// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Option.h --
// 
// Copyright (c) 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_TRBACKUP_OPTION_H
#define __TRMEISTER_TRBACKUP_OPTION_H

#include "Common/Common.h"
#include "ModUnicodeString.h"

//
//	CLASS
//	Sqli::Option --
//
//	NOTES
//
//
class Option 
{
public:
	//コンストラクタ
	Option();
	//デストラクタ
	virtual ~Option();

	//set
	bool set(int argc, char* argv[]);

	//サーバ名を得る
	const ModUnicodeString& getHostName() const
	{
		return m_cstrHostName;
	}
	//ポート番号を得る
	int getPortNumber() const
	{
		return m_iPortNumber;
	}

	//データベース名を得る
	const ModUnicodeString& getDatabaseName() const
	{
		return m_cstrDatabaseName;
	}

	//get user name
	const ModUnicodeString& getUserName() const
	{
		return m_cstrUserName;
	}

	//get password
	const ModUnicodeString& getPassword() const
	{
		return m_cstrPassword;
	}
	
private:
	//サーバ名
	ModUnicodeString m_cstrHostName;
	//ポート番号
	int m_iPortNumber;
	//データベース名
	ModUnicodeString m_cstrDatabaseName;
	//user name
	ModUnicodeString m_cstrUserName;
	//password
	ModUnicodeString m_cstrPassword;
};

#endif //__TRMEISTER_TRBACKUP_OPTION_H

//
//	Copyright (c) 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
