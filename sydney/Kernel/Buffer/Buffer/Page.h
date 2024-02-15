// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Page.h -- バッファページ関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BUFFER_PAGE_H
#define	__SYDNEY_BUFFER_PAGE_H

#include "Buffer/Module.h"
#include "Buffer/File.h"
#include "Buffer/HashTable.h"
#include "Buffer/Memory.h"
#include "Buffer/Pool.h"

#ifdef DUMMY
#else
#include "Buffer/ReplacementPriority.h"
#endif

#include "Common/CRC.h"
#include "Os/RWLock.h"

template <class T>
class ModAutoPointer;

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}

_SYDNEY_BUFFER_BEGIN

namespace Manager
{
	class Page;
}

//	CLASS
//	Buffer::Page -- バッファページ記述子を表すクラス
//
//	NOTES

class Page
{
	friend class AutoPage;
	friend class File;
	friend class Manager::Page;
	friend class Memory;
	friend class ModAutoPointer<Page>;
	friend class Pool;
	friend class Pool::FlushablePageFilter::ForFile;
	friend class Pool::FlushablePageFilter::ForPool;
	friend class Pool::DirtyList;
	friend class Pool::LruList;
	
public:
	struct FixMode
	{
		//	ENUM
		//	Buffer::Page::FixMode::Value --
		//		フィックスする OS ファイル領域をどう使用するかを表す値の型
		//
		//	NOTES

		typedef	Memory::Category::Value	Value;
		enum
		{
			// 不明である
			Unknown =		Memory::Category::Unknown,
			// 読み込みのみ
			ReadOnly =		Memory::Category::ReadOnly,
			// 読み書きのため
			Write =			Memory::Category::Write,
			// ページ確保のため
			Allocate =		Memory::Category::Allocate,

			// フラッシュが抑制され得る
			Deterrentable =	Memory::Category::Deterrentable,
			// 更新は作業用メモリを介して行う
			Discardable =	Memory::Category::Discardable,
			// 読み取り書き込みロックを行わない
			NoLock =		Memory::Category::NoLock,

			// マスク
			Mask =			Memory::Category::Mask
		};
	};

	// バッファリングされたある OS ファイル領域をフィックスする
#ifdef DUMMY
#else
	static Memory
	fix(File& file, Os::File::Offset offset, FixMode::Value mode,
		ReplacementPriority::Value priority,
		const Trans::Transaction* trans) {
		return fix(file, offset, mode, trans);
	}
#endif
	static Memory
	fix(File& file,	Os::File::Offset offset, FixMode::Value mode) {
		return fix(file, offset, mode, 0);
	}
	SYD_BUFFER_FUNCTION
	static Memory
	fix(File& file,	Os::File::Offset offset, FixMode::Value mode,
		const Trans::Transaction* trans);

	// ページサイズを検算する
	SYD_BUFFER_FUNCTION
	static Os::Memory::Size
	correctSize(Os::Memory::Size size);

	// バッファページを持つバッファファイルの識別子を得る
	File::ID
	getFileID() const;
	// 読み込む OS ファイル領域のファイルの先頭からのオフセットを得る
	Os::File::Offset
	getOffset() const;
	// 格納可能な内容のサイズを得る
	SYD_BUFFER_FUNCTION
	static Os::Memory::Size
	getContentSize(Os::Memory::Size size);

private:
	struct UnfixMode
	{
		//	ENUM
		//	Buffer::Page::UnfixMode::Value --
		//		アンフィックスする OS ファイル領域をどうするかを表す値の型
		//
		//	NOTES

		typedef unsigned char	Value;
		enum
		{
			// ダーティにしなかったので、なにもしない
			None =			0,
			// ダーティにしたので、後でフラッシュする必要がある
			Dirty,
			// ダーティにしたので、すぐフラッシュする
			Flush,
			// 値の数
			Count,
			// 不明
			Unknown =		Count
		};
	};

	struct Status
	{
		//	ENUM
		//	Buffer::Page::Status::Value -- バッファページの状態を表す値の型
		//
		//	NOTES

		typedef unsigned char	Value;
		enum Bit
		{
			// バッファメモリが未確保
			Empty =			0x00,
			// バッファメモリを確保済、OS ファイルを読み込んでいない
			NoRead =		0x01,
			// 保持している内容と OS ファイルは一致している
			Read =			0x02,
			// 保持している内容と OS ファイルは一致し、CRCもチェック済み
			Normal =		0x04,
			// 保持している内容が修正され、OS ファイルと一致しない
			Dirty =			0x08,

			// ダーティリストに載っている
			Flushable =		0x10,
			// 次のチェックポイント処理までに
			// フラッシュしなければならないとしてマークされている
			Marked =		0x20,
			// 必要に応じてフラッシュが抑制され得る
			Deterrentable =	0x40,

			// マスク
			Mask =			0x77
		};
	};

	//	CLASS
	//	Buffer::Page::Header -- バッファページのヘッダを表すクラス
	//
	//	NOTES

	struct Header
	{
		// バッファページ全体に対して CRC を計算しているか
		bool					_calculated;
		// ヘッダの上記メンバの部分から計算した 16 ビット CRC
		Common::CRC16::Value	_crc;
	};

	//	CLASS
	//	Buffer::Page::Footer -- バッファページのフッタを表すクラス
	//
	//	NOTES

	struct Footer
	{
		// バッファページのフッタ以外の部分から計算した 32 ビット CRC
		Common::CRC32::Value	_crc;
	};

	// コンストラクター
	Page(File& file, Os::File::Offset offset);
	// デストラクター
	~Page();

	// 記述子を生成する
	static Page*
	attach(File& file, Os::File::Offset offset);
	// 参照数を 1 増やす
	Page*
	attach();
	// 記述子を破棄する
	static void
	detach(Page*& page, bool reserve);
	// 参照数を 1 減らす
	void
	detach();
	// 参照数を得る
	unsigned int
	getRefCount() const;

	// 記述子を格納すべきハッシュ表のバケットを求める
	HashTable<Page>::Bucket&
	getBucket();
	const HashTable<Page>::Bucket&
	getBucket() const;
	static HashTable<Page>::Bucket&
	getBucket(File& file, Os::File::Offset offset);

	// バッファリングしている OS ファイル領域をアンフィックスする
	void
	unfix(Memory::Category::Value category, UnfixMode::Value mode);
	// これまでの更新内容を破棄不可にする
	void
	touch(Memory::Category::Value category);

	// 読み出したバッファリング内容が書き込み時と
	// 同じものかを検証するための CRC を計算する
	void
	calculateCRC();
	// バッファリング内容が正しく格納され、読み出されたかを確認する
	void
	verify() const;

	// サイズを得る
	Os::Memory::Size
	getSize() const;

	// 排他制御用のラッチを得る
	Os::CriticalSection&
	getLatch() const;
	// バッファメモリの排他制御用の読み取り書き込みロックを得る
	Os::RWLock&
	getRWLock() const;

	// OS ファイルから読み込む
	static void
	readAhead(File& file, Page& page, Os::File::Offset offset);

	// 排他制御用のラッチ
	mutable	Os::CriticalSection	_latch;
	// バッファメモリの排他制御用の読み取り書き込みロック
	mutable Os::RWLock		_rwlock;

	// バッファページを持つバッファファイルのバッファファイル記述子
	File&					_file;
	// バッファリングする OS ファイル領域の
	// ファイルの先頭からのオフセット(B 単位)
	const Os::File::Offset	_offset;
	// 参照数
	//【注意】	$$$::_Page::_pageTable->getBucket(i).getLatch() で保護される
	mutable unsigned short	_refCount;
	// バッファページの状態
	Status::Value			_status;
	// バッファページが登録されている LRU リストの種別
	//【注意】	_file._pool.getLatch() で保護される
	Pool::LruList::Category::Value	_lruListCategory;

	// バッファメモリ、すなわち、
	// 読み込んだ OS ファイル領域を記録する領域の先頭アドレス
	void*					_body;
	// 作業用のバッファメモリ
	void*					_working;
	
	// ハッシュリストでの直前の要素へのポインタ
	//【注意】	$$$::_Page::_pageTable->getBucket(i).getLatch() で保護される
	Page*					_hashPrev;
	// ハッシュリストでの直後の要素へのポインタ
	//【注意】	$$$::_Page::_pageTable->getBucket(i).getLatch() で保護される
	Page*					_hashNext;
	// LRU リスト(B1, B2, T1, T2)での直前の要素へのポインタ
	//【注意】	_file._pool.getLatch() で保護される
	Page*					_lruPrev;
	// LRU リスト(B1, B2, T1, T2)での直後の要素へのポインタ
	//【注意】	_file._pool.getLatch() で保護される
	Page*					_lruNext;
	// ファイル内リストでの直前の要素へのポインタ
	//【注意】	_file._pool.getLatch() で保護される
	Page*					_filePrev;
	// ファイル内リストでの直後の要素へのポインタ
	//【注意】	_file._pool.getLatch() で保護される
	Page*					_fileNext;
};

//	FUNCTION private
//	Buffer::Page::Page -- バッファページ記述子を表すクラスのコンストラクター
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
//		なし
//
//	EXCEPTIONS
//		なし

inline
Page::Page(File& file, Os::File::Offset offset)
	: _file(file)
	, _offset(offset)
	, _refCount(0)
	, _status(Status::Empty)
	, _lruListCategory(Pool::LruList::Category::Unknown)
	, _body(0)
	, _working(0)
	, _hashPrev(0)
	, _hashNext(0)
	, _lruPrev(0)
	, _lruNext(0)
	, _filePrev(0)
	, _fileNext(0)
{}

//	FUNCTION private
//	Buffer::Page::~Page -- バッファページ記述子の表すクラスのデストラクター
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
Page::~Page()
{}

//	FUNCTION private
//	Buffer::Page::getBucket --
//		バッファページ記述子を格納すべきハッシュ表のバケットを求める
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバケット
//
//	EXCEPTIONS
//		なし

inline
HashTable<Page>::Bucket&
Page::getBucket()
{
	return getBucket(_file, getOffset());
}

inline
const HashTable<Page>::Bucket&
Page::getBucket() const
{
	return getBucket(_file, getOffset());
}

//	FUNCTION public
//	Buffer::Page::getFileID --
//		読み込む OS ファイルのバッファファイルの識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバッファファイル識別子
//
//	EXCEPTIONS
//		なし

inline
File::ID
Page::getFileID() const
{
	return _file.getID();
}

//	FUNCTION public
//	Buffer::Page::getOffset --
//		読み込む OS ファイル領域のファイルの先頭からのオフセットを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたオフセット(B 単位)
//
//	EXCEPTIONS
//		なし

inline
Os::File::Offset
Page::getOffset() const
{
	return _offset;
}

//	FUNCTION private
//	Buffer::Page::getSize -- サイズを得る
//
//	NOTES
//		バッファページのサイズとは、
//		バッファページが占有する OS ファイル領域のサイズであり、
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

inline
Os::Memory::Size
Page::getSize() const
{
	return _file.getPageSize();
}

//	FUNCTION private
//	Buffer::Page::getLatch --
//		バッファページの操作の排他制御をするためのラッチを得る
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

//	FUNCTION private
//	Buffer::Page::getRWLock --
//		バッファメモリの操作の排他制御をするための読み取り書き込みロックを得る
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
Page::getRWLock() const
{
	return _rwlock;
}

_SYDNEY_BUFFER_END
_SYDNEY_END

#endif	// __SYDNEY_BUFFER_PAGE_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
