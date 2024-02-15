// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogFile.h --	論理ログファイル情報関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2009, 2010, 2014, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_TRANS_LOGFILE_H
#define	__SYDNEY_TRANS_LOGFILE_H

#include "Trans/Module.h"
#include "Trans/TimeStamp.h"

#include "Lock/Name.h"
#include "LogicalLog/File.h"
#include "LogicalLog/LSN.h"

#include "Common/Thread.h"

#include "Os/Event.h"
#include "Os/CriticalSection.h"

#include "Schema/ObjectID.h"

#include "ModPair.h"
#include "ModUnicodeString.h"
#include "ModVector.h"
#include "ModMap.h"

template <class T>
class ModAutoPointer;

_SYDNEY_BEGIN

namespace Communication
{
	class Connection;
}

_SYDNEY_TRANS_BEGIN

namespace Manager
{
	namespace Log
	{
		class File;
	}
}

_SYDNEY_TRANS_LOG_BEGIN

class Data;

//	TYPEDEF
//	Trans::Log::LSN --
//		論理ログファイルのログシーケンシャル番号を表す型
//
//	NOTES

typedef	LogicalLog::LSN		LSN;

//	CONST
//	Trans::Log::IllegalLSN --
//		論理ログファイルの不正なログシーケンシャル番号を表す値
//
//	NOTES

const LSN					IllegalLSN = LogicalLog::NoLSN;

//	CLASS
//	Trans::LogicalLogFile --
//		Microsoft C/C++ Compiler のバグを回避するためのクラス
//
//	NOTES
//		Trans::Log::File が LogicalLog::File を直接継承したとき、
//		Trans::Log::File::operator = を呼び出すと、
//		Microsoft C/C++ Compiler Version 12.00.8804 では、
//		Trans::Log::File::operator = を
//		無限に呼び出すコードが生成される

class LogicalLogFile
	: public	LogicalLog::File
{
public:
	// コンストラクター
	LogicalLogFile(const LogicalLog::File::StorageStrategy& storageStrategy)
		: LogicalLog::File(storageStrategy)
	{}
};

//	CLASS
//	Trans::LogicalLogFileStorageStrategy --
//		Microsoft C/C++ Compiler のバグを回避するためのクラス
//
//	NOTES
//		Trans::Log::File::StorageStrategy が
//		LogicalLog::File::StorageStrategy を直接継承したとき、
//		Trans::Log::File::StorageStrategy::operator = を呼び出すと、
//		Microsoft C/C++ Compiler Version 12.00.8804 では、
//		Trans::Log::File::StorageStrategy::operator = を
//		無限に呼び出すコードが生成される

struct LogicalLogFileStorageStrategy
	: public	LogicalLog::File::StorageStrategy
{};

//	CLASS
//	Trans::Log::File --
//		ある論理ログファイルに関する情報を記憶するクラス
//
//	NOTES

class File
	: public	LogicalLogFile
{
	friend class Manager::Log::File;
	friend class ModAutoPointer<File>;
public:
	//	CLASS
	//	Trans::Log::File::Category --
	//		論理ログファイルの種別を表すクラス
	//
	//	NOTES

	typedef LogicalLog::File::Category Category;

	//	CLASS
	//	Trans::Log::File::StorageStrategy --
	//		論理ログファイルのファイル格納戦略を表すクラス
	//
	//	NOTES

	struct StorageStrategy
		: public	LogicalLogFileStorageStrategy
	{
		// コンストラクター
		StorageStrategy();
		// デストラクター
		~StorageStrategy();

		// トランケート可能か
		bool					_truncatable;
	};

	//	CLASS
	//	Trans::Log::File::Queue --
	//		スレーブに転送する論理ログを格納するキュー
	//
	//	NOTES
	class Queue
	{
	public:
		// コンストラクタ
		Queue();
		// デストラクタ
		~Queue() {}

		// 参照数を１増やす
		int attach();
		// 参照数を１減らす
		int detach();

		// 論理ログを書き出す
		void pushBack(Schema::ObjectID::Value databaseid_,
					  int category_,
					  const void* p_, ModSize size_, LSN lsn_);
		// 論理ログを取り出す
		bool popFront(Schema::ObjectID::Value& databaseid_,
					  int& category_,
					  void*& p_, ModSize& size_, LSN& lsn_, int timeout_);

		// 動作中か？
		bool isActive();
		// 停止する
		void setInActive();

	private:
		struct QueueData
		{
			QueueData()
				: m_uiDatabaseID(0),
				  m_iCategory(0), m_pBuffer(0), m_uiSize(0) {}
			QueueData(Schema::ObjectID::Value dbid_, int category_,
					  void* p_, ModSize size_, LSN lsn_)
				: m_uiDatabaseID(dbid_), m_iCategory(category_),
				  m_pBuffer(p_), m_uiSize(size_), m_ulLSN(lsn_) {}

			Schema::ObjectID::Value m_uiDatabaseID;
			int m_iCategory;
			void* m_pBuffer;
			ModSize m_uiSize;
			LSN m_ulLSN;
		};
		
		// キュー
		ModVector<QueueData>	m_vecQueue;

		// 排他制御用
		Os::CriticalSection		m_cLatch;
		Os::Event				m_cEvent;

		// 参照数
		int						m_iRefCount;
		// 生死フラグ
		bool					m_bActive;
	};

	//	CLASS
	//	Trans::Log::File::MasterThread --
	//		論理ログをスレーブに転送するマスター側のスレッド
	//
	//	NOTES

	class MasterThread : public Common::Thread
	{
	public:
		// コンストラクタ
		MasterThread(Communication::Connection* pSlaveConnection_,
					 const ModUnicodeString& cSlaveHostName_,
					 int iSlavePortNumber_);
		// デストラクタ
		~MasterThread() {}

		// スレッドとして実行されるメソッド
		void runnable();

		// 論理ログキュー
		Queue*							m_pQueue;
		// スレーブ用のコネクション
		Communication::Connection*		m_pSlaveConnection;

		// スレーブのホスト名 (ログ用)
		ModUnicodeString				m_cSlaveHostName;
		// スレーブのポート番号 (ログ用)
		int								m_iSlavePortNumber;
	};

	// ある論理ログファイルに関する情報を記憶するクラスを生成する
	SYD_TRANS_FUNCTION 
	static File*
	attach(const StorageStrategy& storageStrategy,
		   const Lock::LogicalLogName& lockName,
		   const ModUnicodeString& dbName);
	// 参照数を 1 増やす
	SYD_TRANS_FUNCTION 
	File*
	attach();
	// ある論理ログファイルに関する情報を記憶するクラスを破棄する
	SYD_TRANS_FUNCTION 
	static void
	detach(File*& file, bool reserve);
	// 参照数を 1 減らす
	SYD_TRANS_FUNCTION
	void
	detach();

	// 論理ログファイルを生成する
	SYD_TRANS_FUNCTION
	void
	create();
	// 論理ログファイルをマウントする
	SYD_TRANS_FUNCTION
	void
	mount();
	// 論理ログファイルを破棄する
	SYD_TRANS_FUNCTION 
	void
	destroy();
	// 論理ログファイルのアンマウントする
	SYD_TRANS_FUNCTION 
	void
	unmount();
	// 論理ログファイルをトランケートする
	SYD_TRANS_FUNCTION 
	void
	truncate();
	// 論理ログファイルの実体である OS ファイルの絶対パス名を変更する
	SYD_TRANS_FUNCTION 
	void
	rename(const Os::Path& path);
	// 論理ログファイルをローテートする
	SYD_TRANS_FUNCTION
	void
	rotate(bool persisted);

	// 論理ログを取り出す
	SYD_TRANS_FUNCTION 
	Data*					load(LSN lsn);
	// 論理ログを記録する
	SYD_TRANS_FUNCTION 
	LSN						store(const Data& data,
								  Log::LSN masterLSN = IllegalLSN);
	// LogicalLog::File:;
	// 論理ログファイルをフラッシュする
	// void					flush(LSN lsn = IllegalLSN);

	// LogicalLog::File:;
	// 先頭の論理ログの LSN を得る
	// LSN					getFirstLSN();
	// 末尾の論理ログの LSN を得る
	// LSN					getLastLSN();
	// ある論理ログの直後のものの LSN を得る
	// LSN					getNextLSN(LSN lsn);
	// ある論理ログの直前のものの LSN を得る
	// LSN					getPrevLSN(LSN lsn);

	// 論理ログファイルを表すロック名を得る
	const Lock::LogicalLogName&	getLockName() const;
	// 論理ログファイルに記録する操作の対象であるデータベースの名前を得る
	const ModUnicodeString&	getDatabaseName() const;
	// 起動してから最後に論理ログを挿入したときのタイムスタンプを得る
	const TimeStamp&		getLastModification() const;
	// タイムスタンプファイルに書かれているタイムスタンプ値を得る
	SYD_TRANS_FUNCTION 
	TimeStamp				getTimeStamp() const;
	// タイムスタンプファイルにタイムスタンプ値を書き込む
	SYD_TRANS_FUNCTION
	void					setTimeStamp(const TimeStamp& timestamp_);

	// 現在、使用中の論理ログファイルに関する情報を得る
	SYD_TRANS_FUNCTION 
	static ModVector<File*>	getInUseList();

	// トランケート可能か
	bool					isTruncatable() const;
	// 最後に記録したデータベースの更新を表す論理ログは
	// バージョンファイルの同期を表す論理ログか
	SYD_TRANS_FUNCTION 
	bool					isSynchronized();
	// LogicalLog::File::
	// 構成する OS ファイルが存在するか調べる
	// bool					isAccessable() const;

	// 不要な論理ログを削除する
	SYD_TRANS_FUNCTION
	void					discardLog(bool isDiscardFull_);

	// 前々回のチェックポイント時のタイムスタンプを記録する
	SYD_TRANS_FUNCTION
	void					storeSecondMostRecent();

	// スレーブとの接続スレッドを作成する
	SYD_TRANS_FUNCTION
	static void
	startReplication(const ModUnicodeString& cHostName_,
					 int iPortNumber_,
					 Communication::Connection* pConnection_);
	// スレーブのキューを得る
	SYD_TRANS_FUNCTION
	static Queue*		getQueue(const ModUnicodeString& cHostName_,
								 int iPortNumber_);
	

	// スレーブのキューを設定する
	SYD_TRANS_FUNCTION
	void				setQueue(const ModUnicodeString& cHostName_,
								 int iPortNumber_,
								 Queue* pQueue_);
	// 指定されたスレーブデータベースへの転送を停止する
	SYD_TRANS_FUNCTION
	void
	stopTransferLog(const ModUnicodeString& cHostName_,
					int iPortNumber_);

private:
	// コンストラクター
	File(const StorageStrategy& storageStrategy,
		 const Lock::LogicalLogName& lockName, const ModUnicodeString& dbName);
	// デストラクター
	~File();

	// クラスが参照されているか
	bool					isAttached() const;
	// 論理ログをスレーブ用のキューに書き込む
	void					pushSlaveQueue(int category_,
										   void* p_, ModSize size_, LSN lsn_);

	// 論理ログファイルを表すロック名
	const Lock::LogicalLogName _lockName;
	// 論理ログファイルが記録する操作の対象であるデータベースの名前
	const ModUnicodeString	_dbName;
	// チェックポイント処理の終了時に可能であればトランケートするか
	const bool				_truncatable;
	// 最後に記録したデータベースの更新を表す論理ログは
	// バージョンファイルの同期を表す論理ログか
	Boolean::Value			_synchronized;
	// 参照回数
	mutable unsigned int	_refCount;
	// 最後に論理ログを挿入したときのタイムスタンプ
	TimeStamp				_lastModification;
	// 最後に更新操作を表す論理ログがフラッシュされたときのタイムスタンプ
	TimeStamp				_lastFlush;

	// 前回のチェックポイント時のログシーケンス番号
	LSN						_firstLSN;
	// 前々回のチェックポイント時のログシーケンス番号
	LSN						_secondLSN;

	//【注意】	以下のメンバーは、
	//			ハッシュリストのラッチにより保護される

	// 格納されているハッシュリストのバケットアドレス
	unsigned int			_hashAddr;
	// ハッシュリストでの直前の要素へのポインタ
	File*					_hashPrev;
	// ハッシュリストでの直後の要素へのポインタ
	File*					_hashNext;

	typedef ModPair<ModUnicodeString, int>	QueueKey;
	typedef ModMap<QueueKey, Queue*, ModLess<QueueKey> >	QueueMap;
	
	// スレーブ用のキュー
	QueueMap				m_cQueueMap;
	// 上記リソースの排他制御用
	Os::CriticalSection		m_cLatch;
};

//	FUNCTION private
//	Trans::Log::File::File --
//		ある論理ログファイルに関する情報を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::File::StorageStrategy&	storageStrategy
//			操作する論理ログファイルのファイル格納戦略
//		Lock::LogicalLogName&	lockName
//			操作する論理ログファイルを表すロック名
//		ModUnicodeString&	dbName
//			操作する論理ログファイルに記録する操作の対象である
//			データベースの名前
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
File::File(const StorageStrategy& storageStrategy,
		   const Lock::LogicalLogName& lockName,
		   const ModUnicodeString& dbName)
	: LogicalLogFile(storageStrategy),
	  _lockName(lockName),
	  _dbName(dbName),
	  _truncatable(storageStrategy._truncatable),
	  _synchronized(Boolean::Unknown),
	  _refCount(0),
	  _firstLSN(Log::IllegalLSN),
	  _secondLSN(Log::IllegalLSN),
	  _hashAddr(0),
	  _hashPrev(0),
	  _hashNext(0)
{}

//	FUNCTION private
//	Trans::Log::File::isAttached --
//		ある論理ログファイルに関する情報を表すクラスが参照中か調べる
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
File::isAttached() const
{
	return _refCount;
}

//	FUNCTION public
//	Trans::Log::File::getLockName -- 論理ログファイルを表すロック名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたロック名
//
//	EXCEPTIONS
//		なし

inline
const Lock::LogicalLogName&
File::getLockName() const
{
	return _lockName;
}

//	FUNCTION public
//	Trans::Log::File::getDatabaseName --
//		論理ログファイルに記録する操作の対象であるデータベースの名前を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたデータベースの名前
//
//	EXCEPTIONS
//		なし

inline
const ModUnicodeString&
File::getDatabaseName() const
{
	return _dbName;
}

//	FUNCTION public
//	Trans::Log::File::getLastModification --
//		システムを起動してから、論理ログファイルに対して
//		最後に論理ログを記録したときのタイムスタンプを得る
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
File::getLastModification() const
{
	return _lastModification;
}

//	FUNCTION public
//	Trans::Log::File::isTruncatable -- 論理ログファイルがトランケート可能か
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			トランケート可能である
//		false
//			トランケート不可である
//
//	EXCEPTIONS
//		なし

inline
bool
File::isTruncatable() const
{
	return _truncatable && isMounted() && !isReadOnly();
}

//	FUNCTION public
//	Trans::Log::File::StorageStrategy::StorageStrategy --
//		ファイル格納戦略を表すクラスのコンストラクター
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
File::StorageStrategy::StorageStrategy()
	: _truncatable(false)
{}

//	FUNCTION public
//	Trans::Log::File::StorageStrategy::~StorageStrategy --
//		ファイル格納戦略を表すクラスのデストラクター
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
File::StorageStrategy::~StorageStrategy()
{}

_SYDNEY_TRANS_LOG_END
_SYDNEY_TRANS_END
_SYDNEY_END

#endif	// __SYDNEY_TRANS_LOGFILE_H

//
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2009, 2010, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
