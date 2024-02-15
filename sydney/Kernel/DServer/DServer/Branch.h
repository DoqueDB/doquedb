// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Branch.h -- 
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_DSERVER_BRANCH_H
#define __SYDNEY_DSERVER_BRANCH_H

#include "DServer/Module.h"

#include "Server/SessionID.h"
#include "Trans/Transaction.h"

#include "Os/CriticalSection.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN

namespace Schema {
	class Database;
}

_SYDNEY_DSERVER_BEGIN

class Manager;

//
// CLASS
//	DServer::Branch --
//
// NOTES
//	子サーバのトランザクションブランチを管理するクラス
//
class SYD_DSERVER_FUNCTION Branch
{
	friend class Manager;
	friend class ModAutoPointer<Branch>;
	
public:
	// 状態
	struct Status
	{
		//	ENUM
		//	DServer::Branch::Status::Value --
		//		トランザクションブランチの実行状況を表す値の列挙型
		//
		//	NOTES

		enum Value
		{
			// 不明
			Unknown,
			
			// 存在しない
			NonExistent,
			// データ操作中
			Active,
			// 待機中
			Idle,
			// コミット準備完了
			Prepared,
			
			// 値の数
			Count
		};
	};

	// 分散マネージャ用のトランザクションブランチ記述子を得る
	static Branch* attach();
	static Branch* attach(const Trans::XID& xid_);
	// 分散マネージャ用のトランザクションブランチ記述子を破棄する
	static void detach(Branch* pBranch_);

	// 
	//	親セッションと連係している間しか操作できないもの
	//	よって、Server::Session から子セッションを得る
	//
	
	// トランザクションブランチを開始する
	void start(Server::SessionID uiSessionID_,
			   Trans::Transaction& cTrans_,
			   const Trans::Transaction::Mode& cMode_);
	// トランザクションブランチを終了する
	void end(Server::SessionID uiSessionID_,
			 Trans::Transaction& cTrans_);
	// トランザクションブランチを一気にコミットする
	void commitOnePhase(Server::SessionID uiSessionID_,
						Trans::Transaction& cTrans_);

	//
	//	親セッションと連携していなくても操作できるもの
	//	よって、DServer::Cascade で新たに子セッションを得る
	//
	
	// トランザクションブランチをコミット準備する
	void prepare(Trans::Transaction& cTrans_);
	// トランザクションブランチをコミットする
	void commit(Trans::Transaction& cTrans_,
				bool isOnePhase_ = false);
	// トランザクションブランチをロールバックする
	void rollback(Trans::Transaction& cTrans_);


	//
	//	その他のメソッド
	//
	
	// トランザクションブランチ識別子を得る
	const Trans::XID& getID() { return m_cXid; }

	// rollback する
	static void rollback(Trans::Transaction& cTrans_,
						 const Trans::Log::Data& cData_,
						 const ModUnicodeString& cstrDatabase_);
	// rollback する
	static void rollback(Trans::Transaction& cTrans_,
						 const Trans::Log::Data& cData_,
						 Schema::Database& cDatabase_);

	// エラーが発生したか否か
	bool isErrorOccurred() const { return m_bErrorOccurred; }
	// エラーが発生した。以後 rollback しか受け付けない
	void setErrorFlag() { m_bErrorOccurred = true; }

private:
	// コンストラクタ
	Branch(const Trans::XID& cXid_);
	// デストラクタ
	~Branch();

	// DServer::Manager から呼び出される
	static void initialize();	// 初期化
	static void terminate();	// 後処理

	// トランザクションブランチ識別子
	Trans::XID m_cXid;
	// 状態
	Status::Value m_eStatus;

	// 排他制御用のラッチ
	mutable Os::CriticalSection	m_cLatch;
	// 参照回数
	mutable unsigned int m_iRefCount;

	// エラーが発生したか否か
	bool m_bErrorOccurred;

	// ハッシュリストでの直前の要素へのポインタ
	Branch*					m_pPrev;
	// ハッシュリストでの直後の要素へのポインタ
	Branch*					m_pNext;
};

_SYDNEY_DSERVER_END
_SYDNEY_END

#endif // __SYDNEY_DSERVER_BRANCH_H

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
