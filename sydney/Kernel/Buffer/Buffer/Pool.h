// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Pool.h -- バッファプール関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2008, 2009, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BUFFER_POOL_H
#define	__SYDNEY_BUFFER_POOL_H

#include "Buffer/Module.h"

#include "Common/DoubleLinkedList.h"
#include "Os/CriticalSection.h"
#include "Os/File.h"
#include "Os/RWLock.h"

#include "ModTime.h"
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
#include "Common/LargeVector.h"
#else
#include "ModVector.h"
#endif

template <class T>
class ModAutoPointer;

_SYDNEY_BEGIN
_SYDNEY_BUFFER_BEGIN

class File;
class Page;

namespace Manager
{
	class Page;
}

//	CLASS
//	Buffer::Pool -- バッファプール記述子を表すクラス
//
//	NOTES

class Pool
{
	friend class DirtyPageFlusher;
	friend class File;
	friend class Manager::Page;
	friend class Memory;
	friend class ModAutoPointer<Pool>;
	friend class Page;
public:
	struct Category
	{
		//	ENUM
		//	Buffer::Pool::Category::Value --
		//		バッファプールの種別を表す値の列挙型
		//
		//	NOTES

		typedef unsigned char	Value;
		enum
		{
			// 通常
			Normal =		0,
			// 一時データを格納する
			Temporary,
			// 読取専用のデータを格納する
			ReadOnly,
			// 論理ログデータを格納する
			LogicalLog,
			// 値の数
			Count,
			// 不明
			Unknown =		Count
		};
	};

	struct DiscardablePageFilter
	{
		//	CLASS
		//	Buffer::Pool::DiscardablePageFilter::Base --
		//		破棄すべきページかを
		//		判定するために使うクラスの純粋仮想基底クラス
		//
		//	NOTES

		struct Base
		{
			// 破棄すべきバッファページか調べる
			virtual bool
			operator ()(const Page& page) const = 0;
		};

		//	CLASS
		//	Buffer::Pool::DiscardablePageFilter::ForPool --
		//		バッファプール中のバッファページについて
		//		破棄すべきページかを判定するために使うクラス
		//
		//	NOTES

		class ForPool
			: public	Base
		{
		public:
			// コンストラクター
			ForPool();
			// デストラクター
			~ForPool();

			// 破棄すべきバッファページか調べる
			SYD_BUFFER_FUNCTION 
			bool
			operator ()(const Page& page) const;
		};

		//	CLASS
		//	Buffer::Pool::DiscardablePageFilter::ForFile --
		//		あるバッファファイルのバッファページについて
		//		破棄すべきページかを判定するために使うクラス
		//
		//	NOTES

		class ForFile
			: public	Base
		{
		public:
			// コンストラクター
			ForFile(const File& file, Os::File::Offset offset);
			// デストラクター
			~ForFile();

			// 破棄すべきバッファページか調べる
			SYD_BUFFER_FUNCTION 
			bool
			operator ()(const Page& page) const;

			// このバッファファイル記述子の表す
			// バッファファイルのバッファページに限定する
			const File&				_file;
			// このファイルオフセット以上の領域を
			// バッファリングするバッファページに限定する
			const Os::File::Offset	_offset;
		};
	};

	struct FlushablePageFilter
	{
		//	CLASS
		//	Buffer::Pool::FlushablePageFilter::Base --
		//		フラッシュすべきページかを
		//		判定するために使うクラスの純粋仮想基底クラス
		//
		//	NOTES

		struct Base
		{
			// フラッシュすべきバッファページか調べる
			virtual bool
			operator ()(const Page& page) const = 0;
		};

		//	CLASS
		//	Buffer::Pool::FlushablePageFilter::ForPool --
		//		バッファプール中のバッファページについて
		//		フラッシュすべきページかを判定するために使うクラス
		//
		//	NOTES

		class ForPool
			: public	Base
		{
		public:
			// コンストラクター
			ForPool(bool onlyMarked, bool noDeterred);
			// デストラクター
			~ForPool();

			// フラッシュすべきバッファページか調べる
			SYD_BUFFER_FUNCTION 
			bool
			operator ()(const Page& page) const;

			// マークされているものに限定する
			const bool				_onlyMarked;
			// フラッシュが抑止されていないものに限定する
			const bool				_noDeterred;
		};

		//	CLASS
		//	Buffer::Pool::FlushablePageFilter::ForFile --
		//		あるバッファファイルのバッファページについて
		//		フラッシュすべきページかを判定するために使うクラス
		//
		//	NOTES

		class ForFile
			: public	Base
		{
		public:
			// コンストラクター
			ForFile(const File& file, Os::File::Offset offset, bool noDeterred);
			// デストラクター
			~ForFile();

			// フラッシュすべきバッファページか調べる
			SYD_BUFFER_FUNCTION 
			bool
			operator ()(const Page& page) const;

			// このバッファファイル記述子の表す
			// バッファファイルのバッファページに限定する
			const File&				_file;
			// このファイルオフセット未満の領域を
			// バッファリングするバッファページに限定する
			const Os::File::Offset	_offset;
			// フラッシュが抑止されていないものに限定する
			const bool				_noDeterred;
		};
	};

	// 記述子を生成する
	SYD_BUFFER_FUNCTION 
	static Pool*
	attach(Category::Value category);
	// 記述子を破棄する
	SYD_BUFFER_FUNCTION 
	static void
	detach(Pool*& pool);

	// あるバッファページ記述子を破棄する
	SYD_BUFFER_FUNCTION
	void
	discardPage(const DiscardablePageFilter::Base& filter, File* pFile_);
	// ダーティなバッファページをフラッシュする
	SYD_BUFFER_FUNCTION
	void
	flushDirtyPage(const FlushablePageFilter::Base& filter, bool force);
	// ダーティリスト上のダーティなバッファページをすべてマークする
	SYD_BUFFER_FUNCTION
	bool
	markDirtyPage();

	// すべてのファイルをsyncする
	SYD_BUFFER_FUNCTION
	void
	syncFile();

	// 種別を得る
	Category::Value
	getCategory() const;
	// 上限を得る
	Os::Memory::LongSize
	getLimit() const;

private:
	//	CLASS
	//	Pool::DirtyList -- ダーティリストを表すクラス
	//
	//	NOTES

	class DirtyList
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
		: private	Common::LargeVector<Page*>
#else
		: private	ModVector<Page*>
#endif	
	{
		//	TYPEDEF
		//	Buffer::Pool::DirtyList::Super -- 親クラスを表す型
		//
		//	NOTES

#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
		typedef Common::LargeVector<Page*>			Super;
#else
		typedef ModVector<Page*>					Super;
#endif

	public:
		using Super::Iterator;

		//	TYPEDEF
		//	Buffer::Pool::DirtyList::Size -- リストの長さを表す型
		//
		//	NOTES

		typedef ModSize						Size;

		// デフォルトコンストラクター
		DirtyList();
		// コンストラクター
		DirtyList(Size n);

		using Super::reserve;
		using Super::begin;
		using Super::end;

		// ある位置へ要素を挿入する
		void
		insert(Iterator position, Iterator start, Iterator end);
		// ある位置へあるリストの全要素を挿入する
		void
		insert(Iterator position, const DirtyList& src);

		// 末尾に要素を追加する
		void
		pushBack(Page* page);

		using Super::isEmpty;
		using Super::getSize;
		// ダーティリストに登録されているバッファページの総サイズ(B 単位)
		Os::Memory::LongSize		_total;
	};

	//	CLASS
	//	Buffer::Pool::LruList -- LRU リストを表すクラス
	//
	//	NOTES

	class LruList
		: private	Common::DoubleLinkedList<Page>
	{
		//	TYPEDEF
		//	Buffer::Pool::LruList::Super -- 親クラスを表す型
		//
		//	NOTES

		typedef Common::DoubleLinkedList<Page>	Super;

	public:
		struct Category
		{
			//	ENUM
			//	Buffer::Pool::LruList::Category::Value --
			//		LRU リストの種別を表す値の列挙型
			//
			//	NOTES

			typedef unsigned char	Value;
			enum
			{
				T1 =			0,
#ifdef USE_ARC
				T2,
				B1,
				B2,
#endif
				// 値の数
				Count,
				// 不明
				Unknown =		Count
			};
		};

		using Super::Iterator;
		using Super::Size;

		// コンストラクター
		LruList(Category::Value category);

		using Super::begin;
		using Super::end;
		using Super::getFront;

		// 末尾に要素を追加する
		void
		pushBack(Page& page);

		// 要素を削除する
		void
		erase(Page& page);
		// 先頭の要素を削除する
		void
		popFront();

		// あるバッファページ記述子をある位置へ移動する
		void
		splice(Iterator& position, LruList& src, Page& page);

		using Super::getSize;

		// 種別
		const Category::Value	_category;
		// LRU リストに登録されているバッファページの総サイズ(B 単位)
		Os::Memory::LongSize		_total;
	};

	// コンストラクター
	Pool(Category::Value category, Os::Memory::LongSize limit);
	// デストラクター
	~Pool();
	// デストラクター下位関数
	void
	destruct();

	// 参照数を 1 増やす
	Pool*
	attach();
	// 参照数を 1 減らす
	void
	detach();
	// 参照数を得る
	unsigned int
	getRefCount() const;

	// バッファメモリを確保する
	void*
	allocateMemory(Os::Memory::Size size,
				   bool reset, bool& allocated, bool force = false);
	// バッファメモリを破棄する
	void
	freeMemory(void*& p, Os::Memory::Size size);
	// バッファメモリを再利用する
	void*
	replaceMemory(Os::Memory::Size size,
				  bool reset, bool& allocated, Page& page, bool force);

	// 確保中のバッファメモリの総サイズを得る
	Os::Memory::LongSize
	getSize() const;

#ifdef USE_ARC
	// ARC パラメータを計算しなおす
	void
	calculateParameter(const Page& page);
	// 置換の候補を得る
	void*
	getCandidate(Os::Memory::Size size,
				 bool reset, bool& allocated, const Page& page);
#endif
	void*
	getCandidate(Os::Memory::Size size,
				 bool reset, bool& allocated
#ifdef USE_ARC
				 , LruList& t, LruList& b
#endif
				 , bool force
				 );
	void*
	getCandidate(Os::Memory::Size size,
				 bool reset, bool& allocated
#ifdef USE_ARC
				 , LruList& t, LruList& b
#endif
				 , unsigned int& rest, bool force);
#ifdef USE_ARC
	// バッファメモリが未確保のバッファページをあるサイズぶん破棄する
	bool
	releaseVictim(Os::Memory::LongSize size, LruList& b);
#endif
	// サイズを減らす
	bool
	shrink(Os::Memory::LongSize upper);
#ifdef USE_ARC
	bool
	shrink(Os::Memory::LongSize upper, LruList& t, LruList& b);
#endif

	// 連続したバッファメモリを一括して書き込む
	void
	writeBodies(Os::File::IOBuffer* bodies, unsigned int& n,
				DirtyList::Iterator& ite, const DirtyList::Iterator& end);

	// LRU リストを得る
	LruList&
	getLruList(LruList::Category::Value category);

	// 排他制御用のラッチを得る
	Os::CriticalSection&
	getLatch() const;
	// ダーティリスト切り替えの排他制御用の読み取り書き込みロックを得る
	Os::RWLock&
	getRWLock() const;

	// 排他制御用のラッチ
	mutable Os::CriticalSection	_latch;
	// ダーティリスト切り替えの排他制御用の読み取り書き込みロック
	mutable Os::RWLock		_rwlock;

	// バッファプールの種別
	const Category::Value	_category;
	// バッファプールのサイズの上限
	const Os::Memory::LongSize	_limit;
	// 参照回数
	//【注意】	$$$::_Pool::_latch で保護される
	mutable unsigned int	_refCount;
	// ARC パラメータ
	Os::Memory::LongSize	_parameter;
	// LRU リスト
	LruList					_t1;
#ifdef USE_ARC
	LruList					_t2;
	LruList					_b1;
	LruList					_b2;
#endif
	// ダーティリスト
	DirtyList*				_dirtyList;

	// フラッシュに用いるバッファ
	Os::File::IOBuffer*		m_pIOBuffer;
};

//	FUNCTION private
//	Buffer::Pool::~Pool -- バッファプール記述子を表すクラスのデストラクター
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
Pool::~Pool()
{
	destruct();
}

//	FUNCTION public
//	Buffer::Pool::getCategory -- 種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた種別
//
//	EXCEPTIONS
//		なし

inline
Pool::Category::Value
Pool::getCategory() const
{
	return _category;
}

//	FUNCTION public
//	Buffer::Pool::getLimit -- 上限を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた上限(B 単位)
//
//	EXCEPTIONS
//		なし

inline
Os::Memory::LongSize
Pool::getLimit() const
{
	return _limit;
}

//	FUNCTION private
//	Buffer::Pool::getSize -- 確保中のバッファメモリの総サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた総サイズ(B単位)
//
//	EXCEPTIONS
//		なし

inline
Os::Memory::LongSize
Pool::getSize() const
{
	//【注意】	バッファプールはラッチ済であること

	return _t1._total
#ifdef USE_ARC
		+ _t2._total
#endif
		;
}

//	FUNCTION private
//	Buffer::Pool::getLatch --
//		バッファプールの操作の排他制御をするためのラッチを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ラッチへのリファレンス
//
//	EXCEPTIONS
//		なし

inline
Os::CriticalSection&
Pool::getLatch() const
{
	return _latch;
}

//	FUNCTION private
//	Buffer::Page::getRWLock --
//		ダーティリスト切り替えの排他制御をするための
//		読み取り書き込みロックを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		読み取り書き込みロックへのリファレンス
//
//	EXCEPTIONS
//		なし

inline
Os::RWLock&
Pool::getRWLock() const
{
	return _rwlock;
}

//	FUNCTION public
//	Buffer::Pool::FlushablePageFilter::ForPool::ForPool -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		bool				onlyMarked
//			true
//				マークされているものに限定する
//			false
//				マークされているものに限定しない
//		bool				noDeterred
//			true
//				フラッシュが抑止されていないものに限定しない
//			false
//				フラッシュが抑止されていないものに限定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Pool::FlushablePageFilter::ForPool::ForPool(bool onlyMarked, bool noDeterred)
	: _onlyMarked(onlyMarked)
	, _noDeterred(noDeterred)
{}

//	FUNCTION public
//	Buffer::Pool::FlushablePageFilter::ForPool::~ForPool -- デストラクター
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
Pool::FlushablePageFilter::ForPool::~ForPool()
{}

//	FUNCTION public
//	Buffer::Pool::FlushablePageFilter::ForFile::ForFile -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::File&		file
//			このバッファファイル記述子の表す
//			バッファファイルのバッファページに限定する
//		Os::File::Offset	offset
//			0 以上の値
//				このファイルオフセット未満の領域を
//				バッファリングするバッファページに限定する
//			それ以外の値
//				バッファファイル全体をフラッシュする
//		bool				noDeterred
//			true
//				フラッシュが抑止されていないものに限定しない
//			false
//				フラッシュが抑止されていないものに限定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Pool::FlushablePageFilter::ForFile::ForFile(
	const File& file, Os::File::Offset offset, bool noDeterred)
	: _file(file)
	, _offset(offset)
	, _noDeterred(noDeterred)
{}

//	FUNCTION public
//	Buffer::Pool::FlushablePageFilter::ForFile::~ForFile -- デストラクター
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
Pool::FlushablePageFilter::ForFile::~ForFile()
{}

//	FUNCTION public
//	Buffer::Pool::DiscardablePageFilter::ForPool::ForPool -- コンストラクター
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
Pool::DiscardablePageFilter::ForPool::ForPool()
{}

//	FUNCTION public
//	Buffer::Pool::DiscardablePageFilter::ForPool::~ForPool -- デストラクター
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
Pool::DiscardablePageFilter::ForPool::~ForPool()
{}

//	FUNCTION public
//	Buffer::Pool::DiscardablePageFilter::ForFile::ForFile -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::File&		file
//			このバッファファイル記述子の表す
//			バッファファイルのバッファページに限定する
//		Os::File::Offset	offset
//			このファイルオフセット以上の領域を
//			バッファリングするバッファページに限定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Pool::DiscardablePageFilter::ForFile::ForFile(
	const File& file, Os::File::Offset offset)
	: _file(file)
	, _offset(offset)
{}

//	FUNCTION public
//	Buffer::Pool::DiscardablePageFilter::ForFile::~ForFile -- デストラクター
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
Pool::DiscardablePageFilter::ForFile::~ForFile()
{}

//	FUNCTION public
//	Buffer::Pool::DirtyList::DirtyList -- デフォルトコンストラクター
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
Pool::DirtyList::DirtyList()
	: _total(0)
{}

//	FUNCTION public
//	Buffer::Pool::DirtyList::DirtyList -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Pool::DirtyList::Size	n
//			この個数の要素を格納するための領域を事前に確保する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
Pool::DirtyList::DirtyList(Size n)
	: _total(0)
{
	reserve(n);
}

_SYDNEY_BUFFER_END
_SYDNEY_END

#endif	// __SYDNEY_BUFFER_POOL_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2008, 2009, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
