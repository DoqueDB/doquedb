// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp -- バッファファイル関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
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
#include "SyFor.h"

#include "Buffer/Configuration.h"
#include "Buffer/Deterrent.h"
#include "Buffer/File.h"
#include "Buffer/List.h"
#include "Buffer/Manager.h"
#include "Buffer/Page.h"
#include "Buffer/Statistics.h"

#include "Common/Assert.h"
#include "Common/HashTable.h"
#include "Common/DoubleLinkedList.h"
#include "Common/Thread.h"
#include "Common/Message.h"
#include "Exception/BadDataPage.h"
#include "Exception/FileNotFound.h"
#include "Exception/TooManyOpenFiles.h"
#include "Exception/Unexpected.h"
#include "Os/AutoCriticalSection.h"
#include "Os/AutoRWLock.h"
#include "Os/SysConf.h"

#include "ModOsDriver.h"

#include "Exception/FakeError.h"
#include <new>

_SYDNEY_USING
_SYDNEY_BUFFER_USING

namespace 
{

namespace _File
{
	//	$$$::_File::_List --
	//		バッファファイル記述子を管理するリストの型
	//
	//	NOTES

	typedef Common::DoubleLinkedList<File>	_List;

	//	$$$::_File::_HashTable --
	//		すべてのバッファファイル記述子を管理するハッシュ表の型
	//
	//	NOTES

	typedef	Common::HashTable<_List, File>	_HashTable;

	// すべてのバッファファイル記述子を管理するハッシュ表のハッシュ関数
	unsigned int
	fileTableHash(const Os::Path& path);
	// 生成済のバッファファイル記述子を探す
	File*
	find(_HashTable::Bucket& bucket, const Os::Path& path);

	// OS の許可する最大ファイルサイズ
	const Os::File::Size	OsFileSizeMax = Os::File::getSizeMax();

	// 現在オープン中のバッファファイルを管理するためのリスト
	_List*					_openList = 0;
	Os::CriticalSection		_openListLatch;

	// Buffer以外が利用しているファイルディスクプリターの数
	//【注意】この数値は適当
	unsigned int			_fdCount = 20;	// 標準入出力、UNA、Modなど

	// 以下の情報を保護するためのラッチ
	Os::CriticalSection		_latch;

	// すべてのバッファファイル記述子を管理するハッシュ表
	_HashTable*				_fileTable = 0;
}

//	FUNCTION
//	$$$::_File::fileTableHash --
//		すべてのバッファファイル記述子を管理するハッシュ表に
//		登録するためのハッシュ値を計算する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			ハッシュ表に登録するバッファファイル記述子の表す
//			バッファファイルがバッファリングする OS ファイルの絶対パス名
//
//	RETURN
//		得られたハッシュ値
//
//	EXCEPTIONS
//		なし

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

//	FUNCTION
//	$$$::_File::find -- 生成済のバッファファイル記述子を探す
//
//	NOTES
//
//	ARGUMENTS
//		$$$::_File::_HashTable::Bucket&	bucket
//			バッファファイル記述子が格納されるべきハッシュ表のバケット
//		Os::Path&			path
//			バッファリングする OS ファイルの絶対パス名
//
//	RETURN
//		0 以外の値
//			得られたバッファファイル記述子を格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPTIONS

File*
_File::find(_HashTable::Bucket& bucket, const Os::Path& path)
{
	//【注意】	呼び出し側で _File::_latch をラッチする必要がある

	switch (bucket.getSize()) {
	default:
	{
		// バケットに登録されているバッファファイル記述子のうち、
		// 与えられた絶対パス名の OS ファイルをバッファリングする
		// バッファファイルのものを探す

		_HashTable::Bucket::Iterator		begin(bucket.begin());
		_HashTable::Bucket::Iterator		ite(begin);
		const _HashTable::Bucket::Iterator&	end = bucket.end();

		for (; ite != end; ++ite)
		{
			File& file = *ite;
			if (path.compare(file.getPath()) ==
				Os::Path::CompareResult::Identical) {

				// 見つかったバケットファイル記述子を
				// バケットの先頭に移動して、
				// 最近に参照されたものほど、見つかりやすくする

				bucket.splice(begin, bucket, ite);

				return &file;
			}
		}

		break;
	}
	case 1:
	{
		File& file = bucket.getFront();
		if (path.compare(file.getPath()) ==	Os::Path::CompareResult::Identical)
			return &file;

		break;
	}
	case 0:
		break;
	}

	// 見つからなかった

	return 0;
}

}

//	FUNCTION private
//	Manager::File::initialize --
//		マネージャーの初期化のうち、バッファファイル関連のものを行う
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

// static
void
Manager::File::initialize()
{
	try {
		// すべてのバッファファイル記述子を管理するハッシュ表を確保する

		_File::_fileTable = new _File::_HashTable(
			Configuration::FileTableSize::get(),
			&Buffer::File::_hashPrev, &Buffer::File::_hashNext);
		; _SYDNEY_ASSERT(_File::_fileTable);

		// オープン中のバッファファイルの
		// バッファファイル記述子を管理するためのリストを確保する

		_File::_openList = new _File::_List(
			&Buffer::File::_openedPrev, &Buffer::File::_openedNext);
		; _SYDNEY_ASSERT(_File::_openList);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		terminate();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Manager::File::terminate --
//		マネージャーの後処理のうち、バッファファイル関連のものを行う
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
	if (_File::_openList)

		// オープン中のバッファファイルの
		// バッファファイル記述子を管理するリストを破棄する

		delete _File::_openList, _File::_openList = 0;

	if (_File::_fileTable) {

		// すべてのバッファファイル記述子を管理する
		// ハッシュ表のバケットはすべて空であるべき

		; _SYDNEY_ASSERT(_File::_fileTable->isEmpty());

		// すべてのバッファファイル記述子を管理するハッシュ表を破棄する

		delete _File::_fileTable, _File::_fileTable = 0;
	}
}

//	FUNCTION private
//	Buffer::File::File -- バッファファイル記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Pool&		pool
//			バッファリングするためのバッファを供給するバッファプール
//		Os::Path&			path
//			バッファリングする OS ファイルの絶対パス名
//		Os::Memory::Size	pageSize
//			このバッファファイルのバッファページのサイズ(B 単位)
//		bool				mounted
//			true
//				このバッファファイルはマウントされている
//			false
//				このバッファファイルはマウントされていない
//		bool				readOnly
//			true
//				このバッファファイルは読取専用である
//			false
//				このバッファファイルは読取専用でない
//		bool				noCRC
//			true
//				このバッファファイルの読み込み時に、
//				正しく内容が格納されていることを CRC を使って検証しない
//			false
//				このバッファファイルの読み込み時に、
//				正しく内容が格納されていることを CRC を使って検証する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

File::File(Pool& pool, const Os::Path& path,
		   Os::Memory::Size pageSize, bool mounted, bool readOnly, bool noCRC)
	: _pageSize(Page::correctSize(pageSize))
	, _pool(pool)
	, _osFile(path)
	, _size(_File::OsFileSizeMax)
	, _refCount(0)
	, _deterrentCount(0)
	, _accessible(Accessibility::None)
	, _mounted(mounted)
	, _readOnly(readOnly || pool.getCategory() == Pool::Category::ReadOnly)
	, _noCRC(noCRC)
	, _writen(false)
	, _pageList(&Page::_filePrev, &Page::_fileNext)
	, _hashAddr(0)
	, _hashPrev(0)
	, _hashNext(0)
	, _openedPrev(0)
	, _openedNext(0)
{
	// 指定された絶対パス名の OS ファイルが存在するか調べる

	_accessible = (_osFile.access(Os::File::AccessMode::File)) ?
		Accessibility::Persisted : Accessibility::None;
}

//	FUNCTION public
//	Buffer::File::attach -- バッファファイル記述子を得る
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Pool&		pool
//			バッファリングするためのバッファを供給するバッファプール
//		Os::Path&			path
//			バッファリングする OS ファイルの絶対パス名
//		Os::Memory::Size	pageSize
//			得ようとしているバッファファイル記述子の表す
//			バッファファイルのバッファページのサイズ(B 単位)
//		bool				mounted
//			true
//				得ようとしているバッファファイル記述子の表す
//				バッファファイルはマウントされている
//			false
//				得ようとしているバッファファイル記述子の表す
//				バッファファイルはマウントされていない
//		bool				readOnly
//			true
//				得ようとしているバッファファイル記述子の表す
//				バッファファイルは読取専用である
//			false
//				得ようとしているバッファファイル記述子の表す
//				バッファファイルは読取専用でない
//		bool				noCRC
//			true
//				得ようとしているバッファファイル記述子の表す
//				バッファファイルの読み込み時に、
//				正しく内容が格納されていることを CRC を使って検証しない
//			false
//				得ようとしているバッファファイル記述子の表す
//				バッファファイルの読み込み時に、
//				正しく内容が格納されていることを CRC を使って検証する
//
//	RETURN
//		得られたバッファファイル記述子を格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
File*
File::attach(Pool& pool, const Os::Path& path,
			 Os::Memory::Size pageSize,
			 bool mounted, bool readOnly, bool noCRC)
{
	// バッファファイル記述子の
	// 生成・破棄に関する情報を保護するためにラッチする

	Os::AutoCriticalSection	latch(_File::_latch);

	// 与えられた絶対パス名の OS ファイルをバッファリングする
	// バッファファイルのバッファファイル記述子を
	// 格納すべきハッシュ表のバケットを求める

	const unsigned int addr =
		_File::fileTableHash(path) % _File::_fileTable->getLength();
	_File::_HashTable::Bucket& bucket = _File::_fileTable->getBucket(addr);

	// 与えられた絶対パス名の OS ファイルをバッファリングする
	// バッファファイルのバッファファイル記述子が
	// 求めたバケットに登録されていれば、それを得る

	File* file = _File::find(bucket, path);
	if (file) {

		// 見つかった

		; _SYDNEY_ASSERT(&file->_pool == &pool);

		// 参照回数を 1 増やす

		++file->_refCount;
	} else {

		// 見つからなかったので、生成する
		//
		// このとき、プロセスが生成されてからなん番目に生成される
		// バッファファイル記述子であるかを求めて、
		// それをバッファファイル識別子とする
		//
		//【注意】	バッファファイル識別子は必ず 0 以上の値で、
		//			ディスクへ書き込むバッファページをソートするために使用する

		; _SYDNEY_FAKE_ERROR("Buffer::File::attach", std::bad_alloc());

		file = new File(pool, path, pageSize, mounted, readOnly, noCRC);
		; _SYDNEY_ASSERT(file);

		// 生成したバッファファイル記述子が存在する間に
		// 与えられたバッファプールが破棄されると困るので、
		// 参照回数を 1 増やして、破棄されないようにする

		(void) pool.attach();

		// 生成したバッファファイル記述子を登録すべきバケットのアドレスは
		// 計算コストが高いので、バッファファイル記述子におぼえておく

		file->_hashAddr = addr;

		// 参照回数を 1 にする

		file->_refCount = 1;

		// ハッシュ表のバケットの先頭に挿入して、
		// 最近に参照されたものほど、見つかりやすくする

		bucket.pushFront(*file);
	}

	return file;
}

//	FUNCTION private
//	Buffer::File::attach -- バッファファイル記述子の参照数を 1 増やす
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		参照数が 1 増えたバッファファイル記述子を格納する領域の先頭アドレス
//
//	EXCEPTIONS

File*
File::attach()
{
	// バッファファイル記述子の
	// 生成・破棄に関する情報を保護するためにラッチする

	Os::AutoCriticalSection	latch(_File::_latch);

	// 参照数を 1 増やす

	++_refCount;

	return this;
}

//	FUNCTION public
//	Buffer::File::detach -- バッファファイル記述子の参照をやめる
//
//	NOTES
//		バッファファイル記述子の参照をやめても、
//		他のどこかで参照されていれば、バッファファイル記述子は破棄されない
//		逆にどこからも参照されていなければ、
//		バッファファイル記述子は直ちに破棄される
//
//	ARGUMENTS
//		Buffer::File*&		file
//			参照をやめるバッファファイル記述子を格納する領域の先頭アドレスで、
//			呼び出しから返ると 0 になる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
File::detach(File*& file)
{
	if (file) {
		{
		// バッファファイル記述子の
		// 生成・破棄に関する情報を保護するためにラッチする

		Os::AutoCriticalSection	latch(_File::_latch);

		// 参照数が 0 より大きければ 1 減らす

		if (file->_refCount && --file->_refCount)

			// 他から参照されているので、破棄できない

			file = 0;
		else {

			// どこからも参照されなくなったので、破棄できる
			//
			//【注意】	どこからも参照されていないので、
			//			このバッファファイルのバッファページ記述子は存在しない
			//
			//			ということは、ダーティなバッファページも存在しない

			// クローズする

			file->close();

			// 破棄するバッファファイル記述子を格納する
			// ハッシュ表のバケットを求め、そこから取り除く

			; _SYDNEY_ASSERT(_File::_fileTable);
			_File::_HashTable::Bucket& bucket =
				_File::_fileTable->getBucket(file->_hashAddr);
			bucket.erase(*file);
		}

		// ここで、バッファファイル記述子の
		// 生成・破棄に関する情報を保護するためのラッチがはずれる
		}
		if (file) {
			Pool* pool = &file->_pool;

			// どこからも参照されていないバッファファイル記述子を破棄し、
			// 与えられたポインタは 0 を指すようにする

			delete file, file = 0;

			// 破棄したバッファファイル記述子が参照していた
			// バッファプール記述子の参照数を 1 減らし、
			// どこからも参照されなくなったら破棄する

			Pool::detach(pool);
		}
	}
}

//	FUNCTION private
//	Buffer::File::detach -- バッファファイル記述子の参照数を 1 減らす
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
	// 参照数を 1 減らすバッファファイル記述子の
	// 生成・破棄に関する情報を保護するためにラッチする

	Os::AutoCriticalSection	latch(_File::_latch);
#ifdef DEBUG
	; _SYDNEY_ASSERT(_refCount);
#else
	if (_refCount)
#endif
		// 参照数が 0 より大きければ 1 減らす

		--_refCount;
}

//	FUNCTION private
//	Buffer::File::gerRefCount -- バッファファイル記述子の参照数を得る
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

unsigned int
File::getRefCount() const
{
	// 参照数を得るバッファファイル記述子の
	// 生成・破棄に関する情報を保護するためにラッチする

	Os::AutoCriticalSection	latch(_File::_latch);

	return _refCount;
}

//	FUNCTION public
//	Buffer::File::create -- 生成する
//
//	NOTES
//		すでに生成されているバッファファイルを生成してもエラーにならない
//
//	ARGUMENTS
//		bool				overwrite
//			true
//				バッファリングする OS ファイルが存在すれば、
//				それをトランケートして、そのまま使う
//			false
//				バッファリングする OS ファイルが存在すれば、生成できない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::create(bool overwrite)
{
	// バッファファイルを保護するためにラッチする

	Os::AutoCriticalSection	latch(getLatch());

	if (!isMountedAndAccessible()) {

		// バッファファイルはマウントされていない

		if (!isCreationDelayed() ||	_accessible == Accessibility::Persisted) {

			// バッファリングされる OS ファイルの生成を遅延しないか、
			// すでに永続化されている
			//
			//【注意	すでに永続化されているときでも、
			//			エラーになるか確認するために生成を試みる

			// バッファリングする OS ファイルを生成し、オープンする

			openOsFile(Os::File::OpenMode::Read |
					   Os::File::OpenMode::Write |
					   Os::File::OpenMode::Create |
					   ((overwrite) ?
						Os::File::OpenMode::Truncate :
						Os::File::OpenMode::Exclusive),
					   Configuration::FilePermission::get());

			// 存在することを記憶しておく

			_accessible = Accessibility::Persisted;

		} else if (_accessible == Accessibility::None)

			// バッファリングされる OS ファイルの生成を遅延するとき、
			// バッファページのフラッシュが必要になるまで、
			// 実際には生成されない

			_accessible = Accessibility::Volatile;

		else if (!overwrite) {

			// バッファリングされる OS ファイルは永続化されておらず、
			// 上書きしようとした

			; _SYDNEY_ASSERT(_accessible == Accessibility::Volatile);
			_SYDNEY_THROW0(Exception::Unexpected);
		}

		// マウントされていることを記憶しておく

		_mounted = true;

	} else if (overwrite)

		// バッファファイルはマウントされているので、
		// それをトランケートして、そのまま使う

		truncate();
}

//	FUNCTION public
//	Buffer::File::destroy -- 破棄する
//
//	NOTES
//		バッファファイルが生成されていなくても例外は発生しない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::destroy()
{
	// ダーティリストを切り替えられないように、読み取りロックしておく

	Os::AutoRWLock	rwlock(_pool.getRWLock(), Os::RWLock::Mode::Read);

	// バッファファイルを保護するためにラッチする

	Os::AutoCriticalSection	latch(getLatch());

	if (_mounted) {
		if (_accessible) {

			// 破棄するバッファファイルが
			// バッファリングする OS ファイルの領域を
			// バッファリングするバッファページがあれば、
			// すべて破棄する

			_pool.discardPage(Pool::DiscardablePageFilter::ForFile(*this, 0),
							  this);

			// バッファリングする OS ファイルを
			// オープンしていれば、クローズする

			closeOsFile();

			if (_accessible == Accessibility::Persisted)
				try {
					// バッファリングする OS ファイルを破棄する

					_osFile.remove();

				} catch (Exception::FileNotFound&) {

					// エラー状態を解除する

					Common::Thread::resetErrorCondition();
				}

			// キャッシュしているファイルサイズを忘れる

			_size = _File::OsFileSizeMax;

			// 存在しないことを記憶しておく

			_accessible = Accessibility::None;
		}

		// マウントされていないことを記憶しておく

		_mounted = false;
	}
}

//	FUNCTION public
//	Buffer::File::mount -- マウントする
//
//	NOTES
//		すでにマウントされている
//		バッファファイルをマウントしてもエラーにならない
//
//	ARGUMENTS
//		bool				existing
//			true
//				バッファリングする OS ファイルが
//				存在しなければ、例外を発生する
//			false
//				バッファリングする OS ファイルが
//				存在しなくても、例外を発生しない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::mount(bool existing)
{
	// バッファファイルを保護するためにラッチする

	Os::AutoCriticalSection	latch(getLatch());

	if (!isMountedAndAccessible())

		// バッファファイルはマウントされていない

		try {
			// バッファリングする OS ファイルをオープンする
			//
			//【注意】	読取専用のバッファファイルのとき、
			//			読み取りのみのオープンモードでオープンする

			openOsFile((!isReadOnly()) ?
						Os::File::OpenMode::Read | Os::File::OpenMode::Write :
					   Os::File::OpenMode::Read);

			// 存在し、マウントされていることを記憶しておく
		
			_accessible = Accessibility::Persisted, _mounted = true;

		} catch (Exception::FileNotFound&) {

			// 存在しないことを記憶しておく

			_accessible = Accessibility::None;

			if (existing) {

				// マウントされていないことを記憶しておく

				_mounted = false;
				_SYDNEY_RETHROW;
			}

			// エラー状態を解除して、
			// マウントされていることを記憶しておく

			Common::Thread::resetErrorCondition();
			_mounted = true;
		}
}

//	FUNCTION public
//	Buffer::File::unmount -- アンマウントする
//
//	NOTES
//		マウントされていない
//		バッファファイルをアンマウントしてもエラーにならない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::unmount()
{
	// ダーティリストを切り替えるかもしれないので、書き込みロックしておく

	Os::AutoRWLock	rwlock(_pool.getRWLock(), Os::RWLock::Mode::Write);

	// バッファファイルを保護するためにラッチする

	Os::AutoCriticalSection	latch(getLatch());

	if (_mounted) {
		if (_accessible) {

			// アンマウントするバッファファイルが
			// バッファリングする OS ファイルの領域のうち、
			// ダーティなものがあれば、すべてディスクへ書き込む

			_pool.flushDirtyPage(
				Pool::FlushablePageFilter::ForFile(*this, -1, false), true);

			// アンマウントするバッファファイルが
			// バッファリングする OS ファイルの領域を
			// バッファリングするバッファページがあれば、
			// すべて破棄する

			_pool.discardPage(Pool::DiscardablePageFilter::ForFile(*this, 0),
							  this);

			// バッファリングする OS ファイルを
			// オープンしていれば、クローズする

			closeOsFile();

			// キャッシュしているファイルサイズを忘れる

			_size = _File::OsFileSizeMax;
		}

		// マウントされていないことを記憶しておく

		_mounted = false;
	}
}

//	FUNCTION public
//	Buffer::File::truncate -- トランケートする
//
//	NOTES
//		指定された位置が現在のファイルサイズより大きいときの動作は保証できない
//
//	ARGUMENTS
//		Os::File::Offset	offset
//			指定されたとき
//				ファイルの先頭から指定された値(B 単位)の位置でトランケートする
//			指定されないとき
//				0 が指定されたものとする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::truncate(Os::File::Offset offset)
{
	// 与えられた位置をページサイズ境界に丸める

	offset &= ~(static_cast<Os::File::Offset>(getPageSize()) - 1);

	// ダーティリストを切り替えられないように、読み取りロックしておく

	Os::AutoRWLock	rwlock(_pool.getRWLock(), Os::RWLock::Mode::Read);

	// バッファファイルを保護するためにラッチする

	Os::AutoCriticalSection	latch(getLatch());

	if (_size > static_cast<Os::File::Size>(offset)) {

		// トランケートするバッファファイルがバッファリングする
		// OS ファイルはトランケートされるので、
		// その領域をバッファリングするバッファページがあれば、
		// すべて破棄する

		_pool.discardPage(Pool::DiscardablePageFilter::ForFile(*this, offset),
						  this);

		if (!isCreationDelayed() ||	_accessible == Accessibility::Persisted) {

			// バッファリングされる OS ファイルの生成を遅延しないか、
			// すでに永続化されている

			// バッファリングする OS ファイルが
			// クローズされていれば、オープンする

			openOsFile(Os::File::OpenMode::Read |
					   Os::File::OpenMode::Write);

			// バッファリングする OS ファイルをトランケートする

			_osFile.truncate(offset);
		}

		// キャッシュしているファイルサイズを
		// トランケート後のサイズに合わせる

		_size = offset;
	}
}

//	FUNCTION public
//	Buffer::File::extend -- 拡張する
//
//	NOTES
//		指定された位置が現在のファイルサイズより小さいときの動作は保証できない
//
//	ARGUMENTS
//		Os::File::Offset	offset
//			ファイルの先頭から指定された値(B 単位)の位置まで拡張する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::extend(Os::File::Offset offset)
{
	// 与えられた位置を切り上げて、ページサイズの倍数にする

	offset = (offset + getPageSize() - 1) &
		~(static_cast<Os::File::Offset>(getPageSize()) - 1) ;

	// バッファファイルを保護するためにラッチする

	Os::AutoCriticalSection	latch(getLatch());

	// 現在のファイルサイズと与えられた位置との差分をとり、
	// どれだけ拡張すればよいか求める

	const Os::File::Size current = getSize();

	if (const Os::Memory::Size size =
		static_cast<Os::Memory::Size>(offset - current)) {

		// ファイルをいくらか拡張する必要がある

		if (!isCreationDelayed()) {

			// バッファリングされる OS ファイルの生成を遅延しないとき、
			// 実際に OS ファイルを拡張する
			//
			//【注意】	一時データは永続化する必要がないので、
			//			バッファプールがあふれたときしかフラッシュされない

			// バッファリングする OS ファイルが
			// クローズされていれば、オープンする

			openOsFile(Os::File::OpenMode::Read |
					   Os::File::OpenMode::Write);

			void* p;
			{
			// バッファプールを保護するためにラッチする

			Os::AutoCriticalSection latch(_pool.getLatch());
	
			// OS ファイルへ書き出すための
			// 求めた拡張サイズぶんのバッファを確保する
			//
			//【注意】	すぐに捨てるのでバッファの上限を越えても確保する
			//
			//【注意】	getPageCount でページが確保されていない場所は
			//			必ず Page::verify に失敗する必要があるので、
			//			0 埋めされたバッファで領域を確保する必要がある

			bool dummy;
			p = _pool.allocateMemory(size, true, dummy, true);
			}
			try {
				// OS ファイルの末尾から、確保したバッファを書き出す

				writeOsFile(Os::File::IOBuffer(p), size, current);

			}
#ifdef NO_CATCH_ALL
			catch (Exception::Object&)
#else
			catch (...)
#endif
			{
				// オープンしたファイルをクローズする

				closeOsFile();

				Os::AutoCriticalSection latch(_pool.getLatch());

				_pool.freeMemory(p, size);
				_SYDNEY_RETHROW;
			}

			// バッファプールを保護するためにラッチする

			Os::AutoCriticalSection latch(_pool.getLatch());

			// バッファを破棄する

			_pool.freeMemory(p, size);
		}

		// キャッシュしているファイルサイズを拡張後のサイズに合わせる

		_size = offset;
	}
}

//	FUNCTION public
//	Buffer::File::rename -- バッファリングする OS ファイルの名前を変更する
//
//	NOTES
//		バッファファイルが生成されていなくてもエラーにならない
//
//	ARGUMENTS
//		Os::Path&			path
//			バッファリングする OS ファイルの新しい名前
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::rename(const Os::Path& path)
{
	// 改名後のバッファファイルのバッファファイル記述子を
	// 格納すべきハッシュ表のバケットを求める

	const unsigned int addr =
		_File::fileTableHash(path) % _File::_fileTable->getLength();
	_File::_HashTable::Bucket& dst = _File::_fileTable->getBucket(addr);

	// バッファファイル記述子の生成・破棄に関する情報を
	// 保護するためのラッチをかける

	Os::AutoCriticalSection	latch(_File::_latch);

	// 改名後のバッファファイルのバッファファイル記述子が
	// 求めたバケットにすでに登録されているか調べる

	const File* file = _File::find(dst, path);
	if (file)
		if (file == this)
			return;
		else {

			// 見つかったのは自分自身でない

			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	{
	// バッファファイルを保護するためにラッチする

	Os::AutoCriticalSection	latch(getLatch());

	// バッファリングする OS ファイルをクローズする

	closeOsFile();

	if (_accessible == Accessibility::Persisted) {

		// 変更後の名前の OS ファイルの親ディレクトリを求める

		Os::Path parent;
		if (path.getParent(parent) &&
			!Os::Directory::access(
				parent, Os::Directory::AccessMode::File))

			// 求めた親ディレクトリが存在しなければ、生成する

			Os::Directory::create(
				parent, Os::Directory::Permission::MaskOwner, true);

		// バッファリングする OS ファイルの
		// 名前を指定されたものに変更する

		_osFile.rename(path);
	} else

		// バッファリングする OS ファイルはまだ生成されていないので、
		// 指定された絶対パス名を新たに記憶する

		_osFile.setPath(path);

	// ここで、バッファファイルを保護するラッチがはずれる
	}
	// 自分自身を先ほど求めたバケットへ移動する
	//
	//【注意】	新しいバケットのアドレスはバッファファイル記述子に覚えておく

	_File::_HashTable::Bucket& src = _File::_fileTable->getBucket(_hashAddr);

	dst.splice(
		dst.begin(), src, _File::_HashTable::Bucket::Iterator(src, this));
	_hashAddr =	addr;
}

//	FUNCTION public
//	Buffer::File::open -- オープンする
//
//	NOTES
//		すでにオープンされている
//		バッファファイルをオープンしてもエラーにならない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::open()
{
	// バッファファイルを保護するためにラッチする

	Os::AutoCriticalSection	latch(getLatch());

	// 実際にバッファファイルをオープンする

	openOsFile();
}

//	FUNCTION private
//	Buffer::File::openOsFile -- バッファリングする OS ファイルをオープンする
//
//	NOTES
//		ファイルの生成が遅延されていれば、作成し、
//		読取専用のファイルであれば、読取専用のモードでオープンする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool
//			上限を超えていない場合はtrue、超えている場合はfalse
//
//	EXCEPTIONS

bool
File::openOsFile()
{
	bool result = false;
	
	//【注意】	バッファファイルはラッチ済であること

	if (isCreationDelayed() && !isReadOnly() &&
		_accessible != Accessibility::Persisted) {

		// バッファリングされる OS ファイルの生成を遅延し、
		// バッファファイルは更新可で、存在しない

		// バッファリングする OS ファイルを生成し、オープンする

		result = openOsFile(Os::File::OpenMode::Read |
							Os::File::OpenMode::Write |
							Os::File::OpenMode::Create |
							Os::File::OpenMode::Exclusive,
							Configuration::FilePermission::get());

		// 存在することを記憶しておく

		_accessible = Accessibility::Persisted;
	} else

		// バッファリングする OS ファイルをオープンする
		//
		//【注意】	読取専用のバッファファイルのとき、
		//			読み取りのみのオープンモードでオープンする

		result = openOsFile((!isReadOnly()) ?
							 Os::File::OpenMode::Read |
							 Os::File::OpenMode::Write :
							Os::File::OpenMode::Read);

	return result;
}

//	FUNCTION private
//	Buffer::File::openOsFile -- バッファリングする OS ファイルをオープンする
//
//	NOTES
//
//	ARGUMENTS
//		Os::File::OpenMode::Value	mode
//			どのようにファイルをオープンするかを表す値で、
//			Os::File::OpenMode::Value の論理和を指定する
//		Os::File::Permission::Value	permission
//			指定されたとき
//				生成するファイルの許可モードを表す値で、
//				Os::File::Permission::Value の論理和を指定する
//			指定されないとき
//				Os::File::Permission::OwnerRead |
//				Os::File::Permission::OwnerWrite が指定されたものとみなす
//
//	RETURN
//		bool
//			上限を超えていない場合はtrue、超えている場合はfalse
//
//	EXCEPTIONS

bool
File::openOsFile(
	Os::File::OpenMode::Value mode, Os::File::Permission::Value permission)
{
	//【注意】	バッファファイルはラッチ済であること

	bool result = true;

	if (!_osFile.isOpened()) {

		Os::Path parent;	// 親ディレクトリ
		
		if (mode & Os::File::OpenMode::Create) {

			// バッファリングする OS ファイルの親ディレクトリを求める

			if (getPath().getParent(parent) &&
				!Os::Directory::access(
					parent, Os::Directory::AccessMode::File))

				// 求めた親ディレクトリが存在しなければ、生成する

				Os::Directory::create(
					parent, Os::Directory::Permission::MaskOwner, true);
		}

		// オープンできなかった場合にリトライした数
		
		unsigned int retryCount = 0;

		// オープン中のバッファファイルを管理するリストを
		// 保護するためのラッチをかける

		; _SYDNEY_ASSERT(_File::_openList);
		Os::AutoCriticalSection	latch(_File::_openListLatch);

		while (true)
		{
			if (_File::_openList->getSize() >=
				Configuration::OpenFileCountMax::get() ||
				(_File::_openList->getSize() + _File::_fdCount) >=
				Os::SysConf::OpenMax::get() || retryCount) {

				// オープンすると
				// 制限数を超えてしまうか、
				// ディスクリプター全体でシステムの上限値を超えてしまうか、
				// リトライ中だったら、
				// リストの先頭からバッファファイルのクローズを試みる

				result = closeFile();
			}

			try
			{
				// バッファリングする OS ファイルを実際にオープンする

				_osFile.open(mode, permission);
			}
			catch (Exception::TooManyOpenFiles& e)
			{
				SydErrorMessage << e << ModEndl;
				Common::Thread::resetErrorCondition();
				
				// OSの上限を超えてオープンした
			
				if (++retryCount > 3)
				
					// あきらめる

					_SYDNEY_RETHROW;

				// エラー状況を解除する

				Common::Thread::resetErrorCondition();

				// ほんのちょっと待つ
				
				ModOsDriver::Thread::sleep(10);

				continue;
			}

			break;
		}

		if (parent.getLength() != 0) {

			// OS ファイルを新たに作成したので、親ディレクトリも flush する
			
			Os::Directory::flush(parent);
		}

		// ファイルサイズを一度も求めていなければ、求める

		(void) getSize();

		// リストの末尾に挿入して、
		// 最近にオープンされたものほど、クローズされにくくする

		_File::_openList->pushBack(*this);
	}
	else
	{
		// オープン要求が来たので、クローズされないように、リストの末尾にする

		Os::AutoCriticalSection	latch(_File::_openListLatch);

		_File::_List::Iterator ite(*_File::_openList, this);
		_File::_openList->splice(
			_File::_openList->end(), *_File::_openList, ite);
	}

	if (result == false)

		// 上限を超えてオープンした

		SydMessage << "Too Many Open Files" << ModEndl;

	return result;
}

//	FUNCTION public
//	Buffer::File::close -- クローズする
//
//	NOTES
//		オープンされていないバッファファイルをクローズしてもエラーにならない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::close()
{
	// バッファファイルを保護するためにラッチする

	Os::AutoCriticalSection	latch(getLatch());

	// バッファリングする OS ファイルをクローズする

	closeOsFile();
}

//	FUNCTION private
//	Buffer::File::closeOsFile -- バッファリングする OS ファイルをクローズする
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
File::closeOsFile()
{
	//【注意】	バッファファイルはラッチ済であること

	if (_osFile.isOpened()) {

		// バッファリングする OS ファイルはオープンされている
		{
		// オープン中のバッファファイルを管理するリストを
		// 保護するためのラッチをかける

		; _SYDNEY_ASSERT(_File::_openList);
		Os::AutoCriticalSection	latch(_File::_openListLatch);

		// リストからクローズした
		// バッファファイルを登録している要素を削除する

		_File::_openList->erase(*this);

		// オープンしているソケットとファイル

		// ここで、オープン中のバッファファイルを管理するリストを
		// 保護するためのラッチがはずれる
		}
		// バッファリングする OS ファイルを実際にクローズする
		// クローズ内でsyncが実行される

		_osFile.close();

		_writen = false;
	}
}

//	FUNCTION private
//	Buffer::File::readOsFile -- バッファリングする OS ファイルを読み込む
//
//	NOTES
//
//	ARGUMENTS
//		Os::File::IOBuffer&	buf
//			読み込んだデータを格納するバッファを表すクラス
//		Os::Memory::Size	size
//			読み込むデータのサイズ(B 単位)
//		Os::File::Offset	offset
//			読み込みを開始する位置のファイルの先頭からのオフセット(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::readOsFile(Os::File::IOBuffer& buf,
				 Os::Memory::Size size, Os::File::Offset offset)
{
	//【注意】	バッファファイルはラッチ済であること

	; _SYDNEY_ASSERT(_osFile.isOpened());

	Os::Memory::Size s;

	try {
		s = _osFile.read(buf, size, offset);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// 読み込みに失敗したので、ファイルをクローズする

		closeOsFile();

		_SYDNEY_RETHROW;
	}

	if (s != size)
	 
		// バッファページ全体を読み出すことができなかった

		_SYDNEY_THROW2(Exception::BadDataPage, offset, getPath());

	// 統計情報を記録する

	Statistics::record(Statistics::Category::Read, size);
}

//	FUNCTION private
//	Buffer::File::readOsFile -- バッファリングする OS ファイルを読み込む
//
//	NOTES
//
//	ARGUMENTS
//		Os::File::IOBuffer*	bufs
//			読み込んだデータを格納するバッファを表すクラスの配列
//		Os::Memory::Size	size
//			読み込むデータのサイズ(B 単位)
//		unsigned int		count
//			引数 bufs に指定する配列の長さ
//		Os::File::Offset	offset
//			読み込みを開始する位置のファイルの先頭からのオフセット(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::readOsFile(Os::File::IOBuffer* bufs, Os::Memory::Size size,
				 unsigned int count, Os::File::Offset offset)
{
	//【注意】	バッファファイルはラッチ済であること

	; _SYDNEY_ASSERT(bufs);
	; _SYDNEY_ASSERT(_osFile.isOpened());

	Os::Memory::Size s;

	try {
		s = _osFile.read(bufs, size, count, offset);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// 読み込みに失敗したので、ファイルをクローズする

		closeOsFile();

		_SYDNEY_RETHROW;
	}

	if (s != size)
	 
		// バッファページ全体を読み出すことができなかった

		_SYDNEY_THROW2(Exception::BadDataPage, offset, getPath());

	// 統計情報を記録する

	Statistics::record(Statistics::Category::Read, size);
}

//	FUNCTION private
//	Buffer::File::writeOsFile -- バッファリングする OS ファイルを書き出す
//
//	NOTES
//
//	ARGUMENTS
//		Os::File::IOBuffer&	buf
//			書き出すデータを格納するバッファを表すクラス
//		Os::Memory::Size	size
//			書き出すデータのサイズ(B 単位)
//		Os::File::Offset	offset
//			書き出しを開始する位置のファイルの先頭からのオフセット(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::writeOsFile(const Os::File::IOBuffer& buf,
				  Os::Memory::Size size, Os::File::Offset offset)
{
	//【注意】	バッファファイルはラッチ済であること

	; _SYDNEY_ASSERT(_osFile.isOpened());

	try {
		
		_osFile.write(buf, size, offset);

		_writen = true;
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// 書き出しに失敗したので、ファイルをクローズする

		closeOsFile();

		_SYDNEY_RETHROW;
	}

	// 統計情報を記録する

	Statistics::record(Statistics::Category::Write, size);
}

//	FUNCTION private
//	Buffer::File::writeOsFile -- バッファリングする OS ファイルを書き出す
//
//	NOTES
//
//	ARGUMENTS
//		Os::File::IOBuffer*	bufs
//			書き出すデータを格納するバッファを表すクラスの配列
//		Os::Memory::Size	size
//			書き出すデータのサイズ(B 単位)
//		unsigned int		count
//			引数 bufs に指定する配列の長さ
//		Os::File::Offset	offset
//			書き出しを開始する位置のファイルの先頭からのオフセット(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::writeOsFile(const Os::File::IOBuffer* bufs, Os::Memory::Size size,
				  unsigned int count, Os::File::Offset offset)
{
	//【注意】	バッファファイルはラッチ済であること

	; _SYDNEY_ASSERT(bufs);
	; _SYDNEY_ASSERT(_osFile.isOpened());

	try {
		
		_osFile.write(bufs, size, count, offset);

		_writen = true;
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// 書き出しに失敗したので、ファイルをクローズする

		closeOsFile();

		_SYDNEY_RETHROW;
	}

	// 統計情報を記録する

	Statistics::record(Statistics::Category::Write, size);
}

//	FUNCTION public
//	Buffer::File::flush --
//		ある領域をバッファリングするダーティなバッファページの
//		バッファリング内容を書き込む
//
//	NOTES
//
//	ARGUMENTS
//		Os::File::Offset	offset
//			0 以上の値
//				バッファリングする OS ファイルの先頭から
//				指定された値(B 単位)未満の領域をバッファリングする
//				バッファページのうち、すべてのダーティなものの
//				バッファリング内容を書き出す
//			それ以外の値、または指定されないとき
//				すべてのダーティなバッファページのバッファリング内容を書き出す
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::flush(Os::File::Offset offset)
{
	// バッファリングする OS ファイルは存在する必要がある

	; _SYDNEY_ASSERT(isAccessible());

	{
		// ダーティリストを切り替えるので、書き込みロックしておく

		Os::AutoRWLock	rwlock(_pool.getRWLock(), Os::RWLock::Mode::Write);

		_pool.flushDirtyPage(
			Pool::FlushablePageFilter::ForFile(*this, offset, false), true);
	}
	
	//【注意】	Windowsの場合はファイルオープン時に必ず
	//			FILE_FLAG_NO_BUFERING が指定されているので、
	//			明示的にフラッシュする必要はない
	
#ifndef SYD_OS_WINDOWS
	{
		// OSがキャッシュしている内容を書き出す
		
		Os::AutoCriticalSection	latch(getLatch());
		sync();
	}
#endif
}

//
//	FUNCTION public
//	Buffer::File::sync -- syncする
//
//	NOTES
//	OSがキャッシュしている内容をDISKに書き出す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::sync()
{
	//【注意】	Windowsの場合はファイルオープン時に必ず
	//			FILE_FLAG_NO_BUFERING が指定されているので、
	//			明示的にフラッシュする必要はない
	
#ifndef SYD_OS_WINDOWS
	
	//【注意】	バッファファイルはラッチ済であること

	if (_writen)
	{
		_osFile.flush();
		
		_writen = false;
	}
#endif
}

//	FUNCTION public
//	Buffer::File::startDeterrent -- バッファファイルのフラッシュを抑止する
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
File::startDeterrent()
{
	Os::AutoRWLock rwlock(Deterrent::getRWLock(), Os::RWLock::Mode::Write);

	// まず、参照数を増やし、フラッシュが抑止されている
	// バッファファイルの記述子は破棄されないようにする

	(void) attach();

	// フラッシュの抑止の入れ子回数を 1 増やす

	++_deterrentCount;
}

//	FUNCTION public
//	Buffer::File::endDeterrent -- バッファファイルのフラッシュの抑止をやめる
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
File::endDeterrent()
{
	Os::AutoRWLock rwlock(Deterrent::getRWLock(), Os::RWLock::Mode::Write);

	if (_deterrentCount) {

		// フラッシュの抑止の入れ子回数を 1 減らす

		--_deterrentCount;

		// フラッシュを抑止中は、バッファファイルの記述子が
		// 破棄されないように参照数を増やしていたので、減らしておく

		detach();
	}
}

//	FUNCTION private
//	Buffer::File::isDeterred -- バッファファイルのフラッシュが抑止されているか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			フラッシュは抑止されている
//		false
//			フラッシュは抑止されていない
//
//	EXCEPTIONS
//		なし

bool
File::isDeterred() const
{
	//【注意】	呼び出し側で Deterrent::getRWLock() に対して
	//			Read ロックをかけている必要がある

	return _deterrentCount;
}

//	FUNCTION public
//	Buffer::File::getSize -- バッファファイルのファイルサイズを得る
//
//	NOTES
//		バッファファイルが生成されていないとき、ファイルサイズは 0 とみなす
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイルサイズ(B 単位)
//
//	EXCEPTIONS

Os::File::Size
File::getSize() const
{
	// バッファファイルを保護するためにラッチする

	Os::AutoCriticalSection	latch(getLatch());

	if (_size == _File::OsFileSizeMax)

		// ファイルサイズを一度も求めていないので、求める
		//
		//【注意】	ファイルが存在しないときは、0 とみなす

		_size = (_osFile.isOpened()) ? _osFile.getSize() :
		(_accessible == Accessibility::Persisted) ?
			Os::File::getSize(_osFile.getPath()) : 0;

	return _size;
}



//	FUNCTION public
//	Buffer::File::getPageCount -- バッファファイルのバッファページ数を求める
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバッファページ数
//
//	EXCEPTIONS

unsigned int
File::getPageCount(const Trans::Transaction* trans)
{
	// OS ファイルのサイズを求めて、ページサイズの倍数に切り下げる

	Os::File::Size size =
		getSize() & ~(static_cast<Os::File::Size>(getPageSize()) - 1);

	// バッファファイルの末尾から
	// 正しいバッファページを読み出せるまで、
	// ひとつづつページを読み出していく

	for (; size >= getPageSize(); size -= getPageSize()) {
		try {
			Page::fix(*this, size - getPageSize(),
					  Buffer::Page::FixMode::ReadOnly,
					  Buffer::ReplacementPriority::Low, trans);

		} catch (Exception::BadDataPage&) {

			// このバッファページの内容はおかしいので、
			// バッファページが存在しないものとみなす

			Common::Thread::resetErrorCondition();
			continue;
		}

		// 正しいページを読み出せたので、
		// このバッファページをバッファファイルの末尾のバッファページとみなす

		break;
	}

	return static_cast<unsigned int>(size / getPageSize());
}

//	FUNCTION public
//	Buffer::File::isAccessible --
//		バッファファイルを構成する OS ファイルが存在するか調べる
//
//	NOTES
//
//	ARGUMENTS
//		bool				force
//			true
//				バッファファイルの実体である OS ファイルの存在を実際に調べる
//			false または指定されないとき
//				バッファファイルの実体である
//				OS ファイルの存在を必要があれば調べる
//
//	RETURN
//		true
//			存在する
//		false
//			存在しない
//
//	EXCEPTIONS

bool
File::isAccessible(bool force) const
{
	// バッファファイルを保護するためにラッチする

	Os::AutoCriticalSection	latch(getLatch());

	if (force || !_mounted) {

		// マウントされていないときは、バッファファイルの実体である
		// OS ファイルを自由に移動できるので、調べなおす

		_accessible = (_osFile.access(Os::File::AccessMode::File)) ?
			Accessibility::Persisted : Accessibility::None;
	}

	return _accessible;
}

//	FUNCTION public
//	Buffer::File::isMounted --
//		バッファファイルはマウントされているか
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

bool
File::isMounted() const
{
	// バッファファイルを保護するためにラッチする

	Os::AutoCriticalSection	latch(getLatch());

	return _mounted;
}

//	FUNCTION public
//	Buffer::File::isMountedAndAccessible --
//		バッファファイルはマウントされかつ構成する OS ファイルが存在するか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			マウントされかつ構成する OS ファイルは存在する
//		false
//			マウントされていないか、構成する OS ファイルは存在しない
//
//	EXCEPTIONS

bool
File::isMountedAndAccessible() const
{
	// バッファファイルを保護するためにラッチする

	Os::AutoCriticalSection	latch(getLatch());

	return _mounted && _accessible;
}

//
//	FUNCTION public
//	Buffer::File::reserveDescriptor -- ディスクリプターを1つ予約する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		予約できた場合はtrue、できなかった場合はfalse
//
//	EXCEPTIONS
//
/*static*/
bool
File::reserveDescriptor()
{
	//	【注意】
	//	ソケット等のバッファモジュール以外にファイルディスクリプタを消費する
	//	場合にこの関数を呼び出しファイルディスクリプタを予約する
	//	ファイルディスクリプタの総量規制のため
	
	// ファイルをクローズできなかった場合にリトライした数
		
	unsigned int retryCount = 0;

	// オープン中のバッファファイルを管理するリストを
	// 保護するためのラッチをかける

	; _SYDNEY_ASSERT(_File::_openList);
	Os::AutoCriticalSection	latch(_File::_openListLatch);

	if ((_File::_fdCount + 100) >= Os::SysConf::OpenMax::get())

		// バッファモジュール以外でシステムの上限 - 100を消費してしまっている

		return false;

	while ((_File::_openList->getSize() + _File::_fdCount)
		   >= Os::SysConf::OpenMax::get())
	{

		// ディスクリプターを1つ消費すると
		// ディスクリプター全体でシステムの上限値を超えてしまうので、
		// リストの先頭からバッファファイルのクローズを試みる

		if (closeFile())

			// 1つクローズできたので終了

			break;

		// ほんのちょっと待つ

		ModOsDriver::Thread::sleep(10);

		if (++retryCount > 3)

			// 3回リトライしたがディスクリプターを予約できなかった

			return false;
	}

	++_File::_fdCount;

	return true;
}

//
//	FUNCTION public
//	Buffer::File::returnDescriptor -- ディスクリプターを1つ開放する
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
//
/*static*/
void
File::returnDescriptor()
{
	// オープン中のバッファファイルを管理するリストを
	// 保護するためのラッチをかける

	if (_File::_openList)
	{
		Os::AutoCriticalSection	latch(_File::_openListLatch);

		--_File::_fdCount;
	}
	else
	{
		// 【注意】
		//	Server::Manager::terminateから呼ばれるタイミングが
		//	Bufferのterminateが呼ばれた後であるので、
		//	_File::_openListがnullになっている
		
		--_File::_fdCount;
	}
}

//
//	FUNCTION public
//	Buffer::File::syncAllFiles --
//		オープンしているすべてのファイルをsyncする
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
//
void
File::syncAllFiles()
{
	//【注意】	Windowsの場合はファイルオープン時に必ず
	//			FILE_FLAG_NO_BUFERING が指定されているので、
	//			明示的にフラッシュする必要はない

#ifndef SYD_OS_WINDOWS	
	//【注意】
	//	チェックポイントスレッドから実行されることを想定している
	//	他からアクセスされることはないはずなので、ファイルのロックは行わない

	Os::AutoCriticalSection	latch(_File::_openListLatch);

	_File::_List::Iterator i = _File::_openList->begin();
	_File::_List::Iterator e = _File::_openList->end();
	while (i != e)
	{
		// OSがキャッシュしている内容を書き出す
		
		(*i).sync();
		
		++i;
	}
#endif
}

//	FUNCTION private
//	Buffer::File::isCreationDelayed --
//		バッファリングする OS ファイルの生成を遅延するか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			生成を遅延する
//		false
//			生成を遅延しない
//
//	EXCEPTIONS

bool
File::isCreationDelayed() const
{
	return _pool.getCategory() == Pool::Category::Temporary &&
		Configuration::DelayTemporaryCreation::get();
}

//
//	FUNCTION private
//	Buffer::File::closeFile -- バッファファイルのクローズを試みる
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		ファイルがクローズできた場合はtrue、できなかった場合はfalse
//
//	EXCEPTIONS
//
/*static*/
bool
File::closeFile()
{
	//【注意】
	//	バッファファイルを管理するリストはラッチ済みであること

	//	できるだけ昔にオープンされたものをクローズしようとする

	_File::_List::Iterator			ite = _File::_openList->begin();
	const _File::_List::Iterator&	end = _File::_openList->end();

	for (; ite != end; ++ite)
	{
		File& file = *ite;

		// バッファファイルを保護するためにラッチを試みる
		//
		//【注意】	ここでラッチ待ちすると、
		//			ほかの File::open とデッドロックを起こす

		Os::AutoTryCriticalSection	latch(file.getLatch());

		if (latch.isLocked()) {

			// ラッチできたのでこのバッファファイルが
			// バッファリングする OS ファイルをクローズする

			file.closeOsFile();

			return true;

		}
	}

	//【注意】	クローズできなければ、あきらめる

	return false;
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
