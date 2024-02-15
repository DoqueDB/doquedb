// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Transaction.h -- トランザクション記述子関連のクラス定義、関数宣言
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_TRANS_TRANSACTION_H
#define	__SYDNEY_TRANS_TRANSACTION_H

#include "Trans/Module.h"
#include "Trans/ID.h"
#include "Trans/LogInfo.h"
#include "Trans/TimeStamp.h"
#include "Trans/XID.h"

#include "Common/DataArrayData.h"
#include "Common/ExecutableObject.h"
#include "Lock/Client.h"
#include "Lock/Duration.h"
#include "Lock/Mode.h"
#include "Lock/Timeout.h"
#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"
#include "Os/Timer.h"
#include "Schema/ObjectID.h"
#include "Server/SessionID.h"

#include "ModVector.h"

template <class T>
class ModAutoPointer;

_SYDNEY_BEGIN

namespace Communication
{
	class Connection;
}
namespace Lock
{
	class Name;
}
namespace Schema
{
	class Database;
	class Table;
}

_SYDNEY_TRANS_BEGIN

template <class T>
class List;

class TransactionInformation;

namespace Log
{
	class Data;
}
namespace Manager
{
	class Transaction;
}

//	CLASS
//	Trans::Transaction -- トランザクション記述子を表すクラス
//
//	NOTES

class Transaction
	: public Common::ExecutableObject
{
	friend class AutoLatch;
	friend class AutoUnlatch;
	friend class Branch;
	friend class Manager::Transaction;
	friend class ModAutoPointer<Transaction>;
	friend class TransactionInformation;
	
public:
	//	TYPEDEF
	//	Trans::Tranasction::ID -- トランザクション識別子を表す型
	//
	//	NOTES

	typedef	Trans::ID		ID;

	//	CLASS
	//	Trans::Transaction::Category --
	//		トランザクションの種別を表すクラス
	//
	//	NOTES

	struct Category
	{
		//	ENUM
		//	Trans::Transaction::Category::Value --
		//		トランザクションの種別を表す値の列挙型
		//
		//	NOTES

		enum Value
		{
			// 不明
			Unknown =		0,
			// 読み書き
			ReadWrite,
			// 読み込み専用
			ReadOnly,
			// 値の数
			ValueNum
		};
	};

	//	CLASS
	//	Trans::Transaction::Status --
	//		トランザクションの実行状況を表すクラス
	//
	//	NOTES

	struct Status
	{
		//	ENUM
		//	Trans::Transaction::Status::Value --
		//		トランザクションの実行状況を表す値の列挙型
		//
		//	NOTES
		//		トランザクションの実行状況は以下のように遷移する
		//
		//		    [ Initialized ]
		//			      │
		//		┌────┤
		//		│	      ↓
		//		│   [ InProgress ]
		//		│        │
		//		│        │
		//		│	      ↓
		//		│   [ Preparing (prepare時のみ) ]
		//		│        │
		//		│        ├───────────────┐
		//		│	      ↓                              │
		//		│   [ Prepared (prepare時のみ) ]         │
		//		│        │                              │
		//		│        ├───────┐              │
		//		│        ↓              ↓              │
		//		│   [ Committing ] [ Rollbacking ]       │
		//		│        │              │              │
		//		│        ├───────┼───────┤
		//		│        ↓              ↓              ↓
		//		│   [ Committed ]  [ Rollbacked ]    [ Failed ]
		//		│        │              │              │
		//		└────┴───────┴───────┘ 

		enum Value
		{
			// 記述子が生成されてから一度もトランザクションは開始されていない
			Initialized,
			// コミットが終了した
			Committed,
			// ロールバックが終了した
			Rollbacked,
			// コミットまたはロールバックに失敗した
			Failed,

			//【注意】	以上、トランザクションは実行中でない

			// 実行中
			InProgress,
			// コミット準備中
			Preparing,
			// コミット準備完了
			Prepared,
			// コミット中
			Committing,
			// ロールバック中
			Rollbacking,

			// 値の数
			ValueNum
		};
	};

	//	CLASS
	//	Trans::Transaction::IsolationLevel --
	//		アイソレーションレベルを表すクラス
	//
	//	NOTES

	struct IsolationLevel
	{
		//	ENUM
		//	Trans::Transaction::IsolationLevel::Value --
		//		アイソレーションレベルを表す値の列挙型
		//
		//	NOTES
	
		enum Value
		{
			// 不明
			Unknown =			0,
			// READ UNCOMMITTED
			ReadUncommitted,
			// READ COMMITTED
			ReadCommitted,
			// REPEATABLE READ (Cursor Stability と同等)
			RepeatableRead,
			// SERIALIZABLE
			Serializable,
			// 値の数
			ValueNum
		};
	};

	//	CLASS
	//	Trans::Transaction::Mode --
	//		トランザクションモードを表すクラス
	//
	//	NOTES
	//		このクラスは new することはないはずなので、
	//		Common::Object の子クラスにしない

	struct Mode
	{
		// デフォルトコンストラクター
		Mode();
		// コンストラクター
		Mode(Category::Value category,
			 IsolationLevel::Value isoLevel, Boolean::Value snapshot);
		// デストラクター
		~Mode();

		// 種別
		Category::Value			_category;
		// アイソレーションレベル
		IsolationLevel::Value	_isoLevel;
		// 必ず版管理するか
		Boolean::Value			_snapshot;
	};

	//	CLASS
	//	Trans::Transaction::Type --
	//		トランザクションのタイプを表すクラス
	//
	//	NOTES

	struct Type
	{
		//	ENUM
		//	Trans::Transaction::Type::Value --
		//		トランザクションのタイプを表す値の列挙型
		//
		//	NOTES

		enum Value
		{
			// 不明
			Unknown =			0,
			// 明示的に開始されたトランザクション
			Explicit,
			// 暗黙的に開始されたトランザクション
			Implicit,
			// セッションと連係した「データ操作中」のトランザクションブランチ
			Branch,
			// 値の数
			Count
		};
	};

	// トランザクション記述子を生成する
	SYD_TRANS_FUNCTION
	static Transaction*
	attach(Server::SessionID sessionID = Server::IllegalSessionID);
	// 参照回数を 1 増やす
	SYD_TRANS_FUNCTION
	static Transaction*
	attach(const Transaction& trans);

	// 参照回数を 1 減らす
	SYD_TRANS_FUNCTION
	static void				detach(const Transaction*& trans);
	// トランザクション記述子を破棄する
	SYD_TRANS_FUNCTION
	static void				detach(Transaction*& trans);

	// トランザクションを開始する
	const ID&
	begin(Schema::ObjectID::Value databaseID, Category::Value category,
		  Type::Value type = Type::Unknown);
	const ID&
	begin(Schema::ObjectID::Value databaseID,
		  Category::Value category, IsolationLevel::Value isoLevel,
		  Type::Value type = Type::Unknown);
	SYD_TRANS_FUNCTION
	const ID&
	begin(Schema::ObjectID::Value databaseID, const Mode& mode,
		  Type::Value type = Type::Unknown);
	SYD_TRANS_FUNCTION
	const ID&
	begin(Schema::ObjectID::Value databaseID,
		  const Mode& mode, bool noLock, bool noLog,
		  Type::Value type = Type::Unknown);

	// トランザクションをコミットする
	SYD_TRANS_FUNCTION
	void					commit(bool bNeedReCache_ = false,
								   Log::LSN masterLSN = Log::IllegalLSN);
	// トランザクションをロールバックする
	SYD_TRANS_FUNCTION
	void					rollback(Log::LSN masterLSN = Log::IllegalLSN);

	// デフォルトのトランザクションモードを得る
	const Mode&				getDefaultMode() const;
	// デフォルトのトランザクションモードを設定する
	void					setDefaultMode(const Mode& mode);

	// トランザクションで SQL の 1 文の実行を開始することを通知する
	SYD_TRANS_FUNCTION
	void
	beginStatement(Communication::Connection* connection = 0);
	// トランザクションで SQL の 1 文の実行をコミットしたことを通知する
	SYD_TRANS_FUNCTION
	void					commitStatement(
		Log::LSN masterLSN = Log::IllegalLSN);
	// トランザクションで SQL の 1 文の実行をロールバックしたことを通知する
	SYD_TRANS_FUNCTION
	void					rollbackStatement(
		Log::LSN masterLSN = Log::IllegalLSN);
	// トランザクションで現在実行中の SQL の 1 文の中断が指示されたか
	SYD_TRANS_FUNCTION
	bool
	isCanceledStatement() const;
	
	// クライアントとのコネクションをflushする
	SYD_TRANS_FUNCTION
	void					flushConnection();

	// オブジェクトにロックをかける
	SYD_TRANS_FUNCTION
	bool
	lock(const Lock::Name& name, Lock::Mode::Value mode,
		 Lock::Duration::Value duration,
		 Lock::Timeout::Value timeout =	Lock::Timeout::Unlimited);
	// オブジェクトのロックをはずす
	SYD_TRANS_FUNCTION
	void
	unlock(const Lock::Name& name, Lock::Mode::Value mode,
		   Lock::Duration::Value duration);

	// オブジェクトのロックを変換する
	bool
	convertLock(const Lock::Name& name, Lock::Mode::Value from,
				Lock::Duration::Value duration,	Lock::Mode::Value to,
				Lock::Timeout::Value timeout = Lock::Timeout::Unlimited);
#ifdef OBSOLETE
	void
	convertLock(const Lock::Name& name, Lock::Mode::Value mode,
				Lock::Duration::Value from,	Lock::Duration::Value to);
#endif
	SYD_TRANS_FUNCTION
	bool
	convertLock(const Lock::Name& name,
				Lock::Mode::Value from,	Lock::Duration::Value fromDuration,
				Lock::Mode::Value to, Lock::Duration::Value toDuration,
				Lock::Timeout::Value timeout = Lock::Timeout::Unlimited);

	// ある種類のオブジェクトを操作するときの適切なロックを得る
	SYD_TRANS_FUNCTION
	bool
	getAdequateLock(Lock::Name::Category::Value locked,
					bool readOnly, Lock::Mode::Value& mode,
					Lock::Duration::Value& duration,
					bool batchMode = false) const;
	SYD_TRANS_FUNCTION
	bool
	getAdequateLock(Lock::Name::Category::Value locked,
					Lock::Name::Category::Value manipulated,
					bool readOnly, Lock::Mode::Value& mode,
					Lock::Duration::Value& duration,
					bool batchMode = false) const;

	// オブジェクトにラッチをかける
	SYD_TRANS_FUNCTION
	void					latch(const Lock::Name& name);
	// オブジェクトのラッチをはずす
	SYD_TRANS_FUNCTION
	void					unlatch(const Lock::Name& name);

	// 論理ログファイルを生成する
	void
	createLog(Log::File::Category::Value category);
	// 論理ログファイルを破棄する
	void
	destroyLog(Log::File::Category::Value category);
	// 論理ログファイルをマウントする
	void
	mountLog(Log::File::Category::Value category);
	// 論理ログファイルをアンマウントする
	void
	unmountLog(Log::File::Category::Value category);
	// 論理ログファイルの実体である OS ファイルの絶対パス名を変更する
	void
	renameLog(Log::File::Category::Value category, const Os::Path& path);
	// 論理ログファイルをフラッシュする
	void
	flushLog(Log::File::Category::Value category,
			 Log::LSN lsn = Log::IllegalLSN);

	// 論理ログを記憶する
	SYD_TRANS_FUNCTION
	Log::LSN
	storeLog(Log::File::Category::Value category, const Log::Data& data,
			 Log::LSN masterLSN = Log::IllegalLSN);
	// 論理ログヘッダーに同期処理が完了したことを設定する
	SYD_TRANS_FUNCTION
	void
	setSynchronizeDone(Log::File::Category::Value category);
	// 論理ログを取り出す
	Log::Data*
	loadLog(Log::File::Category::Value category, Log::LSN lsn) const;
	// 不要な論理ログを削除する(トランザクションのコミットまで遅延する)
	SYD_TRANS_FUNCTION
	void					discardLog(bool bDiscardFull_);
#ifdef OBSOLETE
	// 先頭の論理ログの LSN を得る
	Log::LSN
	getFirstLSN(Log::File::Category::Value category) const;
	// 末尾の論理ログの LSN を得る
	Log::LSN
	getLastLSN(Log::File::Category::Value category) const;
	// ある論理ログの直後のものの LSN を得る
	Log::LSN
	getNextLSN(Log::File::Category::Value category, Log::LSN lsn) const;
	// ある論理ログの直前のものの LSN を得る
	Log::LSN
	getPrevLSN(Log::File::Category::Value category, Log::LSN lsn) const;
#endif
	// トランザクションの操作する論理ログファイルを得る
	Log::AutoFile
	getLog(Log::File::Category::Value category) const;
	// トランザクションの操作するデータベース用の論理ログファイルを設定する
	SYD_TRANS_FUNCTION
	void
	setLog(Schema::Database& database);
	// 論理ログファイルが記録されたか
	SYD_TRANS_FUNCTION
	bool
	isLogStored(Log::File::Category::Value category =
				Log::File::Category::Database) const;

	// 論理ログからトランザクションの状態を復元する
	void restoreLog(const Log::Data& data, Log::LSN lsn);

	// トランザクションごとの論理ログファイルに関する情報を得る
	const Log::Info&
	getLogInfo(Log::File::Category::Value category) const;

	// UNDO ログを確実に記憶できるように準備する
	SYD_TRANS_FUNCTION
	void					prepareUndoLog();
	// UNDO ログを記憶する
	SYD_TRANS_FUNCTION
	void					storeUndoLog(Common::DataArrayData::Pointer p);
	// UNDO ログを取り出す
	const Common::DataArrayData& loadUndoLog() const;
	// 記憶している UNDO ログをすべて忘れる
	void					clearUndoLog();

	// 論理ログおよび UNDO ログを記録する必要があるか
	bool					isNecessaryLog() const;

	// セッション識別子を得る
	Server::SessionID		getSessionID() const;
	// トランザクション識別子を得る
	const ID&				getID() const;
	// トランザクションの種類を得る
	Category::Value			getCategory() const;
	// トランザクションの実行状況を得る
	Status::Value			getStatus() const;
	// トランザクションのアイソレーションレベルを得る
	IsolationLevel::Value	getIsolationLevel() const;
	// このトランザクションを実体とする
	// トランザクションブランチのトランザクションブランチ識別子を得る
	const XID&
	getXID() const;
	// 版管理するトランザクションの開始時タイムスタンプを得る
	const TimeStamp&		getBirthTimeStamp() const;
#ifdef OBSOLETE
	// 版管理するトランザクションの現在実行中の
	// SQL の 1 文の開始時タイムスタンプを得る
	const TimeStamp&		getStatementTimeStamp() const;
#endif
	// 版管理するトランザクションの開始時に実行中の
	// 更新トランザクションのトランザクション識別子を得る
	const ModVector<ID>&	getStartingList() const;

	// 実行中のトランザクションのトランザクション記述子を得る
	SYD_TRANS_FUNCTION
	static const List<Transaction>&
	getInProgressList(Schema::ObjectID::Value dbID, Category::Value category);
	SYD_TRANS_FUNCTION
	static const List<Transaction>&
	getInProgressList(Schema::ObjectID::Value dbID, bool noVersion);

	// 実行中のトランザクションが存在しているか否か
	SYD_TRANS_FUNCTION
	static bool					isAnyInProgress(Category::Value category);
	

#ifdef OBSOLETE
	// トランザクションが操作するデータベースの名前を得る
	SYD_TRANS_FUNCTION
	ModUnicodeString
	getDatabaseName() const;
#endif
	// トランザクションが操作するデータベースのIDを得る
	Schema::ObjectID::Value getDatabaseID() const
		{ return _databaseID; }

	// ロックしないトランザクションかを設定する
	SYD_TRANS_FUNCTION
	bool
	setNoLock(bool v);
	// 論理ログおよび UNDO ログを記録しないトランザクションかを設定する
	SYD_TRANS_FUNCTION
	bool
	setNoLog(bool v);
	// バッチモードのトランザクションかを設定する
	SYD_TRANS_FUNCTION
	bool
	setBatchMode(bool v);
	// ロックしないトランザクションか
	bool
	isNoLock() const;
	// 論理ログおよび UNDO ログを記録しないトランザクションか
	bool
	isNoLog() const;
	// 版管理しないトランザクションか
	bool
	isNoVersion() const;
	// バッチモードのトランザクションか
	bool
	isBatchMode() const;
	// 今後、このトランザクションの実行中に開始される
	// 読取専用トランザクションに版管理を使用させないか
	bool
	isDeterrent() const;

	// トランザクションが実行中か
	bool
	isInProgress() const;
	SYD_TRANS_FUNCTION
	static bool
	isInProgress(Schema::ObjectID::Value dbID,
				 const ID& id, Category::Value category = Category::Unknown);
#ifdef OBSOLETE
	SYD_TRANS_FUNCTION
	static bool
	isInProgress(Schema::ObjectID::Value dbID,
				 const ID& id, bool noVersion);
#endif
	SYD_TRANS_FUNCTION
	static bool
	isInProgress(Schema::ObjectID::Value dbID,
				 const ModVector<ID>& ids,
				 Category::Value category = Category::Unknown);
#ifdef OBSOLETE
	SYD_TRANS_FUNCTION
	static bool
	isInProgress(Schema::ObjectID::Value dbID,
				 const ModVector<ID>& ids, bool noVersion);
#endif

	// 開始時にある更新トランザクションが実行中だったか
	SYD_TRANS_FUNCTION
	bool
	isOverlapped(const ID& id) const;
	SYD_TRANS_FUNCTION
	bool
	isOverlapped(const ModVector<ID>& ids) const;

	// 一括登録を開始する
	void
	startBatchInsert(const Schema::Table& cTable_);
	// 一括登録をコミットする
	void
	commitBatchInsert();

	// オーバーラップしているトランザクションの内、
	// 最初に開始したトランザクションのIDを得る
	SYD_TRANS_FUNCTION
	static ID
	getBeginningID(Schema::ObjectID::Value dbID);

	// セッション識別子を設定する
	void setSessionID(Server::SessionID sessionID_)
		{ _sessionID = sessionID_; }

	//
	// パフォーマンス情報
	//

	// ロック時間
	const Os::Timer& getLockTimer() const
		{ return m_cLockTimer; }
	// クライアントに返した行数
	int getSendRowCount() const
		{
			Os::AutoCriticalSection cAuto(m_cLatch);
			return m_iSendRowCount;
		}
	void addSendRowCount()
		{
			Os::AutoCriticalSection cAuto(m_cLatch);
			++m_iSendRowCount;
		}
	// ページ参照数
	int getPageReferenceCount() const
		{
			Os::AutoCriticalSection cAuto(m_cLatch);
			return m_iPageReferenceCount;
		}
	void addPageReferenceCount()
		{
			Os::AutoCriticalSection cAuto(m_cLatch);
			++m_iPageReferenceCount;
		}
	// ファイル読み込み数
	int getPageReadCount() const
		{
			Os::AutoCriticalSection cAuto(m_cLatch);
			return m_iPageReadCount;
		}
	void addPageReadCount()
		{
			Os::AutoCriticalSection cAuto(m_cLatch);
			++m_iPageReadCount;
		}
	// ロック回数
	int getLockCount() const
		{
			Os::AutoCriticalSection cAuto(m_cLatch);
			return m_iLockCount;
		}
	int addLockCount()
		{
			Os::AutoCriticalSection cAuto(m_cLatch);
			return ++m_iLockCount;
		}

	// 暗黙的に開始されたトランザクションか否か
	bool isImplicit() const { return m_eType == Type::Implicit; }

	// 同じ版を更新した
	void setUpdatedSameVersion() const { _updatedSameVersion = true; }

	// 論理ログに記録するか否か
	void setStoreLogFlag(bool flag_) { m_bStoreLog = flag_; }
	

private:
	// コンストラクター
	Transaction(Server::SessionID sessionID);
	Transaction(const XID& xid);
	// デストラクター
	~Transaction();
	// コンストラクター下位関数
	void
	construct();

	// トランザクションのコミットを準備しておく
	void
	prepare();

	// トランザクションの終了時に共通の処理を行う
	void					endTransaction(Status::Value status,
										   bool isCategory = true,
										   bool isVersioning = true,
										   bool bNeedReCache_ = false);
	// トランザクションで SQL の 1 文の実行の終了時に共通の処理を行う
	void
	endStatement(bool isSucceeded);

	// トランザクション中の操作をすべて UNDO する
	void
	undo();
	// ある種類の論理ログファイルに記録されている
	// トランザクション中の操作をすべて UNDO する
	bool
	undo(Log::File::Category::Value category);
	// トランザクションの SQL の 1 文での操作をすべて UNDO する
	void
	undoStatement();
	// ある種類の論理ログファイルに記録されている
	// トランザクションの SQL の 1 文での操作を UNDO する
	bool
	undoStatement(Log::File::Category::Value category);

	// バッチモードの開始時にrecoverする
	void recoverBatch();

	// 記述子が参照されているか
	bool					isAttached() const;

	// セッション識別子
	Server::SessionID		_sessionID;
	// 参照回数
	mutable unsigned int	_refCount;
	// トランザクション識別子
	//【注意】版管理するトランザクションの開始時タイムスタンプでもある
	ID						_id;
	// 種別
	Category::Value			_category;
	// 実行状況
	Status::Value			_status;
	// アイソレーションレベル
	IsolationLevel::Value	_isoLevel;
	// このトランザクションを実体とする
	// トランザクションブランチのトランザクションブランチ識別子
	XID						_xid;
#ifdef OBSOLETE
	/*
	 *【未実装】版管理するトランザクションにおける READ COMMITTED
	 */
	// 版管理するトランザクションで現在実行中の SQL 文の開始時タイムスタンプ
	TimeStamp				_statementTimeStamp;
#endif
	// 開始時(更新)トランザクションリスト
	ModVector<ID>			_startingList;
	// ロックするか
	bool					_noLock;
	// 論理ログを記録しないか
	bool					_noLog;
	// 版管理を使用しないか
	bool					_noVersion;
	// 今後、このトランザクションの実行中に開始される
	// 読取専用トランザクションに版管理を使用させないか
	bool					_deterrent;
	// ロック要求元
	Lock::Client			_client;

	// トランザクションモードのデフォルト値
	Mode					_default;

	// 中断要求を検知するためのコネクション
	Communication::Connection* _connection;

	// トランザクションごとのシステム用の論理ログファイルに関する情報
	Log::Info				_systemLogInfo;
	// トランザクションごとのデータベース用の論理ログファイルに関する情報
	Log::Info				_databaseLogInfo;
	// UNDO ログを記憶する配列
	Common::DataArrayData	_undoLog;
	
	// バッチモード開始前にフラッシュした時点のタイムスタンプ
	TimeStamp				_batchTimeStamp;
	// バッチモードが実行されているデータベースのスキーマオブジェクト識別子
	Schema::ObjectID		_batchDatabaseID;
	// バッチモードが実行されている表のスキーマオブジェクト識別子
	Schema::ObjectID		_batchTableID;
	// バッチモードか
	bool					_batchMode;
	
	// 他のトランザクションと同じ版を更新したか
	mutable bool			_updatedSameVersion;

	// データベースID
	Schema::ObjectID::Value	_databaseID;

	//【注意】	以下のメンバーは、
	//			それぞれのリストのラッチにより保護される

	// ハッシュリストでの直前の要素へのポインタ
	Transaction*			_hashPrev;
	// ハッシュリストでの直後の要素へのポインタ
	Transaction*			_hashNext;

	// 実行中トランザクションの種別別リストでの直前の要素へのポインタ
	Transaction*			_categoryPrev;
	// 実行中トランザクションの種別別リストでの直後の要素へのポインタ
	Transaction*			_categoryNext;
	// 実行中トランザクションの版管理有無別リストでの直前の要素へのポインタ
	Transaction*			_versioningPrev;
	// 実行中トランザクションの版管理有無別リストでの直後の要素へのポインタ
	Transaction*			_versioningNext;

	// ロック待ち時間
	Os::Timer				m_cLockTimer;
	// クライアントの返した行数
	int						m_iSendRowCount;
	// ページ参照数
	int						m_iPageReferenceCount;
	// ファイル読み込み数
	int						m_iPageReadCount;
	// ロック回数
	int						m_iLockCount;

	// 排他制御用
	mutable Os::CriticalSection		m_cLatch;

	// トランザクションのタイプ
	Type::Value				m_eType;
	
	// 論理ログに記録するか否か
	bool					m_bStoreLog;
};

//	FUNCTION private
//	Trans::Transaction::~Transaction --
//		トランザクション記述子を表すクラスのデストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Transaction::~Transaction()
{}

//	FUNCTION public
//	Trans::Transaction::begin -- トランザクションを開始する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction::Category::Value	category
//			Trans::Transaction::Category::Unknown 以外の値
//				指定された種別のトランザクションを開始する
//			Trans::Transaction::Category::Unknown または指定されないとき
//				デフォルトの種別のトランザクションを開始する
//		Trans::Transaction::IsolationLevel::Value	isoLevel
//			Trans::Transaction::IsolationLevel::Unknown 以外の値
//				指定されたアイソレーションレベルのトランザクションを開始する
//			Trans::Transaction::IsolationLevel::Unknown または指定されないとき
//				デフォルトのアイソレーションレベルのトランザクションを開始する
//
//	RETURN
//		開始されたトランザクションのトランザクション識別子
//
//	EXCEPTIONS

inline
const Transaction::ID&
Transaction::begin(Schema::ObjectID::Value databaseID, Category::Value category,
				   Type::Value type)
{
	return begin(databaseID,
				 Mode(category, IsolationLevel::Unknown, Boolean::Unknown),
				 false, false, type);
}

inline
const Transaction::ID&
Transaction::begin(Schema::ObjectID::Value databaseID,
				   Category::Value category, IsolationLevel::Value isoLevel,
				   Type::Value type)
{
	return begin(databaseID,
				 Mode(category, isoLevel, Boolean::Unknown),
				 false, false, type);
}

//	FUNCTION public
//	Trans::Transaction::getDefaultMode --
//		デフォルトのトランザクションモードを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたトランザクションモード
//
//	EXCEPTIONS
//		なし

inline
const Transaction::Mode&
Transaction::getDefaultMode() const
{
	return _default;
}

//	FUNCTION public
//	Trans::Transaction::setDefaultMode --
//		デフォルトのトランザクションモードを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction::Mode&	mode
//			デフォルトとして設定するトランザクションモード
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
Transaction::setDefaultMode(const Mode& mode)
{
	if (mode._category != Category::Unknown)
		_default._category = mode._category;
	if (mode._isoLevel != IsolationLevel::Unknown)
		_default._isoLevel = mode._isoLevel;
	if (mode._snapshot != Boolean::Unknown)
		_default._snapshot = mode._snapshot;
}

//	FUNCTION public
//	Trans::Transaction::convertLock --
//		あるオブジェクトのロックのモードを他のモードに変換する
//
//	NOTES
//		最大待ち時間を指定しないと、返り値として false を返すことはない
//
//	ARGUMENTS
//		Lock::Name&			name
//			モードを変換するオブジェクトのロック名
//		Lock::Mode::Value	from
//			変換前のモード
//		Lock::Duration::Value	duration
//			モードを変換するロックの持続期間
//		Lock::Mode::Value	to
//			変換後のロックのモード
//		Lock::Timeout::Value	timeout
//			指定されたとき
//				ロック待ちするときの最大待ち時間(単位 ミリ秒)
//			指定されないとき
//				無制限にロック待ちする
//
//	RETURN
//		true
//			ロックした
//		false
//			最大待ち時間、ロック待ちしたので、ロックをあきらめた
//
//	EXCEPTIONS

inline
bool
Transaction::convertLock(
	const Lock::Name& name,
	Lock::Mode::Value from, Lock::Duration::Value duration,
	Lock::Mode::Value to, Lock::Timeout::Value timeout)
{
	return convertLock(name, from, duration, to, duration, timeout);
}

#ifdef OBSOLETE
//	FUNCTION public
//	Trans::Transaction::convertLock --
//		あるオブジェクトのロックの持続期間を他の持続期間に変換する
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name&			name
//			モードを変換するオブジェクトのロック名
//		Lock::Mode::Value	mode
//			持続期間を変換するロックのモード
//		Lock::Duration::Value	from
//			変換前の持続期間
//		Lock::Duration::Value	to
//			変換後の持続期間
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
Transaction::convertLock(
	const Lock::Name& name, Lock::Mode::Value mode,
	Lock::Duration::Value from,	Lock::Duration::Value to)
{
	(void) convertLock(name, mode, from, mode, to);
}
#endif

//	FUNCTION public
//	Trans::Transaction::createLog -- 論理ログファイルを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::File::Category::Value	category
//			Trans::Log::File::Category::Database の場合
//				トランザクション中に操作するデータベース用の
//				論理ログファイルを生成する
//			Trans::Log::File::Category::System の場合
//				システム用の論理ログファイルを生成する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
Transaction::createLog(Log::File::Category::Value category)
{
	((category == Log::File::Category::System) ?
	 _systemLogInfo : _databaseLogInfo).create();
}

//	FUNCTION public
//	Trans::Transaction::destroyLog -- 論理ログファイルを破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::File::Category::Value	category
//			Trans::Log::File::Category::Database の場合
//				トランザクション中に操作するデータベース用の
//				論理ログファイルを破棄する
//			Trans::Log::File::Category::System の場合
//				システム用の論理ログファイルを破棄する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
Transaction::destroyLog(Log::File::Category::Value category)
{
	((category == Log::File::Category::System) ?
	 _systemLogInfo : _databaseLogInfo).destroy();
}

//	FUNCTION public
//	Trans::Transaction::mountLog -- 論理ログファイルをマウントする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::File::Category::Value	category
//			Trans::Log::File::Category::Database の場合
//				トランザクション中に操作するデータベース用の
//				論理ログファイルをマウントする
//			Trans::Log::File::Category::System の場合
//				システム用の論理ログファイルをマウントする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
Transaction::mountLog(Log::File::Category::Value category)
{
	((category == Log::File::Category::System) ?
	 _systemLogInfo : _databaseLogInfo).mount();
}

//	FUNCTION public
//	Trans::Transaction::unmountLog -- 論理ログファイルをアンマウントする
//
//	NOTES
//		Trans::Transaction::storeLog によって、
//		トランザクションの開始を表す論理ログを記録してから、
//		その終了を表す論理ログを記録するまでは、実際のアンマウントは遅延される
//
//	ARGUMENTS
//		Trans::Log::File::Category::Value	category
//			Trans::Log::File::Category::Database の場合
//				トランザクション中に操作するデータベース用の
//				論理ログファイルをアンマウントする
//			Trans::Log::File::Category::System の場合
//				システム用の論理ログファイルをアンマウントする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
Transaction::unmountLog(Log::File::Category::Value category)
{
	((category == Log::File::Category::System) ?
	 _systemLogInfo : _databaseLogInfo).unmount();
}

//	FUNCTION public
//	Trans::Transaction::flushLog -- 論理ログファイルをフラッシュする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::File::Category::Value	category
//			Trans::Log::File::Category::Database の場合
//				トランザクション中に操作するデータベース用の
//				論理ログファイルをフラッシュする
//			Trans::Log::File::Category::System の場合
//				システム用の論理ログファイルをフラッシュする
//		Trans::Log::LSN		lsn
//			Trans::Log::IllegalLSN 以外のとき
//				論理ログファイルの先頭から
//				このログシーケンス番号の表す論理ログまでフラッシュする
//			Trans::Log::IllegalLSN または指定されないとき
//				論理ログファイル全体をフラッシュする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
Transaction::flushLog(Log::File::Category::Value category, Log::LSN lsn)
{
	((category == Log::File::Category::System) ?
	 _systemLogInfo : _databaseLogInfo).flush(lsn);
}

//	FUNCTION public
//	Trans::Transaction::renameLog --
//		論理ログファイルの実体である OS ファイルの絶対パス名を変更する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::File::Category::Value	category
//			Trans::Log::File::Category::Database の場合
//				トランザクション中に操作するデータベース用の
//				論理ログファイルの実体である OS ファイルの絶対パス名を変更する
//			Trans::Log::File::Category::System の場合
//				システム用の論理ログファイルの実体である
//				OS ファイルの絶対パス名を変更する
//		Os::Path&			path
//			論理ログファイルの実体である OS ファイルの変更後の絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
Transaction::
renameLog(Log::File::Category::Value category, const Os::Path& path)
{
	((category == Log::File::Category::System) ?
	 _systemLogInfo : _databaseLogInfo).rename(path);
}

//	FUNCTION public
//	Trans::Transaction::loadLog --
//		トランザクションで行った操作に関する論理ログを
//		論理ログファイルから取り出す
//
//	NOTES
//		呼び出し側で論理ログファイルに対して排他制御を行う必要がある
//
//	ARGUMENTS
//		Trans::Log::File::Category::Value	category
//			Trans::Log::File::Category::Database の場合
//				トランザクション中に操作するデータベース用の
//				論理ログファイルから論理ログを取り出す
//			Trans::Log::File::Category::System の場合
//				システム用の論理ログファイルから論理ログを取り出す
//		Trans::Log::LSN		lsn
//			取り出す論理ログの LSN
//
//	RETURN
//		0 以外の値
//			取り出された論理ログのデータを記憶する領域の先頭アドレス
//		0
//			論理ログを取り出すべき論理ログファイルがないので、
//			論理ログを取り出せない
//
//	EXCEPTIONS

inline
Log::Data*
Transaction::loadLog(Log::File::Category::Value category, Log::LSN lsn) const
{
	return getLogInfo(category).load(lsn);
}

#ifdef OBSOLETE
//	FUNCTION public
//	Trans::Transaction::getFirstLSN --
//		トランザクションで行った操作に関する論理ログを記録する
//		論理ログファイルの先頭の論理ログのログシーケンス番号を得る
//
//	NOTES
//		呼び出し側で論理ログファイルに対して排他制御を行う必要がある
//
//	ARGUMENTS
//		Trans::Log::File::Category::Value	category
//			Trans::Log::File::Category::Database の場合
//				トランザクション中に操作するデータベース用の
//				論理ログファイルのものを得る
//			Trans::Log::File::Category::System の場合
//				システム用の論理ログファイルのものを得る
//
//	RETURN
//		得られたログシーケンス番号
//
//	EXCEPTIONS

inline
Log::LSN
Transaction::getFirstLSN(Log::File::Category::Value category) const
{
	return getLogInfo(category).getFirstLSN();
}

//	FUNCTION public
//	Trans::Transaction::getLastLSN --
//		トランザクションで行った操作に関する論理ログを記録する
//		論理ログファイルの末尾の論理ログのログシーケンス番号を得る
//
//	NOTES
//		呼び出し側で論理ログファイルに対して排他制御を行う必要がある
//
//	ARGUMENTS
//		Trans::Log::File::Category::Value	category
//			Trans::Log::File::Category::Database の場合
//				トランザクション中に操作するデータベース用の
//				論理ログファイルのものを得る
//			Trans::Log::File::Category::System の場合
//				システム用の論理ログファイルのものを得る
//
//	RETURN
//		得られたログシーケンス番号
//
//	EXCEPTIONS

inline
Log::LSN
Transaction::getLastLSN(Log::File::Category::Value category) const
{
	return getLogInfo(category).getLastLSN();
}

//	FUNCTION public
//	Trans::Transaction::getNextLSN --
//		トランザクションで行った操作に関する論理ログを記録する
//		論理ログファイルのあるログシーケンス番号の論理ログの
//		直後のもののログシーケンス番号を得る
//
//	NOTES
//		呼び出し側で論理ログファイルに対して排他制御を行う必要がある
//
//	ARGUMENTS
//		Trans::Log::File::Category::Value	category
//			Trans::Log::File::Category::Database の場合
//				トランザクション中に操作するデータベース用の
//				論理ログファイルのものを得る
//			Trans::Log::File::Category::System の場合
//				システム用の論理ログファイルのものを得る
//		Trans::Log::LSN		lsn
//			このログシーケンス番号の論理ログの直後のものの
//			ログシーケンス番号を得る
//
//	RETURN
//		得られたログシーケンス番号
//
//	EXCEPTIONS

inline
Log::LSN
Transaction::
getNextLSN(Log::File::Category::Value category, Log::LSN lsn) const
{
	return getLogInfo(category).getNextLSN(lsn);
}

//	FUNCTION public
//	Trans::Transaction::getPrevLSN --
//		トランザクションで行った操作に関する論理ログを記録する
//		論理ログファイルのあるログシーケンス番号の論理ログの
//		直前のもののログシーケンス番号を得る
//
//	NOTES
//		呼び出し側で論理ログファイルに対して排他制御を行う必要がある
//
//	ARGUMENTS
//		Trans::Log::File::Category::Value	category
//			Trans::Log::File::Category::Database の場合
//				トランザクション中に操作するデータベース用の
//				論理ログファイルのものを得る
//			Trans::Log::File::Category::System の場合
//				システム用の論理ログファイルのものを得る
//		Trans::Log::LSN		lsn
//			このログシーケンス番号の論理ログの直前のものの
//			ログシーケンス番号を得る
//
//	RETURN
//		得られたログシーケンス番号
//
//	EXCEPTIONS

inline
Log::LSN
Transaction::
getPrevLSN(Log::File::Category::Value category, Log::LSN lsn) const
{
	return getLogInfo(category).getPrevLSN(lsn);
}
#endif

//	FUNCTION public
//	Trans::Transaction::getLog --
//		トランザクションの操作する論理ログファイルを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::File::Category::Value	category
//			Trans::Log::File::Category::System のとき
//				システム用の論理ログファイルを得る
//			その他の値のとき
//				データベース用の論理ログファイルを得る
//
//	RETURN
//		得られた論理ログファイルを表すクラス
//
//	EXCEPTIONS
//		なし

inline
Log::AutoFile
Transaction::getLog(Log::File::Category::Value category) const
{
	return getLogInfo(category).getFile();
}

//	FUNCTION public
//	Trans::Transaction::getLogInfo --
//		トランザクションごとの論理ログファイルに関する情報を得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::File::Category::Value	category
//			Trans::Log::File::Category::System のとき
//				システム用の論理ログファイルに関する情報を得る
//			その他の値のとき
//				データベース用の論理ログファイルに関する情報を得る
//
//	RETURN
//		得られたトランザクションごとの論理ログファイルに関する情報を表すクラス
//
//	EXCEPTIONS
//		なし

inline
const Log::Info&
Transaction::getLogInfo(Log::File::Category::Value category) const
{
	return (category == Log::File::Category::System) ?
		_systemLogInfo : _databaseLogInfo;
}

//	FUNCTION public
//	Trans::Transaction::loadUndoLog --
//		論理ファイルに対する操作を UNDO するための情報である
//		UNDO ログを取り出す
//
//	NOTES
//		取り出した UNDO ログは Trans::Transaction::clearUndoLog を
//		呼び出すまでか、トランザクション記述子の表すトランザクションが
//		終了するまで有効である
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		記憶されている UNDO ログを表す配列へのポインタを
//		要素として持つ配列へのレファレンス
//
//	EXCEPTIONS
//		なし

inline
const Common::DataArrayData&
Transaction::loadUndoLog() const
{
	return _undoLog;
}

//	FUNCTION private
//	Trans::Transaction::clearUndoLog --
//		論理ファイルに対する操作を UNDO するための情報である
//		UNDO ログの記憶を忘れる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
Transaction::clearUndoLog()
{
	_undoLog.clear();
}

//	FUNCTION public
//	Trans::Transaction::isNecessaryLog --
//		論理ログと UNDO ログを記録する必要があるか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			論理ログと UNDO ログを記録する必要がある
//		false
//			論理ログまたは UNDO ログを記録する必要がない
//
//	EXCEPTIONS

inline
bool
Transaction::isNecessaryLog() const
{
	return !isNoLog() && !isBatchMode() && getStatus() == Status::InProgress;
}

//	FUNCTION public
//	Trans::Transaction::getSessionID --
//		トランザクション記述子を生成したセッションのセッション識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたセッション識別子
//
//	EXCEPTIONS
//		なし

inline
Server::SessionID
Transaction::getSessionID() const
{
	return _sessionID;
}

//	FUNCTION public
//	Trans::Transaction::getID --
//		実行中のトランザクションのトランザクション識別子を得る
//
//	NOTES
//		トランザクションが実行中でないとき Trans::IllegalID を返す
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたトランザクション識別子
//
//	EXCEPTIONS
//		なし

inline
const Transaction::ID&
Transaction::getID() const
{
	return _id;
}

//	FUNCTION public
//	Trans::Transaction::getCategory -- 実行中のトランザクションの種別を得る
//
//	NOTES
//		トランザクションが実行中でないとき
//		Trans::Transaction::Category::Unknown を返す
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたトランザクションの種別
//
//	EXCEPTIONS
//		なし

inline
Transaction::Category::Value
Transaction::getCategory() const
{
	return _category;
}

//	FUNCTION public
//	Trans::Transaction::getStatus -- トランザクションの実行状況を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたトランザクションの実行状況
//
//	EXCEPTIONS
//		なし

inline
Transaction::Status::Value
Transaction::getStatus() const
{
	return _status;
}

//	FUNCTION public
//	Trans::Transaction::getIsolationLevel --
//		実行中のトランザクションのアイソレーションレベルを得る
//
//	NOTES
//		トランザクションが実行中でないとき
//		Trans::Transaction::IsolationLevel::Unknown を返す
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたトランザクションのアイソレーションレベル
//
//	EXCEPTIONS
//		なし

inline
Transaction::IsolationLevel::Value
Transaction::getIsolationLevel() const
{
	return _isoLevel;
}

//	FUNCTION public
//	Trans::Transaction::getID --
//		このトランザクションを実体とする
//		トランザクションブランチのトランザクションブランチ識別子を得る
//
//	NOTES
//		トランザクションブランチの実体として使用されていない
//		トランザクションについては Trans::XID() を返す
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたトランザクションブランチ識別子
//
//	EXCEPTIONS
//		なし

inline
const Trans::XID&
Transaction::getXID() const
{
	return _xid;
}

//	FUNCTION public
//	Trans::Transaction::getBirthTimeStamp --
//		版管理するトランザクションを開始したときのタイムスタンプを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたタイムスタンプ
//
//	EXCEPTIONS
//		なし

inline
const TimeStamp&
Transaction::getBirthTimeStamp() const
{
	return getID();
}

#ifdef OBSOLETE
//	FUNCTION public
//	Trans::Transaction::getStatementTimeStamp --
//		版管理するトランザクション中で SQL の 1 文の実行を
//		開始したときのタイムスタンプを得る
//
//	NOTES
//		アイソレーションレベルが SERIALIZABLE であれば、
//		版管理するトランザクションを開始したときのタイムスタンプを得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたタイムスタンプ
//
//	EXCEPTIONS
//		なし

inline
const TimeStamp&
Transaction::getStatementTimeStamp() const
{
	return (getIsolationLevel() == IsolationLevel::Serializable) ?
		getBirthTimeStamp() : _statementTimeStamp;
}
#endif

//	FUNCTION public
//	Trans::Transaction::getStartingList --
//		版管理するトランザクションを開始したときに実行中の
//		更新トランザクションのトランザクション識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたトランザクション識別子を要素とするベクター
//
//	EXCEPTIONS
//		なし

inline
const ModVector<Transaction::ID>&
Transaction::getStartingList() const
{
	return _startingList;
}

//	FUNCTION private
//	Trans::Transaction::isAttached -- トランザクション記述子が参照中か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			参照中である
//		false
//			参照中でない
//
//	EXCEPTIONS
//		なし

inline
bool
Transaction::isAttached() const
{
	return _refCount;
}

//	FUNCTION public
//	Trans::Transaction::isNoLock -- ロックしないトランザクションか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			ロックしないトランザクションである
//		false
//			ロックするトランザクションである
//
//	EXCEPTIONS
//		なし

inline
bool
Transaction::isNoLock() const
{
	return _noLock;
}

//	FUNCTION public
//	Trans::Transaction::isNoLog --
//		論理ログおよび UNDO ログを記録しないトランザクションか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			論理ログおよび UNDO ログを記録しないトランザクションである
//		false
//			論理ログおよび UNDO ログを記録するトランザクションである
//
//	EXCEPTIONS
//		なし

inline
bool
Transaction::isNoLog() const
{
	return _noLog;
}

//	FUNCTION public
//	Trans::Transaction::isNoVersion -- 版管理しないトランザクションか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			版管理しないトランザクションである
//		false
//			版管理するトランザクションである
//
//	EXCEPTIONS
//		なし

inline
bool
Transaction::isNoVersion() const
{
	return _noVersion;
}

//	FUNCTION public
//	Trans::Transaction::isBatchMode -- バッチモードのトランザクションか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			バッチモードのトランザクションである
//		false
//			バッチモードでないトランザクションである
//
//	EXCEPTIONS
//		なし

inline
bool
Transaction::isBatchMode() const
{
	return _batchMode;
}

//	FUNCTION public
//	Trans::Transaction::isDeterrent --
//		今後、このトランザクションの実行中に開始される
//		読取専用トランザクションに版管理を使用させないか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			使用させない
//		false
//			使用させる
//
//	EXCEPTIONS
//		なし

inline
bool
Transaction::isDeterrent() const
{
	return _deterrent;
}

//	FUNCTION public
//	Trans::Transaction::isInProgress -- トランザクションが実行中か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			トランザクションが実行中である
//		false
//			実行中でない
//
//	EXCEPTIONS
//		なし

inline
bool
Transaction::isInProgress() const
{
	//【注意】	コミット中またはロールバック中も
	//			トランザクションが実行中とみなす

	return getStatus() >= Status::InProgress;
}

//	FUNCTION public
//	Trans::Transaction::Mode::Mode --
//		トランザクションモードを表すクラスのデフォルトコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Transaction::Mode::Mode()
	: _category(Category::Unknown),
	  _isoLevel(IsolationLevel::Unknown),
	  _snapshot(Boolean::Unknown)
{}

//	FUNCTION public
//	Trans::Transaction::Mode::Mode --
//		トランザクションモードを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction::Category::Value	category
//			種別
//		Trans::Transaction::IsolationLevel::Value	isoLevel
//			アイソレーションレベル
//		Boolean::Value		snapshot
//			版管理を必ず使用するかどうか
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Transaction::Mode::Mode(
	Category::Value category,
	IsolationLevel::Value isoLevel, Boolean::Value snapshot)
	: _category(category),
	  _isoLevel(isoLevel),
	  _snapshot(snapshot)
{}

//	FUNCTION public
//	Trans::Transaction::Mode::Mode --
//		トランザクションモードを表すクラスのデストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Transaction::Mode::~Mode()
{}

_SYDNEY_TRANS_END
_SYDNEY_END

#endif	// __SYDNEY_TRANS_TRANSACTION_H

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
