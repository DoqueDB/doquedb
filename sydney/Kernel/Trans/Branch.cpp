// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Branch.cpp -- トランザクションブランチ記述子関連の関数定義
// 
// Copyright (c) 2007, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#include "Trans/AutoBranch.h"
#include "Trans/AutoLatch.h"
#include "Trans/AutoTransaction.h"
#include "Trans/Branch.h"
#include "Trans/Configuration.h"
#include "Trans/Manager.h"
#include "Trans/LogData.h"

#include "Checkpoint/LogData.h"
#include "Common/Assert.h"
#include "Common/DoubleLinkedList.h"
#include "Common/HashTable.h"
#include "Exception/AlreadyBeginTransaction.h"
#ifdef OBSOLETE
#else
#include "Exception/NotSupported.h"
#endif
#include "Exception/XA_DuplicateIdentifier.h"
#include "Exception/XA_HeurCommit.h"
#include "Exception/XA_HeurMix.h"
#include "Exception/XA_HeurRollback.h"
#include "Exception/XA_InsideActiveBranch.h"
#include "Exception/XA_InvalidIdentifier.h"
#include "Exception/XA_ProtocolError.h"
#include "Exception/XA_UnknownIdentifier.h"

_SYDNEY_USING
_SYDNEY_TRANS_USING

namespace
{

typedef	Common::HashTable<
	Common::DoubleLinkedList<Branch>, Branch>	HashTable;

namespace _Branch
{
	// 生成済のトランザクションブランチ記述子を探す
	Branch*
	find(HashTable::Bucket& bucket, const Branch::ID& id);

	// すべてのトランザクションブランチ記述子を管理する
	// ハッシュ表に登録するためのハッシュ値を計算する
	unsigned int
	branchTableHash(const Branch::ID& id);

	// 以下の情報を保護するためのラッチ
	Os::CriticalSection		_latch;

	// すべてのトランザクションブランチ記述子を管理するハッシュ表
	HashTable*				_branchTable = 0;
}

//	FUNCTION
//	$$$::_Branch::find -- 生成済のトランザクションブランチ記述子を探す
//
//	NOTES
//
//	ARGUMENTS
//		HashTable::Bucket&	bucket
//			トランザクションブランチ記述子が格納されるべきハッシュ表のバケット
//		Trans::Branch::ID&	id
//			このトランザクションブランチ識別子のトランザクションブランチの
//			トランザクションブランチ記述子を探す
//
//	RETURN
//		0 以外の値
//			得られたトランザクションブランチ記述子を格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPTIONS
//		なし

inline
Branch*
_Branch::find(HashTable::Bucket& bucket, const Branch::ID& id)
{
	switch (bucket.getSize()) {
	default:
	{
		// バケットに登録されているトランザクションブランチ記述子のうち、
		// 与えられたトランザクションブランチ識別子のものを探す

		HashTable::Bucket::Iterator			begin(bucket.begin());
		HashTable::Bucket::Iterator			ite(begin);
		const HashTable::Bucket::Iterator&	end = bucket.end();

		do {
			Branch& branch = *ite;

			if (branch.getID() == id) {

				// 見つかったトランザクションブランチ記述子を
				// バケットの先頭に移動して、
				// 最近に参照されたものほど、見つかりやすくする

				bucket.splice(begin, bucket, ite);

				return &branch;
			}
		} while (++ite != end) ;

		break;
	}
	case 1:
	{
		Branch& branch = bucket.getFront();

		if (branch.getID() == id)

			// 見つかった

			return &branch;

		break;
	}
	case 0:
		break;
	}

	// 見つからなかった

	return 0;
}

//	FUNCTION
//	$$$::_Branch::branchTableHash --
//		すべてのトランザクションブランチ記述子を管理するハッシュ表に
//		登録するためのハッシュ値を計算する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Branch::ID&	id
//			ハッシュ表に登録するトランザクションブランチ記述子の
//			トランザクションブランチ識別子
//
//	RETURN
//		得られたハッシュ値
//
//	EXCEPTIONS
//		なし

inline
unsigned int
_Branch::branchTableHash(const Branch::ID& id)
{
	; _SYDNEY_ASSERT(!(id.isNull() || id.isIllegal()));

	const Common::BinaryData& gtrID = id.getGlobalTransactionID();

	const unsigned char* p =
		syd_reinterpret_cast<const unsigned char*>(gtrID.getValue());
	const unsigned char* q = p + gtrID.getSize();

	unsigned int i = 0;
	for (; p < q; ++p)
		i += static_cast<unsigned int>(*p);

	return i;
}

}

//	FUNCTION private
//	Trans::Manager::Branch::initialize --
//		マネージャーの初期化のうち、
//		トランザクションブランチ記述子関連のものを行う
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
Manager::Branch::initialize()
{
	try {
		// すべてのトランザクションブランチ記述子を管理するハッシュ表を生成する

		_Branch::_branchTable =
			new HashTable(Configuration::BranchTableSize::get(),
						  &Trans::Branch::_hashPrev,
						  &Trans::Branch::_hashNext);
		; _SYDNEY_ASSERT(_Branch::_branchTable);

	} catch (...) {

		Manager::Branch::terminate();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Trans::Manager::Branch::prepareTermination --
//		マネージャーの後処理のための準備のうち、
//		トランザクションブランチ記述子関連のものを行う
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
Manager::Branch::prepareTermination()
{
	//【注意】	他のスレッドが同時に実行されることはないので、
	//			ラッチしない

	; _SYDNEY_ASSERT(_Branch::_branchTable);

	unsigned int i = 0;
	do {
		HashTable::Bucket& bucket = _Branch::_branchTable->getBucket(i);

		HashTable::Bucket::Iterator			ite(bucket.begin());
		const HashTable::Bucket::Iterator&	end = bucket.end();

		for (; ite != end; ++ite) {
			Trans::Branch& branch = *ite;

			switch (branch._status) {
			case Trans::Branch::Status::Idle:
			case Trans::Branch::Status::RollbackOnly:

				//『待機中』・『ロールバックのみ可』の状態の
				// トランザクションブランチはロールバックする

				branch.rollback();
				break;

			case Trans::Branch::Status::Prepared:

				//『コミット準備済』の状態のトランザクションブランチは
				// ヒューリステックな解決を行い、ロールバックする

				branch.decideHeuristically();
				break;

			case Trans::Branch::Status::HeuristicallyCompleted:
				break;

			default:
				; _SYDNEY_ASSERT(false);
			}
		}
	} while (++i < _Branch::_branchTable->getLength()) ;
}

//	FUNCTION private
//	Trans::Manager::Branch::terminate --
//		マネージャーの後処理のうち、
//		トランザクションブランチ記述子関連のものを行う
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
Manager::Branch::terminate()
{
	//【注意】	他のスレッドが同時に実行されることはないので、
	//			ラッチしない

	if (_Branch::_branchTable) {

		// すべてのトランザクションブランチ記述子を管理するハッシュ表に
		// 登録されているトランザクションブランチ記述子があれば、破棄する

		unsigned int i = 0;
		do {
			HashTable::Bucket& bucket = _Branch::_branchTable->getBucket(i);

			while (bucket.getSize()) {
				Trans::Branch* branch = &bucket.getFront();
				bucket.popFront();
#ifdef DEBUG
				switch (branch->_status) {
				case Trans::Branch::Status::NonExistent:
				case Trans::Branch::Status::HeuristicallyCompleted:
					break;
				default:
					; _SYDNEY_ASSERT(false);
				}
#endif
				delete branch;
			}
		} while (++i < _Branch::_branchTable->getLength()) ;

		// すべてのトランザクションブランチ記述子を管理するハッシュ表を破棄する

		delete _Branch::_branchTable, _Branch::_branchTable = 0;
	}
}

//	FUNCTION public
//	Trans::Branch::attach -- トランザクションブランチ記述子を得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Branch::ID&	id
//			生成するトランザクションブランチ記述子の表す
//			トランザクションブランチのトランザクションブランチ識別子
//
//	RETURN
//		得られたトランザクションブランチ記述子を格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Branch*
Branch::attach(const ID& id)
{
	if (id.isNull() || id.isIllegal())

		// 不正なトランザクションブランチ識別子が与えられた

		_SYDNEY_THROW0(Exception::XA_InvalidIdentifier);

	// トランザクションブランチ記述子の生成・破棄に関する情報を
	// 保護するためのラッチをかける

	Os::AutoCriticalSection latch(_Branch::_latch);

	// 与えられたトランザクションブランチ識別子の
	// トランザクションブランチ記述子を
	// 格納すべきハッシュ表のバケットを求める

	; _SYDNEY_ASSERT(_Branch::_branchTable);

	const unsigned int addr =
		_Branch::branchTableHash(id) % _Branch::_branchTable->getLength();
	HashTable::Bucket& bucket = _Branch::_branchTable->getBucket(addr);

	// 与えられたトランザクションブランチ識別子の表す
	// トランザクションブランチのトランザクションブランチ記述子が
	// 求めたバケットに登録されていれば、それを得る

	Branch* branch = _Branch::find(bucket, id);
	if (branch)

		// 見つかったので、参照回数を 1 増やす

		++branch->_refCount;
	else {

		// 見つからなかったので、生成する

		branch = new Branch(id);
		; _SYDNEY_ASSERT(branch);

		// 参照回数を 1 にする

		branch->_refCount = 1;

		// ハッシュ表のバケットの先頭に挿入して、
		// 最近に参照されたものほど、見つかりやすくする

		bucket.pushFront(*branch);
	}

	return branch;
}

//	FUNCTION public
//	Trans::Branch::detach -- トランザクションブランチ記述子の参照をやめる
//
//	NOTES
//		トランザクションブランチ記述子の参照をやめても、
//		他のどこかで参照されているか、トランザクションブランチが
//		存在しない状態でなければ、トランザクションブランチ記述子は破棄されない
//		逆にどこからも参照されておらず、トランザクションブランチが
//		存在しない状態であれば、トランザクションブランチ記述子は直ちに破棄される
//
//	ARGUMENTS
//		Trans::Branch*&		branch
//			参照をやめるトランザクションブランチ記述子を
//			格納する領域の先頭アドレス
//			呼び出しから返ると 0 になる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Branch::detach(Branch*& branch)
{
	if (branch) {
		Branch* p = 0;

		// トランザクションブランチ記述子の生成・破棄に関する情報を
		// 保護するためのラッチをかける

		Os::AutoCriticalSection latch(_Branch::_latch);

		; _SYDNEY_ASSERT(branch->_refCount);

		// 参照回数を 1 減らす

		if (!--branch->_refCount) {

			// どこからも参照されなくなる

			Os::AutoCriticalSection latch(branch->_latch);

			if (branch->_status == Status::NonExistent) {

				// 与えられたトランザクションブランチ記述子を格納する
				// ハッシュ表のバケットを求め、
				// トランザクションブランチ記述子を取り除く

				; _SYDNEY_ASSERT(_Branch::_branchTable);

				const unsigned int addr =
					_Branch::branchTableHash(branch->getID()) %
						_Branch::_branchTable->getLength();
				_Branch::_branchTable->getBucket(addr).erase(*branch);

				// アンラッチした後、破棄するようにする

				p = branch;
			}
		}

		if (p)

			// トランザクションブランチ記述子を破棄する

			delete p;

		// 与えられたポインタは 0 を指すようにする

		branch = 0;
	}
}

//	FUNCTION public
//	Trans::Branch::start -- トランザクションブランチを開始する
//
//	NOTES
//
//	ARGUMENTS
//		Server::SessionID		sessionID
//			トランザクションブランチを開始するセッションのセッション識別子
//		Trans::Transaction::Mode&	mode
//			開始するトランザクションブランチのトランザクションモード
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::AlreadyBeginTransaction
//			与えられたセッション識別子の表すセッションでは
//			すでにトランザクションを実行中である
//		Exception::XA_DuplicateIdentifier
//			『データ操作中』・『中断中』・『待機中』の
//			トランザクションブランチを開始しようとした
//		Exception::XA_InsideActiveBranch
//			与えられたセッション識別子の表すセッションには
//			『データ操作中』のトランザクションブランチが存在する
//		Exception::XA_ProtocolError
//			開始できない状態のトランザクションブランチを開始しようとした

void
Branch::start(Schema::ObjectID::Value databaseID_,
			  Server::SessionID sessionID, const Transaction::Mode& mode)
{
	; _SYDNEY_ASSERT(sessionID != Server::IllegalSessionID);

	if (getExistenceOfActive(sessionID))

		// 与えられたセッション識別子の表すセッションには
		//『データ操作中』のトランザクションブランチが存在する

		_SYDNEY_THROW0(Exception::XA_InsideActiveBranch);
	{
	AutoTransaction trans(Transaction::attach(sessionID));
	if (trans->isInProgress())

		// 与えられたセッション識別子の表すセッションでは
		// すでにトランザクションを実行中である

		_SYDNEY_THROW0(Exception::AlreadyBeginTransaction);
	}

	Os::AutoCriticalSection latch(_latch);

	switch (_status) {
	case Status::NonExistent:
		break;

	case Status::Active:
	case Status::Suspended:
	case Status::SuspendedForMigrate:
	case Status::Idle:

		//『データ操作中』・『中断中』・『待機中』の
		// トランザクションブランチを開始しようとした

		_SYDNEY_THROW0(Exception::XA_DuplicateIdentifier);

	default:

		// それ以外の状態(『コミット準備済』・
		//『ロールバックのみ可』・『ヒューリスティックな解決済』) の
		// トランザクションブランチを開始しようとした

		_SYDNEY_THROW0(Exception::XA_ProtocolError);
	}

	// ひとつのトランザクションブランチには
	// 同時にひとつのセッションしか連係できないようにセマフォをロックしておく
	//
	//【注意】	トランザクションブランチが『存在しない』ので、
	//			ここではブロックしない

	_semaphore.lock();

	try {
		// トランザクションブランチの
		// トランザクション処理を行うためのトランザクションを開始する

		getTransaction().begin(databaseID_, mode, Transaction::Type::Branch);

	} catch (...) {

		// セマフォのロックをはずす

		_semaphore.unlock();
		_SYDNEY_RETHROW;
	}

	// トランザクションブランチと指定された
	// セッション識別子の表すセッションを連係させる

	_associatedID = sessionID;

	// トランザクションにセッションを連係させる

	getTransaction().setSessionID(sessionID);

	// トランザクションブランチの状態を『データ操作中』にする

	_status = Status::Active;
}

//	FUNCTION public
//	Trans::Branch::start -- トランザクションブランチを再開・合流する
//
//	NOTES
//
//	ARGUMENTS
//		Server::SessionID		sessionID
//			トランザクションブランチを再開・合流するセッションのセッション識別子
//		Trans::Branch::StartOption::Value	option
//			Trans::Branch::StartOption::Join
//				トランザクションブランチが『待機中』になるまで待ち、合流する
//			Trans::Branch::StartOption::Resume
//				トランザクションブランチが『中断中』であれば、再開する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::AlreadyBeginTransaction
//			与えられたセッション識別子の表すセッションでは
//			すでにトランザクションを実行中である
#ifdef OBSOLETE
//		Exception::NotSupported
//			このメソッドはサポートされていない
#else
//		Exception::XA_UnknownIdentifier
//			存在しないトランザクションブランチに合流・再開しようとした
//		Exception::XA_ProtocolError
//			与えられたセッション識別子の表すセッションにより中断されている
//			トランザクションブランチに合流しようとした
//			または、与えられたセッション識別子によってFOR MIGRATEを指定されずに
//			中断されているトランザクションブランチを再開しようとした
//			または、合流・再開できない状態のトランザクションブランチに
//			合流・再開しようとした
#endif
//		Exception::XA_InsideActiveBranch
//			与えられたセッション識別子の表すセッションには
//			『データ操作中』のトランザクションブランチが存在する
//			

void
Branch::start(Schema::ObjectID::Value databaseID,
			  Server::SessionID sessionID, StartOption::Value option)
{
	; _SYDNEY_ASSERT(sessionID != Server::IllegalSessionID);

	if (getExistenceOfActive(sessionID))

		// 与えられたセッション識別子の表すセッションには
		//『データ操作中』のトランザクションブランチが存在する

		_SYDNEY_THROW0(Exception::XA_InsideActiveBranch);
	{
	AutoTransaction trans(Transaction::attach(sessionID));
	if (trans->isInProgress())

		// 与えられたセッション識別子の表すセッションでは
		// すでにトランザクションを実行中である

		_SYDNEY_THROW0(Exception::AlreadyBeginTransaction);
	}
#ifdef OBSOLETE
	switch (option) {
	case StartOption::Join:
		do {
			{
			Os::AutoCriticalSection latch(_latch);

			switch (_status) {
			case Status::NonExistent:

				// トランザクションブランチは『存在しない』

				_SYDNEY_THROW0(Exception::XA_UnknownIdentifier);

			case Status::Suspended:
			case Status::SuspendedForMigrate:
				if (getAssociatedID() == sessionID)

					// トランザクションブランチは与えられた
					// セッション識別子の表すセッションにより
					// 『中断中』にされている

					_SYDNEY_THROW0(Exception::XA_ProtocolError);
				// thru

			case Status::Active:

				// 他のセッションがトランザクションブランチと連係している

				; _SYDNEY_ASSERT(getAssociatedID() != sessionID);
				break;

			case Status::Idle:
			
				// トランザクションブランチに同時にひとつのセッションしか
				// 連係できないようにセマフォをロックしておく

				if (_semaphore.trylock()) {

					// トランザクションブランチと指定された
					// セッション識別子の表すセッションを連係させる

					_associatedID = sessionID;

					// トランザクションにセッションを連係させる
					
					getTransaction().setSessionID(sessionID);

					// トランザクションブランチの状態を『データ操作中』にする

					_status = Status::Active;
					return;
				}
				break;

			default:

				// それ以外の状態(『コミット準備済』・
				//『ロールバックのみ可』・
				//『ヒューリスティックな解決済』) の
				// トランザクションブランチに合流しようとした

				_SYDNEY_THROW0(Exception::XA_ProtocolError);
			}

			// セマフォをロックできなかった、
			// つまり、他のセッションがトランザクションブランチと連係している
			}
			// そこで、トランザクションブランチ記述子を保護するための
			// ラッチをはずし、セマフォをロックできるようになるまで待つ

			Os::AutoSemaphore(_semaphore);

		} while (true) ;

	case StartOption::Resume:
	{
		Os::AutoCriticalSection latch(_latch);
		
		switch (_status) {
		case Status::NonExistent:

			// トランザクションブランチは『存在しない』

			_SYDNEY_THROW0(Exception::XA_UnknownIdentifier);

		case Status::Suspended:
			if (getAssociatedID() != sessionID)

				// FOR MIGRATEが指定されずに中断されている
				// トランザクションブランチを、
				// 中断したセッションと異なるセッションが再開しようとしている

				_SYDNEY_THROW0(Exception::XA_ProtocolError);
			// thru

		case Status::SuspendedForMigrate:

			// トランザクションブランチと指定された
			// セッション識別子の表すセッションを連係させる

			_associatedID = sessionID;

			// トランザクションにセッションを連係させる

			getTransaction().setSessionID(sessionID);

			// トランザクションブランチの状態を『データ操作中』にする

			_status = Status::Active;
			return;

		default:

			// それ以外の状態(『データ操作中』・『待機中』・
			// 『コミット準備済』・『ロールバックのみ可』・
			// 『ヒューリスティックな解決済』) の
			// トランザクションブランチに再開しようとした

			_SYDNEY_THROW0(Exception::XA_ProtocolError);
		}
	}
	default:
		; _SYDNEY_ASSERT(false);
	}
#else
	// 現状、サポートしていないオプションが指定された

	_SYDNEY_THROW0(Exception::NotSupported);
#endif
}

//	FUNCTION public
//	Trans::Branch::end -- トランザクションブランチを待機・中断する
//
//	NOTES
//
//	ARGUMENTS
//		Server::SessionID		sessionID
//			トランザクションブランチを待機・中断するセッションのセッション識別子
//		Trans::Branch::EndOption::Value	option
#ifdef OBSOLETE
//			Trans::Branch::EndOption::Suspend
//				トランザクションブランチが『データ操作中』であれば中断する。
//				他のセッションからは再開できない
//			Trans::Branch::EndOption::Migrate
//				トランザクションブランチが『データ操作中』であれば、
//				他のセッションからも再開可能なように中断する
#else
//			Trans::Branch::EndOption::Suspend
//				このオプションはサポートされていない
//			Trans::Branch::EndOption::Migrate
//				このオプションはサポートされていない
#endif
//			Trans::Branch::EndOption::Unknown または指定されないとき
//				トランザクションブランチが『データ操作中』・『中断中』であれば、
//				待機状態にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS
#ifdef OBSOLETE
//		Exception::NotSupported
//			サポートされていないオプションが指定された
#endif
//		Exception::XA_ProtocolError
//			待機・中断できない状態のトランザクションブランチを待機・中断を試みた
//		Exception::XA_UnknownIdentifier
//			対象のトランザクションブランチは『存在しない』

void
Branch::end(Server::SessionID sessionID, EndOption::Value option)
{
	; _SYDNEY_ASSERT(sessionID != Server::IllegalSessionID);

	Os::AutoCriticalSection latch(_latch);

	if (_status == Status::NonExistent)

		// トランザクションブランチは『存在しない』

		_SYDNEY_THROW0(Exception::XA_UnknownIdentifier);

	if (getAssociatedID() == sessionID)

		// このトランザクションブランチと連係しているセッションは
		// 指定されたセッション識別子の表すセッションなので、処理できる

		switch (option) {
		case EndOption::Unknown:
			switch (_status) {
			case Status::Active:
			case Status::Suspended:
			case Status::SuspendedForMigrate:

				// トランザクションから連係するセッションをなくす

				getTransaction().setSessionID(Server::IllegalSessionID);

				// 自他のセッションが連係できるように、セマフォのロックをはずす

				_semaphore.unlock();

				//【注意】	ここで実際にはこのトランザクションブランチと連携する
				//			セッションはなくなるわけだが、過去に連携していた
				//			セッションとしてまだセッション識別子を覚えておく。
				//			連携の有無はトランザクションブランチの状態で判断

				// トランザクションブランチの状態を『待機中』にする

				_status = Status::Idle;
				return;
			}
			break;

		case EndOption::Suspend:
#ifdef OBSOLETE
			switch (_status) {
			case Status::Active:

				// トランザクションブランチの状態を『中断中』にする

				_status = Status::Suspended;
				return;
			}
			break;
#else
			// 現状、サポートしていないオプションが指定された

			_SYDNEY_THROW0(Exception::NotSupported);
#endif
		case EndOption::Migrate:
#ifdef OBSOLETE
			switch (_status) {
			case Status::Active:

				// トランザクションブランチの状態を
				//『中断中(他セッションに連係を変更可)』にする

				_status = Status::SuspendedForMigrate;
				return;
			}
			break;
#else
			// 現状、サポートしていないオプションが指定された

			_SYDNEY_THROW0(Exception::NotSupported);
#endif
		default:
			; _SYDNEY_ASSERT(false);
		}

	// それ以外の状態のトランザクションブランチを待機しようとした

	_SYDNEY_THROW0(Exception::XA_ProtocolError);
}

//	FUNCTION public
//	Trans::Branch::prepare -- トランザクションブランチをコミット準備する
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
//		Exception::XA_ProtocolError
//			コミット準備できない状態のトランザクションブランチの
//			コミット準備を試みた
//		Exception::XA_UnknownIdentifier
//			対象のトランザクションブランチは『存在しない』

void
Branch::prepare()
{
	Os::AutoCriticalSection latch(_latch);

	switch (_status) {
	case Status::Idle:
		break;

	case Status::NonExistent:

		// トランザクションブランチは『存在しない』

		_SYDNEY_THROW0(Exception::XA_UnknownIdentifier);

	default:

		// それ以外の状態(『待機中』以外)の
		// トランザクションブランチを待機しようとした

		_SYDNEY_THROW0(Exception::XA_ProtocolError);
	}

	// トランザクションブランチの
	// トランザクション処理を行うためのトランザクションのコミットを準備する

	getTransaction().prepare();

	// トランザクションブランチの状態を『コミット準備済』にする

	_status = Status::Prepared;
}

//	FUNCTION public
//	Trans::Branch::commit -- トランザクションブランチをコミットする
//
//	NOTES
//
//	ARGUMENTS
//		bool		inOnePhase
//			true
//				コミット準備済でなく、待機中のトランザクションをコミットする
//			false
//				コミット準備済のトランザクションをコミットする
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::XA_HeurCommit
//			トランザクションブランチは既にコミットして
//			ヒューリスティックに解決されている
//		Exception::XA_HeurRollback
//			トランザクションブランチは既にロールバックして
//			ヒューリスティックに解決されている
//		Exception::XA_HeurMix
//			トランザクションブランチは既に一部コミットし一部ロールバックして
//			ヒューリスティックに解決されている
//		Exception::XA_ProtocolError
//			コミットできない状態のトランザクションブランチのコミットを試みた
//		Exception::XA_UnknownIdentifier
//			対象のトランザクションブランチは『存在しない』

void
Branch::commit(bool inOnePhase)
{
	Os::AutoCriticalSection latch(_latch);

	switch (_status) {
	case Status::NonExistent:

		// トランザクションブランチは『存在しない』

		_SYDNEY_THROW0(Exception::XA_UnknownIdentifier);

	case Status::HeuristicallyCompleted:

		// どういうふうにヒューリスティックに
		// トランザクションブランチが解決されたかを表す例外を投げる

		switch (_heurDecision) {
		case HeurDecision::Commit:
			_SYDNEY_THROW0(Exception::XA_HeurCommit);
		case HeurDecision::Rollback:
			_SYDNEY_THROW0(Exception::XA_HeurRollback);
		case HeurDecision::Mix:
			_SYDNEY_THROW0(Exception::XA_HeurMix);
		default:
			; _SYDNEY_ASSERT(false);
		}

	case Status::Prepared:
		break;
	case Status::Idle:
		if (inOnePhase)
			break;
		// thru

	default:
		
		// それ以外の状態(『待機中』・『コミット準備済』以外)の
		// トランザクションブランチをコミットしようとした

		_SYDNEY_THROW0(Exception::XA_ProtocolError);
	}

	// トランザクションブランチの
	// トランザクション処理を行うためのトランザクションをコミットする

	getTransaction().commit();

	// トランザクションブランチと連係するセッションをなくす

	_associatedID = Server::IllegalSessionID;

	// トランザクションブランチの状態を『存在しない』にする

	_status = Status::NonExistent;
}

//	FUNCTION public
//	Trans::Branch::rollback -- トランザクションブランチをロールバックする
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
//		Exception::XA_HeurCommit
//			トランザクションブランチは既にコミットして
//			ヒューリスティックに解決されている
//		Exception::XA_HeurRollback
//			トランザクションブランチは既にロールバックして
//			ヒューリスティックに解決されている
//		Exception::XA_HeurMix
//			トランザクションブランチは既に一部コミットし一部ロールバックして
//			ヒューリスティックに解決されている
//		Exception::XA_ProtocolError
//			コミットできない状態のトランザクションブランチのロールバックを試みた
//		Exception::XA_UnknownIdentifier
//			対象のトランザクションブランチは『存在しない』

void
Branch::rollback()
{
	Os::AutoCriticalSection latch(_latch);

	switch (_status) {
	case Status::NonExistent:

		// トランザクションブランチは『存在しない』

		_SYDNEY_THROW0(Exception::XA_UnknownIdentifier);

	case Status::HeuristicallyCompleted:

		// どういうふうにヒューリスティックに
		// トランザクションブランチが解決されたかを表す例外を投げる

		switch (_heurDecision) {
		case HeurDecision::Commit:
			_SYDNEY_THROW0(Exception::XA_HeurCommit);
		case HeurDecision::Rollback:
			_SYDNEY_THROW0(Exception::XA_HeurRollback);
		case HeurDecision::Mix:
			_SYDNEY_THROW0(Exception::XA_HeurMix);
		default:
			; _SYDNEY_ASSERT(false);
		}

	case Status::Idle:
	case Status::RollbackOnly:
	case Status::Prepared:
		break;

	default:
		
		// それ以外の状態(『待機中』・
		//『コミット準備済』・『ロールバックのみ可』以外)の
		// トランザクションブランチをロールバックしようとした

		_SYDNEY_THROW0(Exception::XA_ProtocolError);
	}

	// トランザクションブランチの
	// トランザクション処理を行うためのトランザクションをロールバックする

	(void) getTransaction().rollback();

	// トランザクションブランチと連係するセッションをなくす

	_associatedID = Server::IllegalSessionID;

	// トランザクションブランチの状態を『存在しない』にする

	_status = Status::NonExistent;
}

//	FUNCTION private
//	Trans::Branch::decideHeuristically --
//		トランザクションブランチをヒューリスティックに解決する
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
//		Exception::XA_ProtocolError
//			ヒューリスティックに解決できない状態のトランザクションブランチである

void
Branch::decideHeuristically()
{
	Os::AutoCriticalSection latch(_latch);

	switch (_status) {
	case Status::Prepared:
		break;

	default:

		// それ以外の状態(『コミット準備済』以外)の
		// トランザクションブランチをコミットしようとした

		_SYDNEY_THROW0(Exception::XA_ProtocolError);
	}

	// どうやってヒューリスティックに解決するか決める
	//
	// * 論理ログを記録しないときはロールバックできないので、コミットする
	// * それ以外の場合はロールバックする

	const HeurDecision::Value decision =
		(getTransaction().isNoLog()) ?
		HeurDecision::Commit : HeurDecision::Rollback;

	// 論理ログを記録しているような操作が行われたか調べる

	const bool stored =
		getTransaction().isLogStored(Log::File::Category::System) ||
		getTransaction().isLogStored(Log::File::Category::Database);

	if (stored) {

		// システム用の論理ログファイルに
		// トランザクションブランチをヒューリスティックに解決することを記録する

		AutoLatch latch(getTransaction(),
						getTransaction().getLogInfo(
							Log::File::Category::System).getLockName());
		(void) getTransaction().storeLog(
			Log::File::Category::System,
			Log::BranchHeurDecideData(getID(), decision));
	}

	if (decision == HeurDecision::Commit)
		getTransaction().commit();
	else
		getTransaction().rollback();

	// トランザクションブランチと連係するセッションをなくす

	_associatedID = Server::IllegalSessionID;

	if (stored) {
		_status = Status::HeuristicallyCompleted;
		_heurDecision = decision;
	} else 

		// トランザクションブランチの状態を『存在しない』にする

		_status = Status::NonExistent;
}

//	FUNCTION public
//	Trans::Branch::forget --
//		ヒューリスティックに解決済なトランザクションブランチを抹消する
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
//		Exception::XA_UnknownIdentifier
//			ヒューリスティックに解決済でない
//			トランザクションブランチを抹消しようとした

void
Branch::forget()
{
	Os::AutoCriticalSection latch(_latch);

	if (_status != Status::HeuristicallyCompleted)

		// ヒューリスティックに解決済でない
		// トランザクションブランチを抹消しようとした

		_SYDNEY_THROW0(Exception::XA_UnknownIdentifier);

	// システム用の論理ログファイルに
	// ヒューリスティックに解決済な
	// トランザクションブランチを抹消したことを記録する
	{
	AutoLatch latch(getTransaction(),
					getTransaction().getLogInfo(
						Log::File::Category::System).getLockName());
	(void) getTransaction().storeLog(
		Log::File::Category::System, Log::BranchForgetData(getID()));
	}
	// トランザクションの状態を『存在しない』にする

	_status = Status::NonExistent;
	_heurDecision = HeurDecision::Unknown;
}

//	FUNCTION public
//	Trans::Branch::getID --
//		あるセッションと連係しているある状態のトランザクションブランチの
//		トランザクションブランチ識別子をすべて求める
//
//	NOTES
//
//	ARGUMENTS
//		Server::SessionID	sessionID
//			Server::IllegalSessionID 以外の値
//				このセッション識別子で識別されるセッションと連係している
//				トランザクションブランチの中から求める
//			Server::IllegalSessionID
//				どのセッションと連係しているかを考慮しない
//		Trans::Branch::Status::Value	status
//			Trans::Branch::Status::Unknown 以外の値
//				この状態のトランザクションブランチの中から求める
//			Trans::Branch::Status::Unknown
//				トランザクションブランチの状態は考慮しない
//		ModVector<Trans::Branch::ID>&	result
//			求めたトランザクションブランチ識別子を要素として追加する配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Branch::getID(Server::SessionID sessionID,
			  Status::Value status, ModVector<ID>& result)
{
	Os::AutoCriticalSection latch(_Branch::_latch);

	; _SYDNEY_ASSERT(_Branch::_branchTable);

	unsigned int i = 0;
	do {
		// 今調べているバケットを得る

		HashTable::Bucket& bucket = _Branch::_branchTable->getBucket(i);

		// このバケットに登録されている
		// トランザクションブランチをひとつひとつ調べていく

		HashTable::Bucket::Iterator			ite(bucket.begin());
		const HashTable::Bucket::Iterator&	end = bucket.end();

		for (; ite != end; ++ite) {
			const Branch& branch = *ite;

			Os::AutoCriticalSection	latch(branch._latch);

			if ((branch._status == Status::Unknown ||
				 branch._status == status) &&
				(sessionID == Server::IllegalSessionID ||
				 sessionID == branch.getAssociatedID()))

				// 見つかった

				result.pushBack(branch.getID());
		}
	} while (++i < _Branch::_branchTable->getLength()) ;
}

//	FUNCTION public
//	Trans::Branch::getHeurCompletionInfo --
//		ヒューリスティックに解決済のすべてのトランザクションブランチの情報を得る
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<Trans::Branch::HeurCompletionInfo>&	result
//			求めた情報を要素として追加する配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Branch::getHeurCompletionInfo(ModVector<HeurCompletionInfo>& result)
{
	Os::AutoCriticalSection latch(_Branch::_latch);

	; _SYDNEY_ASSERT(_Branch::_branchTable);

	unsigned int i = 0;
	do {
		// 今調べているバケットを得る

		HashTable::Bucket& bucket = _Branch::_branchTable->getBucket(i);

		// このバケットに登録されている
		// トランザクションブランチをひとつひとつ調べていく

		HashTable::Bucket::Iterator			ite(bucket.begin());
		const HashTable::Bucket::Iterator&	end = bucket.end();

		for (; ite != end; ++ite) {
			const Branch& branch = *ite;

			Os::AutoCriticalSection	latch(branch._latch);

			if (branch._status == Status::HeuristicallyCompleted)

				// 見つかった

				result.pushBack(
					HeurCompletionInfo(branch.getID(), branch._heurDecision));
		}
	} while (++i < _Branch::_branchTable->getLength()) ;
}

//	FUNCTION private
//	Trans::Branch::getExistenceOfActive --
//		あるセッションにより『データ操作中』の
//		トランザクションブランチがあるか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Server::SesionID	sessionID
//			このセッション識別子の表すセッションにより
//			『データ操作中』のトランザクションブランチがあるか調べる
//
//	RETURN
//		true
//			あった
//		false
//			なかった
//
//	EXCEPTIONS

// static
bool
Branch::getExistenceOfActive(Server::SessionID sessionID)
{
	if (sessionID != Server::IllegalSessionID) {
		Os::AutoCriticalSection latch(_Branch::_latch);

		; _SYDNEY_ASSERT(_Branch::_branchTable);

		unsigned int i = 0;
		do {
			// 今調べているバケットを得る

			HashTable::Bucket& bucket = _Branch::_branchTable->getBucket(i);

			// このバケットに登録されている
			// トランザクションブランチをひとつひとつ調べていく

			HashTable::Bucket::Iterator			ite(bucket.begin());
			const HashTable::Bucket::Iterator&	end = bucket.end();

			for (; ite != end; ++ite) {
				const Branch& branch = *ite;

				Os::AutoCriticalSection	latch(branch._latch);

				if (branch._status == Status::Active &&
					branch.getAssociatedID() == sessionID)

					// 見つかった

					return true;
			}
		} while (++i < _Branch::_branchTable->getLength()) ;
	}

	// 見つからなかった

	return false;
}

//	FUNCTION public
//	Trans::Branch::redo --
//		あるトランザクションブランチがヒューリスティックに
//		解決済であったことを思い出す
//
//	NOTES
//		起動時回復処理時に実行する
//
//	ARGUMENTS
//		Checkpoint::Log::CheckpointSystemData&	logData
//			システムに関するチェックポイント処理の終了を表す論理ログ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Branch::redo(const Checkpoint::Log::CheckpointSystemData& logData)
{
	const ModVector<HeurCompletionInfo>& list = logData.getHeurCompletionInfo();
	const unsigned int n = list.getSize();
	for (unsigned int i = 0; i < n; ++i)
		redo(Log::BranchHeurDecideData(list[i]._id, list[i]._decision));
}

//	FUNCTION public
//	Trans::Branch::redo --
//		あるトランザクションブランチがヒューリスティックに
//		解決済であったことを思い出す
//
//	NOTES
//		起動時回復処理時に実行する
//
//	ARGUMENTS
//		Trans::Log::BranchHeurDecideData&	logData
//			トランザクションブランチを
//			ヒューリスティックに解決したことを表す論理ログ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Branch::redo(const Log::BranchHeurDecideData& logData)
{
	AutoBranch branch(attach(logData.getXID()));

	// ヒューリステイックに解決されたこととする
	{
	Os::AutoCriticalSection latch(branch->_latch);
	; _SYDNEY_ASSERT(branch->_status == Status::NonExistent);
	branch->_status = Status::HeuristicallyCompleted;
	branch->_heurDecision = logData.getDecision();
	}
}

//	FUNCTION public
//	Trans::Branch::redo --
//		ヒューリスティックに解決済のトランザクションブランチを抹消する
//
//	NOTES
//		起動時回復処理時に実行する
//
//	ARGUMENTS
//		Trans::Log::BranchForgetData&	logData
//			ヒューリスティックに解決済なトランザクションブランチを
//			抹消したことを表す論理ログ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Branch::redo(const Log::BranchForgetData& logData)
{
	AutoBranch branch(attach(logData.getXID()));

	// トランザクションの状態を『存在しない』にする
	{
	Os::AutoCriticalSection latch(branch->_latch);
	; _SYDNEY_ASSERT(branch->_status == Status::HeuristicallyCompleted);
	branch->_status = Status::NonExistent;
	branch->_heurDecision = HeurDecision::Unknown;
	}
}

//
// Copyright (c) 2007, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
