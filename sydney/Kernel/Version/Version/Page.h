// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Page.h -- バージョンページ関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VERSION_PAGE_H
#define	__SYDNEY_VERSION_PAGE_H

#include "Version/Module.h"
#include "Version/Block.h"

#include "Buffer/Page.h"
#include "Buffer/ReplacementPriority.h"
#include "Common/Object.h"
#include "Os/CriticalSection.h"
#include "Os/Memory.h"
#include "Admin/Verification.h"
#include "Trans/Transaction.h"

#include "ModVector.h"

template <class T>
class ModAutoPointer;

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}

_SYDNEY_VERSION_BEGIN

class File;
template <class T>
class HashTable;
class Verification;

namespace Manager
{
	class File;
	class Page;
}

_SYDNEY_VERSION_VERSIONLOG_BEGIN

class File;

_SYDNEY_VERSION_VERSIONLOG_END

//	CLASS
//	Version::Page -- バージョンページ記述子を表すクラス
//
//	NOTES

class Page
{
	friend class AutoPage;
	friend class DetachedPageCleaner;
	friend class File;
	friend class Manager::File;
	friend class Manager::Page;
	friend class ModAutoPointer<Page>;
	friend class VersionLog::File;
public:
	//	TYPEDEF
	//	Version::Page::ID -- バージョンページ識別子を表す型
	//
	//	NOTES
	//		バージョンファイルの先頭からなん番目に位置する
	//		バージョンページかを表す値で、そのバージョンファイル内で一意である

	typedef	Block::ID		ID;

	//	CLASS
	//	Version::Page::Memory --
	//		バージョンページのバッファリング内容を表すクラス
	//
	//	NOTES
	//		バージョン管理マネジャーの外には、
	//		バージョンページのバッファリング内容をじかに見せない
	//		バージョン管理マネジャーの外では、
	//		このクラスを介してバージョンページの
	//		バッファリング内容を操作することになる

	class Memory
		: private	Block::Memory
	{
		friend class Page;
	public:
		// コンストラクター
		Memory();
		// デストラクター
		~Memory();

		// = 演算子
		Memory&
		operator =(const Memory& src);

		// void* へのキャスト演算子
		operator			void*();
		// const void* へのキャスト演算子
		operator			const void*() const;
		// char* へのキャスト演算子
		SYD_VERSION_FUNCTION
		operator			char*();
		// const char* へのキャスト演算子
		SYD_VERSION_FUNCTION
		operator			const char*() const;

#ifdef OBSOLETE
		// バッファリング内容を上書きする
		SYD_VERSION_FUNCTION
		Memory&				copy(const Memory& src);
#endif
		// バッファリング内容をリセットする
		SYD_VERSION_FUNCTION
		Memory&				reset();

		// アンフィックスする
		SYD_VERSION_FUNCTION
		void				unfix(bool dirty);
		// これまでの更新内容を破棄不可にする
		SYD_VERSION_FUNCTION
		void				touch(bool dirty);
		// 更新したことにする
		SYD_VERSION_FUNCTION
		void				dirty();

		// バッファリングされているバージョンページのページ識別子を得る
		SYD_VERSION_FUNCTION
		ID
		getPageID() const;
		// バッファリング内容を格納する領域のサイズを得る
		Os::Memory::Size
		getSize() const;
		// 最終更新時タイムスタンプを得る
		Trans::TimeStamp::Value
		getLastModification() const;

		// バッファリング内容を所有しているか
		bool
		isOwner() const;
		// バッファリング内容は更新可能か
		bool
		isUpdatable() const;
		// バッファリング内容を更新した結果を破棄可能か調べる
		bool
		isDiscardable() const;

	private:
		// コンストラクター
		Memory(Page& page,
			   const Trans::Transaction& trans, const Block::Memory& src);

		// バッファリングされているバージョンページのサイズを得る
//		Block::Memory::
//		Os::Memory::Size	getPageSize() const;

		// バージョンページ記述子を格納する領域の先頭アドレス
		Page*				_page;
		// バージョンページをフィックスした
		// トランザクションのトランザクション記述子
		const Trans::Transaction*	_trans;
	};

	friend class Memory;

	// 読み出し、バッファリングする
	static void
	fetch(const Trans::Transaction& trans, File& file, ID id);

	// フィックスする
	static Memory
	fix(const Trans::Transaction& trans, File& file, ID id,
		Buffer::Page::FixMode::Value mode =	Buffer::Page::FixMode::ReadOnly,
		Buffer::ReplacementPriority::Value priority =
			Buffer::ReplacementPriority::Low);

	// 整合性を検査する
	SYD_VERSION_FUNCTION
	static void
	verify(const Trans::Transaction& trans, File& file, ID id,
		   Admin::Verification::Progress& progress);
	// 整合性を検査し、一貫性があれば、フィックスする
	SYD_VERSION_FUNCTION
	static Memory
	verify(const Trans::Transaction& trans, File& file, ID id,
		   Buffer::Page::FixMode::Value mode,
		   Admin::Verification::Progress& progress);

	// バージョンファイル記述子を得る
	const File&				getFile() const;
	// バージョンページ識別子を得る
	ID						getID() const;
	// サイズを得る
	SYD_VERSION_FUNCTION
	Os::Memory::Size		getSize() const;

	// 最新版を更新した更新トランザクションの
	// トランザクション識別子を得る
	const ModVector<Trans::Transaction::ID>&
	getModifierList() const;

	// 格納可能な内容のサイズを得る
	SYD_VERSION_FUNCTION
	static Os::Memory::Size	getContentSize(Os::Memory::Size size);

private:
	// コンストラクター
	Page(File& file, ID id);
	// デストラクター
	~Page();

	// メンバーを初期化する
	void initialize(File& file, ID id);

	// 記述子を生成する
	static Page*
	attach(File& file, ID id, bool create = true);
#ifdef OBSOLETE
	// 参照数を 1 増やす
	Page*
	attach();
#endif
	// 記述子を破棄する
	static void
	detach(Page*& page, bool reserve);
#ifdef OBSOLETE
	// 参照数を 1 減らす
	void
	detach();
#endif
	// 参照数を得る
	unsigned int
	getRefCount() const;

	// フィックスする
	SYD_VERSION_FUNCTION
	static Memory
	fix(const Trans::Transaction& trans, Verification* verification,
		File& file, ID id, Buffer::Page::FixMode::Value mode,
		Buffer::ReplacementPriority::Value priority);
	// 操作すべきブロックをフィックスする
	Block::Memory
	fixBlock(const Trans::Transaction& trans, Verification* verification,
			 Buffer::Page::FixMode::Value mode,
			 Buffer::ReplacementPriority::Value priority);

	// 可能であれば、ページ更新トランザクションリストを空にする
	bool					clearModifierList();

	// すべてのバージョンページ記述子を管理するハッシュ表を得る
	static HashTable<Page>&	getHashTable();
	// 排他制御用のラッチを得る
	Os::CriticalSection&	getLatch() const;

	// フリーリストから得る
	static Page* popFreeList();
	// フリーリストへつなぐ
	static void pushFreeList(Page* page);

	// 排他制御用のラッチ
	mutable Os::CriticalSection	_latch;

	// バージョンファイル記述子
	File*					_file;
	// バージョンページ識別子
	ID						_id;
	// 参照数
	//【注意】	Page::getHashTable().getBucket(i).getLatch() で保護される
	mutable unsigned int	_refCount;

	// 最新版を更新した更新トランザクションの
	// トランザクション識別子を管理するリスト
	ModVector<Trans::Transaction::ID>	_modifierList;

	// ハッシュリストでの直前の要素へのポインタ
	//【注意】	Page::getHashTable().getBucket(i).getLatch() で保護される
	Page*					_hashPrev;
	// ハッシュリストでの直後の要素へのポインタ
	//【注意】	Page::getHashTable().getBucket(i).getLatch() で保護される
	Page*					_hashNext;
};

//	FUNCTION private
//	Version::Page::Page --
//		バージョンページを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Version::File&		file
//			バージョンページが存在するバージョンファイルの
//			バージョンファイル記述子
//		Version::Page::ID	id
//			バージョンページのバージョンページ識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Page::Page(File& file, ID id)
	: _file(&file),
	  _id(id),
	  _refCount(0),
	  _hashPrev(0),
	  _hashNext(0)
{}

//	FUNCTION private
//	Version::Page::~Page --
//		バージョンページを表すクラスのデストラクター
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
Page::~Page()
{}

//	FUNCTION private
//	Version::Page::initalize -- メンバーを初期化する
//
//	NOTES
//
//	ARGUMENTS
//		Version::File&		file
//			バージョンページが存在するバージョンファイルの
//			バージョンファイル記述子
//		Version::Page::ID	id
//			バージョンページのバージョンページ識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
Page::initialize(File& file, ID id)
{
	_file = &file;
	_id = id;
	_refCount = 0;
	_hashPrev = 0;
	_hashNext = 0;
}

//	FUNCTION private
//	Version::Page::getRefCount -- バージョンページ記述子の参照数を得る
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
//		なし

inline
unsigned int
Page::getRefCount() const
{
	return _refCount;
}

//	FUNCTION public
//	Version::Page::fetch -- バージョンページをバッファに読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンページを読み出すトランザクションのトランザクション記述子
//		Version::File&		file
//			読み出すバージョンページが存在する
//			バージョンファイルのバージョンファイル記述子
//		Version::Page::ID	id
//			読み出すバージョンページのバージョンページ識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
inline
void
Page::fetch(const Trans::Transaction& trans, File& file, ID id)
{
	(void) fix(trans, 0, file, id, Buffer::Page::FixMode::ReadOnly,
			   Buffer::ReplacementPriority::Low);
}

//	FUNCTION public
//	Version::Page::fix -- バージョンページをフィックスする
//
//	NOTES
//		この関数の呼び出し中、またはフィックスされたバージョンページを
//		アンフィックスされるまでに、チェックポイント処理が行われたとき、
//		バージョンページを更新すると、バージョンファイルを構成する
//		バージョンログファイルの整合性が失われる可能性がある
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンページをフィックスする
//			トランザクションのトランザクション記述子
//		Version::File&		file
//			フィックスするバージョンページが存在する
//			バージョンファイルのバージョンファイル記述子
//		Version::Page::ID	id
//			フィックスするバージョンページのバージョンページ識別子
//		Buffer::Page::FixMode::Value	mode
//			指定されたとき
//				以下の値の論理和を指定する
//				Buffer::Page::FixMode::ReadOnly
//					フィックスするバージョンページは参照しかされない
//				Buffer::Page::FixMode::Write
//					フィックスするバージョンページは更新される
//				Buffer::Page::FixMode::Allocate
//					フィックスするバージョンページは
//					その領域の初期化のために使用する
//				Buffer::Page::FixMode::Discardable
//					フィックスしたバージョンページを更新しても、
//					アンフィックス時にダーティにしなければ、
//					バッファへ反映されない
//			指定されないとき
//				Buffer::Page::FixMode::ReadOnly が指定されたものとみなす
//		Buffer::ReplacementPriority::Value	priority
//			Buffer::ReplacementPriority::Low または指定されないとき
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
inline
Page::Memory
Page::fix(const Trans::Transaction& trans,
		  File& file, ID id, Buffer::Page::FixMode::Value mode,
		  Buffer::ReplacementPriority::Value priority)
{
	return fix(trans, 0, file, id, mode, priority);
}

//	FUNCTION private
//	Version::Page::getLatch --
//		バージョンページの操作の排他制御をするためのラッチを得る
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
Page::getLatch() const
{
	return _latch;
}

//	FUNCTION public
//	Version::Page::getFile --
//		バージョンページの存在する
//		バージョンファイルのバージョンファイル記述子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバージョンファイル記述子
//
//	EXCEPTIONS
//		なし

inline
const File&
Page::getFile() const
{
	return *_file;
}

//	FUNCTION public
//	Version::Page::getID --
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

inline
Page::ID
Page::getID() const
{
	return _id;
}

//	FUNCTION public
//	Version::Page::getModifierList --
//		あるバージョンページの最新版を更新した
//		更新トランザクションのトランザクション識別子を得る
//
//	NOTES
//		得られるトランザクション識別子の中には、
//		すでに終了しているトランザクションのものがある可能性がある
//
//	RETURN
//		得られたトランザクション識別子を要素とするベクター
//
//	EXCEPTIONS
//		なし

inline
const ModVector<Trans::Transaction::ID>&
Page::getModifierList() const
{
	return _modifierList;
}

//	FUNCTION public
//	Version::Page::Memory::Memory --
//		デフォルトコンストラクター
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
//	なし

inline
Page::Memory::Memory()
	: _page(0), _trans(0)
{}

//	FUNCTION public
//	Version::Page::Memory::Memory --
//		バージョンページのバッファリング内容を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Version::Page&		page
//			バージョンページ記述子
//		Trans::Transaction&	trans
//			バージョンページをフィックスした
//			トランザクションのトランザクション記述子
//		Version::Block::Memory&	src
//			バージョンページを記録するブロックのバッファリング内容を表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Page::Memory::Memory(Page& page,
					 const Trans::Transaction& trans, const Block::Memory& src)
	: Block::Memory(src),
	  _page(&page),
	  _trans(&trans)
{}

//	FUNCTION public
//	Version::Page::Memory::~Memory --
//		バージョンページのバッファリング内容を表すクラスのデストラクター
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
Page::Memory::~Memory()
{
	//【注意】	安全のために更新可のバッファリング内容は
	//			デフォルトで更新されたことになる
	//
	//			これは、Version::Block::Memory, 
	//			Buffer::Memory のデフォルトの動作と異なる

	unfix(isUpdatable());
}

//	FUNCTION public
//	Version::Page::Memory::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Version::Page::Memory& src
//			複写元となるバッファページのバッファリング内容を表すクラス
//
//	RETURN
//		複写後の自分自身
//
//	EXCEPTIONS

inline
Page::Memory&
Page::Memory::operator =(const Memory& src)
{
	if (this != &src) {
		static_cast<Block::Memory*>(this)->operator =(src);
	
		_page = src._page;
		_trans = src._trans;
	}
	return *this;
}

//	FUNCTION public
//	Version::Page::Memory::operator void* -- void* へのキャスト演算子
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

inline
Page::Memory::operator void*()
{
	return static_cast<void*>(operator char*());
}

//	FUNCTION public
//	Version::Page::Memory::operator const void* --
//		const void* へのキャスト演算子
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

inline
Page::Memory::operator const void*() const
{
	return static_cast<const void*>(operator const char*());
}

//	FUNCTION public
//	Version::Page::Memory::getSize --
//		バージョンページのバッファリング内容を格納する領域のサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

inline
Os::Memory::Size
Page::Memory::getSize() const
{
	return Page::getContentSize(getPageSize());
}

//	FUNCTION public
//	Version::Page::Memory::getLastModification --
//		バージョンページのバッファリング内容の最終更新時タイムスタンプを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたタイムスタンプの値
//
//	EXCEPTIONS
//		なし

inline
Trans::TimeStamp::Value
Page::Memory::getLastModification() const
{
	return static_cast<const Block::Memory*>(this)->getLastModification();
}

//	FUNCTION public
//	Version::Page::Memory::isOwner --
//		バージョンページのバッファリング内容を所有しているか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			所有している
//		false
//			所有していない
//
//	EXCEPTIONS
//		なし

inline
bool
Page::Memory::isOwner() const
{
	return static_cast<const Block::Memory*>(this)->isOwner();
}

//	FUNCTION public
//	Version::Page::Memory::isUpdatable --
//		バージョンページのバッファリング内容を更新可能か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			更新可能である
//		false
//			更新不可である
//
//	EXCEPTIONS
//		なし

inline
bool
Page::Memory::isUpdatable() const
{
	return static_cast<const Block::Memory*>(this)->isUpdatable();
}

//	FUNCTION public
//	Version::Page::Memory::isDiscardable --
//		バージョンページのバッファリング内容の更新結果を破棄可能か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			更新結果を破棄可能である
//		false
//			更新結果を破棄不可である
//
//	EXCEPTIONS
//		なし

inline
bool
Page::Memory::isDiscardable() const
{
	return static_cast<const Block::Memory*>(this)->isDiscardable();
}

_SYDNEY_VERSION_END
_SYDNEY_END

#endif	// __SYDNEY_VERSION_PAGE_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
