// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogFile.cpp -- 論理ログファイル情報関連の関数定義
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2012, 2014, 2015, 2016, 2017, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Trans";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Trans/LogFile.h"
#include "Trans/Configuration.h"
#include "Trans/LogData.h"
#include "Trans/Manager.h"
#include "Trans/PathParts.h"
#include "Trans/TimeStamp.h"

#include "Checkpoint/Database.h"
#include "Checkpoint/TimeStamp.h"
#include "Checkpoint/LogData.h"
#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/CompressedData.h"
#include "Common/DoubleLinkedList.h"
#include "Common/HashTable.h"
#include "Common/InputArchive.h"
#include "Common/Message.h"
#include "Common/UnsignedInteger64Data.h"
#include "Common/OutputArchive.h"
#include "Communication/Connection.h"
#include "LogicalLog/Log.h"
#include "Os/AutoCriticalSection.h"
#include "Os/Memory.h"
#include "Schema/ObjectID.h"
#include "Schema/LogData.h"

#include "Exception/BadArgument.h"
#include "Exception/Unexpected.h"

#include "ModAutoPointer.h"
#include "ModMap.h"
#include "ModMemory.h"

_SYDNEY_USING
_SYDNEY_TRANS_USING
_SYDNEY_TRANS_LOG_USING

namespace
{

typedef Common::HashTable<Common::DoubleLinkedList<File>, File>	_HashTable;

namespace _File
{
	// すべての論理ログファイルに関する情報を表すクラスを管理する
	// ハッシュ表に登録するためのハッシュ値を計算する
	unsigned int			fileTableHash(const Os::Path& path);

	// 生成済の論理ログファイルに関する情報を表すクラスを探す
	File*
	find(_HashTable::Bucket& bucket, const Os::Path& path);

	// 以下の情報を保護するためのラッチ
	Os::CriticalSection		_latch;
	// すべての論理ログファイルに関する情報を表すクラスを管理するハッシュ表
	_HashTable*				_fileTable = 0;

	typedef ModPair<ModUnicodeString, int>	_MapKey;
	typedef ModLess<_MapKey>				_MapLess;
	typedef ModMap<_MapKey, File::MasterThread*, _MapLess>	_ThreadMap;

	// 以下の情報を保護するためのラッチ
	Os::CriticalSection		_latch2;
	// スレーブサーバごとに存在するスレッドを管理するマップ
	_ThreadMap				_threadMap;
}

namespace _TimeStamp
{
	// パス名を得る
	Os::Path
	getPath(const Os::Path& path);
}

namespace _Data
{
	//	CLASS
	//	$$$::_Data::_Compressed -- 圧縮データを表すクラス
	//
	//	NOTES

	class _Compressed
	{
	public:
		// コンストラクター
		_Compressed(const void* uncompressed,
					Os::Memory::Size uncompressedSize);
		// デストラクター
		~_Compressed();

		// 圧縮されたデータを格納する領域の先頭アドレス
		void*				_p;
		// 圧縮されたデータを格納する領域のサイズ(B 単位)
		Os::Memory::Size	_size;
	};

	//	CLASS
	//	$$$::_Data::_Uncompressed -- 解凍データを表すクラス
	//
	//	NOTES

	class _Uncompressed
	{
	public:
		// コンストラクター
		_Uncompressed(const void* compressed,
					  Os::Memory::Size compressedSize);
		// デストラクター
		~_Uncompressed();

		// 解凍されたデータを格納する領域の先頭アドレス
		void*				_p;
		// 解凍されたデータを格納する領域のサイズ(B 単位)
		Os::Memory::Size	_size;
	};
}

//	FUNCTION
//	$$$::_File::fileTableHash --
//		すべての論理ログファイルに関する情報を表すクラスを管理する
//		ハッシュ表に登録するためのハッシュ値を計算する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			ハッシュ表に登録する情報の元である論理ログファイルの
//			実体の OS ファイルの絶対パス名
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
//	$$$::_File::find --
//		生成済のある論理ログファイルに関する情報を表すクラスを探す
//
//	NOTES
//
//	ARGUMENTS
//		$$$::_HashTable::Bucket&	bucket
//			論理ログファイルに関する情報を表すクラスが
//			格納されるべきハッシュ表のバケット
//		Os::Path&			path
//			論理ログファイルの実体である OS ファイルの絶対パス名
//
//	RETURN
//		0 以外の値
//			得られた論理ログファイルに関する情報を表すクラスを
//			格納する領域の先頭アドレス
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
		// バケットに登録されているある論理ログファイルに関する情報のうち、
		// 実体が同じ OS ファイルである論理ログファイルのものを探す

		_HashTable::Bucket::Iterator		begin(bucket.begin());
		_HashTable::Bucket::Iterator		ite(begin);
		const _HashTable::Bucket::Iterator&	end = bucket.end();

		do {
			File& file = *ite;

			if (file.getPath().compare(path) ==
				Os::Path::CompareResult::Identical) {

				// 見つかったある論理ログファイルに関する情報を表すクラスを
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

		if (file.getPath().compare(path) ==	Os::Path::CompareResult::Identical)

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
//	$$$::_TimeStamp::getPath --
//		前々回のチェックポイント終了時の
//		タイムスタンプを記録する専用のファイルの絶対パス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			この絶対パス名の OS ファイルを実体とする
//			論理ログファイルを使用するデータベースに関する
//			前々回のチェックポイント終了時のタイムスタンプを記録する
//			専用のファイルの絶対パス名を得る
//
//	RETURN
//		得られたファイルの絶対パス名
//
//	EXCEPTIONS

Os::Path
_TimeStamp::getPath(const Os::Path& path)
{
	Os::Path p = path;
	return p.addPart(PathParts::TimeStamp);
}

//	FUNCTION public
//	$$$::_Data::_Compressed::_Compressed --
//		圧縮データを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		void*		uncompressed
//			解凍されたデータを格納する領域の先頭アドレス
//		Os::Memory::Size	uncompressedSize
//			解凍されたデータを格納する領域のサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

_Data::_Compressed::_Compressed(
	const void* uncompressed, Os::Memory::Size uncompressedSize)
	: _p(0),
	  _size(0)
{
	; _SYDNEY_ASSERT(uncompressed);
	; _SYDNEY_ASSERT(uncompressedSize);

	_size = uncompressedSize;

	_p = Os::Memory::allocate(_size + sizeof(uncompressedSize));
	_SYDNEY_ASSERT(_p);

	try {
		Common::CompressedData::compress(
			static_cast<char*>(_p) + sizeof(uncompressedSize), _size,
			uncompressed, uncompressedSize);

	} catch (...) {

		Os::Memory::free(_p);
		_SYDNEY_RETHROW;
	}

	(void) Os::Memory::copy(_p, &uncompressedSize, sizeof(uncompressedSize));

	_size += sizeof(uncompressedSize);
}

//	FUNCTION public
//	$$$::_Data::_Compressed::~_Compressed --
//		圧縮データを表すクラスのデストラクター
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

_Data::_Compressed::~_Compressed()
{
	Os::Memory::free(_p), _size = 0;
}

//	FUNCTION public
//	$$$::_Data::_Uncompressed::_Uncompressed --
//		解凍データを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		void*		compressed
//			圧縮データを格納する領域の先頭アドレス
//		Os::Memory::Size	compressedSize
//			圧縮データを格納する領域のサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

_Data::_Uncompressed::_Uncompressed(
	const void* compressed, Os::Memory::Size compressedSize)
	: _p(0),
	  _size(0)
{
	; _SYDNEY_ASSERT(compressed);
	; _SYDNEY_ASSERT(compressedSize > sizeof(_size));

	(void) Os::Memory::copy(&_size, compressed, sizeof(_size));
	; _SYDNEY_ASSERT(_size);

	_p = Os::Memory::allocate(_size);
	; _SYDNEY_ASSERT(_p);

	compressedSize -= sizeof(_size);
	try {
		if (compressedSize < _size)
			Common::CompressedData::uncompress(
				_p, _size, static_cast<const char*>(compressed) + sizeof(_size),
				compressedSize);
		else
			(void) Os::Memory::copy(
				_p, static_cast<const char*>(compressed) + sizeof(_size),
				compressedSize);

	} catch (...) {

		Os::Memory::free(_p);
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	$$$::_Data::_Uncompressed::~_Uncompressed --
//		解凍データを表すクラスのデストラクター
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

_Data::_Uncompressed::~_Uncompressed()
{
	Os::Memory::free(_p), _size = 0;
}

}

//	FUNCTION private
//	Trans::Manager::Log::File::initialize --
//		マネージャーの初期化のうち、
//		ある論理ログファイルに関する情報関連のものを行う
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
Manager::Log::File::initialize()
{}

//	FUNCTION private
//	Trans::Manager::Log::File::terminate --
//		マネージャーの後処理のうち、
//		ある論理ログファイルに関する情報関連のものを行う
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
Manager::Log::File::terminate()
{
	//【注意】	他のスレッドが同時に実行されることはないので、
	//			ラッチしない

	if (_File::_fileTable) {

		// チェックポイント処理の終了時に可能であれば
		// トランケートする論理ログファイルに関する
		// 情報を表すクラスが存在すれば、破棄する

		unsigned int i = 0;
		do {
			_HashTable::Bucket&	bucket = _File::_fileTable->getBucket(i);

			while (bucket.getSize()) {
				Trans::Log::File& file = bucket.getFront();
				bucket.popFront();
				delete &file;
			}
		} while (++i < _File::_fileTable->getLength()) ;

		// すべての論理ログファイルに関する
		// 情報を表すクラスを管理するハッシュ表を破棄する

		delete _File::_fileTable, _File::_fileTable = 0;
	}

	// スレーブサーバとの通信用のスレッドを停止する

	_File::_ThreadMap::Iterator i = _File::_threadMap.begin();
	for (; i != _File::_threadMap.end(); ++i)
	{
		(*i).second->abort();
		(*i).second->join();
		delete (*i).second;
	}
	_File::_threadMap.erase(_File::_threadMap.begin(), _File::_threadMap.end());
}

//
//	FUNCTION public
//	Trans::Log::File::Queue::Queue
//		-- コンストラクタ
//
File::Queue::Queue()
	: m_vecQueue(), m_cLatch(), m_cEvent(Os::Event::Category::ManualReset),
	  m_iRefCount(0), m_bActive(true)
{
}

//
//	FUNCTION public
//	Trans::Log::File::Queue::attach
//		-- 参照カウンタを１つ増やす
//
int
File::Queue::attach()
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	++m_iRefCount;

	return m_iRefCount;
}

//
//	FUNCTION public
//	Trans::Log::File::Queue::detach
//		-- 参照カウンタを１つ減らす
//
int
File::Queue::detach()
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	
	--m_iRefCount;

	return m_iRefCount;
}

//
//	FUNCTION public
//	Trans::Log::File::Queue::pushBack
//		-- キューの末尾に追加する
//
void
File::Queue::pushBack(Schema::ObjectID::Value databaseid_,
					  int category_,
					  const void* p_, ModSize size_, LSN lsn_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	// 配列の末尾に追加
	// 読み出さなくてもどんどん追加していく

	// 論理ログはコピーする
	
	void* b = Os::Memory::allocate(size_);
	Os::Memory::copy(b, p_, size_);

	// キューに追加
	
	m_vecQueue.pushBack(QueueData(databaseid_, category_, b, size_, lsn_));

	// 追加したので、イベントをシグナル化
	
	m_cEvent.set();
}

//
//	FUNCTION public
//	Trans::Log::File::Queue::popFront
//		-- キューの先頭を取り出す
//
bool
File::Queue::popFront(Schema::ObjectID::Value& databaseid_,
					  int& category_,
					  void*& p_, ModSize& size_, LSN& lsn_, int timeout_)
{
	Os::AutoTryCriticalSection cAuto(m_cLatch);
	cAuto.lock();

	while (true)
	{
		if (m_vecQueue.getSize() != 0)
		{
			// キューにデータが格納されているので、
			// 先頭を取得する
		
			ModVector<QueueData>::Iterator i = m_vecQueue.begin();

			databaseid_ = (*i).m_uiDatabaseID;
			category_ = (*i).m_iCategory;
			p_ = (*i).m_pBuffer;
			size_ = (*i).m_uiSize;
			lsn_ = (*i).m_ulLSN;

			m_vecQueue.popFront();

			return true;
		}

		// キューにデータがないので、イベントをリセットして
		// イベントがシグナル化されるのを待つ

		m_cEvent.reset();
		cAuto.unlock();

		if (m_cEvent.wait(timeout_) == false)
		{
			// データが書き込まれなかった

			break;
		}

		cAuto.lock();
	}

	return false;
}

//
//	FUNCTION public
//	Trans::Log::File::Queue::isActive
//		-- 動作中か？
//
bool
File::Queue::isActive()
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	return m_bActive;
}

//
//	FUNCTION public
//	Trans::Log::File::Queue::setInActive
//		-- 動作中ではなくする
//
void
File::Queue::setInActive()
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	m_bActive = false;
}

//
//	FUNCTION public
//	Trans::Log::File::MasterThread::MasterThread
//		-- コンストラクタ
//
//	NOTES
//
//	ARUGMENTS
//	Communication::Connection* pSlaveConnection_
//		スレーブとのコネクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
File::MasterThread::
MasterThread(Communication::Connection* pSlaveConnection_,
			 const ModUnicodeString& cSlaveHostName_,
			 int iSlavePortNumber_)
	: m_pQueue(0),
	  m_pSlaveConnection(pSlaveConnection_),
	  m_cSlaveHostName(cSlaveHostName_),
	  m_iSlavePortNumber(iSlavePortNumber_)
{
	m_pQueue = new Queue();
	m_pQueue->attach();
}

//
//	FUNCTION public
//	Trans::Log::File::MasterThread::runnable
//		-- スレッドとして実行されるメソッド
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
File::MasterThread::runnable()
{
	if (m_pSlaveConnection == 0)
		return;
	
	while (true)
	{
		Schema::ObjectID::Value dbid;
		int category;
		void* p = 0;
		ModSize size = 0;
		LSN lsn;
		
		if (m_pQueue->popFront(dbid, category, p, size, lsn, 500) == false)
		{
			// 終了をチェックする
			if (isAborted() == false)
				continue;
		}

		// 終了をチェックする
		if (isAborted() == true)
		{
			// 終了
			
			if (m_pSlaveConnection)
			{
				// スレーブとのコネクションをクローズする
				
				m_pSlaveConnection->close();
				delete m_pSlaveConnection, m_pSlaveConnection = 0;
			}

			// 停止中とする
			
			m_pQueue->setInActive();

			// 参照数を１減らす

			if (m_pQueue->detach() == 0)

				delete m_pQueue, m_pQueue = 0;
				
			break;
		}

		// 全スレーブに論理ログを転送する
		//
		// bAllocate_ = false で、かつ、uiAllocatedSize_ = 0 とすると、
		// 渡した領域は解放されない

		Common::UnsignedIntegerData cID(dbid);
		Common::BinaryData cData(p, size, false, 0);
		Common::UnsignedInteger64Data cLSN(lsn);

		try
		{
			m_pSlaveConnection->writeObject(&cID);
			m_pSlaveConnection->writeObject(&cData);
			m_pSlaveConnection->writeObject(&cLSN);

			switch (category)
			{
			case Data::Category::TransactionCommit:
			case Data::Category::TransactionRollback:
			case Data::Category::CheckpointDatabase:

				// フラッシュする
				m_pSlaveConnection->flush();

			default:
				;
			}
		}
		catch (Exception::Object& e)
		{
			SydInfoMessage << "SlaveConnection: "
						   << m_cSlaveHostName << ":"
						   << m_iSlavePortNumber << ModEndl;
			SydInfoMessage << "SlaveConnection Error: "
						   << e << ModEndl;

			m_pSlaveConnection->close();
			delete m_pSlaveConnection, m_pSlaveConnection = 0;
		}
#ifndef NO_CATCH_ALL
		catch (...)
		{
			SydInfoMessage << "SlaveConnection: "
						   << m_cSlaveHostName << ":"
						   << m_iSlavePortNumber << ModEndl;
			SydInfoMessage << "SlaveConnection Error: UnexpectedError"
						   << ModEndl;

			m_pSlaveConnection->close();
			delete m_pSlaveConnection, m_pSlaveConnection = 0;
		}
#endif
 
		// メモリを解放する

		Os::Memory::free(p);

		if (m_pSlaveConnection == 0)
		{
			// スレーブとのコネクションに問題があった

			// 停止中とする
			
			m_pQueue->setInActive();

			// 参照数を１減らす

			if (m_pQueue->detach() == 0)

				delete m_pQueue, m_pQueue = 0;
				
			break;
		}
	}
}

//	FUNCTION public
//	Trans::Log::File::attach --
//		ある論理ログファイルに関する情報を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::File::StorageStrategy&	storageStrategy
//			論理ログファイルのファイル格納戦略
//		Lock::LogicalLogName&	lockName
//			論理ログファイルを表すロック名
//		ModUnicodeString&	dbName
//			論理ログファイルに記録する操作の対象であるデータベースの名前
//
//	RETURN
//		得られたある論理ログファイルに関する情報を表すクラスを
//		格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
File*
File::attach(const StorageStrategy& storageStrategy,
			 const Lock::LogicalLogName& lockName,
			 const ModUnicodeString& dbName)
{
	// ある論理ログファイルの情報を表すクラスの
	// 生成・破棄に関する情報を保護するためにラッチをかける

	Os::AutoCriticalSection	latch(_File::_latch);

	if (!_File::_fileTable) {

		// すべての論理ログファイルに関する
		// 情報を表すクラスを管理するハッシュ表が確保されていないので、
		// まず、確保する

		_File::_fileTable =
			new _HashTable(Configuration::LogFileTableSize::get(),
						   &File::_hashPrev, &File::_hashNext);
		; _SYDNEY_ASSERT(_File::_fileTable);
	}

	// 論理ログファイルの実体である OS ファイルの絶対パス名が
	// 指定されたものである論理ログファイルの情報を格納すべき
	// ハッシュ表のバケットを求める

	const unsigned int addr =
		_File::fileTableHash(storageStrategy._path) %
			_File::_fileTable->getLength();
	_HashTable::Bucket& bucket = _File::_fileTable->getBucket(addr);

	// 論理ログファイルの実体である OS ファイルの絶対パス名が
	// 指定されたものである論理ログファイルの情報を表すクラスが
	// 求めたバケットに登録されていれば、それを得る

	File* file = _File::find(bucket, storageStrategy._path);
	if (file)

		// 見つかったので、参照数を 1 増やす

		++file->_refCount;
	else {

		// 見つからなかったので、生成する

		file = new File(storageStrategy, lockName, dbName);
		; _SYDNEY_ASSERT(file);

		// 生成したある論理ログファイルに関する情報を表すクラスを
		// 登録すべきバケットのアドレスは、計算コストが高いので、覚えておく

		file->_hashAddr = addr;

		// 参照数を 1 にする

		file->_refCount = 1;

		// ハッシュ表のバケットの先頭に挿入して、
		// 最近に参照されたものほど、見つかりやすくする

		bucket.pushFront(*file);
	}

	return file;
}

//	FUNCTION public
//	Trans::Log::File::attach --
//		ある論理ログファイルに関する情報を表すクラスの参照数を 1 増やす
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		参照数が 1 増えた自分自身を格納する領域の先頭アドレス
//
//	EXCEPTIONS

File*
File::attach()
{
	// ある論理ログファイルの情報を表すクラスの
	// 生成・破棄に関する情報を保護するためにラッチをかける

	Os::AutoCriticalSection	latch(_File::_latch);

	// 参照数を 1 増やす

	++_refCount;

	return this;
}

//	FUNCTION public
//	Trans::Log::File::detach --
//		ある論理ログファイルに関する情報を表すクラスの参照をやめる
//
//	NOTES
//		ある論理ログファイルに関する情報を表すクラスの参照をやめても、
//		他のどこかで参照されていれば、そのクラスは破棄されない
//		逆にどこからも参照されていないとき、そのクラスは破棄される
//
//	ARGUMENTS
//		Trans::Log::File*&	file
//			参照をやめるある論理ログファイルに関する
//			情報を表すクラスを格納する領域の先頭アドレスで、
//			呼び出しから返ると、0 になる
//		bool				reserve
//			true
//				どこからも参照されなくなった論理ログファイルに関する
//				情報を表すクラスでも、また参照されるときのために
//				破棄せずにとっておく
//			false
//				どこからも参照されなくなった論理ログファイルに関する
//				情報を表すクラスは破棄する
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

		// ある論理ログファイルの情報を表すクラスの
		// 生成・破棄に関する情報を保護するためにラッチをかける

		Os::AutoCriticalSection	latch(_File::_latch);

		// 参照回数を 1 減らす

		if (!((file->_refCount && --file->_refCount) ||
			  (reserve && file->isMounted() && !file->isReadOnly() &&
			   Checkpoint::Database::isAvailable(
				   static_cast<Schema::ObjectID::Value>(
					   file->getLockName().getDatabasePart()))))) {

			// どこからも参照されなくなり、
			// 破棄せずにとっておくように指示されていないか、
			// マウントされていないか、読み取り専用か、
			// 利用不可のデータベースの論理ログファイルの情報である

			// 格納するハッシュ表のバケットを求め、
			// ある論理ログファイルに関する情報を表すクラスを取り除く
			//
			//【注意】	バケットは _File::_latch で保護される

			; _SYDNEY_ASSERT(_File::_fileTable);
			_HashTable::Bucket& bucket =
				_File::_fileTable->getBucket(file->_hashAddr);
			bucket.erase(*file);

			// ある論理ログファイルに関する情報を表すクラスを破棄する

			delete file;
		}

		// 与えられたポインタは 0 を指すようにする

		file = 0;
	}
}

//	FUNCTION public
//	Trans::Log::File::detach -- 論理ログファイルを表すクラスの参照数を 1 減らす
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
	// ある論理ログファイルの情報を表すクラスの
	// 生成・破棄に関する情報を保護するためにラッチをかける

	Os::AutoCriticalSection	latch(_File::_latch);
#ifdef DEBUG
	; _SYDNEY_ASSERT(_refCount);
#else
	if (_refCount)
#endif
		// 参照数が 0 より大きければ、1 減らす

		--_refCount;
}

//	FUNCTION public
//	Trans::Log::File::create -- 論理ログファイルを生成する
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
File::create()
{
	// 最後に論理ログを挿入したときとフラッシュしたときの
	// タイムスタンプは初期化されているはず

	; _SYDNEY_ASSERT(_lastModification.isIllegal());
	; _SYDNEY_ASSERT(_lastFlush.isIllegal());

	// 論理ログファイルを実際に生成する

	LogicalLogFile::create();

	try {
		if (getLockName().getDatabasePart() != Schema::ObjectID::SystemTable)

			// システム専用でない論理ログファイルを生成するときは、
			// 前々回のチェックポイント終了時の
			// タイムスタンプを記録する専用のファイルも生成する

			TimeStamp::createFile(_TimeStamp::getPath(getPath()));

	} catch (...) {

		LogicalLogFile::destroy();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Trans::Log::File::mount -- 論理ログファイルをマウントする
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
File::mount()
{
	// 論理ログファイルを実際にマウントする

	LogicalLogFile::mount();

	try {
		if (getLockName().getDatabasePart() != Schema::ObjectID::SystemTable)

			// システム専用でない論理ログファイルをマウントするときは、
			// 前々回のチェックポイント終了時の
			// タイムスタンプを記録する専用のファイルもマウントする

			TimeStamp::mountFile(_TimeStamp::getPath(getPath()), isReadOnly());

	} catch (...) {

		LogicalLogFile::unmount();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Trans::Log::File::destroy -- 論理ログファイルを破棄する
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
File::destroy()
{
	// 論理ログファイルを実際に破棄する

	LogicalLogFile::destroy();

	if (getLockName().getDatabasePart() != Schema::ObjectID::SystemTable)

		// システム専用でない論理ログファイルを破棄するときは、
		// 前々回のチェックポイント終了時の
		// タイムスタンプを記録する専用のファイルも破棄する

		TimeStamp::destroyFile(_TimeStamp::getPath(getPath()));

	// 最後に論理ログを挿入したときとフラッシュしたときの
	// タイムスタンプを初期化する

	_lastModification = _lastFlush = IllegalTimeStamp;
}

//	FUNCTION public
//	Trans::Log::File::unmount -- 論理ログファイルをアンマウントする
//
//	NOTES
//		マウントされていない
//		バージョンファイルをアンマウントしてもエラーにならない
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
	// 論理ログファイルを実際にアンマウントする

	LogicalLogFile::unmount();

	if (getLockName().getDatabasePart() != Schema::ObjectID::SystemTable)

		// システム専用でない論理ログファイルをアンマウントするときは、
		// 前々回のチェックポイント終了時の
		// タイムスタンプを記録する専用のファイルもアンマウントする

		TimeStamp::unmountFile(_TimeStamp::getPath(getPath()), isReadOnly());

	// 最後に論理ログを挿入したときとフラッシュしたときの
	// タイムスタンプを初期化する

	_lastModification = _lastFlush = IllegalTimeStamp;
}

//	FUNCTION public
//	Trans::Log::File::truncate -- 論理ログファイルをトランケートする
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
File::truncate()
{
	if (getLockName().getDatabasePart() != Schema::ObjectID::SystemTable)

		// システム専用でない論理ログファイルをトランケートするときは、
		// 前回のチェックポイント終了時のタイムスタンプを
		// 専用のファイルへ記録する
		//
		//【注意】	論理ログファイルをトランケートすると、論理ログが失われる
		//
		//			システムデータベース用の論理ログファイルがない状態で、
		//			データベースをリカバリし、マウント可能にするために、
		//			この専用のファイルからタイムスタンプを取り出し、使用する

		Checkpoint::TimeStamp::getMostRecent(
			getLockName()).store(_TimeStamp::getPath(getPath()));

	//【注意】	以前はここで discardLog(true) を実行していたが、
	//			レプリケーションのスレーブデータベースが自動リカバリされ、
	//			その後実行される terminate 処理中にそれをしてしまうと、
	//			必要なログも削除されてしまうので、実行するのをやめた

	// 論理ログファイルを実際にトランケートする

	LogicalLogFile::truncate();

	// 最後に論理ログを挿入したときとフラッシュしたときの
	// タイムスタンプを初期化する

	_lastModification = _lastFlush = IllegalTimeStamp;
}

//	FUNCTION public
//	Trans::Log::File::rename --
//		論理ログファイルの実体である OS ファイルの名前を変更する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			論理ログファイルの実体である OS ファイルの新しい名前
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::rename(const Os::Path& path)
{
	// ある論理ログファイルの情報を表すクラスの生成・破棄に関する情報を
	// 保護するためにラッチをかける

	Os::AutoCriticalSection	latch(_File::_latch);

	; _SYDNEY_ASSERT(_File::_fileTable);

	// 改名後の論理ログファイルの情報を格納すべきハッシュ表のバケットを求める

	const unsigned int addr =
		_File::fileTableHash(path) % _File::_fileTable->getLength();
	_HashTable::Bucket& dst = _File::_fileTable->getBucket(addr);

	// 改名後の論理ログファイルの情報を表すクラスが
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

	// 論理ログファイルの実体である OS ファイルの名前を実際に変更する

	const Os::Path oldPath(getPath());
	LogicalLogFile::rename(path);

	try {
		if (getLockName().getDatabasePart() != Schema::ObjectID::SystemTable)

			// システム専用でない論理ログファイルの名前を変更するときは、
			// 前々回のチェックポイント終了時の
			// タイムスタンプを記録する専用のファイルの名前も変更する

			TimeStamp::renameFile(
				_TimeStamp::getPath(oldPath), _TimeStamp::getPath(path));

	} catch (...) {

		LogicalLogFile::rename(oldPath);
		_SYDNEY_RETHROW;
	}

	// 自分自身を先ほど求めたバケットへ移動する
	//
	//【注意】	新しいバケットのアドレスは
	//			論理ログファイルに関する情報を表すクラスに覚えておく

	_HashTable::Bucket& src = _File::_fileTable->getBucket(_hashAddr);

	dst.splice(dst.begin(), src, _HashTable::Bucket::Iterator(src, this));
	_hashAddr = addr;
}

//	FUNCTION public
//	Trans::Log::File::rotate -- 論理ログファイルをローテートする
//
//	NOTES
//		チェックポイントが発生するだびに実行される
//		ローテートするのは、データベースの論理ログのみ
//
//	ARGUMENTS
//		bool persisted
//			true
//				バッファにはダーティな内容は存在せず、
//				バッファとディスクは完全に一致している
//			false
//				バッファにダーティな内容が存在する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::rotate(bool persisted)
{
	if (getLockName().getDatabasePart() != Schema::ObjectID::SystemTable) {
		
		// システム専用でない論理ログファイルをローテートするときは、
		// 前回または前々回のチェックポイント終了時のタイムスタンプを
		// 専用のファイルへ記録する
		//
		//【注意】	前回または前々回のチェックポイント時から
		//			ロールフォワードリカバリを行うために必要な処理である

		if (persisted)

			Checkpoint::TimeStamp::getMostRecent(
				getLockName()).store(_TimeStamp::getPath(getPath()));

		else
			
			Checkpoint::TimeStamp::getSecondMostRecent(
				getLockName()).store(_TimeStamp::getPath(getPath()));
	}

	if (!isRecoveryFull())
		
		// 論理ログが破棄できるのなら、可能な限り破棄する

		discardLog(false);

	// 論理ログファイルを実際にローテートする

	LogicalLogFile::rotate();
}

//	FUNCTION public
//	Trans::Log::File::load -- 論理ログを取り出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::LSN		lsn
//			取り出す論理ログのログシーケンス番号
//
//	RETURN
//		0 以外の値
//			取り出された論理ログのデータを記憶する領域の先頭アドレス
//		0
//			論理ログを取り出すべき論理ログファイルがないので、
//			論理ログを取り出せない
//
//	EXCEPTIONS

Data*
File::load(LSN lsn)
{
	// 指定されたログシーケンス番号の論理ログを
	// 論理ログファイルから取り出す

	ModAutoPointer<const LogicalLog::Log> log(read(lsn));

	if (log.get()) {
		if (Configuration::CompressLogicalLog::get()) {

			// 読み出した論理ログを解凍する

			_Data::_Uncompressed uncompressed(*log, log->getSize());

			// 解凍された論理ログを復号化し、
			// 論理ログに保持されているデータを取り出す

			ModMemory memory(uncompressed._p, uncompressed._size);
			return dynamic_cast<Data*>(
				Common::InputArchive(memory).readObject());
		} else {

			// 読み出した論理ログを復号化し、
			// 論理ログに保持されているデータを取り出す

			ModMemory memory(*log, log->getSize());
			return dynamic_cast<Data*>(
				Common::InputArchive(memory).readObject());
		}
	}

	return 0;
}

//	FUNCTION public
//	Trans::Log::File::store -- 論理ログを記録する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::Data&	data
//			論理ログとして記録するデータ
//		Trans::Log::LSN		masterLSN
//			マスター側のLSN
//
//	RETURN
//		Trans::Log::IllegalLSN 以外の値
//			記録された論理ログのログシーケンス番号
//		Trans::Log::IllegalLSN
//			必要なかったので、論理ログを記録しなかった
//
//	EXCEPTIONS

LSN
File::store(const Data& data, LSN masterLSN)
{
	LSN	lsn = IllegalLSN;

	if (!Configuration::NoLogicalLog::get()) {
		if (data._timeStamp.isIllegal()) {

			// 論理ログを記録したときのタイムスタンプとして、
			// 新しいタイムスタンプ値を割り当てる
			//
			//【注意】	他の論理ログファイルにこの論理ログを記録したときに
			//			タイムスタンプが求められているときは、そのまま使う

			data._timeStamp = TimeStamp::assign();
			; _SYDNEY_ASSERT(!data._timeStamp.isIllegal());
		}

		// 指定されたデータを符号化した結果を
		// 格納するためのメモリ領域を確保する
		//
		//【注意】	data.sizeOfSerialized では正しいサイズが求められない

		ModSerialSize serialSize;
		Common::OutputArchive tmp(serialSize);
		tmp.writeObject(&data);
		const ModSize size = tmp.getSize();

		void* p = Os::Memory::allocate(size);

		try {
			// そのデータを確保した領域に符号化する

			ModMemory memory(p, size);
			Common::OutputArchive(memory).writeObject(&data);

			if (Configuration::CompressLogicalLog::get()) {

				// 符号化されたデータを圧縮する

				_Data::_Compressed compressed(p, size);

				// 圧縮されたデータを論理ログファイルの末尾に記録する

				LogicalLog::Log log(compressed._size, compressed._p);
				lsn = append(log, masterLSN);
			} else {

				// 符号化されたデータを論理ログファイルの末尾に記録する

				LogicalLog::Log log(size, p);
				lsn = append(log, masterLSN);
			}
		} catch (...) {

			Os::Memory::free(p);
			_SYDNEY_RETHROW;
		}

		// レプリケーション用のキューに書き込む
		// 領域が不要になったら、関数内部でメモリが解放される

		pushSlaveQueue(data.getCategory(), p, size, lsn);

		// 記録したときのタイムスタンプを記憶しておく

		_lastModification = data.getTimeStamp();

		switch (data.getCategory()) {
		case Data::Category::FileSynchronizeEnd:

			// 最後に記録したデータベースの更新を表す論理ログは
			// バージョンファイルの同期したことを表す論理ログになった

			_synchronized = Boolean::True;
			break;

		case Data::Category::SchemaModify:
			{
				const Schema::LogData& tmp =
					_SYDNEY_DYNAMIC_CAST(const Schema::LogData&, data);
				
				if (tmp.getSubCategory()
					== Schema::LogData::Category::StartBackup
					|| tmp.getSubCategory()
					== Schema::LogData::Category::EndBackup)
				{
					// バックアップではファイルの更新はしない
					break;
				}
			}
		
		case Data::Category::TupleModify:
		case Data::Category::DriverModify:
		case Data::Category::StatementRollback:
		case Data::Category::TransactionRollback:

			// 最後に記録したデータベースの更新を表す論理ログは
			// バージョンファイルの同期したことを表す論理ログでなくなった

			_synchronized = Boolean::False;
			break;

		case Data::Category::CheckpointDatabase:
			{
			// チェックポイントが発生したので、そのLSNを記憶する

			// 前回のLSNを前々回のLSNに移動する
			
			_secondLSN = _firstLSN;
			_firstLSN = lsn;

			const Checkpoint::Log::CheckpointDatabaseData& tmp
				= _SYDNEY_DYNAMIC_CAST(
					const Checkpoint::Log::CheckpointDatabaseData&, data);

			// このチェックポイント実行時に実行中のトランザクションのうち、
			// もっとも最初に起動されたものを得る

			LSN beginLSN = tmp.getBeginTransactionLSN();

			if (beginLSN != Log::IllegalLSN)

				// トランザクションの開始時点のログシーケンス番号を記憶する

				_firstLSN = beginLSN;

			}
			break;

		}

		switch (data.getCategory()) {
		case Data::Category::SchemaModify:
			{
				const Schema::LogData& tmp =
					_SYDNEY_DYNAMIC_CAST(const Schema::LogData&, data);
				
				if (tmp.getSubCategory()
					== Schema::LogData::Category::StartBackup
					|| tmp.getSubCategory()
					== Schema::LogData::Category::EndBackup)
				{
					// バックアップではファイルの更新はしない
					break;
				}
			}
			
		case Data::Category::TupleModify:
		case Data::Category::DriverModify:
		case Data::Category::StatementRollback:

			if (!setSyncDone(false) && !_lastFlush.isIllegal() &&
				_lastFlush >
				Checkpoint::TimeStamp::getMostRecent(getLockName()))
				break;

			// 直前のチェックポイント処理以降に
			// 更新操作を表す論理ログがフラッシュされてないとき、
			// フラッシュする
			//
			//【注意】	起動時障害回復では、
			//			あるデータベースを障害回復処理する必要があるかを、
			//			障害回復の起点のなるチェックポイント処理を表す
			//			論理ログより後に、更新操作を表す論理ログが
			//			記録されているかで判断する
			//
			//			しかし、ある操作を表す論理ログを書き出す前に
			//			その操作によって更新された内容が
			//			先に書き出される可能性があるため、
			//			ここでフラッシュしておかないと正しく判定できない

		case Data::Category::TransactionRollback:

			//【注意】	起動時障害回復で
			//			『コミット準備済』のトランザクションブランチが
			//			そのままの状態で回復されずにヒューリスティックに
			//			解決されるため、ここでフラッシュしておかないと、
			//			ロールバックしたトランザクションブランチが
			//			起動時障害回復でヒューリスティックに
			//			解決されてしまう可能性がある

		case Data::Category::TimeStampAssign:

			// 更新操作を表す論理ログをフラッシュしたときの
			// タイムスタンプを記憶しておく

			_lastFlush = _lastModification;

		case Data::Category::FileSynchronizeBegin:

			//【注意】	バージョンファイルを同期しても
			//			データベースを更新するとは限らないので、
			//			フラッシュしたときのタイムスタンプは記憶しない

		case Data::Category::TransactionPrepare:
		case Data::Category::TransactionCommit:
		case Data::Category::CheckpointDatabase:
		case Data::Category::CheckpointSystem:
		case Data::Category::BranchHeurDecide:
		case Data::Category::BranchForget:
		case Data::Category::XATransaction:
		case Data::Category::ReplicationEnd:

			// これまで記録した論理ログをすべてディスクへ書き出す

			flush();
		}
	}

	return lsn;
}

//	FUNCTION public
//	Trans::Log::File::getTimeStamp --
//		タイムスタンプファイルに書かれているタイムスタンプ値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたタイムスタンプ
//
//	EXCEPTIONS

TimeStamp
File::getTimeStamp() const
{
	if (getLockName().getDatabasePart() == Schema::ObjectID::SystemTable)

		// システム専用の論理ログファイルのタイムスタンプは、
		// システム全体のタイムスタンプ値が永続化されたものであり、
		// 個々のデータベースのファイルが永続化されている
		// タイムスタンプ値ではないので、不正なタイムスタンプ値を返す

		return IllegalTimeStamp;

	TimeStamp t;
	t.load(_TimeStamp::getPath(getPath()));

	return t;
}

//	FUNCTION public
//	Trans::Log::File::setTimeStamp --
//		タイムスタンプファイルにタイムスタンプ値を格納する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたタイムスタンプ
//
//	EXCEPTIONS

void
File::setTimeStamp(const TimeStamp& timestamp_)
{
	// システム専用の論理ログファイルのタイムスタンプは対象外
	; _SYDNEY_ASSERT(getLockName().getDatabasePart()
					 != Schema::ObjectID::SystemTable);

	// タイムスタンプファイルに書き込む
	timestamp_.store(_TimeStamp::getPath(getPath()));
}

//	FUNCTION public
//	Trans::Log::File::getInUseList --
//		現在、使用中の論理ログファイルに関する情報を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		論理ログファイルに関する情報を表すクラスを格納する領域の
//		先頭アドレスを要素として持つベクター
//
//	EXCEPTIONS

// static
ModVector<File*>
File::getInUseList()
{
	ModVector<File*> list;

	Os::AutoCriticalSection	latch(_File::_latch);

	if (_File::_fileTable)
		if (const _HashTable::Bucket::Size n = _File::_fileTable->getSize()) {
			list.reserve(n);

			unsigned int i = 0;
			do {
				_HashTable::Bucket& bucket = _File::_fileTable->getBucket(i);

				if (bucket.getSize()) {
					_HashTable::Bucket::Iterator		ite(bucket.begin());
					const _HashTable::Bucket::Iterator&	end = bucket.end();

					do {
						list.pushBack(&(*ite));
					} while (++ite != end) ;
				}
			} while (++i < _File::_fileTable->getLength()) ;
		}

	return list;
}

//	FUNCTION public
//	Trans::Log::File::isSynchronized -- 
//		最後に記録したデータベースの更新を表す論理ログは
//		バージョンファイルの同期を表す論理ログか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			そうである
//		false
//			そうでない
//
//	EXCEPTIONS
//		なし

bool
File::isSynchronized()
{
	if (_synchronized == Boolean::Unknown)
	{
		if (!Configuration::NoLogicalLog::get())
			_synchronized = isSyncDone() ? Boolean::True : Boolean::False;
		else
			_synchronized = Boolean::False;
	}
	return (_synchronized == Boolean::True);
}

//	FUNCTION public
//	Trans::Log::File::discardLog -- 不要な論理ログを削除する
//
//	NOTES
//
//	ARGUMENTS
//		bool
//			最新以外の論理ログをすべて削除する場合はtrue、
//			前々回のチェックポイント以前の論理ログを削除する場合はfalse
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::discardLog(bool isDiscardFull_)
{
	if (isDiscardFull_)

		// 最新の論理ログ以外のすべての論理ログを削除する

		discard();

	else if (_secondLSN != Log::IllegalLSN)

		// 前々回のチェックポイント以前の論理ログを削除する

		discard(_secondLSN);
}

//	FUNCTION public
//	Trans::Log::File::storeSecondMostRecent
//		-- 前々回のチェックポイント時のタイムスタンプを記録する
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
File::storeSecondMostRecent()
{
	if (getLockName().getDatabasePart() != Schema::ObjectID::SystemTable)

		// 前々回のチェックポイント終了時のタイムスタンプを
		// 専用のファイルへ記録する
		
		Checkpoint::TimeStamp::getSecondMostRecent(
			getLockName()).store(_TimeStamp::getPath(getPath()));
	
}

//
//	FUNCTION public static
//	Trans::Log::File::startReplication
//		-- スレーブとの接続スレッドを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cHostName_,
//		スレーブのホスト名
//	int iPortNumber_
//		スレーブのポート番号
//	Communication::Connection* pConnection_
//		コネクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::startReplication(const ModUnicodeString& cHostName_,
					   int iPortNumber_,
					   Communication::Connection* pConnection_)
{
	Os::AutoCriticalSection cAuto(_File::_latch2);

	_File::_MapKey key(cHostName_, iPortNumber_);
	_File::_ThreadMap::Iterator i = _File::_threadMap.find(key);
	if (i != _File::_threadMap.end())
	{
		// 存在した
			
		if ((*i).second->getStatus() == Common::Thread::Running)
		{
			// 活動中であるので、停止する
				
			(*i).second->abort();
		}
			
		// 終了するのと待って、削除

		(*i).second->join();
		delete (*i).second;
		_File::_threadMap.erase(i);
	}

	// 新たに作成する
		
	File::MasterThread* pThread
		= new File::MasterThread(pConnection_,
								 cHostName_, iPortNumber_);
	pThread->create();	// スレッド起動

	_File::_threadMap.insert(key, pThread);	// マップに挿入
}

//
//	FUNCTION public static
//	Trans::Log::File::setQueue
//		-- スレーブとのキューを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cHostName_,
//		スレーブのホスト名
//	int iPortNumber_
//		スレーブのポート番号
//
//	RETURN
//	Trans::Log::File::Queue*
//		キュー
//
//	EXCEPTIONS
//
File::Queue*
File::getQueue(const ModUnicodeString& cHostName_,
			   int iPortNumber_)
{
	Os::AutoCriticalSection cAuto(_File::_latch2);

	_File::_MapKey key(cHostName_, iPortNumber_);
	_File::_ThreadMap::Iterator i = _File::_threadMap.find(key);
	if (i == _File::_threadMap.end())
	{
		// 存在しない
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	return (*i).second->m_pQueue;
}

//
//	FUNCTION public
//	Trans::Log::File::setQueue
//		-- スレーブのキューを設定する
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Log::File::Queue*
//		キュー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::setQueue(const ModUnicodeString& cHostName_,
			   int iPortNumber_,
			   Queue* pQueue_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	// 破棄されないように参照回数を増やす
	attach();

	pQueue_->attach();

	QueueKey key(cHostName_, iPortNumber_);
	ModPair<QueueMap::Iterator, ModBoolean> r
		= m_cQueueMap.insert(key, pQueue_);
	if (r.second == ModFalse)
	{
		// すでに登録されているので、新しいものに置き換える
		Queue* old = (*r.first).second;
		if (old->detach() == 0)
			delete old;

		// 置き換える
		(*r.first).second = pQueue_;

		// 参照回数を減らす
		detach();
	}
}

//
//	FUNCTION public
//	Trans::Log::File::stopTransferLog
//		-- スレーブへの転送を停止する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cHostName_,
//		スレーブのホスト名
//	int iPortNumber_
//		スレーブのポート番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::stopTransferLog(const ModUnicodeString& cHostName_,
					  int iPortNumber_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	QueueKey key(cHostName_, iPortNumber_);
	QueueMap::Iterator i = m_cQueueMap.find(key);
	if (i != m_cQueueMap.end())
	{
		Queue* q = (*i).second;
		(void) m_cQueueMap.erase(i);

		if (q->detach() == 0)
			delete q;

		// 参照回数を減らす
		detach();
	}
}

//
//	FUNCTION private
//	Trans::Log::File::~File --
//		ある論理ログファイルに関する情報を表すクラスのデストラクター
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
//
File::~File()
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	QueueMap::Iterator i = m_cQueueMap.begin();
	for (;i != m_cQueueMap.end(); ++i)
	{
		Queue* q = (*i).second;
		if (q->detach() == 0)
			delete q;
	}
	m_cQueueMap.erase(m_cQueueMap.begin(), m_cQueueMap.end());
}

//
//	FUNCTION private
//	Trans::Log::File::pushSlaveQueue
//		-- 論理ログをスレーブ用のキューに書き込む
//
//	NOTES
//
//	ARGUMENTS
//	int category_
//		カテゴリ
//	void* p_
//		論理ログのメモリ領域
//	ModSize size_
//		論理ログのサイズ
//	Trans::Log::LSN
//		マスターのLSN
//
//	RETURN
//	なし
//
//	EXCPEIOTNS
//
void
File::pushSlaveQueue(int category_, void* p_, ModSize size_, LSN lsn_)
{
	{
		Os::AutoCriticalSection cAuto(m_cLatch);
	
		QueueMap::Iterator i = m_cQueueMap.begin();
		while (i != m_cQueueMap.end())
		{
			if ((*i).second->isActive())
			{
				// キューに書き込む
				(*i).second->pushBack(getLockName().getDatabasePart(),
									  category_, p_, size_, lsn_);

				++i;	// 次へ
			}
			else
			{
				// スレーブとのコネクションが切れている
				Queue* q = (*i).second;
				QueueMap::Iterator e = i;
				++i;	// 次へ
				
				m_cQueueMap.erase(e);

				if (q->detach() == 0)
					delete q;
			}
		}
	}

	Os::Memory::free(p_);
}

//
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2012, 2014, 2015, 2016, 2017, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
