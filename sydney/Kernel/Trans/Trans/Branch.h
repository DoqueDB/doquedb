// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Branch.h -- トランザクションブランチ関連のクラス定義、関数宣言
// 
// Copyright (c) 2006, 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_TRANS_BRANCH_H
#define __SYDNEY_TRANS_BRANCH_H

#include "Trans/Module.h"
#include "Trans/Transaction.h"
#include "Trans/XID.h"

#include "Os/CriticalSection.h"
#include "Os/Semaphore.h"

#include "Schema/ObjectID.h"

#include "Server/SessionID.h"

template <class T>
class ModAutoPointer;
template <class T>
class ModVector;

_SYDNEY_BEGIN

namespace Checkpoint
{
namespace Log
{
	class CheckpointSystemData;
}
}

_SYDNEY_TRANS_BEGIN

class Literal;

namespace Log
{
	class BranchForgetData;
	class BranchHeurDecideData;
}
namespace Manager
{
	class Branch;
}

//	CLASS
//	Trans::Branch -- トランザクションブランチを表す型
//
//	NOTES

class Branch
{
	friend class Transaction;
	friend class Manager::Branch;
	friend class ModAutoPointer<Branch>;
public:
	//	TYPEDEF
	//	Trans::Branch::ID -- トランザクションブランチ識別子を表す型
	//
	//	NOTES

	typedef Trans::XID		ID;

	struct Status
	{
		//	ENUM
		//	Trans::Branch::Status::Value --
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
			// 中断中
			Suspended,
			// 中断中(他セッションへ連係を変更可)
			SuspendedForMigrate,
			// 待機中
			Idle,
			// コミット準備完了
			Prepared,
			// ロールバックのみ可
			RollbackOnly,
			// ヒューリスティックな解決済
			HeuristicallyCompleted,
			// 値の数
			Count
		};
	};

	struct HeurDecision
	{
		//	ENUM
		//	Trans::Branch::HeurDecision::Value --
		//		トランザクションブランチのヒューリスティックな解決の仕方を表す値
		//
		//	NOTES

		enum Value
		{
			// 不明
			Unknown =		0,
			// コミット
			Commit,
			// ロールバック
			Rollback,
			// 一部コミット・一部ロールバック
			Mix,
			// 値の数
			Count
		};
	};

	// トランザクションブランチ記述子を生成する
	SYD_TRANS_FUNCTION
	static Branch*
	attach(const ID& id);
	// トランザクションブランチ記述子を破棄する
	SYD_TRANS_FUNCTION
	static void
	detach(Branch*& branch);

	// トランザクションブランチを開始する

	SYD_TRANS_FUNCTION
	void
	start(Schema::ObjectID::Value databaseID,
		  Server::SessionID sessionID, const Transaction::Mode& mode);

	struct StartOption
	{
		//	TYPEDEF
		//	Trans::Branch::StartOption::Value --
		//		Trans::Branch::start に与えるオプションを表す型
		//
		//	NOTES

		enum Value
		{
			// 不明
			Unknown =		0,
			// JOIN
			Join,
			// RESUME
			Resume,
			// 値の数
			Count
		};
	};

	// トランザクションブランチを再開・合流する
	SYD_TRANS_FUNCTION
	void
	start(Schema::ObjectID::Value databaseID,
		  Server::SessionID sessionID, StartOption::Value option);

	struct EndOption
	{
		//	TYPEDEF
		//	Trans::Branch::EndOption::Value --
		//		Trans::Branch::end に与えるオプションを表す型
		//
		//	NOTES

		enum Value
		{
			// 不明
			Unknown =		0,
			// SUSPEND
			Suspend,
			// SUSPEND FOR MIGRATE
			Migrate,
			// 値の数
			Count
		};
	};

	// トランザクションブランチを待機・中断する
	SYD_TRANS_FUNCTION
	void
	end(Server::SessionID sessionID,
		EndOption::Value option = EndOption::Unknown);

	// トランザクションブランチをコミット準備する
	SYD_TRANS_FUNCTION
	void
	prepare();
	// トランザクションブランチをコミットする
	SYD_TRANS_FUNCTION
	void
	commit(bool inOnePhase);
	// トランザクションブランチをロールバックする
	SYD_TRANS_FUNCTION
	void
	rollback();

	// ヒューリスティックに解決済のトランザクションブランチを抹消する
	SYD_TRANS_FUNCTION
	void
	forget();

	// トランザクションブランチ識別子を得る
	const ID&
	getID() const;
	// ある状態のトランザクションブランチのトランザクションブランチ識別子を得る
	static void
	getID(Status::Value status, ModVector<ID>& result);
	SYD_TRANS_FUNCTION
	static void
	getID(Server::SessionID sessionID,
		  Status::Value status, ModVector<ID>& result);

	// トランザクションブランチと連携中のセッションのセッション識別子を得る
	Server::SessionID
	getAssociatedID() const;

	//	CLASS
	//	Trans::Branch::HeurCompletionInfo --
	//		ヒューリスティックに解決された
	//		トランザクションブランチに関する情報を表すクラス
	//
	//	NOTES

	struct HeurCompletionInfo
	{
		// デフォルトコンストラクター
		HeurCompletionInfo()
			: _decision(HeurDecision::Unknown)
		{}
		// コンストラクター
		HeurCompletionInfo(const ID& id, HeurDecision::Value decision)
			: _id(id),
			  _decision(decision)
		{}

		// ヒューリスティックに解決された
		// トランザクションブランチのトランザクションブランチ識別子
		ID					_id;
		// どのようにしてヒューリスティックに解決したか
		HeurDecision::Value	_decision;
	};

	// ヒューリスティックに解決済のすべてのトランザクションブランチの情報を得る
	SYD_TRANS_FUNCTION
	static void
	getHeurCompletionInfo(ModVector<HeurCompletionInfo>& result);

	// トランザクションブランチの
	// トランザクション処理を行うためのトランザクション記述子を得る
	Transaction&
	getTransaction();

	// 子のトランザクションブランチ識別子を得る
	const ID& getChildID() const { return _childID; }
	// 子のトランザクションブランチ識別子を設定する
	void setChildID(const ID& id_) { _childID = id_; }

	// 障害回復のために REDO する
	SYD_TRANS_FUNCTION
	static void
	redo(const Checkpoint::Log::CheckpointSystemData& logData);
	SYD_TRANS_FUNCTION
	static void
	redo(const Log::BranchHeurDecideData& logData);
	SYD_TRANS_FUNCTION
	static void
	redo(const Log::BranchForgetData& logData);

private:
	// コンストラクター
	Branch(const ID& id);
	// デストラクター
	~Branch();

	// トランザクションブランチをヒューリスティックに解決する
	void
	decideHeuristically();

	// あるセッションにより『データ操作中』の
	// トランザクションブランチの有無を調べる
	static bool
	getExistenceOfActive(Server::SessionID sessionID);

	// 排他制御用のラッチ
	mutable Os::CriticalSection	_latch;
	// 連係中中のセッションをひとつに限定するためのセマフォ
	Os::Semaphore			_semaphore;

	// トランザクションブランチ識別子
	const ID				_id;
	// 参照回数
	mutable unsigned int	_refCount;

	// トランザクションブランチと連係中のセッションのセッション識別子
	Server::SessionID		_associatedID;
	// 実行状況
	Status::Value			_status;
	// どのようにしてヒューリスティックに解決したか
	HeurDecision::Value		_heurDecision;
	// トランザクションブランチの
	// トランザクション処理を行うためのトランザクション記述子
	Transaction				_transaction;

	// 子サーバのトランザクションブランチ識別子(分散マネージャで利用)
	ID						_childID;

	// ハッシュリストでの直前の要素へのポインタ
	Branch*					_hashPrev;
	// ハッシュリストでの直後の要素へのポインタ
	Branch*					_hashNext;
};

//	FUNCTION private
//	Trans::Branch::Branch --
//		トランザクションブランチ記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Branch::ID&	id
//			生成するトランザクションブランチ記述子の表す
//			トランザクションブランチのトランザクションブランチ識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
Branch::Branch(const ID& id)
	: _semaphore(1),
	  _id(id),
	  _refCount(0),
	  _associatedID(Server::IllegalSessionID),
	  _status(Status::NonExistent),
	  _heurDecision(HeurDecision::Unknown),
	  _transaction(id),
	  _hashPrev(0),
	  _hashNext(0)
{}

//	FUNCTION private
//	Trans::Branch::~Branch --
//		トランザクションブランチ記述子を表すクラスのデストラクター
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
Branch::~Branch()
{}

//	FUNCTION public
//	Trans::Branch::getID -- トランザクションブランチ識別子を得る
//
//	NOTES
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
const Branch::ID&
Branch::getID() const
{
	return _id;
}

//	FUNCTION public
//	Trans::Branch::getID --
//		ある状態のトランザクションブランチの
//		トランザクションブランチ識別子をすべて求める
//
//	NOTES
//		Trans::Branch::getID(Server::IllegalSessionID, status, result)
//		と同じ動作となる
//
//	ARGUMENTS
//		Trans::Branch::Status::Value	status
//			Trans::Branch::Status::Unknown 以外の値
//				この状態のトランザクションブランチの中から求める
//			Trans::Branch::Status::Unknown
//				トランザクションブランチの状態は考慮しない
//		ModVector<Trans::Branch::ID>&	result
//			求めたトランザクションブランチ識別子を要素として持つ配列
//
//	RETURN
//		なし

// static
inline
void
Branch::getID(Status::Value status, ModVector<ID>& result)
{
	getID(Server::IllegalSessionID, status, result);
}

//	FUNCTION public
//	Trans::Branch::getAssociatedID --
//		トランザクションブランチと連携している、または、
//		最後に連携していたセッションのセッション識別子を得る
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
Branch::getAssociatedID() const
{
	return _associatedID;
}

//	FUNCTION public
//	Trans::Branch::getTransaction -- 
//		トランザクションブランチの
//		トランザクション処理を行うためのトランザクション記述子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたトランザクション記述子
//
//	EXCEPTIONS
//		なし

inline
Transaction&
Branch::getTransaction()
{
	return _transaction;
}

_SYDNEY_TRANS_END
_SYDNEY_END

#endif //__SYDNEY_TRANS_BRANCH_H

//
//	Copyright (c) 2006, 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
