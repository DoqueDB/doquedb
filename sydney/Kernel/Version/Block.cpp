// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Block.cpp -- ブロック関連の関数定義
// 
// Copyright (c) 2000, 2001, 2023 Ricoh Company, Ltd.
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

#include "Version/Block.h"
#include "Version/Configuration.h"

#include "Buffer/Page.h"
#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_VERSION_USING

namespace
{

namespace _Block
{
	namespace _Memory
	{
		// ブロックを更新したことにする
		void				dirty(Buffer::Memory& bufMemory,
								  const Trans::TimeStamp& lastModification);
	}
}

//	FUNCTION
//	$$$::_Block::_Memory::dirty -- ブロックを更新したことにする
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Memory&		bufMemory
//			ブロックのバッファリング内容を格納するバッファメモリ
//		Trans::TimeStamp&	lastModification
//			ブロックの最終更新時タイムスタンプ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
_Block::_Memory::dirty(Buffer::Memory& bufMemory,
					   const Trans::TimeStamp& lastModification)
{
	; _SYDNEY_ASSERT(bufMemory.isOwner());

	// ブロックの先頭のブロックヘッダに
	// 与えられた最終更新時タイムスタンプを記録する

	Block::Header* header =
		static_cast<Block::Header*>(bufMemory.operator void*());

	header->_lastModification = lastModification;
}

}

//	FUNCTION public
//	Version::Block::Memory::Memory --
//		ブロックのバッファリング内容を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::ID	id
//			ブロックのブロック識別子
//		Buffer::Memory&		bufMemory
//			ブロックを記録するバージョンページのバッファリング内容を表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Block::Memory::Memory(ID id, const Buffer::Memory& bufMemory)
	: Buffer::Memory(bufMemory),
	  _id(id)
{
	if (bufMemory.getCategory() & Buffer::Memory::Category::Allocate)

		// 最終更新時タイムスタンプを不正な値にしておく

		_Block::_Memory::dirty(*this, Trans::IllegalTimeStamp);
}

//	FUNCTION
//	Version::Block::getContentSize --
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

Os::Memory::Size
Block::getContentSize(Os::Memory::Size size)
{
	const Os::Memory::Size overhead = sizeof(Header);
	return ((size = Buffer::Page::getContentSize(size)) > overhead) ?
		size - overhead : 0;
}

//	FUNCTION public
//	Version::Block::Memory::copy -- バッファリング内容を上書きする
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::Memory&	src
//			自分自身のバッファリング内容へ上書きする
//			バッファリング内容をもつブロック
//
//	RETURN
//		上書き後の自分自身
//
//	EXCEPTIONS
//		なし

Block::Memory&
Block::Memory::copy(const Block::Memory& src)
{
	; _SYDNEY_ASSERT(isOwner() && src.isOwner());
	; _SYDNEY_ASSERT(getSize() == src.getSize());

	Os::Memory::copy(operator void*(), src.operator const void*(), getSize());

	return *this;
}

//	FUNCTION public
//	Version::Block::Memory::reset -- バッファリング内容を初期化する
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

Block::Memory&
Block::Memory::reset()
{
	; _SYDNEY_ASSERT(isOwner());

	Os::Memory::reset(operator void*(), getSize());
	return *this;
}

//	FUNCTION public
//	Version::Block::Memory::unfix --
//		ブロックのバッファリング内容をアンフィックスし、所有権を放棄する
//
//	NOTES
//		ブロックのバッファリング内容を更新したにもかかわらず、
//		引数 dirty に false を与えたときの動作は保証しない
//
//	ARGUMENTS
//		bool				dirty
//			true
//				ブロックのバッファリング内容を更新した
//			false または指定されないとき
//				ブロックのバッファリング内容を更新しなかった
//		bool				asynchronously
//			true または指定されないとき
//				必要があれば、ブロックのバッファリング内容を
//				非同期的にディスクへ書き出す
//			false
//				必要があれば、ブロックのバッファリング内容を
//				同期的にディスクへ書き出す
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Block::Memory::unfix(bool dirty, bool asynchronously)
{
	if (isOwner()) {

		// ブロックのバッファリング内容の所有権をもっている

		Buffer::Memory&	bufMemory = *static_cast<Buffer::Memory*>(this);

		if (dirty || isDirty())

			// 現時点のタイムスタンプを得て、
			// その時点にブロックを更新したことにする

			_Block::_Memory::dirty(bufMemory, Trans::TimeStamp::assign());

		// ブロックのバッファリング内容を格納する
		// バッファメモリをアンフィックスする

		bufMemory.unfix(dirty, asynchronously);
	}
}

//	FUNCTION public
//	Page::Block::Memory::unfix --
//		ブロックのバッファリング内容をアンフィックスし、所有権を放棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp&	t
//			ブロックを最後に更新したときに取得したタイムスタンプ
//		bool				asynchronously
//			true または指定されないとき
//				必要があれば、ブロックのバッファリング内容を
//				非同期的にディスクへ書き出す
//			false
//				必要があれば、ブロックのバッファリング内容を
//				同期的にディスクへ書き出す
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Block::Memory::unfix(const Trans::TimeStamp& t, bool asynchronously)
{
	if (isOwner()) {

		// ブロックのバッファリング内容の所有権をもっている

		Buffer::Memory&	bufMemory = *static_cast<Buffer::Memory*>(this);

		// 指定されたタイムスタンプの時点にブロックを更新したことにする

		_Block::_Memory::dirty(bufMemory, t);

		// ブロックのバッファリング内容を格納する
		// バッファメモリをアンフィックスする

		bufMemory.unfix(true, asynchronously);
	}
}

//	FUNCTION public
//	Version::Block::Memory::touch --
//		ブロックのこれまでの更新内容を破棄不可にする
//
//	NOTES
//
//	ARGUMENTS
//		bool				dirty
//			true
//				ブロックのバッファリング内容を更新した
//			false または指定されないとき
//				ブロックのバッファリング内容を更新しなかった
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Block::Memory::touch(bool dirty)
{
	if (isOwner() && (dirty || isDirty())) {

		// ブロックのバッファリング内容の所有権をもち、
		// バッファリング内容はダーティである

		Buffer::Memory&	bufMemory = *static_cast<Buffer::Memory*>(this);

		// 現時点のタイムスタンプを得て、
		// その時点にブロックを更新したことにする

		_Block::_Memory::dirty(bufMemory, Trans::TimeStamp::assign());

		// ブロックのバッファリング内容を格納する
		// バッファメモリのこれまでの更新内容を破棄不可にする

		bufMemory.touch();
	}
}

//	FUNCTION public
//	Version::Block::Memory::discardable -- ブロックの更新内容の破棄を可能にする
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
Block::Memory::discardable()
{
	if (isOwner() && !isDiscardable()) {

		// ブロックのバッファリング内容の所有権をもち、
		// 更新内容の破棄が不可である

		Buffer::Memory&	bufMemory = *static_cast<Buffer::Memory*>(this);

		if (isDirty())

			// 現時点のタイムスタンプを得て、
			// その時点にブロックを更新したことにする

			_Block::_Memory::dirty(bufMemory, Trans::TimeStamp::assign());

		// ブロックのバッファリング内容を格納する
		// バッファメモリの更新内容の破棄を可能にする

		bufMemory.discardable();
	}
}

//
// Copyright (c) 2000, 2001, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
