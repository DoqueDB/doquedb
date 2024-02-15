// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Page.cpp -- バージョンページ関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Version";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"

#include "Version/AutoPage.h"
#include "Version/AutoVerification.h"
#include "Version/Configuration.h"
#include "Version/HashTable.h"
#include "Version/Manager.h"
#include "Version/MasterData.h"
#include "Version/Page.h"
#include "Version/VersionLog.h"

#include "Checkpoint/TimeStamp.h"
#include "Common/Assert.h"
#include "Exception/ReadOnlyTransaction.h"
#include "Exception/Unexpected.h"
#include "Os/AutoCriticalSection.h"
#include "Os/AutoRWLock.h"
#include "Trans/List.h"

_SYDNEY_USING
_SYDNEY_VERSION_USING

namespace
{

namespace _Page
{
	namespace _Memory
	{
		// バージョンページを記録するブロックの
		// バッファリング内容を実際に更新したことにする
		void
		dirty(Block::Memory& logMemory);
	}

	// 生成済のバージョンページ記述子を捜す
	Page*
	find(HashTable<Page>::Bucket& bucket, File& file, Page::ID id);

	// あるバージョンファイルの
	// すべてのバージョンページを管理するハッシュ表のハッシュ関数
	unsigned int			pageTableHash(const File& file, Page::ID id);

	// ページ更新トランザクションリストにトランザクション識別子を登録する
	void
	registerModifier(ModVector<Trans::Transaction::ID>& list,
					 const Trans::Transaction::ID& id);

	// すべてのバージョンページ記述子を管理するハッシュ表
	HashTable<Page>*		_pageTable = 0;

	// 開放したページ記述子を管理するリスト
	Page*					_freeList = 0;

	// フリーリストを操作するためのクリティカルセクション
	Os::CriticalSection		_freeLatch;

	// フリーリスト上の数
	int						_freeListCount = 0;

}

namespace _MasterData
{
	namespace _File
	{
		// マスタデータファイルが生成されていなければ、生成する
		void
		create(MasterData::File& file, Os::AutoTryRWLock& rwlock);
	}
}

namespace _VersionLog
{
	namespace _File
	{
		// バージョンログファイルが生成されていなければ、生成する
		void
		create(const Trans::Transaction& trans,
			   VersionLog::File& versionLog,
			   Os::AutoTryRWLock& rwlock, MasterData::File& masterData);
	}
}

//	FUNCTION
//	$$$::_Page::find -- 生成済のバージョンページ記述子を探す
//
//	NOTES
//
//	ARGUMENTS
//		Version::HashTable<Page>::Bucket&	bucket
//			バージョンページ記述子が格納されるべきハッシュ表のバケット
//		Version::File&		file
//			探しているバージョンページ記述子の表す
//			バージョンページを持つバージョンファイルのバージョンファイル記述子
//		Version::Page::ID	id
//			探しているバージョンページ記述子の表す
//			バージョンページのバージョンページ識別子
//
//	RETURN
//		0 以外の値
//			得られたバージョンページ記述子を格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPTIONS
//		なし

Page*
_Page::find(HashTable<Page>::Bucket& bucket, File& file, Page::ID id)
{
	//【注意】	呼び出し側で bucket.getLatch() をラッチする必要がある

	switch (bucket.getSize()) {
	default:
	{
		// バケットに登録されているバージョンページ記述子のうち、
		// 与えられたバージョンページ識別子のバージョンページを表すものを探す

		HashTable<Page>::Bucket::Iterator			begin(bucket.begin());
		HashTable<Page>::Bucket::Iterator			ite(begin);
		const HashTable<Page>::Bucket::Iterator&	end = bucket.end();

		do {
			Page& page = *ite;

			if (page.getID() == id && &page.getFile() == &file) {

				// 見つかったバージョンページ記述子を
				// バケットの先頭に移動して、
				// 最近に参照されたものほど、見つかりやすくする

				bucket.splice(begin, bucket, ite);

				return &page;
			}
		} while (++ite != end) ;

		break;
	}
	case 1:
	{
		Page& page = bucket.getFront();

		if (page.getID() == id && &page.getFile() == &file)

			// 見つかった

			return &page;

		break;
	}
	case 0:
		break;
	}

	// 見つからなかった

	return 0;
}

//	FUNCTION
//	$$$::_Page::pageTableHash --
//		あるバージョンファイルのすべてのバージョンページ記述子を
//		管理するハッシュ表に登録するためにハッシュ値を計算する
//
//	NOTES
//
//	ARGUMENTS
//		Version::File&		file
//			ハッシュ表に登録するバージョンページが所属する
//			バージョンファイルのバージョンファイル記述子
//		Version::Page::ID	id
//			ハッシュ表に登録するバージョンページのバージョンページ識別子
//
//	RETURN
//		得られたハッシュ値
//
//	EXCEPTIONS
//		なし

inline
unsigned int
_Page::pageTableHash(const File& file, Page::ID id)
{
	// 指定されたバージョンページ識別子に
	// バージョンページが存在する
	// バージョンファイルの識別子を 4 左シフトしたものを加える

	return id + (static_cast<unsigned int>(file.getID()) << 4);
}

//	FUNCTION
//	$$$::_Page::registerModifier --
//		ページ更新トランザクションリストにトランザクション識別子を登録する
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<Trans::Transaction::ID>&	list
//			あるバージョンページを更新したトランザクションの
//			トランザクション識別子を要素とするリストで、
//			リスト中のトランザクション識別子は、それの表す
//			トランザクションの開始時刻の昇順に並んでいる必要がある
//		Trans::Transaction::ID&	id
//			登録するトランザクション識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
_Page::registerModifier(ModVector<Trans::Transaction::ID>& list,
						const Trans::Transaction::ID& id)
{
	ModVector<Trans::Transaction::ID>::Iterator	ite(list.end());

	if (list.getSize()) {

		//【注意】	最近開始したトランザクションほど、
		//			早く挿入位置を見つけられるようにする

		const ModVector<Trans::Transaction::ID>::Iterator&
			begin = list.begin();

		while (ite != begin) {
			--ite;
			if (*ite == id)
				return;
			if (*ite < id) {
				++ite;
				break;
			}
		}
	}

	list.insert(ite, id);
}

//	FUNCTION
//	$$$::_Page::_Memory::dirty --
//		バージョンページを記録するブロックの
//		バッファリング内容を実際に更新したことにする
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::Memory&	logMemory
//			バージョンページを記録するブロックのバッファリング内容
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
_Page::_Memory::dirty(Block::Memory& logMemory)
{
	; _SYDNEY_ASSERT(logMemory.isOwner());

	// バージョンページを記録するブロックは、
	// マスタデータファイルのデータブロックか、
	// バージョンログファイルのバージョンログである
	//
	//【注意】	マスタデータファイルのデータブロックと
	//			バージョンログファイルのバージョンログは
	//			まったく同じ形式である

	VersionLog::Log::get(logMemory).dirty();
	logMemory.dirty();
}

//	FUNCTION
//	$$$::_MasterData::_File::create --
//		マスタデータファイルが生成されていなければ、生成する
//
//	NOTES
//
//	ARGUMENTS
//		Version::MasterData::File&	file
//			生成するマスタデータファイルのマスタデータファイル記述子
//		Os::AutoTryRWLock&	rwlock
//			マスタデータファイルの生成・破棄の排他制御用の
//			読み取り書き込みロックで、
//			呼び出し時にはすでに読み取りロックされている
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
_MasterData::_File::create(MasterData::File& file, Os::AutoTryRWLock& rwlock)
{
	while (!file.isMountedAndAccessible()) {

		// マスタデータファイルがマウントされていない

		// マスタデータファイル、バージョンログファイル、
		// 同期ログファイルの生成・破棄の排他制御用の
		// 読み取り書き込みロックに対する
		// 読み取りロックをはずし、書き込みロックする
		//
		//【注意】	Os::RWLock に対する同じスレッドの
		//			読み取りロックと書き込みロックはデッドロックを起こすので、
		//			このようなコードにする

		rwlock.unlock();
		rwlock.lock(Os::RWLock::Mode::Write);

		// マスタデータファイルを生成する

		file.create();

		// マスタデータファイル、バージョンログファイル、
		// 同期ログファイルの生成・破棄の排他制御用の
		// 読み取り書き込みロックを元に戻す

		rwlock.unlock();
		rwlock.lock(Os::RWLock::Mode::Read);
	}
}

//	FUNCTION
//	$$$::_VersionLog::_File::create --
//		バージョンログファイルが生成されていなければ、生成する
//
//	NOTES
//
//	ARGUMENTS
//		Version::VersionLog::File&	versionLog
//			生成するバージョンログファイルのバージョンログファイル記述子
//		Os::AutoTryRWLock&	rwlock
//			バージョンログファイルの生成・破棄の排他制御用の
//			読み取り書き込みロックで、
//			呼び出し時にはすでに読み取りロックされている
//		Version::MasterData::File&	masterData
//			生成するバージョンログファイルが所属するバージョンファイルの
//			マスタデータファイルのマスタデータファイル記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
_VersionLog::_File::create(
	const Trans::Transaction& trans,
	VersionLog::File& versionLog,
	Os::AutoTryRWLock& rwlock, MasterData::File& masterData)
{
	while (!versionLog.isMountedAndAccessible()) {

		// バージョンログファイルがマウントされていない

		// マスタデータファイル、バージョンログファイル、
		// 同期ログファイルの生成・破棄の排他制御用の
		// 読み取り書き込みロックに対する
		// 読み取りロックをはずし、書き込みロックする

		rwlock.unlock();
		rwlock.lock(Os::RWLock::Mode::Write);

		// マスタデータファイルが生成されていなければ、生成する

		masterData.create();

		// マスタデータファイルの総ブロック数を求め、
		// それを現在のバージョンページの総数として
		// バージョンログファイルを生成する

		versionLog.create(trans, masterData.getBlockCount(trans));

		// マスタデータファイル、バージョンログファイル、
		// 同期ログファイルの生成・破棄の排他制御用の
		// 読み取り書き込みロックを元に戻す

		rwlock.unlock();
		rwlock.lock(Os::RWLock::Mode::Read);
	}
}

}

//	FUNCTION private
//	Manager::Page::initialize --
//		マネージャーの初期化のうち、バージョンページ記述子関連のものを行う
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
Manager::Page::initialize()
{
	// すべてのバージョンページ記述子を管理する
	// ハッシュ表が確保されていないので、まず、確保する

	_Page::_pageTable =
		new HashTable<Version::Page>(Configuration::PageTableSize::get(),
									 &Version::Page::_hashPrev,
									 &Version::Page::_hashNext);
	; _SYDNEY_ASSERT(_Page::_pageTable);
}

//	FUNCTION private
//	Manager::Page::terminate --
//		マネージャーの後処理のうち、バージョンページ関連のものを行う
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
Manager::Page::terminate()
{
	//【注意】	他のスレッドが同時に実行されることはないので、
	//			ラッチしない

	if (_Page::_pageTable) {

		// この時点で、バージョンページ記述子は存在しないはず

		; _SYDNEY_ASSERT(_Page::_pageTable->isEmpty());

		// すべてのバージョンページ記述子を管理するハッシュ表を破棄する

		delete _Page::_pageTable, _Page::_pageTable = 0;
	}
}

//	FUNCTION private
//	Version::Page::attach -- バージョンページ記述子を得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::File&		file
//			得ようとしているバージョンページ記述子の表す
//			バージョンページを持つバージョンファイルのバージョンファイル記述子
//		Version::Page::ID	id
//			得ようとしているバージョンページ記述子の表す
//			バージョンページのバージョンページ識別子
//		bool create
//			バージョンページ識別子が存在していなかった場合、
//			作成するかどうかを指定するフラグ(default true)
//
//	RETURN
//		得られたバージョンページ記述子を格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Page*
Page::attach(File& file, ID id, bool create)
{
	// 指定されたバージョンページ識別子のバージョンページ記述子を
	// 格納すべきハッシュ表のバケットを求める

	HashTable<Page>& table = getHashTable();
	const unsigned int addr =
		_Page::pageTableHash(file, id) % table.getLength();
	HashTable<Page>::Bucket& bucket = table.getBucket(addr);

	// バケットを保護するためにラッチをかける

	Os::AutoCriticalSection	latch(bucket.getLatch());

	// 指定されたバージョンページ識別子のバージョンページの
	// バージョンページ記述子が求めたバケットに登録されていれば、それを得る

	Page* page = _Page::find(bucket, file, id);
	if (page)

		// 見つかったので、参照回数を 1 増やす

		++page->_refCount;
	else if (create) {

		// フリーリストを得る
		page = popFreeList();

		if (page == 0)
		{
			// 見つからなかったので、生成する

			page = new Page(file, id);
			
		}
		else
		{
			// メンバーをリセットする
			
			page->initialize(file, id);

		}
		
		; _SYDNEY_ASSERT(page);

		// 生成したバージョンページ記述子が存在する間に
		// 与えられたバージョンファイル記述子が破棄されると困るので、
		// 参照回数を 1 増やして、破棄されないようにする

		(void) file.attach();

		// 参照回数を 1 にする

		page->_refCount = 1;

		// ハッシュ表のバケットの先頭に挿入して、
		// 最近に参照されたものほど、見つかりやすくする

		bucket.pushFront(*page);
	}

	return page;
}

#ifdef OBSOLETE
//	FUNCTION private
//	Version::Page::attach -- バージョンページ記述子の参照数を 1 増やす
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		参照数が 1 増えたバージョンページ記述子を格納する領域の先頭アドレス
//
//	EXCEPTIONS

Page*
Page::attach()
{
	// 参照数を 1 増やすバージョンページ記述子を
	// 格納するハッシュ表のバケットを求める

	HashTable<Page>& table = getHashTable();
	const unsigned int addr =
		_Page::pageTableHash(getFile(), getID()) % table.getLength();
	const HashTable<Page>::Bucket& bucket = table.getBucket(addr);
		
	// バケットを保護するためにラッチをかける

	Os::AutoCriticalSection	latch(bucket.getLatch());

	// 参照数を 1 増やす

	++_refCount;

	return this;
}
#endif

//	FUNCTION private
//	Version::Page::detach -- バージョンページ記述子の参照をやめる
//
//	NOTES
//
//	ARGUMENTS
//		Version::Page*&		page
//			参照をやめるバージョンページ記述子を格納する領域の先頭アドレス
//			呼び出しから返ると 0 になる
//		bool				reserve
//			true
//				どこからも参照されなくなったバージョンページ記述子でも、
//				また参照されるときのために破棄せずにとっておく
//			false
//				どこからも参照されなくなったバージョンページ記述子は破棄する
//
//	RETURN
//		なし	
//
//	EXCEPTIONS


// static
void
Page::detach(Page*& page, bool reserve)
{
	if (page) {
		File* file = page->_file;
		{
		// 与えられたバージョンページ記述子を格納する
		// ハッシュ表のバケットを求める

		HashTable<Page>& table = getHashTable();
		const unsigned int addr =
			_Page::pageTableHash(*file, page->getID()) % table.getLength();
		HashTable<Page>::Bucket& bucket = table.getBucket(addr);

		// バケットを保護するためにラッチする

		Os::AutoCriticalSection	latch(bucket.getLatch());

		; _SYDNEY_ASSERT(page->getRefCount());

		// 参照数が 0 より大きければ 1 減らす

		if ((page->getRefCount() && --page->_refCount) ||
			reserve || page->_modifierList.getSize())

			// 他から参照されているか、
			// また参照されるときのためにとっておくか、
			// ページ更新トランザクションリストが空でないので、破棄できない

			page = 0;
		else
			// バケットから取り除く

			bucket.erase(*page);

		// ここで、バケットを保護するためのラッチがはずれる
		}
		if (page) {

			// どこからも参照されていないバージョンページ記述子を
			// フリーリストにつなぎ、与えられたポインタは 0 を指すようにする

			pushFreeList(page);

			page = 0;

			// 破棄したバージョンページ記述子が参照していた
			// バージョンファイル記述子の参照数を 1 減らし、
			// どこからも参照されなくなったら破棄する

			File::detach(file, false);
		}
	}
}

#ifdef OBSOLETE
//	FUNCTION private
//	Version::Page::detach -- バージョンページ記述子の参照数を 1 減らす
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
Page::detach()
{
	// 参照数を 1 減らすバージョンページ記述子を
	// 格納するハッシュ表のバケットを求める

	HashTable<Page>& table = getHashTable();
	const unsigned int addr =
		_Page::pageTableHash(getFile(), getID()) % table.getLength();
	const HashTable<Page>::Bucket& bucket = table.getBucket(addr);

	// バケットを保護するためにラッチする

	Os::AutoCriticalSection	latch(bucket.getLatch());
#ifdef DEBUG
	; _SYDNEY_ASSERT(getRefCount());
#else
	if (getRefCount())
#endif
		// 参照数が 0 より大きければ 1 減らす

		--_refCount;
}
#endif

//	FUNCTION private
//	Version::Page::fix -- バージョンページをフィックスする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンページをフィックスする
//			トランザクションのトランザクション記述子
//		Version::Verification*	verification
//			0 以外の値
//				バージョンページをフィックスするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				バージョンページがフィックスするトランザクションは
//				整合性検査中でない
//		Version::File&		file
//			フィックスするバージョンページが存在する
//			バージョンファイルのバージョンファイル記述子
//		Version::Page::ID	id
//			フィックスするバージョンページのバージョンページ識別子
//		Buffer::Page::FixMode::Value	mode
//			以下の値の論理和を指定する
//			Buffer::Page::FixMode::ReadOnly
//				フィックスするバージョンページは参照しかされない
//			Buffer::Page::FixMode::Write
//				フィックスするバージョンページは更新される
//			Buffer::Page::FixMode::Allocate
//				フィックスするバージョンページは
//				その領域の初期化のために使用する
//			Buffer::Page::FixMode::Discardable
//				フィックスしたバージョンページを更新しても、
//				アンフィックス時にダーティにしなければ、
//				バッファへ反映されない
//		Buffer::ReplacementPriority::Value	priority
//			Buffer::ReplacementPriority::Low
//				フィックスするバージョンページは、バッファから破棄されやすい
//			Buffer::ReplacementPriority::Middle
//				フィックスするバージョンページは、バッファにある程度残る
//			Buffer::ReplacementPriority::High
//				フィックスするバージョンページは、バッファからかなりの間残る
//
//	RETURN
//		フィックスしたバージョンページのバッファリング内容
//
//	EXCEPTIONS

// static
Page::Memory
Page::fix(const Trans::Transaction& trans, Verification* verification,
		  File& file, ID id, Buffer::Page::FixMode::Value mode,
		  Buffer::ReplacementPriority::Value priority)
{
	const bool discardable = mode & Buffer::Page::FixMode::Discardable;

	// 指定されたモードのうち、下位関数で処理するモードを得る

	mode &=	(Buffer::Page::FixMode::ReadOnly |
			 Buffer::Page::FixMode::Write | Buffer::Page::FixMode::Allocate);

	// 指定されたバージョンページ識別子の
	// バージョンページのバージョンページ記述子を得る

	AutoPage page(Page::attach(file, id), false);

	// バージョンページの操作すべき版を格納するブロックをフィックスする

	Memory	memory(
		*page, trans, page->fixBlock(trans, verification, mode, priority));
	page.release();

	if (discardable) {

		// フィックスされたバージョンページを更新しても、
		// そのアンフィックス時にバージョンページが
		// ダーティになったことを宣言しなければ、
		// 更新内容が破棄されるようにするために、
		// たった今フィックスしたバージョンページをフィックスしなおす

		memory.discardable();

		if (mode == Buffer::Page::FixMode::Allocate)

			// フィックスされたバージョンページを初期化する
			//
			//【注意】	バージョンファイルはすでに存在する
			//			バージョンページを初期化できる
			//
			//			Buffer::Page::FixMode::Discardable が
			//			指定されているとき、すでに存在する
			//			バージョンページの初期化を元に戻せるようにする

			memory.reset();
	} else
		if (mode == Buffer::Page::FixMode::Allocate) {
		
			// フィックスされたバージョンページを初期化する

			memory.reset();
			memory.dirty();
		}

	return memory;
}

//	FUNCTION private
//	Version::Page::fixBlock --
//		バージョンページの操作すべき版を格納するブロックをフィックスする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			ブロックをフィックスするトランザクションのトランザクション記述子
//		Version::Verification*	verification
//			0 以外の値
//				ブロックをフィックスするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				ブロックをフィックスするトランザクションは整合性検査中でない
//		Buffer::Page::FixMode::Value	mode
//			以下の値を指定する
//			Buffer::Page::FixMode::ReadOnly
//				フィックスするブロックは参照しかされない
//			Buffer::Page::FixMode::Write
//				フィックスするブロックは更新される
//			Buffer::Page::FixMode::Allocate
//				フィックスするブロックはその領域の初期化のために使用する
//		Buffer::ReplacementPriority::Value	priority
//			Buffer::ReplacementPriority::Low
//				フィックスするブロックは、バッファから破棄されやすい
//			Buffer::ReplacementPriority::Middle
//				フィックスするブロックは、バッファにある程度残る
//			Buffer::ReplacementPriority::High
//				フィックスするブロックは、バッファからかなりの間残る
//
//	RETURN
//		フィックスしたブロックのバッファリング内容
//
//	EXCEPTIONS
//		Exception::ReadOnlyTransaction
//			読取専用トランザクションは
//			参照以外の目的でブロックをフィックスできない

Block::Memory
Page::fixBlock(const Trans::Transaction& trans, Verification* verification,
			   Buffer::Page::FixMode::Value mode,
			   Buffer::ReplacementPriority::Value priority)
{
	; _SYDNEY_ASSERT(_file->_file._masterData);

	if (trans.getCategory() == Trans::Transaction::Category::ReadOnly &&
		mode != Buffer::Page::FixMode::ReadOnly)

		// 読取専用トランザクションは
		// 参照以外の目的でバージョンページをフィックスできない
		
		_SYDNEY_THROW0(Exception::ReadOnlyTransaction);

	// マスタデータファイル、バージョンログファイル、
	// 同期ログファイルの生成・破棄の排他制御用の
	// 読み取り書き込みロックを読み取りロックする

	Os::AutoTryRWLock rwlock(_file->getRWLock());
	rwlock.lock(Os::RWLock::Mode::Read);

	if (!_file->_file._versionLog)

		// バージョンページごとにひとつの版しか
		// 格納できないバージョンファイルである

		if (mode == Buffer::Page::FixMode::Allocate) {

			// マスタデータファイルが生成されていなければ、生成する

			_MasterData::_File::create(*_file->_file._masterData, rwlock);

			// 指定されたバージョンページ識別子のバージョンページが
			// 記録されているマスタデータファイルのブロックを確保する

			return MasterData::Data::allocate(
				trans, *_file->_file._masterData,
				static_cast<Block::ID>(getID()), priority);
		} else

			// 指定されたバージョンページ識別子のバージョンページが
			// 記録されているマスタデータファイルのブロックをフィックスする
			//
			//【注意】	マスタデータファイルが存在しなければ、例外が発生する

			return MasterData::Data::fix(
				trans, *_file->_file._masterData,
					static_cast<Block::ID>(getID()), mode, priority);

	// バージョンページごとに複数の版を生成可能なバージョンファイルである

	if (mode == Buffer::Page::FixMode::ReadOnly) {

		// フィックスされる版に対して参照しか行わない

		if (_file->_file._versionLog->isMountedAndAccessible()) {

			// バージョンログファイルはマウントされている

			// まず、バージョンログファイルのファイルヘッダをフィックスする

			const Block::Memory& headerMemory =
				VersionLog::FileHeader::fix(
					trans, verification, *_file->_file._versionLog,
					Buffer::Page::FixMode::ReadOnly);
			const VersionLog::FileHeader& header =
				VersionLog::FileHeader::get(headerMemory);
			
			// トランザクションが参照すべき版は、
			// 更新中でない一貫性のある最新の版である

			// 指定されたバージョンページ識別子の
			// バージョンページの最新版が記録されている
			// バージョンログのブロック識別子を記録する PBCT リーフを得る

			const Block::Memory& leafMemory =
				_file->_file._versionLog->traversePBCT(
					trans, verification, headerMemory, getID(),
					Buffer::Page::FixMode::ReadOnly);

			if (leafMemory.getID() != Block::IllegalID) {

				// 得られた PBCT リーフに記録されている
				// 最新版のバージョンログのブロック識別子を求める

				const VersionLog::PBCTLeaf& leaf =
					VersionLog::PBCTLeaf::get(leafMemory);
				const Block::ID blockID = leaf.getLatestID(getID(), getSize());

				if (blockID != Block::IllegalID) {

					// 最新版のバージョンログから古いものへたどりながら、
					// 参照すべき版のバージョンログを得る
					/*
					  【注意】優先度が過剰に使用される
					*/
					
					Trans::TimeStamp::Value oldest = Trans::IllegalTimeStamp;

					if (header.getVersion()	== VersionLog::VersionNumber::First)
					{
						// Version 1 は leaf に最古の版のタイムスタンプがある
						// ので、それを利用する
						
						oldest = leaf.getOldestTimeStamp(getID(), getSize());
						; _SYDNEY_ASSERT(!Trans::TimeStamp::isIllegal(oldest));
					}

					const Block::Memory& logMemory =
						_file->_file._versionLog->traverseLog(
							trans, verification, *this,
								blockID, oldest, priority);

					if (logMemory.getID() != Block::IllegalID)

						// 参照すべき版のバージョンログが得られた

						return logMemory;
				}
			}
		}

		// 参照すべき版はマスタデータファイルに記録されている
		//
		//【注意】	マスタデータファイルが存在しなければ、例外が発生する

		return MasterData::Data::fix(
			trans, *_file->_file._masterData,
				static_cast<Block::ID>(getID()), mode, priority);
	}

	// フィックスされる版に対して更新も行う

	if (mode == Buffer::Page::FixMode::Allocate ||
		_file->_file._masterData->isMountedAndAccessible())

		// バージョンログファイルが生成されていなければ、生成する
		//
		//【注意】	マスタデータファイルも生成されていなければ、生成される

		_VersionLog::_File::create(
			trans,
			*_file->_file._versionLog, rwlock, *_file->_file._masterData);

	// バージョンログファイルのファイルヘッダをフィックスする

	VersionLog::MultiplexBlock headerMulti;
	VersionLog::FileHeader::fix(
		trans, verification, *_file->_file._versionLog,
			Buffer::Page::FixMode::Write, headerMulti);

	Block::Memory& headerMemory = headerMulti._memories[headerMulti._master];
	VersionLog::FileHeader&	header = VersionLog::FileHeader::get(headerMemory);

	if (mode == Buffer::Page::FixMode::Allocate) {

		// トランザクションが初期化すべき版は最新版である
		// ただし、必要に応じて新しい最新版を生成する必要がある

		if (getID() > header.getPageCount()) {

			// 最後に確保したバージョンページの
			// 直後のバージョンページを確保しようとしていない

			/* 【注意】	バージョンログファイルが
						あるときとないときで仕様があっていない ... */

			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::Unexpected);

		} else if (getID() == header.getPageCount()) {

			// 最大ページ数を求めて、
			// それと同じIDを確保しようとしている場合には
			// マスターページをそのまま返す
		
			unsigned int i = 0;
			unsigned int maxPageCount = 0;
			do {
				VersionLog::FileHeader* h = 0;
				if ((h = (headerMulti._memories[i].isOwner() ?
						  &VersionLog::FileHeader::get(
							  headerMulti._memories[i]) : 0)) &&
					maxPageCount < h->getPageCount())
					maxPageCount = h->getPageCount();
			} while (++i < VersionLog::MultiplexCount);

			if (getID() >= maxPageCount) {
				
				// このバージョンページを初めて初期化するときは、
				// マスタデータファイル上に最初の版を
				// 記録するためのデータブロックを確保する

				const Block::Memory& dataMemory =
					MasterData::Data::allocate(
						trans, *_file->_file._masterData,
						static_cast<Block::ID>(getID()), priority);

				// 新しいページが確保できたので、
				// リターンする前にバージョンログファイルのファイルヘッダの
				// 確保されているバージョンページの数を 1 増やす

				header.setPageCount(getID() + 1);
				headerMemory.dirty();

				return dataMemory;
			}
			
			// 実際にページを確保しなくても、
			// バージョンログファイルのファイルヘッダの
			// 確保されているバージョンページの数を 1 増やす

			header.setPageCount(getID() + 1);
			headerMemory.dirty();
		}
	}

	// 指定されたバージョンページ識別子の
	// バージョンページの最新版が記録されている
	// バージョンログのブロック識別子を記録する PBCT リーフを得る
	//
	//【注意】	このとき、必要があれば、
	//			複数の PBCT ノードやリーフが確保される

	Block::Memory leafMemory(
		_file->_file._versionLog->allocatePBCT(
			trans, verification, headerMulti, getID()));
				
	// 得られた PBCT リーフに記録されている
	// 最新版のバージョンログのブロック識別子を求め、フィックスする
	//
	//【注意】	得られたブロック識別子が 0 であれば、
	//			最新版はマスタデータファイルの
	//			指定されたバージョン識別子の表すブロックに、
	//			そうでなければ、バージョンログファイルの
	//			得られたブロック識別子の表すブロックに、
	//			それぞれ記録されている
	/*
	  【注意】	新たに最新版が確保されたときに、
				元の最新版も指定された優先度でフィックスされてしまう

	  【注意】	マスタデータファイルのほうは優先度に対応していない
	 */
	VersionLog::PBCTLeaf& leaf = VersionLog::PBCTLeaf::get(leafMemory);
	const Block::ID blockID = leaf.getLatestID(getID(), getSize());

	Block::Memory src(
		(blockID == Block::IllegalID) ?
		MasterData::Data::fix(
			trans, *_file->_file._masterData, static_cast<Block::ID>(getID()),
				Buffer::Page::FixMode::Write, priority) :
		VersionLog::Log::fix(
			trans, verification, *_file->_file._versionLog,
				blockID, Buffer::Page::FixMode::Write, priority));

	// バージョンログファイルに新しい最新版を確保する
	//
	//【注意】	このとき、マスタデータファイル上の版に対する
	//			最新版が確保されずにそのまま使用するとき、
	//			無駄に PBCT ノードやリーフを確保することになってしまう
	//【注意】	Version 2 以降では、getOldestTimeStamp()で、NewestTimeStamp が
	//			得られるが、allocateLog 内では参照されないので、そのまま

	const Block::Memory& dst =
		_file->_file._versionLog->allocateLog(
			trans, verification, headerMulti, *this, src,
				leaf.getOldestTimeStamp(getID(), getSize()), priority);

	if (src != dst) {

		// 確保した新しい最新版を記録するバージョンログの
		// ブロック識別子を PBCT リーフに記録する

		leaf.setLatestID(getID(), dst.getID(), getSize());

		if (VersionLog::FileHeader::get(
				headerMulti._memories[headerMulti._master]).getVersion()
			>= VersionLog::VersionNumber::Second)

			// バージョン2以降は最新版のタイムスタンプを PBCT リーフに記録する
			//
			//【注意】
			// ここでのタイムスタンプは、本当の最新版の最終更新タイムスタンプ
			// ではないが、PBCT リーフのタイムスタンプは最新版をマスターに同期
			// する目安に過ぎないので、速度などを考慮してこのままとする
			
			leaf.setNewestTimeStamp(getID(),
									dst.getLastModification(),
									getSize());
			
		else if (blockID == Block::IllegalID)

			// 最新版はマスタデータファイルに存在しなくなったので、
			// 最古の版のタイムスタンプを設定する
			//
			//【注意】	最古の版の最終更新時タイムスタンプは
			//			まだ求められていないので、
			//			代わりに直前のチェックポイント処理の
			//			終了時のタイムスタンプを設定する
			//【注意】	最古の版の最終更新時タイムスタンプを格納するのは、
			//			バージョン1まで

			leaf.setOldestTimeStamp(
				getID(),
				Checkpoint::TimeStamp::getMostRecent(getFile().getLockName()),
				getSize());

		leafMemory.dirty();
	}

	return dst;
}

//	FUNCTION public
//	Version::Page::verify -- 整合性を検査する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンページの整合性を検査する
//			トランザクションのトランザクション記述子
//		Version::File&		file
//			整合性を検査するバージョンページが存在する
//			バージョンファイルのバージョンファイル記述子
//		Version::Page::ID	id
//			整合性を検査するバージョンページのバージョンページ識別子
//		Admin::Verification::Progress&	result
//			整合性検査の経過を設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Page::verify(const Trans::Transaction& trans,
			 File& file, ID id, Admin::Verification::Progress& result)
{
	if (file._file._versionLog) {

		// バージョンログファイル、同期ログファイルの生成・破棄の
		// 排他制御用の読み取り書き込みロックを読み取りロックする

		Os::AutoRWLock rwlock(file.getRWLock(), Os::RWLock::Mode::Read);

		if (file._file._versionLog->isMountedAndAccessible()) {

			// 整合性検査するバージョンページが存在する
			// バージョンファイルのバージョンログファイルがマウントされている

			// 指定されたトランザクション記述子の表すトランザクションの
			// 指定されたバージョンファイルの整合性検査に関する
			// 情報を表すクラスを生成する

			AutoVerification
				verification(Verification::attach(trans, file), true);

			// 指定されたバージョンページ識別子の
			// バージョンページを整合性検査する

			file._file._versionLog->verify(trans, *verification, id, result);
		}
	}
}

//	FUNCTION public
//	Version::Page::verify -- 整合性を検査し、矛盾がなければ、フィックスする
//
//	NOTES
//		この関数の呼び出し中、またはフィックスされたバージョンページを
//		アンフィックスされるまでに、チェックポイント処理が行われたとき、
//		バージョンページを更新すると、バージョンファイルを構成する
//		バージョンログファイルの整合性が失われる可能性がある
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンページの整合性を検査する
//			トランザクションのトランザクション記述子
//		Version::File&		file
//			整合性を検査するバージョンページが存在する
//			バージョンファイルのバージョンファイル記述子
//		Version::Page::ID	id
//			整合性を検査するバージョンページのバージョンページ識別子
//		Buffer::Page::FixMode::Value	mode
//			以下の値の論理和を指定する
//			Buffer::Page::FixMode::ReadOnly
//				フィックスするバージョンページは参照しかされない
//			Buffer::Page::FixMode::Write
//				フィックスするバージョンページは更新される
//			Buffer::Page::FixMode::Allocate
//				フィックスするバージョンページは
//				その領域の初期化のために使用する
//			Buffer::Page::FixMode::Discardable
//				フィックスしたバージョンページを更新しても、
//				アンフィックス時にダーティにしなければ、
//				バッファへ反映されない
//		Admin::Version::Progress&	result
//			整合性検査の経過を設定する
//
//	RETURN
//		フィックスしたバージョンページのバッファリング内容
//
//	EXCEPTIONS

// static
Page::Memory
Page::verify(const Trans::Transaction& trans, File& file, ID id,
			 Buffer::Page::FixMode::Value mode,
			 Admin::Verification::Progress& result)
{
	if (file._file._versionLog) {

		// バージョンログファイル、同期ログファイルの生成・破棄の
		// 排他制御用の読み取り書き込みロックを読み取りロックする

		Os::AutoRWLock rwlock(file.getRWLock(), Os::RWLock::Mode::Read);

		if (file._file._versionLog->isMountedAndAccessible()) {

			// 整合性検査するバージョンページが存在する
			// バージョンファイルのバージョンログファイルがマウントされている

			// 指定されたトランザクション記述子の表すトランザクションの
			// 指定されたバージョンファイルの整合性検査に関する
			// 情報を表すクラスを生成する

			AutoVerification
				verification(Verification::attach(trans, file), true);

			// 指定されたバージョンページ識別子の
			// バージョンページを整合性検査する

			Admin::Verification::Progress progress(result.getConnection());
			file._file._versionLog->verify(trans, *verification, id, progress);
			result += progress;

			if (!progress.isGood()) {

				// 検査したバージョンページには不良があるので、
				// 所有権を放棄されたバッファリング内容を返す

				AutoPage page(Page::attach(file, id), false);
				return Memory(*page, trans, Block::Memory());
			}

			// 検査したバージョンページをフィックスする

			return fix(trans, verification.get(),
					   file, id, mode, Buffer::ReplacementPriority::Low);
		}
	}

	// バージョンページをフィックスする

	return fix(trans, 0, file, id, mode, Buffer::ReplacementPriority::Low);
}

//	FUNCTION private
//	Version::Page::clearModifierList --
//		可能であれば、ページ更新トランザクションリストを空にする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			空にできた
//		false
//			空にできなかった
//
//	EXCEPTIONS

bool
Page::clearModifierList()
{
	// ページ更新トランザクションリストを保護するためにラッチする

	Os::AutoCriticalSection	latch0(getLatch());

	// 現在実行中の版管理するトランザクションのうち、
	// このバージョンページの最新版を更新した更新トランザクションが
	// すべて終了するまでに、開始されたものがあるか調べる

	const Trans::List<Trans::Transaction>& list =
		Trans::Transaction::getInProgressList(
			getFile().getLockName().getDatabasePart(), false);

	Os::AutoCriticalSection	latch1(list.getLatch());

	if (list.getSize()) {

		// 現在実行中の版管理するトランザクションごとに調べる

		Trans::List<Trans::Transaction>::ConstIterator
			ite(list.begin());
		const Trans::List<Trans::Transaction>::ConstIterator&
			end = list.end();

		do {
			if ((*ite).isOverlapped(getModifierList()))

				// この版管理するトランザクションは、
				// 最新版を更新した更新トランザクションが
				// すべて終了するまでに開始された

				return false;

		} while (++ite != end) ;
	}

	// 開始されたものはひとつもないので、
	// このバージョンページの最新版を更新した更新トランザクションのうち、
	// 実行中のものがあるか調べる

	if (Trans::Transaction::isInProgress(
			getFile().getLockName().getDatabasePart(),
			getModifierList(), Trans::Transaction::Category::ReadWrite))

		// 実行中のものがあれば、
		// ページ更新トランザクションリストを空にできない
		//
		//【注意】	ページ更新トランザクションリストから
		//			実行中でないトランザクション識別子を
		//			格納する要素を削除できるが、
		//			ModVector の削除操作は遅いので、行わない

		return false;

	// 実行中のものはひとつもないので、
	// 版管理するトランザクションが参照すべき版の選択のために、
	// ページ更新トランザクションリストを使用する必要はない
	//
	// そこで、ページ更新トランザクションリストを空にする

	_modifierList.clear();

	return true;
}

//	FUNCTION public
//	Version::Page::getSize -- サイズを得る
//
//	NOTES
//		バージョンページのサイズとは、
//		バージョンページが占有する OS ファイル領域のサイズであり、
//		利用者が実際に使用可能なサイズとは同じであるとは限らない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

Os::Memory::Size
Page::getSize() const
{
	return getFile().getPageSize();
}

//	FUNCTION public
//	Version::Page::getContentSize --
//		あるサイズのブロックに実際に格納可能なデータのサイズを求める
//
//	NOTES
//
//	ARGUMENTS
//		Os::Memory::Size	size
//			格納可能なデータのサイズを計算するブロックのサイズ(B 単位)
//
//	RETURN
//		得られたサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

// static
Os::Memory::Size
Page::getContentSize(Os::Memory::Size size)
{
	const Os::Memory::Size overhead = sizeof(VersionLog::Log);
	return ((size = Block::getContentSize(size)) > overhead) ?
		size - overhead : 0;
}

//	FUNCTION private
//	Version::Page::getHashTable --
//		すべてのバージョンページ記述子を管理するハッシュ表を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたハッシュ表
//
//	EXCEPTIONS
//		なし

// static
HashTable<Page>&
Page::getHashTable()
{
	; _SYDNEY_ASSERT(_Page::_pageTable);
	return *_Page::_pageTable;
}

//	FUNCTION public
//	Version::Page::Memory::operator char* -- char* へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		バージョンページのバッファリング内容を格納する
//		自由記憶領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

Page::Memory::operator char*()
{
	return static_cast<Block::Memory*>(this)->operator char*() +
		sizeof(VersionLog::Log);
}

//	FUNCTION public
//	Version::Page::Memory::operator const char* --
//		const char* へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		バージョンページのバッファリング内容を格納する
//		自由記憶領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

Page::Memory::operator const char*() const
{
	return static_cast<const Block::Memory*>(this)->operator const char*() +
		sizeof(VersionLog::Log);
}

#ifdef OBSOLETE
//	FUNCTION public
//	Version::Page::Memory::copy -- バッファリング内容を上書きする
//
//	NOTES
//
//	ARGUMENTS
//		Version::Page::Memory&	src
//			自分自身のバッファリング内容へ上書きする
//			バッファリング内容をもつブロック
//
//	RETURN
//		上書き後の自分自身
//
//	EXCEPTIONS
//		なし

Page::Memory&
Page::Memory::copy(const Memory& src)
{
	; _SYDNEY_ASSERT(isOwner() && src.isOwner());
	; _SYDNEY_ASSERT(getSize() == src.getSize());

	Os::Memory::copy(operator void*(), src.operator const void*(), getSize());

	return *this;
}
#endif

//	FUNCTION public
//	Version::Page::Memory::reset -- バッファリング内容を初期化する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		初期化後の自分自身
//
//	EXCEPTIONS
//		なし

Page::Memory&
Page::Memory::reset()
{
	; _SYDNEY_ASSERT(isOwner());

	Os::Memory::reset(operator void*(), getSize());
	return *this;
}

//	FUNCTION public
//	Version::Page::Memory::unfix --
//		バージョンページのバッファリング内容をアンフィックスし、
//		所有権を放棄する
//
//	NOTES
//		バージョンページのバッファリング内容をを更新したにもかかわらず、
//		引数 dirty に false を与えたときの動作は保証しない
//
//	ARGUMENTS
//		bool				dirty
//			true
//				バージョンページのバッファリング内容を更新した
//			false
//				バージョンページのバッファリング内容を更新しなかった
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Page::Memory::unfix(bool dirty)
{
	if (isOwner() && _page) {

		{
		// バージョンページの
		// ページ更新トランザクションリストを保護するためにラッチする

		Os::AutoCriticalSection	latch(_page->getLatch());

		if (dirty || isDirty()) {

			if (_page->_file->isBatchInsert() == false)
				
				// バージョンページのバッファリング内容を更新したので、
				// バージョンページを更新したトランザクションの
				// トランザクション識別子を管理するリストに
				// 更新したトランザクションのものを登録する

				_Page::registerModifier(_page->_modifierList, _trans->getID());

			if (dirty)

				// バージョンページを記録するブロックの
				// バッファリング内容を実際に更新したことにする

				_Page::_Memory::dirty(*this);
		}

		// バージョンページを記録するブロックの
		// バッファリング内容をアンフィックスする

		static_cast<Block::Memory*>(this)->unfix();

		}

		// バージョンページ記述子の参照数を 1 減らし、
		// どこからも参照されなくなれば、破棄する

		Page::detach(_page, false);

	}
}

//	FUNCTION public
//	Version::Page::Memory::touch --
//		バージョンページのバッファリング内容に対する更新内容を破棄不可にする
//
//	NOTES
//
//	ARGUMENTS
//		bool				dirty
//			true
//				バージョンページのバッファリング内容を更新した
//			false
//				バージョンページのバッファリング内容を更新しなかった
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Page::Memory::touch(bool dirty)
{
	if (isOwner() && _page && (dirty || isDirty())) {

		Os::AutoCriticalSection	latch(_page->getLatch());

		
		if (_page->_file->isBatchInsert() == false)

			// バージョンページのバッファリング内容を更新したので、
			// バージョンページを更新したトランザクションの
			// トランザクション識別子を管理するリストに
			// 更新したトランザクションのものを登録する

			_Page::registerModifier(_page->_modifierList, _trans->getID());

		if (dirty)

			// バージョンページを記録するブロックの
			// バッファリング内容を実際に更新したことにする

			_Page::_Memory::dirty(*this);

		// バージョンページを記録するブロックの
		// バッファリング内容に対する更新内容を破棄不可にする

		static_cast<Block::Memory*>(this)->touch();
	}
}

//	FUNCTION public
//	Version::Page::Memory::dirty --
//		バージョンページのバッファリング内容を更新したことにする
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

void
Page::Memory::dirty()
{
	if (isOwner() && _page)
		_Page::_Memory::dirty(*this);
}

//	FUNCTION public
//	Version::Page::Memory::getPageID --
//		バージョンページのバージョンページ識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバージョンページ識別子
//
//	EXCEPTIONS
//		なし

Page::ID
Page::Memory::getPageID() const
{
	return (isOwner() && _page) ? _page->getID() : Block::IllegalID;
}

//	FUNCTION private
//	Version::Page::popFreeList -- フリーリストから得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//
//	EXCEPTIONS

Page*
Page::popFreeList()
{
	Os::AutoCriticalSection cAuto(_Page::_freeLatch);
	
	Page* page = 0;
	if (_Page::_freeList)
	{
		page = _Page::_freeList;
		_Page::_freeList = _Page::_freeList->_hashNext;

		--_Page::_freeListCount;
	}
	
	return page;
}

//	FUNCTION private
//	Version::Page::pushFreeList -- ページ記述子をリストにつなぐ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

void
Page::pushFreeList(Page* page_)
{
	Os::AutoCriticalSection cAuto(_Page::_freeLatch);
	
	if (_Page::_freeListCount < Configuration::PageInstanceCacheSize::get())
	{
		page_->_hashNext = _Page::_freeList;
		_Page::_freeList = page_;
		
		++_Page::_freeListCount;
	}
	else
	{
		delete page_;
	}
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
