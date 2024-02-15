// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.h -- バージョンファイル関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2010, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VERSION_FILE_H
#define	__SYDNEY_VERSION_FILE_H

#include "Version/Module.h"
#include "Version/Page.h"

#include "Admin/Verification.h"
#include "Buffer/File.h"
#include "Buffer/Page.h"
#include "Buffer/Pool.h"
#include "Common/Object.h"
#include "Lock/Name.h"
#include "Os/CriticalSection.h"
#include "Os/File.h"
#include "Os/Memory.h"
#include "Os/RWLock.h"
#include "Trans/TimeStamp.h"

template <class T>
class ModAutoPointer;

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}

_SYDNEY_VERSION_BEGIN

namespace Manager
{
	class File;
}
namespace MasterData
{
	class File;
}
namespace SyncLog
{
	class File;
}
namespace VersionLog
{
	class File;
}

//	CLASS
//	Version::File -- バージョンファイル記述子を表すクラス
//
//	NOTES

class File
{
	friend class AutoFile;
	friend class DetachedPageCleaner;
	friend class Manager::File;
	friend class ModAutoPointer<File>;
	friend class Page;
	friend class Verification;
public:
	//	TYPEDEF
	//	Version::File::ID --
	//		バージョンファイルを表すクラスを一意に識別する値を表す型
	//
	//	NOTES

	typedef Buffer::File::ID	ID;

	//	CLASS
	//	Version::File::StorageStrategy --
	//		バージョンファイルのファイル格納戦略を表すクラス
	//
	//	NOTES

	struct StorageStrategy
	{
		// コンストラクター
		StorageStrategy();
		// デストラクター
		~StorageStrategy();

		// マウントされているか
		bool					_mounted;
		// 読取専用か
		bool					_readOnly;
		// バージョンページサイズ(B 単位)
		Os::Memory::Size		_pageSize;

		struct Path
		{
			// マスタデータファイルの親ディレクトリの絶対パス名
			Os::Path			_masterData;
			// バージョンログファイルの親ディレクトリの絶対パス名
			Os::Path			_versionLog;
			// 同期ログファイルの親ディレクトリの絶対パス名
			Os::Path			_syncLog;
		}					_path;
		struct SizeMax
		{
			// コンストラクター
			SizeMax();
			// デストラクター
			~SizeMax();

			// マスタデータファイルの最大ファイルサイズ(B 単位)
			Os::File::Size		_masterData;
			// バージョンログファイルの最大ファイルサイズ(B 単位)
			Os::File::Size		_versionLog;
			// 同期ログファイルの最大ファイルサイズ(B 単位)
			Os::File::Size		_syncLog;
		}					_sizeMax;
		struct ExtensionSize
		{
			// コンストラクター
			ExtensionSize();
			// デストラクター
			~ExtensionSize();

			// マスタデータファイルのエクステンションサイズ(B 単位)
			Os::File::Size		_masterData;
			// バージョンログファイルのエクステンションサイズ(B 単位)
			Os::File::Size		_versionLog;
			// 同期ログファイルのエクステンションサイズ(B 単位)
			Os::File::Size		_syncLog;
		}					_extensionSize;
	};

	//	CLASS
	//	Version::File::BufferingStrategy --
	//		バージョンファイルのバッファリング戦略を表すクラス
	//
	//	NOTES

	struct BufferingStrategy
	{
		// コンストラクター
		BufferingStrategy();
		// デストラクター
		~BufferingStrategy();

		// バッファプールの種別
		Buffer::Pool::Category::Value _category;
	};

	// 記述子を生成する
	SYD_VERSION_FUNCTION 
	static File*
	attach(const StorageStrategy& storageStrategy,
		   const BufferingStrategy& bufferingStrategy,
		   bool isBatch_ = false);
	SYD_VERSION_FUNCTION 
	static File*
	attach(const StorageStrategy& storageStrategy,
		   const BufferingStrategy& bufferingStrategy,
		   const Lock::FileName& lockName,
		   bool isBatch_ = false);
	// 記述子を破棄する
	SYD_VERSION_FUNCTION 
	static void
	detach(File*& file, bool reserve);

	// 生成する
	SYD_VERSION_FUNCTION 
	void					create(const Trans::Transaction& trans);
	// 破棄する
	SYD_VERSION_FUNCTION 
	void					destroy(const Trans::Transaction& trans);
	// マウントする
	SYD_VERSION_FUNCTION 
	void					mount(const Trans::Transaction& trans);
	// アンマウントする
	SYD_VERSION_FUNCTION 
	void					unmount(const Trans::Transaction& trans);
	// 指定されたバージョンページ識別子の
	// バージョンページの直後からトランケートする
	SYD_VERSION_FUNCTION 
	void
	truncate(const Trans::Transaction& trans, Page::ID id);
	// 移動する
	SYD_VERSION_FUNCTION 
	void
	move(const Trans::Transaction& trans, const StorageStrategy::Path& path);

	// フラッシュする
	SYD_VERSION_FUNCTION 
	void					flush(const Trans::Transaction& trans);

	// バックアップの開始を指示する
	SYD_VERSION_FUNCTION 
	void					startBackup(const Trans::Transaction& trans,
										bool restorable = true);
	// バックアップの終了を指示する
	SYD_VERSION_FUNCTION 
	void					endBackup(const Trans::Transaction& trans);
	// ある時点に開始された版管理するトランザクションが参照する版を最新版とする
	SYD_VERSION_FUNCTION 
	void
	restore(const Trans::Transaction& trans, const Trans::TimeStamp& point);
	// ある時点以前のチェックポイント処理終了時の状態に障害回復する
	SYD_VERSION_FUNCTION 
	void
	recover(const Trans::Transaction& trans, const Trans::TimeStamp& point);

	// 整合性検査の開始を指示する
	SYD_VERSION_FUNCTION 
	void
	startVerification(const Trans::Transaction& trans,
					  Admin::Verification::Treatment::Value treatment,
					  Admin::Verification::Progress& result,
					  bool overall = false);
	// 整合性検査の終了を指示する
	SYD_VERSION_FUNCTION 
	void
	endVerification(const Trans::Transaction& trans,
					Admin::Verification::Progress& result);

	// 同期を取る
	SYD_VERSION_FUNCTION 
	void
	sync(const Trans::Transaction& trans, bool& incomplete, bool& modified);

	// 識別子を得る
	SYD_VERSION_FUNCTION
	ID						getID() const;
	// 実際のファイルサイズを得る
	SYD_VERSION_FUNCTION 
	Os::File::Size			getSize() const;
	// ファイルの使用中部分のサイズを得る
	SYD_VERSION_FUNCTION 
	Os::File::Size			getBoundSize(const Trans::Transaction& trans);

	// マスタデータファイルの親ディレクトリの絶対パス名を得る
	SYD_VERSION_FUNCTION 
	const Os::Path&			getParent() const;
	// ページサイズを得る
	SYD_VERSION_FUNCTION 
	Os::Memory::Size		getPageSize() const;

	// ファイル格納戦略を得る
	SYD_VERSION_FUNCTION 
	StorageStrategy			getStorageStrategy() const;
	// バッファリング戦略を得る
	SYD_VERSION_FUNCTION 
	BufferingStrategy		getBufferingStrategy() const;
	// バージョンファイルを使用する論理ファイルのロック名を得る
	const Lock::FileName&	getLockName() const;

	// 構成する OS ファイルが存在するか調べる
	SYD_VERSION_FUNCTION 
	bool
	isAccessible(bool force = false) const;
	// マウントされているか調べる
	SYD_VERSION_FUNCTION 
	bool
	isMounted(const Trans::Transaction& trans) const;
	// 読取専用か調べる
	SYD_VERSION_FUNCTION 
	bool
	isReadOnly() const;
	// バッチインサートか調べる
	SYD_VERSION_FUNCTION
	bool
	isBatchInsert() const;
	// バッチインサートフラグを設定する
	SYD_VERSION_FUNCTION
	void
	setBatch(bool batch_);

	// バージョンページサイズを矯正する
	static Os::Memory::Size	verifyPageSize(Os::Memory::Size size);
	// 各ファイルの最大ファイルサイズを矯正する
	SYD_VERSION_FUNCTION 
	static Os::File::Size	verifySizeMax(Os::File::Size size);
	// 各ファイルのエクステンションサイズを矯正する
	SYD_VERSION_FUNCTION 
	static Os::File::Size	verifyExtensionSize(Os::File::Size size);

	// 各ファイルのエクステンションサイズを得る
	SYD_VERSION_FUNCTION 
	static Os::File::Size	getExtensionSize(Os::File::Size fileSize_,
											 Os::File::Size extensionSize_);
	
private:
	// コンストラクター
	File(const StorageStrategy& storageStrategy,
		 const BufferingStrategy& bufferingStrategy,
		 const Lock::FileName& lockName);
	// デストラクター
	~File();

	// デストラクター下位関数
	void					destruct();

	// ファイルのすべてのバージョンページ記述子を破棄する
	void					discardPage();

	// いくつかのバージョンページの同期を取る
	bool
	syncPage(const Trans::Transaction& trans,
			 Page::ID& id, unsigned int n, bool& modified, bool& isContinue);

	// バージョンログファイル・同期ログファイルの
	// 生成・破棄の排他制御用の読み取り書き込みロックを得る
	Os::RWLock&				getRWLock() const;

	// 参照数を 1 増やす
	File*
	attach();
	// 参照数を 1 減らす
	void
	detach();
	// 参照数を得る
	unsigned int
	getRefCount() const;

	// バージョンファイルを使用する論理ファイルのロック名
	const Lock::FileName	_lockName;

	// バージョンログファイル・同期ログファイルの
	// 生成・破棄の排他制御用の読み取り書き込みロック
	mutable Os::RWLock		_rwlock;

	struct
	{
		// マスタデータファイル記述子
		MasterData::File*		_masterData;
		// バージョンログファイル記述子
		VersionLog::File*		_versionLog;
		// 同期ログファイル記述子
		SyncLog::File*			_syncLog;
	}						_file;

	// 参照回数
	//【注意】	Page::getHashTable().getBucket(i).getLatch() で保護される
	mutable unsigned int	_refCount;
	// バージョンファイルの生成(マウント)したトランザクションの識別子
	Trans::Transaction::ID	_creatorTransaction;

	// 格納されているハッシュリストのバケットアドレス
	//【注意】	$$$::_File::_fileTable->getBucket(i).getLatch() で保護される
	unsigned int			_hashAddr;
	// ハッシュリストでの直前の要素へのポインタ
	//【注意】	$$$::_File::_fileTable->getBucket(i).getLatch() で保護される
	File*					_hashPrev;
	// ハッシュリストでの直後の要素へのポインタ
	//【注意】	$$$::_File::_fileTable->getBucket(i).getLatch() で保護される
	File*					_hashNext;

	// バッチインサートモードかどうか
	bool					_batch;
};

//
//	FUNCTION public
//	Version::File::setBatch -- バッチモードを設定する
//
//	NOTES
//	バッチインサートモードの場合、他のトランザクションがこのファイルを参照する
//	ことはないので、ページの更新トランザクションリストはメンテナンスしない
//
//	ARGUMENTS
//	bool batch_
//		バッチインサートモードの場合はtrueを指定する。それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
inline
void
File::setBatch(bool batch_)
{
	_batch = batch_;
}

//	FUNCTION public
//	Version::File::verifyPageSize --
//		バージョンページサイズとして与えられた値を矯正する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Memory::Size	size
//			矯正する値(B 単位)
//
//	RETURN
//		矯正後の値(B 単位)
//
//	EXCEPTIONS

// static
inline
Os::Memory::Size
File::verifyPageSize(Os::Memory::Size size)
{
	return Buffer::Page::correctSize(size);
}

//	FUNCTION private
//	Version::File::~File --
//		バージョンファイル記述子を表すクラスのデストラクター
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
	destruct();
}

//	FUNCTION public
//	Version::File::getLockName --
//		バージョンファイルを使用する論理ファイルのロック名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたロック名
//
//	EXCEPTIONS
//		なし

inline
const Lock::FileName&
File::getLockName() const
{
	return _lockName;
}

//	FUNCTION private
//	Version::File::getRWLock --
//		バージョンログファイル・同期ログファウルの生成・破棄の
//		排他制御用の読み取り書き込みロックを得る
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
File::getRWLock() const
{
	return _rwlock;
}

//	FUNCTION private
//	Version::File::getRefCount -- バージョンファイル記述子の参照数を得る
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
File::getRefCount() const
{
	return _refCount;
}

//	FUNCTION public
//	Version::File::StorageStrategy::StorageStrategy --
//		ファイル格納戦略を表すクラスのコンストラクター
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
File::StorageStrategy::StorageStrategy()
	: _mounted(true),
	  _readOnly(false),
	  _pageSize(0)
{}

//	FUNCTION public
//	Version::File::StorageStrategy::~StorageStrategy --
//		ファイル格納戦略を表すクラスのデストラクター
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
File::StorageStrategy::~StorageStrategy()
{}

//	FUNCTION public
//	Version::File::StorageStrategy::SizeMax::SizeMax --
//		ファイル格納戦略の最大ファイルサイズを格納するクラスのコンストラクター
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
File::StorageStrategy::SizeMax::SizeMax()
	: _masterData(0),
	  _versionLog(0),
	  _syncLog(0)
{}

//	FUNCTION public
//	Version::File::StorageStrategy::SizeMax::~SizeMax --
//		ファイル格納戦略の最大ファイルサイズを格納するクラスのデストラクター
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
File::StorageStrategy::SizeMax::~SizeMax()
{}

//	FUNCTION public
//	Version::File::StorageStrategy::ExtensionSize::ExtensionSize --
//		ファイル格納戦略のファイル拡張サイズを格納するクラスのコンストラクター
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
File::StorageStrategy::ExtensionSize::ExtensionSize()
	: _masterData(0),
	  _versionLog(0),
	  _syncLog(0)
{}

//	FUNCTION public
//	Version::File::StorageStrategy::ExtensionSize::~ExtensionSize --
//		ファイル格納戦略のファイル拡張サイズを格納するクラスのデストラクター
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
File::StorageStrategy::ExtensionSize::~ExtensionSize()
{}

//	FUNCTION public
//	Version::File::BufferingStrategy::BufferingStrategy --
//		バッファリング戦略を表すクラスのコンストラクター
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
File::BufferingStrategy::BufferingStrategy()
	: _category(Buffer::Pool::Category::Normal)
{}

//	FUNCTION public
//	Version::File::BufferingStrategy::~BufferingStrategy --
//		バッファリング戦略を表すクラスのデストラクター
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
File::BufferingStrategy::~BufferingStrategy()
{}

_SYDNEY_VERSION_END
_SYDNEY_END

#endif	// __SYDNEY_VERSION_FILE_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2010, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
