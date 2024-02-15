// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SyncLog.cpp -- 同期ログファイル関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2013, 2023 Ricoh Company, Ltd.
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

#include "Version/Configuration.h"
#include "Version/File.h"
#include "Version/MasterData.h"
#include "Version/Message_SyncLogFileFound.h"
#include "Version/PathParts.h"
#include "Version/SyncLog.h"

#include "Buffer/AutoPool.h"
#include "Buffer/File.h"
#include "Common/Assert.h"
#include "Os/AutoCriticalSection.h"

#include "Exception/Object.h"

_SYDNEY_USING
_SYDNEY_VERSION_USING
_SYDNEY_VERSION_SYNCLOG_USING

namespace
{

namespace _File
{
	// 格納する OS ファイルの絶対パス名を得る
	Os::Path				getPath(const Os::Path& parent);

	// 同期ログファイル記述子を保護するためのラッチ
	Os::CriticalSection		_latch;
}

//	FUNCTION
//	$$$::_File::getPath --
//		同期ログファイルを格納する OS ファイルの絶対パス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			parent
//			同期ログファイルを格納する OS ファイルの
//			親ディレクトリの絶対パス名
//
//	RETURN
//		得られた絶対パス名
//
//	EXCEPTIONS

Os::Path
_File::getPath(const Os::Path& parent)
{
	return Os::Path(parent).addPart(PathParts::SyncLog);
}

}

//	FUNCTION publuc
//	Version::SyncLog::File::File --
//		同期ログファイル記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Version::File::StorageStrategy&	storageStrategy
//			同期ログファイルを持つバージョンファイルのファイル格納戦略
//		Version::File::BufferingStrategy&	bufferingStrategy
//			同期ログファイルを持つバージョンファイルのバッファリング戦略
//
//	RETURN
//		なし
//
//	EXCEPTIONS

SyncLog::
File::File(const Version::File::StorageStrategy& storageStrategy,
		   const Version::File::BufferingStrategy& bufferingStrategy)
	: _bufFile(0),
	  _parent(storageStrategy._path._syncLog),
	  _sizeMax(Version::File::verifySizeMax(
				   storageStrategy._sizeMax._syncLog)),
	  _extensionSize(Version::File::verifyExtensionSize(
						(storageStrategy._extensionSize._syncLog) ?
						storageStrategy._extensionSize._syncLog :
						Configuration::SyncLogExtensionSize::get()))
{
	// 同期ログファイルの実体である
	// OS ファイルの絶対パス名は必ず指定されている必要がある

	; _SYDNEY_ASSERT(storageStrategy._path._syncLog.getLength());

	// 同期ログファイルをバッファリングするための
	// バッファプール記述子を得る
	//
	//【注意】	使用するバッファプールの種別は必ず通常になる

	Buffer::AutoPool pool(
		Buffer::Pool::attach(Buffer::Pool::Category::Normal));

	// ファイル格納戦略から同期ログファイルの絶対パス名を生成し、
	// 同期ログファイルの実体である
	// バッファファイルのバッファファイル記述子を得る
	//
	//【注意】	現状では、読み込み時に CRC による整合性の検証を行わない

	_bufFile = Buffer::File::attach(
		*pool, _File::getPath(_parent),	storageStrategy._pageSize,
		storageStrategy._mounted, storageStrategy._readOnly, true);
}

//	FUNCTION public
//	Version::SyncLog::File::create -- 生成する
//
//	NOTES
//		すでに生成されている
//		同期ログファイルを生成してもエラーにならない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SyncLog::
File::create(const Trans::Transaction& trans)
{
	if (!isMountedAndAccessible()) {

		// まず、同期ログファイルの実体である
		// OS ファイルを生成する

		; _SYDNEY_ASSERT(_bufFile);
		_bufFile->create(false);

		try {
			// ファイルヘッダを確保する

			(void) FileHeader::allocate(trans, *this);

			//【注意】	ここでフラッシュしなくても、
			//			recover 時にはファイルヘッダを読み出せなければ、
			//			同期処理中の同期ログファイルの
			//			フラッシュ前とみなすので、大丈夫

		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{

			// 生成したバッファファイルを破棄する

			_bufFile->destroy();

			_SYDNEY_RETHROW;
		}
	}
}

//	FUNCTION public
//	Version::SyncLog::File::move -- 移動する
//
//	NOTES
//		同期ログファイルが生成されていなくてもエラーにならない
//
//	ARGUMENTS
//		Os::Path&			path
//			移動先の絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SyncLog::
File::move(const Os::Path& path)
{
	// 同期ログファイルを保護するためにラッチする

	Os::AutoCriticalSection	latch(_File::_latch);

	if (path.compare(getParent()) != Os::Path::CompareResult::Identical) {

		// 指定された移動先の絶対パス名が
		// 同期ログファイルの実体である
		// バッファファイルを格納場所と同じでない

		// 指定された移動先の絶対パス名から、
		// 同期ログファイルの実体である
		// バッファファイルの新しい絶対パス名を生成し、
		// それに変更する
		//
		//【注意】	改名するバッファファイルの実体である
		//			OS ファイルが存在しなくても、例外を発生しない

		; _SYDNEY_ASSERT(_bufFile);
		_bufFile->rename(_File::getPath(path));

		// 新しい格納場所を記憶しておく

		_parent = path;
	}
}

#ifdef OBSOLETE
//	FUNCTION public
//	Version::SyncLog::File::truncate -- トランケートする
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::ID	id
//			指定されたとき
//				指定されたブロック識別子のブロックの
//				直前の位置でトランケートする
//			指定されないとき
//				0 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SyncLog::
File::truncate(const Trans::Transaction& trans, Block::ID id)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	if (id == 0)

		// 指定されたブロック識別子の表すブロック以降をトランケートすると、
		// ファイルヘッダがトランケートされてしまうとき、
		// トランケートされないようにする

		id = 1;

	// ファイルヘッダをフィックスする

	Block::Memory	headerMemory(
		FileHeader::fix(trans, *this, Buffer::Page::FixMode::Write));

	// 同期ログファイルの実体である
	// バッファファイルをトランケートする

	_bufFile->truncate(static_cast<Os::File::Offset>(getBlockSize()) * id);

	// ブロック数を修正する

	FileHeader::get(headerMemory)._blockCount = id;
	headerMemory.dirty();
}
#endif

//	FUNCTION private
//	Version::SyncLog::File::extend -- 拡張する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::ID	id
//			このブロック識別子のブロックの直前のものまでが
//			確保可能になるまで、拡張する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SyncLog::
File::extend(Block::ID id)
{
	// 指定されたブロックの直前のブロックまでが確保可能になるまで、
	// ファイル拡張サイズごとに実体であるバッファファイルを拡張していく

	const Os::File::Offset offset =
		static_cast<Os::File::Offset>(getBlockSize()) * id;

	while (getSize() < static_cast<Os::File::Size>(offset))
		_bufFile->extend(getSize() + getExtensionSize());
}

//	FUNCTION public
//	Version::SyncLog::File::startVerification -- 整合性検査の開始を指示する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Verification&	verification
//			あるトランザクションの整合性検査に関する情報を表すクラス
//		Admin::Verification::Progress&	result
//			整合性検査の経過を設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SyncLog::
File::startVerification(Verification& verification,
						Admin::Verification::Progress& result)
{
	; _SYDNEY_ASSERT(result.isGood());

	if (isAccessible(true)) {

		// 同期中でないのに、同期ログファイルが生成されている

		_SYDNEY_VERIFY_INCONSISTENT(
			result, getParent(), Message::SyncLogFileFound());
		return;
	}
}

//	FUNCTION public
//	Version::SyncLog::File::allocateLog --
//		あるバージョンページの最古の版の複写である同期ログを確保する
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			処理を行うトランザクションのトランザクション記述子
//		Version::Block::Memory&	headerMemory
//			最古の版の複写である同期ログを確保する
//			同期ログファイルのファイルヘッダのバッファリング内容
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

Block::Memory
SyncLog::
File::allocateLog(const Trans::Transaction& trans,
				  Block::Memory& headerMemory,
				  Page::ID pageID, MasterData::File& masterData,
				  Trans::TimeStamp::Value& allocation)
{
	// 新しい同期ログを記録するブロックを確保する

	Block::Memory dst(Log::allocate(trans, *this, headerMemory));
	; _SYDNEY_ASSERT(dst.isDirty());

	// マスタデータファイル中の複写する最古の版のブロックをフィックスする

	const Block::Memory& src =
		MasterData::Data::fix(
			trans, masterData, static_cast<Block::ID>(pageID),
			Buffer::Page::FixMode::ReadOnly, Buffer::ReplacementPriority::Low);

	// 先ほど確保したブロックへフィックスした最古の版を複写する

	(void) dst.copy(src);

	Log& dstLog = Log::get(dst);

	// 同期ログの元となったマスタデータファイルの
	// データブロックのブロック識別子を記録する
	dstLog._older = src.getID();
	// 必ず Block::IllegalID を格納しているはず
	; _SYDNEY_ASSERT(dstLog._physicalLog == Block::IllegalID);
	// 必ずデータブロックが確保されたときのタイムスタンプ値を格納しているはず
	allocation = dstLog._olderTimeStamp;
	; _SYDNEY_ASSERT(!Trans::TimeStamp::isIllegal(allocation));
	// 必ず Log::Category::Oldest を格納しているはず
	; _SYDNEY_ASSERT(dstLog._category == Log::Category::Oldest);

	return dst;
}

//	FUNCTION private
//	Version::SyncLog::File::allocate -- ブロックを 1 つ確保する
//
//	NOTES
//
//	ARGUMENTS
//		Version::Block::Memory&		headerMemory
//			ブロックを確保する同期ログファイルの
//			ファイルヘッダのバッファリング内容
//
//	RETURN
//		確保したブロックのブロック識別子
//
//	EXCEPTIONS

Block::ID
SyncLog::
File::allocate(Block::Memory& headerMemory)
{
	FileHeader& header = FileHeader::get(headerMemory);

	// 確保するブロックの後のブロック識別子を求める

	const Block::ID begin = header._blockCount;
	const Block::ID end = begin + 1;

	// ファイルの末尾に新しいブロックを
	// 確保できるようにファイルを拡張する

	extend(end);

	// ヘッダの総ブロック数を変更する

	header._blockCount = end;
	headerMemory.dirty();

	return begin;
}

//	FUNCTION public
//	Version::SyncLog::FileHeader::allocate --
//		同期ログファイルにファイルヘッダを確保する
//	
//	NOTES
//
//	ARGUMENTS
//		Version::SyncLog::File&	file
//			ファイルヘッダを確保する同期ログファイルの同期ログファイル記述子
//
//	RETURN
//		確保したファイルヘッダを記録するブロックのバッファリング内容
//
//	EXCEPTIONS

// static
Block::Memory
FileHeader::allocate(const Trans::Transaction& trans, SyncLog::File& file)
{
	// 同期ログファイルを保護するためにラッチする

	Os::AutoCriticalSection	latch(_File::_latch);

	; _SYDNEY_ASSERT(!file.getSize());

	// ファイルの先頭にファイルヘッダを
	// 確保できるようにファイルを拡張する

	file.extend(1);

	// ファイルヘッダをフィックスして、初期化する

	Block::Memory	headerMemory(
		FileHeader::fix(trans, file, Buffer::Page::FixMode::Allocate));
	FileHeader& header = FileHeader::get(headerMemory);

	header._versionNumber = VersionNumber::Current;
	header._blockCount = 1;
	headerMemory.dirty();

	return headerMemory;
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
//		Version::Block::Memory&	headerMemory
//			同期ログを確保する同期ログファイルの
//			ファイルヘッダのバッファリング内容
//
//	RETURN
//		確保した同期ログを記録するブロックのバッファリング内容
//
//	EXCEPTIONS

// static
Block::Memory
Log::allocate(const Trans::Transaction& trans,
			  SyncLog::File& file, Block::Memory& headerMemory)
{
	// 同期ログを記録するためのブロックを確保する

	Block::ID id = file.allocate(headerMemory);
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	// 同期ログをフィックスして、初期化する

	Block::Memory	logMemory(
		Log::fix(trans, file, id, Buffer::Page::FixMode::Allocate));
	Log& log = Log::get(logMemory);

	log._older = Block::IllegalID;
	log._physicalLog = Block::IllegalID;
	log._olderTimeStamp = Trans::IllegalTimeStamp;
	log._category = Category::Oldest;
	logMemory.dirty();

	return logMemory;
}

//	FUNCTION public
//	Version::SyncLog::Log::fix -- 同期ログをフィックスする
//
//	NOTES
//
//	ARGUMENTS
//		Version::SyncLog::File&	file
//			フィックスする同期ログが存在する
//			同期ログファイルの同期ログファイル記述子
//		Version::Block::ID	id
//			フィックスする同期ログが存在するブロックのブロック識別子
//		Buffer::Page::FixMode::Value	mode
//			Buffer::Page::FixMode::ReadOnly
//				フィックスする同期ログは参照しかされない
//			Buffer::Page::FixMode::Write
//				フィックスする同期ログは更新される
//			Buffer::Page::FixMode::Allocate
//				フィックスする同期ログは
//				その領域の初期化のために使用する
//
//	RETURN
//		フィックスした同期ログのバッファリング内容
//
//	EXCEPTIONS

// static
Block::Memory
Log::fix(const Trans::Transaction& trans,
		 SyncLog::File& file, Block::ID id, Buffer::Page::FixMode::Value mode)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	return Block::Memory(
		id, Buffer::Page::fix(
			*file._bufFile,
			static_cast<Os::File::Offset>(file.getBlockSize()) * id,
			mode, Buffer::ReplacementPriority::Low, &trans));
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
