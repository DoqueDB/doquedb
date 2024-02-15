// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.h -- バッファファイル関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BUFFER_FILE_H
#define	__SYDNEY_BUFFER_FILE_H

#include "Buffer/Module.h"

#include "Os/CriticalSection.h"
#include "Os/File.h"
#include "Os/Memory.h"

#include "Common/DoubleLinkedList.h"

#include "ModTypes.h"

#ifdef DUMMY
#else
template <class T>
class ModAutoPointer;
#endif

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}

_SYDNEY_BUFFER_BEGIN

namespace Manager
{
	class File;
}

class Page;
class Pool;

//	CLASS
//	Buffer::File -- バッファファイル記述子を表すクラス
//
//	NOTES

class File
{
	friend class Memory;
#ifdef DUMMY
#else
	friend class ModAutoPointer<File>;
#endif
	friend class Manager::File;
	friend class Page;
	friend class Pool;
public:
	//	TYPEDEF
	//	Buffer::File::ID -- バッファファイル記述子を一意に識別する値を表す型
	//
	//	NOTES
	//		あるバッファファイルに対して記述子が生成され、
	//		それが破棄され、再生成されると、異なる値が割り振られるかもしれない

	typedef	ModUPtr			ID;

	// 記述子を生成する
	SYD_BUFFER_FUNCTION
	static File*
	attach(Pool& pool, const Os::Path& path, Os::Memory::Size pageSize,
		   bool mounted, bool readOnly, bool noCRC);
	// 記述子を破棄する
	SYD_BUFFER_FUNCTION
	static void
	detach(File*& file);

	// 生成する
	SYD_BUFFER_FUNCTION
	void
	create(bool overwrite);
	// 破棄する
	SYD_BUFFER_FUNCTION
	void
	destroy();
	// マウントする
	SYD_BUFFER_FUNCTION
	void
	mount(bool existing);
	// アンマウントする
	SYD_BUFFER_FUNCTION
	void
	unmount();
	// トランケートする
	SYD_BUFFER_FUNCTION
	void
	truncate(Os::File::Offset offset = 0);
	// 拡張する
	SYD_BUFFER_FUNCTION
	void
	extend(Os::File::Offset offset);
	// バッファリングする OS ファイルの名前を変更する
	SYD_BUFFER_FUNCTION
	void
	rename(const Os::Path& path);

	// オープンする
	SYD_BUFFER_FUNCTION
	void
	open();
	// クローズする
	SYD_BUFFER_FUNCTION
	void
	close();

	// ダーティなバッファページのバッファリング内容を書き込む
	SYD_BUFFER_FUNCTION
	void
	flush(Os::File::Offset offset = -1);

	// syncする
	SYD_BUFFER_FUNCTION
	void
	sync();

	// フラッシュを抑止する
	SYD_BUFFER_FUNCTION
	void
	startDeterrent();
	// フラッシュの抑止をやめる
	SYD_BUFFER_FUNCTION
	void
	endDeterrent();

	// 識別子を得る
	ID
	getID() const;
 	// バッファページのサイズを得る
	Os::Memory::Size
	getPageSize() const;
	// バッファプール記述子を得る
	const Pool&
	getPool() const;
	// ファイルの絶対パス名を得る
	const Os::Path&
	getPath() const;
	// ファイルサイズを得る
	SYD_BUFFER_FUNCTION
	Os::File::Size
	getSize() const;
	// バッファページ数を得る
	SYD_BUFFER_FUNCTION
	unsigned int
	getPageCount(const Trans::Transaction* trans);

	// フラッシュは抑止されているか
	SYD_BUFFER_FUNCTION
	bool
	isDeterred() const;
	// 構成する OS ファイルが存在するか調べる
	SYD_BUFFER_FUNCTION
	bool
	isAccessible(bool force = false) const;
	// マウントされているか
	SYD_BUFFER_FUNCTION
	bool
	isMounted() const;
	// マウントされかつ構成する OS ファイルが存在するか
	SYD_BUFFER_FUNCTION
	bool
	isMountedAndAccessible() const;
	// 読取専用か
	bool
	isReadOnly() const;

	//
	//	【注意】
	//	ディスクリプターはプロセスごとに利用できる上限が決まっている
	//	特にUNIX系では数が少ないので、その上限管理を行う必要がある
	//	ディスクリプターはソケットでも消費されるので、
	//	ソケットを作成する場合、以下の関数を利用すること
	//
	// ディスクリプターを1つ予約する
	static bool
	reserveDescriptor();
	// ディスクリプターを1つ返却する
	static void
	returnDescriptor();
	// オープンしているすべてのファイルに対してsyncを実行する
	static void
	syncAllFiles();

private:
	//	CLASS
	//	Buffer::File::Accessibility --
	//		実体である OS ファイルの生成状態を表すクラス
	//
	//	NOTES

	struct Accessibility
	{
		//	ENUM
		//	Buffer::File::Accessibility::Value --
		//		実体である OS ファイルの生成状態を表す値の列挙型
		//
		//	NOTES

		typedef unsigned char	Value;
		enum
		{
			// 生成されていない
			None =			0,
			// 生成されたが永続化されていない
			Volatile,
			// 生成され、永続化された
			Persisted,
			// 値の数
			Count,
			// 不明
			Unknown =		Count
		};
	};

	// コンストラクター
	File(Pool& pool, const Os::Path& path,
		 Os::Memory::Size pageSize, bool mounted, bool readOnly, bool noCRC);
	// デストラクター
	~File();

	// 参照数を 1 増やす
	File*
	attach();
	// 参照数を 1 減らす
	void
	detach();
	// 参照数を得る
	unsigned int
	getRefCount() const;

	// バッファリングする OS ファイルをオープンする
	bool
	openOsFile();
	bool
	openOsFile(Os::File::OpenMode::Value mode,
			   Os::File::Permission::Value permission =
					Os::File::Permission::OwnerRead |
					Os::File::Permission::OwnerWrite);
	// バッファリングする OS ファイルをクローズする
	void
	closeOsFile();

	// バッファリングする OS ファイルを読み込む
	void
	readOsFile(Os::File::IOBuffer& buf,
			   Os::Memory::Size size, Os::File::Offset offset);
	void
	readOsFile(Os::File::IOBuffer* bufs, Os::Memory::Size size,
			   unsigned int count, Os::File::Offset offset);
	// バッファリングする OS ファイルを書き出す
	void
	writeOsFile(const Os::File::IOBuffer& buf,
				Os::Memory::Size size, Os::File::Offset offset);
	void
	writeOsFile(const Os::File::IOBuffer* bufs, Os::Memory::Size size,
				unsigned int count, Os::File::Offset offset);

	// バッファリングする OS ファイルの生成を遅延するか
	bool
	isCreationDelayed() const;

	// 排他制御用のラッチを得る
	Os::CriticalSection&
	getLatch() const;

	// クローズできるファイルを1つクローズする
	static bool
	closeFile();

	// 排他制御用のラッチ
	mutable Os::CriticalSection	_latch;

	// バッファページのサイズ(B 単位)
	const Os::Memory::Size	_pageSize;
	// バッファファイルを管理するバッファプール
	Pool&					_pool;
	// バッファリングする OS ファイル
	Os::File				_osFile;
	// バッファリングする OS ファイルのファイルサイズ(B 単位)
	mutable Os::File::Size	_size;
	// 参照回数
	//【注意】	$$$::_File::_latch で保護される
	mutable unsigned int	_refCount;
	// バッファページの抑止の開始の入れ子数
	unsigned int			_deterrentCount;
	// 実体である OS ファイルが生成されているか
	mutable Accessibility::Value _accessible;
	// マウントされているか
	bool					_mounted;
	// 読取専用か
	const bool				_readOnly;
	// バッファファイルの読み出し時に、その内容が正しく格納されているかを
	// CRC を使って検証しないか
	const bool				_noCRC;
	// writeされたかどうか
	bool					_writen;

	// LRUリストに登録されているバッファページのリスト
	Common::DoubleLinkedList<Page>	_pageList;

	// 格納されているハッシュリストのバケットアドレス
	//【注意】	$$$::_File::_latch で保護される
	unsigned int			_hashAddr;
	// ハッシュリストでの直前の要素へのポインタ
	//【注意】	$$$::_File::_latch で保護される
	File*					_hashPrev;
	// ハッシュリストでの直後の要素へのポインタ
	//【注意】	$$$::_File::_latch で保護される
	File*					_hashNext;

	// オープン中リストでの直前の要素へのポインタ
	//【注意】	$$$::_File::_openList->getLatch() で保護される
	File*					_openedPrev;
	// オープン中リストでの直後の要素へのポインタ
	//【注意】	$$$::_File::_openList->getLatch() で保護される
	File*					_openedNext;
};

//	FUNCTION private
//	Buffer::File::~File -- バッファファイル記述子を表すクラスのデストラクター
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
File::~File()
{}

//	FUNCTION public
//	Buffer::File::getID -- バッファファイル識別子を得る
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
File::getID() const
{
	return syd_reinterpret_cast<ID>(this);
}

//	FUNCTION public
//	Buffer::File::getPageSize --
//		バッファファイルの持つバッファページのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		バッファページのサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

inline
Os::Memory::Size
File::getPageSize() const
{
	return _pageSize;
}

//	FUNCTION public
//	Buffer::File::getPool --
//		バッファファイルを管理するバッファプールのバッファプール記述子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバッファプール記述子
//
//	EXCEPTIONS
//		なし

inline
const Pool&
File::getPool() const
{
	return _pool;
}

//	FUNCTION public
//	Buffer::File::getPath --
//		バッファファイルがバッファリングする OS ファイルの絶対パス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた絶対パス名
//
//	EXCEPTIONS

inline
const Os::Path&
File::getPath() const
{
	return _osFile.getPath();
}

//	FUNCTION private
//	Buffer::File::getLatch --
//		バッファファイルの操作の排他制御をするためのラッチを得る
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
File::getLatch() const
{
	return _latch;
}

//	FUNCTION public
//	Buffer::File::isReadOnly -- バッファファイルは読取専用か
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			読取専用である
//		false
//			読取専用でない
//
//	EXCEPTIONS
//		なし

inline
bool
File::isReadOnly() const
{
	return _readOnly;
}

_SYDNEY_BUFFER_END
_SYDNEY_END

#endif	// __SYDNEY_BUFFER_FILE_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
