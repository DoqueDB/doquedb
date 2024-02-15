// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Option.h --
// 
// Copyright (c) 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_SQLI_OPTION_H
#define __TRMEISTER_SQLI_OPTION_H

#include "Common/Common.h"
#include "Communication/CryptMode.h"	// 暗号化対応
#include "ModCharString.h"
#include "ModUnicodeString.h"
#include "ModPair.h"
#include "ModVector.h"

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

	//get user name
	const ModUnicodeString& getUserName() const
	{
		return m_cstrUserName;
	}

	//is password specified by command line option?
	bool isPasswordSpecified() const
	{
		return m_bPasswordSpecified;
	}
	// ユーザーIDを得る
	int getUserID() const
	{
		return m_iUserID;
	}
	// 権限を得る
	int getRevoke() const
	{
		return m_iRevoke;
	}
	// スーパーユーザー名を得る
	const ModUnicodeString& getRootUserName() const
	{
		return m_cstrRootUserName;
	}
	// スーパーユーザーパスワードを得る
	const ModUnicodeString& getRootUserPassword() const
	{
		return m_cstrRootUserPassword;
	}

	//レジストリの親パスを得る
	const ModUnicodeString& getParentRegPath() const
	{
		return m_cstrParentRegPath;
	}

	//暗号モードを得る(暗号化対応)
	const TRMeister::Communication::CryptMode::Value getCryptMode() const
	{
		return m_eCryptMode;
	}

private:
	//サーバ名
	ModUnicodeString m_cstrHostName;
	//ポート番号
	int m_iPortNumber;
	//レジストリの親パス
	ModUnicodeString m_cstrParentRegPath;
	// 暗号モード(暗号化対応)
	TRMeister::Communication::CryptMode::Value m_eCryptMode;
	//user name
	ModUnicodeString m_cstrUserName;
	bool m_bPasswordSpecified;
	//userID
	int m_iUserID;
	// super user name
	ModUnicodeString m_cstrRootUserName;
	// super user password
	ModUnicodeString m_cstrRootUserPassword;
	//権限
	int m_iRevoke;


};

#endif //__TRMEISTER_SQLI_OPTION_H

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
