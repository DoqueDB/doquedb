// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PrepareStatement.cpp -- プリペアステートメントクラスの関数定義
// 
// Copyright (c) 2006, 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Client2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Client2/PrepareStatement.h"
#include "Client2/DataSource.h"
#include "Client2/Connection.h"
#include "Client2/Session.h"

_TRMEISTER_USING
_TRMEISTER_CLIENT2_USING

//	FUNCTION public
//	Client2::PrepareStatement::PrepareStatement -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	DataSource&				cDataSource_
//		データソース
//	const ModUnicodeString&	cstrDatabaseName_
//		データベース名
//	int						iPrepareID_
//		プリペア ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS

PrepareStatement::PrepareStatement(DataSource&				cDataSource_,
								   const ModUnicodeString&	cstrDatabaseName_,
								   int						iPrepareID_)
	: Object(Object::Type::PrepareStatement),
	  m_pDataSource(&cDataSource_),
	  m_pSession(0),
	  m_iPrepareID(iPrepareID_)
{
	// データベース名を代入
	m_cstrDatabaseName = cstrDatabaseName_;
}

PrepareStatement::PrepareStatement(Session*	pSession_,
								   int		iPrepareID_)
	: Object(Object::Type::PrepareStatement),
	  m_pDataSource(0),
	  m_pSession(pSession_),
	  m_iPrepareID(iPrepareID_)
{
}

//	FUNCTION public
//	Client2::PrepareStatement::~PrepareStatement -- デストラクタ
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

PrepareStatement::~PrepareStatement()
{
	close();
}

//	FUNCTION public
//	Client2::PrepareStatement::close -- クローズする
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

void
PrepareStatement::close()
{
	if (m_iPrepareID != 0) {

		try {
			
			if (m_pSession != 0) {

				m_pSession->erasePrepareStatement(m_iPrepareID);

			} else {

				Connection*	pClientConnection = m_pDataSource->getClientConnection();
				if (pClientConnection != 0) {

					// プリペアステートメントを削除する
					pClientConnection->erasePrepareStatement(m_cstrDatabaseName, m_iPrepareID);
				}
			}

		} catch (...) {
			Common::Thread::resetErrorCondition();
			// 例外は無視する
		}

		m_iPrepareID = 0;
	}
}

//	FUNCTION public
//	Client2::PrepareStatement::getDatabaseName -- データベース名を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUnicodeString&
//		データベース名
//
//	EXCEPTIONS

const ModUnicodeString&
PrepareStatement::getDatabaseName() const
{
	return m_cstrDatabaseName;
}

//	FUNCTION public
//	Client2::PrepareStatement::getPrepareID -- プリペアステートメント ID を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		プリペアステートメント ID
//
//	EXCEPTIONS

int
PrepareStatement::getPrepareID() const
{
	return m_iPrepareID;
}

//
//	Copyright (c) 2006, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
