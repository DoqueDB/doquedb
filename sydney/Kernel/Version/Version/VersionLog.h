// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VersionLog.h -- バージョンログファイル関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VERSION_VERSIONLOG_H
#define	__SYDNEY_VERSION_VERSIONLOG_H

#include "Version/Module.h"
#include "Version/Block.h"
#include "Version/File.h"

#include "Buffer/File.h"
#include "Buffer/Page.h"
#include "Buffer/Pool.h"
#include "Buffer/ReplacementPriority.h"
#include "Common/Object.h"
#include "Os/File.h"
#include "Os/Memory.h"
#include "Admin/Verification.h"
#include "Trans/TimeStamp.h"

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}

_SYDNEY_VERSION_BEGIN

template <class T>
class HashTable;

namespace MasterData
{
	class File;
}

class Page;
class Verification;

_SYDNEY_VERSION_VERSIONLOG_BEGIN

class FileHeader;
class MultiplexBlock;
class MultiplexInfo;

//	CONST
//	Version::VersionLog::MultiplexCount --
//		ファイルヘッダー、アロケーションテーブル、
//		PBCT のそれぞれのブロックを安全のためになん重に記録するか
//
//	NOTES

const unsigned int			MultiplexCount = 3;

//	NAMESPACE
//	Version::VersionLog::PBCTLevel -- PBCT の深さに関する名前空間
//
//	NOTES

namespace PBCTLevel
{
	//	TYPEDEF
	//	Version::VersionLog::PBCTLevel::Value -- PBCT の深さを表す値の列挙型
	//
	//	NOTES

	typedef	int				Value;
	enum
	{
		// 不正
		Illegal =			-1
	};
}

//	NAMESPACE
//	Version::VersionLog::VersionNumber --
//		バージョンログファイルのバージョン番号に関する名前空間
//
//	NOTES

namespace VersionNumber
{
	//	ENUM
	//	Version::VersionLog::VersionNumber::Value --
	//		バージョンログファイルのバージョン番号を表す値の列挙型
	//
	//	NOTES

	enum Value
	{
		// 不明
		Unknown =			-1,
		// 最初
		First =				0,
		// それ以降
		Second,
		// 値の数
		ValueNum,
		// 現在
		Current =			ValueNum - 1
	};
}

//	CLASS
//	Version::VersionLog::File -- バージョンログファイルを表すクラス
//
//	NOTES

class File
{
	friend class FileHeader;
	friend class AllocationTable;
	friend class PBCTNode;
	friend class PBCTLeaf;
	friend class Log;
	friend class MultiplexInfo;
public:
	// コンストラクター
	File(Version::File& versionFile,
		 const Version::File::StorageStrategy& storageStrategy,
		 const Version::File::BufferingStrategy& bufferingStrategy);
	// デストラクター
	~File();

	// 生成する
	void					create(const Trans::Transaction& trans,
								   unsigned int pageCount = 0);
	// 破棄する
	void					destroy();
	// マウントする
	void					mount();
	// アンマウントする
	void					unmount();

	// トランケートして、空にする
	bool
	truncate(const Trans::Transaction& trans);
	// あるバージョンページ以降をすべて使用済にし、可能な限りトランケートする
	bool
	truncate(const Trans::Transaction& trans, Page::ID pageID);

	// 移動する
	void					move(const Os::Path& path);

	// フラッシュする
	void					flush();

	// バックアップの開始を指示する
	void					startBackup();
	// バックアップの終了を指示する
	void					endBackup();

	// ある時点に開始された版管理するトランザクションが参照する版を最新版とする
	bool
	restore(const Trans::Transaction& trans, const Trans::TimeStamp& point);
	// ある時点以前のチェックポイント処理終了時の状態に障害回復する
	bool
	recover(const Trans::Transaction& trans,
			const Trans::TimeStamp& point, unsigned int& pageCount);

	// 整合性検査の開始を指示する
	void
	startVerification(const Trans::Transaction& trans,
					  Verification& verification,
					  MasterData::File& masterData,
					  Admin::Verification::Progress& result);
	// 整合性検査の終了を指示する
	void
	endVerification(const Trans::Transaction& trans,
					Verification& verification,
					Admin::Verification::Progress& result);

	// あるバージョンページに関して整合性検査する
	void
	verify(const Trans::Transaction& trans, Verification& verification,
		   Page::ID pageID, Admin::Verification::Progress& result);

	// あるバージョンページの版のうち、
	// できる限り最新のものをマスタデータファイルへ複写する
	Trans::TimeStamp::Value
	syncLog(const Trans::Transaction& trans,
			Block::Memory& headerMemory, Page& page, Block::ID id,
			Trans::TimeStamp::Value oldest, Trans::TimeStamp::Value eldest,
			MasterData::File& masterData, Trans::TimeStamp::Value allocation);

	// PBCT のルートノード以下で、あるバージョンページの
	// 最新版のブロック識別子を記録する PBCT のリーフを探す
	Block::Memory
	traversePBCT(const Trans::Transaction& trans,
				 Verification* verification,
				 Page::ID pageID, Buffer::Page::FixMode::Value mode);
	Block::Memory
	traversePBCT(const Trans::Transaction& trans,
				 Verification* verification, const Block::Memory& headerMemory,
				 Page::ID pageID, Buffer::Page::FixMode::Value mode);

	// あるバージョンページの版のうち、
	// あるトランザクションが参照すべきもののブロック識別子を探す
	Block::Memory
	traverseLog(const Trans::Transaction& trans,
				Verification* verification,	const Page& page,
				Block::ID id, Trans::TimeStamp::Value oldest,
				Buffer::ReplacementPriority::Value priority);

	// あるバージョンページの最新版のブロック識別子を得るためにたどる
	// PBCT ノード、リーフを必要なだけ確保する
	Block::Memory
	allocatePBCT(const Trans::Transaction& trans,
				 Verification* verification, Page::ID pageID);
	Block::Memory
	allocatePBCT(const Trans::Transaction& trans,
				 Verification* verification,
				 MultiplexBlock& headerMulti, Page::ID pageID);

	// あるバージョンページの新しい最新版のバージョンログを確保する
	Block::Memory
	allocateLog(const Trans::Transaction& trans,
				Verification* verification, Page& page,
				Block::Memory& src, Trans::TimeStamp::Value oldest,
				Buffer::ReplacementPriority::Value priority);
	Block::Memory
	allocateLog(const Trans::Transaction& trans, Verification* verification,
				MultiplexBlock& headerMulti, Page& page,
				Block::Memory& src, Trans::TimeStamp::Value oldest,
				Buffer::ReplacementPriority::Value priority);

	// あるバージョンページの新しい最新版をバージョンログを必要があれば、
	// 確保し、バックアップを可能にする
	Block::Memory
	allocateLogForBackup(const Trans::Transaction& trans,
						 MultiplexBlock& headerMulti,
						 Page& page, const Block::Memory& src);

	// あるバージョンページの最新版のブロック識別子を得るためにたどる
	// PBCT ノード、リーフを可能な限り使用済にする
	void
	freePBCT(const Trans::Transaction& trans,
			 Verification* verification, Page::ID pageID);
	void
	freePBCT(const Trans::Transaction& trans,
			 Verification* verification,
			 Block::Memory& headerMemory, Page::ID pageID);

	// あるバージョンログとそれより前のものをすべて使用済にする
	void
	freeLog(const Trans::Transaction& trans,
			VersionNumber::Value v, Verification* verification,
			Block::ID id, Trans::TimeStamp::Value oldest);

	// ファイルサイズを得る
	Os::File::Size			getSize() const;
	// ファイルの使用中部分のサイズを得る
	Os::File::Size
	getBoundSize(const Trans::Transaction& trans,
				 Verification* verification);
	Os::File::Size
	getBoundSize(const Trans::Transaction& trans,
				 Verification* verification, const FileHeader& header);
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
	Buffer::Pool::Category::Value
	getPoolCategory() const;
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
	// 使用済み(開放されたブロック)をアロケーションテーブルに反映する
	void
	applyFree(const Trans::Transaction& trans);

private:
	// デストラクター下位関数
	void					destruct();

	// 拡張する
	void
	extend(Block::ID id);

	// ブロックを使用中にする
	Block::ID
	allocate(const Trans::Transaction& trans,
			 Verification* verification, unsigned int n = 1);
	Block::ID
	allocate(const Trans::Transaction& trans, Verification* verification,
			 MultiplexBlock& headerMulti, unsigned int n = 1);

	// ブロックを使用済にする
	void
	free(const Trans::Transaction& trans,
		 VersionNumber::Value v, Verification* verification,
		 Block::ID id, unsigned int n = 1);
	// あるバージョンページの最新版のブロック識別子を得るためにたどる
	// PBCT ノード、リーフのうち、あるレベル以下のものを可能な限り使用済にする
	bool
	freePBCT(const Trans::Transaction& trans,
			 VersionNumber::Value v,
			 Verification* verification, Page::ID pageID,
			 Block::Memory& nodeMemory, PBCTLevel::Value current,
			 PBCTLevel::Value level);
	// あるバージョンログに対するすべての物理ログを使用済にする
	void
	freePhysicalLog(const Trans::Transaction& trans,
					VersionNumber::Value v,
					Verification* verification, Block::ID id);

	// ブロックが最新かどうかを設定する
	void
	setNewest(const Trans::Transaction& trans,
			  VersionNumber::Value v,
			  Verification* verification, Block::ID id, bool on);

	// ブロックをフィックスする
	Block::Memory
	fixBlock(const Trans::Transaction& trans,
			 Verification* verification,
			 Block::ID id, Buffer::Page::FixMode::Value mode,
			 Buffer::ReplacementPriority::Value priority);
	// マスタブロックをフィックスする
	Block::Memory
	fixMaster(const Trans::Transaction& trans,
			  Verification* verification,
			  Block::ID id,	Buffer::Page::FixMode::Value mode,
			  Buffer::ReplacementPriority::Value priority,
			  void (*initializeFunc)(Block::Memory&) = 0);
	// マスタおよびスレーブブロックをフィックスする
	void
	fixMasterAndSlaves(const Trans::Transaction& trans,
					   Verification* verification,
					   Block::ID id, Buffer::Page::FixMode::Value mode,
					   Buffer::ReplacementPriority::Value priority,
					   MultiplexBlock& multi,
					   void (*initializeFunc)(Block::Memory&) = 0);

	// PBCT を直前のチェックポイント処理終了時の状態に戻す
	void
	recoverPBCT(const Trans::Transaction& trans,
				const Block::Memory& nodeMemory, PBCTLevel::Value current,
				PBCTLevel::Value level,	const Trans::TimeStamp& point);
	// マスタブロックを直前のチェックポイント処理終了時の状態に戻す
	Block::Memory
	recoverMaster(const Trans::Transaction& trans,
				  Block::ID id, const Trans::TimeStamp& point);

	// あるバージョンページの最新版のバージョンログの
	// ブロック識別子を得るための PBCT ノード、リーフを整合性検査する
	Block::Memory
	verifyPBCT(const Trans::Transaction& trans,
			   Verification& verification, Page::ID pageID,
			   Admin::Verification::Progress& result);
	// あるバージョンページのすべての版のバージョンログに対して整合性検査する
	void
	verifyLog(
		const Trans::Transaction& trans, Verification& verification,
		Page::ID pageID, Block::ID id, Trans::TimeStamp::Value oldest,
		Admin::Verification::Progress& result);
	// あるバージョンログに対するすべての物理ログに対して整合性検査する
	void
	verifyPhysicalLog(
		const Trans::Transaction& trans, Verification& verification,
		Block::ID id, Block::ID older, Admin::Verification::Progress& result);
	// すべてのアロケーションテーブルを整合性検査する
	void
	verifyAllocationTable(
		const Trans::Transaction& trans,
		Verification& verification, const FileHeader& header,
		Admin::Verification::Progress& result);

	// バージョンログファイルの多重化されたブロックの
	// どれを選択するかを決めるための情報をすべて破棄する
	void
	clearMultiplexInfoTable(Block::ID id = 0);

	// ファイルで最後の使用中のブロックのブロック識別子を求める
	Block::ID
	getLastBoundBlockID(const Trans::Transaction& trans,
						VersionNumber::Value v, unsigned int blockCount);

	// 実体であるバッファファイル
	Buffer::File*			_bufFile;
	// バージョンログファイルを持つバージョンファイル
	Version::File&			_versionFile;
	// 格納する OS ファイルの親ディレクトリの絶対パス名
	Os::Path				_parent;
	// バージョンログファイルの多重化されたブロックの
	// どれを選択するかを決めるための情報をすべて管理するハッシュ表
	HashTable<MultiplexInfo>*	_infoTable;
	// 最大ファイルサイズ(B 単位)
	const Os::File::Size	_sizeMax;
	// ファイル拡張サイズ(B 単位)
	const Os::File::Size	_extensionSize;
};

//	CLASS
//	Version::VersionLog::FileHeader --
//		バージョンログファイルのファイルヘッダを表すクラス
//
//	NOTES

class FileHeader
{
	friend class File;
	friend class AllocationTable;
public:
	// 確保する
	static Block::Memory
	allocate(const Trans::Transaction& trans,
			 Verification* verification, File& file);
	// マスタブロックをフィックスする
	static Block::Memory
	fix(const Trans::Transaction& trans,
		Verification* verification,
		File& file, Buffer::Page::FixMode::Value mode);
	// マスタおよびスレーブブロックをフィックスする
	static void
	fix(const Trans::Transaction& trans,
		Verification* verification,
		File& file, Buffer::Page::FixMode::Value mode,
		MultiplexBlock& multi);

	// ファイルヘッダが格納されているブロックからファイルヘッダを得る
	static FileHeader&
	get(Block::Memory& memory);
	static const FileHeader&
	get(const Block::Memory& memory);

	// 総バージョンページ数を得る
	unsigned int			getPageCount() const;
	// 総バージョンページ数を設定する
	void					setPageCount(unsigned int n);

	// PBCT のレベル数を得る
	PBCTLevel::Value		getPBCTLevel() const;

	// PBCT が空であるか調べる
	bool					isPBCTEmpty() const;

	// バージョンを得る
	VersionNumber::Value	getVersion() const;

private:
	// バージョン番号
	VersionNumber::Value	_versionNumber;
	// 総ブロック数
	unsigned int			_blockCount;
	// 総バージョンページ数
	unsigned int			_pageCount;
	// PBCT のレベル数(0 以上)
	PBCTLevel::Value		_PBCTLevel;
	// バージョンログファイルの生成時のタイムスタンプ値
	Trans::TimeStamp::Value	_creationTimeStamp;
	// 将来のための予約領域
	unsigned int			_reserved[10];
};

//	CLASS
//	Version::VersionLog::AllocationTable --
//		バージョンログファイルのアロケーションテーブルを表すクラス
//
//	NOTES

class AllocationTable
{
	friend class File;
public:
	// 確保する
	static Block::Memory
	allocate(const Trans::Transaction& trans,
			 Verification* verification, File& file);
	static Block::Memory
	allocate(const Trans::Transaction& trans,
			 Verification* verification, File& file,
			 MultiplexBlock& headerMulti);
	// マスタブロックをフィックスする
	static Block::Memory
	fix(const Trans::Transaction& trans,
		Verification* verification,
		File& file, Block::ID id, Buffer::Page::FixMode::Value mode);
	// マスタおよびスレーブブロックをフィックスする
	static void
	fix(const Trans::Transaction& trans,
		Verification* verification,
		File& file, Block::ID id, Buffer::Page::FixMode::Value mode,
		MultiplexBlock& multi);

	// アロケーションテーブルが格納されているブロックから
	// アロケーションテーブルを得る
	static AllocationTable&
	get(Block::Memory& memory);
	static const AllocationTable&
	get(const Block::Memory& memory);

	// ブロックが最新版かを表すビットを操作する
	void
	setNewestBit(VersionNumber::Value v, Verification* verification,
				 Os::Memory::Size size, Block::ID id, unsigned int n, bool on);

	// 使用済みを反映する
	static void applyFree(MultiplexBlock& multi,
						  const Trans::TimeStamp::Value& second,
						  VersionNumber::Value v, Os::Memory::Size size);

	// チェックポイント後に初めて確保されたアロケーションテーブルを初期化する
	static void initializeBlock(Block::Memory& memory);

private:
	// ブロックが使用中かを表すビットを操作する 
	void
	setBoundBit(VersionNumber::Value v, Verification* verification,
				Os::Memory::Size size, Block::ID id, unsigned int n, bool on);
	// ブロックが使用中かを表すビットが立っているか
	bool
	getBoundBit(VersionNumber::Value v,
				Os::Memory::Size size, Block::ID id) const;

	// 使用済みが反映されているかどうか
	bool
	isApplyFree() const;
	// 使用済みが反映されていることにする
	void
	setApplyFree();
	// 使用済みが反映されていないことにする
	void
	unsetApplyFree();

	// 使用中のブロック数を得る
	unsigned int
	getCount() const;

	// 使用中のブロック数
	unsigned int			_count;
	// ブロックが使用中かを表すビットマップ配列
	unsigned int			_bitmap[1];

	// _count中で、使用済みが反映されているかどうかを表すビット位置
	enum { FREE_BIT = 0x80000000 };
};

//	CLASS
//	Version::VersionLog::PBCTNode --
//		バージョンログファイルのページ→ブロック識別子変換木(PBCT)の
//		ノードを表すクラス
//
//	NOTES

class PBCTNode
{
	friend class File;
public:
	// 確保する
	static Block::Memory
	allocate(const Trans::Transaction& trans,
			 Verification* verification, File& file);
	static Block::Memory
	allocate(const Trans::Transaction& trans,
			 Verification* verification, File& file,
			 MultiplexBlock& headerMulti);
	// 使用済にする
	static void
	free(const Trans::Transaction& trans,
		 VersionNumber::Value v,
		 Verification* verification, File& file, Block::ID id);
	// フィックスする
	static Block::Memory
	fix(const Trans::Transaction& trans,
		Verification* verification,
		File& file, Block::ID id, Buffer::Page::FixMode::Value mode);

	// PBCT ノードが格納されているブロックから PBCT ノードを得る
	static PBCTNode&
	get(Block::Memory& memory);
	static const PBCTNode&
	get(const Block::Memory& memory);

	// あるバージョンページの版をたどるための
	// 子ノードを記憶するブロック識別子を設定する
	void
	setChildID(Page::ID pageID, Block::ID id,
			   PBCTLevel::Value level, Os::Memory::Size size);
	void
	setChildID(Page::ID pageID, PBCTLevel::Value current, Block::ID id,
			   PBCTLevel::Value level, Os::Memory::Size size);
	// あるバージョンページを版をたどるための
	// 子ノードを記憶するブロック識別子を得る
	Block::ID
	getChildID(Page::ID pageID,
			   PBCTLevel::Value level, Os::Memory::Size size) const;
	Block::ID
	getChildID(Page::ID pageID, PBCTLevel::Value current,
			   PBCTLevel::Value level, Os::Memory::Size size) const;

	// 記録されている下位のノードの数を得る
	unsigned int
	getCount() const;
	// 記録可能な下位のノードの最大数を得る
	static unsigned int
	getCountMax(bool isNotRoot, Os::Memory::Size size);

	// あるブロック識別子のブロックを含む多重化されたブロックのうち、
	// 先頭のもののブロック識別子を得る
	static Block::ID
	normalizeID(Block::ID id);

private:
	// 初期化する
	void
	initialize(bool isNotRoot, Os::Memory::Size size);
	// 与えられたノードで上書きする
	void
	copy(const PBCTNode& src, bool isNotRoot, Os::Memory::Size size);

	// 整合性を検査する
	void
	verify(const File& file, bool isNotRoot,
		   Admin::Verification::Progress& result) const;

	// なん番目に記録されている子ノードのブロック識別子を設定する
	void
	setChildID(unsigned int i, Block::ID id);
	// なん番目に記録されている子ノードのブロック識別子を得る
	Block::ID
	getChildID(unsigned int i) const;

	// 記録されている下位のノードの数
	unsigned int			_count;
	// 下位のノードのブロック識別子を記録する配列
	Block::ID				_child[1];
};

//	CLASS
//	Version::VersionLog::PBCTLeaf --
//		バージョンログファイルのページ→ブロック識別子変換木(PBCT)の
//		リーフを表すクラス
//
//	NOTES

class PBCTLeaf
{
	friend class File;
	friend class FileHeader;
public:
	// 確保する
	static Block::Memory
	allocate(const Trans::Transaction& trans,
			 Verification* verification, File& file);
	static Block::Memory
	allocate(const Trans::Transaction& trans,
			 Verification* verification, File& file,
			 MultiplexBlock& headerMulti);
	// 使用済にする
	static void
	free(const Trans::Transaction& trans,
		 VersionNumber::Value v,
		 Verification* verification, File& file, Block::ID id);
	// フィックスする
	static Block::Memory
	fix(const Trans::Transaction& trans,
		Verification* verification,
		File& file, Block::ID id, Buffer::Page::FixMode::Value mode);

	// PBCT リーフが格納されているブロックから PBCT リーフを得る
	static PBCTLeaf&
	get(Block::Memory& memory);
	static const PBCTLeaf&
	get(const Block::Memory& memory);

	// あるバージョンページの最新版のバージョンログのブロック識別子を設定する
	void
	setLatestID(Page::ID pageID, Block::ID id, Os::Memory::Size size);
	// あるバージョンページの最新版のバージョンログのブロック識別子を得る
	Block::ID
	getLatestID(Page::ID pageID, Os::Memory::Size size) const;

	// あるバージョンページの最古版の最終更新時タイムスタンプ値を設定する(V1)
	void
	setOldestTimeStamp(Page::ID pageID,
					   Trans::TimeStamp::Value t, Os::Memory::Size size);
	// あるバージョンページの最古版の最終更新時タイムスタンプ値を得る(V1)
	Trans::TimeStamp::Value
	getOldestTimeStamp(Page::ID pageID, Os::Memory::Size size) const;

	// あるバージョンページの最新版の最終更新時タイムスタンプ値を設定する(V2)
	void
	setNewestTimeStamp(Page::ID pageID,
					   Trans::TimeStamp::Value t, Os::Memory::Size size);
	// あるバージョンページの最新版の最終更新時タイムスタンプ値を得る(V2)
	Trans::TimeStamp::Value
	getNewestTimeStamp(Page::ID pageID, Os::Memory::Size size) const;

	// 記録されている最新版のバージョンログの数を得る
	unsigned int
	getCount() const;
	// 記録可能な最新版のバージョンログの数を得る
	static unsigned int
	getCountMax(bool isNotRoot, Os::Memory::Size size);

	// あるブロック識別子のブロックを含む多重化されたブロックのうち、
	// 先頭のもののブロック識別子を得る
	static Block::ID
	normalizeID(Block::ID id);

private:
	// 初期化する
	void
	initialize(bool isNotRoot, Os::Memory::Size size);
	// 与えられたリーフで上書きする
	void
	copy(const PBCTLeaf& src, bool isNotRoot, Os::Memory::Size size);

	// 整合性を検査する
	void
	verify(const File& file, bool isNotRoot,
		   Admin::Verification::Progress& result) const;

	// なん番目に記録されている最新版を記録するブロック識別子を設定する
	void
	setLatestID(unsigned int i, Block::ID id);
	// なん番目に記録されている最新版を記録するブロック識別子を得る
	Block::ID
	getLatestID(unsigned int i) const;

	// なん番目に記録されている最古版の最終更新時タイムスタンプ値を設定する(V1)
	void
	setOldestTimeStamp(unsigned int i, Trans::TimeStamp::Value t);
	// なん番目に記録されている最古版の最終更新時タイムスタンプ値を得る(V1)
	Trans::TimeStamp::Value
	getOldestTimeStamp(unsigned int i) const;

	// なん番目に記録されている最新版の最終更新時タイムスタンプ値を設定する(V2)
	void
	setNewestTimeStamp(unsigned int i, Trans::TimeStamp::Value t);
	// なん番目に記録されている最新版の最終更新時タイムスタンプ値を得る(V2)
	Trans::TimeStamp::Value
	getNewestTimeStamp(unsigned int i) const;

	// 記録されている最新版のブロックの数
	unsigned int			_count;
	// 最新版のブロック識別子を記録する配列
	Block::ID				_latest[1];
};

//	CLASS
//	Version::VersionLog::Log --
//		バージョンログファイルのバージョンログを表すクラス
//
//	NOTES

class Log
{
	friend class File;
public:
	//	CLASS
	//	Version::VersionLog::Log::Category -- バージョンログの種別を表すクラス
	//
	//	NOTES

	struct Category
	{
		//	ENUM
		//	Version::VersionLog::Log::Category::Value --
		//		バージョンログの種別を表す値の列挙型
		//
		//	NOTES

		enum Value
		{
			// 不明
			Unknown =		0,
			// マスタデータファイルの最古の版
			Oldest,
			// 古い版のたんなるコピー
			Copy,
			// バージョンログファイルの新しい版
			Newer,
			// 値の数
			ValueNum
		};
	};

	// 確保する
	static Block::Memory
	allocate(const Trans::Transaction& trans,
			 Verification* verification, File& file,
			 Buffer::ReplacementPriority::Value priority);
	static Block::Memory
	allocate(const Trans::Transaction& trans,
			 Verification* verification, File& file,
			 MultiplexBlock& headerMulti,
			 Buffer::ReplacementPriority::Value priority);
	// 使用済にする
	static void
	free(const Trans::Transaction& trans,
		 VersionNumber::Value v,
		 Verification* verification, File& file, Block::ID id);
	// フィックスする
	static Block::Memory
	fix(const Trans::Transaction& trans, Verification* verification,
		File& file, Block::ID id, Buffer::Page::FixMode::Value mode,
		Buffer::ReplacementPriority::Value priority);

	// バージョンログが格納されているブロックからバージョンログを得る
	static Log&
	get(Block::Memory& memory);
	static const Log&
	get(const Block::Memory& memory);

	// バージョンログを更新したことにする
	void					dirty();

	// 直前の版を記録するバージョンログのブロック識別子
	Block::ID				_older;
	// 自分自身の物理ログを表すバージョンログのブロック識別子
	Block::ID				_physicalLog;
	// 直前の版を記録するバージョンログの最終更新時タイムスタンプ値
	Trans::TimeStamp::Value	_olderTimeStamp;
	// 種別
	Category::Value			_category;
	// ページID -- verify時に利用するために記述する
	Block::ID				_pageID;
};

//	CLASS
//	Version::VersionLog::MultiplexInfo --
//		バージョンログファイルの多重化されたブロックのうち、
//		どのブロックを選択するかを決めるための情報を表すクラス
//
//	NOTES

class MultiplexInfo
{
	friend class File;
public:
	// クラスを生成する
	static MultiplexInfo*	attach(File& file, Block::ID id);
	// クラスを破棄する
	static void				detach(File& file, Block::ID id);

	// 多重化されたブロックのうち、先頭のもののブロック識別子を得る
	Block::ID
	getID() const;

private:
	// コンストラクター
	MultiplexInfo(Block::ID id);
	// デストラクター
	~MultiplexInfo();

	// 排他制御用のラッチを得る
	Os::CriticalSection&	getLatch() const;

	// 排他制御用のラッチ
	mutable	Os::CriticalSection	_latch;

	// 多重化されたブロックの先頭のもののブロック識別子
	const Block::ID			_id;
	// 多重化されたブロックのうち、それぞれの最終更新時タイムスタンプ
	Trans::TimeStamp::Value	_lastModification[MultiplexCount];

	// ハッシュリストでの直前の要素へのポインタ
	MultiplexInfo*			_hashPrev;
	// ハッシュリストでの直後の要素へのポインタ
	MultiplexInfo*			_hashNext;
};

//	CLASS
//	Version::VersionLog::MultiplexBlock --
//		バージョンログファイルの多重化されたブロックに関する情報を表すクラス
//
//	NOTES

class MultiplexBlock
{
public:
	// コンストラクター
	MultiplexBlock();
	// デストラクター
	~MultiplexBlock();

	// 多重化されたブロックのバッファリング内容
	Block::Memory			_memories[MultiplexCount];
	// マスタブロックは先頭からなん番目か
	unsigned int			_master;
};

//	FUNCTION public
//	Version::VersionLog::File::~File --
//		バージョンログファイル記述子を表すクラスのデストラクター
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
//	Version::VersionLog::File::mount -- マウントする
//
//	NOTES
//		すでにマウントされているバージョンログファイルや、
//		生成されていないバージョンログファイルを
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
	// バージョンログファイルの実体である
	// バッファファイルをマウントする
	//
	//【注意】	マウントするバッファファイルの実体である
	//			OS ファイルが存在しなくても、例外を発生しない

	_bufFile->mount(false);
}

//	FUNCTION public
//	Version::VersionLog::File::flush -- フラッシュする
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
	// バージョンログファイルの実体である
	// バッファファイルをフラッシュする

	_bufFile->flush();
}

//	FUNCTION private
//	Version::VersionLog::File::allocate --
//		ブロックを(必要があれば確保し、)使用中にする
//
//	NOTES
//		使用中にする連続したブロックの数として
//		0 を指定したときの動作は保証しない
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				ブロックを確保するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				ブロックを確保するトランザクションは整合性検査中でない
//		unsigned int		n
//			指定されたとき
//				使用中にする連続したブロックの数
//			指定されないとき
//				1 が指定されたものとみなす
//
//	RETURN
//		確保した連続したブロックのうちの最初のもののブロック識別子
//
//	EXCEPTIONS

inline
Block::ID
File::allocate(const Trans::Transaction& trans,
			   Verification* verification, unsigned int n)
{
	// ファイルヘッダをフィックスし、
	// それを使ってブロックを使用中にする

	MultiplexBlock headerMulti;
	FileHeader::fix(
		trans, verification, *this, Buffer::Page::FixMode::Write, headerMulti);

	return allocate(trans, verification, headerMulti, n);
}

//	FUNCTION public
//	Version::VersionLog::File::traversePBCT --
//		あるバージョンページの最新版を表すバージョンログの
//		ブロック識別子を記録する PBCT のリーフを得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				ある最新版を表すバージョンログのブロック識別子を記録する
//				PBCT のリーフを探すトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				ある最新版を表すバージョンログのブロック識別子を記録する
//				PBCT のリーフを探すトランザクションは整合性検査中でない
//		Version::Page::ID	pageID
//			このバージョンページ識別子の表すバージョンページの
//			最新版を表すバージョンログのブロック識別子を
//			記録する PBCT のリーフを探す
//		Buffer::Page::FixMode::Value	mode
//			Buffer::Page::FixMode::ReadOnly
//				見つかった PBCT のリーフは参照用にフィックスされる
//			Buffer::Page::FixMode::Write または
//			Buffer::Page::FixMode::Allocate
//				見つかった PBCT のリーフは更新用にフィックスされる
//
//	RETURN
//		見つかった PBCT のリーフを表す PBCT ノードのバッファリング内容
//
//		ただし、それに対する Version::Block::Memory::getID の結果が
//		0 のとき、PBCT のリーフが存在しないことを表し、
//		その場合、指定されたバージョンページ識別子の
//		バージョンページの最新版は、バージョンログファイルに格納されていない
//
//	EXCEPTIONS

inline
Block::Memory
File::traversePBCT(const Trans::Transaction& trans,
				   Verification* verification,
				   Page::ID pageID, Buffer::Page::FixMode::Value mode)
{
	// ファイルヘッダをフィックスし、それを使って、PBCT を辿る

	return traversePBCT(
		trans, verification,
		FileHeader::fix(trans, verification, *this,
						(mode != Buffer::Page::FixMode::Allocate) ?
						mode : Buffer::Page::FixMode::Write),
		pageID, mode);
}

//	FUNCTION public
//	Version::VersionLog::File::allocatePBCT --
//		あるバージョンページの最新版を表すバージョンログの
//		ブロック識別子を記録する PBCT ノードを確保する
//
//	NOTES
//		PBCT のルートノード以下で探した結果、見つからなければ、
//		記録できるように複数の PBCT ノードを確保する
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				ある最新版を表すバージョンログのブロック識別子を記録する
//				PBCT ノードを確保するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				ある最新版を表すバージョンログのブロック識別子を記録する
//				PBCT ノードを確保するトランザクションは整合性検査中でない
//		Version::Page::ID	pageID
//			このバージョンページ識別子のバージョンページの最新版を表す
//			バージョンログのブロック識別子を記録する PBCT ノードを確保する
//
//	RETURN
//		確保された PBCT ノードのバッファリング内容
//
//	EXCEPTIONS

inline
Block::Memory
File::allocatePBCT(const Trans::Transaction& trans,
				   Verification* verification, Page::ID pageID)
{
	// ファイルヘッダをフィックスし、
	// それを使って PBCT ノードを確保する

	MultiplexBlock headerMulti;
	FileHeader::fix(
		trans, verification, *this, Buffer::Page::FixMode::Write, headerMulti);

	return allocatePBCT(trans, verification, headerMulti, pageID);
}

//	FUNCTION public
//	Version::VersionLog::File::allocateLog --
//		あるバージョンページの新たな最新版を表すバージョンログを確保する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			新たな最新版を表すバージョンログを確保する
//			トランザクションのトランザクション記述子
//		Version::Verification*	verification
//			0 以外の値
//				新たな最新版を表すバージョンログを確保するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				新たな最新版を表すバージョンログを確保するトランザクションは
//				整合性検査中でない
//		Version::Page&		page
//			このバージョンページ記述子のバージョンページの
//			新たな最新版を表すバージョンログを確保する
//		Version::Block::Memory&	src
//			あるバージョンページの現在の最新版のバッファリング内容
//		Trans::TimeStamp::Value	oldest
//			最古のバージョンログの最終更新時タイムスタンプ値
//		Buffer::Replacement::Priority::Value priority
//			Buffer::ReplacementPriority::Low
//				確保するバージョンログは、バッファから破棄されやすい
//			Buffer::ReplacementPriority::Middle
//				確保するバージョンログは、バッファにある程度残る
//			Buffer::ReplacementPriority::High
//				確保するバージョンログは、バッファからかなりの間残る
//
//	RETURN
//		確保されたバージョンログのバッファリング内容
//
//	EXCEPTIONS

inline
Block::Memory
File::allocateLog(const Trans::Transaction& trans,
				  Verification* verification, Page& page,
				  Block::Memory& src, Trans::TimeStamp::Value oldest,
				  Buffer::ReplacementPriority::Value priority)
{
	// ファイルヘッダをフィックスし、
	// それを使ってバージョンログを確保する

	MultiplexBlock headerMulti;
	FileHeader::fix(
		trans, verification, *this, Buffer::Page::FixMode::Write, headerMulti);

	return allocateLog(
		trans, verification, headerMulti, page, src, oldest, priority);
}

//	FUNCTION public
//	Version::VersionLog::File::freePBCT --
//		あるバージョンページの最新版のブロック識別子を得るためにたどる
//		PBCT ノード、リーフを可能な限り使用済にする
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				PBCT ノード、リーフを使用済にするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				PBCT ノード、リーフを使用済にする
//				トランザクションは整合性検査中でない
//		Version::Page::ID	pageID
//			このバージョンページ識別子のバージョンページの
//			最新版のブロック識別子を得るためにたどる
//			PBCT ノード、リーフを可能な限り使用済にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
File::freePBCT(const Trans::Transaction& trans,
			   Verification* verification, Page::ID pageID)
{
	// ファイルヘッダをフィックスし、
	// それを使って可能な限り PBCT ノードを使用済にする

	Block::Memory	headerMemory(
		FileHeader::fix(trans,
						verification, *this, Buffer::Page::FixMode::Write));
	freePBCT(trans, verification, headerMemory, pageID);
}

//	FUNCTION public
//	Version::VersionLog::File::getSize --
//		バージョンログファイルのファイルサイズを得る
//
//	NOTES
//		バージョンログファイルが生成されていないとき、
//		ファイルサイズは 0 とみなす
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
//	Version::VersionLog::File::getBoundSize --
//		使用中のブロックの総サイズを求める
//
//	NOTES
//		バージョンログファイルが生成されていないとき、
//		使用中のブロックの総サイズは 0 とみなす
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				使用中のブロックの総サイズを求めるトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				使用中のブロックの総サイズを求めるトランザクションは
//				整合性検査中でない
//
//	RETURN
//		求めたサイズ(B 単位)
//
//	EXCEPTIONS

inline
Os::File::Size
File::getBoundSize(const Trans::Transaction& trans,
				   Verification* verification)
{
	if (isMountedAndAccessible()) {

		// ファイルヘッダをフィックスし、それを使って、
		// 使用中のブロックの総サイズを求める

		const Block::Memory& headerMemory =
			FileHeader::fix(
				trans, verification, *this, Buffer::Page::FixMode::ReadOnly);
		return getBoundSize(trans, verification, FileHeader::get(headerMemory));
	}
	return 0;
}

//	FUNCTION public
//	Version::VersionLog::File::getBlockSize --
//		バージョンログファイルのブロックサイズを得る
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
//	Version::VersionLog::File::getParent --
//		バージョンログファイルを格納する OS ファイルの
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
//	Version::VersionLog::File::getSizeMax --
//		バージョンログファイルの最大ファイルサイズを得る
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
//	Version::VersionLog::File::getExtensionSize --
//		バージョンログファイルのファイル拡張サイズを得る
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
//	Version::VersionLog::File::getPoolCategory --
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
//	Version::VersionLog::File::isAccessible --
//		バージョンログファイルを構成する OS ファイルが存在するか調べる
//
//	NOTES
//
//	ARGUMENTS
//		bool				force
//			true
//				バージョンログファイルの実体である
//				OS ファイルの存在を実際に調べる
//			false または指定されないとき
//				バージョンログファイルの実体である
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
//	Version::VersionLog::File::isMounted -- マウントされているか調べる
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
//	Version::VersionLog::File::isMountedAndAccessible --
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
//	Version::VersionLog::File::isReadOnly -- 読取専用か調べる
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
//	Version::VersionLog::FileHeader::fix --
//		ファイルヘッダを格納するマスタブロックをフィックスする
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				マスタブロックをフィックスするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				マスタブロックをフィックスする
//				トランザクションは整合性検査中でない
//		Version::VersionLog::File&	file
//			フィックスするマスタブロックが存在する
//			バージョンログファイルのバージョンログファイル記述子
//		Buffer::Page::FixMode::Value	mode
//			Buffer::Page::FixMode::ReadOnly
//				フィックスするマスタブロックは参照しかされない
//			Buffer::Page::FixMode::Write
//				フィックスするマスタブロックは更新される
//			Buffer::Page::FixMode::Allocate
//				フィックスするマスタブロックは
//				その領域の初期化のために使用する
//
//	RETURN
//		フィックスしたマスタブロックのバッファリング内容
//
//	EXCEPTIONS

// static
inline
Block::Memory
FileHeader::fix(const Trans::Transaction& trans,
				Verification* verification,
				File& file, Buffer::Page::FixMode::Value mode)
{
	return file.fixMaster(
		trans, verification, 0, mode, Buffer::ReplacementPriority::High);
}

//	FUNCTION public
//	Version::VersionLog::FileHeader::fix --
//		ファイルヘッダを格納するマスタおよびスレーブブロックをフィックスする
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				ブロック達をフィックスするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				ブロック達をフィックスする
//				トランザクションは整合性検査中でない
//		Version::VersionLog::File&	file
//			フィックスするブロック達が存在する
//			バージョンログファイルのバージョンログファイル記述子
//		Buffer::Page::FixMode::Value	mode
//			Buffer::Page::FixMode::ReadOnly
//				フィックスするブロック達は参照しかされない
//			Buffer::Page::FixMode::Write
//				フィックスするブロック達は更新される
//			Buffer::Page::FixMode::Allocate
//				フィックスするブロック達はその領域の初期化のために使用する
//		Version::VersionLog::MultiplexBlock& multi
//			マスタブロックおよびリカバリによって将来回復される
//			可能性のあるスレーブブロックに関する情報が設定される
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
inline
void
FileHeader::fix(const Trans::Transaction& trans,
				Verification* verification,	File& file,
				Buffer::Page::FixMode::Value mode, MultiplexBlock& multi)
{
	file.fixMasterAndSlaves(
		trans, verification, 0, mode, Buffer::ReplacementPriority::High, multi);
}

//	FUNCTION public
//	Version::VersionLog::FileHeader::get --
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
	return *static_cast<FileHeader*>(static_cast<void*>(
		memory.operator char*() + (memory.getSize() - sizeof(FileHeader))));
}

// static
inline
const FileHeader&
FileHeader::get(const Block::Memory& memory)
{
	return *static_cast<const FileHeader*>(static_cast<const void*>(
	memory.operator const char*() + (memory.getSize() - sizeof(FileHeader))));
}

//	FUNCTION public
//	Version::VersionLog::FileHeader::getPageCount --
//		総バージョンページ数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた総バージョンページ数
//
//	EXCEPTIONS
//		なし

inline
unsigned int
FileHeader::getPageCount() const
{
	return _pageCount;
}

//	FUNCTION public
//	Version::VersionLog::FileHeader::setPageCount --
//		総バージョンページ数を設定する
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		n
//			設定する総バージョンページ数の値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
FileHeader::setPageCount(unsigned int n)
{
	_pageCount = n;
}

//	FUNCTION public
//	Version::VersionLog::FileHeader::getPBCTLevel -- PBCT のレベル数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた PBCT のレベル数
//
//	EXCEPTIONS
//		なし

inline
PBCTLevel::Value
FileHeader::getPBCTLevel() const
{
	return _PBCTLevel;
}

//	FUNCTION public
//	Version::VersionLog::FileHeader::isPBCTEmpty -- PBCT が空か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			PBCT は空である
//		false
//			PBCT は空でない
//
//	EXCEPTIONS
//		なし

inline
bool
FileHeader::isPBCTEmpty() const
{
	return getPBCTLevel() < 0;
}

//	FUNCTION public
//	Version::VersionLog::FileHeader::getVersion
//		-- バージョン番号を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Version::VersionNumber::Value
//		   	バージョン番号
//
//	EXCEPTIONS
//		なし

inline
VersionNumber::Value
FileHeader::getVersion() const
{
	return _versionNumber;
}

//	FUNCTION public
//	Version::VersionLog::AllocationTable::allocate --
//		バージョンログファイルにアロケーションテーブルを確保する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				アロケーションテーブルを確保するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				アロケーションテーブルを確保するトランザクションは
//				整合性検査中でない
//		Version::VersionLog::File&	file
//			アロケーションテーブルを確保する
//			バージョンログファイルのバージョンログファイル記述子
//
//	RETURN
//		確保したアロケーションテーブルを記録するブロックのバッファリング内容
//
//	EXCEPTIONS

// static
inline
Block::Memory
AllocationTable::allocate(const Trans::Transaction& trans,
						  Verification* verification, VersionLog::File& file)
{
	// ファイルヘッダをフィックスし、
	// それを使ってアロケーションテーブルを確保する

	MultiplexBlock headerMulti;
	FileHeader::fix(
		trans, verification, file, Buffer::Page::FixMode::Write, headerMulti);

	return allocate(trans, verification, file, headerMulti);
}

//	FUNCTION public
//	Version::VersionLog::AllocationTable::fix --
//		アロケーションテーブルを格納するマスタブロックをフィックスする
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				マスタブロックをフィックスするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				マスタブロックをフィックスする
//				トランザクションは整合性検査中でない
//		Version::VersionLog::File&	file
//			フィックスするマスタブロックが存在する
//			バージョンログファイルのバージョンログファイル記述子
//		Version::Block::ID	id
//			アロケーションテーブルが格納される
//			多重化されたブロックの先頭のもののブロック識別子
//		Buffer::Page::FixMode::Value	mode
//			Buffer::Page::FixMode::ReadOnly
//				フィックスするマスタブロックは参照しかされない
//			Buffer::Page::FixMode::Write
//				フィックスするマスタブロックは更新される
//			Buffer::Page::FixMode::Allocate
//				フィックスするマスタブロックは
//				その領域の初期化のために使用する
//
//	RETURN
//		フィックスしたマスタブロックのバッファリング内容
//
//	EXCEPTIONS

// static
inline
Block::Memory
AllocationTable::fix(
	const Trans::Transaction& trans, Verification* verification,
	File& file, Block::ID id, Buffer::Page::FixMode::Value mode)
{
	return file.fixMaster(
		trans, verification, id, mode, Buffer::ReplacementPriority::High,
		initializeBlock);
}

//	FUNCTION public
//	Version::VersionLog::AllocationTable::fix --
//		アロケーションテーブルを格納する
//		マスタおよびスレーブブロックをフィックスする
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				ブロック達をフィックスするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				ブロック達をフィックスする
//				トランザクションは整合性検査中でない
//		Version::VersionLog::File&	file
//			フィックスするブロック達が存在する
//			バージョンログファイルのバージョンログファイル記述子
//		Version::Block::ID	id
//			アロケーションテーブルが格納される
//			多重化されたブロックの先頭のもののブロック識別子
//		Buffer::Page::FixMode::Value	mode
//			Buffer::Page::FixMode::ReadOnly
//				フィックスするブロック達は参照しかされない
//			Buffer::Page::FixMode::Write
//				フィックスするブロック達は更新される
//			Buffer::Page::FixMode::Allocate
//				フィックスするブロック達はその領域の初期化のために使用する
//		Version::VersionLog::MultiplexBlock& multi
//			マスタブロックおよびリカバリによって将来回復される
//			可能性のあるスレーブブロックに関する情報が設定される
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
inline
void
AllocationTable::fix(
	const Trans::Transaction& trans,
	Verification* verification, File& file, Block::ID id,
	Buffer::Page::FixMode::Value mode, MultiplexBlock& multi)
{
	file.fixMasterAndSlaves(
		trans, verification, id, mode, Buffer::ReplacementPriority::High, multi,
		initializeBlock);
}

//	FUNCTION public
//	Version::VersionLog::AllocationTable::get --
//		アロケーションテーブルが格納されているブロックから
//		アロケーションテーブルを得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::Memory&	memory
//			アロケーションテーブルが格納されているブロックのバッファリング内容
//
//	RETURN
//		得られたアロケーションテーブルへのリファレンス
//
//	EXCEPTIONS
//		なし

// static
inline
AllocationTable&
AllocationTable::get(Block::Memory& memory)
{
	return *static_cast<AllocationTable*>(memory.operator void*());
}

// static
inline
const AllocationTable&
AllocationTable::get(const Block::Memory& memory)
{
	return *static_cast<const AllocationTable*>(memory.operator const void*());
}

//	FUNCTION private
//	Version::VersionLog::AllocationTable::isApplyFree --
//		使用済み(フリーした)ブロックを反映させたかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		true	反映済みの場合
//		false	反映していない場合
//
//	EXCEPTIONS

inline
bool
AllocationTable::isApplyFree() const
{
	return (_count & FREE_BIT) != 0;
}

//	FUNCTION private
//	Version::VersionLog::AllocationTable::setApplyFree --
//		使用済み(フリーした)ブロックを反映させたことにする
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

inline
void
AllocationTable::setApplyFree()
{
	_count |= FREE_BIT;
}

//	FUNCTION private
//	Version::VersionLog::AllocationTable::unsetApplyFree --
//		使用済み(フリーした)ブロックを反映させてないことにする
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

inline
void
AllocationTable::unsetApplyFree()
{
	_count &= ~(FREE_BIT);
}

//	FUNCTION private
//	Version::VersionLog::AllocationTable::getCount --
// 		使用中のブロック数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	unsigned int
//		使用中のブロックする
//
//	EXCEPTIONS

inline
unsigned int
AllocationTable::getCount() const
{
	return _count & ~(FREE_BIT);
}

//	FUNCTION public
//	Version::VersionLog::PBCTNode::allocate --
//		バージョンログファイルに PBCT ノードを確保する
//
//	NOTES
//		Version::Verification*	verification
//			0 以外の値
//				PBCT ノードを確保するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				PBCT ノードを確保するトランザクションは整合性検査中でない
//		Version::VersionLog::File&	file
//			PBCT ノードを確保する
//			バージョンログファイルのバージョンログファイル記述子
//
//	RETURN
//		確保した PBCT ノードを記録するブロックのバッファリング内容
//
//	EXCEPTIONS

// static
inline
Block::Memory
PBCTNode::allocate(const Trans::Transaction& trans,
				   Verification* verification, VersionLog::File& file)
{
	// ファイルヘッダをフィックスし、
	// それを使って PBCT ノードを確保する

	MultiplexBlock headerMulti;
	FileHeader::fix(
		trans, verification, file, Buffer::Page::FixMode::Write, headerMulti);

	return allocate(trans, verification, file, headerMulti);
}

//	FUNCTION public
//	Version::VersionLog::PBCTNode::fix -- PBCT ノードをフィックスする
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				PBCT ノードをフィックスするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				PBCT ノードをフィックスするトランザクションは
//				整合性検査中でない
//		Version::VersionLog::File&	file
//			フィックスする PBCT ノードが存在する
//			バージョンログファイルのバージョンログファイル記述子
//		Version::Block::ID	id
//			フィックスする PBCT ノードが存在するブロックのブロック識別子
//		Buffer::Page::FixMode::Value	mode
//			Buffer::Page::FixMode::ReadOnly
//				フィックスする PBCT ノードは参照しかされない
//			Buffer::Page::FixMode::Write
//				フィックスする PBCT ノードは更新される
//			Buffer::Page::FixMode::Allocate
//				フィックスする PBCT ノードはその領域の初期化のために使用する
//
//	RETURN
//		フィックスした PBCT ノードのバッファリング内容
//
//	EXCEPTIONS

// static
inline
Block::Memory
PBCTNode::fix(const Trans::Transaction& trans,
			  Verification* verification,
			  File& file, Block::ID id, Buffer::Page::FixMode::Value mode)
{
	return file.fixMaster(
		trans, verification, id, mode, Buffer::ReplacementPriority::High);
}

//	FUNCTION public
//	Version::VersionLog::PBCTNode::get --
//		PBCT ノードが格納されているブロックから PBCT ノードを得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::Memory&	memory
//			PBCT ノードが格納されているブロックのバッファリング内容
//
//	RETURN
//		得られた PBCT ノードへのリファレンス
//
//	EXCEPTIONS
//		なし

// static
inline
PBCTNode&
PBCTNode::get(Block::Memory& memory)
{
	return *static_cast<PBCTNode*>(memory.operator void*());
}

// static
inline
const PBCTNode&
PBCTNode::get(const Block::Memory& memory)
{
	return *static_cast<const PBCTNode*>(memory.operator const void*());
}

//	FUNCTION public
//	Version::VersionLog::PBCTNode::setChildID --
//		リーフに最新版のバージョンログを記憶するブロック識別子を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Page::ID	pageID
//			このバージョンページ識別子の表すバージョンページの最新版の
//			バージョンログを記憶するブロック識別子を設定しようとしている
//		Version::Block::ID	id
//			設定するブロック識別子
//		Version::VersionLog::PBCTLevel::Value	level
//			ブロック識別子を設定するリーフを含む PBCT の深さ(0 以上)
//		Os::Memory::Size	size
//			ブロック識別子を設定するリーフが存在するブロックのサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
PBCTNode::setChildID(Page::ID pageID, Block::ID id,
					 PBCTLevel::Value level, Os::Memory::Size size)
{
	setChildID(pageID, level, id, level, size);
}

//	FUNCTION public
//	Version::VersionLog::PBCTNode::getChildID --
//		最新版のバージョンログのブロック識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::Page::ID	pageID
//			このバージョンページ識別子の表すバージョンページの
//			最新版のバージョンログのブロック識別子を得ようとしている
//		Version::VersionLog::PBCTLevel::Value	level
//			得ようとしているブロック識別子が設定されている
//			リーフを含む PBCT の深さ(0 以上)
//		Os::Memory::Size	size
//			得ようとしているブロック識別子が設定されている
//			リーフが存在するブロックのサイズ(B 単位)
//
//	RETURN
//		得られたブロック識別子
//
//	EXCEPTIONS
//		なし

inline
Block::ID
PBCTNode::getChildID(
	Page::ID pageID, PBCTLevel::Value level, Os::Memory::Size size) const
{
	return getChildID(pageID, level, level, size);
}

//	FUNCTION private
//	Version::VersionLog::PBCTNode::getChildID -- 子ノードのブロック識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		i
//			得ようとしているブロック識別子がなん番目に記録されているものか
//
//	RETURN
//		得られたブロック識別子
//
//	EXCEPTIONS
//		なし

inline
Block::ID
PBCTNode::getChildID(unsigned int i) const
{
	return _child[i];
}

//	FUNCTION public
//	Version::VersionLog::PBCTNode::getCount --
//		記録されている子ノードのブロック識別子の数を求める
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた記録されているブロック識別子の数
//
//	EXCEPTIONS
//		なし

inline
unsigned int
PBCTNode::getCount() const
{
	return _count;
}

//	FUNCTION public
//	Version::VersionLog::PBCTNode::getCountMax --
//		あるブロックサイズのブロックに
//		PBCT ノードがあるときの記録可能な子ノードのブロック識別子の数を求める
//
//	NOTES
//
//	ARGUMENTS
//		bool				isNotRoot
//			true
//				記録可能なブロック識別子の数を求める PBCT ノードはルートでない
//			false
//				記録可能なブロック識別子の数を求める PBCT ノードはルートである
//		Os::Memory::Size	size
//			記録可能なブロック識別子の数を求める PBCT ノードが
//			存在するブロックのサイズ(B 単位)
//
//	RETURN
//		得られた記録可能なブロック識別子の数
//
//	EXCEPTIONS
//		なし

// static
inline
unsigned int
PBCTNode::getCountMax(bool isNotRoot, Os::Memory::Size size)
{
	return (Block::getContentSize(size) - sizeof(unsigned int) -
			(isNotRoot ? 0 : sizeof(FileHeader))) / sizeof(Block::ID);
}

//	FUNCTION public
//	Version::VersionLog::PBCTNode::normalizeID --
//		あるブロック識別子のブロックを含む
//		PBCT ノードの多重化されたブロックのうち、
//		先頭のもののブロック識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::ID	id
//			このブロック識別子のブロックを含む
//			PBCT ノードの多重化されたブロックのうち、
//			先頭のもののブロック識別子を得る
//
//	RETURN
//		得られたブロック識別子
//
//	EXCEPTIONS
//		なし

// static
inline
Block::ID
PBCTNode::normalizeID(Block::ID id)
{
	return id / MultiplexCount * MultiplexCount;
}

//	FUNCTION public
//	Version::VersionLog::PBCTLeaf::allocate --
//		バージョンログファイルに PBCT リーフを確保する
//
//	NOTES
//		Version::Verification*	verification
//			0 以外の値
//				PBCT リーフを確保するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				PBCT リーフを確保するトランザクションは整合性検査中でない
//		Version::VersionLog::File&	file
//			PBCT リーフを確保する
//			バージョンログファイルのバージョンログファイル記述子
//
//	RETURN
//		確保した PBCT リーフを記録するブロックのバッファリング内容
//
//	EXCEPTIONS

// static
inline
Block::Memory
PBCTLeaf::allocate(const Trans::Transaction& trans,
				   Verification* verification, VersionLog::File& file)
{
	// ファイルヘッダをフィックスし
	// それを使って PBCT リーフを確保する

	MultiplexBlock headerMulti;
	FileHeader::fix(
		trans, verification, file, Buffer::Page::FixMode::Write, headerMulti);

	return allocate(trans, verification, file, headerMulti);
}

//	FUNCTION public
//	Version::VersionLog::PBCTLeaf::fix -- PBCT リーフをフィックスする
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				PBCT リーフをフィックスするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				PBCT リーフをフィックスするトランザクションは
//				整合性検査中でない
//		Version::VersionLog::File&	file
//			フィックスする PBCT リーフが存在する
//			バージョンログファイルのバージョンログファイル記述子
//		Version::Block::ID	id
//			フィックスする PBCT リーフが存在するブロックのブロック識別子
//		Buffer::Page::FixMode::Value	mode
//			Buffer::Page::FixMode::ReadOnly
//				フィックスする PBCT リーフは参照しかされない
//			Buffer::Page::FixMode::Write
//				フィックスする PBCT リーフは更新される
//			Buffer::Page::FixMode::Allocate
//				フィックスする PBCT リーフはその領域の初期化のために使用する
//
//	RETURN
//		フィックスした PBCT リーフのバッファリング内容
//
//	EXCEPTIONS

// static
inline
Block::Memory
PBCTLeaf::fix(const Trans::Transaction& trans,
			  Verification* verification,
			  File& file, Block::ID id, Buffer::Page::FixMode::Value mode)
{
	return file.fixMaster(
		trans, verification, id, mode, Buffer::ReplacementPriority::High);
}

//	FUNCTION public
//	Version::VersionLog::PBCTLeaf::get --
//		PBCT リーフが格納されているブロックから PBCT リーフを得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::Memory&	memory
//			PBCT リーフが格納されているブロックのバッファリング内容
//
//	RETURN
//		得られた PBCT リーフへのリファレンス
//
//	EXCEPTIONS
//		なし

// static
inline
PBCTLeaf&
PBCTLeaf::get(Block::Memory& memory)
{
	return *static_cast<PBCTLeaf*>(memory.operator void*());
}

// static
inline
const PBCTLeaf&
PBCTLeaf::get(const Block::Memory& memory)
{
	return *static_cast<const PBCTLeaf*>(memory.operator const void*());
}

//	FUNCTION public
//	Version::VersivonLog::PBCTLeaf::setOldestTimeStamp --
//		あるバージョンページの最古版の最終更新時タイムスタンプ値を設定する(V1)
//
//	NOTES
//		これはV1用のメソッド
//		昔は最古の版の最終更新時のタイムスタンプが格納されていた
//		過去との互換性のためのメソッド
//
//	ARGUMENTS
//		Version::Page::ID	pageID
//			このバージョンページ識別子の表すバージョンページの
//			最古版の最終更新時タイムスタンプ値を設定しようとしている
//		Trans::TimeStamp::Value	t
//			設定するタイムスタンプ値
//		Os::Memory::Size	size
//			タイムスタンプ値を設定する PBCT リーフが
//			存在するブロックのサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
PBCTLeaf::setOldestTimeStamp(
	Page::ID pageID, Trans::TimeStamp::Value t, Os::Memory::Size size)
{
	setNewestTimeStamp(pageID, t, size);
}

//	FUNCTION public
//	Version::VersionLog::PBCTLeaf::getOldestTimeStamp --
//		あるバージョンページの最古版の最終更新時タイムスタンプ値を得る(V1)
//
//	NOTES
//		これはV1用のメソッド
//		昔は最古の版の最終更新時のタイムスタンプが格納されていた
//		過去との互換性のためのメソッド
//
//	ARGUMENTS
//		Version::Page::ID	pageID
//			このバージョンページ識別子の表すバージョンページの
//			最古版の最終更新時タイムスタンプ値を得ようとしている
//		Os::Memory::Size	size
//			得ようとしているタイムスタンプ値が設定されている
//			PBCT リーフが存在するブロックのサイズ(B 単位)
//
//
//	RETURN
//		得られたブ最終更新時タイムスタンプ値
//
//	EXCEPTIONS
//		なし

inline Trans::TimeStamp::Value
PBCTLeaf::getOldestTimeStamp(Page::ID pageID, Os::Memory::Size size) const
{
	return getNewestTimeStamp(pageID, size);
}

//	FUNCTION private
//	Version::VersionLog::PBCTLeaf::getLatestID --
//		なん番目に記録されている最新版のバージョンログのブロック識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		i
//			得ようとしているブロック識別子がなん番目に記録されているものか
//
//	RETURN
//		得られたブロック識別子
//
//	EXCEPTIONS
//		なし

inline
Block::ID
PBCTLeaf::getLatestID(unsigned int i) const
{
	return *syd_reinterpret_cast<const Block::ID*>(
		syd_reinterpret_cast<const char*>(_latest) +
		(sizeof(Block::ID) + sizeof(Trans::TimeStamp::Value)) * i);
}

//	FUNCTION private
//	Version::VersionLog::PBCTLeaf::setOldestTimeStamp --
//		あるバージョンページの最古版の最終更新時タイムスタンプ値を設定する
//
//	NOTES
//		これはV1用のメソッド
//		昔は最古の版の最終更新時のタイムスタンプが格納されていた
//		過去との互換性のためのメソッド
//
//	ARGUMENTS
//		unsigned int		i
//			設定しようとしている最終更新時タイムスタンプ値が
//			なん番目に記録されているものか
//		Trans::TimeStamp::Value	t
//			設定するタイムスタンプ値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
PBCTLeaf::setOldestTimeStamp(unsigned int i, Trans::TimeStamp::Value t)
{
	setNewestTimeStamp(i, t);
}

//	FUNCTION private
//	Version::VersionLog::PBCTLeaf::getOldestTimeStamp --
//		なん番目に記録されている最古版の最終更新時タイムスタンプ値を得る
//
//	NOTES
//		これはV1用のメソッド
//		昔は最古の版の最終更新時のタイムスタンプが格納されていた
//		過去との互換性のためのメソッド
//
//	ARGUMENTS
//		unsigned int		i
//			得ようとしている最終更新時タイムスタンプ値が
//			なん番目に記録されているものか
//
//	RETURN
//		得られたブ最終更新時タイムスタンプ値
//
//	EXCEPTIONS
//		なし

inline
Trans::TimeStamp::Value
PBCTLeaf::getOldestTimeStamp(unsigned int i) const
{
	return getNewestTimeStamp(i);
}

//	FUNCTION private
//	Version::VersionLog::PBCTLeaf::getNewestTimeStamp --
//		なん番目に記録されている最新版の最終更新時タイムスタンプ値を得る
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		i
//			得ようとしている最終更新時タイムスタンプ値が
//			なん番目に記録されているものか
//
//	RETURN
//		得られたブ最終更新時タイムスタンプ値
//
//	EXCEPTIONS
//		なし

inline
Trans::TimeStamp::Value
PBCTLeaf::getNewestTimeStamp(unsigned int i) const
{
	return *syd_reinterpret_cast<const Trans::TimeStamp::Value*>(
		syd_reinterpret_cast<const char*>(_latest) +
		(sizeof(Block::ID) + sizeof(Trans::TimeStamp::Value)) * i +
		sizeof(Block::ID));
}

//	FUNCTION public
//	Version::VersionLog::PBCTLeaf::getCount --
//		記録されている最新版のバージョンログのブロック識別子の数を求める
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた記録されているブロック識別子の数
//
//	EXCEPTIONS
//		なし

inline
unsigned int
PBCTLeaf::getCount() const
{
	return _count;
}

//	FUNCTION public
//	Version::VersionLog::PBCTLeaf::getCountMax --
//		あるブロックサイズのブロックに PBCT リーフがあるときの
//		記録可能な最新版のバージョンログのブロック識別子
//		または最古の版のタイムスタンプ値の数を求める
//
//	NOTES
//
//	ARGUMENTS
//		bool				isNotRoot
//			true
//				記録可能なブロック識別子またはタイムスタンプ値の数を求める
//				PBCT リーフはルートでない
//			false
//				記録可能なブロック識別子またはタイムスタンプ値の数を求める
//				PBCT リーフはルートである
//		Os::Memory::Size	size
//			記録可能なブロック識別子またはタイムスタンプ値の数を求める
//			PBCT リーフが存在するブロックのサイズ(B 単位)
//
//	RETURN
//		得られた記録可能なブロック識別子またはタイムスタンプ値の数
//
//	EXCEPTIONS
//		なし

// static
inline
unsigned int
PBCTLeaf::getCountMax(bool isNotRoot, Os::Memory::Size size)
{
	return (Block::getContentSize(size) - sizeof(unsigned int) -
		(isNotRoot ? 0 : sizeof(FileHeader))) /
		(sizeof(Block::ID) + sizeof(Trans::TimeStamp::Value));
}

//	FUNCTION public
//	Version::VersionLog::PBCTLeaf::normalizeID --
//		あるブロック識別子のブロックを含む
//		PBCT リーフの多重化されたブロックのうち、
//		先頭のもののブロック識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::ID	id
//			このブロック識別子のブロックを含む
//			PBCT リーフの多重化されたブロックのうち、
//			先頭のもののブロック識別子を得る
//
//	RETURN
//		得られたブロック識別子
//
//	EXCEPTIONS
//		なし

// static
inline
Block::ID
PBCTLeaf::normalizeID(Block::ID id)
{
	return id / MultiplexCount * MultiplexCount;
}

//	FUNCTION public
//	Version::VersionLog::Log::allocate --
//		バージョンログファイルにバージョンログを確保する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification*	verification
//			0 以外の値
//				バージョンログを確保するトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				バージョンログを確保するトランザクションは整合性検査中でない
//		Version::VersionLog::File&	file
//			バージョンログを確保する
//			バージョンログファイルのバージョンログファイル記述子
//		Buffer::ReplacementPriority::Value	priority
//			Buffer::ReplacementPriority::Low
//				確保するバージョンログは、バッファから破棄されやすい
//			Buffer::ReplacementPriority::Middle
//				確保するバージョンログは、バッファにある程度残る
//			Buffer::ReplacementPriority::High
//				確保するバージョンログは、バッファからかなりの間残る
//
//	RETURN
//		確保したバージョンログを記録するブロックのバッファリング内容
//
//	EXCEPTIONS

// static
inline
Block::Memory
Log::allocate(const Trans::Transaction& trans,
			  Verification* verification, VersionLog::File& file,
			  Buffer::ReplacementPriority::Value priority)
{
	// ファイルヘッダをフィックスし、
	// それを使ってバージョンログを確保する

	MultiplexBlock headerMulti;
	FileHeader::fix(
		trans, verification, file, Buffer::Page::FixMode::Write, headerMulti);

	return allocate(trans, verification, file, headerMulti, priority);
}

//	FUNCTION public
//	Version::VersionLog::Log::free --
//		バージョンログファイルのバージョンログを使用済にする
//
//	NOTES
//		Version::Verification*	verification
//			0 以外の値
//				バージョンログを使用済にするトランザクションが
//				実行中の整合性検査に関する情報を保持するクラスを
//				格納する領域の先頭アドレス
//			0
//				バージョンログを使用済にする
//				トランザクションは整合性検査中でない
//		Version::VersionLog::File&	file
//			バージョンログを使用済にする
//			バージョンログファイルのバージョンログファイル記述子
//		Version::Block::ID	id
//			使用済にするバージョンログが存在するブロックのブロック識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
inline
void
Log::free(const Trans::Transaction& trans, VersionNumber::Value v,
		  Verification* verification, VersionLog::File& file, Block::ID id)
{
	file.free(trans, v, verification, id);
}

//	FUNCTION public
//	Version::VersionLog::Log::get --
//		バージョンログが格納されているブロックからバージョンログを得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::Memory&	memory
//			バージョンログが格納されているブロックのバッファリング内容
//
//	RETURN
//		得られたバージョンログへのリファレンス
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

//	FUNCTION public
//	VersionLog::Log::dirty -- バージョンログを更新したことにする
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
void
Log::dirty()
{
	if (_category == Category::Copy)

		// 古い版が記録されているバージョンログを
		// たんにコピーしたものでなくする

		_category = Category::Newer;
}

//	FUNCTION private
//	Version::VersionLog::MultiplexInfo::MultiplexInfo --
//		バージョンログファイルの多重化されたブロックのうち、
//		どのブロックを選択するかを決めるための情報を表すクラスの
//		コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::ID	id
//			多重化されたブロックのうち、先頭のもののブロック識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
MultiplexInfo::MultiplexInfo(Block::ID id)
	: _id(id),
	  _hashPrev(0),
	  _hashNext(0)
{
	const Trans::TimeStamp::Value illegal = Trans::IllegalTimeStamp;

	unsigned int i = 0;
	do {
		_lastModification[i] = illegal;
	} while (++i < MultiplexCount) ;
}

//	FUNCTION private
//	Version::VersionLog::MultiplexInfo::~MultiplexInfo --
//		バージョンログファイルの多重化されたブロックのうち、
//		どのブロックを選択するかを決めるための情報を表すクラスの
//		デストラクター
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
MultiplexInfo::~MultiplexInfo()
{}

//	FUNCTION public
//	Version::VersionLog::MultiplexInfo::getID --
//		バージョンログファイルの多重化されたブロックのうち、
//		先頭のもののブロック識別子を得る
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
MultiplexInfo::getID() const
{
	return _id;
}

//	FUNCTION private
//	Version::VersionLog::MultiplexInfo::getLatch --
//		バージョンログファイルの多重化されたブロックのうち、
//		どのブロックを選択するかを決めるための情報を表すクラスの操作の
//		排他制御をするためのラッチを得る
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
MultiplexInfo::getLatch() const
{
	return _latch;
}

//	FUNCTION public
//	Version::VersionLog::MultiplexBlock::MultiplexBlock --
//		バージョンログファイルの多重化された
//		ブロックに関する情報を表すクラスのコンストラクター
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
MultiplexBlock::MultiplexBlock()
	: _master(MultiplexCount)
{}

//	FUNCTION public
//	Version::VersionLog::MultiplexBlock::~MultiplexBlock --
//		バージョンログファイルの多重化された
//		ブロックに関する情報を表すクラスのデストラクター
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
MultiplexBlock::~MultiplexBlock()
{}

_SYDNEY_VERSION_VERSIONLOG_END
_SYDNEY_VERSION_END
_SYDNEY_END

#endif	// __SYDNEY_VERSION_VERSIONLOG_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
