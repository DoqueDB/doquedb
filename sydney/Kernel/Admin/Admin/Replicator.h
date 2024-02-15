// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Replicator.h -- データベースを複製するクラス
// 
// Copyright (c) 2014, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_ADMIN_REPLICATOR_H
#define	__SYDNEY_ADMIN_REPLICATOR_H

#include "Admin/Module.h"

#include "Admin/LogData.h"

#include "Common/LargeVector.h"
#include "Common/Thread.h"

#include "Communication/Protocol.h"

#include "Schema/Database.h"

#include "Trans/LogData.h"
#include "Trans/LogInfo.h"

#include "Os/CriticalSection.h"
#include "Os/Event.h"

#include "ModList.h"
#include "ModMap.h"
#include "ModPair.h"

_SYDNEY_BEGIN

namespace Client2
{
class DataSource;
class Port;
}

_SYDNEY_ADMIN_BEGIN

//
//	CLASS
//	Admin::Replicator -- データベースを複製するスレーブ側のクラス
//
//	NOTES
//
class Replicator : public Common::Thread
{
public:
	// コンストラクタ
	Replicator(const ModUnicodeString& cMasterHostName_,
			   int iMasterPortNumber_,
			   int iProtocol_);
	// デストラクタ
	virtual ~Replicator();

	// 初期化する
	static void initialize();
	// 後処理する
	static void terminate();
	
	// 該当するレプリケーターのみ開始する
	static void start(Trans::Transaction& cTransaction_,
					  Schema::Database& cDatabase_);
	// 該当するレプリケーターのみ停止する
	static void stop(Trans::Transaction& cTransaction_,
					 Schema::Database& cDatabase_);

	// マスターデータベースに接続できるかチェックする
	static ModUnicodeString checkConnectMaster(const ModUnicodeString& url_);

	// 実行スレッドを追加する
	void addExecutor(Schema::ObjectID::Value uiSlaveDatabaseID_,
					 const ModUnicodeString& cSlaveDatabaseName_,
					 Schema::ObjectID::Value uiMasterDatabaseID_);
	// 実行スレッドを削除する
	void delExecutor(Trans::Transaction& cTransaction_,
					 Schema::Database& cDatabase_,
					 Schema::ObjectID::Value uiMasterDatabaseID_);

	// すべての実行スレッドを停止する
	void abortAll();

	// ラッチを取得する
	Os::CriticalSection& getLatch() { return m_cLatch; }
	// 実行中か否か
	bool isRunning() { return m_bRunning; }
	// データソースを得る
	Client2::DataSource* getDataSource() { return m_pDataSource; }

private:
	//
	//	CLASS
	//	Admin::Replicator::Queue
	//		-- 論理ログを格納するキュー
	//
	class Queue
	{
	public:
		// コンストラクタ
		Queue();
		// デストラクタ
		~Queue();

		// 参照カウンタを１つ増やす
		int attach();
		// 参照カウンタを１つ減らす
		int detach();

		// 論理ログを書き出す
		void pushBack(Trans::Log::Data* pLog_,
					  Trans::Log::LSN lsn_);
		// 論理ログを取り出す
		bool popFront(Trans::Log::Data*& pLog_,
					  Trans::Log::LSN& lsn_);

		// 終了する
		void abort();
		// 終了したかどうか
		bool isAborted();

	private:
		typedef ModList<ModPair<Trans::Log::Data*, Trans::Log::LSN> >	List;
		// キュー
		List	m_cQueue;

		// 排他制御用
		Os::CriticalSection		m_cLatch;
		Os::Event				m_cEvent;

		// 終了かどうか
		bool					m_bAborted;
	};

	//
	//	CLASS
	//	Admin::Replicator::Executor
	//		-- REDOを実行するスレッド
	//
	class Executor : public Common::Thread
	{
	public:
		//
		//	STRUCT
		//	Admin::Replicator::Executor::TransInfo
		//
		struct TransInfo
		{
			TransInfo()
				: m_pTransaction(0), m_bSchemaModify(false) {}
			TransInfo(Trans::Transaction* pTrans_)
				: m_pTransaction(pTrans_), m_bSchemaModify(false) {}
		
			Trans::Transaction* m_pTransaction;
			bool				m_bSchemaModify;
		};
	
		//
		//	TYPEDEF
		//	Admin::Replicator::Executor::TransMap
		//
		typedef ModMap<Trans::Log::LSN, TransInfo,
					   ModLess<Trans::Log::LSN> > TransMap;

		// コンストラクタ
		Executor(Replicator* pReplicator_,
				 Schema::ObjectID::Value uiSlaveDatabaseID_,
				 const ModUnicodeString& cSlaveDatabaseName_,
				 Schema::ObjectID::Value uiMasterDatabaseID_);
		// デストラクタ
		virtual ~Executor();

		// 終了時に実行途中だったトランザクションを復元する
		Trans::Log::LSN restoreTransaction();
		// マスターに接続する
		void connectMaster(Client2::DataSource* pDataSource_);
		// マスターとの接続を切る
		void disconnectMaster(Client2::DataSource* pDataSource_);

		// 停止する
		void abort() { m_cQueue.abort(); }
		// 停止しているかどうか
		bool isAborted() { return m_cQueue.isAborted(); }

		// キューを得る
		Queue* getQueue() { return &m_cQueue; }

		// ログを作る
		void makeLog(Log::ReplicationEndData& data_);

		// スレーブ終了の論理ログを書き出す
		void storeLog();
		// 実行中のトランザクションをロールバックする
		void rollbackAll();

		// 接続状態を解除する
		void clear() { m_bIsConnected = false; }

	private:
		//
		//	TYPEDEF
		//	Admin::Replicator::Executor::RestoreMap
		//
		typedef ModMap<Trans::Log::LSN, Trans::Log::LSN,
					   ModLess<Trans::Log::LSN> > RestoreMap;
		
		// スレッドとして起動されるメソッド
		void runnable();
		
		// ログをREDOする
		void redo(Trans::Log::Data& cLog_, Trans::Log::LSN uiLSN_);
		
		// トランザクションを開始する
		void transactionBegin(Trans::Log::Data& cLog_,
							  Trans::Log::LSN uiLSN_);
		// トランザクションを確定する
		void transactionCommit(Trans::Log::Data& cLog_,
							   Trans::Log::LSN uiLSN_);
		// トランザクションをロールバックする
		void transactionRollback(Trans::Log::Data& cLog_,
								 Trans::Log::LSN uiLSN_);
		// SQL文を確定する
		void statementCommit(Trans::Log::Data& cLog_,
							 Trans::Log::LSN uiLSN_);
		// SQL文をロールバックする
		void statementRollback(Trans::Log::Data& cLog_,
							   Trans::Log::LSN uiLSN_);
		// データベースのチェックポイント処理
		void checkpointDatabase(Trans::Log::Data& cLog_,
								Trans::Log::LSN uiLSN_);
		// タプル操作
		void tupleModify(Trans::Log::Data& cLog_,
						 Trans::Log::LSN uiLSN_);
		// スキーマ操作
		void schemaModify(Trans::Log::Data& cLog_,
						  Trans::Log::LSN uiLSN_);

		// マスター側のトランザクションのLSNを得る
		Trans::Log::LSN getMasterLSN(Trans::Log::Data& cLog_);
		
		// 復元するトランザクションを探す
		Trans::Log::LSN
		findRestoreLSN(Trans::Transaction& trans_,
					   Schema::Database* pDatabase_,
					   RestoreMap& restoreLSN_);

		// 再実行対象か否か
		bool isRestoreLog(const Trans::Log::Data& cLog_,
						  Trans::Log::LSN lsn_);

		// レプリケーター
		Replicator* m_pReplicator;
		
		// マスター側のデータベースID
		Schema::ObjectID::Value m_uiMasterDatabaseID;

		// スレーブ側のデータベースID
		Schema::ObjectID::Value m_uiDatabaseID;
		// スレーブ側のデータベース名
		ModUnicodeString m_cDatabaseName;

		// トランザクション
		TransMap m_mapTransaction;

		// キュー
		Queue m_cQueue;

		// 再現するトランザクションの開始時のLSN
		// キーはSlaveのLSNで、バリューはMasterのLSN
		RestoreMap m_mapRestoreLSN;

		// トランザクションの再現中か否か
		bool m_bRestore;

		// 最後に書き出した論理ログのマスター側のLSN
		Trans::Log::LSN m_ulLastMasterLSN;

		// 属性の排他制御用
		Os::CriticalSection m_cLatch;

		// 接続済みか否か
		bool m_bIsConnected;
	};
	
	// レプリケーションを開始する
	static void startReplication(Schema::Database* pDatabase_);

	// スレッドとして起動されるメソッド
	void runnable();

	// マスターに接続する
	Client2::Port* getConnection();
	// ログを反映する
	bool execute(Client2::Port* pPort_);
	// キューに追加する
	void pushQueue(Schema::ObjectID::Value dbid_,
				   Trans::Log::Data* pLog_, Trans::Log::LSN uiLSN_);
	// 一定時間待つ
	bool waitConnect();

	// データソース
	Client2::DataSource* m_pDataSource;
	// ホスト名
	ModUnicodeString m_cMasterHostName;
	// ポート番号
	int m_iMasterPortNumber;
	// プロトコル
	Communication::Protocol::Value m_iProtocol;
	
	// キュー (マスターデータベースのIDがキー)
	ModHashMap<Schema::ObjectID::Value, Queue*,
			   ModHasher<Schema::ObjectID::Value> > m_cQueueMap;
	// スレッド (スレーブデータベースのIDがキー)
	ModHashMap<Schema::ObjectID::Value, Executor*,
			   ModHasher<Schema::ObjectID::Value> > m_cExecutorMap;

	// ラッチ
	Os::CriticalSection m_cLatch;
	// 起動済みイベント
	Os::Event m_cRunEvent;

	// ステータス
	bool	m_bRunning;
};

_SYDNEY_ADMIN_END
_SYDNEY_END

#endif	// __SYDNEY_ADMIN_REPLICATOR_H

//
// Copyright (c) 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
