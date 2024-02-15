// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Option.h --
// 
// Copyright (c) 2002, 2007, 2009, 2011, 2016, 2023 Ricoh Company, Ltd.
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
#include "Client2/DataSource.h"
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
	//置換文字列
	typedef ModVector<ModPair<ModUnicodeString, ModUnicodeString> >	Vector;

	//実行対象
	struct Target
	{
		enum Value
		{
			Stdin,
			File,
			Line
		};
	};

	//ロケーション
	struct Location
	{
		enum Value
		{
			InProcess,
			Remote,
			Install,
			Unknown
		};
	};

	//コンストラクタ
	Option();
	//デストラクタ
	virtual ~Option();

	//set
	bool set(int argc, char* argv[]);

	//ターゲットを得る
	Target::Value getTarget() const
	{
		return m_eTarget;
	}

	//ロケーションを得る
	Location::Value getLocation() const
	{
		return m_eLocation;
	}

	//ファイル名を得る
	const ModCharString& getFileName() const
	{
		return m_cstrFileName;
	}

	//コマンドラインのSQL文を得る
	const ModUnicodeString& getCommandLine() const
	{
		return m_cstrCommandLine;
	}

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

	//is password specified by command line option?
	bool isPasswordSpecified() const
	{
		return m_bPasswordSpecified;
	}

	//shutdownか
	bool isShutdown() const
	{
		return m_bShutdown;
	}

	//versionか
	bool isVersion() const
	{
		return m_bVersion;
	}

	//置換文字列を得る
	const Vector& getReplaceString() const
	{
		return m_vecReplaceString;
	}

	//レジストリの親パスを得る
	const ModUnicodeString& getParentRegPath() const
	{
		// [NOTE] 2007/07/25よりレジストリのパスを指すようになった
		//  参照 Common::SystemParameter::setParentPath
		return m_cstrParentRegPath;
	}

	//暗号モードを得る(暗号化対応)
	const TRMeister::Communication::CryptMode::Value getCryptMode() const
	{
		return m_eCryptMode;
	}

	// プロトコルバージョンを得る
	const TRMeister::Client2::DataSource::Protocol::Value getProtocol() const
	{
		return m_iProtocol;
	}

	// ユーザーとパスワードが指定されているか
	bool isUserPasswordSpecified() const
	{
		return m_cstrUserName.getLength() > 0 && m_cstrPassword.getLength() > 0;
	}

	// アドレスファミリー指定を得る
	TRMeister::Client2::DataSource::Family::Value getFamily() const
	{
		return m_eFamily;
	}

	// ユーザー名を入力してもらう
	bool inputUserName() const;
	// パスワードを入力してもらう
	bool inputPassword() const;

	// ユーザー名とパスワードの指定をクリアする
	void clearUserPassword() const;

private:
	//ターゲット
	Target::Value m_eTarget;
	//ロケーション
	Location::Value m_eLocation;
	//ファイル名
	ModCharString m_cstrFileName;
	//コマンドライン
	ModUnicodeString m_cstrCommandLine;
	//サーバ名
	ModUnicodeString m_cstrHostName;
	//ポート番号
	int m_iPortNumber;
	//データベース名
	ModUnicodeString m_cstrDatabaseName;
	//レジストリの親パス
	// [NOTE] 2007/07/25よりレジストリのパスを指すようになった
	//  参照 Common::SystemParameter::setParentPath
	ModUnicodeString m_cstrParentRegPath;
	//shutdown?
	bool m_bShutdown;
	//version?
	bool m_bVersion;
	//置換文字列
	Vector m_vecReplaceString;
	//文字コード
	ModCharString m_cstrCode;

	// 暗号モード(暗号化対応)
	TRMeister::Communication::CryptMode::Value m_eCryptMode;
	// プロトコル
	TRMeister::Client2::DataSource::Protocol::Value m_iProtocol;

	//user name
	ModUnicodeString m_cstrUserName;
	//password
	ModUnicodeString m_cstrPassword;
	bool m_bPasswordSpecified;

	// アドレスファミリー
	TRMeister::Client2::DataSource::Family::Value m_eFamily;
};

#endif //__TRMEISTER_SQLI_OPTION_H

//
//	Copyright (c) 2002, 2007, 2009, 2011, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
