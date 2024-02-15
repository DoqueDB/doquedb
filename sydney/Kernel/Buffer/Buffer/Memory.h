// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Memory.h -- バッファメモリ関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BUFFER_MEMORY_H
#define	__SYDNEY_BUFFER_MEMORY_H

#include "Buffer/Module.h"

#include "Os/Memory.h"

_SYDNEY_BEGIN
_SYDNEY_BUFFER_BEGIN

class Page;

//	CLASS
//	Buffer::Memory --
//		バッファメモリをバッファマネージャー外から操作するためのクラス
//
//	NOTES
//		バッファマネージャーの外には、
//		バッファページ記述子内のバッファメモリをじかに見せない
//		バッファマネージャーの外では、
//		このクラスを介してバッファメモリを操作することになる

class Memory
{
	friend class Page;
public:
	struct Category
	{
		//	ENUM
		//	Buffer::Memory::Category::Value -- バッファメモリの種別を表す値の型
		//
		//	NOTES

		typedef	unsigned char	Value;
		enum
		{
			// 不明
			Unknown =		0x00,
			// 読取専用
			ReadOnly =		0x01,
			// 読み書き用
			Write =			0x02,
			// ページ確保用
			Allocate =		0x04,

			// 更新内容のフラッシュは抑制され得る
			Deterrentable =	0x10,
			// 更新は作業用メモリを介して行う
			Discardable =	0x20,
			// 読み取り書き込みロックを行わない
			NoLock =		0x40,

			// マスク
			Mask =			0x77
		};
	};

	// デフォルトコンストラクター
	Memory();
	// コピーコンストラクター
	Memory(const Memory& r);
	// デストラクター
	~Memory();

	// = 演算子
	Memory&
	operator =(const Memory& src);

	// == 演算子
	bool
	operator ==(const Memory& r) const;
	// != 演算子
	bool
	operator !=(const Memory& r) const;

	// void* へのキャスト演算子
	operator void*();
	// const void* へのキャスト演算子
	operator const void*() const;
	// char* へのキャスト演算子
	SYD_BUFFER_FUNCTION
	operator char*();
	// const char* へのキャスト演算子
	SYD_BUFFER_FUNCTION
	operator const char*() const;

	// アンフィックスする
	SYD_BUFFER_FUNCTION
	void
	unfix(bool dirty = false, bool asynchronously = true);
	// 再フィックスする
	SYD_BUFFER_FUNCTION
	Memory
	refix() const;
	// これまでの更新内容を破棄不可にする
	SYD_BUFFER_FUNCTION
	void
	touch(bool dirty = false);
	// 更新したことにする
	void
	dirty();
	// 更新内容を破棄可能にする
	void
	discardable();

	// バッファメモリの種別を得る
	Category::Value
	getCategory() const;
	// バッファメモリのサイズを得る
	SYD_BUFFER_FUNCTION
	Os::Memory::Size
	getSize() const;
	// バッファリングされているバッファページのサイズを得る
	SYD_BUFFER_FUNCTION
	Os::Memory::Size
	getPageSize() const;

	// バッファメモリを所有しているか調べる
	bool
	isOwner() const;
	// バッファメモリは更新可能か調べる
	bool
	isUpdatable() const;
	// バッファメモリは更新内容を破棄可能か調べる
	bool
	isDiscardable() const;
	// バッファメモリは更新したことになっているか調べる
	bool
	isDirty() const;

private:
	// コンストラクター
	Memory(Category::Value category, Page& page, bool dirty);

	// 所有権を放棄する
	Page*
	release();

	// バッファメモリの種別
	Category::Value			_category;
	// バッファメモリを所有しているか
	bool					_owner;
	// バッファメモリは更新された
	bool					_dirty;
	// バッファメモリを使って OS ファイル領域をバッファリングする
	// バッファページのバッファページ記述子を格納する先頭アドレス
	Page*					_page;
};

//	FUNCTION public
//	Buffer::Memory::Memory --
//		バッファメモリを表すクラスのデフォルトコンストラクター
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
Memory::Memory()
	: _category(Category::Unknown)
	, _owner(false)
	, _dirty(false)
	, _page(0)
{}

//	FUNCTION public
//	Buffer::Memory::Memory --
//		バッファメモリを表すクラスのコピーコンストラクター
//
//	NOTES
//		もし、コピー元がバッファメモリの所有権を持っていれば、
//		所有権はコピー先へ譲渡される
//
//	ARGUMENTS
//		Buffer::Memory&		r
//			コピー元のバッファメモリを表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Memory::Memory(const Memory& r)
	: _category(r.getCategory())
	, _owner(r.isOwner())
	, _dirty(r.isDirty())
	, _page(0)
{
	//【注意】	r.isOwner() による _owner の初期化を先にしてから
	//			r.release() を呼び出す必要がある

	_page = const_cast<Memory&>(r).release();
}

//	FUNCTION public
//	Buffer::Memory::~Memory -- バッファメモリを表すクラスのデストラクター
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
Memory::~Memory()
{
	unfix();
}

//	FUNCTION public
//	Buffer::Memory::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Memory&		src
//			複写元となるバッファメモリを表すクラス
//
//	RETURN
//		複写後の自分自身
//
//	EXCEPTIONS

inline
Memory&
Memory::operator =(const Memory& src)
{
	if (this != &src) {

		// まず、自分自身をアンフィックスする
		
		unfix();

		//【注意】	src.isOwner() による _owner の初期化を先にしてから
		//			src.release() を呼び出す必要がある

		_category = src.getCategory();
		_owner = src.isOwner();
		_dirty = src.isDirty();
		_page = const_cast<Memory&>(src).release();
	}
	return *this;
}

//	FUNCTION public
//	Buffer::Memory::operator == -- == 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Memory&		r
//			自分自身と比較するバッファメモリを表すクラス
//
//	RETURN
//		true
//			自分自身と等しい
//		false
//			自分自身と等しくない
//
//	EXCEPTIONS
//		なし

inline
bool
Memory::operator ==(const Memory& r) const
{
	//【注意】	バッファページ記述子はバッファページに対して
	//			たった 1 つしか作られないので、
	//			バッファページ記述子を格納する領域のアドレスを比較すればよい

	return _page == r._page;
}

//	FUNCTION public
//	Buffer::Memory::operator != -- != 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Memory&		r
//			自分自身と比較するバッファメモリを表すクラス
//
//	RETURN
//		true
//			自分自身と等しくない
//		false
//			自分自身と等しい
//
//	EXCEPTIONS
//		なし

inline
bool
Memory::operator !=(const Memory& r) const
{
	return _page != r._page;
}

//	FUNCTION public
//	Buffer::Memory::operator void* -- void* へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		バッファメモリまたは作業用メモリの実体である自由記憶領域の先頭アドレス
//
//	EXCEPTIONS

inline
Memory::operator void*()
{
	return operator char*();
}

//	FUNCTION public
//	Buffer::Memory::operator const void* -- const void* へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		バッファメモリまたは作業用メモリの実体である自由記憶領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

inline
Memory::operator const void*() const
{
	return operator const char*();
}

//	FUNCTION private
//	Buffer::Memory::release -- バッファメモリの所有権を放棄する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		所有権を放棄したバッファメモリを使って OS ファイル領域を
//		バッファリングするバッファページのバッファページ記述子を
//		格納する先頭アドレス
//
//	EXCEPTIONS
//		なし

inline
Page*
Memory::release()
{
	//【注意】	メンバー _page を初期化すると、
	//			operator == や != が正しく動かなくなる

	_owner = _dirty = false;
	return _page;
}

//	FUNCTION public
//	Buffer::Memory::dirty -- バッファメモリを更新したことにする
//
//	NOTES
//		アンフィックス時に更新しなかったことにしても、
//		更新したことになる
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
void
Memory::dirty()
{
	if (isOwner() && isUpdatable())

		// バッファメモリの所有権を持ち、更新可なので、
		// 更新したことにする

		_dirty = true;
}

//	FUNCTION public
//	Buffer::Memory::getCategory -- バッファメモリの種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバッファメモリの種別
//
//	EXCEPTIONS
//		なし

inline
Memory::Category::Value
Memory::getCategory() const
{
	return _category;
}

//	FUNCTION publuc
//	Buffer::Memory::isOwner -- バッファメモリを所有しているか調べる
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
Memory::isOwner() const
{
	return _owner;
}

//	FUNCTION public
//	Buffer::Memory::isUpdatable -- バッファメモリを更新可能か調べる
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
Memory::isUpdatable() const
{
	return !(getCategory() & Category::ReadOnly);
}

//	FUNCTION public
//	Buffer::Memory::isDiscardable -- バッファメモリの更新内容を破棄可能か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			更新内容を破棄可能である
//		false
//			更新内容を破棄不可である
//
//	EXCEPTIONS
//		なし

inline
bool
Memory::isDiscardable() const
{
	return getCategory() & Category::Discardable;
}

//	FUNCTION public
//	Buffer::Memory::isDirty --
//		バッファメモリは更新されていることになっているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			更新されていることになっている
//		false
//			更新されていることになっていない
//
//	EXCEPTIONS
//		なし

inline
bool
Memory::isDirty() const
{
	return _dirty;
}

_SYDNEY_BUFFER_END
_SYDNEY_END

#endif	// __SYDNEY_BUFFER_MEMORY_H

//
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
