// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Session.h -- セッション関連のクラス定義、関数宣言
// 
// Copyright (c) 2006, 2007, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_CLIENT2_SESSION_H
#define __TRMEISTER_CLIENT2_SESSION_H

#include "Client2/Module.h"
#include "Client2/Object.h"
#include "Os/CriticalSection.h"
#include "Common/DataArrayData.h"
#include "Communication/User.h"

#include "ModUnicodeString.h"

_TRMEISTER_BEGIN
_TRMEISTER_CLIENT2_BEGIN

class DataSource;
class ResultSet;
class PrepareStatement;

//	CLASS
//	Client2::Session -- セッションクラス
//
//	NOTES

class SYD_FUNCTION Session : public Object
{
public:
	// コンストラクタ
	Session(DataSource&				cDataSource_,
			const ModUnicodeString&	cstrDatabaseName_,
			int						iSessionID_);
	Session(DataSource&				cDataSource_,
			const ModUnicodeString&	cstrDatabaseName_,
			const ModUnicodeString&	cstrUserName_,
			int						iSessionID_);

	// デストラクタ
	virtual ~Session();

	// クローズする
	void close();

	// SQL 文を実行する
	ResultSet* executeStatement(const ModUnicodeString&			cstrStatement_,
								const Common::DataArrayData&	cParameter_ = Common::DataArrayData());

	// プリペアした SQL 文を実行する
	ResultSet* executePrepareStatement(	const PrepareStatement&			cPrepareStatement_,
										const Common::DataArrayData&	cParameter_);

	// 新しくプリペアステートメントを作成する
	PrepareStatement* createPrepareStatement(const ModUnicodeString&	cstrStatement_);

	// Create new user
	void createUser(const ModUnicodeString& cstrUserName_,
					const ModUnicodeString& cstrPassword_,
					int iUserID_ = Communication::User::ID::Auto);

	// Drop a user
	void dropUser(const ModUnicodeString& cstrUserName_,
				  int iBehavior_ = Communication::User::DropBehavior::Ignore);

	// Change session user's password
	void changeOwnPassword(const ModUnicodeString& cstrPassword_);

	// Change a user's password
	void changePassword(const ModUnicodeString& cstrUserName_,
						const ModUnicodeString& cstrPassword_);

	// プロダクトバージョンを問い合わせる
	ModUnicodeString queryProductVersion();

	// データソースオブジェクトを得る
	DataSource& getDataSource();

	// データベース名を得る
	const ModUnicodeString& getDatabaseName();

	// セッション ID を得る
	int getID();

	// プリペアステートメントを削除する
	void erasePrepareStatement(int	iID_);

	// クローズする
	int closeInternal();

	// セッションを利用不可にする
	void invalid();

	// セッションが利用可能かどうか
	bool isValid();

private:

	// データソース
	DataSource&			m_cDataSource;

	// データベース名
	ModUnicodeString	m_cstrDatabaseName;

	// ユーザー名
	ModUnicodeString	m_cstrUserName;

	// セッション ID
	int					m_iSessionID;

	// 排他制御用のクリティカルセクション
	Os::CriticalSection	m_cLatch;
};

_TRMEISTER_CLIENT2_END
_TRMEISTER_END

#endif //__TRMEISTER_CLIENT2_SESSION_H

//
//	Copyright (c) 2006, 2007, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
