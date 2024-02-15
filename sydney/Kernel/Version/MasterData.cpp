// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MasterData.cpp -- マスタデータファイル関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2006, 2013, 2023 Ricoh Company, Ltd.
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
#include "Version/Message_MasterDataFileNotFound.h"
#include "Version/PathParts.h"
#include "Version/SyncLog.h"
#include "Version/Verification.h"
#include "Version/VersionLog.h"

#include "Buffer/AutoPool.h"
#include "Buffer/File.h"
#include "Common/Assert.h"
#include "Common/Thread.h"
#include "Exception/BadDataPage.h"
#include "Os/AutoCriticalSection.h"
#include "Trans/Transaction.h"

#include "Exception/PreservedDifferentPage.h"

_SYDNEY_USING
_SYDNEY_VERSION_USING
_SYDNEY_VERSION_MASTERDATA_USING

namespace
{

namespace _File
{
	// 格納する OS ファイルの絶対パス名を得る
	Os::Path				getPath(const Os::Path& parent);
	// 同期ログファイルの内容でマスタデータファイルを上書きする
	void
	overwrite(const Trans::Transaction& trans,
			  MasterData::File& masterData, SyncLog::File& syncLog);

	// マスタデータファイル記述子を保護するためのラッチ
	Os::CriticalSection		_latch;
}

//	FUNCTION
//	$$$::_File::getPath --
//		マスタデータファイルを格納する OS ファイルの絶対パス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			parent
//			マスタデータファイルを格納する OS ファイルの
//			親ディレクトリの絶対パス名
//
//	RETURN
//		得られた絶対パス名
//
//	EXCEPTIONS

Os::Path
_File::getPath(const Os::Path& parent)
{
	return Os::Path(parent).addPart(PathParts::MasterData);
}

//	FUNCTION
//	$$$::_File::overwrite --
//		同期ログファイルの内容でマスタデータファイルを上書きする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			処理を行うトランザクションのトランザクション記述子
//		Version::MasterData::File&	masterData
//			同期ログファイルの内容で上書きする
//			マスタデータファイルのマスタデータファイル記述子
//		Version::SyncLog::File&	syncLog
//			マスタデータファイルを上書きする内容を得るための
//			同期ログファイルの同期ログファイル記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_File::overwrite(const Trans::Transaction& trans,
				 MasterData::File& masterData, SyncLog::File& syncLog)
{
	if (syncLog.isAccessible()) {

		// 同期ログファイルは存在する

		// 同期ログファイルが存在しても、
		// 同期ログファイルの生成または
		// 同期ログファイルへのバックアップ中に障害が発生したため、
		// 同期ログファイルが壊れている可能性がある
		//
		// そこで、同期ログファイルをすべて読み出してみる

		unsigned int n;

		try {
			const Block::Memory& headerMemory =
				SyncLog::FileHeader::fix(
					trans, syncLog, Buffer::Page::FixMode::ReadOnly);

			n = SyncLog::FileHeader::get(headerMemory).getBlockCount();
			; _SYDNEY_ASSERT(n > 0);

			for (Block::ID id = 1; id < n; ++id)
				(void) SyncLog::Log::fix(
					trans, syncLog, id, Buffer::Page::FixMode::ReadOnly);

		} catch (Exception::BadDataPage&) {

			// 同期ログファイルが壊れている場合、
			// 同期ログファイルのフラッシュ前なので、
			// マスタデータファイルは更新されていないはず

			Common::Thread::resetErrorCondition();
			n = 0;
		}

		if (n > 1) {

			// 先頭の同期ログから、ひとつひとつ処理していく

			Block::ID id = 1;
			do {
				// 同期ログをフィックスし、
				// マスタデータファイルのデータブロックへ複写する

				(void) masterData.syncData(
					trans, SyncLog::Log::fix(
						trans, syncLog, id, Buffer::Page::FixMode::ReadOnly));

			} while (++id < n) ;
		}
	}
}

}

//	FUNCTION publuc
//	Version::MasterData::File::File --
//		マスタデータファイル記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Version::File::StorageStrategy&	storageStrategy
//			マスタデータファイルを持つバージョンファイルのファイル格納戦略
//		Version::File::BufferingStrategy&	bufferingStrategy
//			マスタデータファイルを持つバージョンファイルのバッファリング戦略
//
//	RETURN
//		なし
//
//	EXCEPTIONS

MasterData::
File::File(
	const Version::File::StorageStrategy& storageStrategy,
	const Version::File::BufferingStrategy& bufferingStrategy)
	: _bufFile(0),
	  _parent(storageStrategy._path._masterData),
	  _sizeMax(Version::File::verifySizeMax(
				   storageStrategy._sizeMax._masterData)),
	  _extensionSize(Version::File::verifyExtensionSize(
						(storageStrategy._extensionSize._masterData) ?
						storageStrategy._extensionSize._masterData :
						Configuration::MasterDataExtensionSize::get()))
{
	// マスタデータファイルを格納する
	// OS ファイルの絶対パス名は必ず指定されている必要がある

	; _SYDNEY_ASSERT(storageStrategy._path._masterData.getLength());

	// マスタデータファイルをバッファリングするための
	// バッファプール記述子を得る

	Buffer::AutoPool pool(Buffer::Pool::attach(
							  bufferingStrategy._category));

	// ファイル格納戦略からマスタデータファイルの絶対パス名を生成し、
	// マスタデータファイルの実体である
	// バッファファイルのバッファファイル記述子を得る
	//
	//【注意】	現状では、読み込み時に CRC による整合性の検証を行わない

	_bufFile = Buffer::File::attach(
		*pool, _File::getPath(getParent()), storageStrategy._pageSize,
		storageStrategy._mounted, storageStrategy._readOnly, true);
}

//	FUNCTION public
//	Version::MasterData::File::move -- 移動する
//
//	NOTES
//		マスタデータファイルが生成されていなくてもエラーにならない
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
MasterData::
File::move(const Os::Path& path)
{
	// マスタデータファイルを保護するためにラッチする

	Os::AutoCriticalSection	latch(_File::_latch);

	if (path.compare(getParent()) != Os::Path::CompareResult::Identical) {

		// 指定された移動先の絶対パス名が
		// マスタデータファイルの実体である
		// バッファファイルを格納場所と同じでない

		// 指定された移動先の絶対パス名から、
		// マスタデータファイルの実体である
		// バッファファイルの新しい絶対パス名を生成し、
		// それに変更する

		; _SYDNEY_ASSERT(_bufFile);
		_bufFile->rename(_File::getPath(path));

		// 新しい格納場所を記憶しておく

		_parent = path;
	}
}

//	FUNCTION private
//	Version::MasterData::File::extend -- 拡張する
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
MasterData::
File::extend(Block::ID id)
{
	//【注意】	マスタデータファイルはラッチ済である必要がある

	// 指定されたブロックの直前のブロックまでが確保可能になるまで、
	// ファイル拡張サイズごとに実体であるバッファファイルを拡張していく

	const Os::File::Offset offset =
		static_cast<Os::File::Offset>(getBlockSize()) * id;

	while (getSize() < static_cast<Os::File::Size>(offset))
		_bufFile->extend(getSize() + getExtensionSize());
}

//	FUNCTION public
//	Version::MasterData::File::truncate -- トランケートする
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
MasterData::
File::truncate(Block::ID id)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	// マスタデータファイルの実体である
	// バッファファイルをトランケートする

	_bufFile->truncate(static_cast<Os::File::Offset>(getBlockSize()) * id);
}

//	FUNCTION public
//	Version::MasterData::File::restore --
//		あるタイムスタンプの表す時点以降に確保されたブロックをなくす
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			処理を行うトランザクションのトランザクション記述子
//		Trans::TimeStamp&	point
//			このタイムスタンプの表す時点以降に確保されたブロックをなくす
//
//	RETURN
//		true
//			条件を満たすブロックをなくすことができた
//		false
//			ファイル中のブロックは与えられた
//			タイムスタンプの表す時点以降に確保されたため、なにも処理しなかった
//
//	EXCEPTIONS

bool
MasterData::
File::restore(const Trans::Transaction& trans, const Trans::TimeStamp& point)
{
	; _SYDNEY_ASSERT(!point.isIllegal());

	// マスタデータファイルの総ブロック数を求める

	Block::ID id = getBlockCount(trans);
	if (id) {
		do {
			const Block::Memory& dataMemory =
				Data::fix(trans, *this, id - 1, Buffer::Page::FixMode::ReadOnly,
						  Buffer::ReplacementPriority::Low);

			if (Data::get(dataMemory)._olderTimeStamp < point)

				// 指定されたタイムスタンプより前に
				// 確保されたデータブロックが見つかったので、
				// 直後のデータブロックが探しているものである

				break;

		} while (--id) ;

		// マスタデータファイルの指定されたタイムスタンプより後に
		// 確保されたデータブロックの部分をトランケートする
		//
		//【注意】	バージョンログファイルは空になると、削除される
		//
		//			バージョンログファイルのファイルヘッダーに
		//			記録される総バージョンページ数は
		//			バージョンログファイルが生成されるときに
		//			MasterData::File::getBlockCount で求められる
		//
		//			ここで、与えられたタイムスタンプの表す時点以降に
		//			確保されたデータブロックをトランケートしておかないと、
		//			正しい値が求められなくなってしまう

		truncate(id);
	}

	return id;
}

//	FUNCTION public
//	Version::MasterData::File:recover -- ある時点に障害回復する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			処理を行うトランザクションのトランザクション記述子
//		Trans::TimeStamp&	point
//			このタイムスタンプの表す時点以前の
//			チェックポイント処理の終了時の状態にバージョンファイルを回復する
//		Version::SyncLog::File&	syncLog
//			同期を取る前のマスタデータを同期ログとして記憶する
//			同期ログファイルの同期ログファイル記述子
//
//	RETURN
//		true
//			障害回復できた
//		false
//			マスタデータファイルを生成したときより
//			前の状態には障害回復できなかった
//
//	EXCEPTIONS

bool
MasterData::
File::recover(const Trans::Transaction& trans,
			  const Trans::TimeStamp& point, SyncLog::File& syncLog)
{
	; _SYDNEY_ASSERT(!point.isIllegal());

	// 同期ログファイルが存在すれば、
	// 同期処理中に障害が発生したので、
	// その内容でマスタデータファイルを上書きする

	_File::overwrite(trans, *this, syncLog);

	// 与えられたタイムスタンプの表す時点以降に確保されたブロックをなくす

	return restore(trans, point);
}

//	FUNCTION public
//	Version::MasterData::File::recover --
//		ある数のデータブロックを持つように障害回復する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			処理を行うトランザクションのトランザクション記述子
//		unsigned int	pageCount
//			障害回復時点に存在したバージョンページの総数で、
//			マスタデータファイルはこの数のデータブロックを
//			持つようにトランケートされる
//		Version::SyncLog::File&	syncLog
//			同期を取る前のマスタデータを同期ログとして記憶する
//			同期ログファイルの同期ログファイル記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
MasterData::
File::recover(const Trans::Transaction& trans,
			  unsigned int pageCount, SyncLog::File& syncLog)
{
	// 同期ログファイルが存在すれば、
	// 同期処理中に障害が発生したので、
	// その内容でマスタデータファイルを上書きする

	_File::overwrite(trans, *this, syncLog);

	// マスタデータファイルが指定された数の
	// バージョンページのみを記録するように、トランケートする

	truncate(pageCount);
}

//	FUNCTION public
//	Version::MasterData::File::startVerification -- 整合性検査の開始を指示する
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
MasterData::
File::startVerification(Verification& verification,
						Admin::Verification::Progress& result)
{
	; _SYDNEY_ASSERT(result.isGood());

	if (!isAccessible(true)) {

		// バージョンファイルを構成する
		// マスタデータファイルが生成されていない

		_SYDNEY_VERIFY_INCONSISTENT(
			result, getParent(), Message::MasterDataFileNotFound());
		return;
	}
}

//	FUNCTION public
//	Version::MasterData::File::syncData --
//		あるバージョンページのバージョンログが
//		最古の版になるようにデータブロックに複写する
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			処理を行うトランザクションのトランザクション記述子
//		Version::Block::ID	id
//			バージョンログを複写するデータブロックのブロック識別子
//		Version::Block::Memory&	src
//			データブロックに複写する
//			あるバージョンページのバージョンログのバッファリング内容
//		Trans::TimeStamp::Value	allocation
//			バージョンログを複写するデータブロックが
//			確保されたときのタイムスタンプ値
//
//	RETURN
//		複写後のデータブロックのバッファリング内容
//
//	EXCEPTIONS

Block::Memory
MasterData::
File::syncData(const Trans::Transaction& trans,
			   Block::ID id, const Block::Memory& src,
			   Trans::TimeStamp::Value allocation)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);
	; _SYDNEY_ASSERT(!Trans::TimeStamp::isIllegal(allocation));

	// 指定されたブロック識別子のデータブロックに
	// 指定されたバージョンログの内容を複写し、更新されたことにしておく

	Block::Memory dst(
		Data::fix(trans, *this, id, Buffer::Page::FixMode::Allocate,
				  Buffer::ReplacementPriority::Low));
	dst.copy(src).dirty();

	// データブロックのヘッダー情報が上書きされたので、初期化しておく

	Data& dstData = Data::get(dst);

	dstData._older = Block::IllegalID;
	dstData._physicalLog = Block::IllegalID;
	dstData._olderTimeStamp = allocation;
	dstData._category = Data::Category::Oldest;

	return dst;
}

//	FUNCTION public
//	Version::MasterData::Data::allocate -- データブロックを確保する
//
//	NOTES
//
//	ARGUMENTS
//		Version::MasterData::File&	file
//			データブロックを確保する
//			マスタデータファイルのマスタデータファイル記述子
//		Version::Block::ID	id
//			確保するデータブロックのブロック識別子
//		Buffer::ReplacementPriority::Value	priority
//			Buffer::ReplacementPriority::Low
//				確保するデータブロックは、
//				バッファから破棄されやすい
//			Buffer::ReplacementPriority::Middle
//				確保するデータブロックは、
//				バッファにある程度残る
//			Buffer::ReplacementPriority::High
//				確保するデータブロックは、
//				バッファからかなりの間残る
//
//	RETURN
//		確保したデータブロックのバッファリング内容
//
//	EXCEPTIONS

// static
Block::Memory
Data::allocate(const Trans::Transaction& trans,
			   MasterData::File& file, Block::ID id,
			   Buffer::ReplacementPriority::Value priority)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	// マスタデータファイルを保護するためにラッチする

	Os::AutoCriticalSection	latch(_File::_latch);

	// 指定されたブロック識別子のブロックを確保できるように、
	// ファイルを拡張する

	file.extend(id + 1);

	// データブロックをフィックスして、初期化する

	Block::Memory dataMemory(
		id, Buffer::Page::fix(
			*file._bufFile,
			static_cast<Os::File::Offset>(file.getBlockSize()) * id,
			Buffer::Page::FixMode::Allocate, priority, &trans));
	Data& data = Data::get(dataMemory);
	
	// 必ず Block::IllegalID を格納する
	data._older = Block::IllegalID;
	// 必ず Block::IllegalID を格納する
	data._physicalLog = Block::IllegalID;
	// データブロックが確保されたときのタイムスタンプ値を格納する
	data._olderTimeStamp = Trans::TimeStamp::assign();
	// 必ず Data::Category::Oldest を格納する
	data._category = Category::Oldest;
	// 必ず ページID を格納する
	data._pageID = id;
	
	dataMemory.dirty();

	return dataMemory;
}

//	FUNCTION public
//	Version::MasterData::Data::fix -- データブロックをフィックスする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&		trans
//			データブロックをフィックスする
//			トランザクションのトランザクション記述
//		Version::MasterData::File&	file
//			フィックスするデータブロックが存在する
//			マスタデータファイルのマスタデータファイル記述子
//		Version::Block::ID	id
//			フィックスするデータブロックのブロック識別子
//		Buffer::Page::FixMode::Value	mode
//			Buffer::Page::FixMode::ReadOnly
//				フィックスするデータブロックは参照しかされない
//			Buffer::Page::FixMode::Write
//				フィックスするデータブロックは更新される
//			Buffer::Page::FixMode::Allocate
//				フィックスするデータブロックは
//				その領域の初期化のために使用する
//		Buffer::ReplacementPriority::Value	priority
//			Buffer::ReplacementPriority::Low
//				フィックスするデータブロックは、
//				バッファから破棄されやすい
//			Buffer::ReplacementPriority::Middle
//				フィックスするデータブロックは、
//				バッファにある程度残る
//			Buffer::ReplacementPriority::High
//				フィックスするデータブロックは、
//				バッファからかなりの間残る
//
//	RETURN
//		フィックスしたデータブロックのバッファリング内容
//
//	EXCEPTIONS

// static
Block::Memory
Data::fix(const Trans::Transaction& trans,
		  MasterData::File& file, Block::ID id,
		  Buffer::Page::FixMode::Value mode,
		  Buffer::ReplacementPriority::Value priority)
{
	; _SYDNEY_ASSERT(id != Block::IllegalID);

	if (!trans.isNoVersion() && mode & Buffer::Page::FixMode::ReadOnly)

		// 版管理するトランザクションが
		// 参照のみのためにフィックスしたデータブロックを
		// 更新するものはいないはずなので、
		// 読み取り書き込みロックする必要はない

		mode |= Buffer::Page::FixMode::NoLock;

	Block::Memory blockMemory(
		id, Buffer::Page::fix(
			*file._bufFile,
			static_cast<Os::File::Offset>(file.getBlockSize()) * id,
			mode, priority, &trans));

	const VersionLog::Log& log
		= VersionLog::Log::get(static_cast<const Block::Memory&>(blockMemory));
	if ((log._pageID != 0 || id == 0) && log._pageID != id)

		// 違うページIDのページが格納されている

		_SYDNEY_THROW3(Exception::PreservedDifferentPage,
					   file._bufFile->getPath(),
					   id, log._pageID);

	return blockMemory;
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2006, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
