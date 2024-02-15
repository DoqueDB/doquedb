// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PrepareStatement.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Client";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Client/PrepareStatement.h"
#include "Client/DataSource.h"
#include "Client/Connection.h"

_TRMEISTER_USING
_TRMEISTER_CLIENT_USING

//
//	FUNCTION public
//	Client::PrepareStatement::PrepareStatement -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	DataSource& cDataSource_
//		データソース
//	const ModUnicodeString& cstrDatabaseName_
//		データベース名
//	int iPrepareID_
//		最適化ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
PrepareStatement::PrepareStatement(DataSource& cDataSource_,
								   const ModUnicodeString& cstrDatabaseName_, int iPrepareID_)
: Object(Type::PrepareStatement), m_cDataSource(cDataSource_), m_iPrepareID(iPrepareID_)
{
	//データベース名を代入
	m_cstrDatabaseName = cstrDatabaseName_;
//	m_cDataSource.incrementCount(Type::PrepareStatement);
}

//
//	FUNCTION public
//	Client::PrepareStatement::~PrepareStatement -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
PrepareStatement::~PrepareStatement()
{
	try
	{
		close();
	} catch (...) {}	// 例外は無視
//	m_cDataSource.decrementCount(Type::PrepareStatement);
}

//
//	FUNCTION public
//	Client::PrepareStatement::close -- 最適化結果を削除する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
PrepareStatement::close()
{
	if (m_iPrepareID)
	{
		// 最適化結果を削除する
		
		//クライアントコネクションを得る
		Connection* pClientConnection = m_cDataSource.getClientConnection();
		//最適化結果を削除する
		pClientConnection->erasePrepareStatement(m_cstrDatabaseName, m_iPrepareID);

		m_iPrepareID = 0;
	}
}

//
//	Copyright (c) 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
