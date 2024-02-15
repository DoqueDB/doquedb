// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Transaction.cpp -- トランザクション記述子関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2014, 2015, 2016, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Trans";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Trans/AutoLatch.h"
#include "Trans/Configuration.h"
#include "Trans/Transaction.h"
#include "Trans/TransactionInformation.h"
#include "Trans/List.h"
#include "Trans/LogInfo.h"
#include "Trans/LogData.h"
#include "Trans/Manager.h"

#include "Admin/EndBackup.h"
#include "Admin/Transaction.h"
#include "Checkpoint/Database.h"
#include "Checkpoint/Executor.h"
#include "Checkpoint/TimeStamp.h"
#include "Common/Assert.h"
#include "Common/DoubleLinkedList.h"
#include "Common/HashTable.h"
#include "Common/Thread.h"
#include "Communication/Connection.h"
#include "DServer/Branch.h"
#include "Exception/AlreadyBeginTransaction.h"
#include "Exception/Cancel.h"
#include "Exception/Deadlock.h"
#include "Exception/LackForChild.h"
#include "Exception/LackOfParent.h"
#include "Exception/LockTimeout.h"
#include "Exception/LogItemCorrupted.h"
#include "Exception/NotBeginTransaction.h"
#include "Exception/OtherDatabaseAlreadyModified.h"
#include "Exception/XA_InsideActiveBranch.h"
#include "Lock/AutoItem.h"
#include "Lock/Name.h"
#include "Lock/Status.h"
#include "Os/AutoCriticalSection.h"
#include "Schema/Database.h"
#include "Schema/Manager.h"
#include "Schema/ObjectID.h"
#include "Schema/Table.h"

#include "ModAlgorithm.h"
#include "ModAutoPointer.h"

#ifdef SYD_FAKE_ERROR
#include "Exception/FakeError.h"
#include "Exception/Unexpected.h"

#define _TRANS_FAKE_ERROR(func) \
{ \
	_TRMEISTER_FAKE_ERROR("Trans::" #func, Exception::Unexpected(moduleName, srcFile, __LINE__)); \
}
#else
#define _TRANS_FAKE_ERROR(func)
#endif

_SYDNEY_USING
_SYDNEY_TRANS_USING

namespace
{

typedef	Common::HashTable<
	Common::DoubleLinkedList<Transaction>, Transaction>	HashTable;

typedef Common::HashTable<Common::DoubleLinkedList<TransactionInformation>,
						  TransactionInformation> DatabaseTable;

namespace _Transaction
{
	// すべてのトランザクション記述子を管理する
	// ハッシュ表に登録するためのハッシュ値を計算する
	unsigned int
	transTableHash(Server::SessionID sessionID);

	// データベースごとのトランザクション記述子を管理する
	// ハッシュ表に登録するためのハッシュ値を計算する
	unsigned int
	databaseTableHash(Schema::ObjectID::Value dbID);

	// データベースごとのトランザクション記述子を管理するクラスを得る
	TransactionInformation*
	getTransactionInformation(Schema::ObjectID::Value dbID);

	// 版管理するトランザクションのために、スキーマデータベースをロックする
	bool
	lockSchema(Transaction& trans,
			   Lock::Timeout::Value timeout = Lock::Timeout::Unlimited);

	// トランザクションのための論理ログを記録する
	void
	storeLog(Transaction& trans, const Log::Data& data,
			 Log::LSN masterLSN = Log::IllegalLSN);

	// 開始するトランザクションが版管理を使用しないか判断する
	bool
	isNoVersion(const Transaction& trans, const Transaction::Mode& mode);

	//	MACRO local
	//	_CHECK_LOCK_STATUS --
	//		ロック行為の結果を見て、適切な例外を投げる
	//
	//	NOTES
	//		Lock::Status::Value に列挙子が増えた場合、本メソッドもメンテが必要
	//
	//	ARGUMENTS
	//		Lock::Status::Value		status
	//			調べるロック行為の結果を表す値
	//
	//	RETURN
	//		なし
	//
	//	EXCEPTIONS

#define _CHECK_LOCK_STATUS(_status_) \
{ \
	switch (_status_) { \
	case Lock::Status::Succeeded:		break; \
	case Lock::Status::Timeout:			_SYDNEY_THROW0(Exception::LockTimeout); \
	case Lock::Status::Deadlock:		_SYDNEY_THROW0(Exception::Deadlock); \
	case Lock::Status::LackOfParent:	_SYDNEY_THROW0(Exception::LackOfParent); \
	case Lock::Status::LackForChild:	_SYDNEY_THROW0(Exception::LackForChild); \
	} \
}

	// 以下の情報を保護するためのラッチ
	Os::CriticalSection		_latch;

	// すべてのトランザクション記述子を管理するハッシュ表
	HashTable*				_transTable = 0;

	// 以下の情報を保護するためのラッチ
	Os::CriticalSection		_latch2;

	// データベースごとのトランザクション記述子情報を管理するハッシュ表
	DatabaseTable*			_databaseTable = 0;

	// Os::Timer の Auto 変数
	class AutoTimer
	{
	public:
		AutoTimer(Os::Timer& cTimer_)
			: m_cTimer(cTimer_)
		{
			m_cTimer.start();
		}
		
		~AutoTimer()
		{
			m_cTimer.end();
		}

	private:
		Os::Timer& m_cTimer;
	};
}

namespace _List
{
	// あるトランザクション識別子が指定されたリストに登録されているか調べる
	bool
	exists(const List<Transaction>& list, const ID& id);
	bool
	exists(const List<Transaction>& list, const ModVector<ID>& ids);
}

namespace _AdequateLock
{
	//	STRUCT
	//	$$$::_Transaction::_Info --
	//		ある条件時にかけるべきロックに関する情報を表すクラス
	//
	//	NOTES

	struct _Info
	{
		// ロックモード
		Lock::Mode::Value		_mode;
		// ロックの持続期間
		Lock::Duration::Value	_duration;
	};

#define	IS		Lock::Mode::IS
#define	IX		Lock::Mode::IX
#define	S		Lock::Mode::S
#define	SIX		Lock::Mode::SIX
#define	U		Lock::Mode::U
#define	X		Lock::Mode::X
#define VS		Lock::Mode::VS
#define	VX		Lock::Mode::VX
#define	VIX		Lock::Mode::VIX
#define	N		Lock::Mode::N

#define	Instant	Lock::Duration::Instant
#define	Stmt	Lock::Duration::Statement
#define	Short	Lock::Duration::Short
#define	Middle	Lock::Duration::Middle
#define	Long	Lock::Duration::Long

// 版管理するトランザクションがかける適切なロックを得るための配列
const _Info	infoForVersioningTransaction
	[Transaction::IsolationLevel::ValueNum][Lock::Name::Category::ValueNum] =
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// ReadUncommitted
{{ N, Instant },{ VS,Stmt    },{ N, Instant },{ N, Instant },{ N, Instant }},
															// ReadCommitted
{{ N, Instant },{ VS,Middle  },{ N, Instant },{ N, Instant },{ N, Instant }},
															// RepeatableRead
{{ N, Instant },{ VS,Middle  },{ N, Instant },{ N, Instant },{ N, Instant }}
															// Serializable
};

// 版管理しないトランザクションがかける適切なロックを得るための配列
const _Info infoForNoVersionTransaction
	[][Lock::Name::Category::ValueNum]
	[Transaction::IsolationLevel::ValueNum][Lock::Name::Category::ValueNum] =
{

// for Read Write operation
{
// Unknown 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Database 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ X, Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ X, Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ X, Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ X, Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Table 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ IX,Middle  },{ X, Middle  },{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ IX,Middle  },{ X, Middle  },{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ IX,Middle  },{ X, Middle  },{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ X, Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Tuple 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ IX,Middle  },{ IX,Middle  },{ X, Middle  },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ IX,Middle  },{ IX,Middle  },{ X, Middle  },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ IX,Middle  },{ IX,Middle  },{ X, Middle  },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ IX,Middle  },{ X, Middle  },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// LogicalLog 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }}
															// Serializable
}
},

// for Read Only operation
{
// Unknown 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IS,Stmt    }}
															// Serializable
},

// Database 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ S, Short   },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ S, Middle  },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ S, Middle  },{ N, Instant },{ N, Instant },{ IS,Stmt    }}
															// Serializable
},

// Table 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ IS,Middle  },{ N, Instant },{ N, Instant },{ IS,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ IS,Middle  },{ S, Short   },{ N, Instant },{ IS,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ IS,Middle  },{ S, Middle  },{ N, Instant },{ IS,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ S, Middle  },{ N, Instant },{ N, Instant },{ IS,Stmt    }}
															// Serializable
},

// Tuple 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ IS,Middle  },{ IS,Middle  },{ N, Instant },{ IS,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ IS,Middle  },{ IS,Middle  },{ S, Short   },{ IS,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ IS,Middle  },{ IS,Middle  },{ S, Middle  },{ IS,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ IS,Middle  },{ S, Middle  },{ N, Instant },{ IS,Stmt    }}
															// Serializable
},

// LogicalLog 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ S, Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ S, Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ S, Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ S, Stmt    }}
															// Serializable
}
},

// for Batch mode operation
{
// Unknown 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Database 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ X, Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ X, Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ X, Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ X, Middle  },{ N, Instant },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Table 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ X, Middle  },{ N, Middle  },{ N, Instant },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ X, Middle  },{ N, Middle  },{ N, Instant },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ X, Middle  },{ N, Middle  },{ N, Instant },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ X, Middle  },{ N, Middle  },{ N, Instant },{ IX,Stmt    }}
															// Serializable
},

// Tuple 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ X, Middle  },{ N, Middle  },{ N, Middle  },{ IX,Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ X, Middle  },{ N, Middle  },{ N, Middle  },{ IX,Stmt    }},
															// ReadCommitted
{{ N, Instant },{ X, Middle  },{ N, Middle  },{ N, Middle  },{ IX,Stmt    }},
															// RepeatableRead
{{ N, Instant },{ X, Middle  },{ N, Middle  },{ N, Middle  },{ IX,Stmt    }}
															// Serializable
},

// LogicalLog 操作時
{
// Unknown        Database       Table          Tuple          LogicalLog

{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant }},
															// Unknown
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// ReadUncommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// ReadCommitted
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }},
															// RepeatableRead
{{ N, Instant },{ N, Instant },{ N, Instant },{ N, Instant },{ X, Stmt    }}
															// Serializable
}
}
};

#undef	IS
#undef	IX
#undef	S
#undef	SIX
#undef	X
#undef	VS
#undef	VX
#undef	VIX
#undef	N

#undef	Instant
#undef	Short
#undef	Middle
#undef	Long

}

//	FUNCTION
//	$$$::_Transaction::transTableHash --
//		すべてのトランザクション記述子を管理するハッシュ表に
//		登録するためのハッシュ値を計算する
//
//	NOTES
//
//	ARGUMENTS
//		Server::SessionID	sessionID
//			ハッシュ表に登録するトランザクション記述子を
//			生成したセッションのセッション識別子
//
//	RETURN
//		得られたハッシュ値
//
//	EXCEPTIONS
//		なし

inline
unsigned int
_Transaction::transTableHash(Server::SessionID sessionID)
{
	return sessionID;
}

//	FUNCTION
//	$$$::_Transaction::databaseTableHash --
//		データベースごとのトランザクション記述子を管理するハッシュ表に
//		登録するためのハッシュ値を計算する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectID::Value
//			ハッシュ表に登録するトランザクション記述子が操作する
//			データベースのデータベースID
//
//	RETURN
//		得られたハッシュ値
//
//	EXCEPTIONS
//		なし

inline
unsigned int
_Transaction::databaseTableHash(Schema::ObjectID::Value dbID)
{
	return dbID;
}

//	FUNCTION public
//	$$$::_Transaction::getTransactionInformation --
//		データベースごとのトランザクション記述子を管理するクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectID::Value databaseID
//			データベースID
//
//	RETURN
//		データベースごとのトランザクション記述子を管理するクラス
//
//	EXCEPTIONS
//		なし

inline
TransactionInformation*
_Transaction::getTransactionInformation(Schema::ObjectID::Value dbID)
{
	Os::AutoCriticalSection cAuto(_latch2);
	
	unsigned int addr =
		_Transaction::databaseTableHash(dbID) %
			_Transaction::_databaseTable->getLength();
	DatabaseTable::Bucket& bucket
		= _Transaction::_databaseTable->getBucket(addr);

	if (bucket.getSize())
	{
		// バケットに登録されているデータベース情報のうち、
		// 与えられたデータベースIDを持つものを探す

		DatabaseTable::Bucket::Iterator			begin(bucket.begin());
		DatabaseTable::Bucket::Iterator			ite(begin);
		const DatabaseTable::Bucket::Iterator&	end = bucket.end();

		do
		{
			TransactionInformation& info = *ite;
			if (info.getDatabaseID() == dbID)
			{
				// 見つかったデータベース情報を
				// バケットの先頭に移動して、
				// 最近に参照されたものほど、見つけやすくする

				bucket.splice(begin, bucket, ite);

				return &info;
			}
		}
		while (++ite != end);
	}

	// 見つからなかったの新たに生成する

	TransactionInformation* info = new TransactionInformation(dbID);

	// ハッシュ表のバケットの先頭に挿入して、
	// 最近に参照されたものほど、見つかりやすくする

	bucket.pushFront(*info);

	return info;
}
	
//	FUNCTION
//	$$$::_Transaction::lockSchema --
//		版管理するトランザクションのためにスキーマデータベースをロックする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			スキーマデータベースをロックする
//			版管理するトランザクションのトランザクション記述子
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

bool
_Transaction::lockSchema(Transaction& trans,
						 Lock::Timeout::Value timeout)
{
	// 版管理するトランザクションである

	; _SYDNEY_ASSERT(!trans.isNoVersion());

	// かけるロックのモードと持続期間を得る

	Lock::Mode::Value		mode;
	Lock::Duration::Value	duration;

	trans.getAdequateLock(
		Lock::Name::Category::Database, Lock::Name::Category::Database,
		trans.getCategory() == Transaction::Category::ReadOnly,
		mode, duration);

	if (timeout == Lock::Timeout::Unlimited) {

		// 無制限にロック待ちするときは、duration は Instant にする

		duration = Lock::Duration::Instant;
	}

	// スキーマデータベースを求めたモードと持続期間でロックする

	return trans.lock(Lock::DatabaseName(static_cast<Lock::Name::Part>(
											 Schema::ObjectID::SystemTable)),
					  mode, duration, timeout);
}

//	FUNCTION
//	$$$::_Transaction::storeLog --
//		トランザクションのための論理ログを記録する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			このトランザクション記述子の表す
//			トランザクションのための論理ログを記録する
//		Trans::Log::Data&	data
//			記録する論理ログのログデータ
//		Trans::Log::LSN		masterLSN
//			マスター側のLSN
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_Transaction::storeLog(Trans::Transaction& trans, const Log::Data& data,
					   Log::LSN masterLSN)
{
	if (!Checkpoint::Database::isAvailable(Schema::ObjectID::SystemTable))

		// スキーマデータベースが利用不可のとき、
		// トランザクションは論理ログを記録できない

		return;

	// トランザクションが操作する通常のデータベースの
	// スキーマオブジェクト識別子を求める

	const Log::Info& logInfo = trans.getLogInfo(Log::File::Category::Database);

	if (!Checkpoint::Database::isAvailable(logInfo.getDatabaseID()))

		// トランザクションが操作する通常のデータベースが利用不可のとき、
		// トランザクションは論理ログを記録できない

		return;

	// システム用の論理ログファイルに記録する

	try {
		AutoLatch latch(trans, trans.getLogInfo(
							Log::File::Category::System).getLockName());
		(void) trans.storeLog(Log::File::Category::System, data);

	} catch (...) {

		// スキーマデータベースが利用不可になった

		(void) Checkpoint::Database::setAvailability(
			Schema::ObjectID::SystemTable, false);
		_SYDNEY_RETHROW;
	}

	// 通常のデータベース用の論理ログファイルに記録する

	try {
		AutoLatch latch(trans, logInfo.getLockName());
		(void) trans.storeLog(Log::File::Category::Database, data, masterLSN);

	} catch (...) {

		// 通常のデータベースはアンマウントまたは破棄されていないので、
		// 利用不可になったとみなす

		(void) Checkpoint::Database::setAvailability(
			logInfo.getDatabaseID(), false);
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION
//	$$$::_Transaction::isNoVersion --
//		開始するトランザクションが版管理を使用しないか判断する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&		trans
//			版管理を使用するか判断するトランザクションのトランザクション記述子
//		Trans::Transaction::Mode&	mode
//			版管理を使用するか判断するトランザクションのモード
//
//	RETURN
//		true
//			版管理を使用しない
//		false
//			版管理を使用する
//
//	EXCEPTIONS

bool
_Transaction::isNoVersion(
	const Transaction& trans, const Transaction::Mode& mode)
{
	if (Configuration::NoVersion::get() ||
		trans.getCategory() == Transaction::Category::ReadWrite)
		return true;

	if (((mode._snapshot != Boolean::Unknown) ?
		 mode._snapshot : trans.getDefaultMode()._snapshot) == Boolean::True)

		// 読取専用トランザクションを開始するとき、
		// USING SNAPSHOT オプションが指定されていれば、
		// 版管理を使用する

		return false;

	if (trans.getDatabaseID() == Schema::ObjectID::Invalid)

		// データベースを限定できないトランザクションでは、
		// 版管理は使用できない

		return true;

	const List<Transaction>& list =
		trans.getInProgressList(trans.getDatabaseID(),
								Transaction::Category::ReadWrite);

	Os::AutoCriticalSection latch(list.getLatch());

	if (list.getSize()) {

		// 開始時に実行中の更新トランザクションのうち、
		// 版管理を抑止するものがあれば、版管理を使用しない

		List<Transaction>::ConstIterator		ite(list.begin());
		const List<Transaction>::ConstIterator&	end = list.end();

		do {
			if ((*ite).isDeterrent())
				return true;
		} while (++ite != end) ;
	}

	return false;
}

//	FUNCTION
//	$$$::_List::exists --
//		トランザクション記述子が要素として登録されているリスト中に
//		指定されたトランザクション識別子の表すトランザクションの
//		トランザクション記述子が登録されているか
//
//	NOTES
//
//	ARGUMENTS
//		Trans::List<Transaction>&	list
//			トランザクション記述子を要素とするリストで、
//			要素であるトランザクション記述子は、それぞれの表す
//			トランザクションの開始時刻の昇順に並んでいる必要がある
//		Trans::Transaction::ID&	id
//			このトランザクション識別子の表すトランザクションの
//			トランザクション記述子が list に登録されているか調べる
//
//	RETURN
//		true
//			登録されている
//		false
//			登録されていない
//
//	EXCEPTIONS

bool
_List::exists(const List<Transaction>& list, const ID& id)
{
	Os::AutoCriticalSection	latch(list.getLatch());

	if (list.getSize()) {
		List<Transaction>::ConstIterator		ite(list.begin());
		const List<Transaction>::ConstIterator&	end = list.end();
		
		do {
			if ((*ite).getID() >= id)
				return (*ite).getID() == id;
		} while (++ite != end) ;
	}

	return false;
}

//	FUNCTION
//	$$$::_List::exists --
//		トランザクション識別子が要素として登録されているリスト中に
//		指定された複数のトランザクション識別子のいずれかが登録されているか
//
//	NOTES
//
//	ARGUMENTS
//		Trans::List<Transaction>&	list
//			トランザクション記述子を要素とするリストで、
//			要素であるトランザクション記述子は、それぞれの表す
//			トランザクションの開始時刻の昇順に並んでいる必要がある
//		ModVector<Trans::Transaction::ID>&	ids
//			複数のトランザクション識別子を要素するリストで、
//			いずれかの表すトランザクションの
//			トランザクション記述子が list に登録されているか調べる
//			要素であるトランザクション識別子は、それぞれの表す
//			トランザクションの開始時刻の昇順に並んでいる必要がある
//			
//	RETURN
//		true
//			登録されている
//		false
//			登録されていない
//
//	EXCEPTIONS

bool
_List::exists(const List<Transaction>& list, const ModVector<ID>& ids)
{
	if (ids.getSize()) {
		Os::AutoCriticalSection	latch(list.getLatch());

		if (list.getSize()) {
			List<Transaction>::ConstIterator		ite0(list.begin());
			const List<Transaction>::ConstIterator&	end0 = list.end();
			ModVector<ID>::ConstIterator			ite1(ids.begin());
			const ModVector<ID>::ConstIterator&		end1 = ids.end();

			do {
				if ((*ite0).getID() == *ite1)
					return true;

			} while (((*ite0).getID() < *ite1) ?
					 (++ite0 != end0) : (++ite1 != end1)) ;
		}
	}

	return false;
}

}

//	FUNCTION private
//	Trans::Manager::Transaction::initialize --
//		マネージャーの初期化のうち、トランザクション記述子関連のものを行う
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

// static
void
Manager::Transaction::initialize()
{
	try {
		// すべてのトランザクション記述子を管理するハッシュ表を生成する

		_Transaction::_transTable =
			new HashTable(Configuration::TransTableSize::get(),
						  &Trans::Transaction::_hashPrev,
						  &Trans::Transaction::_hashNext);
		; _SYDNEY_ASSERT(_Transaction::_transTable);

		// データベースごとのトランザクション記述子を管理する
		// ハッシュ表を生成する

		_Transaction::_databaseTable =
			new DatabaseTable(Configuration::InfoTableSize::get(),
							  &Trans::TransactionInformation::m_pPrev,
							  &Trans::TransactionInformation::m_pNext);
		; _SYDNEY_ASSERT(_Transaction::_databaseTable);
		
	} catch (...) {

		Manager::Transaction::terminate();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Trans::Manager::Transaction::terminate --
//		マネージャーの後処理のうち、トランザクション記述子関連のものを行う
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

// static
void
Manager::Transaction::terminate()
{
	//【注意】	他のスレッドが同時に実行されることはないので、
	//			ラッチしない
	
	if (_Transaction::_databaseTable)
	{
		unsigned int i = 0;
		unsigned int length = _Transaction::_databaseTable->getLength();

		for (; i < length; ++i)
		{
			DatabaseTable::Bucket& bucket
				= _Transaction::_databaseTable->getBucket(i);
			if (bucket.getSize())
			{
				DatabaseTable::Bucket::Iterator	ite(bucket.begin());
				
				do
				{
					TransactionInformation& info = *ite;
					++ite;

					bucket.erase(info);
					delete &info;
				}
				while (bucket.getSize());
			}
		}

		// データベースごとのトランザクション記述子を管理する
		// ハッシュ表を破棄する

		delete _Transaction::_databaseTable, _Transaction::_databaseTable = 0;
	}
					
	if (_Transaction::_transTable) {

		// すべてのトランザクション記述子を管理する
		// ハッシュ表のバケットは空であるべき

		; _SYDNEY_ASSERT(_Transaction::_transTable->isEmpty());

		// すべてのトランザクション記述子を管理するハッシュ表を破棄する

		delete _Transaction::_transTable, _Transaction::_transTable = 0;
	}
}

//	FUNCTION private
//	Trans::Transaction::Transaction --
//		トランザクション記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Server::SessionID	sessionID
//			トランザクション記述子を生成するセッションのセッション識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Transaction::Transaction(Server::SessionID sessionID)
	: _sessionID(sessionID)
{
	construct();
}

//	FUNCTION private
//	Trans::Transaction::Transaction --
//		トランザクション記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::XID&			xid
//			生成するトランザクション記述子の表すトランザクションを実体とする
//			トランザクションブランチのトランザクションブランチ識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Transaction::Transaction(const XID& xid)
	: _sessionID(Server::IllegalSessionID),
	  _xid(xid)
{
	construct();
}

//	FUNCTION private
//	Trans::Transaction::construct --
//		トランザクション記述子を表すクラスのコンストラクター下位関数
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

void
Transaction::construct()
{
	_refCount = 0;
	_category = Category::Unknown;
	_status = Status::Initialized;
	_isoLevel = IsolationLevel::Unknown;
	_noLock = false;
	_noLog = false;
	_noVersion = false;
	_deterrent = false;
	_connection = 0;
	_batchMode = false;
	_updatedSameVersion = false;
	_databaseID = Schema::ObjectID::Invalid;
	_hashPrev = 0;
	_hashNext = 0;
	_categoryPrev = 0;
	_categoryNext = 0;
	_versioningPrev = 0;
	_versioningNext = 0;
	m_iSendRowCount = 0;
	m_iPageReferenceCount = 0;
	m_iPageReadCount = 0;
	m_iLockCount = 0;
	m_eType = Type::Unknown;
	m_bStoreLog = true;

	// このトランザクションに関する
	// システム用の論理ログファイルに関する情報を初期化する

	Schema::Database* database =
		Schema::Database::get(Schema::ObjectID::SystemTable, *this);
	; _SYDNEY_ASSERT(database);
	_systemLogInfo = Log::Info(*database);
}

//	FUNCTION public
//	Trans::Transaction::attach -- トランザクション記述子を得る
//
//	NOTES
//
//	ARGUMENTS
//		Server::SessionID	sessionID
//			Server::IllegalSessionID 以外の値
//				トランザクション記述子を取得する
//				セッションのセッション識別子で、
//				セッションごとにひとつしか生成されない
//			Server::IllegalSessionID または指定されないとき
//				セッションに関連づかないトランザクション記述子を取得する
//
//	RETURN
//		得られたトランザクション記述子を格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Transaction*
Transaction::attach(Server::SessionID sessionID)
{
	// トランザクション記述子の生成・破棄に関する情報を
	// 保護するためのラッチをかける

	Os::AutoCriticalSection latch(_Transaction::_latch);

	if (sessionID == Server::IllegalSessionID) {

		// 無効なセッション ID が与えられたので、
		// ハッシュ表で管理しないトランザクション記述子を生成する

		Transaction* trans = new Transaction(Server::IllegalSessionID);
		; _SYDNEY_ASSERT(trans);

		// 参照回数を 1 にする

		trans->_refCount = 1;

		return trans;
	}

	// 有効なセッション ID が与えられた

	// 与えられたセッション識別子のセッションが
	// 生成したトランザクション記述子を
	// 格納すべきハッシュ表のバケットを求める

	unsigned int addr =
		_Transaction::transTableHash(sessionID) %
			_Transaction::_transTable->getLength();
	HashTable::Bucket& bucket = _Transaction::_transTable->getBucket(addr);

	switch (bucket.getSize()) {
	default:
	{
		// バケットに登録されているトランザクション記述子のうち、
		// 与えられたセッション識別子のセッションが生成したものを探す

		HashTable::Bucket::Iterator			begin(bucket.begin());
		HashTable::Bucket::Iterator			ite(begin);
		const HashTable::Bucket::Iterator&	end = bucket.end();

		do {
			Transaction& trans = *ite;
			if (trans.getSessionID() == sessionID) {

				// 見つかったので、参照回数を 1 増やす

				++trans._refCount;

				// 見つかったトランザクション記述子を
				// バケットの先頭に移動して、
				// 最近に参照されたものほど、見つかりやすくする

				bucket.splice(begin, bucket, ite);

				return &trans;
			}
		} while (++ite != end) ;

		break;
	}
	case 1:
	{
		Transaction& trans = bucket.getFront();
		if (trans.getSessionID() == sessionID) {
			++trans._refCount;
			return &trans;
		}
		break;
	}
	case 0:
		break;
	}

	// 見つからなかったので、生成する

	Transaction* trans = new Transaction(sessionID);
	; _SYDNEY_ASSERT(trans);

	// 参照回数を 1 にする

	trans->_refCount = 1;

	// ハッシュ表のバケットの先頭に挿入して、
	// 最近に参照されたものほど、見つかりやすくする

	bucket.pushFront(*trans);

	return trans;
}

//	FUNCTION public
//	Trans::Transaction::attach --
//		トランザクション記述子の参照回数を 1 増やす
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			参照回数を 1 増やすトランザクション記述子
//
//	RETURN
//		参照回数が 1 増えたトランザクション記述子を格納する領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

// static
Transaction*
Transaction::attach(const Transaction& trans)
{
	// トランザクション記述子の生成・破棄に関する情報を
	// 保護するためのラッチをかける

	Os::AutoCriticalSection latch(_Transaction::_latch);

	// 参照回数を 1 増やす

	++trans._refCount;

	return &const_cast<Transaction&>(trans);
}

//	FUNCTION public
//	Trans::Transaction::detach -- トランザクション記述子の参照をやめる
//
//	NOTES
//		トランザクション記述子の参照をやめても、
//		他のどこかで参照されているか、トランザクションが終了していなければ、
//		トランザクション記述子は破棄されない
//		逆にどこからも参照されておらず、トランザクションが実行中でなければ、
//		トランザクション記述子は直ちに破棄される
//
//	ARGUMENTS
//		Trans::Transaction*&	trans
//			参照をやめるトランザクション記述子を格納する領域の先頭アドレス
//			呼び出しから返ると 0 になる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Transaction::detach(const Transaction*& trans)
{
	if (trans) {

		// トランザクション記述子の生成・破棄に関する情報を
		// 保護するためのラッチをかける

		Os::AutoCriticalSection latch(_Transaction::_latch);

		// 参照回数を 1 減らす

		--trans->_refCount;

		// 必ずどこからか参照されていなければならない

		; _SYDNEY_ASSERT(trans->isAttached());

		// 与えられたポインタは 0 を指すようにする

		trans = 0;
	}
}

// static
void
Transaction::detach(Transaction*& trans)
{
	if (trans) {

		// トランザクション記述子の生成・破棄に関する情報を
		// 保護するためのラッチをかける

		Os::AutoCriticalSection latch(_Transaction::_latch);

		if (trans->_refCount > 1)

			// 参照回数を 1 減らす

			--trans->_refCount;
		else {

			// どこからも参照されなくなる

			if (trans->isInProgress())

				// 実行中のトランザクションを表す
				// トランザクション記述子を実際に破棄しようとしている

				_SYDNEY_THROW0(Exception::AlreadyBeginTransaction);

			// 参照回数を 0 にする

			trans->_refCount = 0;

			if (trans->_sessionID != Server::IllegalSessionID) {

				// 有効なセッションから参照されているトランザクション記述子は、
				// ハッシュ表により管理されている
				//
				// そこで、与えられたトランザクション記述子を格納する
				// ハッシュ表のバケットを求め、
				// トランザクション記述子を取り除く

				; _SYDNEY_ASSERT(_Transaction::_transTable);

				unsigned int addr =
					_Transaction::transTableHash(trans->getSessionID()) %
						_Transaction::_transTable->getLength();
				HashTable::Bucket& bucket =
					_Transaction::_transTable->getBucket(addr);
				bucket.erase(*trans);
			}

			// トランザクション記述子を破棄する

			delete trans;
		}

		// 与えられたポインタは 0 を指すようにする

		trans = 0;
	}
}

//	FUNCTION public
//	Trans::Transaction::begin -- トランザクションを開始する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction::Mode&	mode
//			指定されたとき
//				開始するトランザクションのモード
//			指定されないとき
//				Trans::Transaction::Mode() が指定されたものとみなす
//		bool					noLock
//			true
//				トランザクション中に行われる操作についてロックしない
//			false または指定されないとき
//				トランザクション中に行われる操作についてロックする
//		bool					noLog
//			true
//				トランザクション中に行われる操作の論理ログを記録しない
//			false または指定されないとき
//				トランザクション中に行われる操作の論理ログを記録する
//		Trans::Transaction::Type::Value type
//			指定されたとき
//				開始するトランザクションのタイプ
//			指定されないとき
//				Trans::Transaction::Type::Unknown が指定されたものとみなす
//
//	RETURN
//		開始されたトランザクションのトランザクション識別子
//
//	EXCEPTIONS
//		Exception::AlreadyBeginTransaction
//			トランザクションが実行中なので、
//			新たにトランザクションを開始できない

const Transaction::ID&
Transaction::begin(Schema::ObjectID::Value databaseID, const Mode& mode,
				   Type::Value type_)
{
	return begin(databaseID, mode, false, false, type_);
}

const Transaction::ID&
Transaction::begin(Schema::ObjectID::Value databaseID,
				   const Mode& mode, bool noLock, bool noLog, Type::Value type_)
{
	if (isInProgress())

		// トランザクションを実行中なので、
		// 新たにトランザクションを開始できない

		_SYDNEY_THROW0(Exception::AlreadyBeginTransaction);

	if (Branch::getExistenceOfActive(getSessionID()))

		// トランザクションを開始するセッションは
		// 『データ操作中』のトランザクションブランチと連係している

		_SYDNEY_THROW0(Exception::XA_InsideActiveBranch);

	// データベースIDを保存する
	
	_databaseID = databaseID;

	// トランザクションを開始する前の
	// トランザクションの状態を取っておく

	const Status::Value saved = getStatus();

	// 開始するトランザクションの種別を求める

	Category::Value category =
		((mode._category != Category::Unknown) ? mode._category :
		 (getDefaultMode()._category != Category::Unknown) ?
		 getDefaultMode()._category : Configuration::Category::get());

	if (category == Category::Unknown)

		// トランザクションの種別が設定されていない
		//
		// SQL 規格では、アイソレーションレベルが
		// READ UNCOMMITTED でなければ、READ WRITE、
		// READ UNCOMMITTED であれば、READ ONLY になる
		//
		// 現状では、READ UNCOMMITTED のアイソレーションレベルは
		// READ COMMITTED として扱うので、
		// 更新トランザクションを開始することになる

		category = Category::ReadWrite;

	while (true)
	{

		// トランザクション記述子の生成・破棄に関する情報を
		// 保護するためのラッチをかける
		//
		//【注意】	読取専用トランザクションを開始するとき、
		//			実行中の更新トランザクションの有無によって
		//			版管理を使用することを決めてから実際に開始するまでの間に、
		//			更新トランザクションが開始されないようにすることが目的である

		Os::AutoTryCriticalSection latch(_Transaction::_latch, false);
		latch.lock();

		// 実行中のトランザクションを表す
		// トランザクション記述子が破棄されないように、
		// 参照回数を 1 増やしておく

		++_refCount;
		bool isCategory = false;
		bool isVersioning = false;

		try {
			// 開始するトランザクションの種別を設定する

			_category = category;

			// トランザクション中の操作に対してロックするか

			(void) setNoLock(noLock);

			// トランザクション中の操作の論理ログを記録するか
			//
			//【注意】	開始するトランザクションが読取専用であっても、
			//			論理ログは記録する

			(void) setNoLog(noLog);

			// トランザクションで版管理を使用しないか

			_noVersion = _Transaction::isNoVersion(*this, mode);

			// 開始するトランザクションのアイソレーションレベルを求める

			_isoLevel =
				((mode._isoLevel != IsolationLevel::Unknown) ? mode._isoLevel :
				 (getDefaultMode()._isoLevel != IsolationLevel::Unknown) ?
				 getDefaultMode()._isoLevel : Configuration::IsolationLevel::get());

			if (!isNoVersion())

				//【注意】	現状では、版管理するトランザクションの
				//			アイソレーションレベルには SERIALIZABLE しかない
				/*
				 *【未実装】READ COMMITTED を導入するには、
				 *			ロックモジュールに持続期間を管理させるようにし、
				 *			版管理モジュールに SQL 文の開始時間による
				 *			版の選択を可能にする必要がある
				 */
				_isoLevel = IsolationLevel::Serializable;

			else if (getIsolationLevel() == IsolationLevel::ReadUncommitted)

				//【注意】	現状では READ UNCOMMITTED のアイソレーションレベルは
				//			READ COMMITTED として扱う

				_isoLevel = IsolationLevel::ReadCommitted;

			// トランザクションの実行状況を実行中に変更する

			_status = Status::InProgress;
			m_eType = type_;

			if (isNoVersion())

				// 開始するトランザクションの識別子を求める

				_id = ID::assign();
			else {

				// 版管理するトランザクションを開始するときは、
				// スキーマデータベースをロックすることにより、
				// アイソレーションレベルに応じた一貫性を提供する

				if (_Transaction::lockSchema(*this, 0) == false)
				{
					// ロックできなかった
					// トランザクション記述子の生成・破棄に関する情報を
					// 保護するためのラッチをはずし、ロックできるまで待つ

					--_refCount;

					latch.unlock();

					_Transaction::lockSchema(*this);	// Instant

					continue;
				}

				// 開始するトランザクションの識別子を求める
				//
				//【注意】	版管理するトランザクションを開始したときの
				//			タイムスタンプでもあるので、
				//			スキーマデータベースをロック後に取得しないと
				//			版管理のために使用できない

				_id = ID::assign();
#ifdef OBSOLETE
				/*
				 *【未実装】版管理するトランザクションにおける READ COMMITTED
				 */
				if (getIsolationLevel() == IsolationLevel::ReadCommitted)
					_statementTimeStamp = getBirthTimeStamp();
#endif
				const List<Transaction>& list =
					getInProgressList(getDatabaseID(), Category::ReadWrite);

				Os::AutoCriticalSection	latch(list.getLatch());

				const unsigned int n = list.getSize();
				if (n) {

					// 版管理するトランザクションを開始したときに実行中の
					// 更新トランザクションのトランザクション記述子を求める

					_startingList.reserve(n);

					List<Transaction>::ConstIterator		ite(list.begin());
					const List<Transaction>::ConstIterator&	end = list.end();

					do {
						_startingList.pushBack((*ite).getID());
					} while (++ite != end) ;
				}
			}

			// データベースごとのトランザクション記述子を
			// 管理するクラスを得る

			TransactionInformation* info
				= _Transaction::getTransactionInformation(getDatabaseID());
			
			{
			// トランザクションの種別に応じて
			// トランザクション記述子を管理するリストの末尾に追加する
			//
			//【注意】	トランザクションの開始時刻の昇順に登録されるようにする

			List<Transaction>& list = info->getInProgressList(getCategory());
			Os::AutoCriticalSection	latch(list.getLatch());
			list.pushBack(*this);
			isCategory = true;
			}
			{
			// トランザクションの版管理の有無に応じて
			// トランザクション記述子を管理するリストの末尾に追加する
			//
			//【注意】	トランザクションの開始時刻の昇順に登録されるようにする

			List<Transaction>& list =info->getInProgressList(isNoVersion());
			Os::AutoCriticalSection	latch(list.getLatch());
			list.pushBack(*this);
			isVersioning = true;
			}

			{
				Os::AutoCriticalSection cAuto(_Transaction::_latch2);
				if (info->getBeginningID() == IllegalID)
				{
					// 版管理のページ再利用が可能かどうかを判断するために、
					// 一連のトランザクションの先頭のIDを利用する
					// 本来なら、もう少し細かく設定することも可能であるが、
					// 実行スピードを考慮し、簡易な方法を取る
					//
					// この _beginningID よりも2つ前のチェックポイントの
					// タイムスタンプが小さければ、2つ前のチェックポイント時の
					// 最新版以外のすべての版が再利用可能である
				
					info->setBeginningID(_id);
				}
			}

			break;	// while ループから抜ける
		
		} catch (...) {

			endTransaction(saved, isCategory, isVersioning);
			_SYDNEY_RETHROW;
		}
	}

	// 開始されたトランザクションの
	// トランザクション識別子を返す

	return getID();
}

//	FUNCTION private
//	Trans::Transaction::prepare -- トランザクションのコミットを準備する
//
//	NOTES
//		XAトランザクションにおける
//		トランザクションブランチのコミット準備のために使用する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::prepare()
{
	; _SYDNEY_ASSERT(!(getXID().isNull() || getXID().isIllegal()));

	if (getStatus() != Status::InProgress)

		// トランザクションが実行中でないので、コミット準備できない

		_SYDNEY_THROW0(Exception::NotBeginTransaction);

	// トランザクションの実行状況をコミット準備中に変更する

	_status = Status::Preparing;

	try {
		// このトランザクションで開始したバックアップのうち、
		// 終了していないものがあれば、すべて終了する

		Admin::EndBackup::endAll(*this);

		// コミット準備したことを表す論理ログを
		// システム用およびトランザクションが操作する
		// データベース用の論理ログファイルへ記録する
		//
		//【注意】	記憶している UNDO ログがあれば、同時にクリアされる

		_Transaction::storeLog(*this, Log::TransactionPrepareData(getXID()));

		// バッチモードであることはない

		; _SYDNEY_ASSERT(!isBatchMode());

	} catch (...) {

		// コミット準備が失敗した

		endTransaction(Status::Failed);
		_SYDNEY_RETHROW;
	}

	// トランザクションの実行状況をコミット準備完了に変更する

	_status = Status::Prepared;
}

//	FUNCTION public
//	Trans::Transaction::commit -- トランザクションをコミットする
//
//	NOTES
//
//	ARGUMENTS
//		bool bNeedReCache_ = false
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::NotBeginTransaction
//			トランザクションが実行中でないので、コミットできない

void
Transaction::commit(bool bNeedReCache_,
					Log::LSN masterLSN)
{
	if (getStatus() != Status::InProgress &&
		getStatus() != Status::Prepared)

		// トランザクションが実行中または
		// コミット準備完了でないので、コミットできない

		_SYDNEY_THROW0(Exception::NotBeginTransaction);

	// トランザクションの実行状況をコミット中に変更する

	_status = Status::Committing;

	try {
		// このトランザクションで開始したバックアップのうち、
		// 終了していないものがあれば、すべて終了する

		Admin::EndBackup::endAll(*this);

		// コミットしたことを表す論理ログを
		// システム用およびトランザクションが操作する
		// データベース用の論理ログファイルへ記録する
		//
		//【注意】	記憶している UNDO ログがあれば、同時にクリアされる

		_Transaction::storeLog(*this,
							   Log::TransactionCommitData(),
							   masterLSN);

		// flush all files
		if (!_batchTimeStamp.isIllegal()) {
			commitBatchInsert();
		}

	} catch (...) {

		// コミットが失敗した

		endTransaction(Status::Failed);
		_SYDNEY_RETHROW;
	}

	// トランザクションの終了時に共通の処理を行う

	endTransaction(Status::Committed, true, true, bNeedReCache_);
}

//	FUNCTION public
//	Trans::Transaction::rollback -- トランザクションをロールバックする
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
//		Exception::NotBeginTransaction
//			トランザクションが実行中でないので、ロールバックできない

void
Transaction::rollback(Log::LSN masterLSN)
{
	if (getStatus() != Status::InProgress &&
		getStatus() != Status::Prepared)

		// トランザクションが実行中または
		// コミット準備完了でないので、ロールバックできない

		_SYDNEY_THROW0(Exception::NotBeginTransaction);

	// トランザクションの実行状況をロールバック中に変更する

	_status = Status::Rollbacking;

	try {
		// トランザクションがかけたラッチのうち、
		// まだかかっているものをすべてはずす
		//
		//【注意】	トランザクション中に行った操作を UNDO する前に
		//			ラッチをはずしておかないと、
		//			UNDO 中にラッチしたときにデッドロックが起きる可能性がある

		_client.unlatchAll();

		if (!isNoLog())

			// 論理ログを記録するトランザクションであれば、
			// トランザクション中に行った操作をすべて UNDO する

			//
			//【注意】	論理ログを記録しない更新トランザクションを
			//			ロールバックしても、実行された更新操作は UNDO されない

			undo();

		if (!_batchTimeStamp.isIllegal()) {
			// バッチモードでの実行であればrecoverする
			recoverBatch();
		}

		// このトランザクションで開始したバックアップのうち、
		// 終了していないものがあれば、すべて終了する
		//
		//【注意】	UNDO 時にバックアップの開始と終了がそれぞれ UNDO されて、
		//			バックアップが終了されたり開始されてもよいように思えるが、
		//			今はそうなっていないので、ここで明示的に終了する

		Admin::EndBackup::endAll(*this);

		// ロールバックログを出力する直前でエラーを発生させる
		
		_TRANS_FAKE_ERROR(Transaction::rollback);

		// ロールバックしたことを表す論理ログを
		// システム用およびトランザクションが操作する
		// データベース用の論理ログファイルへ記録する
		//
		//【注意】	記憶している UNDO ログがあれば、同時にクリアされる

		_Transaction::storeLog(*this,
							   Log::TransactionRollbackData(),
							   masterLSN);

	} catch (...) {

		// ロールバックに失敗した

		endTransaction(Status::Failed);
		_SYDNEY_RETHROW;
	}

	// トランザクションの終了時に共通の処理を行う

	endTransaction(Status::Rollbacked);
}

//	FUNCTION private
//	Trans::Transaction::endTransaction --
//		トランザクションの終了時に共通の処理を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction::Status::Value	status
//			トランザクションの終了時のトランザクションの実行状況を表す値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::endTransaction(Status::Value status,
							bool isCategory,
							bool isVersioning,
							bool bNeedReCache_)
{
	{
	// トランザクション記述子の生成・破棄に関する情報を
	// 保護するためのラッチをかける
	//
	//【注意】	読取専用トランザクションを開始するとき、
	//			実行中の更新トランザクションの有無によって
	//			版管理を使用することを決めてから実際に開始するまでの間に、
	//			更新トランザクションが終了しないようにすることが目的である

	Os::AutoCriticalSection latch(_Transaction::_latch);

	TransactionInformation* info
		= _Transaction::getTransactionInformation(getDatabaseID());
	
	if (isCategory)
	{
		// トランザクションの種別に応じて
		// トランザクション記述子を管理するリストから削除する

		List<Transaction>& list = info->getInProgressList(getCategory());

		Os::AutoCriticalSection	latch(list.getLatch());
		list.erase(*this);

		if (_updatedSameVersion &&
			getCategory() == Category::ReadWrite &&	list.getSize()) {

			// 同じ最新版を更新した更新トランザクションのうち、
			// １つでもトランザクションが終了するとき、
			// 他に実行中の同じ最新版を更新した更新トランザクションがいれば、
			// それらが実行中に開始された読取専用トランザクションは
			// 版管理を使用しないように、その由をおぼえておく

			List<Transaction>::Iterator			ite(list.begin());
			const List<Transaction>::Iterator&	end = list.end();

			do {
				if ((*ite)._updatedSameVersion == true)
					(*ite)._deterrent = true;
			} while (++ite != end) ;
		}
	}
	if (isVersioning)
	{
		// トランザクションの版管理の有無に応じて
		// トランザクション記述子を管理するリストから削除する

		// 実行中のトランザクションを数える
		unsigned int c = 0;

		{
			List<Transaction>& list = info->getInProgressList(isNoVersion());

			Os::AutoCriticalSection	latch(list.getLatch());
			list.erase(*this);

			c = list.getSize();
		}

		if (c == 0)
		{
			// 版管理の有無に応じてチェックした方のトランザクションは0に
			// なったので、反対側もチェックする
			
			List<Transaction>& list = info->getInProgressList(!isNoVersion());

			Os::AutoCriticalSection	latch(list.getLatch());
			c = list.getSize();
		}

		if (c == 0)
		{
			Os::AutoCriticalSection cAuto(_Transaction::_latch2);
			
			// オーバーラップしているトランザクションの内、
			// 最初に開始したトランザクションのIDをクリアする

			info->setBeginningID(IllegalID);
		}

	}

	}
	// ここでトランザクション記述子の生成・破棄に関する情報を保護するための
	// ラッチがはずれる
	
	if (bNeedReCache_) {
		// 版管理用のスキーマオブジェクトキャッシュをクリアする
		Schema::Manager::SystemTable::reCache();
	}

	// このトランザクションがかけたロックのうち、
	// まだかかっているものをすべてはずす

	_CHECK_LOCK_STATUS(_client.releaseAll(Lock::Duration::Inside));

	// トランザクションが終了するので、格納している情報のうち、
	// トランザクションが実行中のときのみ有効なものを初期化する

	if (!isNoVersion()) {
#ifdef OBSOLETE
		/*
		 *【未実装】版管理するトランザクションにおける READ COMMITTED
		 */
		if (getIsolationLevel() == IsolationLevel::ReadCommitted)
			_statementTimeStamp = TimeStamp();
#endif
		if (_startingList.getSize())
			(void) _startingList.erase(
				_startingList.begin(), _startingList.end());
	}

	_id = ID();
	_category = Category::Unknown;
	_isoLevel = IsolationLevel::Unknown;
	_xid = XID();
	_noLock = false;
	_noLog = false;
	_noVersion = false;
	_deterrent = false;
	_connection = 0;
	_databaseLogInfo = Log::Info();

	_batchTimeStamp = TimeStamp();
	_batchDatabaseID = Schema::ObjectID();
	_batchTableID = Schema::ObjectID();
	_batchMode = false;

	m_eType = Type::Unknown;
	m_bStoreLog = true;

	_updatedSameVersion = false;
	_databaseID = Schema::ObjectID::Invalid;

	// トランザクションの実行状況を指定されたものに変更する

	_status = status;

	// 実行中のトランザクションを表す
	// トランザクション記述子が破棄されないように
	// 参照回数を 1 増やしていたので、ここで 1 減らす

	--_refCount;
}

//	FUNCTION private
//	Trans::Transaction::undo -- トランザクション中の操作をすべて UNDO する
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

void
Transaction::undo()
{
	; _SYDNEY_ASSERT(!isNoLog());

	try {
		// トランザクションが行った操作のうち、
		// データベース用の論理ログファイルに記録されているもので、
		// UNDO すべきものがあれば UNDO する

		if (undo(Log::File::Category::Database))

			// UNDO すべきものがあったので UNDO された

			return;

	} catch (...) {
		try {
			// トランザクションが行った操作のうち、
			// システム用の論理ログファイルに記録されているもので、
			// UNDO すべきものがあれば UNDO してみる

			if (undo(Log::File::Category::System)) {

				// UNDO されたので、データベース用の論理ログファイルに
				// 記録されているものには UNDO すべきものはなく、
				// その論理ログファイルを参照していて
				// なんらかのエラーがおきただけのようなので、
				// エラー状態を解除して、処理を継続する

				Common::Thread::resetErrorCondition();
				return;
			}
		} catch (...) {

			// 両方の論理ログファイルについてエラーになったので、
			// スキーマデータベースが利用不可になったとみなす

			(void) Checkpoint::Database::setAvailability(
				Schema::ObjectID::SystemTable, false);
			_SYDNEY_RETHROW;
		}

		// システム用の論理ログファイルについては
		// エラーにならずに UNDO すべきものがなかったので、
		// 通常のデータベースが利用不可になったとみなす

		(void) Checkpoint::Database::setAvailability(
			getLogInfo(Log::File::Category::Database).getDatabaseID(), false);
		_SYDNEY_RETHROW;
	}

	try {
		// データベース用の論理ログファイルについては
		// UNDO すべきものがなかったので、
		// トランザクションが行った操作のうち、
		// システム用の論理ログファイルに記録されているもので、
		// UNDO すべきものがあれば UNDO する

		(void) undo(Log::File::Category::System);

	} catch (...) {

		// スキーマデータベースが利用不可になったとみなす

		(void) Checkpoint::Database::setAvailability(
			Schema::ObjectID::SystemTable, false);
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Trans::Transaction::undo --
//		トランザクション中の操作のうち、
//		ある種類の論理ログファイルに記録されている操作をすべて UNDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::File::Category::Value	category
//			Trans::Log::File::Category::Database の場合
//				トランザクション中に操作するデータベース用の
//				論理ログファイルに記録されている操作を UNDO する
//			Trans::Log::File::Category::System の場合
//				システム用の論理ログファイルへ記録されている操作を UNDO する
//
//	RETURN
//		true
//			指定された論理ログファイルに記録されている
//			トランザクション中に操作を UNDO した
//		false
//			トランザクションは指定された
//			論理ログファイルに論理ログを記録していない
//
//	EXCEPTIONS

bool
Transaction::undo(Log::File::Category::Value category)
{
	; _SYDNEY_ASSERT(getStatus() == Status::Rollbacking);

	// 指定された種別の論理ログファイルを操作するための情報を得る

	const Log::Info& logInfo = getLogInfo(category);

	// トランザクションが記録した論理ログを最後に記録したものから
	// ひとつひとつ読み出しながら、論理ログの表す操作を UNDO していく

	Log::LSN lsn = logInfo.getLastTransactionLSN();
	if (lsn == Log::IllegalLSN ||
		logInfo.getBeginTransactionLSN() == Log::IllegalLSN)

		// トランザクションがそもそも開始されていないので、
		// UNDO する操作はひとつもない

		return false;

	// prepare が実行されているか
	bool prepared = false;

	while (lsn != logInfo.getBeginTransactionLSN()) {
		if (lsn == Log::IllegalLSN) {

			// トランザクションの開始を表す論理ログを読み出す前に、
			// このトランザクションで記録した論理ログがなぜか尽きてしまった
corrupted:
			_SYDNEY_THROW0(Exception::LogItemCorrupted);
		}

		// 処理する論理ログを読み出す

		ModAutoPointer<const Log::Data> data;
		{
		AutoLatch latch(*this, logInfo.getLockName());
		data = loadLog(category, lsn);
		}

		if (!data->isInsideTransactionData())

			// トランザクション中に行われた操作を表す論理ログとして、
			// トランザクション中でないときに記録可能な論理ログが
			// なぜか記録されている

			goto corrupted;
		
		switch (data->getCategory()) {
		case Log::Data::Category::TupleModify:

			// TupleModify should be ignored
			// in rollbacking batch mode transaction

			if (isBatchMode() == false) {

				if (lsn > logInfo.getEndStatementLSN() &&
					loadUndoLog().getCount()) {

					// 実行が終了していない SQL 文で最後に行われた
					// タプル更新操作は読み出した論理ログでなく、
					// UNDO ログを使って UNDO する

					Admin::Recovery::Transaction::undoTuple(
						*this, loadUndoLog(), logInfo.getDatabaseName());

					// 使用した UNDO ログは消去する

					clearUndoLog();
				} else

					// 読み出した論理ログの表すタプルの更新操作を UNDO する

					Admin::Recovery::Transaction::undoTuple(
						*this, *data, logInfo.getDatabaseName());
			}
			// thru

		case Log::Data::Category::SchemaModify:

			// 現状ではスキーマデータベースを更新する
			// ひとつの SQL 文に対して、ひとつの論理ログが記録され、
			// トランザクション中にスキーマデータベースを
			// 更新する SQL 文を複数実行できない
			//
			// また、スキーマデータベースの更新を行う関数内で、
			// エラーが起きても、そのエラー処理で必ず関数を
			// 実行する前の状態に回復される
			// 
			// そのため、スキーマデータベースの更新操作を UNDO する必要はない

		case Log::Data::Category::DriverModify:

			// マージ処理は UNDO する必要はない

		case Log::Data::Category::FileSynchronizeBegin:
		case Log::Data::Category::FileSynchronizeEnd:

			// バージョンファイルの同期を行う関数内で、
			// エラーが起きても、そのエラー処理で必ず関数を
			// 実行する前の状態に回復される
			//
			// そのため、バージョンファイルの同期を UNDO する必要はない

		case Log::Data::Category::TransactionPrepare:

			// トランザクションのコミット準備は UNDO する必要はない

			prepared = true;

		case Log::Data::Category::StatementCommit:

			// トランザクションが行った直前の操作を表す論理ログまで読み飛ばす

			lsn = _SYDNEY_DYNAMIC_CAST(const Log::InsideTransactionData*,
									   data.get())->getBackwardLSN();
			break;

		case Log::Data::Category::StatementRollback:

			// トランザクション中にロールバックされている
			// SQL 文で行われた操作を表す論理ログをすべて読み飛ばす

		case Log::Data::Category::StartBatch:
			
			// バッチモード -> 直前の操作まで読み飛ばす

			lsn = _SYDNEY_DYNAMIC_CAST(const Log::InsideTransactionData*,
									   data.get())->getEndStatementLSN();
			break;

		case Log::Data::Category::TransactionCommit:
		case Log::Data::Category::TransactionRollback:

			// トランザクション中に行われた操作を表す論理ログとして、
			// 認められない操作を表す論理ログがなぜか記録されている

			goto corrupted;

		case Log::Data::Category::XATransaction:
			
			// 子サーバのトランザクションブランチを rollback する
			
			DServer::Branch::rollback(*this, *data,
									  logInfo.getDatabaseName());
				
			// トランザクションが行った直前の操作を表す論理ログまで読み飛ばす

			lsn = _SYDNEY_DYNAMIC_CAST(const Log::InsideTransactionData*,
									   data.get())->getBackwardLSN();
			break;
		}
	}

	return true;
}

//	FUNCTION public
//	Trans::Transaction::beginStatement --
//		トランザクションで SQL 文の 1 文の実行を開始することを通知する
//
//	NOTES
//		ここでの SQL 文とはトランザクション中に実行可能な SQL 文である
//
//		最初の 1 文の実行の開始の通知については、
//		Trans::Transaction::begin で代用できる
//
//	ARGUMENTS
//		Communication::Connection* connection
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::NotBeginTransaction
//			トランザクションが実行中でないので、SQL 文の実行を開始できない

void
Transaction::beginStatement(Communication::Connection* connection)
{
	if (getStatus() != Status::InProgress)

		// トランザクションが実行中でないので、SQL 文の実行を開始できない

		_SYDNEY_THROW0(Exception::NotBeginTransaction);

	try {
#ifdef OBSOLETE
		/*
		 *【未実装】版管理するトランザクションにおける READ COMMITTED
		 */
		if (!isNoVersion() &&
			getIsolationLevel() == IsolationLevel::ReadCommitted) {

			// 版管理するトランザクションにおいて
			// SQL 文の実行を開始するときは、
			// スキーマデータベースをロックすることにより、
			// アイソレーションレベルに応じた一貫性を提供する

			_Transaction::lockSchema(*this);

			// SQL 文の実行を開始時のタイムスタンプを求める
			//
			//【注意】	スキーマデータベースをロック後に取得すること

			_statementTimeStamp = TimeStamp::assign();
		}
#endif
		// SQL 文の実行が中断されたか検知するための情報を覚える

		_connection = connection;

		// パフォーマンス用の変数を初期化する

		m_cLockTimer.reset();
		m_iSendRowCount = 0;
		m_iPageReferenceCount = 0;
		m_iPageReadCount = 0;
		m_iLockCount = 0;

	} catch (...) {

		endStatement(false);
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Trans::Transaction::commitStatement --
//		トランザクションで SQL の 1 文の実行をコミットしたことを通知する
//
//	NOTES
//		ここでの SQL 文とはトランザクション中に実行可能な SQL 文である
//
//		最後の 1 文の実行のコミットまたはロールバックの通知については、
//		Trans::Transaction::commit または
//		Trans::Transaction::rollback で代用できる
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::NotBeginTransaction
//			トランザクションが実行中でないので、SQL 文の実行をコミットできない

void
Transaction::commitStatement(Log::LSN masterLSN)
{
	if (getStatus() != Status::InProgress)

		// トランザクションが実行中でないので、SQL 文の実行を終了できない

		_SYDNEY_THROW0(Exception::NotBeginTransaction);

	// トランザクションの実行状況をコミット中に変更する

	_status = Status::Committing;

	try {
		// 文の実行をコミットしたことを表す論理ログを
		// システム用およびトランザクションが操作する
		// データベース用の論理ログファイルへ記録する
		//
		//【注意】	記憶している UNDO ログがあれば、同時にクリアされる

		_Transaction::storeLog(*this,
							   Log::StatementCommitData(),
							   masterLSN);

	} catch (...) {

		// SQL の 1 文の実行のコミットに失敗したので、
		// トランザクションを終了する

		endTransaction(Status::Failed);
		_SYDNEY_RETHROW;
	}

	// トランザクションで SQL の 1 文の実行の終了時に共通の処理を行う

	endStatement(true);
}

//	FUNCTION public
//	Trans::Transaction::rollbackStatement --
//		トランザクションで SQL の 1 文の実行をロールバックしたことを通知する
//
//	NOTES
//		ここでの SQL 文とはトランザクション中に実行可能な SQL 文である
//
//		最後の 1 文の実行の終了の通知については、
//		Trans::Transaction::commit または
//		Trans::Transaction::rollback で代用できる
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::NotBeginTransaction
//			トランザクションが実行中でないので、
//			SQL 文の実行をロールバックできない

void
Transaction::rollbackStatement(Log::LSN masterLSN)
{
	if (getStatus() != Status::InProgress)

		// トランザクションが実行中でないので、SQL 文の実行を終了できない

		_SYDNEY_THROW0(Exception::NotBeginTransaction);

	// トランザクションの実行状況をロールバック中に変更する

	_status = Status::Rollbacking;

	try {
		// トランザクションがかけたラッチのうち、
		// まだかかっているものをすべてはずす
		//
		//【注意】	文をまたいでラッチをかけることはないので、
		//			トランザクションがかけているラッチをすべてはずしてよい
		//
		//【注意】	SQL の 1 文で行った操作を UNDO する前に
		//			ラッチをはずしておかないと、
		//			UNDO 中にラッチしたときにデッドロックが起きる可能性がある

		_client.unlatchAll();

		if (!isNoLog())

			// 論理ログを記録するトランザクションであれば、
			// SQL の 1 文で行った操作をすべて UNDO する
			//
			//【注意】	論理ログを記録しない更新トランザクションでの
			//			SQL の更新文をロールバックしても、
			//			実行された更新操作は UNDO されない

			undoStatement();

		// 文の実行をロールバックしたことを表す論理ログを
		// システム用およびトランザクションが操作する
		// データベース用の論理ログファイルへ記録する
		//
		//【注意】	記憶している UNDO ログがあれば、同時にクリアされる

		_Transaction::storeLog(*this,
							   Log::StatementRollbackData(),
							   masterLSN);

	} catch (...) {

		// SQL 文の 1 文の実行のロールバックに失敗したので、
		// トランザクションを終了する

		endTransaction(Status::Failed);
		_SYDNEY_RETHROW;
	}

	// トランザクションで SQL の 1 文の実行の終了時に共通の処理を行う

	endStatement(false);
}

//	FUNCTION private
//	Trans::Transaction::endStatement --
//		トランザクションで SQL の 1 文の実行の終了時に共通の処理を行う
//
//	NOTES
//
//	ARGUMENTS
//		bool					isSucceeded
//			true
//				SQL 文の実行が成功した
//			false
//				SQL 文の実行が失敗した
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::endStatement(bool isSucceeded)
{
	// 実行が終了した SQL 文中でかけたロックのうち、
	// まだかかっているものを必要があればはずす

	_CHECK_LOCK_STATUS(
		_client.releaseStackRequest(Lock::Duration::Inside, isSucceeded));
#ifdef OBSOLETE
	/*
	 *【未実装】版管理するトランザクションにおける READ COMMITTED
	 */
	if (!isNoVersion() && getIsolationLevel() == IsolationLevel::ReadCommitted)

		// 実行が終了した SQL 文の開始時タイムスタンプを初期化しておく

		_statementTimeStamp = TimeStamp();
#endif
	// SQL 文の実行が中断されたか検知するための情報を忘れる

	_connection = 0;

	// トランザクションの実行状況をトランザクションの実行中に戻す

	_status = Status::InProgress;
}

//	FUNCTION private
//	Trans::Transaction::undoStatement --
//		トランザクションの SQL の 1 文での操作をすべて UNDO する
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

void
Transaction::undoStatement()
{
	; _SYDNEY_ASSERT(!isNoLog());

	try {
		// トランザクションの SQL の 1 文での操作のうち、
		// データベース用の論理ログファイルに記録されているもので、
		// UNDO すべきものがあれば UNDO する

		if (undoStatement(Log::File::Category::Database))

			// UNDO すべきものがあったので UNDO された

			return;

	} catch (...) {
		try {
			// トランザクションの SQL の 1 文での操作のうち、
			// システム用の論理ログファイルに記録されているもので、
			// UNDO すべきものがあれば UNDO してみる

			if (undoStatement(Log::File::Category::System)) {

				// UNDO されたので、データベース用の論理ログファイルに
				// 記録されているものには UNDO すべきものはなく、
				// その論理ログファイルを参照していて
				// なんらかのエラーが起きただけのようなので、
				// エラー状態を解除して、処理を継続する

				Common::Thread::resetErrorCondition();
				return;
			}
		} catch (...) {

			// 両方の論理ログファイルについてエラーになったので、
			// スキーマデータベースが利用不可になったとみなす

			(void) Checkpoint::Database::setAvailability(
				Schema::ObjectID::SystemTable, false);
			_SYDNEY_RETHROW;
		}

		// システム用の論理ログファイルについては
		// エラーにならずに UNDO すべきものがなかったので、
		// 通常のデータベースが利用不可になったとみなす

		(void) Checkpoint::Database::setAvailability(
			getLogInfo(Log::File::Category::Database).getDatabaseID(), false);
		_SYDNEY_RETHROW;
	}

	try {
		// データベース用の論理ログファイルについては
		// UNDO すべきものがなかったので、
		// トランザクションの SQL の 1 文での操作のうち、
		// システム用の論理ログファイルに記録されているもので、
		// UNDO すべきものがあれば UNDO する

		(void) undoStatement(Log::File::Category::System);

	} catch (...) {

		// スキーマデータベースが利用不可になったとみなす

		(void) Checkpoint::Database::setAvailability(
			Schema::ObjectID::SystemTable, false);
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Trans::Transaction::undoStatement --
//		トランザクションの SQL の 1 文での操作のうち、
//		ある種類の論理ログファイルに記録されている操作をすべて UNDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::File::Category::Value	category
//			Trans::Log::File::Category::Database の場合
//				トランザクション中に操作するデータベース用の
//				論理ログファイルに記録されている操作を UNDO する
//			Trans::Log::File::Category::System の場合
//				システム用の論理ログファイルへ記録されている操作を UNDO する
//
//	RETURN
//		true
//			指定された論理ログファイルに記録されている
//			トランザクションの SQL の 1 文での操作を UNDO した
//		false
//			トランザクションは指定された
//			論理ログファイルに論理ログを記録していない
//
//	EXCEPTIONS

bool
Transaction::undoStatement(Log::File::Category::Value category)
{
	; _SYDNEY_ASSERT(getStatus() == Status::Rollbacking);

	// 指定された種別の論理ログファイルを操作するための情報を得る

	const Log::Info& logInfo = getLogInfo(category);

	// トランザクションが記録した論理ログを最後に記録したものから
	// ひとつひとつ読み出しながら、論理ログの表す操作を UNDO していく

	Log::LSN lsn = logInfo.getLastTransactionLSN();
	if (lsn == Log::IllegalLSN ||
		logInfo.getBeginTransactionLSN() == Log::IllegalLSN)

		// トランザクションがそもそも開始されていないので、
		// UNDO する操作はひとつもない

		return false;

	while (lsn != logInfo.getEndStatementLSN()) {
		; _SYDNEY_ASSERT(lsn != logInfo.getBeginTransactionLSN());
		if (lsn == Log::IllegalLSN) {

			// 直前の SQL 文の終了を表す論理ログを読み出す前に、
			// このトランザクションで記録した論理ログがなぜか尽きてしまった
corrupted:
			_SYDNEY_THROW0(Exception::LogItemCorrupted);
		}

		// 処理する論理ログを読み出す

		ModAutoPointer<const Log::Data> data;
		{
		AutoLatch latch(*this, logInfo.getLockName());
		data = loadLog(category, lsn);
		}

		if (!data->isInsideTransactionData())

			// トランザクション中に行われた操作を表す論理ログとして、
			// トランザクション中でないときに記録可能な論理ログが
			// なぜか記録されている

			goto corrupted;

		switch (data->getCategory()) {
		case Log::Data::Category::TupleModify:

			if (lsn > logInfo.getEndStatementLSN() &&
				loadUndoLog().getCount()) {

				// 実行が終了していない SQL 文で最後に行われた
				// タプル更新操作は読み出した論理ログでなく、
				// UNDO ログを使って UNDO する

				Admin::Recovery::Transaction::undoTuple(
					*this, loadUndoLog(), logInfo.getDatabaseName());

				// 使用した UNDO ログは消去する

				clearUndoLog();
			} else

				// 読み出した論理ログの表すタプルの更新操作を UNDO する

				Admin::Recovery::Transaction::undoTuple(
					*this, *data, logInfo.getDatabaseName());
			// thru

		case Log::Data::Category::SchemaModify:

			// 現状ではスキーマデータベースを更新する
			// ひとつの SQL 文に対して、ひとつの論理ログが記録され、
			// トランザクション中にスキーマデータベースを
			// 更新する SQL 文を複数実行できない
			//
			// また、スキーマデータベースの更新を行う関数内で、
			// エラーが起きても、そのエラー処理で必ず関数を
			// 実行する前の状態に回復される
			// 
			// そのため、スキーマデータベースの更新操作を UNDO する必要はない

		case Log::Data::Category::DriverModify:

			// マージ処理は UNDO する必要はない

		case Log::Data::Category::FileSynchronizeBegin:
		case Log::Data::Category::FileSynchronizeEnd:

			// バージョンファイルの同期を行う関数内で、
			// エラーが起きても、そのエラー処理で必ず関数を
			// 実行する前の状態に回復される
			//
			// そのため、バージョンファイルの同期を UNDO する必要はない

		case Log::Data::Category::StartBatch:
			
			// バッチモード

		case Log::Data::Category::XATransaction:

			// 分散トランザクションは UNDO する必要はない

			// トランザクションが行った直前の操作を表す論理ログまで読み飛ばす

			lsn = _SYDNEY_DYNAMIC_CAST(const Log::InsideTransactionData*,
									   data.get())->getBackwardLSN();
			break;

		case Log::Data::Category::StatementCommit:
		case Log::Data::Category::StatementRollback:
		case Log::Data::Category::TransactionCommit:
		case Log::Data::Category::TransactionRollback:
		case Log::Data::Category::TransactionPrepare:

			// SQL 文の実行中に行われた操作を表す論理ログとして、
			// 認められない操作を表す論理ログがなぜか記録されている

			goto corrupted;
		}
	}

	return true;
}

//	FUNCTION public
//	Trans::Transaction::isCanceledStatement --
//		トランザクションで現在実行中の SQL の 1 文の中断が指示されたか
//
//	NOTES
//		ここでの SQL 文とはトランザクション中に実行可能な SQL 文である
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			中断が指示された
//		false
//			中断が指示されていない
//
//	EXCEPTIONS

bool
Transaction::isCanceledStatement() const
{
	//【注意】	SQL 文が開始されていないときは、
	//			実行スレッドが終了中かどうかで判断する

	return getStatus() == Status::InProgress &&
		(_connection ? _connection->isCanceled() : ModThisThread::isAborted());
}

//
//	FUNCTION public
//	Trans::Transaction::flushConnection
//		-- クライアントとのコネクションをflushする
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
Transaction::flushConnection()
{
	if (_connection) _connection->flush();
}

//	FUNCTION public
//	Trans::Transaction::lock -- あるオブジェクトにロックをかける
//
//	NOTES
//		最大待ち時間を指定しないと、返り値として false を返すことはない
//
//	ARGUMENTS
//		Lock::Name&			name
//			ロックをかけるオブジェクトのロック名
//		Lock::Mode::Value	mode
//			かけるロックのモード
//		Lock::Duration::Value	duration
//			かけるロックの持続期間
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
//		Exception::NotBeginTransaction
//			トランザクション中でないのに、
//			ユーザロックでないロックをかけようとしている

bool
Transaction::lock(const Lock::Name& name,
				  Lock::Mode::Value mode,
				  Lock::Duration::Value duration,
				  Lock::Timeout::Value timeout)
{
	if (!isInProgress() && duration != Lock::Duration::Long)

		// トランザクション中でないのに、
		// ユーザロックでないロックをかけようとしている

		_SYDNEY_THROW0(Exception::NotBeginTransaction);

	if (!(isNoLock() || mode == Lock::Mode::N)) {

		// 指定されたロック名のロック項目を得て、
		// そのロック項目を与えられたモードでロックする

		// ロック回数を増やす
		if ((addLockCount() % 10000) == 0)
		{
			// 1万回ロックしたら1回キャンセル要求を確認する

			if (isCanceledStatement())

				_SYDNEY_THROW0(Exception::Cancel);
		}

		Lock::AutoItem	item(Lock::Item::attach(name));
		; _SYDNEY_ASSERT(item.isOwner());

		Lock::Status::Value status;

		// ロック待ち時間を集計する
		_Transaction::AutoTimer cAutoTimer(m_cLockTimer);

		if (timeout == Lock::Timeout::Unlimited)
		{
			// 無限に待つ場合でも1秒に一回キャンセルを確認する

			Lock::Timeout::Value t = 1000;	// 1秒
			
			while ((status = item->hold(_client, mode, duration, t))
				   == Lock::Status::Timeout)
			{
				// キャンセル要求を確認する

				Common::Thread::resetErrorCondition();
				
				if (isCanceledStatement())

					_SYDNEY_THROW0(Exception::Cancel);
			}
		}
		else
		{
			// 有限の場合
			
			status = item->hold(_client, mode, duration, timeout);
		}

		switch (status) {
		case Lock::Status::Timeout:

			// ロック待ちしたが、最大待ち時間に達した

			Common::Thread::resetErrorCondition();
			return false;

		default:
			_CHECK_LOCK_STATUS(status);
		}
	}

	return true;
}

//	FUNCTION public
//	Trans::Transaction::unlock -- あるオブジェクトのロックをはずす
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name&			name
//			ロックをはずすオブジェクトのロック名
//		Lock::Mode::Value	mode
//			はずすロックのモード
//		Lock::Duration::Value	duration
//			かけるロックの持続期間
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::AlreadyBeginTransaction
//			トランザクション中なのに、
//			カーソルロックでないロックをはずそうとしている
//		Exception::NotBeginTransaction
//			トランザクション中でないのに、
//			ユーザロックでないロックをはずそうとしている

void
Transaction::unlock(const Lock::Name& name,
					Lock::Mode::Value mode, Lock::Duration::Value duration)
{
#ifdef OBSOLETE
	/*
	 *【未実装】	持続期間が Statement のロックが実装されていないため
	 *				明示的に Statement のロックをはずしたいことがある
	 */
	if (duration != ((isInProgress()) ?
					 Lock::Duration::Cursor : Lock::Duration::User))

		// トランザクション中のときカーソルロックでないロックを、
		// トランザクション中でないときユーザロックでないロックを、
		// それぞれはずそうとしている

		if (isInProgress())
			_SYDNEY_THROW0(Exception::AlreadyBeginTransaction);
		else
			_SYDNEY_THROW0(Exception::NotBeginTransaction);
#endif
	if (!(isNoLock() || mode == Lock::Mode::N)) {

		// 指定されたロック名のロック項目を得て、
		// そのロック項目に対する与えられたモードのロックをはずす

		Lock::AutoItem	item(Lock::Item::attach(name));
		; _SYDNEY_ASSERT(item.isOwner());

		_CHECK_LOCK_STATUS(item->release(_client, mode, duration));
	}
}

//	FUNCTION public
//	Trans::Transaction::convertLock --
//		あるオブジェクトのロックを他のロックに変換する
//
//	NOTES
//		最大待ち時間を指定しないと、返り値として false を返すことはない
//
//	ARGUMENTS
//		Lock::Name&			name
//			ロックを変換するオブジェクトのロック名
//		Lock::Mode::Value	from
//			変換前のロックのモード
//		Lock::Duration::Value	fromDuration
//			変換前のロックの持続期間
//		Lock::Mode::Value	to
//			変換後のロックのモード
//		Lock::Duration::Value	toDuration
//			変換後のロックの持続期間
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
//		Exception::AlreadyBeginTransaction
//			トランザクション中なのに、ユーザロックを変換しようとしている
//		Exception::NotBeginTransaction
//			トランザクション中でないのに、
//			ユーザロックでないロックを変換しようとしている

bool
Transaction::convertLock(
	const Lock::Name& name,
	Lock::Mode::Value from, Lock::Duration::Value fromDuration,
	Lock::Mode::Value to, Lock::Duration::Value toDuration,
	Lock::Timeout::Value timeout)
{
	if (from > to && ((isInProgress()) ?
					  fromDuration == Lock::Duration::User :
					  fromDuration != Lock::Duration::User))

		// トランザクション中のときユーザロックを、
		// トランザクション中でないときユーザロックでないロックを、
		// それより弱いロックに変換しようとしてる

		(isInProgress()) ?
			_SYDNEY_THROW0(Exception::AlreadyBeginTransaction) :
			_SYDNEY_THROW0(Exception::NotBeginTransaction);

	if (isNoLock())

		// ロックしないトランザクションである

		return false;

	// 指定されたロック名のロック項目を得て、
	// そのロック項目に対する与えられたモードのロックを変換する

	// ロック回数を増やす
	if ((addLockCount() % 10000) == 0)
	{
		// 1万回ロックしたら1回キャンセル要求を確認する

		if (isCanceledStatement())

			_SYDNEY_THROW0(Exception::Cancel);
	}

	Lock::AutoItem	item(Lock::Item::attach(name));
	; _SYDNEY_ASSERT(item.isOwner());

	Lock::Status::Value status;

	if (timeout == Lock::Timeout::Unlimited)
	{
		// 無限に待つ場合でも1秒に一回キャンセルを確認する

		Lock::Timeout::Value t = 1000;		// 1秒

		while ((status = item->convert(_client, from, fromDuration,
									   to, toDuration, t))
			   == Lock::Status::Timeout)
		{
			// キャンセル要求を確認する

			Common::Thread::resetErrorCondition();
				
			if (isCanceledStatement())

				_SYDNEY_THROW0(Exception::Cancel);
		}
	}
	else
	{
		// 有限の場合
		
		status = item->convert(
			_client, from, fromDuration, to, toDuration, timeout);
	}

	switch (status) {
	case Lock::Status::Timeout:

		// 変換しようとしてロック待ちしたが、最大待ち時間に達した

		Common::Thread::resetErrorCondition();
		return false;

	default:
		_CHECK_LOCK_STATUS(status);
	}

	return true;
}

//	FUNCTION public
//	Trans::Transaction::getAdequateLock --
//		ある種類のオブジェクトを操作するときにかけるべき適切なロックを得る
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name::Category::Value	locked
//			ロックするオブジェクトの種類
//		Lock::Name::Category::Value	manipulated
//			指定されたとき
//				操作するオブジェクトの種類
//			指定されないとき
//				Lock::Name::Category::Tuple が指定されたものとみなす
//		bool				readOnly
//			true
//				読み込みのみをしようとしている
//			false
//				読み書きしようとしている
//		Lock::Mode::Value&	mode
//			かけるべきロックのモードが設定され、
//			Lock::Mode::N が設定されたときは、ロックする必要はない
//		Lock::Duration::Value&	duration
//			かけるべきロックの持続期間が設定される
//		bool batchMode = false
//			trueのときバッチモード
//
//	RETURN
//		true
//			ロックする必要がある
//		false
//			ロックする必要はない
//
//	EXCEPTIONS
//		なし

bool
Transaction::getAdequateLock(Lock::Name::Category::Value locked,
							 bool readOnly,
							 Lock::Mode::Value& mode,
							 Lock::Duration::Value& duration,
							 bool batchMode /* = false */) const
{
	return getAdequateLock(
			   locked, Lock::Name::Category::Tuple, readOnly, mode, duration, batchMode);
}

bool
Transaction::getAdequateLock(Lock::Name::Category::Value locked,
							 Lock::Name::Category::Value manipulated,
							 bool readOnly,
							 Lock::Mode::Value& mode,
							 Lock::Duration::Value& duration,
							 bool batchMode /* = false */) const
{
	; _SYDNEY_ASSERT(isInProgress());

	if (isNoLock() ||
		getStatus() == Status::Preparing ||
		getStatus() == Status::Rollbacking ||
		getStatus() == Status::Committing) {

		// ロックしないトランザクションである
		// または、コミット準備、ロールバック、
		// コミット中のときはロックする必要はない

		mode = Lock::Mode::N;
		duration = Lock::Duration::Instant;

		return false;
	}

	// 適切なロックモードと持続期間を求める

	const _AdequateLock::_Info& info =
		((!isNoVersion() && readOnly) ?
		 _AdequateLock::infoForVersioningTransaction
			[getIsolationLevel()][locked] :
		 _AdequateLock::infoForNoVersionTransaction
			[(readOnly ? 1 : (batchMode ? 2 : 0))][manipulated][getIsolationLevel()][locked]);
	mode = info._mode;
	duration = info._duration;

	// 求めた適切なロックモードが N でなければ、ロックの必要がある

	return mode != Lock::Mode::N;
}

//	FUNCTION public
//	Trans::Transaction::latch -- あるオブジェクトにラッチをかける
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name&			name
//			ラッチをかけるオブジェクトのロック名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::latch(const Lock::Name& name)
{
	if (!isNoLock()) {

		// 指定されたロック名のロック項目を得て、
		// そのロック項目をラッチする

		Lock::AutoItem	item(Lock::Item::attach(name));
		; _SYDNEY_ASSERT(item.isOwner());

		item->latch(_client);
	}
}

//	FUNCTION public
//	Trans::Transaction::unlatch -- あるオブジェクトのラッチをはずす
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name&			name
//			ラッチをはずすオブジェクトのロック名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::unlatch(const Lock::Name& name)
{
	if (!isNoLock()) {

		// 指定されたロック名のロック項目を得て、
		// そのロック項目に対するラッチをはずす

		Lock::AutoItem	item(Lock::Item::attach(name));
		; _SYDNEY_ASSERT(item.isOwner());

		item->unlatch(_client);
	}
}

//	FUNCTION public
//	Trans::Transaction::storeLog --
//		トランザクションで行った操作に関する論理ログを
//		論理ログファイルに記録する
//
//	NOTES
//		呼び出し側で論理ログファイルに対して排他制御を行う必要がある
//
//	ARGUMENTS
//		Trans::Log::File::Category::Value	category
//			Trans::Log::File::Category::Database の場合
//				トランザクション中に操作するデータベース用の
//				論理ログファイルへ記録する
//			Trans::Log::File::Category::System の場合
//				システム用の論理ログファイルへ記録する
//		Trans::Log::Data&	data
//			論理ログとして記録するデータ
//		Trans::Log::LSN		masterLSN
//			マスター側のLSN
//
//	RETURN
//		Trans::Log::IllegalLSN 以外の値
//			記録された論理ログのログシーケンス番号
//		Trans::Log::IllegalLSN
//			必要なかったので、論理ログを記録しなかった
//
//	EXCEPTIONS
//		Exception::NotBeginTransaction
//			トランザクション中でないので、論理ログを記録できない

Log::LSN
Transaction::storeLog(Log::File::Category::Value category,
					  const Log::Data& data,
					  Log::LSN masterLSN)
{
	if (!isInProgress() && data.isInsideTransactionData())

		// トランザクション中でないのにトランザクション中のみ
		// 記録可能な論理ログを記録しようとしている

		_SYDNEY_THROW0(Exception::NotBeginTransaction);

	Log::LSN lsn = Log::IllegalLSN;

	if (!isNoLog()) {

		// 新しい論理ログを記録するということは、
		// 直前のタプルの処理は終わっているということなので、
		// 記憶している UNDO ログがあれば、すべて忘れる

		clearUndoLog();

		if (m_bStoreLog)
		{
			// 与えられたデータを論理ログファイルに記録する

			lsn = ((category == Log::File::Category::System) ?
				   _systemLogInfo : _databaseLogInfo).store(data, masterLSN);
		}
	}

	return lsn;
}

//
//	FUNCTION public
//	Trans::Transaction::setSynchronizeDone --
//		同期処理が完了したことを論理ログヘッダーに記録する
//
//	NOTES
//
//	ARGUMENTS
//	Log::File::Category::Value category
//		Trans::Log::File::Category::Database の場合
//			トランザクション中に操作するデータベース用の
//			論理ログファイルへ記録する
//		Trans::Log::File::Category::System の場合
//			システム用の論理ログファイルへ記録する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Transaction::setSynchronizeDone(Log::File::Category::Value category)
{
	if (!isNoLog()) {
		((category == Log::File::Category::System) ?
		 _systemLogInfo : _databaseLogInfo).setSyncDone();
	}
}

//
//	FUNCTION public
//	Trans::Transaction::discardLog -- 不要な論理ログを削除する
//
//	NOTES
//
//	ARGUMENTS
//	bool bDiscardFull_
//		最新以外すべて削除する場合はtrue、前々回のチェックポイント以前を
//		削除する場合はfalseを指定する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Transaction::discardLog(bool bDiscardFull_)
{
	if (!isNoLog()) {
		_databaseLogInfo.discardLog(bDiscardFull_);
	}
}

//	FUNCTION public
//	Trans::Transaction::setLog --
//		トランザクション記述子の表すトランザクションの操作する
//		データベース用の論理ログファイルを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			トランザクションの操作するデータベースの
//			スキーマオブジェクトを表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::OtherDatabaseAlreadyModified
//			トランザクションですでに更新したデータベースと異なる
//			データベース用の論理ログファイルに関する情報を設定しようとしている

void
Transaction::setLog(Schema::Database& database)
{
	if (_databaseLogInfo.getDatabaseID() != database.getID()) {

		// 与えられたデータベース用の論理ログファイルが
		// 現在、設定済のものと異なる

		if (_databaseLogInfo.getBeginTransactionLSN() != Log::IllegalLSN)

			// トランザクションを実行中なので、
			// 論理ログファイルが使用されている可能性がある
			//
			//【注意】	トランザクションが開始されていても、
			//			トランザクションの開始を表す論理ログが
			//			記録されていなければ大丈夫なので、
			//			Transaction::isInProgress で検査しない

			_SYDNEY_THROW2(Exception::OtherDatabaseAlreadyModified,
						   _databaseLogInfo.getDatabaseName(),
						   database.getName());

		_databaseLogInfo = Log::Info(database);
	}
}

//	FUNCTION public
//	Trans::Transaction::isLogStored --
//		論理ログファイルに論理ログを記録したか
//
//	NOTES
//
//	ARGUMETNS
//		Trans::Log::File::Category::Value	category
//			Trans::Log::File::Category::Database　または指定されないとき
//				データベース用の論理ログファイルについて調べる
//			Trans::Log::File::Category::System
//				システム用の論理ログファイルについて調べる
//
//	RETURN
//		true
//			論理ログファイルに記録した
//		false
//			論理ログファイルに記録していない
//
//	EXCEPTIONS
//		なし

bool
Transaction::isLogStored(Log::File::Category::Value category) const
{
	return getLogInfo(category).getBeginTransactionLSN() != Log::IllegalLSN;
}

//
//	FUNCTION public
//	Trans::Transaction::restoreLog
//		-- 論理ログからトランザクションの状態を復元する
//
//	NOTES
//
//	ARGUMENTS
// 	const Trans::Log::Data& data_
//		論理ログ
//	Trans::Log::LSN lsn_
//		与えた論理ログのログシーケンス番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Transaction::restoreLog(const Log::Data& data_, Log::LSN lsn_)
{
	_databaseLogInfo.restore(data_, lsn_);
}

//	FUNCTION public
//	Trans::Transaction::prepareUndoLog --
//		論理ファイルに対する操作を UNDO するための情報である
//		UNDO ログを確実に記憶できるように準備する
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
//		Exception::NotBeginTransaction
//			トランザクション中でないので、
//			UNDO ログを記憶するための準備はできない

void
Transaction::prepareUndoLog()
{
	if (getStatus() != Status::InProgress)

		// トランザクション中でないのに
		// UNDO ログの記憶できるように準備しようとしている

		_SYDNEY_THROW0(Exception::NotBeginTransaction);

	if (!isNoLog())
		_undoLog.reserve(_undoLog.getCount() + 1);
}

//	FUNCTION public
//	Trans::Transaction::storeUndoLog --
//		論理ファイルに対する操作を UNDO するための情報である
//		UNDO ログを記憶する
//
//	NOTES
//		記録した情報は、必要がなくなった時点で自動的に破棄される
//
//	ARGUMENTS
//		Common::DataArrayData::Pointer	p
//			UNDO ログを表す配列へのポインタ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::NotBeginTransaction
//			トランザクション中でないので、UNDO ログを記憶できない

void
Transaction::storeUndoLog(Common::DataArrayData::Pointer p)
{
	if (getStatus() != Status::InProgress)

		// トランザクション中でないのに
		// UNDO ログを記録しようとしている

		_SYDNEY_THROW0(Exception::NotBeginTransaction);

	if (!isNoLog())
		_undoLog.pushBack(p);
}

//	FUNCTION public
//	Trans::Transaction::getInProgressList --
//		現在、実行中のトランザクションのうち、
//		ある種別のトランザクションのトランザクション記述子を得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction::Category::Value	category
//			Trans::Transaction::Category::ReadWrite
//				更新トランザクションのトランザクション記述子を得る
//			Trans::Transaction::Category::ReadOnly
//				読取専用トランザクションのトランザクション記述子を得る
//
//	RETURN
//		得られたトランザクション記述子のリスト
//
//	EXCEPTIONS
//		なし

// static
const List<Transaction>&
Transaction::getInProgressList(Schema::ObjectID::Value db,
							   Category::Value category)
{
	TransactionInformation* info
		= _Transaction::getTransactionInformation(db);
	return info->getInProgressList(category);
}

//	FUNCTION public
//	Trans::Transaction::getInProgressList --
//		現在、実行中のトランザクションのうち、
//		版管理の有無のいずれかのトランザクションのトランザクション記述子を得る
//
//	NOTES
//
//	ARGUMENTS
//		bool				noVersion
//			true
//				版管理しないトランザクションのトランザクション記述子を得る
//			false
//				版管理するトランザクションのトランザクション記述子を得る
//
//	RETURN
//		得られたトランザクション記述子のリスト
//
//	EXCEPTIONS
//		なし

// static
const List<Transaction>&
Transaction::getInProgressList(Schema::ObjectID::Value db,
							   bool noVersion)
{
	TransactionInformation* info
		= _Transaction::getTransactionInformation(db);
	return info->getInProgressList(noVersion);
}

//	FUNCTION public
//	Trans::Transaction::isAnyInProgress --
//		現在、実行中のトランザクションのうち、
//		指定された種別のトランザクションのトランザクション記述子が
//		存在するかを確認する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction::Category::Value	category
//			Trans::Transaction::Category::ReadWrite
//				更新トランザクションのトランザクション記述子を得る
//			Trans::Transaction::Category::ReadOnly
//				読取専用トランザクションのトランザクション記述子を得る
//
//	RETURN
//		指定された種別のトランザクションのトランザクション記述子が
//		存在しているか否か
//
//	EXCEPTIONS
//		なし

// static
bool
Transaction::isAnyInProgress(Category::Value category)
{
	Os::AutoCriticalSection cAuto(_Transaction::_latch2);

	unsigned int i = 0;
	unsigned int length = _Transaction::_databaseTable->getLength();

	for (; i < length; ++i)
	{
		DatabaseTable::Bucket& bucket
			= _Transaction::_databaseTable->getBucket(i);
		
		DatabaseTable::Bucket::Iterator	ite(bucket.begin());
		const DatabaseTable::Bucket::Iterator&	end = bucket.end();
				
		while (ite != end)
		{
			TransactionInformation& info = *ite;

			List<Transaction>& list
				= info.getInProgressList(category);

			Os::AutoCriticalSection cAuto2(list.getLatch());

			if (list.getSize())

				// 指定した種別のトランザクションのトランザクション記述子が
				// 存在した
				
				return true;

			++ite;
		}
	}

	return false;
}

//	FUNCTION public
//	Trans::Transaction::setNoLock --
//		ロックしないトランザクションかを設定する
//
//	NOTES
//		トランザクションを開始してから設定を変更できるが、
//		トランザクションマネージャの動作を理解した上で設定を変更しないと、
//		その場合の動作は保証できない
//
//	ARGUMENTS
//		bool				v
//			true
//				ロックしないトランザクションにする
//			false
//				ロックするトランザクションにする
//
//	RETURN
//		true
//			ロックしないトランザクションだった
//		false
//			ロックするトランザクションだった
//
//	EXCEPTIONS

bool
Transaction::setNoLock(bool v)
{
	v = (v || Configuration::NoLock::get());
	ModSwap(_noLock, v);
	return v;
}

//	FUNCTION public
//	Trans::Transaction::setNoLog --
//		論理ログおよび UNDO ログを記録しないトランザクションかを設定する
//
//	NOTES
//		トランザクションを開始してから設定を変更できるが、
//		トランザクションマネージャの動作を理解した上で設定を変更しないと、
//		その場合の動作は保証できない
//
//	ARGUMENTS
//		bool				v
//			true
//				論理ログおよび UNDO ログを記録しないトランザクションにする
//			false
//				論理ログおよび UNDO ログを記録するトランザクションにする
//
//	RETURN
//		true
//			論理ログおよび UNDO ログを記録しないトランザクションだった
//		false
//			論理ログおよび UNDO ログを記録するトランザクションだった
//
//	EXCEPTIONS

bool
Transaction::setNoLog(bool v)
{
	v = (v || Configuration::NoLogicalLog::get());
	ModSwap(_noLog, v);
	return v;
}

//	FUNCTION public
//	Trans::Transaction::setBatchMode --
//		バッチモードのトランザクションかを設定する
//
//	NOTES
//		トランザクションを開始してから設定を変更できるが、
//		トランザクションマネージャの動作を理解した上で設定を変更しないと、
//		その場合の動作は保証できない
//
//	ARGUMENTS
//		bool				v
//			true
//				バッチモードのトランザクションにする
//			false
//				バッチモードでないトランザクションにする
//
//	RETURN
//		true
//			バッチモードでないトランザクションだった
//		false
//			バッチモードのトランザクションだった
//
//	EXCEPTIONS

bool
Transaction::setBatchMode(bool v)
{
	ModSwap(_batchMode, v);
	return v;
}

#ifdef OBSOLETE
//	FUNCTION public
//	Trans::Transaction::getDatabaseName --
//		トランザクションが操作するデータベースの名前を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた名前
//
//	EXCEPTIONS
//		なし

ModUnicodeString
Transaction::getDatabaseName() const
{
	ModUnicodeString name =
		getLogInfo(Log::File::Category::Database).getDatabaseName();
	return (name.getLength()) ?
		name : getLogInfo(Log::File::Category::System).getDatabaseName();
}
#endif

//	FUNCTION public
//	Trans::Transaction::isOverlapped --
//		版管理するトランザクションを開始したときに、
//		あるトランザクション識別子の表す更新トランザクションが
//		実行中だったか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction::ID&	id
//			実行中だったか調べる更新トランザクションのトランザクション識別子
//
//	RETURN
//		true
//			実行中だった
//		false
//			実行中でなかった
//
//	EXCEPTIONS

bool
Transaction::isOverlapped(const ID& id) const
{
	; _SYDNEY_ASSERT(!isNoVersion());

	if (getID() > id) {
		const ModVector<ID>& list = getStartingList();
		if (list.getSize()) {

			// 版管理するトランザクションを開始したときに
			// 実行中の更新トランザクションの
			// トランザクション識別子を管理するリストに
			// 指定されたトランザクション識別子があるか調べる
			//
			//【注意】	指定されたトランザクション識別子より
			//			自分自身のもののほうが大きければ、
			//			指定されたものの表すトランザクションは、
			//			自分自身より前に開始されたことがわかる

			ModVector<ID>::ConstIterator		ite(list.begin());
			const ModVector<ID>::ConstIterator&	end = list.end();

			do {
				if (*ite >= id)
					return *ite == id;
			} while (++ite != end) ;
		}
	}

	return false;
}

//	FUNCTION public
//	Trans::Transaction::isOverlapped --
//		版管理するトランザクションを開始したときに、
//		あるトランザクション識別子の表す更新トランザクションのうち、
//		ひとつでも実行中だったか調べる
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<Trans::Transaction::ID>&	ids
//			実行中だったか調べる更新トランザクションの
//			トランザクション識別子を要素として持つリストで、
//			要素は格納するトランザクション識別子の表す
//			トランザクションの開始時刻の昇順に並んでいる必要がある
//
//	RETURN
//		true
//			実行中だった
//		false
//			実行中でなかった
//
//	EXCEPTIONS

bool
Transaction::isOverlapped(const ModVector<ID>& ids) const
{
	; _SYDNEY_ASSERT(!isNoVersion());

	if (ids.getSize()) {
		const ModVector<ID>& list = getStartingList();
		if (list.getSize()) {

			// 版管理するトランザクションを開始したときに
			// 実行中の更新トランザクションの
			// トランザクション識別子を管理するリストに
			// 指定されたトランザクション識別子のうち、
			// 等しいものがひとつでもあるか調べる

			ModVector<ID>::ConstIterator		ite0(list.begin());
			const ModVector<ID>::ConstIterator&	end0 = list.end();
			ModVector<ID>::ConstIterator		ite1(ids.begin());
			const ModVector<ID>::ConstIterator&	end1 = ids.end();

			while (*ite1 < getID()) {
				if (*ite0 == *ite1)
					return true;

				if ((*ite0 < *ite1) ? (++ite0 == end0) : (++ite1 == end1))
					break;
			}
		}
	}

	return false;
}

//	FUNCTION public
//	Trans::Transaction::isInProgress --
//		あるトランザクション識別子が、現在実行中のトランザクションのうち、
//		ある種別のトランザクションのものか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction::ID&	id
//			現在実行中か調べるトランザクションのトランザクション識別子
//		Trans::Transaction::Category::Value	category
//			Trans::Transaction::Category::ReadWrite
//				現在実行中のすべての更新トランザクションの中から調べる
//			Trans::Transaction::Category::ReadOnly
//				現在実行中のすべての読取専用トランザクションの中から調べる
//			それ以外の値
//				現在実行中のすべてのトランザクションの中から調べる
//
//	RETURN
//		true
//			現在実行中のものがあった
//		false
//			現在実行中のものはない
//
//	EXCEPTIONS

// static
bool
Transaction::isInProgress(Schema::ObjectID::Value dbID,
						  const ID& id, Category::Value category)
{
	switch (category) {
	case Category::ReadWrite:
	case Category::ReadOnly:

		// 指定された種別の現在実行中のトランザクションの中に
		// 指定されたトランザクション識別子のものがあるか調べる

		return _List::exists(Transaction::getInProgressList(dbID, category),
							 id);
	}

	// 現在実行中のすべてのトランザクションの中に
	// 指定されたトランザクション識別子のものがあるか調べる
	//
	//【注意】	読取専用トランザクションのほうが
	//			一般的に多いはずなので、先に調べる

	return
	_List::exists(
		Transaction::getInProgressList(dbID, Category::ReadOnly), id) ||
	_List::exists(
		Transaction::getInProgressList(dbID, Category::ReadWrite), id);
}

#ifdef OBSOLETE
//	FUNCTION public
//	Trans::Transaction::isInProgress --
//		あるトランザクション識別子が、現在実行中のトランザクションのうち、
//		版管理の有無のいずれかのトランザクションのものか調べる
//
//	NOTES
//		Trans::Transaction::ID&	id
//			現在実行中か調べるトランザクションのトランザクション識別子
//		bool					noVersion
//			true
//				現在実行中の版管理しないトランザクションの中から調べる
//			false
//				現在実行中の版管理するトランザクションの中から調べる
//
//	RETURN
//		true
//			現在実行中のものがあった
//		false
//			現在実行中のものはない
//
//	EXCEPTIONS

// static
bool
Transaction::isInProgress(Schema::ObjectID::Value dbID,
						  const ID& id, bool noVersion)
{
	// 指定された版管理をするまたはしないトランザクションの中に
	// 指定されたトランザクション識別子のものがあるか調べる

	return _List::exists(Transaction::getInProgressList(dbID, noVersion), id);
}
#endif

//	FUNCTION public
//	Trans::Transaction::isInProgress --
//		複数のトランザクション識別子のいずれかが、
//		現在実行中のトランザクションのうち、
//		ある種別のトランザクションのもののか調べる
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<Trans::Transaction::ID>&	ids
//			現在実行中か調べるトランザクションの
//			トランザクション識別子を要素とするリストで、
//			要素であるトランザクション識別子は、それぞれの表す
//			トランザクションの開始時刻の昇順に並んでいる必要がある
//		Trans::Transaction::Category::Value	category
//			Trans::Transaction::Category::ReadWrite
//				現在実行中のすべての更新トランザクションの中から調べる
//			Trans::Transaction::Category::ReadOnly
//				現在実行中のすべての読取専用トランザクションの中から調べる
//			それ以外の値
//				現在実行中のすべてのトランザクションの中から調べる
//
//	RETURN
//		true
//			現在実行中のものがあった
//		false
//			現在実行中のものはない
//
//	EXCEPTIONS

// static
bool
Transaction::isInProgress(Schema::ObjectID::Value dbID,
						  const ModVector<ID>& ids, Category::Value category)
{
	switch (category) {
	case Category::ReadWrite:
	case Category::ReadOnly:

		// 指定された種別の現在実行中のトランザクションの中に
		// 指定されたトランザクション識別子のものがあるか調べる

		return _List::exists(
			Transaction::getInProgressList(dbID, category), ids);
	}

	// 現在実行中のすべてのトランザクションの中に
	// 指定されたトランザクション識別子のものがあるか調べる
	//
	//【注意】	読取専用トランザクションのほうが
	//			一般的に多いはずなので、先に調べる

	return
	_List::exists(
		Transaction::getInProgressList(dbID, Category::ReadOnly), ids) ||
	_List::exists(
		Transaction::getInProgressList(dbID, Category::ReadWrite), ids);
}

#ifdef OBSOLETE
//	FUNCTION public
//	Trans::Transaction::isInProgress --
//		複数のトランザクション識別子のいずれかが、
//		現在実行中のトランザクションのうち、
//		版管理の有無のいずれかのトランザクションのものか調べる
//
//	NOTES
//		ModVector<Trans::Transaction::ID>&	ids
//			現在実行中か調べるトランザクションの
//			トランザクション識別子を要素とするリストで、
//			要素であるトランザクション識別子は、それぞれの表す
//			トランザクションの開始時刻の昇順に並んでいる必要がある
//		bool					noVersion
//			true
//				現在実行中の版管理しないトランザクションの中から調べる
//			false
//				現在実行中の版管理するトランザクションの中から調べる
//
//	RETURN
//		true
//			現在実行中のものがあった
//		false
//			現在実行中のものはない
//
//	EXCEPTIONS

// static
bool
Transaction::isInProgress(Schema::ObjectID::Value dbID,
						  const ModVector<ID>& ids, bool noVersion)
{
	// 指定された版管理をするまたはしないトランザクションの中に
	// 指定されたトランザクション識別子のものがあるか調べる

	return _List::exists(Transaction::getInProgressList(dbID, noVersion), ids);
}
#endif

// FUNCTION public
//	Trans::Transaction::startBatchInsert -- 一括登録を開始する
//
// NOTES
//
// ARGUMENTS
//	const Schema::Table& cTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Transaction::
startBatchInsert(const Schema::Table& cTable_)
{
	Schema::Database& cDatabase = *cTable_.getDatabase(*this);
	// flush all the database files
	Checkpoint::Executor::cause(*this, cDatabase);
	// get current timestamp (used in rollback)
	_batchTimeStamp = Checkpoint::TimeStamp::getMostRecent(Lock::FileName(cTable_.getDatabaseID(),
																		  cTable_.getID(),
																		  cTable_.getID()));
	// record database id and table id
	_batchDatabaseID = cTable_.getDatabaseID();
	_batchTableID = cTable_.getID();

	// store logicallog data
	storeLog(Log::File::Category::Database,
			 Log::StartBatchData());
	flushLog(Trans::Log::File::Category::Database);

	// lock is not needed
	setNoLock(true);
}

// FUNCTION public
//	Trans::Transaction::commitBatchInsert -- 一括登録をコミットする
//
// NOTES
//  [OBSOLETE] CommitCountにより途中でコミットする場合はPlanモジュールから直接呼ばれる
//	Transaction::commitからも呼ばれる
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Transaction::
commitBatchInsert()
{
	; _SYDNEY_ASSERT(!_batchTimeStamp.isIllegal());
	; _SYDNEY_ASSERT(!_batchDatabaseID.isInvalid());
	; _SYDNEY_ASSERT(!_batchTableID.isInvalid());

	// get database object
	Schema::Database& cDatabase = *Schema::Database::get(_batchDatabaseID, *this);
	// flush all the database files
	Checkpoint::Executor::cause(*this, cDatabase);
	_batchTimeStamp = TimeStamp();
}

// FUNCTION public
//	Trans::Transaction::recoverBatch -- バッチモードの開始時にreceoverする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Transaction::
recoverBatch()
{
	; _SYDNEY_ASSERT(!_batchTimeStamp.isIllegal());
	; _SYDNEY_ASSERT(!_batchDatabaseID.isInvalid());
	; _SYDNEY_ASSERT(!_batchTableID.isInvalid());

	// get database ond table object from the ids
	Schema::Database* pDatabase =
		Schema::Database::get(_batchDatabaseID, *this);
	Schema::Table* pTable =
		Schema::Table::get(_batchTableID, pDatabase, *this);

	// recover all the files under the table
	pTable->recover(*this, _batchTimeStamp, pDatabase->getName());
}

//	FUNCTION public
//	Trans::Transaction::getBeginningID
//		-- オーバーラップしているトランザクションの内、
//		   最初に開始したトランザクションのIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Trans::ID&
//		最初に開始したトランザクションID
//
//	EXCEPTIONS

/* static */
ID
Transaction::getBeginningID(Schema::ObjectID::Value dbID)
{
	TransactionInformation* info
		= _Transaction::getTransactionInformation(dbID);
	Os::AutoCriticalSection cAuto(_Transaction::_latch2);
	return info->getBeginningID();
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2014, 2015, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
