// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PrepareStatement.h -- プリペアステートメント関連のクラス定義、関数宣言
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_CLIENT2_PREPARESTATEMENT_H
#define __TRMEISTER_CLIENT2_PREPARESTATEMENT_H

#include "Client2/Module.h"
#include "Client2/Object.h"

#include "ModUnicodeString.h"

_TRMEISTER_BEGIN
_TRMEISTER_CLIENT2_BEGIN

class DataSource;
class Session;

//	CLASS
//	Client2::PrepareStatement -- プリペアステートメントクラス
//
//	NOTES

class SYD_FUNCTION PrepareStatement : public Object
{
public:

	// コンストラクタ
	PrepareStatement(	DataSource&				cDataSource_,
						const ModUnicodeString&	cstrDatabaseName_,
						int						iPrepareID_);
	PrepareStatement(	Session*	pSession_,
						int			iPrepareID_);

	// デストラクタ
	virtual ~PrepareStatement();

	// クローズする
	void close();

	// データベース名を得る
	const ModUnicodeString& getDatabaseName() const;

	// プリペアステートメント ID を得る
	int getPrepareID() const;

private:

	// データソース
	DataSource*			m_pDataSource;

	// データベース名
	ModUnicodeString	m_cstrDatabaseName;

	// セッション
	Session*			m_pSession;

	// プリペアステートメント ID
	int					m_iPrepareID;
};

_TRMEISTER_CLIENT2_END
_TRMEISTER_END

#endif //__TRMEISTER_CLIENT2_PREPARESTATEMENT_H

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
