// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Page.cpp -- バッファページ関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Buffer";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"
#include "SyFor.h"

#include "Buffer/AutoPage.h"
#include "Buffer/AutoPool.h"
#include "Buffer/Configuration.h"
#include "Buffer/Deterrent.h"
#include "Buffer/Manager.h"
#include "Buffer/Statistics.h"

#include "Common/Assert.h"
#include "Common/CRC.h"
#include "Common/Thread.h"
#include "Exception/BadDataPage.h"
#include "Exception/FlushPrevented.h"
#include "Exception/MemoryExhaust.h"
#include "Os/AutoCriticalSection.h"
#include "Os/AutoRWLock.h"
#include "Os/SysConf.h"
#include "Trans/Transaction.h"

#include "ModTypes.h"
#include "ModList.h"

#include "Exception/FakeError.h"
#include <new>

_SYDNEY_USING
_SYDNEY_BUFFER_USING

namespace 
{

namespace _Page
{
	//	$$$::_Page::_List --
	//		バッファページ記述子を管理するリストの型
	//
	//	NOTES

	typedef Common::DoubleLinkedList<Page>	_List;

	// 入出力の最小単位(B 単位)
	const Os::Memory::Size	IOUnit = Os::SysConf::PageSize::get();
	// あるページがファイルの先頭からなん番目の IOUnit から
	// 始まるかを求めるために、そのオフセットをなんビット右シフトすればよいか
	unsigned int			PageOffsetShift;
	// ハッシュ値の計算時にファイル記述子を
	// 格納する領域の先頭アドレスを何ビット右シフトするか
	const unsigned int		HashFilePtrShift = 8;

	// 生成済のバッファページ記述子を探す
	Page*
	find(HashTable<Page>::Bucket& bucket,
		 const File& file, Os::File::Offset offset);

	// あるバッファプールの
	// すべてのバッファページ記述子を管理するハッシュ表のハッシュ関数
	unsigned int
	pageTableHash(const File& file, Os::File::Offset offset);

	// すべてのバッファページ記述子を管理するハッシュ表
	//【注意】	_pageTable->getBucket(i).getLatch() で保護される
	HashTable<Page>*					_pageTable = 0;
	// すべての使用済バッファページ記述子を管理するハッシュ表
	_List*					_freeList = 0;
	Os::CriticalSection		_freeListLatch;

#ifdef USE_READ_BUFFER
	// read ahead 用のバッファを管理するためのクリティカルセクション
	Os::CriticalSection		_cReadAheadLatch;
	// read ahead 用のバッファを格納するリスト
	ModList<char*>			_cReadAheadList;
	// read ahead 用のバッファを取得する
	char*					popReadAheadBuffer();
	// read ahead 用のバッファを返却する
	void					pushReadAheadBuffer(char* buf_);
	// read ahead 用のバッファを自動的に返却するためのクラス
	class AutoReadAheadBuffer
	{
	public:
		// コンストラクタ
		AutoReadAheadBuffer(char* buf_)
			: _buf(buf_) {}
		// デストラクタ
		~AutoReadAheadBuffer()
			{
				pushReadAheadBuffer(_buf);
			}

		// バッファを取得する
		char* get() { return _buf; }

	private:
		char* _buf;
	};
#endif
}

//	FUNCTION
//	$$$::_Page::find -- 生成済のバッファページ記述子を探す
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::HashTable<Page>::Bucket&	bucket
//			バッファページ記述子が格納されるべきハッシュ表のバケット
//		Buffer::File&		file
//			探しているバッファページ記述子の表す
//			バッファページを持つバッファファイルのバッファファイル記述子
//		Os::File::Offset	offset
//			探しているバッファページ記述子の表す
//			バッファページがバッファリングする
//			OS ファイル領域のファイルの先頭からのオフセット(B 単位)
//
//	RETURN
//		0 以外の値
//			得られたバッファページ記述子を格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPTIONS
//		なし

Page*
_Page::find(HashTable<Page>::Bucket& bucket,
			const File& file, Os::File::Offset offset)
{
	//【注意】	呼び出し側で bucket.getLatch() をラッチする必要がある

	switch (bucket.getSize()) {
	default:
	{
		// バケットに登録されているバッファページ記述子のうち、
		// 与えられたバッファファイル記述子の表す
		// バッファファイルがバッファリングする OS ファイルの
		// 与えられたオフセットから始まる領域をバッファリングする
		// バッファページのものを探す

		HashTable<Page>::Bucket::Iterator			begin(bucket.begin());
		HashTable<Page>::Bucket::Iterator			ite(begin);
		const HashTable<Page>::Bucket::Iterator&	end = bucket.end();

		do {
			Page& page = *ite;

			if (page.getOffset() == offset &&
				page.getFileID() == file.getID()) {

				// 見つかったバッファページ記述子を
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

		if (page.getOffset() == offset && page.getFileID() == file.getID())

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
//		あるバッファプールのすべてのバッファページ記述子を
//		管理するハッシュ表に登録するためにハッシュ値を計算する
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::File&		file
//			ハッシュ表に登録するバッファページが所属する
//			バッファファイルのバッファファイル記述子
//		Os::File::Offset	offset
//			ハッシュ表に登録するバッファページがバッファリングする
//			OS ファイルのある領域の OS ファイルの先頭からのオフセット(B 単位)
//
//	RETURN
//		得られたハッシュ値
//
//	EXCEPTIONS
//		なし

inline
unsigned int
_Page::pageTableHash(const File& file, Os::File::Offset offset)
{
	// これは UNIX の page 構造体を管理するハッシュ表と同じハッシュ関数である

	return static_cast<unsigned int>(
		(offset >> PageOffsetShift) +
		(syd_reinterpret_cast<ModUPtr>(&file) >> HashFilePtrShift));
}

#ifdef USE_READ_BUFFER
//	FUNCTION
//	$$$::_Page::popReadAheadBuffer --
//		read ahead 用のバッファを取得する

char*
_Page::popReadAheadBuffer()
{
	char*	p = 0;
	
	Os::AutoCriticalSection cAuto(_cReadAheadLatch);
	if (_cReadAheadList.getSize())
	{
		// 残っているのでそれを返す
		p = _cReadAheadList.getFront();
		_cReadAheadList.popFront();
	}
	else
	{
		// バッファが存在していないので、確保する
		p = syd_reinterpret_cast<char*>(
			Os::Memory::map(Configuration::ReadAheadBlockSize::get(),
							false));
	}

	return p;
}

void
_Page::pushReadAheadBuffer(char* buf_)
{
	Os::AutoCriticalSection cAuto(_cReadAheadLatch);
	_cReadAheadList.pushBack(buf_);
}
#endif

}

//	FUNCTION private
//	Buffer::Manager::Page::initialize --
//		マネージャーの初期化のうち、バッファページ関連のものを行う
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
	try {
		// ページオフセットをなんビット右シフトすると、
		// そのページがファイルの先頭からなん番目の IOUnit かがわかるか調べる

		for (_Page::PageOffsetShift = 0;
			 static_cast<Os::Memory::Size>(
				 1 << _Page::PageOffsetShift) < _Page::IOUnit;
			 ++_Page::PageOffsetShift) ;

		// すべてのバッファページ記述子を管理するハッシュ表のサイズを求める

		unsigned int length = Configuration::PageTableSize::get();
		if (length == Configuration::PageTableSize::Default)
		{
			// Buffer_NormalPoolSize が256M以上の場合には、
			// ハッシュ表のサイズも大きくする

			length *= static_cast<unsigned int>(
				(Configuration::NormalPoolSize::get() + (1 << 28) - 1) >> 28);
		}

		// すべてのバッファページ記述子を管理するハッシュ表を確保する

		_Page::_pageTable = new HashTable<Buffer::Page>(
			length,
			&Buffer::Page::_hashPrev, &Buffer::Page::_hashNext);
		; _SYDNEY_ASSERT(_Page::_pageTable);

		// 使用済のバッファファイル記述子を管理するリストを確保する

		_Page::_freeList = new _Page::_List(
			&Buffer::Page::_hashPrev, &Buffer::Page::_hashNext);
		; _SYDNEY_ASSERT(_Page::_freeList);

	} catch (...) {

		terminate();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Buffer::Manager::Page::terminate --
//		マネージャーの後処理のうち、バッファページ関連のものを行う
//
//	NOTES
//		Buffer::Manager::Daemon::terminate を先に呼び出して、
//		ダーティなバッファページをすべてフラッシュしておく必要がある
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
	unsigned int i = 0;
	do {
		// このバッファプールのすべてのバッファページ記述子を破棄する
		//
		//【注意】	処理するバッファプールの参照数を増やし、
		//			バッファページがすべて破棄されたときに、
		//			処理中のバッファプールが破棄されないようにする

		AutoPool pool(
			Buffer::Pool::attach(
				static_cast<Buffer::Pool::Category::Value>(i)));
		pool->discardPage(Buffer::Pool::DiscardablePageFilter::ForPool(), 0);

	} while (++i < Buffer::Pool::Category::Count) ;

	if (_Page::_freeList) {

		// 使用済のバッファページ記述子をすべて破棄する

		while (_Page::_freeList->getSize()) {
			Buffer::Page* page = &_Page::_freeList->getFront();
			_Page::_freeList->popFront();
			delete page;
		}

		// 使用済のバッファファイル記述子を管理するリストを破棄する

		delete _Page::_freeList, _Page::_freeList = 0;
	}

	// すべてのバッファページ記述子を管理するハッシュ表を削除する

	delete _Page::_pageTable, _Page::_pageTable = 0;
}

//	FUNCTION private
//	Buffer::Page::attach -- バッファページ記述子を得る
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::File&		file
//			得ようとしているバッファページ記述子の表す
//			バッファページを持つバッファファイルのバッファファイル記述子
//		Os::File::Offset	offset
//			得ようとしているバッファページ記述子の表す
//			バッファページがバッファリングする
//			OS ファイル領域のファイルの先頭からのオフセット(B 単位)
//
//	RETURN
//		得られたバッファページ記述子を格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Page*
Page::attach(File& file, Os::File::Offset offset)
{
	// 与えられたバッファファイル記述子の表す
	// バッファファイルがバッファリングする OS ファイルの
	// 与えられたオフセットから始まる領域を
	// バッファリングするバッファページの
	// バッファページ記述子を格納すべきハッシュ表のバケットを求める

	HashTable<Page>::Bucket& bucket = getBucket(file, offset);

	// バケットを保護するためにラッチする

	Os::AutoCriticalSection	latch(bucket.getLatch());

	// 与えられたオフセットから始まる領域をバッファリングする
	// バッファページのバッファページ記述子が
	// 求めたバケットに登録されていれば、それを得る

	Page* page = _Page::find(bucket, file, offset);
	if (page)

		// 見つかったので、参照回数を 1 増やす

		++page->_refCount;
	else {

		// 使用済のバッファページ記述子があれば、再利用する

		; _SYDNEY_ASSERT(_Page::_freeList);
		{
		Os::AutoCriticalSection	latch(_Page::_freeListLatch);

		if (_Page::_freeList->getSize()) {
			page = &_Page::_freeList->getFront();
			_Page::_freeList->popFront();

			page->~Page();
			page = new(page) Page(file, offset);
		}
		}
		if (!page) {

			// 見つからなかったので、生成する

			; _SYDNEY_FAKE_ERROR("Buffer::Page::attach", std::bad_alloc());

			page = new Page(file, offset);
			; _SYDNEY_ASSERT(page);
		}

		// 生成したバッファページ記述子が存在する間に
		// 与えられたバッファファイル記述子が破棄されると困るので、
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

//	FUNCTION private
//	Buffer::Page::attach -- バッファページ記述子の参照数を 1 増やす
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		参照数が 1 増えたバッファページ記述子を格納する領域の先頭アドレス
//
//	EXCEPTIONS

Page*
Page::attach()
{
	// 参照数を 1 増やすバッファファイル記述子を格納する
	// バケットを保護するためにラッチする

	Os::AutoCriticalSection	latch(getBucket().getLatch());

	// 参照数を 1 増やす

	++_refCount;

	return this;
}

//	FUNCTION private
//	Buffer::Page::detach -- バッファページ記述子の参照をやめる
//
//	NOTES
//		バッファページ記述子の参照をやめても、
//		他のどこかで参照されていれば、バッファページ記述子は破棄されない
//		逆にどこからも参照されていないとき、
//		バッファページ記述子は破棄される
//
//	ARGUMENTS
//		Buffer::Page*&		page
//			参照をやめるバッファページ記述子を格納する領域の先頭アドレスで、
//			呼び出しから返ると 0 になる
//		bool				reserve
//			true
//				どこからも参照されなくなったバッファページ記述子でも、
//				また参照されるときのために破棄せずにとっておく
//			false
//				どこからも参照されなくなったバッファページ記述子は破棄する
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
		{
		// 与えられたバッファページ記述子を格納する
		// ハッシュ表のバケットを求め、保護するためにラッチする

		HashTable<Page>::Bucket& bucket = page->getBucket();
		Os::AutoCriticalSection	latch(bucket.getLatch());

		// 参照数が 0 より大きければ 1 減らす

		if ((page->_refCount && --page->_refCount) || reserve)

			// 他から参照されているか、
			// また参照されるときのためにとっておくので、
			// 破棄できない

			page = 0;
		else
			// バケットから取り除く

			bucket.erase(*page);
			
		// ここで、バケットを保護するためのラッチがはずれる
		}
		if (page) {
			File* file = &page->_file;

			; _SYDNEY_ASSERT(_Page::_freeList);
			{
			Os::AutoCriticalSection	latch(_Page::_freeListLatch);

			if (_Page::_freeList->getSize() <
				Configuration::FreePageCountMax::get())

				// どこからも参照されていないバッファページ記述子は
				// 使用済リストにつなぎ、後日、再利用されるようにする

				_Page::_freeList->pushBack(*page);
			else

				// 使用済リストに空きがないので、バッファページ記述子を破棄する

				delete page;
			}

			page = 0;

			// 破棄したバッファページ記述子が参照していた
			// バッファファイル記述子の参照数を 1 減らし、
			// どこからも参照されなくなったら破棄する

			File::detach(file);
		}
	}
}

//	FUNCTION private
//	Buffer::Page::detach -- バッファページ記述子の参照数を 1 減らす
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
	// 参照数を 1 減らすバッファページ記述子を格納する
	// バケットを保護するためにラッチする

	Os::AutoCriticalSection	latch(getBucket().getLatch());
#ifdef DEBUG
	; _SYDNEY_ASSERT(_refCount);
#else
	if (_refCount)
#endif
		// 参照数が 0 より大きければ、1 減らす

		--_refCount;
}

//	FUNCTION private
//	Buffer::Page::getRefCount -- バッファページ記述子の参照数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた参照数
//
//	EXCEPTIONS

unsigned int
Page::getRefCount() const
{
	// 参照数を得るバッファページ記述子を格納する
	// バケットを保護するためにラッチする

	Os::AutoCriticalSection	latch(getBucket().getLatch());

	return _refCount;
}

//	FUNCTION private
//	Buffer::Page::getBucket --
//		バッファページ記述子を格納すべきハッシュ表のバケットを求める
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::File&		file
//			バッファページ記述子の表すバッファページを持つ
//			バッファファイルのバッファファイル記述子
//		Os::File::Offset	offset
//			バッファページ記述子の表すバッファページが
//			バッファリングする OS ファイル領域の
//			OS ファイルの先頭からのオフセット(B 単位)
//
//	RETURN
//		得られたバケット
//
//	EXCEPTIONS
//		なし

// static
HashTable<Page>::Bucket&
Page::getBucket(File& file, Os::File::Offset offset)
{
	; _SYDNEY_ASSERT(_Page::_pageTable);

	const unsigned int addr =
		_Page::pageTableHash(file, offset) % _Page::_pageTable->getLength();
	return _Page::_pageTable->getBucket(addr);
}

//	FUNCTION public
//	Buffer::Page::fix --
//		バッファリングされたある OS ファイル領域をフィックスする
//
//	NOTES
//		必要に応じて、フィックスする OS ファイル領域は読み出されて、
//		バッファリングされる
//
//		フィックスされたある OS ファイル領域のバッファページは、
//		アンフィックスされるまで、バッファから棄てることはできない
//
//		ひとつのスレッドから同じバッファページに対して、
//		更新のためのフィックスと
//		更新または参照のためのフィックスを同時に行うと、
//		デッドロックが発生し、そのスレッドはハングアップする
//
//	ARGUMENTS
//		Buffer::File&		file
//			フィックスする OS ファイル領域をバッファリングする
//			バッファページ記述子を持つバッファファイルの
//			バッファファイル記述子
//		Os::File::Offset	offset
//			フィックスする OS ファイル領域の
//			OS ファイルの先頭からのオフセット(B 単位)
//		Buffer::Page::FixMode::Value	mode
//			以下の値の論理和を指定する
//			Buffer::Page::FixMode::ReadOnly
//				フィックスする OS ファイル領域は参照しかされない
//			Buffer::Page::FixMode::Write
//				フィックスする OS ファイル領域は更新される
//			Buffer::Page::FixMode::Allocate
//				フィックスする OS ファイル領域はその初期化のために使用する
//			Buffer::Page::FixMode::Deterrentable
//				フィックスした OS ファイル領域へのフラッシュは抑制され得る
//			Buffer::Page::FixMode::Discardable
//				フィックスした OS ファイル領域の更新は作業用メモリを介して行う
//			Buffer::Page::FixMode::NoLock
//				フィックスする OS ファイル領域の操作の
//				排他制御用の読み取り書き込みロックを行わない
//
//	RETURN
//		フィックスした OS ファイル領域を記録する
//		バッファメモリを操作するためのクラス
//
//	EXCEPTIONS

// static
Memory
Page::fix(File& file, Os::File::Offset offset, FixMode::Value mode,
		  const Trans::Transaction* trans)
{
	// バッファメモリ操作の排他制御用のロックモードを求める

	mode &= FixMode::Mask;
	if (file._pool.getCategory() == Pool::Category::ReadOnly) {
		; _SYDNEY_ASSERT(mode & FixMode::ReadOnly);
		mode |= FixMode::NoLock;
	}

	const Os::RWLock::Mode::Value lockMode =
		(mode & FixMode::NoLock) ? Os::RWLock::Mode::Unknown :
		(mode & FixMode::ReadOnly) ? Os::RWLock::Mode::Read :
									 Os::RWLock::Mode::Write;

	// 初期化のためにフィックスしようとしているか

	const bool reset = mode & FixMode::Allocate;

	// 与えられたオフセットの OS ファイル領域を
	// バッファリングするバッファページ記述子を得る
	//
	//【注意】	回数が多いので AutoPage による間接参照は可能な限り行わない
	//
	//【注意】 	dirty リストに存在する Page を attach してもこれ以下で例外が
	//			発生することはないはずなので、reserve する必要はない。
	//			逆に reserve してしまうと、破棄するタイミングがなく、
	//			ファイルの参照カウンタが残ってしまう場合がある。

	AutoPage destructor(Page::attach(file, offset), false);
	Page& page = *destructor;

	const Os::Memory::Size size = page.getSize();
	unsigned int retryCount = 0;

	while (true)
	{

		if (lockMode)

			// バッファメモリ操作の排他制御用の
			// 読み取り書き込みロックをする

			page.getRWLock().lock(lockMode);

		while (true)
		{

			try {
				// バッファページを保護するためにラッチする

				Os::AutoCriticalSection latch(page.getLatch());

				try {
					bool allocated;

					try {
						// バッファプールを保護するためにラッチする

						Os::AutoCriticalSection latch(file._pool.getLatch());

						// バッファメモリをどうにかして確保する

						bool force = (retryCount != 0);
						page._body =
							file._pool.replaceMemory(size, reset,
													 allocated, page, force);
						; _SYDNEY_ASSERT(page._body);

					} catch (Exception::MemoryExhaust&) {

						// バッファプールの上限に達したため、
						// これ以上、バッファメモリを確保できなかった

						if (++retryCount ==
							Configuration::RetryAllocationCountMax::get())

							// あきらめる

							_SYDNEY_RETHROW;

						// エラー状況を解除する

						Common::Thread::resetErrorCondition();

						break;
					}

					if (reset && !allocated)

						// バッファメモリを初期化する

						(void) Os::Memory::reset(page._body, size);

					switch (page._status) {
					case Status::Empty:

						// バッファページの状態を変更する

						page._status = Status::NoRead;
						// thru

					case Status::NoRead:
						
						if (!reset) {
							// OS ファイルからバッファページを読み込む
							
							readAhead(file, page, offset);

							if (trans)

								// 本来なら引数から const を外したいが、
								// ファイルドライバから const がついているので、
								// ここで const_cast していまう
								
								const_cast<Trans::Transaction*>(trans)
									->addPageReadCount();
						}

						page._status = Status::Read;
						// thru

					case Status::Read:
						
						if (!reset) {
							// バッファページがOSファイル領域に正しく格納され、
							// それを正しく読み出したかを検証する

							page.verify();
						}

						// バッファページの状態を変更する

						page._status = Status::Normal;
					}

					if (trans)

						// 本来なら引数から const を外したいが、
						// ファイルドライバから const がついているので、
						// ここで const_cast していまう

						const_cast<Trans::Transaction*>(trans)
							->addPageReferenceCount();

					// 統計情報を記録する
					Statistics::record(Statistics::Category::Fix, size);

					// バッファメモリを外部から操作するためのクラスを生成し、
					// 返す
					//
					//【注意】	生成したバッファページ記述子は
					//			アンフィックスするまで存在することが保証される
					//【注意】	読み取り書き込みロックしていれば、
					//			アンフィックスするまでアンロックされない

					return Memory(mode, *destructor.release(), reset);

				} catch (...) {

					if (page._status != Status::Empty) {
						{
							// バッファプールを保護するためにラッチする

							Os::AutoCriticalSection
								latch(file._pool.getLatch());

							// LRU リスト T1 または T2 から
							// バッファページ記述子をはずす

							file._pool.getLruList(page._lruListCategory)
								.erase(page);

							// バッファファイル内の LRU リストに存在している
							// ページリストからも削除する

							page._file._pageList.erase(page);
				
							// バッファメモリを破棄する

							file._pool.freeMemory(page._body, size);
						}
						// バッファページの状態を未確保とする

						page._status = Page::Status::Empty;
					}
					// BadDataPageを期待している場合があるので、ログには出さない
					throw;
				}
			} catch (Exception::BadDataPage&) {

				if (lockMode)
					page.getRWLock().unlock(lockMode);
				// BadDataPageを期待している場合があるので、ログには出さない
				throw;
			} catch (...) {

				if (lockMode)
					page.getRWLock().unlock(lockMode);
				_SYDNEY_RETHROW;
			}

		}

		if (lockMode)
			page.getRWLock().unlock(lockMode);

		// 統計情報を記録する
		Statistics::record(Statistics::Category::Exhaust, size);

		// ダーティリストを切り替えるかもしれないので、書き込みロックしておく

		Os::AutoRWLock rwlock(file._pool.getRWLock(), Os::RWLock::Mode::Write);

		// ダーティなバッファページをできるかぎりフラッシュする

		file._pool.flushDirtyPage(
			Pool::FlushablePageFilter::ForPool(false, false), false);

		// 再試行する
	}
}

//	FUNCTION private
//	Buffer::Page::unfix --
//		バッファリングしている OS ファイル領域をアンフィックスする
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Memory::Category::Value	category
//			アンフィックスするバッファページのバッファメモリの種別で、
//			Buffer::Memory::Category::Value の論理和を指定する
//		Buffer::Page::UnfixMode::Value
//			以下のいずれかの値を指定する
//			Buffer::Page::UnfixMode::None
//				アンフィックスするだけで、なにもしない
//			Buffer::Page::UnfixMode::Dirty
//				ダーティにしたので、後でフラッシュする必要がある
//			Buffer::Page::UnfixMode::Flush
//				ダーティにしたので、すぐフラッシュする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Page::unfix(Memory::Category::Value category, UnfixMode::Value mode)
{
	const Os::Memory::Size size = getSize();

	if (category != Memory::Category::ReadOnly) {

		// 更新可能なバッファメモリを持つ
		// バッファページをアンフィックスしようとしている

		switch (mode) {
		case UnfixMode::None:
			if (category & Memory::Category::Discardable) {

				// バッファページを保護するためにラッチする

				Os::AutoCriticalSection	latch(getLatch());

				if (_working) {

					// バッファプールを保護するためにラッチする

					Os::AutoCriticalSection	latch(_file._pool.getLatch());

					// 作業用メモリが確保されているので、破棄する
					//
					//【注意】	以前は確保したままにしていたが
					//			2 倍のメモリが確保されたままになり、
					//			ディスクの入出力が増えてしまい、遅くなっていた

					_file._pool.freeMemory(_working, size);
				}
			}
			break;

		case UnfixMode::Dirty:
		{
			// バッファページを保護するためにラッチする

			Os::AutoCriticalSection	latch(getLatch());

			if (category & Memory::Category::Discardable) {
				if (!_working)

					// 作業用メモリが確保されていないので、
					// バッファページは更新されていなかった

					break;

				if (getRefCount() > 1)

					// バッファメモリは自分以外から参照されているので、
					// 作業用メモリの内容でバッファメモリを上書きする

					(void) Os::Memory::copy(_body, _working, size);
				else

					// 更新した作業用メモリとバッファメモリを入れ替える

					ModSwap(_body, _working);

				// バッファプールを保護するためにラッチする

				Os::AutoCriticalSection	latch(_file._pool.getLatch());

				// 作業用メモリを破棄する

				_file._pool.freeMemory(_working, size);

				// バッファページの状態を変更する

				if (_status & Status::Normal) {
					if (!(_status & Status::Flushable)) {

						// ダーティリストに登録されていないので、登録する

						; _SYDNEY_ASSERT(_file._pool._dirtyList);
						_file._pool._dirtyList->pushBack(this);
					}
					_status = Status::Dirty | Status::Flushable;
				}
			} else
				if (_status & Status::Normal) {
					if (!(_status & Status::Flushable)) {
						Os::AutoCriticalSection	latch(_file._pool.getLatch());

						; _SYDNEY_ASSERT(_file._pool._dirtyList);
						_file._pool._dirtyList->pushBack(this);
					}
					_status = Status::Dirty | Status::Flushable;
				}

			if (category & Memory::Category::Deterrentable)
				_status |= Status::Deterrentable;
			break;
		}
		case UnfixMode::Flush:
		{
			// フラッシュ中に
			// バッファページのフラッシュの抑止が開始されたり、
			// 終了されたりしないように、読み取りロックする

			Os::AutoRWLock rwlock(
				Deterrent::getRWLock(), Os::RWLock::Mode::Read);

			// バッファページを保護するためにラッチする

			Os::AutoCriticalSection	latch(getLatch());

			if (_file.isDeterred() &&
				(category & Memory::Category::Deterrentable ||
				 _status & Status::Deterrentable))

				// バッファページのフラッシュが抑止されているので、
				// フラッシュの抑止が可能なバッファページはフラッシュできない

				_SYDNEY_THROW0(Exception::FlushPrevented);

			if (category & Memory::Category::Discardable) {
				if (!_working)
					break;

				if (getRefCount() > 1)
					(void) Os::Memory::copy(_body, _working, size);
				else
					ModSwap(_body, _working);

				Os::AutoCriticalSection	latch(_file._pool.getLatch());

				_file._pool.freeMemory(_working, size);
			}

			// 書き込んだものと読み出したものが
			// 同じであることを検証するために CRC を計算する

			calculateCRC();
			{
			// バッファファイルを保護するためにラッチする

			Os::AutoCriticalSection	latch(_file.getLatch());

			// バッファファイルをオープンする

			_file.openOsFile();

			// バッファメモリを書き込む

			_file.writeOsFile(Os::File::IOBuffer(_body), size, getOffset());

			// OSがキャッシュしている内容をDISKに書き出す
			
			_file.sync();
			
			}
			// バッファページの状態を変更する
			//
			//【注意】	ダーティリストへの登録はそのままになり、
			//			マークやフラッシュの抑止は消える

			_status = Status::Normal | (_status & Status::Flushable);
		}
		}
#ifdef DEBUG
	} else {
		; _SYDNEY_ASSERT(mode == UnfixMode::None);
#endif
	}

	if (!(category & Memory::Category::NoLock))

		// バッファメモリ操作の排他制御用の
		// 読み取り書き込みロックをはずす

		getRWLock().unlock((category & Memory::Category::ReadOnly) ?
						   Os::RWLock::Mode::Read : Os::RWLock::Mode::Write);

	// 統計情報を記録する
	Statistics::record(Statistics::Category::Unfix, size);

	// 参照数を 1 減らす

	detach();
}

//	FUNCTION private
//	Buffer::Page::touch --
//		バッファリングしている OS ファイル領域の
//		これまでの更新内容を破棄不可にする
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Memory::Category::Value	category
//			バッファページのバッファメモリの種別で、
//			Buffer::Memory::Category::Value の論理和を指定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Page::touch(Buffer::Memory::Category::Value	category)
{
	if (category != Memory::Category::ReadOnly) {

		// バッファページを保護するためにラッチする

		Os::AutoCriticalSection	latch(getLatch());

		if (category & Memory::Category::Discardable) {
			if (!_working)

				// 作業用メモリが確保されていないので、
				// バッファページは更新されていなかった

				return;

			// 作業用メモリの内容でバッファメモリを上書きする

			(void) Os::Memory::copy(_body, _working, getSize());
		}

		// バッファページの状態を変更する

		if (_status & Status::Normal) {
			if (!(_status & Status::Flushable)) {

				// バッファプールを保護するためにラッチする

				Os::AutoCriticalSection	latch(_file._pool.getLatch());

				// ダーティリストに登録されていないので、登録する

				; _SYDNEY_ASSERT(_file._pool._dirtyList);
				_file._pool._dirtyList->pushBack(this);
			}
			_status = Status::Dirty | Status::Flushable;
		}
		if (category & Memory::Category::Deterrentable)
			_status |= Status::Deterrentable;
	}
}

//	FUNCTION public
//	Buffer::Page::getContentSize --
//		あるサイズのバッファページに実際に格納可能なデータのサイズを求める
//
//	NOTES
//
//	ARGUMENTS
//		Os::Memory::Size	size
//			格納可能なデータのサイズを計算するバッファページのサイズ(B 単位)
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
	static const Os::Memory::Size overhead = sizeof(Header) + sizeof(Footer);

	return (size > overhead) ? size - overhead : 0;
}

//	FUNCTION private
//	Buffer::Page::calculateCRC --
//		読み出したバッファリング内容が書き込み時と
//		同じものかを検証するための CRC を計算する
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
Page::calculateCRC()
{
	//【注意】	バッファページはラッチ済であること

	; _SYDNEY_ASSERT(_body);

	// バッファページの先頭にあるバッファページヘッダに、
	// ヘッダからその末尾の CRC 格納部を
	// 除いた部分の CRC を計算し、記録する

	Header& header = *static_cast<Header*>(_body);

	header._calculated =
		Configuration::CalculateCheckSum::All <=
			Configuration::CalculateCheckSum::get() + (!_file._noCRC);
	header._crc = Common::CRC16::generate(
		_body, sizeof(header) - sizeof(header._crc));

	if (header._calculated) {

		// バッファページからその末尾の CRC 格納部を
		// 除いた部分の CRC を計算し、記録する

		const Os::Memory::Size n = getSize() - sizeof(Footer);
		Footer& footer =
			*syd_reinterpret_cast<Footer*>(static_cast<char*>(_body) + n);

		footer._crc = Common::CRC32::generate(_body, n);
	}
}

//	FUNCTION private
//	Buffer::Page::verify --
//		バッファリング内容が正しく格納され、読み出されたかを確認する
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
//		Exception::BadDataPage
//			バッファリング内容に不正が見つかった

void
Page::verify() const
{
	//【注意】	バッファページはラッチ済であること

	; _SYDNEY_ASSERT(_body);

	// バッファページヘッダの CRC と
	// 必要があれば、バッファページ全体の CRC を検証する

	if (!Common::CRC16::verify(_body, sizeof(Header)) ||
		(!_file._noCRC && static_cast<const Header*>(_body)->_calculated &&
		 !Common::CRC32::verify(_body, getSize())))

		// 不正が見つかった

		_SYDNEY_THROW2(Exception::BadDataPage, getOffset(), _file.getPath());
}

//	FUNCTION public
//	Buffer::Page::correctSize -- ページサイズを検算する
//
//	NOTES
//		ページサイズはシステムのメモリページサイズのべき数である必要がある
//
//	ARGUMENTS
//		Os::Memory::Size	size
//			検算するページサイズ(B 単位)
//
//	RETURN
//		検算結果(B 単位)
//
//	EXCEPTIONS

// static
Os::Memory::Size
Page::correctSize(Os::Memory::Size size)
{
	// 2 のべき乗のうち、
	// ページサイズの最大値以下、与えられた値以上で、
	// システムのメモリページサイズの倍数の最小のものを求める

	Os::Memory::Size i = Os::SysConf::PageSize::get();
	for (; size > i; i <<= 1) ;

	const Os::Memory::Size max = Configuration::PageSizeMax::get();
	return (max < i) ? max : i;
}

//
//	FUNCTION private
//	Buffer::Page::readAhead -- OS ファイルからページを読み込む
//
//	NOTES
//	OS ファイルから指定のページの内容をバッファに読み込む
//	さらに、指定された offset から先のページの内容を先読みする
//
//	ARGUMENTS
//	Buffer::File& file_
//		OS ファイルから読み込むバッファページ記述子をもつバッファファイルの
//		バッファファイル記述子
//	Buffer::Page& page_
//		OS ファイルから読み込むバッファページ記述子
//	Os::File::Offset offset_
//		OS ファイルから読み込むバッファページ領域の
//		ファイルの先頭からのオフセット(B 単位)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
//static
#ifdef USE_READ_BUFFER
void
Page::readAhead(File& file_, Page& page_, Os::File::Offset offset_)
{
	// 【注意】
	//	引数の page_ はラッチ済みであること
	
	// 【注意】
	//	本来なら、IOVMAX 個の配列を用意するべきであるが、
	//	ヒープメモリーを確保したくないので、スタック上に十分な大きさのものを
	//	確保しておく

	//	先読みは Configuration::ReadAheadBlockSize のブロック単位で行う

	// ページサイズの最小値は 4KB である
	// 本来なら、_Page::IOUnit を利用するべきであるが、スタック上に
	// 配列を用意したいので、ここはハードコーディングとする
	// よって、各アーキテクチャーの内、一番小さいものとする

	const Os::Memory::Size	pageSizeMin = (4 << 10);
	const int ARRAY_SIZE
		= Configuration::ReadAheadBlockSize::Maximum / pageSizeMin;

	Page*				pages[ARRAY_SIZE];
	
	const Os::Memory::Size pageSize = page_.getSize();
	const Os::File::Size fileSize = file_.getSize();
	const int unitCount = static_cast<int>(pageSize / _Page::IOUnit);
	
	// 先読みするブロック数の上限
	unsigned int blockSize = Configuration::ReadAheadBlockSize::get();
	if (blockSize < pageSize)
		// ページサイズよりも小さかったらページサイズとする
		blockSize = pageSize;

	Os::Memory::reset(pages, sizeof(Page*) * (blockSize / pageSize));

	// 与えられた位置をブロックの境界に丸める
	const Os::File::Offset start
		= offset_ & ~(static_cast<Os::File::Offset>(blockSize - 1));

	// ブロックの境界内の与えられた位置
	const int pagepos = static_cast<int>((offset_ - start) / pageSize);
	// 与えられたページの位置
	Os::File::Offset offset = offset_;

	// 与えられたページのみ読み込めたかどうか
	bool isRead = false;
	
	int pagestart = pagepos;
	int pageend = pagepos;

	Page* p = &page_;
	pages[pageend++] = p;

	// 読み込むためのバッファを取得する
	_Page::AutoReadAheadBuffer buf = _Page::popReadAheadBuffer();

	try
	{
		// まず、与えられたページの後方の先読みするページを取得する
		// ロックできなかったり、途中にすでに読み込み済みのページが
		// あったりしたらそこまでとする

		while ((offset += pageSize) < (start + blockSize))
		{
			if (static_cast<Os::File::Size>(offset) >= fileSize)
				// ファイルの最後まで読んだので終了
				break;

			// 次のページを得る
		
			p = Page::attach(file_, offset);

			// ロックできないページがあったり、すでに読み込んでいるページが
			// あったら、それ以降は先読みしない
		
			if (!p->getRWLock().trylock(Os::RWLock::Mode::Read))
			{
				p->detach();
				break;
			}
			if (!p->getLatch().trylock())
			{
				p->getRWLock().unlock(Os::RWLock::Mode::Read);
				p->detach();
				break;
			}
			if (p->_status > Status::NoRead)
			{
				p->getLatch().unlock();
				p->getRWLock().unlock(Os::RWLock::Mode::Read);
				p->detach();
				break;
			}

			try {
				// バッファプールを保護するためにラッチする

				Os::AutoCriticalSection latch(file_._pool.getLatch());

				// バッファメモリを確保する

				bool allocated;

				p->_body =
					file_._pool.replaceMemory(pageSize, false,
											  allocated, *p, false);
				; _SYDNEY_ASSERT(p->_body);

				// ステータスを変更
				p->_status = Status::NoRead;

			} catch (Exception::MemoryExhaust&) {

				// バッファプールの上限に達したため、
				// これ以上、バッファメモリを確保できなかった
				// あきらめる

				// エラー状況を解除する
				Common::Thread::resetErrorCondition();
			
				p->getLatch().unlock();
				p->getRWLock().unlock(Os::RWLock::Mode::Read);

				// ここで例外が発生するということは、
				// LRUにもどこにもないページであるということ。
				// なので、参照カウンタが0であったら、破棄する
				
				Page::detach(p, false);
				
				break;
				
			} catch (...) {

				// その他の例外
				
				p->getLatch().unlock();
				p->getRWLock().unlock(Os::RWLock::Mode::Read);

				// ここで例外が発生するということは、
				// LRUにもどこにもないページであるということ。
				// なので、参照カウンタが0であったら、破棄する
				
				Page::detach(p, false);
				
				_SYDNEY_RETHROW;
			}

			// 先読みするページである
			pages[pageend++] = p;
		}

		// 次に、与えられたページの前方の先読みするページを取得する
		// ロックできなかったり、途中にすでに読み込み済みのページが
		// あったりしたらそこまでとする

		offset = offset_;

		while (offset != start)
		{
			// 前のページを得る

			p = Page::attach(file_, offset - pageSize);

			// ロックできないページがあったり、すでに読み込んでいるページが
			// あったら、それ以降は先読みしない
		
			if (!p->getRWLock().trylock(Os::RWLock::Mode::Read))
			{
				p->detach();
				break;
			}
			if (!p->getLatch().trylock())
			{
				p->getRWLock().unlock(Os::RWLock::Mode::Read);
				p->detach();
				break;
			}
			if (p->_status > Status::NoRead)
			{
				p->getLatch().unlock();
				p->getRWLock().unlock(Os::RWLock::Mode::Read);
				p->detach();
				break;
			}

			try {
				// バッファプールを保護するためにラッチする

				Os::AutoCriticalSection latch(file_._pool.getLatch());

				// バッファメモリを確保する

				bool allocated;

				p->_body =
					file_._pool.replaceMemory(pageSize, false,
											  allocated, *p, false);
				; _SYDNEY_ASSERT(p->_body);

				// ステータスを変更
				p->_status = Status::NoRead;

			} catch (Exception::MemoryExhaust&) {

				// バッファプールの上限に達したため、
				// これ以上、バッファメモリを確保できなかった
				// あきらめる

				// エラー状況を解除する
				Common::Thread::resetErrorCondition();
			
				p->getLatch().unlock();
				p->getRWLock().unlock(Os::RWLock::Mode::Read);

				// ここで例外が発生するということは、
				// LRUにもどこにもないページであるということ。
				// なので、参照カウンタが0であったら、破棄する
				
				Page::detach(p, false);
				
				break;
				
			} catch (...) {

				// その他の例外
				
				p->getLatch().unlock();
				p->getRWLock().unlock(Os::RWLock::Mode::Read);

				// ここで例外が発生するということは、
				// LRUにもどこにもないページであるということ。
				// なので、参照カウンタが0であったら、破棄する
				
				Page::detach(p, false);
				
				_SYDNEY_RETHROW;
			}

			// 先読みするページである
			pages[--pagestart] = p;

			offset -= pageSize;
		}

		// 先読みするページを確保できたので、読み込む

		try
		{
			// バッファファイルを保護するためにラッチする

			Os::AutoCriticalSection	latch(file_.getLatch());

			// バッファファイルをオープンする

			bool nolimit = file_.openOsFile();

			// 与えられたオフセットから iovcount 分の
			// OS ファイル領域を読み出す

			Os::File::IOBuffer b(buf.get());
			file_.readOsFile(b, pageSize * (pageend - pagestart), offset);

			if (!nolimit)

				// ファイルディスクリプタの上限に達している
				// ので、クローズする

				file_.closeOsFile();
			
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			// 先読みするのに失敗したので、与えられたページのみ読み込む

			try
			{
				// バッファファイルを保護するためにラッチする

				Os::AutoCriticalSection	latch(file_.getLatch());

				// バッファファイルをオープンする

				bool nolimit = file_.openOsFile();

				// 与えられたオフセットからページサイズぶんの
				// OS ファイル領域を読み出す

				Os::File::IOBuffer buf(page_._body);
				file_.readOsFile(buf, pageSize, offset_);

				if (!nolimit)

					// ファイルディスクリプタの上限に達している
					// ので、クローズする

					file_.closeOsFile();

				// 読み込めた
				
				isRead = true;
			}
			catch (...)
			{
				// やっぱり読み込めなかった
				;
			}
			
			_SYDNEY_RETHROW;
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		for (int i = pagestart; i < pageend; ++i)
		{
			if (i == pagepos)
				// 引数の与えられたページの部分は飛ばす
				continue;

			// 例外が発生した場合には、確保したバッファを返し、
			// 参照カウンタが 0 であったら、ページ記述子も破棄する
			
			AutoPage destructor(pages[i], false);
			Page& page = *destructor;

			if (page._status != Status::Empty)
			{
				{
					// バッファプールを保護するためにラッチする

					Os::AutoCriticalSection
						latch(file_._pool.getLatch());

					// LRU リストから
					// バッファページ記述子をはずす

					file_._pool.getLruList(page._lruListCategory)
						.erase(page);

					// バッファファイル内の LRU リストに存在している
					// ページリストからも削除する

					page._file._pageList.erase(page);

					// バッファメモリを破棄する

					file_._pool.freeMemory(page._body, pageSize);
				}
				
				// バッファページの状態を未確保とする

				page._status = Page::Status::Empty;
			}

			// ロックを外す
			page.getLatch().unlock();
			page.getRWLock().unlock(Os::RWLock::Mode::Read);
		}

		if (isRead == false)
		{
			_SYDNEY_RETHROW;
		}
	}

	if (isRead == true)

		// 与えられたページのみ読み込めたので、ここでリターンする
		// 他のページのエラー処理は直前の catch 節で実施済み
		
		return;	 

	// すべてのページが読み込めたので、
	// 先読み対象のページのステータスを Status::Read にし、
	// ロックを開放して、ページをデタッチする
	//
	//【注意】
	// 引数の page_ は上位関数で処理されるので、何もしない

	char* b = buf.get();
	for (int i = pagestart; i < pageend; ++i, b += pageSize)
	{
		Page* p = pages[i];

		// 内容をコピーする
		Os::Memory::copy(p->_body, b, pageSize);
		
		if (i == pagepos)
			// 引数の与えられたページの部分は飛ばす
			continue;

		// 引数以外のページは参照カウンターを減らす必要がある
		AutoPage destructor(p, true);
		Page& page = *destructor;

		// ステータスを変更
		page._status = Status::Read;

		// ロックを外す
		page.getLatch().unlock();
		page.getRWLock().unlock(Os::RWLock::Mode::Read);
	}
}
#else
void
Page::readAhead(File& file_, Page& page_, Os::File::Offset offset_)
{
	// 【注意】
	//	引数の page_ はラッチ済みであること
	
	// 【注意】
	//	本来なら、IOVMAX 個の配列を用意するべきであるが、
	//	ヒープメモリーを確保したくないので、スタック上に十分な大きさのものを
	//	確保しておく

	//	先読みは Configuration::ReadAheadBlockSize のブロック単位で行う

	// ページサイズの最小値は 4KB である
	// 本来なら、_Page::IOUnit を利用するべきであるが、スタック上に
	// 配列を用意したいので、ここはハードコーディングとする
	// よって、各アーキテクチャーの内、一番小さいものとする

	const Os::Memory::Size	pageSizeMin = (4 << 10);
	const int ARRAY_SIZE
		= Configuration::ReadAheadBlockSize::Maximum / pageSizeMin;
	
	Page*				pages[ARRAY_SIZE];
	Os::File::IOBuffer	bufs[ARRAY_SIZE];
	
	const Os::Memory::Size pageSize = page_.getSize();
	const Os::File::Size fileSize = file_.getSize();
	const int unitCount = static_cast<int>(pageSize / _Page::IOUnit);
	
	// 先読みするブロック数の上限
	unsigned int blockSize = Configuration::ReadAheadBlockSize::get();
	if (blockSize < pageSize)
		// ページサイズよりも小さかったらページサイズとする
		blockSize = pageSize;

	// ブロック全体のバッファを格納するための配列数
	const int	IOVMAX = static_cast<int>(blockSize / _Page::IOUnit);
	; _SYDNEY_ASSERT(IOVMAX <= ARRAY_SIZE);

	Os::Memory::reset(pages, sizeof(Page*) * IOVMAX);

	// 与えられた位置をブロックの境界に丸める
	const Os::File::Offset start
		= offset_ & ~(static_cast<Os::File::Offset>(blockSize - 1));

	// ブロックの境界内の与えられた位置
	const int pagepos = static_cast<int>((offset_ - start) / pageSize);
	// _Page::IOUnitの位置
	const int iovpos = static_cast<int>(pagepos * pageSize / _Page::IOUnit);
	// 与えられたページの位置
	Os::File::Offset offset = offset_;

	// 与えられたページのみ読み込めたかどうか
	bool isRead = false;
	
	int pagestart = pagepos;
	int pageend = pagepos;

	Page* p = &page_;
	pages[pageend++] = p;

	try
	{
		// まず、与えられたページの後方の先読みするページを取得する
		// ロックできなかったり、途中にすでに読み込み済みのページが
		// あったりしたらそこまでとする

		int iovend = iovpos;
	
		do
		{
			// バッファページのバッファメモリを読み込み単位に分割する
			// Windows の場合、読み込み単位はカーネルのページサイズに
			// 固定されている
			// Linux や Solaris の場合は特に固定されていないが、
			// 分割しても速度的な問題はあまりないと思うので、同じコードとする
		
			Os::Memory::Offset off = 0;
			do {
				bufs[iovend++] =
					static_cast<char*>(p->_body) + off;
			}
			while (pageSize - (off += _Page::IOUnit));

			if (iovend >= IOVMAX)
				break;

			offset += pageSize;
			if (static_cast<Os::File::Size>(offset) >= fileSize)
				// ファイルの最後まで読んだので終了
				break;

			// 次のページを得る
		
			p = Page::attach(file_, offset);

			// ロックできないページがあったり、すでに読み込んでいるページが
			// あったら、それ以降は先読みしない
		
			if (!p->getRWLock().trylock(Os::RWLock::Mode::Read))
			{
				p->detach();
				break;
			}
			if (!p->getLatch().trylock())
			{
				p->getRWLock().unlock(Os::RWLock::Mode::Read);
				p->detach();
				break;
			}
			if (p->_status > Status::NoRead)
			{
				p->getLatch().unlock();
				p->getRWLock().unlock(Os::RWLock::Mode::Read);
				p->detach();
				break;
			}

			try {
				// バッファプールを保護するためにラッチする

				Os::AutoCriticalSection latch(file_._pool.getLatch());

				// バッファメモリを確保する

				bool allocated;

				p->_body =
					file_._pool.replaceMemory(pageSize, false,
											  allocated, *p, false);
				; _SYDNEY_ASSERT(p->_body);

				// ステータスを変更
				p->_status = Status::NoRead;

			} catch (Exception::MemoryExhaust&) {

				// バッファプールの上限に達したため、
				// これ以上、バッファメモリを確保できなかった
				// あきらめる

				// エラー状況を解除する
				Common::Thread::resetErrorCondition();
			
				p->getLatch().unlock();
				p->getRWLock().unlock(Os::RWLock::Mode::Read);

				// ここで例外が発生するということは、
				// LRUにもどこにもないページであるということ。
				// なので、参照カウンタが0であったら、破棄する
				
				Page::detach(p, false);
				
				break;
				
			} catch (...) {

				// その他の例外
				
				p->getLatch().unlock();
				p->getRWLock().unlock(Os::RWLock::Mode::Read);

				// ここで例外が発生するということは、
				// LRUにもどこにもないページであるということ。
				// なので、参照カウンタが0であったら、破棄する
				
				Page::detach(p, false);
				
				_SYDNEY_RETHROW;
			}

			// 先読みするページである
			pages[pageend++] = p;
		}
		while (iovend < IOVMAX);

		// 次に、与えられたページの前方の先読みするページを取得する
		// ロックできなかったり、途中にすでに読み込み済みのページが
		// あったりしたらそこまでとする

		int iovstart = iovpos;
		offset = offset_;

		while (offset != start)
		{
			// 前のページを得る

			p = Page::attach(file_, offset - pageSize);

			// ロックできないページがあったり、すでに読み込んでいるページが
			// あったら、それ以降は先読みしない
		
			if (!p->getRWLock().trylock(Os::RWLock::Mode::Read))
			{
				p->detach();
				break;
			}
			if (!p->getLatch().trylock())
			{
				p->getRWLock().unlock(Os::RWLock::Mode::Read);
				p->detach();
				break;
			}
			if (p->_status > Status::NoRead)
			{
				p->getLatch().unlock();
				p->getRWLock().unlock(Os::RWLock::Mode::Read);
				p->detach();
				break;
			}

			try {
				// バッファプールを保護するためにラッチする

				Os::AutoCriticalSection latch(file_._pool.getLatch());

				// バッファメモリを確保する

				bool allocated;

				p->_body =
					file_._pool.replaceMemory(pageSize, false,
											  allocated, *p, false);
				; _SYDNEY_ASSERT(p->_body);

				// ステータスを変更
				p->_status = Status::NoRead;

			} catch (Exception::MemoryExhaust&) {

				// バッファプールの上限に達したため、
				// これ以上、バッファメモリを確保できなかった
				// あきらめる

				// エラー状況を解除する
				Common::Thread::resetErrorCondition();
			
				p->getLatch().unlock();
				p->getRWLock().unlock(Os::RWLock::Mode::Read);

				// ここで例外が発生するということは、
				// LRUにもどこにもないページであるということ。
				// なので、参照カウンタが0であったら、破棄する
				
				Page::detach(p, false);
				
				break;
				
			} catch (...) {

				// その他の例外
				
				p->getLatch().unlock();
				p->getRWLock().unlock(Os::RWLock::Mode::Read);

				// ここで例外が発生するということは、
				// LRUにもどこにもないページであるということ。
				// なので、参照カウンタが0であったら、破棄する
				
				Page::detach(p, false);
				
				_SYDNEY_RETHROW;
			}

			// 先読みするページである
			pages[--pagestart] = p;

			// バッファページのバッファメモリを読み込み単位に分割する
			// Windows の場合、読み込み単位はカーネルのページサイズに
			// 固定されている
			// Linux や Solaris の場合は特に固定されていないが、
			// 分割しても速度的な問題はあまりないと思うので、同じコードとする
		
			Os::Memory::Offset off = 0;
			iovstart -= unitCount;
			do {
				bufs[iovstart++] =
					static_cast<char*>(p->_body) + off;
			}
			while (pageSize - (off += _Page::IOUnit));
			
			iovstart -= unitCount;
			offset -= pageSize;
		}

		// 先読みするページを確保できたので、読み込む

		try
		{
			// バッファファイルを保護するためにラッチする

			Os::AutoCriticalSection	latch(file_.getLatch());

			// バッファファイルをオープンする

			bool nolimit = file_.openOsFile();

			// 与えられたオフセットから iovcount 分の
			// OS ファイル領域を読み出す

			file_.readOsFile(&bufs[iovstart],
							 _Page::IOUnit * (iovend - iovstart),
							 (iovend - iovstart), offset);

			if (!nolimit)

				// ファイルディスクリプタの上限に達している
				// ので、クローズする

				file_.closeOsFile();
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			// 先読みするのに失敗したので、与えられたページのみ読み込む

			try
			{
				// バッファファイルを保護するためにラッチする

				Os::AutoCriticalSection	latch(file_.getLatch());

				// バッファファイルをオープンする

				bool nolimit = file_.openOsFile();

				// 与えられたオフセットからページサイズぶんの
				// OS ファイル領域を読み出す

				Os::File::IOBuffer buf(page_._body);
				file_.readOsFile(buf, pageSize, offset_);

				if (!nolimit)

					// ファイルディスクリプタの上限に達している
					// ので、クローズする

					file_.closeOsFile();

				// 読み込めた
				
				isRead = true;
			}
			catch (...)
			{
				// やっぱり読み込めなかった
				;
			}
			
			_SYDNEY_RETHROW;
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		for (int i = pagestart; i < pageend; ++i)
		{
			if (i == pagepos)
				// 引数の与えられたページの部分は飛ばす
				continue;

			// 例外が発生した場合には、確保したバッファを返し、
			// 参照カウンタが 0 であったら、ページ記述子も破棄する
			
			AutoPage destructor(pages[i], false);
			Page& page = *destructor;

			if (page._status != Status::Empty)
			{
				{
					// バッファプールを保護するためにラッチする

					Os::AutoCriticalSection
						latch(file_._pool.getLatch());

					// LRU リストから
					// バッファページ記述子をはずす

					file_._pool.getLruList(page._lruListCategory)
						.erase(page);

					// バッファファイル内の LRU リストに存在している
					// ページリストからも削除する

					page._file._pageList.erase(page);

					// バッファメモリを破棄する

					file_._pool.freeMemory(page._body, pageSize);
				}
				
				// バッファページの状態を未確保とする

				page._status = Page::Status::Empty;
			}

			// ロックを外す
			page.getLatch().unlock();
			page.getRWLock().unlock(Os::RWLock::Mode::Read);
		}

		if (isRead == false)
		{
			_SYDNEY_RETHROW;
		}
	}

	if (isRead == true)

		// 与えられたページのみ読み込めたので、ここでリターンする
		// 他のページのエラー処理は直前の catch 節で実施済み
		
		return;	 

	// すべてのページが読み込めたので、
	// 先読み対象のページのステータスを Status::Read にし、
	// ロックを開放して、ページをデタッチする
	//
	//【注意】
	// 引数の page_ は上位関数で処理されるので、何もしない
	
	for (int i = pagestart; i < pageend; ++i)
	{
		if (i == pagepos)
			// 引数の与えられたページの部分は飛ばす
			continue;
		
		AutoPage destructor(pages[i], true);
		Page& page = *destructor;

		// ステータスを変更
		page._status = Status::Read;

		// ロックを外す
		page.getLatch().unlock();
		page.getRWLock().unlock(Os::RWLock::Mode::Read);
	}
}
#endif

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
