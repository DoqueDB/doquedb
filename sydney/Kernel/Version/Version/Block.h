// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Block.h -- ブロック関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VERSION_BLOCK_H
#define	__SYDNEY_VERSION_BLOCK_H

#include "Version/Module.h"

#include "Buffer/Memory.h"

#include "Os/Memory.h"
#include "Trans/TimeStamp.h"

_SYDNEY_BEGIN
_SYDNEY_VERSION_BEGIN

//	NAMESPACE
//	Version::Block -- ブロックに関する名前空間
//
//	NOTES

namespace Block
{
	//	TYPEDEF
	//	Version::Block::ID -- ブロック識別子を表す型
	//
	//	NOTES
	//		ブロックが存在する OS ファイルの先頭からなん番目に位置する
	//		ブロックかを表す値で、その OS ファイル内で一意である

	typedef	unsigned int	ID;

	//	CONST
	//	Version::Block::IllegalID -- 不正なブロック識別子
	//
	//	NOTES

	const ID				IllegalID = ~static_cast<ID>(0);

	//	CLASS
	//	Version::Block::Header -- ブロックのヘッダを表すクラス
	//
	//	NOTES

	struct Header
	{
		// ブロックの最終更新時刻を表すタイムスタンプ
		Trans::TimeStamp::Value	_lastModification;
	};

	//	CLASS
	//	Version::Block::Memory --
	//		ブロックのバッファリング内容を表すクラス
	//
	//	NOTES

	class Memory
		: public	Buffer::Memory
	{
	public:
		// デフォルトコンストラクター
		Memory();
		// コンストラクター
		SYD_VERSION_FUNCTION 
		Memory(ID id, const Buffer::Memory& bufMemory);
		// デストラクター
		~Memory();

		// void* へのキャスト演算子
		operator			void*();
		// const void* へのキャスト演算子
		operator			const void*() const;
		// char* へのキャスト演算子
		operator			char*();
		// const char* へのキャスト演算子
		operator			const char*() const;

		// = 演算子
		Memory&				operator =(const Memory& src);

		// == 演算子
		bool				operator ==(const Memory& r) const;
		// != 演算子
		bool				operator !=(const Memory& r) const;

		// バッファリング内容を上書きする
		SYD_VERSION_FUNCTION 
		Memory&				copy(const Memory& src);
		// バッファリング内容をリセットする
		SYD_VERSION_FUNCTION 
		Memory&				reset();

		// アンフィックスする
		SYD_VERSION_FUNCTION 
		void				unfix(bool dirty = false,
								  bool asynchronously = true);
		SYD_VERSION_FUNCTION 
		void				unfix(const Trans::TimeStamp& t,
								  bool asynchronously = true);
		// 再フィックスする
		Memory				refix() const;
		// これまでの更新内容を破棄不可にする
		SYD_VERSION_FUNCTION 
		void				touch(bool dirty = false);
		// 更新したことにする
//		Buffer::Memory::
//		void				dirty();

		// 更新内容を破棄可能にする
		SYD_VERSION_FUNCTION 
		void				discardable();

		// バッファリング内容を格納する領域のサイズを得る
		Os::Memory::Size	getSize() const;
		// バッファリングされているブロックのサイズを得る
//		Buffer::Memory::
//		Os::Memory::Size	getPageSize() const;

		// ブロック識別子を得る
		ID					getID() const;
		// ブロックのヘッダを得る
		const Header&		getHeader() const;
		// 最終更新時タイムスタンプを得る
		Trans::TimeStamp::Value	getLastModification() const;

		// バッファリング内容は更新可能か
//		Buffer::Memory::
//		bool				isUpdatable() const;
		// バッファメモリは更新内容を破棄可能か調べる
//		Buffer::Memory::
//		bool				isDiscardable() const;
		// バッファリング内容は更新したことになっているか
//		Buffer::Memory::
//		bool				isDirty() const;

	private:
		// ブロック識別子
		ID					_id;
	};

	// 格納可能な内容のサイズを得る
	SYD_VERSION_FUNCTION
	Os::Memory::Size		getContentSize(Os::Memory::Size size);
}

//	FUNCTION public
//	Version::Block::Memory::Memory --
//		ブロックのバッファリング内容を表すクラスのデフォルトコンストラクター
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
Block::Memory::Memory()
	: _id(IllegalID)
{}

//	FUNCTION public
//	Version::Block::Memory::~Memory --
//		ブロックのバッファリング内容を表すクラスのデストラクター
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
Block::Memory::~Memory()
{
	unfix();
}

//	FUNCTION public
//	Version::Block::Memory::operator void* -- void* へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ブロックのバッファリング内容を格納する自由記憶領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

inline
Block::Memory::operator void*()
{
	return operator char*();
}

//	FUNCTION public
//	Version::Block::Memory::operator const void* --
//		const void* へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ブロックのバッファリング内容を格納する自由記憶領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

inline
Block::Memory::operator const void*() const
{
	return operator const char*();
}

//	FUNCTION public
//	Version::Block::Memory::operator char* -- char* へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ブロックのバッファリング内容を格納する自由記憶領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

inline
Block::Memory::operator char*()
{
	return static_cast<Buffer::Memory*>(this)->operator char*() +
		sizeof(Header);
}

//	FUNCTION public
//	Version::Block::Memory::operator const char* --
//		const char* へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ブロックのバッファリング内容を格納する自由記憶領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

inline
Block::Memory::operator const char*() const
{
	return static_cast<const Buffer::Memory*>(this)->operator const char*() +
		sizeof(Header);
}

//	FUNCTION public
//	Version::Block::Memory::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::Memory&	src
//			自分自身へ代入するブロックのバッファリング内容
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		なし

inline
Block::Memory&
Block::Memory::operator =(const Memory& src)
{
	if (this != &src) {

		// まず、自分自身をアンフィックスする

		unfix();

		// 親クラスの代入

		static_cast<Buffer::Memory*>(this)->operator =(src);

		// 自メンバの代入

		_id = src.getID();
	}
	return *this;
}

//	FUNCTION public
//	Version::Block::Memory::operator == -- == 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::Memory&		r
//			自分自身と比較するバッファリング内容をもつブロック
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
Block::Memory::operator ==(const Memory& r) const
{
	return static_cast<const Buffer::Memory*>(this)->operator ==(
		static_cast<const Buffer::Memory&>(r));
}

//	FUNCTION public
//	Version::Block::Memory::operator != -- != 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::Memory&		r
//			自分自身と比較するバッファリング内容をもつブロック
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
Block::Memory::operator !=(const Memory& r) const
{
	return static_cast<const Buffer::Memory*>(this)->operator !=(
		static_cast<const Buffer::Memory&>(r));
}

//	FUNCTION public
//	Version::Block::Memory::refix -- 再フィックスする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		再フィックスされたバッファリング内容
//
//	EXCEPTIONS

inline
Block::Memory
Block::Memory::refix() const
{
	return Block::Memory(
		getID(), static_cast<const Buffer::Memory*>(this)->refix());
}

//	FUNCTION public
//	Version::Block::Memory::getSize --
//		ブロックのバッファリング内容を格納する領域のサイズを得る
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
Block::Memory::getSize() const
{
	return Block::getContentSize(getPageSize());
}

//	FUNCTION public
//	Version::Block::Memory::getID -- ブロックのブロック識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたブロック識別子
//
//	EXCEPTIONS
//		なし

inline
Block::ID
Block::Memory::getID() const
{
	return _id;
}

//	FUNCTION public
//	Version::Block::Memory::getHeader -- ブロックのヘッダを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたヘッダを表すクラスのレファレンス
//
//	EXCEPTIONS
//		なし

inline
const Block::Header&
Block::Memory::getHeader() const
{
	return *static_cast<const Header*>(
		static_cast<const Buffer::Memory*>(this)->operator const void*());
}

//	FUNCTION public
//	Version::Block::Memory::getLastModification --
//		ブロックの最終更新時タイムスタンプを得る
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
Block::Memory::getLastModification() const
{
	return getHeader()._lastModification;
}

_SYDNEY_VERSION_END
_SYDNEY_END

#endif	// __SYDNEY_VERSION_BLOCK_H

//
// Copyright (c) 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
