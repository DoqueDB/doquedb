// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp -- バージョンファイル関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#include "Version/AutoPage.h"
#include "Version/AutoVerification.h"
#include "Version/Configuration.h"
#include "Version/DetachedPageCleaner.h"
#include "Version/File.h"
#include "Version/HashTable.h"
#include "Version/Manager.h"
#include "Version/MasterData.h"
#include "Version/VersionLog.h"
#include "Version/SyncLog.h"

#include "Checkpoint/Daemon.h"
#include "Checkpoint/Database.h"
#include "Checkpoint/TimeStamp.h"
#include "Common/Assert.h"
#include "Common/AutoCaller.h"
#include "Common/Thread.h"
#include "Common/Message.h"
#include "Exception/Cancel.h"
#include "Exception/Unexpected.h"
#include "Exception/ModLibraryError.h"
#include "Os/AutoCriticalSection.h"
#include "Os/AutoRWLock.h"
#include "Os/SysConf.h"
#include "Trans/AutoLatch.h"
#include "Trans/AutoTransaction.h"
#include "Trans/List.h"

#include "Exception/FakeError.h"

#include "ModUnicodeChar.h"

#include <exception>

#define _MOD_EXCEPTION(e) \
	Exception::ModLibraryError(moduleName, srcFile, __LINE__, e)
#define	_TRMEISTER_VERSION_FAKE_ERROR(name, e) \
		_TRMEISTER_FAKE_ERROR(name, e(moduleName, srcFile, __LINE__))

_SYDNEY_USING
_SYDNEY_VERSION_USING

namespace
{

namespace _File
{
	//	CLASS
	//	$$$::_File::_SyncPageInfo --
	//		同期候補のバージョンページに関する情報を表すクラス
	//
	//	NOTES

	struct _SyncPageInfo
	{
		// 同期候補のバージョンページのバージョンページ識別子
		Page::ID				_id;
		// バージョンログファイル中の最新版を記録する
		// バージョンログのブロック識別子
		Block::ID				_latestBlockID;
		// バージョンログファイル中の最新版を記録する
		// バージョンログのブロック識別子を記録する PBCT リーフのブロック識別子
		Block::ID				_leafBlockID;
		// バージョンログファイル中の最古の版を記録する
		// バージョンログの最終更新時タイムスタンプ値
		Trans::TimeStamp::Value	_oldestTimeStamp;
		// マスタデータファイル中の最古の版を
		// 記録するブロックが確保されたときのタイムスタンプ値
		Trans::TimeStamp::Value	_allocationTimeStamp;
	};

	// すべてのバージョンファイル記述子を管理する
	// ハッシュ表に登録するためのハッシュ値を計算する
	unsigned int			fileTableHash(const Os::Path& path);

	// 生成済のバージョンファイル記述子を探す
	File*
	find(HashTable<File>::Bucket& bucket, const Os::Path& path);

	// すべてのバージョンファイル記述子を管理するハッシュ表
	HashTable<File>*		_fileTable = 0;
}

namespace _Transaction
{
	// 現在実行中の版管理するトランザクションのうち、
	// 最も昔に開始されたものの開始時タイムスタンプを求める
	Trans::TimeStamp::Value
	getOldestBirthTimeStamp(const Lock::FileName& lockName);
}

//	FUNCTION
//	$$$::_File::fileTableHash --
//		すべてのバージョンファイル記述子を管理するハッシュ表に
//		登録するためのハッシュ値を計算する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			ハッシュ表に登録するバージョンファイル記述子の表す
//			バージョンファイルのマスタデータファイルの実体である
//			OS ファイルの親ディレクトリの絶対パス名
//
//	RETURN
//		得られたハッシュ値
//
//	EXCEPTIONS
//		なし

#ifdef OBSOLETE
unsigned int
_File::fileTableHash(const Os::Path& path)
{
	// 絶対パス名の末尾から最大 w 文字ぶんの UNICODE 値の和を求める

	const unsigned int w = 70;

	const ModUnicodeChar* p = path;
	const ModUnicodeChar* q = path.getTail();
	if (q > p + w)
		p = q - w;

	unsigned int i = 0;
	for (; q >= p; --q)
		i += static_cast<unsigned int>(*q);

	return i;
}
#else

//【注意】	以前は、末尾から一定文字数だけで計算することにより、
//			ハッシュ値の計算をできるかぎり早くしようとしていたが、
//			キャビネット数が多いと、
//			ファイル名の可変部分がキャビネット名の部分だけなので、
//			全文字を使って計算するようにした

inline
unsigned int
_File::fileTableHash(const Os::Path& path)
{
	return path.hashCode();
}
#endif

//	FUNCTION
//	$$$::_File::find -- 生成済のバージョンファイル記述子を探す
//
//	NOTES
//
//	ARGUMENTS
//		Version::HashTable<File>::Bucket&	bucket
//			バージョンファイル記述子が格納されるべきハッシュ表のバケット
//		Os::Path&			path
//			バッファリングする OS ファイルの絶対パス名
//
//	RETURN
//		0 以外の値
//			得られたバージョンファイル記述子を格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPTIONS

File*
_File::find(HashTable<File>::Bucket& bucket, const Os::Path& path)
{
	//【注意】	呼び出し側で bucket.getLatch() をラッチする必要がある

	switch (bucket.getSize()) {
	default:
	{
		// バケットに登録されているバージョンファイル記述子のうち、
		// 与えられた絶対パス名がマスタデータファイルの親ディレクトリである
		// バージョンファイルのものを探す

		HashTable<File>::Bucket::Iterator			begin(bucket.begin());
		HashTable<File>::Bucket::Iterator			ite(begin);
		const HashTable<File>::Bucket::Iterator&	end = bucket.end();

		do {
			File& file = *ite;

			if (path.compare(file.getParent()) ==
				Os::Path::CompareResult::Identical) {

				// 見つかったバージョンファイル記述子を
				// バケットの先頭に移動して、
				// 最近に参照されたものほど、見つかりやすくする

				bucket.splice(begin, bucket, ite);

				return &file;
			}
		} while (++ite != end) ;

		break;
	}
	case 1:
	{
		File& file = bucket.getFront();

		if (path.compare(file.getParent()) ==
			Os::Path::CompareResult::Identical)

			// 見つかった

			return &file;

		break;
	}
	case 0:
		break;
	}

	// 見つからなかった

	return 0;
}

//	FUNCTION
//	$$$::_Transaction::getOldestBirthTimeStamp --
//		現在実行中の版管理するトランザクションのうち、
//		最も昔に開始されたものの開始時タイムスタンプを求める
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Trans::IllegalTimeStamp 以外の値
//			求めたタイムスタンプ
//		Trans::IllegalTimeStamp
//			版管理するトランザクションは現在ひとつも実行されていない
//
//	EXCEPTIONS

Trans::TimeStamp::Value
_Transaction::getOldestBirthTimeStamp(const Lock::FileName& lockName)
{
	return Trans::Transaction::getBeginningID(lockName.getDatabasePart());
}

}

//	FUNCTION private
//	Manager::File::initialize --
//		マネージャーの初期化のうち、バージョンファイル記述子関連のものを行う
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

// static
void
Manager::File::initialize()
{
	// すべてのバージョンファイル記述子を管理する
	// ハッシュ表が確保されていないので、まず、確保する

	_File::_fileTable =
		new HashTable<Version::File>(Configuration::FileTableSize::get(),
									 &Version::File::_hashPrev,
									 &Version::File::_hashNext);
	; _SYDNEY_ASSERT(_File::_fileTable);
}

//	FUNCTION private
//	Manager::File::terminate --
//		マネージャーの後処理のうち、バージョンファイル関連のものを行う
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

// static
void
Manager::File::terminate()
{
	//【注意】	他のスレッドが同時に実行されることはないので、
	//			ラッチしない

	if (_File::_fileTable) {

		// いつか参照されるときのためにとってある
		// バージョンページ記述子をすべて破棄する

		HashTable<Version::Page>& table = Version::Page::getHashTable();
		{
		unsigned int i = 0;
		do {
			// 今調べているアドレスのバケットを得る

			HashTable<Version::Page>::Bucket& bucket = table.getBucket(i);

			while (bucket.getSize()) {

				// バケットの先頭に登録されている
				// バージョンページ記述子を得る

				Version::Page* page = &bucket.getFront();

				// このバージョンページ記述子をバケットから除き、破棄する

				Version::File* file = page->_file;
				bucket.erase(*page);
				delete page;

				// このバージョンページ記述子の表す
				// バージョンページが存在するバージョンファイルの
				// バージョンファイル記述子の参照数を 1 減らし、
				// どこからも参照されなくなれば、破棄する

				Version::File::detach(file, false);
			}
		} while (++i < table.getLength()) ;
		}
		// いつか参照されるときのためにとってある
		// バージョンファイル記述子のうち、
		// この関数の呼び出し時点でどこからも
		// 参照されていないものが残っているので、すべて破棄する
		{
		unsigned int i = 0;
		do {
			// 今調べているアドレスのバケットを得る

			HashTable<Version::File>::Bucket&
				bucket = _File::_fileTable->getBucket(i);

			while (bucket.getSize()) {

				// バケットの先頭に登録されている
				// バージョンファイル記述子を得る

				Version::File* file = &bucket.getFront();

				// この時点でバージョンファイル記述子は
				// どこからも参照されていないはず

				; _SYDNEY_ASSERT(!file->getRefCount());

				// このバージョンファイル記述子をバケットから除き、破棄する

				bucket.erase(*file);
				delete file;
			}
		} while (++i < _File::_fileTable->getLength()) ;
		}
		// すべてのバージョンファイル記述子を管理するハッシュ表を破棄する

		delete _File::_fileTable, _File::_fileTable = 0;
	}
}

//	FUNCTION private
//	Version::DetachedPageCleaner::repeatable --
//		参照済バージョンページ記述子の破棄を行う
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
DetachedPageCleaner::repeatable()
{
	// あるバージョンファイルの参照済のバージョンページ記述子のうち、
	// 最大、何パーセント破棄するかを求める

	const unsigned int coefficient =
		Configuration::CleanPageCoefficient::get();

	// すべてのバージョンページ記述子を管理する
	// ハッシュ表のバケットをひとつひとつ調べていく

	HashTable<Page>& table = Page::getHashTable();
	{
	unsigned int i = 0;
	do {
		// 今調べているアドレスのバケットを得る

		HashTable<Page>::Bucket& bucket = table.getBucket(i);

		// バケットを保護するためにラッチを試みる

		Os::AutoCriticalSection	latch(bucket.getLatch());

		// このバケットで最大いくつの
		// バージョンページ記述子を破棄するか求める

		unsigned int n = bucket.getSize();
		if (n && !(n = n * coefficient / 100))

			// バケットにひとつでもバージョンページ記述子が登録されていれば、
			// 最低でも 1 つは破棄を試みる

			n = 1;

		// このバケットに登録されている
		// バージョンページ記述子をひとつひとつ処理していく
		//
		//【注意】	バケットの先頭ほど、最近に参照されたものが
		//			登録されているので、バケットの末尾から処理していく

		const HashTable<Page>::Bucket::Iterator&	end = bucket.end();
		HashTable<Page>::Bucket::Iterator			ite(end);

		for (--ite; ite != end && n; ) {

			// 反復子のさす要素が削除されると
			// 前の要素を得られなくなるので、ここで反復子を前に進めておく

			Page* page = &*ite;
			--ite;

			// このバージョンページ記述子がどこからも参照されていないとき、
			// そのページ更新トランザクションリストをできれば、空にする

			if (!page->getRefCount() && page->clearModifierList()) {

				// 空にしたので、
				// このバージョンページ記述子をバケットから除き、破棄する

				File* file = page->_file;
				bucket.erase(*page);
				delete page;

				// このバージョンページ記述子の表す
				// バージョンページが存在するバージョンファイルの
				// バージョンファイル記述子の参照数を 1 減らす
				//
				//【注意】	後でバージョンファイル記述子を
				//			まとめて処理したいので、
				//			どこからも参照されなくなっても、ここでは破棄しない

				File::detach(file, true);

				// 最大破棄数を 1 減らす

				--n;
			}
		}
	} while (++i < table.getLength()) ;
	}
	; _SYDNEY_ASSERT(_File::_fileTable);
	{
	unsigned int i = 0;
	do {
		// 今調べているアドレスのバケットを得る

		HashTable<File>::Bucket& bucket = _File::_fileTable->getBucket(i);

		// バケットを保護するためにラッチをかける

		Os::AutoCriticalSection	latch(bucket.getLatch());

		// このバケットで最大いくつの
		// バージョンファイル記述子を破棄するか求める

		unsigned int n = bucket.getSize();
		if (n && !(n = n * coefficient / 100))

			// バケットにひとつでもバージョンファイル記述子が登録されていれば、
			// 最低でもひとつは破棄を試みる

			n = 1;

		// このバケットに登録されている
		// バッファファイル記述子をひとつひとつ処理していく
		//
		//【注意】	バケットの先頭ほど、最近に参照されたものが
		//			登録されているので、バケットの末尾から処理していく

		const HashTable<File>::Bucket::Iterator&	end = bucket.end();
		HashTable<File>::Bucket::Iterator			ite(end);

		for (--ite; ite != end && n; ) {

			// 反復子のさす要素が削除されると
			// 前の要素が得られなくなるので、ここで反復子を前に進めておく

			File* file = &*ite;
			--ite;

			if (!(file->getRefCount() ||
				  Trans::Transaction::isInProgress(
					  file->getLockName().getDatabasePart(),
					  file->_creatorTransaction,
					  Trans::Transaction::Category::ReadWrite))) {

				// 現在実行中の版管理するトランザクションを求める

				const Trans::List<Trans::Transaction>& list =
					Trans::Transaction::getInProgressList(
						file->getLockName().getDatabasePart(), false);

				Os::AutoCriticalSection latch(list.getLatch());

				if (!list.getSize()) {

					// このバッファファイル記述子がどこからも参照されておらず、
					// このバージョンファイルがマウントされているかを
					// 版管理するトランザクションが判断するために必要な
					// このバージョンファイルを生成したトランザクションの
					// 識別子をおぼえておく必要もないので、
					// バケットから除き、破棄する

					bucket.erase(*file);
					delete file;

					// 最大破棄数を 1 減らす

					--n;
				}
			}
		}
	} while (++i < _File::_fileTable->getLength()) ;
	}
}

//	FUNCTION private
//	Version::File::File --
//		バージョンファイル記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Version::File::StorageStrategy&	storageStrategy
//			バージョンファイルのファイル格納戦略
//		Version::File::BufferingStrategy& bufferingStrategy
//			バージョンファイルのバッファリング戦略
//		Lock::FileName&		lockName
//			バージョンファイルが存在する論理ファイルのロック名
//			(現状では無視される)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

File::File(const StorageStrategy& storageStrategy,
		   const BufferingStrategy& bufferingStrategy,
		   const Lock::FileName& lockName)
	: _lockName(lockName),
	  _refCount(0),
	  _hashAddr(0),
	  _hashPrev(0),
	  _hashNext(0),
	  _batch(false)
{
	_file._masterData = 0;
	_file._versionLog = 0;
	_file._syncLog = 0;

	try {
		// 指定された絶対パス名のファイルが実体である
		// マスタデータファイルのマスタデータファイル記述子を生成する

		_file._masterData =
			new MasterData::File(storageStrategy, bufferingStrategy);
		; _SYDNEY_ASSERT(_file._masterData);

		if (!Configuration::NoVersion::get() &&
			storageStrategy._path._versionLog.getLength()) {

			// 版を生成するバージョンファイルのとき

			// 指定された絶対パス名のファイルが実体である
			// バージョンログファイルの
			// バージョンログファイル記述子を生成する
			//
			//【注意】	版を生成しないバージョンファイルは
			//			バージョンログファイルを使用しない

			_file._versionLog =
				new VersionLog::File(
					*this, storageStrategy, bufferingStrategy);
			; _SYDNEY_ASSERT(_file._versionLog);

			if (!_file._masterData->isReadOnly() &&
				storageStrategy._path._syncLog.getLength()) {

				// 読み取り専用データを格納しないバージョンファイルのとき

				// 指定された絶対パス名のファイルが実体である
				// 同期ログファイルの同期ログファイル記述子を生成する
				//
				//【注意】	版を生成しないバージョンファイルは
				//			同期ログファイルは使用しない

				_file._syncLog =
					new SyncLog::File(storageStrategy, bufferingStrategy);
				; _SYDNEY_ASSERT(_file._syncLog);
			}
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		destruct();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Version::File::destruct --
//		バージョンファイル記述子を表すクラスのデストラクター下位関数
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
File::destruct()
{
	delete _file._masterData, _file._masterData = 0;
	delete _file._syncLog, _file._syncLog = 0;
	delete _file._versionLog, _file._versionLog = 0;
}

//	FUNCTION public
//	Version::File::attach -- バージョンファイル記述子を得る
//
//	NOTES
//
//	ARGUMENTS
//		Version::File::StorageStrategy&	storageStrategy
//			バージョンファイルのファイル格納戦略
//		Version::File::BufferingStrategy& bufferingStrategy
//			バージョンファイルのバッファリング戦略
//		Lock::FileName&		lockName
//			指定されたとき
//				バージョンファイルが存在する論理ファイルのロック名
//			指定されないとき
//				Lock::FileName(0, 0, 0) が指定されたものとみなす
//		bool				batch_
//			バッチインサートモードかどうか(default false)
//
//	RETURN
//		得られたバージョンファイル記述子を格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
File*
File::attach(const StorageStrategy& storageStrategy,
			 const BufferingStrategy& bufferingStrategy,
			 bool batch_)
{
	return attach(storageStrategy, bufferingStrategy,
				  Lock::FileName(0, 0, 0), batch_);
}

// static
File*
File::attach(const StorageStrategy& storageStrategy,
			 const BufferingStrategy& bufferingStrategy,
			 const Lock::FileName& lockName,
			 bool batch_)
{
	// マスタデータファイルの親ディレクトリが
	// ファイル格納戦略に指定された絶対パス名であるバージョンファイルの
	// バージョンファイル記述子を格納すべきハッシュ表のバケットを求める

	; _SYDNEY_ASSERT(_File::_fileTable);
	const unsigned int addr = _File::fileTableHash(
		storageStrategy._path._masterData) % _File::_fileTable->getLength();
	HashTable<File>::Bucket& bucket = _File::_fileTable->getBucket(addr);

	// バケットを保護するためにラッチをかける

	Os::AutoCriticalSection	latch(bucket.getLatch());

	// マスタデータファイルの親ディレクトリが
	// ファイル格納戦略に指定された絶対パス名であるバージョンファイルの
	// バージョンファイル記述子が求めたバケットに登録されていれば、それを得る

	File* file = _File::find(bucket, storageStrategy._path._masterData);
	if (file) {
		if (file->getLockName() != lockName)
		{
			// ロック名が違うということは、drop -> create されたが
			// 古いファイルの参照回数が残っており、古いファイルが
			// 削除されていないということ
			// このような状態は想定外なのでassertではなくエラーとする

			SydErrorMessage << "Old File Exist: "
							<< storageStrategy._path._masterData << ModEndl;
			_SYDNEY_THROW0(Exception::Unexpected);
		}

		// 見つかったので、参照回数を 1 増やす

		++file->_refCount;

		if ((batch_ || file->isBatchInsert()) && file->_refCount > 1)
		{

			// バッチインサートモードなのにどこからか参照されてる

			_SYDNEY_THROW0(Exception::Unexpected);
		}
		
	} else {

		// 見つからなかったので、生成する

		file = new File(storageStrategy, bufferingStrategy, lockName);
		; _SYDNEY_ASSERT(file);

		// 生成したバージョンファイル記述子を登録すべきバケットのアドレスは、
		// 計算コストが高いので、バージョンファイル記述子に覚えておく

		file->_hashAddr = addr;

		// 参照回数を 1 にする

		file->_refCount = 1;

		// ハッシュ表のバケットの先頭に挿入して、
		// 最近に参照されたものほど、見つかりやすくする

		bucket.pushFront(*file);
	}

	// バッチインサートモードかどうかを設定する
	
	file->setBatch(batch_);

	return file;
}

//	FUNCTION private
//	Version::File::attach -- バージョンファイル記述子の参照数を 1 増やす
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		参照数が 1 増えたバージョンファイル記述子を格納する領域の先頭アドレス
//
//	EXCEPTIONS

File*
File::attach()
{
	// 参照数を 1 増やすバージョンファイル記述子を
	// 格納するハッシュ表のバケットを求め、
	// バケットを保護するためにラッチする

	; _SYDNEY_ASSERT(_File::_fileTable);
	Os::AutoCriticalSection latch(
		_File::_fileTable->getBucket(_hashAddr).getLatch());

	// 参照数を 1 増やす

	++_refCount;

	return this;
}

//	FUNCTION public
//	Version::File::detach -- バージョンファイル記述子の参照をやめる
//
//	NOTES
//		バージョンファイル記述子の参照をやめても、
//		他のどこかで参照されていれば、バージョンファイル記述子は破棄されない
//		逆にどこからも参照されていなければ、
//		バージョンファイル記述子は直ちに破棄される
//
//	ARGUMENTS
//		Version::File*&		file
//			参照をやめるバージョンファイル記述子を格納する領域の先頭アドレス
//			呼び出しから返ると、0 になる
//		bool				reserve
//			true
//				どこからも参照されなくなったバージョンファイル記述子でも、
//				また参照されるときのために破棄せずにとっておく
//			false
//				どこからも参照されなくなったバージョンファイル記述子は破棄する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
File::detach(File*& file, bool reserve)
{
	if (file) {
		{
		// 与えられたバージョンファイル記述子を格納する
		// ハッシュ表のバケットを求める

		; _SYDNEY_ASSERT(_File::_fileTable);
		HashTable<File>::Bucket& bucket =
			_File::_fileTable->getBucket(file->_hashAddr);

		// バケットを保護するためにラッチする

		Os::AutoCriticalSection	latch(bucket.getLatch());

		// 参照数が 0 より大きければ 1 減らす

		Schema::ObjectID::Value dbid = file->getLockName().getDatabasePart();

		if (file->getRefCount() &&
			(--file->_refCount ||
			 (file->_file._masterData->isMounted() &&
			  (reserve ||
			   Trans::Transaction::isInProgress(
				   dbid,
				   file->_creatorTransaction,
				   Trans::Transaction::Category::ReadWrite) ||
			   Trans::Transaction::getInProgressList(dbid, false)
			   		.getSizeWithLatch()))))
				   
			// このバージョンファイルがマウントされているかを
			// 版管理するトランザクションが判断するために必要な
			// このバージョンファイルを生成したトランザクションの
			// 識別子をおぼえておく必要があるので、破棄できない

			file = 0;
		else
			// バケットから取り除く

			bucket.erase(*file);

		// ここで、バケットを保護するためのラッチがはずれる
		}
		if (file)

			// どこからも参照されていないバージョンファイル記述子を破棄し、
			// 与えられたポインタは 0 を指すようにする

			delete file, file = 0;
	}
}

//	FUNCTION private
//	Version::File::detach -- バージョンファイル記述子の参照数を 1 減らす
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
File::detach()
{
	// 参照数を 1 減らすバージョンファイル記述子を
	// 格納するハッシュ表のバケットを求める

	; _SYDNEY_ASSERT(_File::_fileTable);
	Os::AutoCriticalSection latch(
		_File::_fileTable->getBucket(_hashAddr).getLatch());
#ifdef DEBUG
	; _SYDNEY_ASSERT(getRefCount() > 1);
#else
	if (getRefCount())
#endif
		// 参照数が 0 より大きければ 1 減らす

		--_refCount;
}

//	FUNCTION public
//	Version::File::create -- 生成する
//
//	NOTES
//		すでに生成されているバージョンファイルを生成してもエラーにならない
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンファイルを生成するトランザクションの
//			トランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::create(const Trans::Transaction& trans)
{
	//【注意】	引数に渡されたトランザクション記述子は使用していない

	// 生成するバージョンファイルは、読取専用データを格納しない

	; _SYDNEY_ASSERT(!isReadOnly());

	//【注意】	バージョンファイルの生成時にマスタデータファイル、
	//			バージョンログファイル、同期ログファイルは生成しない
	//
	//			マスタデータファイル、バージョンログファイルは
	//			新しい版を記録するときに生成し、
	//			同期ログファイルは同期時に生成し、破棄する

	// バージョンファイルを生成したトランザクションの識別子をおぼえておく

	_creatorTransaction = trans.getID();
}

//	FUNCTION public
//	Version::File::destroy -- 破棄する
//
//	NOTES
//		バージョンファイルが生成されていなくてもエラーにならない
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンファイルを破棄するトランザクションの
//			トランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::destroy(const Trans::Transaction& trans)
{
	//【注意】	引数に渡されたトランザクション記述子は使用していない

	// 破棄するバージョンファイルは、読取専用データを格納しない

	; _SYDNEY_ASSERT(!isReadOnly());

	// 呼び出し側から破棄するバージョンファイルは
	// どこからも参照されていないことを保証されているので、
	// 更新したトランザクションを記憶するために、
	// このバージョンファイルのバージョンページ記述子を
	// 保持しておく必要はない
	//
	// そこで、管理するすべてのバージョンページ記述子を破棄する

	discardPage();

	// マスタデータファイル、バージョンログファイル、
	// 同期ログファイルの生成・破棄の排他制御用の
	// 読み取り書き込みロックを書き込みロックする

	Os::AutoRWLock rwlock(getRWLock(), Os::RWLock::Mode::Write);

	if (_file._versionLog) {

		// 版を生成するバージョンファイルである


		if (_file._syncLog)

			// 同期ログファイルは、同期時に生成し、
			// 破棄しているはずだが、念のために、ここでも破棄を試みる

			_file._syncLog->destroy();

		// バージョンログファイルが存在すれば、破棄する

		_file._versionLog->destroy();
	}

	// マスタデータファイルが存在すれば、破棄する

	; _SYDNEY_ASSERT(_file._masterData);
	_file._masterData->destroy();

	// バージョンファイルを生成したトランザクションの識別子を忘れる

	_creatorTransaction = Trans::IllegalID;
}

//	FUNCTION public
//	Version::File::mount -- マウントする
//
//	NOTES
//		すでにマウントされている
//		バージョンファイルをマウントしてもエラーにならない
//
//	ARGUMENTS
//		Trans::Transaction& trans
//			バージョンファイルをマウントするトランザクションの
//			トランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::mount(const Trans::Transaction& trans)
{
	//【注意】	引数に渡されたトランザクション記述子は使用していない

	// マスタデータファイル、バージョンログファイル、
	// 同期ログファイルの生成・破棄の排他制御用の
	// 読み取り書き込みロックを書き込みロックする

	Os::AutoRWLock rwlock(getRWLock(), Os::RWLock::Mode::Write);

	// マスタデータファイルが存在すれば、マウントする

	; _SYDNEY_ASSERT(_file._masterData);
	Common::AutoCaller0<MasterData::File>
		masterData(_file._masterData, &MasterData::File::unmount);
	masterData->mount();

	if (_file._versionLog) {

		// 版を生成するバージョンログファイルである

		// バージョンログファイルが存在すれば、マウントする

		Common::AutoCaller0<VersionLog::File>
			versionLog(_file._versionLog, &VersionLog::File::unmount);
		versionLog->mount();

		if (_file._syncLog)

			// 同期ログファイルは、同期時に生成、破棄されるので、
			// 存在しないはずだが、念のためにマウントする

			_file._syncLog->mount();

		// エラー処理のために
		// バージョンログファイルをアンマウントする必要はなくなった

		versionLog.release();
	}

	// エラー処理のために
	// マスタデータファイルをアンマウントする必要はなくなった

	masterData.release();

	// バージョンファイルをマウントしたトランザクションの識別子をおぼえておく

	_creatorTransaction = trans.getID();
}

//	FUNCTION public
//	Version::File::unmount -- アンマウントする
//
//	NOTES
//		マウントされていない
//		バージョンファイルをアンマウントしてもエラーにならない
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンファイルをアンマウントするトランザクションの
//			トランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::unmount(const Trans::Transaction& trans)
{
	//【注意】	引数に渡されたトランザクション記述子は使用していない

	// 呼び出し側からアンマウントされるバージョンファイルは
	// どこからも参照されていないことを保証されているので、
	// 更新したトランザクションを記憶するために、
	// このバージョンファイルのバージョンページ記述子を
	// 保持しておく必要はない
	//
	// そこで、管理するすべてのバージョンページ記述子を破棄する

	discardPage();

	// マスタデータファイル、バージョンログファイル、
	// 同期ログファイルの生成・破棄の排他制御用の
	// 読み取り書き込みロックを書き込みロックする

	Os::AutoRWLock rwlock(getRWLock(), Os::RWLock::Mode::Write);

	if (_file._versionLog) {

		// 版を生成するバージョンファイルである

		if (_file._syncLog)

			// 同期ログファイルは、同期時に生成、破棄されるので、
			// 存在しないはずだが、念のためにアンマウントする

			_file._syncLog->unmount();

		// バージョンログファイルが存在すれば、アンマウントする

		_file._versionLog->unmount();
	}

	// マスタデータファイルが存在すれば、アンマウントする

	; _SYDNEY_ASSERT(_file._masterData);
	_file._masterData->unmount();

	// バージョンファイルをマウントしたトランザクションの識別子を忘れる

	_creatorTransaction = Trans::IllegalID;
}

//	FUNCTION public
//	Version::File::truncate -- トランケートする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンファイルをトランケートするトランザクションの
//			トランザクション記述子
//		Version::Page::ID	id
//			このバージョンページ識別子の表す
//			バージョンページからトランケートする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::truncate(const Trans::Transaction& trans, Page::ID id)
{
	// 破棄するバージョンファイルは、読取専用データを格納しない

	; _SYDNEY_ASSERT(!isReadOnly());

	// マスタデータファイル、バージョンログファイル、
	// 同期ログファイルの生成・破棄の排他制御用の
	// 読み取り書き込みロックを読み取りロックする

	Os::AutoRWLock rwlock(getRWLock(), Os::RWLock::Mode::Write);

	; _SYDNEY_ASSERT(_file._masterData);

	// トランケート対象のページ識別子の更新トランザクションリストを空にする

	unsigned int pageCount = _file._masterData->getBlockCount(trans);
	for (unsigned int i = id; i < pageCount; ++i) {

		// ページ識別子が存在していれば、アタッチする
		
		AutoPage page(Page::attach(*this, i, false), false);
		
		if (page.get() != 0) {

			// ページ更新トランザクションリストを保護するためにラッチする
			
			Os::AutoCriticalSection latch(page->getLatch());
			
			// 更新トランザクションリストを空にする
			//
			//【注意】	更新したトランザクションがまだ生きていても、
			//			上位層から不要なページと言われているわけで、
			//			そのまま削除する

			page->_modifierList.clear();
		}
	}

	if (_file._versionLog && _file._versionLog->isMountedAndAccessible()) {

		// 版を生成するバージョンファイルで、
		// バージョンログファイルがマウントされている

		// 指定されたバージョンページ識別子の表すバージョンページ以降を
		// すべて使用済にし、可能な限りトランケートする

		if (!_file._versionLog->truncate(trans, id)) {

			// バージョンログファイルが空になった

			// 既存のバージョンページのみ格納可能なように
			// マスタデータファイルをトランケートする
			//
			//【注意】	バージョンログファイルを削除してから
			//			マスタデータファイルをトランケートすると、
			//			トランケートがエラーになったときに
			//			バージョンファイルとしての整合性が失われてしまう

			_file._masterData->truncate(id);

			// バージョンログファイルを削除する

			_file._versionLog->destroy();
		}
	} else

		// バージョンログファイルが存在しないので、
		// マスタデータファイルを指定されたバージョンページ識別子の
		// バージョンページの直前のページまで格納可能なようにトランケートする

		_file._masterData->truncate(id);
}

//	FUNCTION public
//	Version::File::move -- 移動する
//
//	NOTES
//		バージョンファイルが生成されていなくてもエラーにならない
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンファイルを移動するトランザクションの
//			トランザクション記述子
//		Version::File::StorageStrategy::Path& path
//			バージョンファイルを構成するそれぞれのファイルの移動先の絶対パス名
//
//	RETURN
//		なし
//
//	EXCETPIONS

void
File::move(const Trans::Transaction& trans, const StorageStrategy::Path& path)
{
	//【注意】	引数に渡されたトランザクション記述子は使用していない

	// 移動するバージョンファイルは、読取専用データを格納しない

	; _SYDNEY_ASSERT(!isReadOnly());

	// 移動前、移動後のバージョンファイルのバージョンファイル記述子を
	// 格納すべきハッシュ表のバケットを求める

	; _SYDNEY_ASSERT(_File::_fileTable);

	const unsigned int addr = _File::fileTableHash(
		path._masterData) % _File::_fileTable->getLength();

	HashTable<File>::Bucket& src = _File::_fileTable->getBucket(_hashAddr);
	HashTable<File>::Bucket& dst = _File::_fileTable->getBucket(addr);

	if (_hashAddr != addr) {

		// アドレスが移動前と異るので、
		// 移動前のバケットから自分自身を除く

		// すべてのバージョンファイル記述子を管理する
		// ハッシュ表の移動前のバケットを保護するためにラッチする

		Os::AutoCriticalSection	latch(src.getLatch());

		// すべてのバージョンファイル記述子を管理する
		// ハッシュ表の移動前のバケットから自分自身を除く

		src.erase(*this);
	}

	try {
		// すべてのバージョンファイル記述子を管理する
		// ハッシュ表の移動後のバケットを保護するためにラッチする

		Os::AutoCriticalSection	latch(dst.getLatch());

		// すべてのバージョンファイル記述子を管理する
		// ハッシュ表の移動後のバケットに、すでに登録されているか調べる

		File* file = _File::find(dst, path._masterData);
		if (file && file != this) {

			// 見つかったものは自分自身でない

			if (file->getRefCount()) {

				// 登録されているバージョンファイル記述子は
				// どこからか参照されている

				; _SYDNEY_ASSERT(false);
				_SYDNEY_THROW0(Exception::Unexpected);
			}

			// 登録されているバージョンファイル記述子を破棄する

			File::detach(file, false);
		}

		// マスタデータファイル、バージョンログファイル、
		// 同期ログファイルの生成・破棄の排他制御用の
		// 読み取り書き込みロックを読み取りロックする

		Os::AutoRWLock rwlock(getRWLock(), Os::RWLock::Mode::Read);

		// マスタデータファイルが存在すれば、移動する

		; _SYDNEY_ASSERT(_file._masterData);
		Common::AutoCaller1<MasterData::File, const Os::Path&>
		   masterData(_file._masterData, &MasterData::File::move, getParent());
		masterData->move(path._masterData);

		if (_file._versionLog) {

			// 版を生成するバージョンファイルである

			// バージョンログファイルが存在すれば、移動する

			Common::AutoCaller1<VersionLog::File, const Os::Path&>
				versionLog(_file._versionLog, &VersionLog::File::move,
						   _file._versionLog->getParent());
			versionLog->move(path._versionLog);

			if (_file._syncLog)

				// 同期ログファイルを移動する

				_file._syncLog->move(path._syncLog);

			// エラー処理のために
			// バージョンログファイルを移動する必要はなくなった

			versionLog.release();
		}

		// エラー処理のために
		// マスタデータファイルを移動する必要はなくなった

		masterData.release();

		if (_hashAddr != addr) {

			// 移動後のバケットに自分自身を登録する
			//
			//【注意】	本来であれば、移動後のバケットに対して splice で
			//			移動前のバケットから自分自身を移動したいところだが、
			//			そうすると、ハッシュ表全体でなくバケットごとに
			//			ラッチするため、デッドロックが起きる可能性がある

			dst.pushFront(*this);
			_hashAddr = addr;
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		if (_hashAddr != addr) {

			// 移動前のバケットへ自分自身を登録しなおす

			Os::AutoCriticalSection	latch(src.getLatch());
			src.pushFront(*this);
		}
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Version::File::flush -- フラッシュする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンファイルをフラッシュするトランザクションの
//			トランザクション記述子
//
//	RETURN
//		なし
//
//	EXCETPIONS

void
File::flush(const Trans::Transaction& trans)
{
	//【注意】	引数に渡されたトランザクション記述子は使用していない

	// フラッシュするバージョンファイルは、読取専用データを格納しない

	; _SYDNEY_ASSERT(!isReadOnly());

	// マスタデータファイル、バージョンログファイル、
	// 同期ログファイルの生成・破棄の排他制御用の
	// 読み取り書き込みロックを読み取りロックする

	Os::AutoRWLock rwlock(getRWLock(), Os::RWLock::Mode::Read);

	; _SYDNEY_ASSERT(_file._masterData);
	if (_file._masterData->isMountedAndAccessible())

		// マスタデータファイルがマウントされているので、フラッシュする

		_file._masterData->flush();

	if (_file._versionLog) {

		// 版を生成するバージョンファイルである

		if (_file._versionLog->isMountedAndAccessible())

			// バージョンログファイルがマウントされているので、フラッシュする

			_file._versionLog->flush();

		if (_file._syncLog && _file._syncLog->isMountedAndAccessible())

			// 同期ログファイルは、同期時に生成、破棄されるので、
			// 存在しないはずだが、なぜかマウントされていれば、
			// 念のためにフラッシュする

			_file._syncLog->flush();
	}
}

//	FUNCTION public
//	Version::File::startBackup -- バックアップを開始を指示する
//
//	NOTES
//		全ファイル方式でのバックアップを開始するために必要な処理を行う
//
//		版を生成しないバージョンファイルは、
//		全ファイル方式でのバックアップは行えない
//
//		この関数を呼び出し中、
//		または呼び出してから Version::File::endBackup を呼び出すまでに、
//		バージョンファイルに対して Version::File::sync を呼び出してはならない
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンファイルのバックアップの開始を指示する
//			トランザクションのトランザクション記述子
//		bool				restorable
//			true または指定されないとき
//				バックアップされた内容をリストアしたとき、
//				あるタイムスタンプの表す時点に開始された
//				版管理するトランザクションの参照する版が
//				最新版になるように変更可能にする
//			false
//				バックアップされた内容をリストアしたとき、
//				バックアップ開始時点に障害回復可能にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::startBackup(const Trans::Transaction& trans, bool restorable)
{
	if (isReadOnly())

		// 読み取り専用データを格納するバージョンログファイルは
		// バックアップの開始を指示する必要はない

		return;

	if (!_file._versionLog) {

		// 版を生成しないバージョンファイルは、
		// 全ファイル方式でのバックアップを行うことはできない

		; _SYDNEY_ASSERT(false);
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	// マスタデータファイル、バージョンログファイル、
	// 同期ログファイルの生成・破棄の排他制御用の
	// 読み取り書き込みロックを読み取りロックする

	Os::AutoRWLock rwlock(getRWLock(), Os::RWLock::Mode::Read);

	if (restorable && !trans.isNoVersion() &&
		trans.getIsolationLevel() ==
		Trans::Transaction::IsolationLevel::Serializable) {

		// アイソレーションレベルが SERIALIZABLE の
		// 版管理するトランザクションがバックアップを開始しようとしている

		if (_file._versionLog->isMountedAndAccessible()) {

			// バージョンログファイルはマウントされている

			if (getRefCount() > 1) {

				// 自分以外の誰かが参照している

				// バージョンページごとのページ更新トランザクションリストは
				// バックアップされないので、リストアしたときに、
				// バックアップを開始した版管理するトランザクションが
				// 参照していた版を記録しているバージョンログが
				// どれであるか選択できない可能性がある
				//
				// そこで、ページ更新トランザクションリストが存在する
				// バージョンページごとにバックアップの準備を行う

				// まず、バージョンログファイルのファイルヘッダをフィックスする

				VersionLog::MultiplexBlock headerMulti;
				VersionLog::FileHeader::fix(
					trans, 0, *_file._versionLog,
					Buffer::Page::FixMode::Write, headerMulti);

				const Block::Memory& headerMemory =
					headerMulti._memories[headerMulti._master];

				HashTable<Page>& table = Page::getHashTable();

				unsigned int i = 0;
				do {
					// 今調べているアドレスのバケットを得る

					HashTable<Page>::Bucket& bucket = table.getBucket(i);

					// バケットを保護するためにラッチする

					Os::AutoCriticalSection	latch(bucket.getLatch());

					if (bucket.getSize()) {

						// このバケットに登録されている
						// バージョンページ記述子をひとつひとつ処理していく

						HashTable<Page>::Bucket::Iterator
							ite(bucket.begin());
						const HashTable<Page>::Bucket::Iterator&
							end = bucket.end();

						do {
							Page& page = *ite;
							if (&page.getFile() != this)

								// 自分以外のバージョンファイルの
								// バージョンページを表すので、処理しない

								continue;

							// ページ更新トランザクションリストを
							// 保護するためにラッチする

							Os::AutoCriticalSection	latch(page.getLatch());

							if (page.getModifierList().isEmpty())

								// このバージョンページの最新版を更新した
								// トランザクションは存在しないので、
								// そのままバックアップできる

								continue;

							// このバージョンページの最新版を記録する
							// バージョンログのブロック識別子を記録する
							// PBCT リーフを得る

							Block::Memory leafMemory(
								_file._versionLog->traversePBCT(
									trans, 0, headerMemory, page.getID(),
									Buffer::Page::FixMode::Write));
							if (leafMemory.getID() == Block::IllegalID)

								// このバージョンページの最新版は
								// マスタデータファイルに存在するので、
								// そのままバックアップできる

								continue;

							VersionLog::PBCTLeaf& leaf =
								VersionLog::PBCTLeaf::get(leafMemory);

							// 得られた PBCT リーフに記録されている
							// このバージョンページの最新版を記録する
							// バージョンログのブロック識別子を求める

							const Block::ID blockID =
								leaf.getLatestID(page.getID(), getPageSize());

							if (blockID == Block::IllegalID)

								// このバージョンページの最新版は
								// マスタデータファイルに存在する

								continue;

							// 最新版を記録するバージョンログをフィックスする

							const Block::Memory& src =
								VersionLog::Log::fix(
									trans, 0, *_file._versionLog, blockID,
									Buffer::Page::FixMode::Write,
									Buffer::ReplacementPriority::Low);

							// 必要があれば、新しい最新版のバージョンログを
							// 確保することにより、バックアップ可能にする

							const Block::Memory& dst =
								_file._versionLog->allocateLogForBackup(
									trans, headerMulti, page, src);

							if (src != dst) {

								// 新しい最新版を記録する
								// バージョンログが確保されたので、
								// そのブロック識別子を PBCT リーフに記録する

								leaf.setLatestID(
									page.getID(), dst.getID(), getPageSize());
								leafMemory.dirty();
							}

							//【注意】	最新版しかないときに
							//			その最終更新時タイムスタンプが
							//			変更されたとしても、
							//			必ず、大きな値に更新されるので
							//			PBCT リーフに設定しなくても問題ない

						} while (++ite != end) ;
					}
				} while (++i < table.getLength()) ;
			}
		}

		// バージョンファイルをフラッシュする
		//
		// バックアップを開始するトランザクションの
		// 参照する版にリストアできるようにするとき、
		// その版を記録しているバージョンログは
		// フラッシュされていない可能性がある
		// マスターデータファイルもフラッシュされていない
		// 可能性がある

		flush(trans);
	}
	
	if (_file._versionLog->isMountedAndAccessible())

		// バージョンログファイルはマウントされている
		// バージョンログファイルにバックアップの開始を指示する

		_file._versionLog->startBackup();
}

//	FUNCTION public
//	Version::File::endBackup -- バックアップを終了を指示する
//
//	NOTES
//		全ファイル方式でのバックアップを終了するために必要な処理を行う
//
//		Version::File::startBackup を呼び出し中、
//		または呼び出してからこの関数を呼び出すまでに、
//		バージョンファイルに対して Version::File::sync を呼び出してはならない
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンファイルのバックアップの終了を指示する
//			トランザクションのトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::endBackup(const Trans::Transaction& trans)
{
	//【注意】	引数に渡されたトランザクション記述子は使用していない

	if (!isReadOnly()) {

		// 読取専用データを格納しないバージョンファイルである

		if (!_file._versionLog) {

			// 版を生成しないバージョンファイルは、
			// 全ファイル方式でのバックアップを行うことはできない

			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::Unexpected);
		}

		// マスタデータファイル、バージョンログファイル、
		// 同期ログファイルの生成・破棄の排他制御用の
		// 読み取り書き込みロックを読み取りロックする

		Os::AutoRWLock rwlock(getRWLock(), Os::RWLock::Mode::Read);

		if (_file._versionLog->isMountedAndAccessible())

			// バージョンログファイルがマウントされていれば、
			// バージョンログファイルにバックアップの終了を指示する

			_file._versionLog->endBackup();
	}
}

//	FUNCTION public
//	Version::File::restore --
//		あるタイムスタンプの表す時点に開始された版管理するトランザクションの
//		参照する版が最新版になるようにバージョンファイルを変更する
//
//	NOTES
//		版を生成しないバージョンファイルは、
//		ある版が最新版になるように変更できない
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンファイルを戻すトランザクションのトランザクション記述子
//		Trans::TimeStamp&	point
//			このタイムスタンプの表す時点に開始された
//			版管理するトランザクションの参照する版が
//			最新版になるようにバージョンファイルを変更する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::restore(const Trans::Transaction& trans, const Trans::TimeStamp& point)
{
	; _SYDNEY_ASSERT(!point.isIllegal());

	if (!isReadOnly()) {

		// 読取専用データを格納しないバージョンファイルである

		if (!_file._versionLog) {

			// 版を生成しないバージョンファイルは、
			// ある版が最新版になるように変更できない

			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::Unexpected);
		}

		// マスタデータファイル、バージョンログファイル、
		// 同期ログファイルの生成・破棄の排他制御用の
		// 読み取り書き込みロックを書き込みロックする

		Os::AutoRWLock rwlock(getRWLock(), Os::RWLock::Mode::Write);

		// バージョンログファイルがマウントされているか調べる

		if (_file._versionLog->isMountedAndAccessible() &&
			!_file._versionLog->restore(trans, point))

			// バージョンログファイルを生成したときより
			// 前に開始されたトランザクションが参照する版を
			// 最新版にしようとしているので、
			// バージョンログファイルを破棄する

			_file._versionLog->destroy();

		// マスタデータファイルがマウントされているか調べる

		; _SYDNEY_ASSERT(_file._masterData);

		if (_file._masterData->isMountedAndAccessible() &&
			!_file._masterData->restore(trans, point))

			// マスタデータファイルを生成したときより
			// 前の状態にリストアしようとしているので、
			// マスタデータファイルを破棄する

			_file._masterData->destroy();
	}
}

//	FUNCTION public
//	Version::File::recover --
//		あるタイムスタンプの表す時点の状態にバージョンファイルを障害回復する
//
//	NOTES
//		版を生成しないバージョンファイルは、ある時点の状態に障害回復できない
//
//		障害回復した結果、バージョンファイルが破棄されることがある
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンファイルを戻すトランザクションのトランザクション記述子
//		Trans::TimeStamp&	point
//			このタイムスタンプの表す時点以前の
//			チェックポイント処理の終了時の状態にバージョンファイルを回復する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::recover(const Trans::Transaction& trans, const Trans::TimeStamp& point)
{
	//【注意】	引数に渡されたトランザクション記述子は使用していない

	; _SYDNEY_ASSERT(!point.isIllegal());

	if (!isReadOnly()) {

		// 読取専用データを格納しないバージョンファイルである

		if (!_file._versionLog || !_file._syncLog) {

			// 版を生成しなかったり、同期ログファイルを使用しない
			// バージョンファイルはある時点に障害回復できない

			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::Unexpected);
		}

		// マスタデータファイル、バージョンログファイル、
		// 同期ログファイルの生成・破棄の排他制御用の
		// 読み取り書き込みロックを書き込みロックする

		Os::AutoRWLock rwlock(getRWLock(), Os::RWLock::Mode::Write);

		// バージョンログファイルがマウントされているか調べる

		; _SYDNEY_ASSERT(_file._masterData);

		if (_file._versionLog->isMountedAndAccessible()) {

			// バージョンログファイルがマウントされていれば、
			// マスタデータファイルもマウントされているはず

			; _SYDNEY_ASSERT(_file._masterData->isMountedAndAccessible());

			// バージョンログファイルを障害回復する

			unsigned int pageCount;
			if (!_file._versionLog->recover(trans, point, pageCount)) {

				// マスタデータファイルを
				// 必要があれば、同期を取る前の状態に障害回復する

				if (!_file._masterData->recover(trans, point, *_file._syncLog))

					// マスタデータファイルを生成したときより
					// 前の状態に障害回復しようとしているので、
					// マスタデータファイルを破棄する

					_file._masterData->destroy();
				else
					// マスタデータファイルの組となる
					// バージョンログファイルまたは同期ログファイルを
					// 破棄すると、バージョンログファイルまたは
					// 同期ログファイルが生成されるまでは
					// 障害回復時にマスタデータファイルは処理されなくなる
					//
					// そこで、マスタデータファイルをフラッシュし、
					// 障害回復中に障害が発生した結果、
					// 再度、障害回復を行う必要が起きても大丈夫にする

					_file._masterData->flush();

				// バージョンログファイルを生成したときより
				// 前の状態に障害回復しようとしているので、
				// バージョンログファイルを破棄する

				_file._versionLog->destroy();

			} else if (pageCount) {

				// マスタデータファイルを
				// 必要があれば、同期をとる前の状態に障害回復し、フラッシュする

				_file._masterData->recover(trans, pageCount, *_file._syncLog);
				_file._masterData->flush();
			} else {

				// マスタデータファイルは空なので、
				// バージョンログファイル、マスタデータファイルを破棄する

				_file._masterData->destroy();
				_file._versionLog->destroy();
			}

			// 同期ログファイルが存在すれば、削除する

			_file._syncLog->destroy();

		} else if (_file._masterData->isMountedAndAccessible())

			// マスタデータファイルはマウントされているが、
			// バージョンログファイルはマウントされていないので、
			// マスタデータファイルのみ障害回復する
			//
			//【注意】	前々回のチェックポイント処理以降に
			//			バージョンファイルに新たなバージョンページが確保され、
			//			同期処理によりバージョンログファイルが
			//			削除されることもあるので、障害回復は必要である

			if (!_file._masterData->recover(trans, point, *_file._syncLog))
				_file._masterData->destroy();
	}
}

//	FUNCTION public
//	Version::File::startVerification -- 整合性検査の開始を指示する
//
//	NOTES
//		この関数を呼び出し中、
//		または呼び出したから Version::File::endVerification を呼び出すまでに、
//		バージョンファイルに対して Version::File::sync を呼び出してはならない
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			整合性検査を開始するトランザクションのトランザクション記述子
//		Admin::Verification::Treatment::Value treatment
//			開始される整合性検査で矛盾が見つかったときの処置で、
//			Admin::Verification::Treatment::Value の論理和を指定する
//		Admin::Verification::Progress&	result
//			整合性検査の経過を設定する
//		bool				overall
//			true
//				バージョンファイル全体の整合性検査を行う
//			false または指定されないとき
//				バージョンファイルの一部の整合性検査を行う
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::startVerification(const Trans::Transaction& trans,
						Admin::Verification::Treatment::Value treatment,
						Admin::Verification::Progress& result,
						bool overall)
{
	; _SYDNEY_ASSERT(result.isGood());

	// 指定されたトランザクション記述子の表すトランザクションの
	// 整合性検査に関する情報を表すクラスを生成する

	AutoVerification verification(Verification::attach(trans, *this), true);

	// 指定された引数をおぼえておく

	verification->_treatment = treatment;
	verification->_overall = overall;

	// マスタデータファイル、バージョンログファイル、
	// 同期ログファイルの生成・破棄の排他制御用の
	// 読み取り書き込みロックを読み取りロックする

	Os::AutoRWLock rwlock(getRWLock(), Os::RWLock::Mode::Read);

	try {

		; _SYDNEY_ASSERT(_file._masterData);
		if (_file._masterData->isMountedAndAccessible()) {

			// マスタデータファイルがマウントされているので
			// マスタデータファイルに対して、整合性検査の開始を指示する

			Admin::Verification::Progress progress(result.getConnection());
			_file._masterData->startVerification(*verification, progress);
			result += progress;

			if (!progress.isGood())
				return;
		}

		if (_file._versionLog) {

			// 版を生成するバージョンファイルである

			if (_file._versionLog->isMountedAndAccessible()) {

				// バージョンログファイルがマウントされているので
				// バージョンログファイルに対して、整合性検査の開始を指示する

				Admin::Verification::Progress progress(result.getConnection());
				_file._versionLog->startVerification(
					trans, *verification, *_file._masterData, progress);
				result += progress;

				if (!progress.isGood())
					return;
			}

			if (_file._syncLog) {

				// 同期ログファイルに対して、整合性検査の開始を指示する

				Admin::Verification::Progress progress(result.getConnection());
				_file._syncLog->startVerification(*verification, progress);
				result += progress;

				if (!progress.isGood())
					return;
			}
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// 整合性検査に関する情報を保持するクラスをクリアする
		verification.free(false);
		
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Version::File::endVerification -- 整合性検査の終了を指示する
//
//	NOTES
//		Version::File::startVerification を呼び出し中、
//		または呼び出してからこの関数を呼び出すまでに、
//		バージョンファイルに対して Version::File::sync を呼び出してはならない
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			整合性検査を終了するトランザクションのトランザクション記述子
//		Admin::Verification::Progress&	result
//			整合性検査の経過を設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::endVerification(const Trans::Transaction& trans,
					  Admin::Verification::Progress& result)
{
	// 指定されたトランザクション記述子の表すトランザクションの
	// 整合性検査に関する情報を表すクラスを得る
	//
	//【注意】	整合性検査に関する情報を表すクラスは
	//			この関数の実行終了時に破棄される

	AutoVerification verification(Verification::attach(trans, *this), false);

	if (_file._versionLog) {

		// 版を生成するバージョンファイルである

		// マスタデータファイル、バージョンログファイル、
		// 同期ログファイルの生成・破棄の排他制御用の
		// 読み取り書き込みロックを読み取りロックする

		Os::AutoRWLock rwlock(getRWLock(), Os::RWLock::Mode::Read);

		if (_file._versionLog->isMountedAndAccessible())

			// バージョンログファイルがマウントされているので
			// バージョンログファイルに対して、整合性検査の終了を指示する

			_file._versionLog->endVerification(trans, *verification, result);
	}
}

//	FUNCTION public
//	Version::File::sync -- バージョンファイルの同期を取る
//
//	NOTES
//		バージョンログファイル中の版をできる限り、
//		マスタデータファイルに反映し、
//		バージョンログファイル中の使用されなくなった版の
//		バージョンログを使用済にし、できる限りトランケートする
//
//		Version::File::startBackup が呼び出し中、
//		または呼び出してから Version::File::endBackup を呼び出すまでと、
//		Version::File::startVerification が呼び出し中、
//		または呼び出してから Version::File::endVerification を呼び出すまでに、
//		バージョンファイルに対してこの関数を呼び出してはならない
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンファイルの同期を取る
//			トランザクションのトランザクション記述子
//		bool&				incomplete
//			true
//				今回の同期処理でバージョンファイルを持つ
//				オブジェクトの一部に処理し残しがある
//			false
//				今回の同期処理でバージョンファイルを持つ
//				オブジェクトを完全に処理してきている
//
//				同期処理の結果、バージョンファイルを処理し残したかを設定する
//		bool&				modified
//			true
//				今回の同期処理でバージョンファイルを持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の同期処理でバージョンファイルを持つ
//				オブジェクトはまだ更新されていない
//
//				同期処理の結果、バージョンファイルが更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::Cancel
//			同期処理が途中で中断された

void
File::sync(const Trans::Transaction& trans, bool& incomplete, bool& modified)
{
	if (isReadOnly() || !trans.isNoVersion() ||
		!_file._versionLog || !_file._syncLog)

		// 読み取り専用データを格納していたり、
		// 版を使用するトランザクションだったり、
		// 版を生成しなかったり、同期ログファイルを使用しない
		// バージョンファイルの同期は取る必要がない

		return;

	{
		// マスタデータファイル、バージョンログファイル、
		// 同期ログファイルの生成・破棄の排他制御用の
		// 読み取り書き込みロックを書き込みロックする

		Os::AutoRWLock rwlock(getRWLock(), Os::RWLock::Mode::Write);

		if (_file._versionLog->isMountedAndAccessible()) {

			// バージョンログファイルのバージョンが２以上の場合、
			// 使用済みブロックを反映させる

			_file._versionLog->applyFree(trans);
		}
	}

	// 一度に処理するバージョンページ数のソフトリミットを求める

	const unsigned int soft = Configuration::SyncPageCountMax::get();

	Page::ID id = 0;
	bool remain;

	do {
		{
		// 論理ファイル以下の層で論理ファイルへの
		// ラッチを操作するのは変則的だが、同期処理の間、
		// まったく論理ファイルへの更新操作ができないのは問題なので、
		// 一瞬、論理ファイルへのラッチをはずし、更新操作を解禁する

		Trans::AutoUnlatch unlatch(
			const_cast<Trans::Transaction&>(trans), getLockName());

		if (trans.isCanceledStatement()) {

			// 処理の中断が指示され、途中で処理をやめるので、
			// まだ、同期処理する必要がある

			incomplete = true;

			_SYDNEY_THROW0(Exception::Cancel);
		}
		}
		// マスタデータファイル、バージョンログファイル、
		// 同期ログファイルの生成・破棄の排他制御用の
		// 読み取り書き込みロックを書き込みロックする

		Os::AutoRWLock rwlock(getRWLock(), Os::RWLock::Mode::Write);

		// いくつかのバージョンページについて、必要があれば、同期する

		bool isContinue = true;
		remain = syncPage(trans, id, soft, modified, isContinue);

		if (isContinue == false)
		{
			// このファイルに関して、これ以上続けられないエラーが発生したので、
			// これ以上同期処理は実施しない

			// でも、まだ、同期処理する必要がある

			incomplete = true;

			return;
		}

	} while (remain) ;

	// マスタデータファイル、バージョンログファイル、
	// 同期ログファイルの生成・破棄の排他制御用の
	// 読み取り書き込みロックを書き込みロックする

	Os::AutoRWLock rwlock(getRWLock(), Os::RWLock::Mode::Write);

	if (_file._versionLog->isMountedAndAccessible()) {

		// バージョンログファイルがマウントされていれば、
		// マスタデータファイルもマウントされているはず

		; _SYDNEY_ASSERT(_file._masterData &&
						 _file._masterData->isMountedAndAccessible());

		try {
			// 以下の処理でバージョンファイルは更新される可能性がある

			modified = true;

			// バージョンログファイルを
			// できる限り小さくなるようにトランケートする
			//
			//【注意】	今回、バージョンログファイルからマスタデータファイルへ
			//			版をひとつも反映していなくても、
			//			チェックポイント処理が行われた結果、
			//			使用済のブロックが破棄可能になっているかもしれないので、
			//			ここでトランケートを試みる

			if (_file._versionLog->truncate(trans))

				// バージョンログファイルは空でないので、
				// まだ、同期処理をする必要がある

				incomplete = true;
			else {
				// トランケートした結果、
				// バージョンログファイルが空になった

				// 現在のバージョンページ数を求めておく

				unsigned int pageCount;
				{
				const Block::Memory& headerMemory =
					VersionLog::FileHeader::fix(
						trans,
						0, *_file._versionLog, Buffer::Page::FixMode::ReadOnly);
				const VersionLog::FileHeader& header =
					VersionLog::FileHeader::get(headerMemory);

				pageCount = header.getPageCount();
				}
				// 既存のバージョンページのみ格納可能なように
				// マスタデータファイルをトランケートする
				//
				//【注意】	バージョンログファイルを削除してから
				//			マスタデータファイルをトランケートすると、
				//			トランケートがエラーになったときに
				//			バージョンファイルとしての整合性が失われてしまう

				_file._masterData->truncate(pageCount);

				// バージョンログファイルを削除する

				_file._versionLog->destroy();
			}
		} catch (Exception::Object& e) {

			SydMessage << e << ModEndl;
			
			// トランケートできなくても、
			// バージョンログファイルは壊れていないはずなので、
			// そのまま処理を継続する

			Common::Thread::resetErrorCondition();
			
		} catch (ModException& e) {

			SydMessage << _MOD_EXCEPTION(e) << ModEndl;

			// トランケートできなくても、
			// バージョンログファイルは壊れていないはずなので、
			// そのまま処理を継続する

			Common::Thread::resetErrorCondition();
			
		}
#ifndef NO_CATCH_ALL
		catch (std::exception& e) {

			SydMessage << "std::exception occurred. " << e.what() << ModEndl;

			// トランケートできなくても、
			// バージョンログファイルは壊れていないはずなので、
			// そのまま処理を継続する

			Common::Thread::resetErrorCondition();
			
		} catch (...) {

			SydMessage << "Unexpected Exception." << ModEndl;

			// トランケートできなくても、
			// バージョンログファイルは壊れていないはずなので、
			// そのまま処理を継続する

			Common::Thread::resetErrorCondition();
		}
#endif
	}
}

//	FUNCTION private
//	Version::File::syncPage -- バージョンページの同期を取る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			バージョンページの同期を取る
//			トランザクションのトランザクション記述子
//		Version::Page::ID&	id
//			同期を取るバージョンページのバージョンページ識別子で、
//			次に同期を取るべきバージョンページ識別子が設定される
//		unsigned int		n
//			同期を取るバージョンページの最大個数
//		bool&				modified
//			true
//				今回の同期処理でバージョンファイルを持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の同期処理でバージョンファイルを持つ
//				オブジェクトはまだ更新されていない
//
//				同期処理の結果、バージョンファイルが更新されたかを設定する
//
//	RETURN
//		true
//			まだ、同期を取っていないバージョンページが残っている
//		false
//			すべてのバージョンページを処理し終えた
//
//	EXCEPTIONS

bool
File::syncPage(const Trans::Transaction& trans,
			   Page::ID& id, unsigned int n, bool& modified, bool& isContinue)
{
	if (!_file._versionLog->isMountedAndAccessible())

		// バージョンログファイルがマウントされていない
		// バージョンログファイルの同期は取る必要がない

		return false;

	// バージョンログファイルがマウントされていれば、
	// マスタデータファイルもマウントされているはず

	; _SYDNEY_ASSERT(_file._masterData &&
					 _file._masterData->isMountedAndAccessible());

	// 自分以外に実行中のトランザクションが存在するとき、
	// ソフトリミットを1/10にし、念のため、長時間ブロックするのを避ける

	Schema::ObjectID::Value dbid = getLockName().getDatabasePart();
	if (Trans::Transaction::getInProgressList(dbid, true).getSizeWithLatch() > 1 ||
		Trans::Transaction::getInProgressList(dbid, false).getSizeWithLatch())

		n /= 10;
	
	// 前々回のチェックポイント処理の終了したときと、
	// 版管理するトランザクションが参照する可能性のあるタイムスタンプ値との
	// いずれか昔のほうのタイムスタンプ値を求める

	const Trans::TimeStamp::Value second =
		Checkpoint::TimeStamp::getSecondMostRecent(getLockName());
	const Trans::TimeStamp::Value eldest =
		ModMin(second, _Transaction::getOldestBirthTimeStamp(getLockName()));

	// 同期ログファイルが生成されていれば、
	// いつでも破棄できるようにしておく

	; _SYDNEY_ASSERT(!_file._syncLog->isAccessible());
	Common::AutoCaller0<SyncLog::File>
		syncLog(_file._syncLog, &SyncLog::File::destroy);

	ModVector<_File::_SyncPageInfo>	infoList;
	unsigned int pageCount;
	VersionLog::PBCTLevel::Value level;

	Page::ID tmp = id;

	try {
		{
		// ヘッダー読み込み時にエラーが発生すると無限ループになっていた
		
		_TRMEISTER_VERSION_FAKE_ERROR("Version::File::syncPage_readHeader",
									  Exception::Unexpected);
			
		// バージョンログファイルのファイルヘッダをフィックスし、
		// 存在するバージョンページの数と PBCT のレベルを求める

		const Block::Memory& headerMemory =
			VersionLog::FileHeader::fix(
				trans, 0, *_file._versionLog, Buffer::Page::FixMode::ReadOnly);
		const VersionLog::FileHeader& header =
			VersionLog::FileHeader::get(headerMemory);

		pageCount = header.getPageCount();
		level = header.getPBCTLevel();

		if (!pageCount || pageCount <= id ||
			level == VersionLog::PBCTLevel::Illegal)

			// バージョンページがひとつもなかったり、
			// 同期を取ろうとしているバージョンページがなかったり、
			// PBCT がないときは
			// バージョンログファイルに版はひとつも記録されていない

			return false;

		// バージョンログファイルの PBCT リーフに
		// 記録可能な最新版のブロック識別子の最大数を求める

		const unsigned int l =
			VersionLog::PBCTLeaf::getCountMax(level, getPageSize());

		// まず、同期可能なバージョンページを
		// ソフトリミットの個数に達するまで求める
		// 
		//【注意】	ある PBCT リーフを処理中にソフトリミットを越えても
		//			その PBCT リーフは完全に処理する

		Block::Memory syncLogHeaderMemory;
		bool selecting = true;

		do {
			// 今調べているバージョンページの最新版を記録する
			// バージョンログのブロック識別子を記録する
			// PBCT リーフがあれば、フィックスする

			const Page::ID first = tmp / l * l;
			const Page::ID last = first + l;

			const Block::Memory& leafMemory =
				_file._versionLog->traversePBCT(
					trans,
					0, headerMemory, first, Buffer::Page::FixMode::ReadOnly);

			if (leafMemory.getID() == Block::IllegalID) {

				// 探している PBCT リーフは存在しないので、
				// 次の PBCT リーフで管理されるべき
				// 先頭のバージョンページについて調べることにする

				tmp = last;
				continue;
			}

			// 以下の処理でバージョンファイルは更新される可能性がある

			modified = true;

			// PBCT リーフには連続する l 個のバージョンページについて、
			// それぞれの最新版を記録するバージョンログの
			// ブロック識別子が記録されている

			const VersionLog::PBCTLeaf& leaf =
				VersionLog::PBCTLeaf::get(leafMemory);

			if (!leaf.getCount()) {

				// この PBCT リーフには最新版を記録するバージョンログの
				// ブロック識別子がひとつも記録されていない
				//
				//【注意】	VersionLog::File::allocateLog の仕様から、
				//			このような場合がありえる

				// 後で PBCT のルートからこの PBCT リーフまでの経路上で
				// PBCT ノードおよびリーフを可能な限り使用済にするために
				// 先頭のバージョンページについての情報を記録しておく

				_File::_SyncPageInfo info = {
					first,
					Block::IllegalID,
					VersionLog::PBCTLeaf::normalizeID(leafMemory.getID()),
					Trans::IllegalTimeStamp,
					Trans::IllegalTimeStamp
				};
				infoList.pushBack(info);

				// 次の PBCT リーフで管理されるべき
				// 先頭のバージョンページについて調べることにする

				tmp = last;
				continue;
			}

			do {
				if (trans.isCanceledStatement())

					// 処理の中断が指示されている

					return true;

				// 今調べているバージョンページの
				// バージョンログファイル中の版のうち、
				// 前々回のチェックポイント処理より前で、
				// 現在実行中の版管理するトランザクションの開始前に
				// 生成されたものがあるか調べる
				//
				//【注意】
				// Version 2 以降は NewestTimeStamp が格納されているが、
				// 同じアルゴリズムで同じように処理できる

				const Trans::TimeStamp::Value oldest =
					leaf.getOldestTimeStamp(tmp, getPageSize());

				if (oldest < eldest) {

					// このバージョンページの
					// マスタデータファイル中の最古の版は、
					// バージョンログファイル中の版によって
					// 上書き(同期)される可能性がある

					if (!syncLog->isMountedAndAccessible()) {

						// 同期ログファイルが生成されていないので、生成する

						syncLog->create(trans);

						// 生成した同期ログファイルの
						// ファイルヘッダをフィックスする

						syncLogHeaderMemory =
							SyncLog::FileHeader::fix(
								trans, *syncLog, Buffer::Page::FixMode::Write);

						// 一度に処理するバージョンページに関する
						// 情報を記録するリスト用の領域を確保しておく

						infoList.reserve(n);
					}

					// このバージョンページの
					// マスタデータファイル中の最古の版は
					// 上書きされるかもしれないので、
					// リカバリ用に同期ログファイルにバックアップする

					Trans::TimeStamp::Value	allocation;
					(void) syncLog->allocateLog(
						trans, syncLogHeaderMemory, tmp,
						*_file._masterData, allocation);

					// 同期候補である
					// このバージョンページの情報を記憶しておく

					_File::_SyncPageInfo info = {
						tmp,
						leaf.getLatestID(tmp, getPageSize()),
						VersionLog::PBCTLeaf::normalizeID(leafMemory.getID()),
						oldest,
						allocation
					};
					infoList.pushBack(info);
				}

				if (infoList.getSize() >= n)

					// 一度に処理する数のバージョンページが見つかったら、
					// 処理すべきバージョンページをこれ以上探すのはやめて、
					// 実際に処理を行う
					//
					//【注意】	goto 文で抜けると、
					//			なぜか headerMemory がデストラクトされないので、
					//			しょうがなくフラグ制御で抜ける

					selecting = false;

				++tmp;

			} while (tmp < pageCount && tmp < last && selecting) ;
		} while (tmp < pageCount && selecting) ;

		// 同期ログファイルのファイルヘッダをフィックスしていれば、
		// ここでアンフィックスされる
		//
		//【注意】	同期ログファイルをフラッシュするときに
		//			アンフィックスされていないとフラッシュされない
		}

		if (infoList.isEmpty())

			// 同期候補のバージョンページがひとつも見つからなかった

			return (id = tmp) < pageCount;

		if (syncLog->isMountedAndAccessible())

			// 同期ログファイルにマスタデータファイルの
			// ブロックをいくつかバックアップしたので、
			// 同期ログファイルをフラッシュし、永続化する

			syncLog->flush();

	} catch (Exception::Object& e) {

		SydErrorMessage << e << ModEndl;
			
		// このバージョンファイルの同期はここまでであきらめる
		isContinue = false;

		Common::Thread::resetErrorCondition();
		return true;

	} catch (ModException& e) {

		SydErrorMessage << _MOD_EXCEPTION(e) << ModEndl;

		// このバージョンファイルの同期はここまでであきらめる
		isContinue = false;

		Common::Thread::resetErrorCondition();
		return true;

	}
#ifndef NO_CATCH_ALL
	catch (std::exception& e) {

		SydErrorMessage << "std::exception occurred. " << e.what() << ModEndl;

		// このバージョンファイルの同期はここまでであきらめる
		isContinue = false;

		Common::Thread::resetErrorCondition();
		return true;
	} catch (...) {

		SydErrorMessage << "Unexpected Exception." << ModEndl;

		// このバージョンファイルの同期はここまでであきらめる
		isContinue = false;

		Common::Thread::resetErrorCondition();
		return true;
	}
#endif

	try {
		// 求めた同期候補のバージョンページをひとつひとつ処理していく
		// バージョンログファイルのファイルヘッダを再びフィックスする

		Block::Memory headerMemory(
			VersionLog::FileHeader::fix(
				trans, 0, *_file._versionLog, Buffer::Page::FixMode::Write));

		Block::Memory leafMemory;

		const unsigned int n = infoList.getSize();
		unsigned int i = 0;

		do {
			const _File::_SyncPageInfo& info = infoList[i];
			id = info._id;

			if (trans.isCanceledStatement())

				// 処理の中断が指示されている

				break;

			if (info._latestBlockID == Block::IllegalID) {

				// この同期候補のバージョンページの最新版を記録する
				// バージョンログのブロック識別子を記録する
				// PBCT ノードにはひとつもブロック識別子が記録されていない

				// PBCT のルートからこの PBCT リーフまでの経路上で
				// PBCT ノードおよびリーフを可能な限り使用済にする

				_file._versionLog->freePBCT(trans, 0, headerMemory, id);
				continue;
			}

			// この同期候補のバージョンページの版のうち、
			// 前々回のチェックポイント処理より前に生成され、
			// かつ最新のものを、可能であれば、マスタデータファイルの
			// 最古の版を記録するブロックへ複写する

			AutoPage page(Page::attach(*this, id), false);
			const Trans::TimeStamp::Value oldest =
				_file._versionLog->syncLog(
					trans, headerMemory, *page, info._latestBlockID,
					info._oldestTimeStamp, eldest,
					*_file._masterData, info._allocationTimeStamp);

			if (oldest != info._oldestTimeStamp) {

				// このバージョンページは同期された

				if (info._leafBlockID !=
					VersionLog::PBCTLeaf::normalizeID(leafMemory.getID()))

					// このバージョンページの最新版のバージョンログの
					// ブロック識別子を記録する PBCT リーフが
					// フィックスされていないので、フィックスする
					//
					//【注意】	バージョンログファイルのファイルヘッダは
					//			すでにフィックスされている

					leafMemory = ((level) ?
								  VersionLog::PBCTLeaf::fix(
									  trans,
									  0, *_file._versionLog, info._leafBlockID,
									  Buffer::Page::FixMode::Write) :
								  headerMemory.refix());

				VersionLog::PBCTLeaf& leaf =
					VersionLog::PBCTLeaf::get(leafMemory);

				// このバージョンページの
				// バージョンログファイル中の最古の版が変わった

				leaf.setOldestTimeStamp(id, oldest, getPageSize());

				if (oldest == Trans::IllegalTimeStamp) {

					// このバージョンページには最古の版しかなくなった

					leaf.setLatestID(id, Block::IllegalID, getPageSize());

					if (!leaf.getCount())

						// 最新版を記録するバージョンログのブロック識別子が
						// この PBCT リーフにひとつも記録されなくなった

						// PBCT のルートからこの PBCT リーフまでの経路上で
						// PBCT ノードおよびリーフを可能な限り使用済にする
						//
						//【注意】	この PBCT リーフは
						//			freePBCT 内でもフィックスされる

						_file._versionLog->freePBCT(trans, 0, headerMemory, id);
				}

				leafMemory.dirty();
			}
		} while (++i < n) ;

		if (i == n)

			// 同期候補をすべて処理した

			id = tmp;

		if (syncLog->isMountedAndAccessible())

			// マスタデータファイルをフラッシュする
			//
			//【注意】	フラッシュする前に同期ログファイルが削除され、
			//			障害が発生すると、リカバリできない
			//
			//【注意】	バージョンログファイルはフラッシュしないので、
			//			以後、障害が発生しリカバリすると、
			//			マスタデータファイル中の版より古いものを記録する
			//			バージョンログが存在することがありえるが、
			//			現状の版の探索方法であれば、問題ないはず

			_file._masterData->flush();

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		// マスタデータファイルの一部が
		// バージョンログによって上書きされているので、
		// バージョンログファイルは壊れてしまった

		Checkpoint::Database::setAvailability(getLockName(), false);
		_SYDNEY_RETHROW;
	}

	return id < pageCount;
}

//	FUNCTION private
//	Version::File::discardPage --
//		バージョンファイルのすべてのバージョンページ記述子を破棄する
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
File::discardPage()
{
	// このバージョンファイルのすべてのバージョンページを管理する
	// ハッシュ表のバケットをひとつひとつ調べていく

	HashTable<Page>& table = Page::getHashTable();

	unsigned int i = 0;
	do {
		// 今調べているアドレスのバケットを得る

		HashTable<Page>::Bucket& bucket = table.getBucket(i);

		// バケットを保護するためにラッチする

		Os::AutoCriticalSection	latch(bucket.getLatch());

		// このバケットに登録されている
		// バージョンページ記述子をひとつひとつ処理していく

		HashTable<Page>::Bucket::Iterator			ite(bucket.begin());
		const HashTable<Page>::Bucket::Iterator&	end = bucket.end();

		while (ite != end) {

			// 反復子のさす要素が削除されると
			// 次の要素が得られなくなるので、ここで反復子を次に進めておく

			Page* page = &*ite;
			++ite;

			// 自分自身のバージョンページであるか調べる

			if (&page->getFile() == this) {

				// このバージョンページをバケットから除き、破棄する

				; _SYDNEY_ASSERT(!page->getRefCount());
				bucket.erase(*page);
				delete page;

				// 破棄したバージョンページが存在していた
				// バージョンファイルの参照数を 1 減らす

				detach();
			}
		}
	} while (++i < table.getLength()) ;
}

//	FUNCTION public
//	Version::File::getID -- バージョンファイル識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバージョンファイル識別子
//
//	EXCEPTIONS
//		なし

File::ID
File::getID() const
{
	; _SYDNEY_ASSERT(_file._masterData);
	return _file._masterData->getID();
}

//	FUNCTION public
//	Version::File::getSize -- 実際のファイルサイズを得る
//
//	NOTES
//		この関数の呼び出し中にチェックポイント処理が行われると、
//		正しいファイルサイズが得られない可能性がある
//
//		バージョンファイルが生成されていないとき、ファイルサイズは 0 とみなす
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた実際のファイルサイズ(B 単位)
//
//	EXCEPTIONS

Os::File::Size
File::getSize() const
{
	// マスタデータファイル、バージョンログファイル、
	// 同期ログファイルの生成・破棄の排他制御用の
	// 読み取り書き込みロックを読み取りロックする

	Os::AutoRWLock rwlock(getRWLock(), Os::RWLock::Mode::Read);

	// バージョンファイルを構成するファイルのうち、
	// 存在するものの実際のファイルサイズの総和を求める

	; _SYDNEY_ASSERT(_file._masterData);
	Os::File::Size	size = _file._masterData->getSize();

	if (_file._versionLog) {
		size += _file._versionLog->getSize();

		if (_file._syncLog)
			size += _file._syncLog->getSize();
	}

	return size;
}

//	FUNCTION public
//	Version::File::getBoundSize -- 実際に使用中のサイズを得る
//
//	NOTES
//		バージョンファイルが生成されていないとき、
//		実際に使用中のサイズは 0 とみなす
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた実際に使用中のサイズ(B 単位)
//
//	EXCEPTIONS

Os::File::Size
File::getBoundSize(const Trans::Transaction& trans)
{
	// マスタデータファイル、バージョンログファイル、
	// 同期ログファイルの生成・破棄の排他制御用の
	// 読み取り書き込みロックを読み取りロックする

	Os::AutoRWLock rwlock(getRWLock(), Os::RWLock::Mode::Read);

	// バージョンファイルを構成するファイルのうち、
	// 存在するものの使用中部分のサイズの総和を求める

	; _SYDNEY_ASSERT(_file._masterData);
	Os::File::Size size = _file._masterData->getBoundSize(trans);

	if (_file._versionLog) {
		size += _file._versionLog->getBoundSize(trans, 0);

		if (_file._syncLog)
			size += _file._syncLog->getBoundSize(trans);
	}

	return size;
}

//	FUNCTION public
//	Version::File::getParent --
//		マスタデータファイルの親ディレクトリの絶対パス名を得る
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

const Os::Path&
File::getParent() const
{
	; _SYDNEY_ASSERT(_file._masterData);
	return _file._masterData->getParent();
}

//	FUNCTION public
//	Version::File::getPageSize -- バージョンページサイズを得る
//
//	NOTES
//		バージョンページサイズとは、
//		バージョンページが占有する OS ファイル領域のサイズであり、
//		利用者が実際に使用可能なサイズとは同じであるとは限らない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバージョンページサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

Os::Memory::Size
File::getPageSize() const
{
	; _SYDNEY_ASSERT(_file._masterData);
	return _file._masterData->getBlockSize();
}

//	FUNCTION public
//	Version::File::getStorageStrategy --
//		バージョンファイルのファイル格納戦略を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイル格納戦略
//
//	EXCEPTIONS

File::StorageStrategy
File::getStorageStrategy() const
{
	StorageStrategy		strategy;

	strategy._pageSize = getPageSize();

	; _SYDNEY_ASSERT(_file._masterData);
	strategy._path._masterData = getParent();
	strategy._sizeMax._masterData = _file._masterData->getSizeMax();
	strategy._extensionSize._masterData =
		_file._masterData->getExtensionSize();

	if (_file._versionLog) {
		strategy._path._versionLog = _file._versionLog->getParent();
		strategy._sizeMax._versionLog = _file._versionLog->getSizeMax();
		strategy._extensionSize._versionLog =
			_file._versionLog->getExtensionSize();

		if (_file._syncLog) {
			strategy._path._syncLog = _file._syncLog->getParent();
			strategy._sizeMax._syncLog = _file._syncLog->getSizeMax();
			strategy._extensionSize._syncLog =
				_file._syncLog->getExtensionSize();
		}
	}

	return strategy;
}

//	FUNCTION public
//	Version::File::getBufferingStrategy --
//		バージョンファイルのバッファリング戦略を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバッファリング戦略
//
//	EXCEPTIONS
//		なし

File::BufferingStrategy
File::getBufferingStrategy() const
{
	BufferingStrategy	strategy;

	; _SYDNEY_ASSERT(_file._masterData);
	strategy._category = _file._masterData->getPoolCategory();

	return strategy;
}

//	FUNCTION public
//	Version::File::isAccessible --
//		バージョンファイルを構成する OS ファイルが存在するか調べる
//
//	NOTES
//		バージョンファイルを構成する OS ファイルのうち、
//		必要なものすべてが生成されているか調べる
//
//	ARGUMENTS
//		bool				force
//			true
//				バージョンファイルを構成する OS ファイルの存在を実際に調べる
//			false または指定されないとき
//				バージョンファイルを構成する 
//				OS ファイルの存在を必要があれば調べる
//
//	RETURN
//		true
//			存在する
//		false
//			存在しない
//
//	EXCEPTIONS
//		なし

bool
File::isAccessible(bool force) const
{
	//【注意】	バージョンログファイルは新しい版を記録するときに生成され、
	//			同期ログファイルは同期時に生成し、破棄されるので、
	//			生成されているか確認しない

	; _SYDNEY_ASSERT(_file._masterData);
	return _file._masterData->isAccessible(force);
}

//	FUNCTION public
//	Version::File::isMounted --
//		バージョンファイルがマウントされているか調べる
//
//	NOTES
//		バージョンファイルを構成するファイルのうち、
//		必要なものすべてがマウントされているか調べる
//
//	ARGUMENTS
//		Trans::Transaction&		trans
//			マウントされているか確認する
//			トランザクションのトランザクション記述子
//
//	RETURN
//		true
//			マウントされている
//		false
//			マウントされていない
//
//	EXCEPTIONS
//		なし

bool
File::isMounted(const Trans::Transaction& trans) const
{
	//【注意】	バージョンファイルを構成するファイルのうち、
	//			他のファイルが存在しないときでも
	//			マスタデータファイルは存在することがあるので、
	//			マスタデータファイルによりマウントされているか確認する

	//【注意】	マウントされかつ構成する OS ファイルが存在することを確認する

	; _SYDNEY_ASSERT(_file._masterData);
	return _file._masterData->isMountedAndAccessible() &&
		(trans.isNoVersion() || _creatorTransaction.isIllegal() ||
		 (_creatorTransaction < trans.getID() &&
		  !trans.isOverlapped(_creatorTransaction)));
}

//	FUNCTION public
//	Version::File::isReadOnly --
//		バージョンファイルが読取専用か調べる
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

bool
File::isReadOnly() const
{
	//【注意】	マスタデータファイルしか調べない

	; _SYDNEY_ASSERT(_file._masterData);
	return _file._masterData->isReadOnly();
}

//
//	FUNCTION public
//	Version::File::isBatchInsert -- バッチインサートモードかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		バッチインサートモードの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
File::isBatchInsert() const
{
	return _batch;
}

//	FUNCTION public
//	Version::File::verifySizeMax --
//		バージョンファイルを構成する各ファイルの
//		最大ファイルサイズとして与えられた値を矯正する
//
//	NOTES
//
//	ARGUMENTS
//		Os::File::Size		size
//			矯正する値(B 単位)
//
//	RETURN
//		矯正後の値(B 単位)
//
//	EXCEPTIONS

// static
Os::File::Size
File::verifySizeMax(Os::File::Size size)
{
	// システムが許可する最大ファイルサイズを求めて、
	// 必ず、それ以下になるようにする
	//
	//【注意】	0 が与えられたときは、0 のままではなんなので、
	//			特別扱いして、システムが許可する
	//			最大ファイルサイズにしてしまう

	const Os::File::Size max = Os::File::getSizeMax();
	return (size && size < max) ? size : max;
}

//	FUNCTION public
//	Version::File::verifyExtensionSize --
//		バージョンファイルを構成する各ファイルの
//		エクステンションサイズとして与えられた値を矯正する
//
//	NOTES
//
//	ARGUMENTS
//		Os::File::Size		size
//			矯正する値(B 単位)
//
//	RETURN
//		矯正後の値(B 単位)
//
//	EXCEPTIONS

// static
Os::File::Size
File::verifyExtensionSize(Os::File::Size size)
{
	// 2 のべき乗のうち、
	// システムが許可するファイルサイズの最大値以下、与えられた値以上で、
	// システムのメモリページサイズの倍数の最小のものを求める

	Os::Memory::Size i = Os::SysConf::PageSize::get();
	for (; size > i; i <<= 1) ;

	if (Os::File::getSizeMax() < i) {
		; _SYDNEY_ASSERT(false);
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	return i;
}

//	FUNCTION public
//	Version::File::getExtensionSize --
//		バージョンファイルを構成する各ファイルの
//		エクステンションサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		Os::File::Size		fileSize_
//			拡張するファイルのファイルサイズ
//
//		Os::File::Size		extensionSize_
//			エクステンションサイズ(最小値として利用)
//
//	RETURN
//		エクステンションサイズ(B 単位)
//
//	EXCEPTIONS

// static
Os::File::Size
File::getExtensionSize(Os::File::Size fileSize_,
					   Os::File::Size extensionSize_)
{
	// 2 のべき乗のうち、
	// fileSize_ の 1/16 以上、または、extensionSize_ 以上で、
	// パラメータ Version_MaxExtensionSize 以下のものを求める

	Os::File::Size min = (extensionSize_ < (fileSize_ / 16)) ?
		(fileSize_ / 16) : extensionSize_;
	Os::File::Size max = Configuration::MaxExtensionSize::get();

	min = (min < max) ? min : max;
	Os::File::Size i = Os::SysConf::PageSize::get();
	for (; min > i; i <<= 1) ;

	return i;
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
