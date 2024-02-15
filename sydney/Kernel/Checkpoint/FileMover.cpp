// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileMover.cpp -- ファイルの移動処理を行うクラス関連の関数定義
// 
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Checkpoint";
}

#ifdef OBSOLETE
#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Checkpoint/FileMover.h"
#include "Checkpoint/Manager.h"
#include "Checkpoint/TimeStamp.h"

#include "Common/Assert.h"
#include "Common/DoubleLinkedList.h"
#include "Common/Object.h"
#include "Common/ObjectPointer.h"
#include "Common/StringArrayData.h"
#include "LogicalFile/File.h"
#include "LogicalFile/FileDriver.h"
#include "Os/AutoCriticalSection.h"
#include "Os/Path.h"
#include "PhysicalFile/File.h"
#include "PhysicalFile/Manager.h"
#include "Trans/AutoTransaction.h"
#include "Trans/TimeStamp.h"

#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_CHECKPOINT_USING

namespace
{

//	CLASS
//	$$$::_EntryInfo --
//		あるトランザクションについて移動するオブジェクトの
//		登録に関する情報を表すクラス
//
//	NOTES

class _EntryInfo
	: public	Common::Object
{
public:
	//	CLASS
	//	$$$::_EntryInfo::_LogicalFile --
	//		移動する論理ファイルに関する情報を表すクラス
	//
	//	NOTES

	class _LogicalFile
	{
	public:
		// デフォルトコンストラクター
		_LogicalFile();
		// コンストラクター
		_LogicalFile(const LogicalFile::FileDriver& driver,
					 LogicalFile::File* file,
					 const Common::ObjectPointer<
						Common::StringArrayData>& areas);
		// デストラクター
		~_LogicalFile();

		// 移動する
		bool
		move(const Trans::Transaction& trans,
			 const Trans::TimeStamp& t, bool force);

		// 登録時点での直前のチェックポイント処理の終了時タイムスタンプ
		Trans::TimeStamp		_t;
		// 論理ファイルドライバー
		const LogicalFile::FileDriver*	_driver;
		// 論理ファイル記述子
		LogicalFile::File*		_file;
		// 移動先のエリアの絶対パス名を要素とする配列へのポインタ
		Common::ObjectPointer<Common::StringArrayData>	_areas;
	};

	//	CLASS
	//	$$$::_EntryInfo::_PhysicalFile --
	//		移動する物理ファイルに関する情報を表すクラス
	//
	//	NOTES

	class _PhysicalFile
	{
	public:
		// デフォルトコンストラクター
		_PhysicalFile();
		// コンストラクター
		_PhysicalFile(PhysicalFile::File* file,
					  const Version::File::StorageStrategy::Path& path);
		// デストラクター
		~_PhysicalFile();

		// 移動する
		bool
		move(const Trans::Transaction& trans,
			 const Trans::TimeStamp& t, bool force);

		// 登録時点での直前のチェックポイント処理の終了時タイムスタンプ
		Trans::TimeStamp		_t;
		// 物理ファイル記述子
		PhysicalFile::File*		_file;
		// 移動先の OS ディレクトリの絶対パス名
		Version::File::StorageStrategy::Path	_path;
	};

	//	CLASS
	//	$$$::_EntryInfo::_LogicalLog --
	//		改名する論理ログファイルに関する情報を表すクラス
	//
	//	NOTES

	class _LogicalLog
	{
	public:
		// デフォルトコンストラクター
		_LogicalLog();
		// コンストラクター
		_LogicalLog(Trans::Log::File* file,	const Os::Path& path);
		// デストラクター
		~_LogicalLog();

		// 改名する
		bool
		rename(const Trans::TimeStamp& t, bool force);

		// 登録時点での直前のチェックポイント処理の終了時タイムスタンプ
		Trans::TimeStamp		_t;
		// 論理ログファイルに関する情報を記憶するクラス
		Trans::Log::File*		_file;
		// 改名後の論理ログファイルの実体である OS ファイルの絶対パス名
		Os::Path				_path;
	};

	//	CLASS
	//	$$$::_EntryInfo::_Directory --
	//		改名する OS ディレクトリに関する情報を表すクラス
	//
	//	NOTES

	class _Directory
	{
	public:
		// デフォルトコンストラクター
		_Directory();
		// コンストラクター
		_Directory(const Os::Path& srcPath, const Os::Path& dstPath);
		// デストラクター
		~_Directory();

		// 改名する
		bool
		rename(const Trans::TimeStamp& t, bool force);

		// 登録時点での直前のチェックポイント処理の終了時タイムスタンプ
		Trans::TimeStamp		_t;
		// 改名前の OS ディレクトリの絶対パス名
		Os::Path				_srcPath;
		// 改名後の OS ディレクトリの絶対パス名
		Os::Path				_dstPath;
	};

	// コンストラクター
	_EntryInfo(const Trans::Transaction::ID& id);
	// デストラクター
	~_EntryInfo();

	// クラスを生成する
	static _EntryInfo*
	attach(const Trans::Transaction::ID& id);
	// クラスを破棄する
	static void
	detach(_EntryInfo*& info);

	// 移動対象の登録を行ったトランザクションのトランザクション識別子
	Trans::Transaction::ID	_id;

	// 登録済の移動する論理ファイルを管理するリスト
	ModVector<_LogicalFile>		_logicalFile;
	// 登録済の移動する物理ファイルを管理するリスト
	ModVector<_PhysicalFile>	_physicalFile;
	// 登録済の改名する論理ログファイルを管理するリスト
	ModVector<_LogicalLog>		_logicalLog;
	// 登録済の改名するディレクトリを管理するリスト
	ModVector<_Directory>		_directory;

	// 登録情報リストでの直前の要素へのポインタ
	_EntryInfo*				_prev;
	// 登録情報リストでの直後の要素へのポインタ
	_EntryInfo*				_next;
};

//	TYPEDEF
//	$$$::_EntryInfoList --
//		あるトランザクションについて移動するオブジェクトの
//		登録に関する情報を表すクラスを管理するためのリスト
//
//	NOTES

typedef	Common::DoubleLinkedList<_EntryInfo>		_EntryInfoList;

namespace _FileMover
{
	// 以下の情報を保護するためのラッチ
	Os::CriticalSection		_latch;
	// 登録情報を管理するリスト
	_EntryInfoList*			_infoList = 0;
}

//	FUNCTION public
//	$$$::_EntryInfo::_EntryInfo --
//		あるトランザクションについて移動するオブジェクトの
//		登録に関する情報を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction::ID&	id
//			このトランザクション識別子の表すトランザクションについて
//			移動するオブジェクトの登録に関する情報を表すクラスを生成する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
_EntryInfo::_EntryInfo(const Trans::Transaction::ID& id)
	: _id(id),
	  _prev(0),
	  _next(0)
{}

//	FUNCTION public
//	$$$::_EntryInfo::~_EntryInfo --
//		あるトランザクションについて移動するオブジェクトの
//		登録に関する情報を表すクラスのデストラクター
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
_EntryInfo::~_EntryInfo()
{}

//	FUNCTION public
//	$$$::_EntryInfo::attach --
//		あるトランザクションについて移動するオブジェクトの
//		登録に関する情報を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction::ID&	id
//			このトランザクション識別子の表すトランザクションについて
//			移動するオブジェクトの登録に関する情報を表すクラスを生成する
//
//	RETURN
//		クラスを格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
_EntryInfo*
_EntryInfo::attach(const Trans::Transaction::ID& id)
{
	//【注意】	登録情報を管理するリストを保護するためにラッチ済であること

	if (!_FileMover::_infoList) {

		// 登録情報を管理するリストが存在しないので、生成する

		_FileMover::_infoList =
			new _EntryInfoList(&_EntryInfo::_prev, &_EntryInfo::_next);
		; _SYDNEY_ASSERT(_FileMover::_infoList);
	}

	// 情報を登録しようとしているトランザクションについて、
	// すでに情報が登録されていないか調べる
	//
	//【注意】	リストはトランザクション識別子の降順にソートされている

	_EntryInfoList::Iterator		ite(_FileMover::_infoList->begin());
	const _EntryInfoList::Iterator&	end = _FileMover::_infoList->end();

	for (; ite != end && (*ite)._id >= id; ++ite)
		if (id == (*ite)._id)

			// 情報を登録しようとしている
			// トランザクションについての情報がすでに登録されている

			return &*ite;

	// 情報を登録しようとしているトランザクションについて、
	// 登録情報を表すクラスを生成する

	_EntryInfo*	info = new _EntryInfo(id);
	; _SYDNEY_ASSERT(info);

	// 生成した登録情報を表すクラスを
	// 登録情報を管理するリストに登録する

	(void) _FileMover::_infoList->insert(ite, *info);

	return info;
}

//	FUNCTION public
//	$$$::_EntryInfo::detach --
//		あるトランザクションについて移動するオブジェクトの
//		登録に関する情報を表すクラスを破棄する
//
//	NOTES
//
//	ARGUMENTS
//		$$$::_EntryInfo*&	info
//			このトランザクション識別子の表すトランザクションについて
//			移動するオブジェクトの登録に関する情報を表すクラスを
//			格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
_EntryInfo::detach(_EntryInfo*& info)
{
	//【注意】	登録情報を管理するリストを保護するためにラッチ済であること

	if (info) {

		// 登録情報を管理するリストから指定された登録情報をはずす

		; _SYDNEY_ASSERT(_FileMover::_infoList);
		_FileMover::_infoList->erase(*info);

		// 指定された登録情報を破棄する

		delete info, info = 0;
	}
}

//	FUNCTION public
//	$$$::_EntryInfo::_LogicalFile::_LogicalFile --
//		移動する論理ファイルに関する情報を表すクラスの
//		デフォルトコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
_EntryInfo::_LogicalFile::_LogicalFile()
	: _driver(0),
	  _file(0)
{}

//	FUNCTION public
//	$$$::_EntryInfo::_LogicalFile::_LogicalFile --
//		移動する論理ファイルに関する情報を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		LogicalFile::FileDriver&	driver
//			移動する論理ファイルの論理ファイルドライバー記述子
//		LogicalFile::File*	file
//			移動する論理ファイルの論理ファイル記述子
//		Common::ObjectPointer<Common::StringArrayData>&	areas
//			移動先のエリアの絶対パス名を要素とする配列へのポインタ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
_EntryInfo::_LogicalFile::_LogicalFile(
	const LogicalFile::FileDriver& driver, LogicalFile::File* file,
	const Common::ObjectPointer<Common::StringArrayData>& areas)
	: _t(TimeStamp::getMostRecent()),
	  _driver(&driver),
	  _file(file),
	  _areas(areas)
{}

//	FUNCTION public
//	$$$::_EntryInfo::_LogicalFile::~_LogicalFile --
//		移動する論理ファイルに関する情報を表すクラスのデストラクター
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
_EntryInfo::_LogicalFile::~_LogicalFile()
{}

//	FUNCTION public
//	$$$::_EntryInfo::_LogicalFile::move --
//		論理ファイルを移動できれば、実際に移動する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			論理ファイルを移動するトランザクションのトランザクション記述子
//		Trans::TimeStamp&	t
//			このタイムスタンプより前に情報が登録されたもののみ移動する
//		bool			force
//			true
//				無条件に移動する
//			false
//				指定されたタイムスタンプより前に
//				情報が登録されたもののみ、移動する
//
//	RETURN
//		true
//			移動した
//		false
//			移動しなかった
//
//	EXCEPTIONS

inline
bool
_EntryInfo::_LogicalFile::move(
	const Trans::Transaction& trans, const Trans::TimeStamp& t, bool force)
{
	if (_file && (force || _t < t)) {

		// 論理ファイルを移動する

		_file->move(trans, *_areas);

		return true;
	}

	return false;
}

//	FUNCTION public
//	$$$::_EntryInfo::_PhysicalFile::_PhysicalFile --
//		移動する物理ファイルに関する情報を表すクラスの
//		デフォルトコンストラクター
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
_EntryInfo::_PhysicalFile::_PhysicalFile()
	: _file(0)
{}

//	FUNCTION public
//	$$$::_EntryInfo::_PhysicalFile::_PhysicalFile --
//		移動する物理ファイルに関する情報を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		PhysicalFile::File*	file
//			移動する物理ファイルの物理ファイル記述子
//		const Version::File::StorageStrategy::Path&	path
//			移動先の OS ディレクトリの絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
_EntryInfo::_PhysicalFile::_PhysicalFile(
	PhysicalFile::File* file,
	const Version::File::StorageStrategy::Path& path)
	: _t(TimeStamp::getMostRecent()),
	  _file(file),
	  _path(path)
{}

//	FUNCTION public
//	$$$::_EntryInfo::_PhysicalFile::~_PhysicalFile --
//		移動する物理ファイルに関する情報を表すクラスのデストラクター
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
_EntryInfo::_PhysicalFile::~_PhysicalFile()
{}

//	FUNCTION public
//	$$$::_EntryInfo::_PhysicalFile::move --
//		物理ファイルを移動できれば、実際に移動する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			物理ファイルを移動するトランザクションのトランザクション記述子
//		Trans::TimeStamp&	t
//			このタイムスタンプより前に情報が登録されたもののみ移動する
//		bool			force
//			true
//				無条件に移動する
//			false
//				指定されたタイムスタンプより前に
//				情報が登録されたもののみ、移動する
//
//	RETURN
//		true
//			移動した
//		false
//			移動しなかった
//
//	EXCEPTIONS

inline
bool
_EntryInfo::_PhysicalFile::move(
	const Trans::Transaction& trans, const Trans::TimeStamp& t, bool force)
{
	if (_file && (force || _t < t)) {

		// 物理ファイルを移動する

		_file->move(trans, _path);

		return true;
	}

	return false;
}

//	FUNCTION public
//	$$$::_EntryInfo::_LogicalLog::_LogicalLog --
//		改名する論理ログファイルに関する情報を表すクラスの
//		デフォルトコンストラクター
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
_EntryInfo::_LogicalLog::_LogicalLog()
	: _file(0)
{}

//	FUNCTION public
//	$$$::_EntryInfo::_LogicalLog::_LogicalLog --
//		改名する論理ログファイルに関する情報を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::File*	file
//			改名する論理ログファイルに関する情報を記憶するクラス
//		Os::Path&			path
//			改名後の論理ログファイルの実体である OS ファイルの絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
_EntryInfo::_LogicalLog::_LogicalLog(
	Trans::Log::File* file, const Os::Path& path)
	: _t(TimeStamp::getMostRecent()),
	  _file(file),
	  _path(path)
{}

//	FUNCTION public
//	$$$::_EntryInfo::_LogicalLog::~_LogicalLog --
//		改名する論理ログファイルに関する情報を表すクラスのデストラクター
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
_EntryInfo::_LogicalLog::~_LogicalLog()
{}

//	FUNCTION public
//	$$$::_EntryInfo::_LogicalLog::rename --
//		論理ログファイルを改名できれば、実際に改名する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp&	t
//			このタイムスタンプより前に情報が登録されたもののみ改名する
//		bool			force
//			true
//				無条件に改名する
//			false
//				指定されたタイムスタンプより前に
//				情報が登録されたもののみ、改名する
//
//	RETURN
//		true
//			改名した
//		false
//			改名しなかった
//
//	EXCEPTIONS

inline
bool
_EntryInfo::_LogicalLog::rename(const Trans::TimeStamp& t, bool force)
{
	if (_file && (force || _t < t)) {

		// 論理ログファイルを改名する

		_file->rename(_path);

		return true;
	}

	return false;
}

//	FUNCTION public
//	$$$::_EntryInfo::_Directory::_Directory --
//		改名する OS ディレクトリに関する情報を表すクラスの
//		デフォルトコンストラクター
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
_EntryInfo::_Directory::_Directory()
{}

//	FUNCTION public
//	$$$::_EntryInfo::_Directory::_Directory --
//		改名する OS ディレクトリに関する情報を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			srcPath
//			改名前の OS ディレクトリの絶対パス名
//		Os::Path&			dstPath
//			改名後の OS ディレクトリの絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
_EntryInfo::_Directory::_Directory(
	const Os::Path& srcPath, const Os::Path& dstPath)
	: _t(TimeStamp::getMostRecent()),
	  _srcPath(srcPath),
	  _dstPath(dstPath)
{}

//	FUNCTION public
//	$$$::_EntryInfo::_Directory::~_Directory --
//		改名する OS ディレクトリに関する情報を表すクラスのデストラクター
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
_EntryInfo::_Directory::~_Directory()
{}

//	FUNCTION public
//	$$$::_EntryInfo::_Directory::rename --
//		OS ディレクトリを改名できれば、実際に改名する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp&	t
//			このタイムスタンプより前に情報が登録されたもののみ改名する
//		bool			force
//			true
//				無条件に改名する
//			false
//				指定されたタイムスタンプより前に
//				情報が登録されたもののみ、改名する
//
//	RETURN
//		true
//			改名した
//		false
//			改名しなかった
//
//	EXCEPTIONS

bool
_EntryInfo::_Directory::rename(const Trans::TimeStamp& t, bool force)
{
	if (force || _t < t) {

		// OS ディレクトリを改名する

		Os::Directory::rename(_srcPath, _dstPath);

		return true;
	}

	return false;
}

}

//	FUNCTION private
//	Checkpoint::Manager::FileMover::initialize --
//		マネージャーの初期化のうち、
//		チェックポイント処理のファイル移動関連の初期化を行う
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

void
Manager::FileMover::initialize()
{}

//	FUNCTION private
//	Checkpoint::Manager::FileMover::terminate --
//		マネージャーの後処理のうち、
//		チェックポイント処理のファイル移動関連の後処理を行う
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
Manager::FileMover::terminate()
{
	if (_FileMover::_infoList) {

		// 登録情報を管理するリストは空である必要がある

		; _SYDNEY_ASSERT(_FileMover::_infoList->isEmpty());

		// 登録情報を管理するリストを破棄する

		delete _FileMover::_infoList, _FileMover::_infoList = 0;
	}
}

//	FUNCTION public
//	Checkpoint::FileMover::enter --
//		ある論理ファイルをチェックポイント処理時に実際に移動するように依頼する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			依頼するトランザクションのトランザクション記述子
//		LogicalFile::FileDriver&	driver
//			移動が依頼された論理ファイルの論理ファイルドライバー記述子
//		LogicalFile::File&	file
//			移動が依頼された論理ファイルの論理ファイル記述子
//		Common::ObjectPointer<Common::StringArrayData>&	areas
//			移動先のエリアの絶対パス名を要素とする配列へのポインタ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
FileMover::enter(const Trans::Transaction& trans,
				 const LogicalFile::FileDriver& driver,
				 const LogicalFile::File& file,
				 const Common::ObjectPointer<Common::StringArrayData>& areas)
{
	// 登録情報を管理するリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(_FileMover::_latch);

	// 情報を登録しようとしているトランザクションに関する
	// 登録情報を表すクラスを得る

	_EntryInfo*	info = _EntryInfo::attach(trans.getID());
	; _SYDNEY_ASSERT(info);

	// 得られた登録情報に指定された論理ファイルを加える

	info->_logicalFile.pushBack(
		_EntryInfo::_LogicalFile(
			driver,	driver.attachFile(&file), areas));
}

//	FUNCTION public
//	Checkpoint::FileMover::enter --
//		ある物理ファイルをチェックポイント処理時に実際に移動するように依頼する
//
//	NOTES
//
//	ARGUMENTS
//		const Trans::Transaction&	trans
//			依頼するトランザクションのトランザクション記述子
//		const PhysicalFile::File&	file
//			移動が依頼された物理ファイルの物理ファイル記述子
//		const Version::File::StorageStrategy::Path&	path
//			移動先の OS ディレクトリの絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
FileMover::enter(const Trans::Transaction& trans,
				 const PhysicalFile::File& file,
				 const Version::File::StorageStrategy::Path& path)
{
	// 登録情報を管理するリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(_FileMover::_latch);

	// 情報を登録しようとしているトランザクションに関する
	// 登録情報を表すクラスを得る

	_EntryInfo*	info = _EntryInfo::attach(trans.getID());
	; _SYDNEY_ASSERT(info);

	// 得られた登録情報に指定された物理ファイルを加える

	info->_physicalFile.pushBack(
		_EntryInfo::_PhysicalFile(
			PhysicalFile::Manager::attachFile(&file), path));
}

//	FUNCTION public
//	Checkpoint::FileMover::enter --
//		ある論理ログファイルをチェックポイント処理時に
//		実際に改名するように依頼する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			依頼するトランザクションのトランザクション記述子
//		Trans::Log::File&	file
//			改名が依頼された論理ログファイルに関する情報を記憶するクラス
//		Os::Path&			path
//			改名後の論理ログファイルの実体である OS ファイルの絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
FileMover::enter(const Trans::Transaction& trans,
				 const Trans::Log::File& file, const Os::Path& path)
{
	// 登録情報を管理するリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(_FileMover::_latch);

	// 情報を登録しようとしているトランザクションに関する
	// 登録情報を表すクラスを得る

	_EntryInfo*	info = _EntryInfo::attach(trans.getID());
	; _SYDNEY_ASSERT(info);

	// 得られた登録情報に指定された論理ログファイルを加える

	info->_logicalLog.pushBack(
		_EntryInfo::_LogicalLog(Trans::Log::File::attach(file), path));
}

//	FUNCTION public
//	Checkpoint::FileMover::enter --
//		ある OS ディレクトリをチェックポイント処理時に
//		実際に改名するように依頼する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			srcPath
//			改名前の OS ディレクトリの絶対パス名
//		Os::Path&			dstPath
//			改名後の OS ディレクトリの絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
FileMover::enter(const Os::Path& srcPath, const Os::Path& dstPath)
{
	// 登録情報を管理するリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(_FileMover::_latch);

	// トランザクションの指定のない登録情報を表すクラスを得る

	_EntryInfo*	info = _EntryInfo::attach(Trans::IllegalID);
	; _SYDNEY_ASSERT(info);

	// 得られた登録情報に指定された OS ディレクトリを加える

	info->_directory.pushBack(_EntryInfo::_Directory(srcPath, dstPath));
}

//	FUNCTION public
//	Checkpoint::FileMover::cancel --
//		ある論理ファイルをチェックポイント処理時に
//		実際に移動する依頼を取り止める
//		
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			取り止める依頼を行ったトランザクションのトランザクション記述子
//		LogicalFile::File&	file
//			移動する依頼を取り止める論理ファイルの論理ファイル記述子
//
//	RETURN
//		true
//			取り止めた
//		false
//			依頼されてなかった
//
//	EXCEPTIONS

// static
bool
FileMover::cancel(
	const Trans::Transaction& trans, const LogicalFile::File& file)
{
	// 登録情報を管理するリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(_FileMover::_latch);

	// 取り止めようとしている依頼を行ったトランザクションに関する
	// 登録情報を表すクラスを得る

	_EntryInfo*	info = _EntryInfo::attach(trans.getID());
	; _SYDNEY_ASSERT(info);

	if (info->_logicalFile.getSize()) {
		ModVector<_EntryInfo::_LogicalFile>::Iterator
			ite(info->_logicalFile.begin());
		const ModVector<_EntryInfo::_LogicalFile>::Iterator&
			end = info->_logicalFile.end();

		do {
			const _EntryInfo::_LogicalFile& object = *ite;

			if (object._file->equals(&file)) {

				// 移動が依頼されている
				// 論理ログファイルの論理ログファイル記述子を破棄する

				; _SYDNEY_ASSERT(object._driver);
				object._driver->detachFile(object._file);

				// 登録されている登録情報をはずす

				info->_logicalFile.erase(ite);

				return true;
			}
		} while (++ite != end) ;
	}

	// 見つからなかった

	return false;
}

//	FUNCTION public
//	Checkpoint::FileMover::cancel --
//		ある物理ファイルをチェックポイント処理時に
//		実際に移動する依頼を取り止める
//		
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			取り止める依頼を行ったトランザクションのトランザクション記述子
//		PhysicalFile::File&	file
//			移動する依頼を取り止める物理ファイルの物理ファイル記述子
//
//	RETURN
//		true
//			取り止めた
//		false
//			依頼されてなかった
//
//	EXCEPTIONS

// static
bool
FileMover::cancel(
	const Trans::Transaction& trans, const PhysicalFile::File& file)

{
	// 登録情報を管理するリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(_FileMover::_latch);

	// 取り止めようとしている依頼を行ったトランザクションに関する
	// 登録情報を表すクラスを得る

	_EntryInfo*	info = _EntryInfo::attach(trans.getID());
	; _SYDNEY_ASSERT(info);

	if (info->_physicalFile.getSize()) {
		ModVector<_EntryInfo::_PhysicalFile>::Iterator
			ite(info->_physicalFile.begin());
		const ModVector<_EntryInfo::_PhysicalFile>::Iterator&
			end = info->_physicalFile.end();

		do {
			_EntryInfo::_PhysicalFile& object = *ite;

			if (object._file->equals(&file)) {

				// 移動が依頼されている
				// 物理ファイルの物理ファイル記述子を破棄する

				PhysicalFile::Manager::detachFile(object._file);

				// 登録されている登録情報をはずす

				info->_physicalFile.erase(ite);

				return true;
			}
		} while (++ite != end) ;
	}

	// 見つからなかった

	return false;
}

//	FUNCTION public
//	Checkpoint::FileMover::cancel --
//		ある論理ログファイルをチェックポイント処理時に
//		実際に改名する依頼を取り止める
//		
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			取り止める依頼を行ったトランザクションのトランザクション記述子
//		Trans::Log::File&	file
//			改名する依頼を取り止める
//			論理ログファイルに関する情報を記憶するクラス
//
//	RETURN
//		true
//			取り止めた
//		false
//			依頼されてなかった
//
//	EXCEPTIONS

// static
bool
FileMover::cancel(
	const Trans::Transaction& trans, const Trans::Log::File& file)
{
	// 登録情報を管理するリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(_FileMover::_latch);

	// 取り止めようとしている依頼を行ったトランザクションに関する
	// 登録情報を表すクラスを得る

	_EntryInfo*	info = _EntryInfo::attach(trans.getID());
	; _SYDNEY_ASSERT(info);

	if (info->_logicalLog.getSize()) {
		ModVector<_EntryInfo::_LogicalLog>::Iterator
			ite(info->_logicalLog.begin());
		const ModVector<_EntryInfo::_LogicalLog>::Iterator&
			end = info->_logicalLog.end();

		do {
			_EntryInfo::_LogicalLog& object = *ite;

			if (object._file == &file) {

				// 改名が依頼されている
				// 論理ログファイルの情報を表すクラスを破棄する

				Trans::Log::File::detach(object._file);

				// 登録されている登録情報をはずす

				info->_logicalLog.erase(ite);

				return true;
			}
		} while (++ite != end) ;
	}

	// 見つからなかった

	return false;
}

//	FUNCTION public
//	Checkpoint::FileMover::cancel --
//		ある OS ディレクトリをチェックポイント処理時に
//		実際に改名する依頼を取り止める
//		
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			srcPath
//			改名する依頼を取り止める OS ディレクトリの絶対パス名
//
//	RETURN
//		true
//			取り止めた
//		false
//			依頼されてなかった
//
//	EXCEPTIONS

// static
bool
FileMover::cancel(const Os::Path& srcPath)
{
	//【注意】	与えられたトランザクション記述子は使用しない

	// 登録情報を管理するリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(_FileMover::_latch);

	// トランザクションの指定のない登録情報を表すクラスを得る

	_EntryInfo*	info = _EntryInfo::attach(Trans::IllegalID);
	; _SYDNEY_ASSERT(info);

	if (info->_directory.getSize()) {
		ModVector<_EntryInfo::_Directory>::Iterator
			ite(info->_directory.begin());
		const ModVector<_EntryInfo::_Directory>::Iterator&
			end = info->_directory.end();

		do {
			const _EntryInfo::_Directory& object = *ite;

			if (!object._srcPath.compare(srcPath)) {

				// 登録されている登録情報をはずす

				info->_directory.erase(ite);

				return true;
			}
		} while (++ite != end) ;
	}

	// 見つからなかった

	return false;
}

//	FUNCTION private
//	Checkpoint::FileMover::execute --
//		移動が依頼されているオブジェクトを可能な限り実際に移動する
//
//	NOTES
//		チェックポイントスレッドから呼び出される
//
//	ARGUMENTS
//		bool				force
//			true
//				移動が依頼されているオブジェクトをすべて実際に移動する
//			false
//				直前のチェックポイント以前に移動が依頼されたもので、
//				それを参照中の参照トランザクションが存在しなければ、移動する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
FileMover::execute(bool force)
{
	// 登録情報を管理するリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(_FileMover::_latch);

	if (!_FileMover::_infoList || _FileMover::_infoList->isEmpty())

		// 登録情報を管理するリストが存在しない、
		// または空なので、なにもしない

		return;

	// 登録情報を管理するリストの
	// 先頭からひとつひとつ登録情報を調べていく
	//
	//【注意】	リストの先頭から調べることにより、
	//			OS ディレクトリを最初に処理することができる

	_EntryInfoList::Iterator		ite(_FileMover::_infoList->begin());
	const _EntryInfoList::Iterator&	end = _FileMover::_infoList->end();

	do {
		_EntryInfo* info = &*ite;

		// 登録情報を表すクラスが破棄されると、
		// その登録情報を指す反復子は不正になるので、先に次を求めておく

		++ite;

		//【注意】	現在実行中の版管理するトランザクションのうち、
		//			その開始時に、情報を登録した更新トランザクションが
		//			実行中だったものがあっても移動できる
		//
		//【注意】	現在実行中の更新トランザクションが
		//			登録したものを移動できる

		// 直前のチェックポイント処理の終了時タイムスタンプを求める

		Trans::TimeStamp	last(TimeStamp::getMostRecent());
		{
		// 論理ファイル、物理ファイルを移動するために、
		// 更新トランザクションを開始する
		//
		//【注意】	他から参照されることはまったくないので
		//			ロックしないトランザクションを開始する

		Trans::AutoTransaction	trans(Trans::Transaction::attach());
		trans->begin(Trans::Transaction::Mode(
						 Trans::Transaction::Category::ReadWrite,
						 Trans::Transaction::IsolationLevel::Serializable,
						 Boolean::False),
					 true, false);

		if (info->_logicalFile.getSize()) {
			ModVector<_EntryInfo::_LogicalFile>::Iterator
				ite(info->_logicalFile.end());
			const ModVector<_EntryInfo::_LogicalFile>::Iterator&
				begin = info->_logicalFile.begin();

			do {
				_EntryInfo::_LogicalFile& object = *--ite;

				// 論理ファイルの移動が指示されてから、
				// 初めてのチェックポイント処理でなければ、
				// 論理ファイルを実際に移動する
				//
				//【注意】	論理ファイルの移動が指示されてから、
				//			初めてのチェックポイント処理であれば、
				//			移動する論理ファイルに関する情報が
				//			メタデータベースにフラッシュされていない
				//			可能性があるので、移動できない

				if (object.move(*trans, last, force)) {

					// 論理ファイルを移動した

					// 移動した論理ファイルの論理ファイル記述子を破棄する

					; _SYDNEY_ASSERT(object._driver);
					object._driver->detachFile(object._file);

					// その移動の指示の記録を抹消する

					info->_logicalFile.erase(ite);
				}
			} while (ite != begin) ;
		}

		if (info->_physicalFile.getSize()) {
			ModVector<_EntryInfo::_PhysicalFile>::Iterator
				ite(info->_physicalFile.end());
			const ModVector<_EntryInfo::_PhysicalFile>::Iterator&
				begin = info->_physicalFile.begin();

			do {
				_EntryInfo::_PhysicalFile& object = *--ite;

				// 物理ファイルの移動が指示されてから、
				// 初めてのチェックポイント処理でなければ、
				// 物理ファイルを実際に移動する
				//
				//【注意】	物理ファイルの移動が指示されてから、
				//			初めてのチェックポイント処理であれば、
				//			移動する物理ファイルに関する情報が
				//			メタデータベースにフラッシュされていない
				//			可能性があるので、移動できない

				if (object.move(*trans, last, force)) {

					// 物理ファイルを移動した

					// 移動した物理ファイルの物理ファイル記述子を破棄する

					PhysicalFile::Manager::detachFile(object._file);

					// その移動の指示の記録を抹消する

					info->_physicalFile.erase(ite);
				}
			} while (ite != begin) ;
		}

		// 論理ファイル、物理ファイルを移動した
		// 更新トランザクションを終了する

		trans->commit();
		}
		if (info->_logicalLog.getSize()) {
			ModVector<_EntryInfo::_LogicalLog>::Iterator
				ite(info->_logicalLog.end());
			const ModVector<_EntryInfo::_LogicalLog>::Iterator&
				begin = info->_logicalLog.begin();

			do {
				_EntryInfo::_LogicalLog& object = *--ite;

				// 論理ログファイルの改名が指示されてから、
				// 初めてのチェックポイント処理でなければ、
				// 論理ログファイルを実際に改名する
				//
				//【注意】	論理ログファイルの改名が指示されてから、
				//			初めてのチェックポイント処理であれば、
				//			改名する論理ログファイルに関する情報が
				//			メタデータベースにフラッシュされていない
				//			可能性があるので、改名できない

				if (object.rename(last, force)) {

					// 論理ログファイルを改名した

					// 移動した論理ログファイルの
					// 論理ログファイルの情報を表すクラスを破棄する

					Trans::Log::File::detach(object._file);

					// その改名の指示の記録を抹消する

					info->_logicalLog.erase(ite);
				}
			} while (ite != begin) ;
		}

		if (info->_directory.getSize()) {
			ModVector<_EntryInfo::_Directory>::Iterator
				ite(info->_directory.end());
			const ModVector<_EntryInfo::_Directory>::Iterator&
				begin = info->_directory.begin();

			do {
				_EntryInfo::_Directory& object = *--ite;

				// OS ディレクトリの改名が指示されてから、
				// 初めてのチェックポイント処理でなければ、
				// OS ディレクトリを実際に改名する
				//
				//【注意】	OS ディレクトリの改名が指示されてから、
				//			初めてのチェックポイント処理であれば、
				//			改名する OS ディレクトリに関する情報が
				//			メタデータベースにフラッシュされていない
				//			可能性があるので、改名できない

				if (object.rename(last, force))

					// OS ディレクトリを改名したので、
					// その改名の指示の記録を抹消する

					info->_directory.erase(ite);

			} while (ite != begin) ;
		}

		if (info->_logicalFile.isEmpty() && info->_physicalFile.isEmpty() &&
			info->_logicalLog.isEmpty() && info->_directory.isEmpty())

			// 登録情報に移動対象がひとつも記録されていないので、
			// 登録情報を表すクラスを破棄する

			_EntryInfo::detach(info);

	} while (ite != end) ;
}
#endif

//
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
