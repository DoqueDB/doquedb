// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SyncLog.h -- 同期ログファイル関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2009, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VERSION_SYNCLOG_H
#define	__SYDNEY_VERSION_SYNCLOG_H

#include "Version/Module.h"
#include "Version/File.h"
#include "Version/VersionLog.h"

#include "Buffer/File.h"
#include "Buffer/Pool.h"
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

namespace MasterData
{
	class File;
}

class Verification;

_SYDNEY_VERSION_SYNCLOG_BEGIN

class FileHeader;

//	CLASS
//	Version::SyncLog::File -- 同期ログファイルを表すクラス
//
//	NOTES

class File
{
	friend class FileHeader;
	friend class Log;
public:
	// コンストラクター
	File(const Version::File::StorageStrategy& storageStrategy,
		 const Version::File::BufferingStrategy& bufferingStrategy); 
	// デストラクター
	~File();

	// 生成する
	void					create(const Trans::Transaction& trans);
	// 破棄する
	void					destroy();
	// マウントする
	void					mount();
	// アンマウントする
	void					unmount();
#ifdef OBSOLETE
	// トランケートする
	void					truncate(const Trans::Transaction& trans,
									 Block::ID id = 0);
#endif
	// 移動する
	void					move(const Os::Path& path);

	// フラッシュする
	void					flush();

	// 整合性検査の開始を指示する
	void
	startVerification(Verification& verification,
					  Admin::Verification::Progress& result);

	// あるバージョンページの最古の版の複写である同期ログを確保する
	Block::Memory
	allocateLog(const Trans::Transaction& trans,
				Page::ID pageID, MasterData::File& masterData,
				Trans::TimeStamp::Value& allocation);
	Block::Memory
	allocateLog(const Trans::Transaction& trans, Block::Memory& headerMemory,
				Page::ID pageID, MasterData::File& masterData,
				Trans::TimeStamp::Value& allocation);

	// ファイルサイズを得る
	Os::File::Size			getSize() const;
	// ファイルの使用中部分のサイズを得る
	Os::File::Size			getBoundSize(const Trans::Transaction& trans);
	Os::File::Size			getBoundSize(const FileHeader& header);
	// ブロックサイズを得る
	Os::Memory::Size		getBlockSize() const;
	// 格納する OS ファイルの親ディレクトリの絶対パス名を得る
	const Os::Path&			getParent() const;
	// 最大ファイルサイズを得る
	Os::File::Size			getSizeMax() const;
	// ファイル拡張サイズを得る
	Os::File::Size			getExtensionSize() const;
#ifdef OBSOLETE
	// バッファリングするバッファプールの種別を得る
	Buffer::Pool::Category::Value getPoolCategory() const;
#endif
	// 構成する OS ファイルが存在するか調べる
	bool
	isAccessible(bool force = false) const;
#ifdef OBSOLETE
	// マウントされているか調べる
	bool
	isMounted() const;
#endif
	// マウントされかつ構成する OS ファイルが存在するか調べる
	bool
	isMountedAndAccessible() const;
#ifdef OBSOLETE
	// 読取専用か調べる
	bool
	isReadOnly() const;
#endif

private:
	// 拡張する
	void
	extend(Block::ID id);

	// ブロックを使用中にする
	Block::ID				allocate(const Trans::Transaction& trans);
	Block::ID				allocate(Block::Memory& headerMemory);

	// 実体であるバッファファイル
	Buffer::File*			_bufFile;
	// 格納する OS ファイルの親ディレクトリの絶対パス名
	Os::Path				_parent;
	// 最大ファイルサイズ(B 単位)
	const Os::File::Size	_sizeMax;
	// ファイル拡張サイズ(B 単位)
	const Os::File::Size	_extensionSize;
};

//	NAMESPACE
//	Version::SyncLog::VersionNumber --
//		同期ログファイルのバージョン番号に関する名前空間
//
//	NOTES

namespace VersionNumber
{
	//	ENUM
	//	Version::SyncLog::VersionNumber::Value --
	//		同期ログファイルのバージョン番号を表す値の列挙型
	//
	//	NOTES

	enum Value
	{
		// 不明
		Unknown =			-1,
		// 最初
		First =				0,
		// 値の数
		ValueNum,
		// 現在
		Current =			ValueNum - 1
	};
}

//	CLASS
//	Version::SyncLog::FileHeader --
//		同期ログファイルのファイルヘッダを表すクラス
//
//	NOTES

class FileHeader
{
	friend class File;
public:
	// 確保する
	static Block::Memory	allocate(const Trans::Transaction& trans,
									 File& file);
	// フィックスする
	static Block::Memory	fix(const Trans::Transaction& trans,
								File& file, Buffer::Page::FixMode::Value mode);

	// ファイルヘッダが格納されているブロックからファイルヘッダを得る
	static FileHeader&
	get(Block::Memory& memory);
	static const FileHeader&
	get(const Block::Memory& memory);

	// 総ブロック数を得る
	unsigned int			getBlockCount() const;

private:
	// バージョン番号
	VersionNumber::Value	_versionNumber;
	// 総ブロック数
	unsigned int			_blockCount;
};

//	CLASS
//	Version::SyncLog::Log --
//		同期ログファイルの同期ログを表すクラス
//
//	NOTES

class Log
	: public	VersionLog::Log
{
	friend class File;
public:
	// 確保する
	static Block::Memory	allocate(const Trans::Transaction& trans,
									 File& file);
	static Block::Memory	allocate(const Trans::Transaction& trans,
									 File& file, Block::Memory& headerMemory);
	// フィックスする
	static Block::Memory	fix(const Trans::Transaction& trans,
								File& file, Block::ID id,
								Buffer::Page::FixMode::Value mode);

	// 同期ログが格納されているブロックから同期ログを得る
	static Log&
	get(Block::Memory& memory);
	static const Log&
	get(const Block::Memory& memory);

/*	VersionLog::Log
protected:
	// 元になったマスタデータファイルのデータブロックのブロック識別子
	Block::ID				_older;
	// Block::IllegalID が必ず格納される
	Block::ID				_physicalLog;
	// 元になったマスタデータファイルの
	// データブロックを確保したときのタイムスタンプ値
	Trans::TimeStamp::Value	_olderTimeStamp;
	// Category::Oldest が必ず格納される
	Category::Value			_category;
*/
};

//	FUNCTION public
//	Version::SyncLog::File::~File --
//		同期ログファイル記述子を表すクラスのデストラクター
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
	// 同期ログファイルの実体である
	// バッファファイルのバッファファイル記述子を破棄する

	Buffer::File::detach(_bufFile);
}

//	FUNCTION public
//	Version::SyncLog::File::destroy -- 破棄する
//
//	NOTES
//		同期ログファイルが生成されていなくてもエラーにならない
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
	// 同期ログファイルが生成されていれば、
	// 同期ログファイルの実体である
	// バッファファイルを破棄する

	_bufFile->destroy();
}

//	FUNCTION public
//	Version::SyncLog::File::mount -- マウントする
//
//	NOTES
//		すでにマウントされている同期ログファイルや
//		生成されていない同期ログファイルを
//		マウントしてもエラーにならない
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
	// 同期ログファイルの実体である
	// バッファファイルをマウントする
	//
	//【注意】	マウントするバッファファイルの実体である
	//			OS ファイルが存在しなくても、例外を発生しない

	_bufFile->mount(false);
}

//	FUNCTION public
//	Version::SyncLog::File::unmount -- アンマウントする
//
//	NOTES
//		マウントされていない
//		同期ログファイルをアンマウントしてもエラーにならない
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
	// 同期ログファイルの実体である
	// バッファファイルをアンマウントする

	_bufFile->unmount();
}

//	FUNCTION public
//	Version::SyncLog::File::flush -- フラッシュする
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
	// 同期ログファイルの実体である
	// バッファファイルをフラッシュする

	_bufFile->flush();
}

//	FUNCTION public
//	Version::SyncLog::File::allocateLog --
//		あるバージョンページの最古の版の複写である同期ログを確保する
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			処理を行うトランザクションのトランザクション記述子
//		Version::Page::ID	pageID
//			このバージョンページ識別子の表す
//			バージョンページの最古の版を複写する
//		Version::MasterData::File&	masterData
//			複写する最古の版を取り出す
//			マスタデータファイルのマスタデータファイル記述子
//		Trans::TimeStamp::Value&	allocation
//			複写する最古の版を記録するブロックを確保したときのタイムスタンプ値
//
//	RETURN
//		確保された同期ログのバッファリング内容
//
//	EXCEPTIONS

inline
Block::Memory
File::allocateLog(const Trans::Transaction& trans,
				  Page::ID pageID, MasterData::File& masterData,
				  Trans::TimeStamp::Value& allocation)
{
	// ファイルヘッダをフィックスし、それを使って、同期ログを確保する

	Block::Memory	headerMemory(
		FileHeader::fix(trans, *this, Buffer::Page::FixMode::Write));
	return allocateLog(trans, headerMemory, pageID, masterData, allocation);
}

//	FUNCTION private
//	Version::SyncLog::File::allocate -- ブロックを 1 つ確保する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		確保したブロックのブロック識別子
//
//	EXCEPTIONS

inline
Block::ID
File::allocate(const Trans::Transaction& trans)
{
	// ファイルヘッダをフィックスする

	Block::Memory	headerMemory(
		FileHeader::fix(trans, *this, Buffer::Page::FixMode::Write));

	return allocate(headerMemory);
}

//	FUNCTION public
//	Version::SyncLog::File::getSize --
//		同期ログファイルのファイルサイズを得る
//
//	NOTES
//		同期ログファイルが生成されていないとき、ファイルサイズは 0 とみなす
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
//	Version::SyncLog::File::getBoundSize --
//		確保済のブロックの総サイズを求める
//
//	NOTES
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
	if (isMountedAndAccessible()) {
		const Block::Memory& headerMemory =
			FileHeader::fix(trans, *this, Buffer::Page::FixMode::ReadOnly);
		return getBoundSize(FileHeader::get(headerMemory));
	}
	return 0;
}

//	FUNCTION public
//	Version::SyncLog::File::getBoundSize --
//		使用中のブロックの総サイズを求める
//
//	NOTES
//		同期ログファイルが生成されていないとき、
//		使用中のブロックの総サイズは 0 とみなす
//
//	ARGUMENTS
//		Version::SyncLog::FileHeader&	header
//			確保済のブロックの総サイズを求める
//			同期ログファイルのファイルヘッダを表すクラス
//
//	RETURN
//		求めたサイズ(B 単位)
//
//	EXCEPTIONS

inline
Os::File::Size
File::getBoundSize(const FileHeader& header)
{
	return static_cast<Os::File::Size>(getBlockSize()) * header._blockCount;
}

//	FUNCTION public
//	Version::SyncLog::File::getBlockSize --
//		同期ログファイルのブロックサイズを得る
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
//	Version::SyncLog::getParent --
//		同期ログファイルを格納する OS ファイルの
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
//	Version::SyncLog::File::getSizeMax --
//		同期ログファイルの最大ファイルサイズを得る
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
//	Version::SyncLog::File::getExtensionSize --
//		同期ログファイルのファイル拡張サイズを得る
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
	return Version::File::getExtensionSize(getSize(), _extensionSize);
}

#ifdef OBSOLETE
//	FUNCTION public
//	Version::SyncLog::File::getPoolCategory --
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
#endif

//	FUNCTION public
//	Version::SyncLog::File::isAccessible --
//		同期ログファイルを構成する OS ファイルが存在するか調べる
//
//	NOTES
//
//	ARGUMENTS
//		bool				force
//			true
//				同期ログファイルの実体である
//				OS ファイルの存在を実際に調べる
//			false または指定されないとき
//				同期ログファイルの実体である
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

#ifdef OBSOLETE
//	FUNCTION public
//	Version::SyncLog::File::isMounted -- マウントされているか調べる
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
#endif

//	FUNCTION public
//	Version::SyncLog::File::isMountedAndAccessible --
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

#ifdef OBSOLETE
//	FUNCTION public
//	Version::SyncLog::File::isReadOnly -- 読取専用か調べる
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
#endif

//	FUNCTION public
//	Version::SyncLog::FileHeader::fix -- ファイルヘッダをフィックスする
//
//	NOTES
//
//	ARGUMENTS
//		Version::SyncLog::File&	file
//			フィックスするファイルヘッダが存在する
//			同期ログファイルの同期ログファイル記述子
//		Buffer::Page::FixMode::Value	mode
//			Buffer::Page::FixMode::ReadOnly
//				フィックスするファイルヘッダは参照しかされない
//			Buffer::Page::FixMode::Write
//				フィックスするファイルヘッダは更新される
//			Buffer::Page::FixMode::Allocate
//				フィックスするファイルヘッダは
//				その領域の初期化のために使用する
//
//	RETURN
//		フィックスしたファイルヘッダのバッファリング内容
//
//	EXCEPTIONS

// static
inline
Block::Memory
FileHeader::fix(const Trans::Transaction& trans,
				File& file, Buffer::Page::FixMode::Value mode)
{
	return Block::Memory(
		0, Buffer::Page::fix(*file._bufFile, 0,
							 mode, Buffer::ReplacementPriority::High, &trans));
}

//	FUNCTION public
//	Version::SyncLog::FileHeader::get --
//		ファイルヘッダが格納されているブロックからファイルヘッダを得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::Memory&	memory
//			ファイルヘッダが格納されているブロックのバッファリング内容
//
//	RETURN
//		得られたファイルヘッダへのリファレンス
//
//	EXCEPTIONS
//		なし

// static
inline
FileHeader&
FileHeader::get(Block::Memory& memory)
{
	return *static_cast<FileHeader*>(memory.operator void*());
}

// static
inline
const FileHeader&
FileHeader::get(const Block::Memory& memory)
{
	return *static_cast<const FileHeader*>(memory.operator const void*());
}

//	FUNCTION public
//	Version::SyncLog::FileHeader::getBlockCount -- 総ブロック数を得る
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
//		なし

inline
unsigned int
FileHeader::getBlockCount() const
{
	return _blockCount;
}

//	FUNCTION public
//	Version::SyncLog::Log::allocate --
//		同期ログファイルに同期ログを確保する
//
//	NOTES
//
//	ARGUMENTS
//		Version::SyncLog::File&	file
//			同期ログを確保する同期ログファイルの同期ログファイル記述子
//	
//	RETURN
//		確保した同期ログを記録するブロックのバッファリング内容
//
//	EXCEPTIONS

// static
inline
Block::Memory
Log::allocate(const Trans::Transaction& trans, File& file)
{
	Block::Memory	headerMemory(
		FileHeader::fix(trans, file, Buffer::Page::FixMode::Write));

	return Log::allocate(trans, file, headerMemory);
}

//	FUNCTION public
//	Version::SyncLog::Log::get --
//		同期ログが格納されているブロックから同期ログを得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::Memory&	memory
//			同期ログが格納されているブロックのバッファリング内容
//
//	RETURN
//		得られた同期ログへのリファレンス
//
//	EXCEPTIONS
//		なし

// static
inline
Log&
Log::get(Block::Memory& memory)
{
	return *static_cast<Log*>(memory.operator void*());
}

// static
inline
const Log&
Log::get(const Block::Memory& memory)
{
	return *static_cast<const Log*>(memory.operator const void*());
}

_SYDNEY_VERSION_SYNCLOG_END
_SYDNEY_VERSION_END
_SYDNEY_END

#endif	// __SYDNEY_VERSION_SYNCLOG_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2009, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
