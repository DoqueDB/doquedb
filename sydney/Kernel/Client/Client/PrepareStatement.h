// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PrepareStatement.h --
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

#ifndef __TRMEISTER_CLIENT_PREPARESTATEMENT_H
#define __TRMEISTER_CLIENT_PREPARESTATEMENT_H

#include "Client/Module.h"
#include "Client/Object.h"

#include "ModUnicodeString.h"

_TRMEISTER_BEGIN
_TRMEISTER_CLIENT_BEGIN

class DataSource;

//
//	CLASS
//	Client::PrepareStatement --
//
//	NOTES
//
//
class SYD_FUNCTION PrepareStatement : public Client::Object
{
public:
	//コンストラクタ
	PrepareStatement(DataSource& cDataSource_,
						const ModUnicodeString& cstrDatabaseName_, int iPrepareID_);
	//デストラクタ
	virtual ~PrepareStatement();

	//最適化結果を削除する
	void close();

	//データベース名を取り出す
	const ModUnicodeString& getDatabaseName() const
	{
		return m_cstrDatabaseName;
	}

	//最適化IDを取り出す
	int getPrepareID() const
	{
		return m_iPrepareID;
	}

private:
	//データソース
	DataSource& m_cDataSource;
	//データベース名
	ModUnicodeString m_cstrDatabaseName;
	//最適化ID
	int m_iPrepareID;
};

_TRMEISTER_CLIENT_END
_TRMEISTER_END

#endif //__TRMEISTER_CLIENT_PREPARESTATEMENT_H

//
//	Copyright (c) 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
