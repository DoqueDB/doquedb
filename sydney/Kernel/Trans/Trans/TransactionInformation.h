// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TransactionInformation.h --
//		データベースのトランザクションに関する情報を管理する
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_TRANS_TRANSACTIONINFORMATION_H
#define	__SYDNEY_TRANS_TRANSACTIONINFORMATION_H

#include "Trans/Module.h"
#include "Trans/List.h"
#include "Trans/Transaction.h"

#include "Os/CriticalSection.h"

#include "Schema/ObjectID.h"

_SYDNEY_BEGIN
_SYDNEY_TRANS_BEGIN

namespace Manager
{
	class Transaction;
}

//	CLASS
//	Trans::TransactionInformation --
//		データベースのトランザクションに関する情報を管理する
//
//	NOTES

class TransactionInformation
{
	friend class Manager::Transaction;
	
public:
	// コンストラクター
	TransactionInformation(Schema::ObjectID::Value dbID_);
	// デストラクタ
	virtual ~TransactionInformation();

	// データベースIDを得る
	Schema::ObjectID::Value	getDatabaseID() const
		{ return m_iDatabaseID; }

	// オーバーラップしているトランザクションの内、
	// 最初に開始したトランザクションのIDを得る
	ID getBeginningID() const
		{ return m_iBeginningID; }

	// オーバーラップしているトランザクションの内、
	// 最初に開始したトランザクションのIDを設定する
	void setBeginningID(ID id_)
		{ m_iBeginningID = id_; }
		
	// 現在実行中のトランザクションのうち、
	// ある種別のトランザクションのトランザクション記述子を得る
	List<Transaction>&
	getInProgressList(Transaction::Category::Value category)
		{ return *m_pCategoryList[
			category == Transaction::Category::ReadOnly]; }

	// 現在実行中のトランザクションのうち、
	// 版管理の有無のいずれかのトランザクションのトランザクション記述子を得る
	List<Transaction>&
	getInProgressList(bool noVersion)
		{ return *m_pVersioningList[noVersion]; }

private:
	// データベースID
	Schema::ObjectID::Value			m_iDatabaseID;

	// オーバーラップしているトランザクションの内、
	// 最初に開始したトランザクションのID
	ID								m_iBeginningID;

	// トランザクション種別数
	enum { CategoryCount = 2 };
	// 版管理有無数
	enum { VersioningCount = 2 };

	// トランザクション種別で
	// トランザクション記述子を管理するリストを記憶する配列
	List<Transaction>*				m_pCategoryList[CategoryCount];

	// トランザクションの版管理の有無で
	// トランザクション記述子を管理する散る祖を記憶する配列
	List<Transaction>*				m_pVersioningList[VersioningCount];
	
	// ハッシュリストでの直前の要素へのポインタ
	TransactionInformation*			m_pPrev;
	// ハッシュリストでの直後の要素へのポインタ
	TransactionInformation*			m_pNext;
};

_SYDNEY_TRANS_END
_SYDNEY_END

#endif	// __SYDNEY_TRANS_TRANSACTIONINFORMATION_H

//
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
