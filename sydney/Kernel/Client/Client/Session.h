// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Session.h --
// 
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_CLIENT_SESSION_H
#define __TRMEISTER_CLIENT_SESSION_H

#include "Client/Module.h"
#include "Client/Object.h"
#include "Common/DataArrayData.h"

#include "ModUnicodeString.h"

_TRMEISTER_BEGIN
_TRMEISTER_CLIENT_BEGIN

class DataSource;
class ResultSet;
class PrepareStatement;

//
//	CLASS
//	Client::Session --
//
//	NOTES
//
class SYD_FUNCTION Session : public Client::Object
{
public:
	//コンストラクタ
	Session(DataSource& cDataSource_,
			const ModUnicodeString& cstrDatabaseName_, int iSessionID_);
	//デストラクタ
	virtual ~Session();

	//接続を切る
	void close();

	//SQL文を実行する
	ResultSet* executeStatement(const ModUnicodeString& cstrStatement_,
							const Common::DataArrayData& cParameter_ = Common::DataArrayData());

	//最適化されたSQL文を実行する
	ResultSet* executePrepareStatement(const PrepareStatement& cPrepareStatement_,
							const Common::DataArrayData& cParameter_);

private:
	//データソース
	DataSource& m_cDataSource;
	//データベース名
	ModUnicodeString m_cstrDatabaseName;
	//セッションID
	int m_iSessionID;
};

_TRMEISTER_CLIENT_END
_TRMEISTER_END

#endif //__TRMEISTER_CLIENT_SESSION_H

//
//	Copyright (c) 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
