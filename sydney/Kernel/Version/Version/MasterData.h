// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MasterData.h -- マスタデータファイル関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VERSION_MASTERDATA_H
#define	__SYDNEY_VERSION_MASTERDATA_H

#include "Version/Module.h"
#include "Version/File.h"
#include "Version/VersionLog.h"

#include "Buffer/File.h"
#include "Buffer/Page.h"
#include "Buffer/Pool.h"
#include "Buffer/ReplacementPriority.h"
#include "Common/Object.h"
#include "Os/File.h"
#include "Os/Memory.h"
#include "Admin/Verification.h"

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}

_SYDNEY_VERSION_BEGIN

namespace SyncLog
{
	class File;
}
class Verification;

_SYDNEY_VERSION_MASTERDATA_BEGIN

//	CLASS
//	Version::MasterData::File -- マスタデータファイルを表すクラス
//
//	NOTES

class File
{
	friend class Data;
public:
	//	TYPEDEF
	//	Version::MasterData::File::ID --
	//		マスタデータファイルを表すクラスを一意に識別する値を表す型
	//
	//	NOTES

	typedef Buffer::File::ID	ID;

	// コンストラクター
	File(const Version::File::StorageStrategy& storageStrategy,
		 const Version::File::BufferingStrategy& bufferingStrategy); 
	// デストラクター
	~File();

	// 生成する
	void					create();
	// 破棄する
	void					destroy();
	// マウントする
	void					mount();
	// アンマウントする
	void					unmount();
	// トランケートする
	void					truncate(Block::ID id = 0);
	// 移動する
	void					move(const Os::Path& path);

	// フラッシュする
	void					flush();

	// ある時点以降に確保されたブロックをなくす
	bool
	restore(const Trans::Transaction& trans, const Trans::TimeStamp& point);
	// ある時点に障害回復する
	bool
	recover(const Trans::Transaction& trans,
			const Trans::TimeStamp& point, SyncLog::File& syncLog);
	// ある数のデータブロックを持つように障害回復する
	void
	recover(const Trans::Transaction& trans,
			unsigned int pageCount, SyncLog::File& syncLog);

	// 整合性検査の開始を指示する
	void
	startVerification(Verification& verification,
					  Admin::Verification::Progress& result);

	// あるバージョンページの同期ログをデータブロックへ複写する
	Block::Memory
	syncData(const Trans::Transaction& trans, const Block::Memory& src);
	// あるバージョンページのバージョンログをデータブロックへ複写する
	Block::Memory
	syncData(const Trans::Transaction& trans,
			 Block::ID id, const Block::Memory& src,
			 Trans::TimeStamp::Value allocation);

	// 識別子を得る
	ID						getID() const;
	// ファイルサイズを得る
	Os::File::Size			getSize() const;
	// ファイルの使用中部分のサイズを得る
	Os::File::Size			getBoundSize(const Trans::Transaction& trans);
	// ブロックサイズを得る
	Os::Memory::Size		getBlockSize() const;
	// 格納する OS ファイルの親ディレクトリの絶対パス名を得る
	const Os::Path&			getParent() const;
	// 最大ファイルサイズを得る
	Os::File::Size			getSizeMax() const;
	// ファイル拡張サイズを得る
	Os::File::Size			getExtensionSize() const;
	// バッファリングするバッファプールの種別を得る
	Buffer::Pool::Category::Value getPoolCategory() const;
	// ブロック数を得る
	unsigned int			getBlockCount(const Trans::Transaction& trans);

	// 構成する OS ファイルが存在するか調べる
	bool
	isAccessible(bool force = false) const;
	// マウントされているか調べる
	bool
	isMounted() const;
	// マウントされかつ構成する OS ファイルが存在するか調べる
	bool
	isMountedAndAccessible() const;
	// 読取専用か調べる
	bool
	isReadOnly() const;

private:
	// 拡張する
	void
	extend(Block::ID id);

	// 実体であるバッファファイル
	Buffer::File*			_bufFile;
	// 格納する OS ファイルの親ディレクトリの絶対パス名
	Os::Path				_parent;
	// 最大ファイルサイズ(B 単位)
	const Os::File::Size	_sizeMax;
	// ファイル拡張サイズ(B 単位)
	const Os::File::Size	_extensionSize;
};

//	CLASS
//	Version::MasterData::Data --
//		マスタデータファイルのデータブロックを表すクラス
//
//	NOTES

class Data
	: public	VersionLog::Log
{
	friend class File;
public:
	// 確保する
	static Block::Memory
	allocate(const Trans::Transaction& trans,
			 File& file, Block::ID id,
			 Buffer::ReplacementPriority::Value priority);
	// フィックスする
	static Block::Memory
	fix(const Trans::Transaction& trans,
		File& file, Block::ID id, Buffer::Page::FixMode::Value mode,
		Buffer::ReplacementPriority::Value priority);

	// データブロックが格納されているブロックからデータブロックを得る
	static Data&
	get(Block::Memory& memory);
	static const Data&
	get(const Block::Memory& memory);

/*	VersionLog::Log
protected:
	// Block::IllegalID が必ず格納される
	Block::ID				_older;
	// Block::IllegalID が必ず格納される
	Block::ID				_physicalLog;
	// データブロックを確保したときのタイムスタンプ値
	Trans::TimeStamp::Value	_olderTimeStamp;
	// Category::Oldest が必ず格納される
	Category::Value			_category;
*/
};

//	FUNCTION public
//	Version::MasterData::File::~File --
//		マスタデータファイル記述子を表すクラスのデストラクター
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
File::~File()
{
	// マスタデータファイルの実体である
	// バッファファイルのバッファファイル記述子を破棄する

	Buffer::File::detach(_bufFile);
}

//	FUNCTION public
//	Version::MasterData::File::create -- 生成する
//
//	NOTES
//		すでに生成されているマスタデータファイルを生成してもエラーにならない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
File::create()
{
	// マスタデータファイルの実体である
	// バッファファイルを生成する

	_bufFile->create(false);
}

//	FUNCTION public
//	Version::MasterData::File::destroy -- 破棄する
//
//	NOTES
//		マスタデータファイルが生成されていなくても例外は発生しない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
File::destroy()
{
	// マスタデータファイルの実体である
	// バッファファイルを破棄する

	_bufFile->destroy();
}

//	FUNCTION public
//	Version::MasterData::File::mount -- マウントする
//
//	NOTES
//		すでにマウントされている
//		マスタデータファイルをマウントしてもエラーにならない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
File::mount()
{
	// マスタデータファイルの実体である
	// バッファファイルをマウントする
	//
	//【注意】	マウントするバッファファイルの実体である
	//			OS ファイルが存在しなくても、例外を発生しない

	_bufFile->mount(false);
}

//	FUNCTION public
//	Version::MasterData::File::unmount -- アンマウントする
//
//	NOTES
//		マウントされていない
//		マスタデータファイルをアンマウントしてもエラーにならない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
File::unmount()
{
	// マスタデータファイルの実体である
	// バッファファイルをアンマウントする

	_bufFile->unmount();
}

//	FUNCTION public
//	Version::MasterData::File::flush -- フラッシュする
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
void
File::flush()
{
	// マスタデータファイルの実体である
	// バッファファイルをフラッシュする

	_bufFile->flush();
}

//	FUNCTION public
//	Version::MasterData::File::syncData --
//		あるバージョンページの同期ログによって
//		リカバリするためにデータブロックへ複写する
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			処理を行うトランザクションのトランザクション記述子
//		Version::Block::Memory&	src
//			データブロックに複写する
//			あるベージョンページの同期ログのバッファリング内容
//
//	RETURN
//		複写後のデータブロックのバッファリング内容
//
//	EXCEPTIONS

inline
Block::Memory
File::syncData(const Trans::Transaction& trans, const Block::Memory& src)
{
	const Data& srcLog = Data::get(src);
	return syncData(trans, srcLog._older, src, srcLog._olderTimeStamp);
}

//	FUNCTION public
//	Version::MasterData::File::getID -- マスタデータファイル識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたマスタデータファイル識別子
//
//	EXCEPTIONS
//		なし

inline
File::ID
File::getID() const
{
	return _bufFile->getID();
}

//	FUNCTION public
//	Version::MasterData::File::getSize --
//		マスタデータファイルのファイルサイズを得る
//
//	NOTES
//		マスタデータファイルが生成されていないとき、ファイルサイズは 0 とみなす
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイルサイズ(B 単位)
//
//	EXCEPTIONS

inline
Os::File::Size
File::getSize() const
{
	return _bufFile->getSize();
}

//	FUNCTION public
//	Version::MasterData::File::getBoundSize --
//		使用中のブロックの総サイズを求める
//
//	NOTES
//		マスタデータファイルが生成されていないとき、
//		使用中のブロックの総サイズは 0 とみなす
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求めたサイズ(B 単位)
//
//	EXCEPTIONS

inline
Os::File::Size
File::getBoundSize(const Trans::Transaction& trans)
{
	return static_cast<Os::File::Size>(getBlockSize()) * getBlockCount(trans);
}

//	FUNCTION public
//	Version::MasterData::File::getBlockSize --
//		マスタデータファイルのブロックサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたブロックサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

inline
Os::Memory::Size
File::getBlockSize() const
{
	return _bufFile->getPageSize();
}

//	FUNCTION public
//	Version::MasterData::getParent --
//		マスタデータファイルを格納する OS ファイルの
//		親ディレクトリの絶対パス名を得る
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
//		なし

inline
const Os::Path&
File::getParent() const
{
	return _parent;
}

//	FUNCTION public
//	Version::MasterData::File::getSizeMax --
//		マスタデータファイルの最大ファイルサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた最大ファイルサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

inline
Os::File::Size
File::getSizeMax() const
{
	return _sizeMax;
}

//	FUNCTION public
//	Version::MasterData::File::getExtensionSize --
//		マスタデータファイルのファイル拡張サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイル拡張サイズ(B 単位)
//
//	EXCEPTIONS
//		なし

inline
Os::File::Size
File::getExtensionSize() const
{
	return _extensionSize;
}

//	FUNCTION public
//	Version::MasterData::File::getPoolCategory --
//		バッファリングするバッファプールの種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバッファプールの種別
//
//	EXCEPTIONS
//		なし

inline
Buffer::Pool::Category::Value
File::getPoolCategory() const
{
	return _bufFile->getPool().getCategory();
}

//	FUNCTION public
//	Version::MasterData::File::getBlockCount --
//		マスタデータファイルの総ブロック数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた総ブロック数
//
//	EXCEPTIONS

inline
unsigned int
File::getBlockCount(const Trans::Transaction& trans)
{
	return _bufFile->getPageCount(&trans);
}

//	FUNCTION public
//	Version::MasterData::File::isAccessible --
//		マスタデータファイルを構成する OS ファイルが存在するか調べる
//
//	NOTES
//
//	ARGUMENTS
//		bool				force
//			true
//				マスタデータファイルの実体である
//				OS ファイルの存在を実際に調べる
//			false または指定されないとき
//				マスタデータファイルの実体である
//				OS ファイルの存在を必要があれば調べる
//
//	RETURN
//		true
//			存在する
//		false
//			存在しない
//
//	EXCEPTIONS

inline
bool
File::isAccessible(bool force) const
{
	return _bufFile->isAccessible(force);
}

//	FUNCTION public
//	Version::MasterData::File::isMounted -- マウントされているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			マウントされている
//		false
//			マウントされていない
//
//	EXCEPTIONS

inline
bool
File::isMounted() const
{
	return _bufFile->isMounted();
}

//	FUNCTION public
//	Version::MasterData::File::isMountedAndAccessible --
//		マウントされかつ構成する OS ファイルが存在するか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			マウントされかつ構成する OS ファイルが存在する
//		false
//			マウントされていないか、構成する OS ファイルが存在しない
//
//	EXCEPTIONS

inline
bool
File::isMountedAndAccessible() const
{
	return _bufFile->isMountedAndAccessible();
}

//	FUNCTION public
//	Version::MasterData::File::isReadOnly -- 読取専用か調べる
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
	return _bufFile->isReadOnly();
}

//	FUNCTION public
//	Version::MasterData::Data::get --
//		データブロックが格納されているブロックからデータブロックを得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::Memory&	memory
//			データブロックが格納されているブロックのバッファリング内容
//
//	RETURN
//		得られたデータブロックへのリファレンス
//
//	EXCEPTIONS
//		なし

// static
inline
Data&
Data::get(Block::Memory& memory)
{
	return *static_cast<Data*>(memory.operator void*());
}

// static
inline
const Data&
Data::get(const Block::Memory& memory)
{
	return *static_cast<const Data*>(memory.operator const void*());
}

_SYDNEY_VERSION_MASTERDATA_END
_SYDNEY_VERSION_END
_SYDNEY_END

#endif	// __SYDNEY_VERSION_MASTERDATA_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
