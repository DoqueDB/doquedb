// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Memory.cpp -- バッファメモリ関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
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

#include "Buffer/Memory.h"
#include "Buffer/Page.h"

#include "Common/Assert.h"
#include "Common/Thread.h"
#include "Os/AutoCriticalSection.h"

#include "Exception/MemoryExhaust.h"

_SYDNEY_USING
_SYDNEY_BUFFER_USING

namespace 
{
}

//	FUNCTION private
//	Buffer::Memory::Memory -- バッファメモリを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Memory::Category::Value	category
//			バッファメモリの種別で、
//			Buffer::Memory::Category::Value の論理和を指定する
//		Buffer::Page&		page
//			生成するクラスの表すフィックス済みのバッファメモリを使って
//			OS ファイル領域をバッファリングする
//			バッファページのバッファページ記述子
//		bool				dirty
//			true
//				バッファメモリは更新されたことになっている
//			false
//				バッファメモリは更新されたことになっていない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Memory::Memory(Category::Value category, Page& page, bool dirty)
	: _category(category),
	  _owner(true),
	  _dirty(dirty),
	  _page(&page)
{
	; _SYDNEY_ASSERT(isUpdatable() || !isDirty());
}

//	FUNCTION public
//	Buffer::Memory::operator char* -- char* へのキャスト演算子
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

Memory::operator char*()
{
	; _SYDNEY_ASSERT(isUpdatable());

	if (isOwner()) {

		// バッファメモリの所有権を持っている

		; _SYDNEY_ASSERT(_page && _page->getRefCount());

		void* p;

		// バッファページを保護するためにラッチする
	
		Os::AutoCriticalSection	latch(_page->getLatch());

		if (isDiscardable()) {
			if (!_page->_working && _page->_body) {
				const Os::Memory::Size pageSize = _page->getSize();
				{
				// バッファプールを保護するためにラッチする

				Os::AutoCriticalSection	latch(_page->_file._pool.getLatch());

				try
				{
					// 作業用メモリを確保する
					//
					//【注意】	バッファプールの上限に達しても例外を発生しない

					bool dummy;
					_page->_working =
						_page->_file._pool.allocateMemory(
							pageSize, false, dummy, true);
				}
				catch (Exception::MemoryExhaust&)
				{
					// エラー状況を解除する
					Common::Thread::resetErrorCondition();
				}

				if (_page->_working == 0)
				{
					// メモリーが確保できなかったので、
					// すでに確保した領域から探す
					//
					//【注意】	SkipDirtyCandidateCountMaxは無視する

					bool dummy;
					_page->_working =
						_page->_file._pool.getCandidate(
							pageSize, false, dummy, true);
					if (_page->_working == 0)
					{
						// 確保できなったのであきらめる
						//
						// 本当はダーティなページをフラッシュしてから
						// もう一度トライしたいが...
						
						_SYDNEY_THROW0(Exception::MemoryExhaust);
					}
				}
					
				; _SYDNEY_ASSERT(_page->_working);
				}
				// 確保した作業用メモリをバッファメモリの内容で初期化する

				(void) Os::Memory::copy(
					_page->_working, _page->_body, pageSize);
			}

			p = _page->_working;
		} else
			p = _page->_body;

		; _SYDNEY_ASSERT(p);
		return static_cast<char*>(p) + sizeof(Page::Header);
	}

	return 0;
}

//	FUNCTION public
//	Buffer::Memory::operator const char* -- const char* へのキャスト演算子
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

Memory::operator const char*() const
{
	if (isOwner()) {

		// バッファメモリの所有権を持っている

		; _SYDNEY_ASSERT(_page && _page->getRefCount());

		// バッファページを保護するためにラッチする
	
		Os::AutoCriticalSection	latch(_page->getLatch());

		return static_cast<char*>(
			(isDiscardable() && _page->_working) ?
			_page->_working : _page->_body) + sizeof(Page::Header);
	}

	return 0;
}

//	FUNCTION public
//	Buffer::Memory::unfix -- バッファメモリをアンフィックスし、所有権を放棄する
//
//	NOTES
//		Buffer::Page::FixMode::Discardable を与えずに
//		フィックスされたバッファメモリを更新したにもかかわらず、
//		引数 dirty に false を与えたときの動作は保証しない
//
//	ARGUMENTS
//		bool				dirty
//			true
//				フィックスしていたバッファメモリを更新した
//			false または指定されないとき
//				フィックスしていたバッファメモリを更新しなかった
//		bool				asynchronously
//			true または指定されないとき
//				必要があれば、バッファメモリの内容を
//				非同期的にディスクへ書き出す
//			false
//				必要があれば、バッファメモリの内容を
//				同期的にディスクへ書き出す
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Memory::unfix(bool dirty, bool asynchronously)
{
	; _SYDNEY_ASSERT(isUpdatable() || !dirty);

	if (isOwner()) {

		// バッファメモリの所有権を持っている

		; _SYDNEY_ASSERT(_page && _page->getRefCount());

		// バッファメモリをアンフィックスする

		_page->unfix(getCategory(),
					 !(dirty || isDirty()) ? Page::UnfixMode::None :
					 (asynchronously) ? Page::UnfixMode::Dirty :
										Page::UnfixMode::Flush);

		// バッファメモリの所有権を放棄する

		(void) release();
	}
}

//	FUNCTION public
//	Buffer::Memory::refix -- バッファメモリを再フィックスする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		再フィックスされたバッファメモリ
//
//	EXCEPTIONS

Memory
Memory::refix() const
{
	if (isOwner()) {

		// バッファメモリの所有権を持っている

		; _SYDNEY_ASSERT(_page && _page->getRefCount());

		// 同じモードでもう一度フィックスする
		//
		//【注意】	Page::FixMode::Allocate だけは
		//			Page::FixMode::Write に変換する

		Category::Value category = getCategory();
		if (category & Category::Allocate)
			category = category & ~Category::Allocate | Category::Write;

		return Page::fix(_page->_file, _page->getOffset(), category);
	}

	return Memory();
}

//	FUNCTION public
//	Buffer::Memory::touch -- バッファメモリのこれまでの更新内容を破棄不可にする
//
//	NOTES
//
//	ARGUMENTS
//		bool				dirty
//			true
//				フィックスしていたバッファメモリを更新した
//			false または指定されないとき
//				フィックスしていたバッファメモリを更新しなかった
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Memory::touch(bool dirty)
{
	if (isOwner() && isUpdatable()) {

		// バッファメモリの所有権を持ち、更新可である

		; _SYDNEY_ASSERT(_page && _page->getRefCount());

		if (dirty || isDirty()) {

			// これまでの更新内容を破棄できないようにする

			_page->touch(getCategory());

			// バッファメモリは更新していないことになる

			_dirty = false;
		}
	}
}

//	FUNCTION public
//	Buffer::Memory::discardable -- バッファメモリの更新内容を破棄可能にする
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
Memory::discardable()
{
	if (isOwner() && isUpdatable() && !isDiscardable()) {

		// バッファメモリの所有権を持ち、更新可であり、破棄可能でない

		if (isDirty()) {

			// バッファメモリはすでに更新されているので、
			// 更新内容を破棄できないようにする

			_page->touch(getCategory());

			// バッファメモリは更新していないことになる

			_dirty = false;
		}

		// バッファメモリの所有権を持ち、更新内容の破棄が不可なので、
		// フィックスしたときのモードを変更する

		_category |= Category::Discardable;
	}
}

//	FUNCTION public
//	Buffer::Memory::getSize -- バッファメモリのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバッファメモリのサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

Os::Memory::Size
Memory::getSize() const
{
	return Page::getContentSize(getPageSize());
}

//	FUNCTION public
//	Buffer::Memory::getPageSize --
//		バッファリングするバッファページのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバッファページのサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

Os::Memory::Size
Memory::getPageSize() const
{
	return (_page) ? _page->getSize() : 0;
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
