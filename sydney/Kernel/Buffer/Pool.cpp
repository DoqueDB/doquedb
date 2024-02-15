// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Pool.cpp -- バッファプール関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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
#include "Buffer/Configuration.h"
#include "Buffer/Deterrent.h"
#include "Buffer/Manager.h"
#include "Buffer/Page.h"
#include "Buffer/Pool.h"
#include "Buffer/Statistics.h"

#include "Common/Assert.h"
#include "Common/Thread.h"
#include "Exception/MemoryExhaust.h"
#include "Os/AutoCriticalSection.h"
#include "Os/AutoRWLock.h"
#include "Os/SysConf.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_BUFFER_USING

namespace 
{

namespace _Pool
{
	namespace _DirtyList
	{
		// ソート用の比較関数
		bool
		sortCompare(const Page* l, const Page* r);
	}

	// 入出力の最小単位(B 単位)
	const Os::Memory::Size	IOUnit = Os::SysConf::PageSize::get();

	// 以下の情報を保護するためのラッチ
	Os::CriticalSection		_latch;

	// すべてのバッファプール記述子を管理する配列
	Pool*					_poolTable[Pool::Category::Count] =	{ 0, 0, 0, 0 };
	// バッファプールのサイズの上限を管理する配列
	Os::Memory::LongSize	_poolLimit[Pool::Category::Count];
}

//	FUNCTION
//	$$$::_Pool::_DirtyList::sortCompare --
//		ダーティリスト中のバッファページ記述子をソートするために、
//		2 つのバッファページ記述子の表すバッファページの大小比較を行う
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Page*		l
//			比較するバッファページを表すバッファページ記述子
//		Buffer::Page*		r
//			比較するバッファページを表すバッファページ記述子
//
//	RETURN
//		true
//			l より r のほうが小さい
//		false
//			l より r のほうが小さくない
//
//	EXCEPTIONS
//		なし

inline
bool
_Pool::_DirtyList::sortCompare(const Page* l, const Page* r)
{
	return l->getFileID() < r->getFileID() ||
		(l->getFileID() == r->getFileID() && l->getOffset() < r->getOffset());
}

}
	
//	FUNCTION private
//	Buffer::Manager::Pool::initialize --
//		マネージャーの初期化のうち、バッファプール関連のものを行う
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
Manager::Pool::initialize()
{
	try {
		// バッファプールのサイズの上限を求める

		typedef Os::Memory::LongSize (*Func)();
		const Func get[Buffer::Pool::Category::Count] =
		{
			&Configuration::NormalPoolSize::get,
			&Configuration::TemporaryPoolSize::get,
			&Configuration::ReadOnlyPoolSize::get,
			&Configuration::LogicalLogPoolSize::get
		};

		for (unsigned int i = 0; i < Buffer::Pool::Category::Count; ++i)
			_Pool::_poolLimit[i] = get[i]();

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		terminate();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Buffer::Manager::Pool::terminate --
//		マネージャーの後処理のうち、バッファプール関連のものを行う
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
Manager::Pool::terminate()
{}

//	FUNCTION public
//	Buffer::Pool::attach -- バッファプール記述子を得る
//
//	NOTES
//		各種別に 1 つしかバッファプール記述子は生成されない
//
//	ARGUMENTS
//		Buffer::Pool::Category::Value	category
//			バッファプールの種別
//
//	RETURN
//		得られたバッファプール記述子を格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Pool*
Pool::attach(Category::Value category)
{
	; _SYDNEY_ASSERT(category < Category::Count);

	// すべてのバッファプール記述子を管理する
	// 配列を保護するためにラッチする

	Os::AutoCriticalSection	latch(_Pool::_latch);

	// 指定された種別のバッファプール記述子が存在するか調べる

	Pool* pool = _Pool::_poolTable[category];
	if (!pool) {

		// 見つからなかったので、生成し、
		// すべてのバッファプール記述子を管理する配列に登録しておく

		_Pool::_poolTable[category] = pool =
			new Pool(category, _Pool::_poolLimit[category]);
		; _SYDNEY_ASSERT(pool);
	}

	// 参照回数を増やす

	++pool->_refCount;

	return pool;
}

//	FUNCTION private
//	Buffer::Pool::attach --	バッファプール記述子の参照数を 1 増やす
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		参照数が 1 増えたバッファプール記述子を格納する領域の先頭アドレス
//
//	EXCEPTIONS

Pool*
Pool::attach()
{
	// すべてのバッファプール記述子を管理する
	// 配列を保護するためにラッチする

	Os::AutoCriticalSection	latch(_Pool::_latch);

	// 参照回数を 1 増やす

	++_refCount;

	return this;
}

//	FUNCTION public
//	Buffer::Pool::detach -- バッファプール記述子の参照をやめる
//
//	NOTES
//		バッファプール記述子の参照をやめても、
//		他のどこかで参照されていれば、バッファプール記述子は破棄されない
//		逆にどこからも参照されていなければ、
//		バッファプール記述子は直ちに破棄される
//
//	ARGUMENTS
//		Buffer::Pool*&		pool
//			参照をやめるバッファプール記述子を格納する領域の先頭アドレスで、
//			呼び出しから返ると 0 になる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Pool::detach(Pool*& pool)
{
	if (pool) {
		{
		// すべてのバッファプール記述子を管理する配列を
		// 保護するためにラッチする

		Os::AutoCriticalSection	latch(_Pool::_latch);

		// 参照数が 0 より大きければ 1 減らす

		if (pool->_refCount && --pool->_refCount)

			// 他から参照されているので、破棄できない

			pool = 0;
		else

			// どこからも参照されていないので、破棄できる
			//
			//【注意】	どこからも参照されていないので、
			//			このバッファプールを使用するバッファファイル記述子、
			//			バッファページ記述子は存在しない
			//
			//			ということは、ダーティなバッファページも存在しない

			// すべてのバッファプール記述子を管理する配列から
			// 破棄するバッファプール記述子の記録を抹消する

			_Pool::_poolTable[pool->getCategory()] = 0;

		// ここで、すべてのバッファプール記述子を管理する配列を
		// 保護するためのラッチがはずれる
		}

		if (pool)

			// どこからも参照されていないバッファプール記述子を破棄し、
			// 与えられたポインタは 0 を指すようにする

			delete pool, pool = 0;
	}
}

//	FUNCTION private
//	Buffer::Pool::detach -- バッファプール記述子の参照数を 1 減らす
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
Pool::detach()
{
	// すべてのバッファプール記述子を管理する
	// 配列を保護するためにラッチする

	Os::AutoCriticalSection	latch(_Pool::_latch);
#ifdef DEBUG
	; _SYDNEY_ASSERT(_refCount);
#else
	if (_refCount)
#endif
		// 参照数が 0 より大きければ 1 減らす

		--_refCount;
}

//	FUNCTION private
//	Buffer::Pool::getRefCount -- バッファプール記述子の参照数を得る
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
Pool::getRefCount() const
{
	// すべてのバッファプール記述子を管理する
	// 配列を保護するためにラッチする

	Os::AutoCriticalSection	latch(_Pool::_latch);

	return _refCount;
}

//	FUNCTION private
//	Buffer::Pool::Pool -- バッファプール記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Pool::Category::Value	category
//			バッファプールの種別
//		Os::Memory::Size	limit
//			バッファプールのサイズの上限(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Pool::Pool(Category::Value category, Os::Memory::LongSize limit)
	: _category(category)
	, _limit(limit)
	, _refCount(0)
	, _parameter(0)
	, _t1(LruList::Category::T1)
#ifdef USE_ARC
	, _t2(LruList::Category::T2)
	, _b1(LruList::Category::B1)
	, _b2(LruList::Category::B2)
#endif
	, _dirtyList(0)
{
	try {
		if (_category != Category::ReadOnly) {

			// ダーティリストを確保する
			//
			//【注意】	管理可能なバッファページの最大数分の領域を予約しておく

			const unsigned int n = static_cast<unsigned int>(
				getLimit() / _Pool::IOUnit);
			_dirtyList = new DirtyList(n);
			; _SYDNEY_ASSERT(_dirtyList);
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		destruct();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Buffer::Pool::destruct --
//		バッファプール記述子を表すクラスのデストラクター下位関数
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
Pool::destruct()
{
	delete _dirtyList, _dirtyList = 0;
}

//	FUNCTION private
//	Buffer::Pool::allocateMemory -- バッファメモリを確保する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Memory::Size	size
//			確保するバッファメモリのサイズ(B 単位)
//		bool				reset
//			true
//				新たなバッファメモリを確保したとき、初期化する
//			false
//				新たなバッファメモリを確保したとき、初期化しない
//		bool&				allocated
//			新たにバッファメモリが確保されたかが設定される
//		bool				force
//			true
//				バッファプールの上限に達していても、確保する
//			false または指定されないとき
//				バッファプールの上限に達したら、例外を発生する
//
//	RETURN
//		確保されたバッファメモリの領域の先頭アドレス
//
//	EXCEPTIONS
//		Exception::MemoryExhaust
//			バッファプールの上限に達したため、バッファメモリを確保できない

void*
Pool::allocateMemory(Os::Memory::Size size,
					 bool reset, bool& allocated, bool force)
{
	//【注意】	バッファプールはラッチ済であること

	if (!force && getSize() + size > getLimit() &&
		!Common::Thread::isErrorCondition())

		// バッファプールの上限に達したため、
		// これ以上、バッファメモリを確保できない
		//
		//【注意】	スレッドがエラー状態のときは
		//			上限を超えてバッファメモリを確保できる

		_SYDNEY_THROW0(Exception::MemoryExhaust);

	void* p = 0;

	try
	{
		// 仮想アドレス空間上に指定されたサイズの領域を確保する
		//
		//【注意】
		//	通常の領域確保関数でなく Os::Memory::map を使用するのは、
		//	読み書きに使用する関数がシステムの
		//	ページサイズ境界の先頭アドレスを持つ領域しか認めないため

		p = Os::Memory::map(size, reset);
	}
	catch (Exception::MemoryExhaust&)
	{
		_SYDNEY_RETHROW;
	}

	// 統計情報を記録する

	Statistics::record(Statistics::Category::Allocate, size);

	allocated = true;

	return p;
}

//	FUNCTION private
//	Buffer::Pool::freeMemory -- バッファメモリを破棄する
//
//	NOTES
//
//	ARGUMENTS
//		void*&				p
//			破棄するバッファメモリの先頭アドレス
//			呼び出しから返ると 0 になる
//		Os::Memory::Size	size
//			破棄するバッファメモリのサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Pool::freeMemory(void*& p, Os::Memory::Size size)
{
	//【注意】	バッファプールはラッチ済であること

	if (p && size) {

		// バッファメモリを破棄する

		Os::Memory::unmap(p, size);
		; _SYDNEY_ASSERT(!p);

		// 統計情報を記録する

		Statistics::record(Statistics::Category::Free, size);
	}
}

//	FUNCTION private
//	Buffer::Pool::replaceMemory -- バッファメモリを再利用する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Memory::Size	size
//			再利用するバッファメモリのサイズ(B 単位)
//		bool				reset
//			true
//				新たなバッファメモリを確保したとき、初期化する
//			false
//				新たなバッファメモリを確保したとき、初期化しない
//		bool&				allocated
//			新たにバッファメモリが確保されたかが設定される
//		Page&				page
//			バッファメモリを必要としている
//			バッファページのバッファページ記述子で、
//			ラッチで保護されていなければならない
//		bool				force
//			true
//				getCandidateを強制モードで実行する
//			false
//				getCandidateを通常モードで実行する
//
//	RETURN
//		再利用で、得られたバッファメモリの領域の先頭アドレス
//
//	EXCEPTIONS

void*
Pool::replaceMemory(Os::Memory::Size size,
					bool reset, bool& allocated, Page& page, bool force)
{
	//【注意】	バッファプールはラッチ済であること

	void* p = 0;

	switch (page._lruListCategory) {
	case LruList::Category::T1:
#ifdef USE_ARC
	case LruList::Category::T2:
	{
		// バッファメモリを必要としているバッファページは
		// LRU リスト T1 または T2 に登録されている

		p = page._body;
		; _SYDNEY_ASSERT(p);
		allocated = false;

		// T2 の MRU へ移動する

		LruList::Iterator position(_t2.end());
		_t2.splice(position, getLruList(page._lruListCategory), page);
		break;
	}
	case LruList::Category::B1:
	case LruList::Category::B2:
	{
		// バッファメモリを必要としているバッファページは
		// LRU リスト B1 または B2 に登録されている

		// 現状に合わせて ARC パラメータを調整する

		calculateParameter(page);

		// バッファプールの上限に達したため、
		// 置換候補のバッファメモリを得る

		p = getCandidate(size, reset, allocated, page);
		if (!p) {

			// 新たにバッファメモリを確保する
			//
			//【注意】	たとえ、バッファプールの上限に達していても、
			//			エラー処理中は上限を超えて確保可能なので、確保を試みる

			p = allocateMemory(size, reset, allocated);
			; _SYDNEY_ASSERT(p);
		}

		// LRU リスト T2 の MRU へ移動する

		LruList::Iterator position(_t2.end());
		_t2.splice(position, getLruList(page._lruListCategory), page);
#else
	{
		// バッファメモリを必要としているバッファページは
		// LRU リストに登録されている

		p = page._body;
		; _SYDNEY_ASSERT(p);
		allocated = false;

		// LRU リストの MRU へ移動する

		LruList::Iterator position(_t1.end());
		_t1.splice(position, _t1, page);
#endif
		break;
	}
	default:
#ifdef USE_ARC
		// バッファメモリを必要としているバッファページは
		// どの LRU リストにも登録されていない

		// バッファメモリを確保すると T1 と T2 のサイズの和が
		// バッファプールの上限を超えるようであれば、
		// 置換候補を T1 または T2 から見つけ T1 に登録する
		//
		// このとき、置換候補は B1 または B2 に登録されるので
		// L1 のサイズは必ず size 増え、L2 のサイズは変化しない

		Os::Memory::LongSize total = _t1._total + _b1._total + size;
		Os::Memory::LongSize limit = getLimit();

		if (total > limit) {

			// バッファメモリを確保すると L1 のサイズが
			// バッファプールの上限を超えてしまう

			const Os::Memory::LongSize shortage = total - limit;
			if (shortage <= _b1._total) {

				// B1 から超えるぶんを削除し、
				// T1 または T2 から置換候補のバッファメモリを得る

				(void) releaseVictim(shortage, _b1);
				p = getCandidate(size, reset, allocated, page);
			} else

				// T1 から置換候補を得るが、例外的に置換候補を B1 に登録しない

				p = getCandidate(size, reset, allocated, _t1, _t1);

		} else if ((total += _t2._total + _b2._total) > limit) {

			// バッファメモリを確保すると L1 と L2 のサイズの和が
			// バッファプールの上限を超えてしまう

			if (total > (limit <<= 1)) {

				// バッファメモリを確保すると L1 と L2 のサイズの和が
				// バッファプールの上限を 2 倍したものを超えてしまうので、
				// B2 から超えるぶんを削除する

				const Os::Memory::LongSize shortage = total - limit;
				; _SYDNEY_ASSERT(_b2._total >= shortage);
				(void) releaseVictim(shortage, _b2);
			}

			// T1 または T2 から置換候補のバッファメモリを得る

			p = getCandidate(size, reset, allocated, page);
		}
#else
		// バッファメモリを必要としているバッファページは
		// LRU リストに登録されていないので、LRU リストから置換候補を得る

		p = getCandidate(size, reset, allocated, force);
#endif
		if (!p) {

			// 新たにバッファメモリを確保する

			p = allocateMemory(size, reset, allocated);
			; _SYDNEY_ASSERT(p);
		}

		// LRU リスト T1 の MRU に登録する

		_t1.pushBack(page);

		// バッファファイルの LRU リストにあるリストにも登録する
		
		page._file._pageList.pushBack(page);
	}

	return p;
}

#ifdef USE_ARC
//	FUNCTION private
//	Buffer::Pool::calculateParameter -- ARC パラメータを計算しなおす
//
//	NOTES
//
//	ARGUMENTS
//		Page&				page
//			バッファメモリを必要としている
//			バッファページのバッファページ記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Pool::calculateParameter(const Page& page)
{
	//【注意】	バッファプールはラッチ済であること

	if (page._lruListCategory == LruList::Category::B2) {
		const unsigned int delta =
			(_b2._total < _b1._total) ? _b1._total / _b2._total : 1;
		const Os::Memory::Size tmp =
			ModMax<Os::Memory::Size>(_Pool::IOUnit * delta, page.getSize());
		_parameter = (_parameter > tmp) ? _parameter - tmp : 0;
	} else {
		const Os::Memory::Size delta =
			(_b1._total < _b2._total) ? _b2._total / _b1._total : 1;
		const Os::Memory::Size tmp =
			ModMax<Os::Memory::Size>(_Pool::IOUnit * delta, page.getSize());
		_parameter = ModMin<Os::Memory::Size>(_parameter + tmp, getLimit());
	}
}

//	FUNCTION private
//	Buffer::Pool::getCandidate -- 確保済のバッファメモリから置換の候補を得る
//
//	NOTES
//
//	ARGUMENTS
//		Os::Memory::Size	size
//			取得するバッファメモリのサイズ(B 単位)
//		bool				reset
//			true
//				新たなバッファメモリを確保したとき、初期化する
//			false
//				新たなバッファメモリを確保したとき、初期化しない
//		bool&				allocated
//			新たにバッファメモリが確保されたかが設定される
//		Page&				page
//			バッファメモリを必要としている
//			バッファページのバッファページ記述子
//
//	RETURN
//		0 以外の値
//			置換候補のバッファメモリとして得られた領域の先頭アドレス
//		0
//			置換候補のバッファメモリとする領域が得られなかった
//
//	EXCEPTIONS

void*
Pool::getCandidate(Os::Memory::Size size,
				   bool reset, bool& allocated, const Page& page)
{
	//【注意】	バッファプールはラッチ済であること

	unsigned int rest = Configuration::SkipDirtyCandidateCountMax::get();
	void* p;
	return (_t1._total &&
			(_t1._total > _parameter ||
			 (_t1._total == _parameter &&
			  page._lruListCategory == LruList::Category::B2))) ?
		((p = getCandidate(size, reset, allocated, _t1, _b1, rest)) ? p :
		 ((getSize() + size > getLimit()) ?
		  getCandidate(size, reset, allocated, _t2, _b2, rest) : 0)) :
		((p = getCandidate(size, reset, allocated, _t2, _b2, rest)) ? p :
		 ((getSize() + size > getLimit()) ?
		  getCandidate(size, reset, allocated, _t1, _b1, rest) : 0));
}
#endif

//	FUNCTION private
//	Buffer::Pool::getCandidate -- 確保済のバッファメモリから置換の候補を得る
//
//	NOTES
//
//	ARGUMENTS
//		Os::Memory::Size	size
//			取得するバッファメモリのサイズ(B 単位)
//		bool				reset
//			true
//				新たなバッファメモリを確保したとき、初期化する
//			false
//				新たなバッファメモリを確保したとき、初期化しない
//		bool&				allocated
//			新たにバッファメモリが確保されたかが設定される
#ifdef USE_ARC
//		Buffer::Pool::LruList&	t
//			置換候補のバッファメモリを探す LRU リスト(T1 または T2)
//		Buffer::Pool::LruList&	b
//			置換候補のバッファメモリを保持する
//			バッファページ記述子を移動する LRU リスト(B1 または B2)
#endif
//		unsigned int&		rest
//			指定されたとき
//				置換候補を探すとき、ダーティなバッファページを何個無視できるか
//			指定されないとき
//				Buffer_SkipDirtyCandidateCountMax が指定されたものとみなす
//
//		bool				force
//			true
//				上限に達していなくてもLRUリストから候補を探す
//				また、restが指定されていても無視する
//			false
//				上限に達していなければLRUリストは探索しない
//
//	RETURN
//		0 以外の値
//			置換候補のバッファメモリとして得られた領域の先頭アドレス
//		0
//			置換候補のバッファメモリとする領域が確保できなかった
//
//	EXCEPTIONS

void*
Pool::getCandidate(
	Os::Memory::Size size, bool reset, bool& allocated
#ifdef USE_ARC
	, LruList& t, LruList& b
#endif
	, bool force
	)
{
	unsigned int rest = Configuration::SkipDirtyCandidateCountMax::get();
	return getCandidate(size, reset, allocated,
#ifdef USE_ARC
						t, b,
#endif
						rest, force);
}

void*
Pool::getCandidate(
	Os::Memory::Size size, bool reset, bool& allocated
#ifdef USE_ARC
	, LruList& t, LruList& b
#endif
	, unsigned int& rest, bool force)
{
	//【注意】	バッファプールはラッチ済であること

	allocated = false;

	if (getSize() + size > getLimit() || force) {

		//【注意】	もともとの ARC の論文では
		//			バッファプールの上限に達したかのチェックは行わない
#ifdef USE_ARC
		const bool saving = t._category != b._category;
#else
		LruList& t = _t1;
#endif
		// バッファプールの上限に達したため、置換候補のバッファメモリを得る

		Os::Memory::Size freed = 0;

		// LRU リスト T の LRU から MRU に向かって、
		// 置換候補のバッファページを探す

		LruList::Iterator			ite(t.begin());
		const LruList::Iterator&	end = t.end();

		while (ite != end) {

			// 処理するバッファページの参照数を増やしておく
			//
			//【注意】	回数が多いので AutoPage による
			//			間接参照は可能な限り行わない

			Page& page = *ite;
			AutoPage destructor(page.attach(), true);
			++ite;

			void* p = 0;
			{
			// 今調べているバッファページを保護するためにラッチを試みる

			Os::AutoTryCriticalSection latch(page.getLatch());

			if (!latch.isLocked() || page.getRefCount() > 1)

				// ラッチできなかったか、
				// 自分以外から参照されているので、次を調べる

				continue;

			if (page._status & Page::Status::Flushable) {

				// このバッファページはダーティリストに載っている

				if (force)

					// 無条件に次を調べる
					
					continue;

				if (!rest)

					// もう与えられた個数のダーティな
					// バッファページを無視したので、あきらめる

					break;

				// 次を調べる

				--rest;
				continue;
			}

			// このバッファページの状態を未確保にする

			ModSwap(p, page._body);

			page._status = Page::Status::Empty;

			// ここでバッファページを保護するためのラッチがはずれる
			}
#ifdef USE_ARC
			if (saving) {

				// このバッファページを LRU リスト T から B の MRU へ移動する

				LruList::Iterator position(b.end());
				b.splice(position, t, page);
			} else {
#endif
				// このバッファページを LRU リスト T からはずす

				t.erase(page);

				// バッファファイル内の LRU リストに存在している
				// ページリストからも削除する

				page._file._pageList.erase(page);

#ifdef USE_ARC
			}
#endif

			// このバッファページのページサイズをおぼえておく

			const Os::Memory::Size pageSize = page.getSize();

			// このバッファページ記述子の参照数を減らし、
			// どこからも参照されていなければ、破棄する

			destructor.free(
#ifdef USE_ARC
				saving
#else
				false
#endif
				);

			// 統計情報を記録する
			Statistics::record(Statistics::Category::Replace, pageSize);

			if (pageSize == size)

				// このバッファページのバッファメモリを返す

				return p;

			// このバッファページのバッファメモリを解放する

			freeMemory(p, pageSize);

			if ((freed += pageSize) >= size)
			{
				try
				{
					// これまでに解放したバッファメモリの総サイズが
					// 必要としているサイズ以上になったら、
					// 必要としているサイズの領域を確保し、返す

					return allocateMemory(size, reset, allocated);
				}
				catch (Exception::MemoryExhaust&)
				{
					// エラー状況を解除する
					Common::Thread::resetErrorCondition();
				
					// まだ足りないので、もう一度候補を探す
					
					freed = 0;
				}
			}
		}
	}

	// 置換候補のバッファメモリを得られなかった

	return 0;
}

#ifdef USE_ARC
//	FUNCTION private
//	Buffer::Pool::releaseVictim --
//		バッファメモリが未確保のバッファページをあるサイズぶん破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Memory::Size	size
//			破棄するバッファページの総サイズ(B 単位)
//		Buffer::Pool::LruList&	b
//			破棄するバッファページ記述子を探す LRU リスト(B1 または B2)
//
//	RETURN
//		true
//			指定されたサイズぶん破棄できた
//		false
//			指定されたサイズぶん破棄できなかった
//
//	EXCEPTIONS

bool
Pool::releaseVictim(Os::Memory::LongSize size, LruList& b)
{
	//【注意】	バッファプールはラッチ済であること

	Os::Memory::LongSize freed = 0;

	// LRU リスト B の LRU から MRU に向かって、
	// 指定されたサイズのバッファページを破棄していく

	while (b.getSize()) {

		// 処理するバッファページの参照数を増やしておく
		//
		//【注意】	回数が多いので AutoPage による間接参照は可能な限り行わない

		Page& page = b.getFront();
		AutoPage destructor(page.attach(), false);

		// このバッファページを LRU リスト B からはずす

		b.popFront();

		if ((freed += page.getSize()) >= size)

			// これまでに破棄したバッファページの総サイズが
			// 指定されたサイズ以上になった

			return true;
	}

	// 指定されたサイズぶんのバッファページを破棄できなかった

	return false;
}
#endif

//	FUNCTION private
//	Buffer::Pool::shrink -- バッファプールのサイズを減らす
//
//	NOTES
//
//	ARGUMENTS
//		Os::Memory::LongSize	upper
//			このサイズ(B 単位)以下にバッファプールのサイズを減らす
//
//	RETURN
//		true
//			指定されたサイズ以下に減らすことができた
//		false
//			指定されたサイズ以下に減らすことができなかった
//
//	EXCEPTIONS

bool
Pool::shrink(Os::Memory::LongSize upper)
{
#ifdef USE_ARC
	if (getSize() <= upper)

		// そもそもバッファメモリは
		// 指定されたサイズより多く確保されていないので、なにもしない

		return true;

	//【注意】	オーバフローする可能性があるので、
	//			概算になるが以下の式を使う

	const bool shrinked1 =
		shrink((_parameter) ? upper / (getLimit() / _parameter) : 0, _t1, _b1);
	const bool shrinked2 =
		shrink((getLimit() - _parameter) ?
			   upper / (getLimit() / (getLimit() - _parameter)) : 0, _t2, _b2);

	return shrinked1 && shrinked2;
}

//	FUNCTION private
//	Buffer::Pool::shrink -- LRU リストに登録されたバッファメモリのサイズを減らす
//
//	NOTES
//
//	ARGUMENTS
//		Os::Memory::LongSize	upper
//			このサイズ(B単位)以下にLRUリストに
//			登録されたバッファメモリのサイズを減らす
//
//	RETURN
//		true
//			指定されたサイズ以下に減らすことができた
//		false
//			指定されたサイズ以下に減らすことができなかった
//
//	EXCEPTIONS

bool
Pool::shrink(Os::Memory::LongSize upper, LruList& t, LruList& b)
{
#else
	LruList& t = _t1;
#endif
	if (upper < t._total) {

		// LRU リスト T の LRU から MRU に向かって、
		// 破棄するバッファページを探す

		bool yet;

		LruList::Iterator			ite(t.begin());
		const LruList::Iterator&	end = t.end();

		do {
			Page& page = *ite;
			AutoPage destructor(page.attach(), true);
			++ite;

			void* p = 0;
			{
			// 今調べているバッファページを保護するためにラッチを試みる

			Os::AutoTryCriticalSection latch(page.getLatch());

			if (!latch.isLocked() || page.getRefCount() > 1 ||
				page._status & Page::Status::Flushable)

				// ラッチできなかったか、
				// このバッファページはダーティリストに載っているか、
				// 自分以外から参照されているので、あきらめる

				continue;

			// このバッファページの状態を未確保にする

			ModSwap(p, page._body);

			page._status = Page::Status::Empty;
			}
#ifdef USE_ARC
			// このバッファページを LRU リスト T から B の MRU へ移動する

			LruList::Iterator position(b.end());
			b.splice(position, t, page);
#else
			// このバッファページを LRU リストからはずす

			t.erase(page);
			
			// バッファファイル内の LRU リストに存在している
			// ページリストからも削除する

			page._file._pageList.erase(page);
#endif
			// このバッファページのページサイズをおぼえておく

			const Os::Memory::Size pageSize = page.getSize();

			// このバッファページ記述子の参照数を減らし、
			// どこからも参照されていなければ、破棄する

			destructor.free(
#ifdef USE_ARC
				true
#else
				false
#endif
				);

			// 統計情報を記録する
			Statistics::record(Statistics::Category::Replace, pageSize);

			// このバッファページのバッファメモリを解放する

			freeMemory(p, pageSize);

		} while (yet = (upper < t._total) && ite != end) ;

		return !yet;
	}

	return true;
}

//	FUNCTION public
//	Buffer::Pool::discardPage --
//		あるバッファファイルのあるバッファページ記述子を破棄する
//
//	NOTES
//		破棄対象のバッファページ記述子の表すバッファページが
//		この関数を実行中に他スレッドからフィックスされている、
//		されたときの動作は保証できない
//
//		この関数の呼び出し側で Pool::getRWLock() に読み取りロックすることで、
//		ダーティリストの入れ替えを自他共にできないようにすること
//
//	ARGUMENTS
//		Buffer::Pool::DiscardablePageFilter::Base&	filter
//			破棄すべきバッファページか
//			判定するために使用するフィルターを表す純粋仮想クラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Pool::discardPage(const DiscardablePageFilter::Base& filter, File* pFile_)
{
	//【注意】	バッファプールとバッファページのラッチ間のデッドロックを防ぎ、
	//			バッファプールをラッチしている時間を短くするため、
	//			まず、破棄対象のバッファページをすべて探した後で、
	//			バッファプールのラッチをはずし、
	//			ひとつひとつ、バッファページを破棄していく

	LruList	t(LruList::Category::T1);
#ifdef USE_ARC
	LruList	b(LruList::Category::B1);
#endif
	{
	// バッファプールを保護するためにラッチする

	Os::AutoCriticalSection	latch(getLatch());

	if (pFile_)
	{
		// バッファファイルが渡されているので、
		// バッファファイル内のリストを利用する

		LruList&	src = _t1;
		LruList&	dst = t;

		Common::DoubleLinkedList<Page>::Iterator
			ite(pFile_->_pageList.begin());
		const Common::DoubleLinkedList<Page>::Iterator& end
			= pFile_->_pageList.end();

		while (ite != end) {

			// 処理するバッファページの参照数を増やしておく

			Page& page = *ite;
			AutoPage destructor(page.attach(), true);
			++ite;

			if (filter(page)) {
				
				// このバッファページ記述子を LRU リストから外し、
				// あとで一括で破棄するためにおぼえておく
				
				LruList::Iterator position(dst.end());
				dst.splice(position, src, page);

				// バッファファイル内のリストからも削除する
				
				pFile_->_pageList.erase(page);
			}
		}
	}
	else
	{
#ifdef USE_ARC
		for (unsigned int i = 0; i < LruList::Category::Count; ++i) {
			LruList&	src
				= getLruList(static_cast<LruList::Category::Value>(i));
			LruList&	dst = (i < LruList::Category::B1) ? t : b;
#else
			LruList&	src = _t1;
			LruList&	dst = t;
#endif
			LruList::Iterator			ite(src.begin());
			const LruList::Iterator&	end = src.end();

			while (ite != end) {

				// 処理するバッファページの参照数を増やしておく
				//
				//【注意】	回数が多いので AutoPage による
				//			間接参照は可能な限り行わない

				Page& page = *ite;
				AutoPage destructor(page.attach(), true);
				++ite;

				if (filter(page)) {

					// このバッファページ記述子を LRU リストから外し、
					// あとで一括で破棄するためにおぼえておく
				
					LruList::Iterator position(dst.end());
					dst.splice(position, src, page);

					// バッファファイル内のリストからも削除する

					page._file._pageList.erase(page);
				}
			}
#ifdef USE_ARC
		}
#endif
	}
	if (_dirtyList && _dirtyList->getSize()) {
		DirtyList::Iterator			ite(_dirtyList->begin());
		const DirtyList::Iterator&	end = _dirtyList->end();

		do {
			if (*ite && filter(*(*ite)))

				// このバッファページ記述子をダーティリストからはずす
				//
				//【注意】	ダーティなバッファページは
				//			フラッシュされないまま破棄される

				*ite = 0;

		} while (++ite < end) ;
	}

	// ここで、バッファプールを保護するためのラッチがはずれる
	//
	//【注意】	これ以降、破棄対象のバッファページ記述子の表すバッファページが
	//			他のスレッドからフィックスされている、
	//			されたときの動作を保証できなくなる
	}

	// 破棄するバッファページのうち、
	// LRU リスト T1 または T2 に登録されていたものを処理する

	while (t.getSize()) {

		// 処理するバッファページの参照数を増やしておく
		//
		//【注意】	回数が多いので AutoPage による間接参照は可能な限り行わない

		Page& page = t.getFront();
		AutoPage destructor(page.attach(), false);

		//【注意】他スレッドからフィックスされていると、動作を保障できない

		; _SYDNEY_ASSERT(page.getRefCount() == 1);

		void* p = 0;
		{
		// 破棄するバッファページを保護するためにラッチする

		Os::AutoCriticalSection latch(page.getLatch());

		// このバッファページの状態を未確保にする

		ModSwap(p, page._body);

		page._status = Page::Status::Empty;

		// ここでバッファページを保護するためのラッチがはずれる
		}
		// このバッファページを LRU リスト T からはずす

		t.popFront();

		// バッファプールを保護するためにラッチする

		Os::AutoCriticalSection latch(getLatch());

		// バッファメモリが確保されていれば、破棄する

		freeMemory(p, page.getSize());
	}
#ifdef USE_ARC
	// 破棄するバッファページのうち、
	// LRU リスト B1 または B2 に登録されていたものは、
	// バッファメモリは確保されていないはずなので、
	// たんにバッファページ記述子を破棄する

	while (b.getSize()) {
		AutoPage destructor(b.getFront().attach(), false);
		b.popFront();
	}
#endif
}

//	FUNCTION public
//	Buffer::Pool::flushDirtyPage --
//		ダーティリストに登録されているダーティなバッファページのうち、
//		条件を満たすものをフラッシュする
//
//	NOTES
//		この関数の呼び出し終了時に、
//		完全にフラッシュされていることを保障するために、
//		この関数の呼び出し側で Pool::getRWLock() に書き込みロックすることで、
//		ダーティリストの入れ替えを、自分はできて、他はできないようにすること
//
//	ARGUMENTS
//		Buffer::Pool::FlushablePageFilter::Base&	filter
//			フラッシュすべきバッファページか
//			判定するために使用するフィルターを表す純粋仮想クラス
//		bool				force
//			true
//				ラッチが取れるまで待って、
//				必ず、フラッシュすべきバッファページをすべて処理するか
//			false
//				フラッシュすべきバッファページのうち、
//				すぐにラッチできるもののみ処理する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Pool::flushDirtyPage(const FlushablePageFilter::Base& filter, bool force)
{
	if (!_dirtyList) {

		// ダーティリストが存在しなければ、これ以上処理する必要はない

		; _SYDNEY_ASSERT(getCategory() == Category::ReadOnly);
		return;
	}

	// フラッシュ中に
	// バッファページのフラッシュの抑止が開始されたり、
	// 終了されたりしないように、読み取りロックする

	Os::AutoRWLock	rwlock(Deterrent::getRWLock(), Os::RWLock::Mode::Read);

	ModAutoPointer<DirtyList> destructor_dirtyList;
	{
	// バッファプールを保護するためにラッチする

	Os::AutoCriticalSection latch(getLatch());

	if (_dirtyList->isEmpty())

		// ダーティリストは空なので、なにもしない

		return;

	// 代わりのダーティリストを確保する

	destructor_dirtyList = new DirtyList(static_cast<unsigned int>(
											 getLimit() / _Pool::IOUnit));
	; _SYDNEY_ASSERT(destructor_dirtyList.get());

	DirtyList* tmp = destructor_dirtyList.release();
	destructor_dirtyList = _dirtyList;
	_dirtyList = tmp;
	}
	//【注意】	参照回数が多いので ModAutoPointer<DirtyList> で間接参照しない

	DirtyList& dirtyList = *destructor_dirtyList.get();

	// フラッシュする/しない
	// バッファページを記憶するためのリストを確保する

	DirtyList candidate;
	DirtyList rest;

	try {
		const unsigned int n = dirtyList.getSize();
		; _SYDNEY_ASSERT(n);

		candidate.reserve(n);
		rest.reserve(n);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		// フラッシュしようとしていたダーティリストを
		// 現在のダーティリストにつなげる
		//
		//【注意】	すでに現在のダーティリストは更新されているかもしれないので、
		//			フラッシュしようとしていたダーティリストを
		//			たんに元に戻してはいけない

		Os::AutoCriticalSection	latch(getLatch());

		_dirtyList->insert(_dirtyList->end(), dirtyList);
		_SYDNEY_RETHROW;
	}

	// フラッシュすべきバッファページを得る

	DirtyList::Iterator			ite(dirtyList.begin());
	const DirtyList::Iterator&	end = dirtyList.end();

	try {
		do {
			if (*ite) {
				Page& page = *(*ite);

				// バッファページを保護するためにラッチする

				Os::AutoCriticalSection	latch(page.getLatch());

				if (page._status & Page::Status::Dirty)

					// バッファページを指定された条件を満たし、
					// フラッシュすべきかで分別する

					(filter(page) ? candidate : rest).pushBack(&page);
				else

					// ダーティリストからはずす

					page._status &= ~Page::Status::Flushable;
			}
		} while (++ite != end) ;

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		// フラッシュしようとしていたバッファページのうち、
		// ダーティでなかったもの以外を現在のダーティリストに戻す

		Os::AutoCriticalSection latch(getLatch());

		_dirtyList->insert(_dirtyList->end(), ite, end);
		_dirtyList->insert(_dirtyList->end(), rest);
		_dirtyList->insert(_dirtyList->end(), candidate);
		_SYDNEY_RETHROW;
	}

	if (candidate.getSize()) {

		// 得られたフラッシュすべきバッファページを
		// それぞれのバッファファイルごとに、
		// バッファリングしている OS ファイル領域の
		// ファイルの先頭からのオフセットの昇順でソートする

		ModSort(candidate.begin(),
				candidate.end(), _Pool::_DirtyList::sortCompare);

		// フラッシュすべきバッファページを
		// ひとつひとつ調べて実際にフラッシュしていく

		DirtyList::Iterator			ite(candidate.begin());
		const DirtyList::Iterator&	end = candidate.end();
		DirtyList::Iterator			start(ite);

		unsigned int		n = 0;
		const unsigned int	max = ModMin<unsigned int>(
			Configuration::FlushingBodyCountMax::get(),
			static_cast<unsigned int>(getLimit() / _Pool::IOUnit));

		try {
			// ファイルへの書き込み単位を表すクラスの配列を確保する

			Os::File::IOBuffer* bodies = new Os::File::IOBuffer[max + 1];
			; _SYDNEY_ASSERT(bodies);

			try {
				do {
					Page& page = *(*ite);
					const Os::Memory::Size pageSize = page.getSize();

					if (n && (n + pageSize / _Pool::IOUnit > max ||
							  (*start)->getFileID() < page.getFileID() ||
							  (*start)->getOffset() +
							  _Pool::IOUnit * n < page.getOffset())) {

						// 書き込もうとしている連続したバッファページの
						// 個数が上限に達したか、このバッファページは
						// 書き込もうとしている連続したバッファページと
						// 隣接していないので、一括して書き込む

						writeBodies(bodies, n, start, ite);
						; _SYDNEY_ASSERT(!n);
						; _SYDNEY_ASSERT(start == ite);
					}

					if (force)
					{
						// バッファメモリ操作の排他制御のために、
						// 読み取りロックする

						page.getRWLock().lock(Os::RWLock::Mode::Read);

						// このバッファページを保護するためにラッチする
						
						page.getLatch().lock();
					}
					else
					{
						// バッファメモリ操作の排他制御のために、
						// 読み取りロックを試みる

						if (!page.getRWLock().trylock(Os::RWLock::Mode::Read)) {

							// 読み取りロックできなかったので、
							// 書き込もうとしているバッファページがあれば、
							// 書き込む

							writeBodies(bodies, n, start, ite);
							; _SYDNEY_ASSERT(!n);
							; _SYDNEY_ASSERT(start == ite);


							// このバッファページのフラッシュはあきらめ、
							// 後で現在のダーティリストへ戻す

							rest.pushBack(*start++);
							continue;
						}

						// このバッファページを保護するためにラッチを試みる

						if (!page.getLatch().trylock()) {

							// ラッチできなかったので、読み取りロックをはずし、
							// 書き込もうとしているバッファページがあれば、
							// 書き込む

							page.getRWLock().unlock(Os::RWLock::Mode::Read);

							writeBodies(bodies, n, start, ite);
							; _SYDNEY_ASSERT(!n);
							; _SYDNEY_ASSERT(start == ite);

							rest.pushBack(*start++);
							continue;
						}
					}

					if (page._status & Page::Status::Dirty) {

						// 書き込んだものと読み出したものが
						// 同じであることを検証するための CRC を計算する

						page.calculateCRC();

						// このバッファページのバッファメモリを
						// 書き込み単位に分割し、配列に記憶しておく

						Os::Memory::Offset offset = 0;
						do {
							bodies[n++] =
								static_cast<char*>(page._body) + offset;
						} while (pageSize - (offset += _Pool::IOUnit)) ;
					} else {

						// このバッファページはダーティリストに載っているが、
						// 今はダーティでなくなっているので、
						// リストからはずし、読み取りロックとラッチをはずす

						page._status &= ~Page::Status::Flushable;

						page.getLatch().unlock();
						page.getRWLock().unlock(Os::RWLock::Mode::Read);

						// 書き込もうとしているバッファページがあれば、書き込む

						writeBodies(bodies, n, start, ite);
						; _SYDNEY_ASSERT(!n);
						; _SYDNEY_ASSERT(start == ite);

						// 調べていたバッファページは書き込まない

						++start;
					}
				} while (++ite != end) ;

				// 書き残しているバッファページがあれば、一括して書き込む

				writeBodies(bodies, n, start, ite);
				; _SYDNEY_ASSERT(!n);
				; _SYDNEY_ASSERT(start == ite);

			}
#ifdef NO_CATCH_ALL
			catch (Exception::Object&)
#else
			catch (...)
#endif
			{

				delete [] bodies;
				_SYDNEY_RETHROW;
			}

			// ファイルへの書き込み単位を表すクラスの配列を破棄する

			delete [] bodies;

		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{

			// 書き込もうとしていた
			// バッファページのラッチと読み取りロックをはずす

			for (DirtyList::Iterator tmp = start; tmp != ite; ++tmp) {
				Page& page = *(*tmp);
				page.getLatch().unlock();
				page.getRWLock().unlock(Os::RWLock::Mode::Read);
			}

			// フラッシュする必要のない、フラッシュできなかった
			// バッファページは現在のダーティリストに戻す

			Os::AutoCriticalSection latch(getLatch());

			_dirtyList->insert(_dirtyList->end(), start, end);
			_dirtyList->insert(_dirtyList->end(), rest);
			_SYDNEY_RETHROW;
		}
	}

	// フラッシュする必要のない、フラッシュできなかった
	// バッファページは現在のダーティリストに戻す

	Os::AutoCriticalSection	latch(getLatch());

	_dirtyList->insert(_dirtyList->end(), rest);
}

//	FUNCTION private
//	Buffer::Pool::writeBodies -- 連続したバッファメモリを一括して書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Os::File::IOBuffer*	bodies
//			ある個数の連続したバッファメモリを書き込むときの
//			書き込み単位を表すクラスの配列のそれぞれを
//		unsigned int&		n
//			なん個の連続したバッファメモリを書き込むかを表し、
//			書き込み後は 0 に設定される
//		Buffer::Pool::DirtyList::Iterator&	ite
//			書き込む連続したバッファメモリの先頭のものを保持する
//			バッファページのバッファページ記述子を指すダーティリストの反復子で、
//			呼出し後は end に設定される
//		Buffer::Pool::DirtyList::Iterator&	end
//			書き込む連続したバッファメモリの末尾の直後のものを保持する
//			バッファページのバッファページ記述子を指すダーティリストの反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Pool::writeBodies(Os::File::IOBuffer* bodies, unsigned int& n,
				  DirtyList::Iterator& ite, const DirtyList::Iterator& end)
{
	; _SYDNEY_ASSERT(bodies);
	; _SYDNEY_ASSERT(getCategory() != Category::ReadOnly);

	if (n) {
		; _SYDNEY_ASSERT(n >= static_cast<unsigned int>(end - ite));

		File& file = (*ite)->_file;
		{
		// バッファファイルを保護するためにラッチする

		Os::AutoCriticalSection	latch(file.getLatch());

		// バッファファイルをオープンする

		bool nolimit = file.openOsFile();

		// 記憶しているバッファメモリを一括して書き込む

		bodies[n] = 0;
		file.writeOsFile(bodies, _Pool::IOUnit * n, n, (*ite)->getOffset());
		n = 0;

		if (!nolimit)
			
			// ファイルディスクリプターの上限に達しているのでクローズする
			
			file.closeOsFile();
		}
		// 書き込み終えたバッファページごとに処理する

		do {
			Page& page = *(*ite);

			// バッファメモリ操作の排他制御用の読み取りロックをはずす

			page.getRWLock().unlock(Os::RWLock::Mode::Read);

			// バッファページの状態を変更する
			//
			//【注意】	同時にダーティリストからはずれ、
			//			マークやフラッシュの抑止は消える

			page._status = Page::Status::Normal;

			// バッファページを保護するためのラッチをはずす

			page.getLatch().unlock();

		} while (++ite != end) ;
	}
}

//	FUNCTION public
//	Buffer::Pool::markDirtyPage --
//		ダーティリスト上のダーティなバッファページをすべてマークする
//
//	NOTES
//		チェックポイントスレッドが実行する
//		チェックポイント処理中のみ実行可能な関数
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			ひとつ以上のバッファページがマークされた
//		false
//			バッファページはひとつもマークされなかった
//
//	EXCEPTIONS

bool
Pool::markDirtyPage()
{
	if (!_dirtyList) {

		// ダーティリストが存在しなければ、これ以上処理する必要はない

		; _SYDNEY_ASSERT(getCategory() == Category::ReadOnly);
		return false;
	}

	//【注意】	チェックポイント処理中に呼び出され、
	//			他スレッドがバッファモジュールを
	//			操作していないことが保障されているので、
	//			スレッド間排他制御を行わない

	if (_dirtyList->isEmpty())

		// ダーティリストは空なので、なにもしない

		return false;

	bool marked = false;

	// ダーティリスト上のバッファページをひとつひとつ調べていく

	DirtyList::Iterator			ite(_dirtyList->begin());
	const DirtyList::Iterator&	end = _dirtyList->end();

	do {
		if (*ite) {
			Page& page = *(*ite);

			if (page._status & Page::Status::Dirty) {

				// バッファページがダーティであれば、マークする

				page._status |= Page::Status::Marked;
				marked = true;
			}
		}
	} while (++ite != end) ;

	return marked;
}

//
//	FUNCTION public
//	Buffer::Pool::syncFile -- すべてのファイルをsyncする
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
Pool::syncFile()
{
	File::syncAllFiles();
}

//	FUNCTION private
//	Buffer::Pool::getLruList --
//		指定された種別の LRU リストへのリファレンスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Pool::LruList::Ctaegory::Value	category
//			リファレンスを取得する LRU リストの種別を表す値
//
//	RETURN
//		得られた LRU リストのリファレンス
//
//	EXCEPTIONS
//		なし

Pool::LruList&
Pool::getLruList(LruList::Category::Value category)
{
	; _SYDNEY_ASSERT(category < LruList::Category::Count);
#ifdef USE_ARC
	LruList Pool::* const lp[] =
	{ &Pool::_t1, &Pool::_t2, &Pool::_b1, &Pool::_b2 };

	return this->*lp[category];
#else
	return _t1;
#endif
}

//	FUNCTION public
//	Buffer::Pool::FlushablePageFilter::ForPool::operator () -- () 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Page&		page
//			フラッシュすべきか判定するバッファページのバッファページ記述子
//
//	RETURN
//		true
//			フラッシュすべき
//		false
//			フラッシュすべきでない
//
//	EXCEPTIONS

bool
Pool::FlushablePageFilter::ForPool::operator ()(const Page& page) const
{
	return !((_onlyMarked && !(page._status & Page::Status::Marked)) ||
			 (!_noDeterred && page._file.isDeterred() &&
			  page._status & Page::Status::Deterrentable));
}

//	FUNCTION public
//	Buffer::Pool::FlushablePageFilter::ForFile::operator () -- () 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Page&		page
//			フラッシュすべきか判定するバッファページのバッファページ記述子
//
//	RETURN
//		true
//			フラッシュすべき
//		false
//			フラッシュすべきでない
//
//	EXCEPTIONS

bool
Pool::FlushablePageFilter::ForFile::operator ()(const Page& page) const
{
	return _file.getID() == page.getFileID() &&
		(_offset < 0 || page.getOffset() < _offset) &&
		!(!_noDeterred && page._file.isDeterred() &&
		  page._status & Page::Status::Deterrentable);
}

//	FUNCTION public
//	Buffer::Pool::DiscardablePageFilter::ForPool::operator () -- () 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Page&		page
//			破棄すべきか判定するバッファページのバッファページ記述子
//
//	RETURN
//		true
//			破棄すべき
//		false
//			破棄すべきでない
//
//	EXCEPTIONS

bool
Pool::DiscardablePageFilter::ForPool::operator ()(const Page& page) const
{
	return true;
}

//	FUNCTION public
//	Buffer::Pool::DiscardablePageFilter::ForFile::operator () -- () 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Page&		page
//			破棄すべきか判定するバッファページのバッファページ記述子
//
//	RETURN
//		true
//			破棄すべき
//		false
//			破棄すべきでない
//
//	EXCEPTIONS

bool
Pool::DiscardablePageFilter::ForFile::operator ()(const Page& page) const
{
	return _file.getID() == page.getFileID() &&	page.getOffset() >= _offset;
}

//	FUNCTION public
//	Buffer::Pool::DirtyList::insert -- ある位置に要素を挿入する
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Pool::DirtyList::Iterator	position
//			要素を挿入する位置を指す反復子
//		Buffer::Pool::DirtyList::Iterator	start
//			挿入する要素のうち、先頭のものを指す反復子
//		Buffer::Pool::DirtyList::Iterator	end
//			挿入する要素のうち、末尾のものの直後を指す反復子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Pool::DirtyList::insert(Iterator position, Iterator start, Iterator end)
{
	Os::Memory::LongSize size = 0;
	for (Iterator ite = start; ite != end; ++ite) {
		; _SYDNEY_ASSERT(*ite);
		size += (*ite)->getSize();
	}

	Super::insert(position, start, end);

	_total += size;
}

//	FUNCTION public
//	Buffer::Pool::DirtyList::insert -- ある位置にリストの全要素を挿入する
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Pool::DirtyList::Iterator	position
//			要素を挿入する位置を指す反復子
//		Buffer::Pool::DirtyList&	src
//			挿入する要素を保持するリスト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Pool::DirtyList::insert(Iterator position, const DirtyList& src)
{
	Super::insert(position, src.begin(), src.end());

	_total += src._total;
}

//	FUNCTION public
//	Buffer::Pool::DirtyList::pushBack -- 末尾に要素を追加する
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Page*		page
//			追加する要素
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Pool::DirtyList::pushBack(Page* page)
{
	; _SYDNEY_ASSERT(page);

	Super::pushBack(page);

	_total += page->getSize();
}

//	FUNCTION public
//	Buffer::Pool::LruList::LruList -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Pool::LruList::Category::Value	category
//			LRU リストの種別
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Pool::LruList::LruList(Category::Value category)
	: Common::DoubleLinkedList<Page>(&Page::_lruPrev, &Page::_lruNext)
	, _category(category)
	, _total(0)
{}

//	FUNCTION public
//	Buffer::Pool::LruList::pushBack -- 末尾に要素を追加する
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Page&		page
//			追加する要素
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Pool::LruList::pushBack(Page& page)
{
	Super::pushBack(page);

	page._lruListCategory = _category;
	_total += page.getSize();
}

//	FUNCTION public
//	Buffer::Pool::LruList::erase -- 要素を削除する
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Page&		page
//			削除する要素
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Pool::LruList::erase(Page& page)
{
	Iterator ite(*this, &page);
	Super::erase(ite);

	page._lruListCategory = Category::Unknown;
	_total -= page.getSize();
}

//	FUNCTION public
//	Buffer::Pool::LruList::popFront -- 先頭の要素を削除する
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
Pool::LruList::popFront()
{
	if (getSize()) {
		Page& page = Super::getFront();
		Super::popFront();

		page._lruListCategory = Category::Unknown;
		_total -= page.getSize();
	}
}

//	FUNCTION public
//	Buffer::Pool::LruList::splice --
//		あるバッファページ記述子を自分のある位置へ移動する
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Pool::LruList::Iterator&	position
//			ある要素を移動する位置の直後の要素を指す反復子
//		Buffer::Pool::LruList&	src
//			移動する要素を持つ LRU リスト
//		Buffer::Page&		page
//			移動するバッファページ記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Pool::LruList::splice(Iterator& position, LruList& src, Page& page)
{
	Iterator ite(src, &page);
	Super::splice(position, src, ite);

	page._lruListCategory = _category;
	const Os::Memory::LongSize pageSize = page.getSize();
	_total += pageSize, src._total -= pageSize;
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
