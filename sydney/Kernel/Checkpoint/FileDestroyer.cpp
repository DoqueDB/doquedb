// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDestroyer.cpp -- ファイルの破棄処理を行うクラス関連の関数定義
// 
// Copyright (c) 2001, 2002, 2005, 2006, 2013, 2015, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Checkpoint/FileDestroyer.h"
#include "Checkpoint/Database.h"
#include "Checkpoint/Manager.h"
#include "Checkpoint/TimeStamp.h"

#include "Common/Assert.h"
#include "Common/DoubleLinkedList.h"
#include "Common/Object.h"
#include "Common/Thread.h"
#include "LogicalFile/File.h"
#include "LogicalFile/FileDriver.h"
#include "LogicalFile/FileID.h"
#include "Os/AutoCriticalSection.h"
#include "Os/File.h"
#include "Os/Path.h"
#include "PhysicalFile/File.h"
#include "PhysicalFile/Manager.h"
#include "Schema/ObjectID.h"
#include "Trans/AutoTransaction.h"
#include "Trans/List.h"
#include "Trans/TimeStamp.h"
#include "Trans/Transaction.h"
#include "Exception/Object.h"

#include "ModException.h"
#include "ModOs.h"
#include "ModOsDriver.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_CHECKPOINT_USING

namespace
{

//	CLASS
//	$$$::_EntryInfo --
//		あるトランザクションについて破棄するオブジェクトの
//		登録に関する情報を表すクラス
//
//	NOTES

class _EntryInfo
	: public	Common::Object
{
public:
	//	CLASS
	//	$$$::_EntryInfo::_LogicalFile --
	//		破棄する論理ファイルに関する情報を表すクラス
	//
	//	NOTES

	class _LogicalFile
	{
	public:
		// デフォルトコンストラクター
		_LogicalFile();
		// コンストラクター
		_LogicalFile(Schema::ObjectID::Value dbID,
					 const LogicalFile::FileDriver& driver,
					 LogicalFile::File* file);
		// デストラクター
		~_LogicalFile();

		// 破棄する
		bool
		destroy(const Trans::Transaction& trans,
				const Trans::TimeStamp& t, bool force);

		// 登録対象のファイルが属するデータベースID
		Schema::ObjectID::Value _dbID;
		// 登録時点での直前のチェックポイント処理の終了時タイムスタンプ
		Trans::TimeStamp		_t;
		// 論理ファイルドライバー
		const LogicalFile::FileDriver*	_driver;
		// 論理ファイル記述子
		LogicalFile::File*		_file;
	};

#ifdef OBSOLETE
	//	CLASS
	//	$$$::_EntryInfo::_PhysicalFile --
	//		破棄する物理ファイルに関する情報を表すクラス
	//
	//	NOTES

	class _PhysicalFile
	{
	public:
		// デフォルトコンストラクター
		_PhysicalFile();
		// コンストラクター
		_PhysicalFile(PhysicalFile::File* file);
		// デストラクター
		~_PhysicalFile();

		// 破棄する
		bool
		destroy(const Trans::Transaction& trans,
				const Trans::TimeStamp& t, bool force);

		// 登録時点での直前のチェックポイント処理の終了時タイムスタンプ
		Trans::TimeStamp		_t;
		// 物理ファイル記述子
		PhysicalFile::File*		_file;
	};

	//	CLASS
	//	$$$::_EntryInfo::_LogicalLog --
	//		破棄する論理ログファイルに関する情報を表すクラス
	//
	//	NOTES

	class _LogicalLog
	{
	public:
		// デフォルトコンストラクター
		_LogicalLog();
		// コンストラクター
		_LogicalLog(Trans::Log::File* file);
		// デストラクター
		~_LogicalLog();

		// 破棄する
		bool
		destroy(const Trans::TimeStamp& t, bool force);

		// 登録時点での直前のチェックポイント処理の終了時タイムスタンプ
		Trans::TimeStamp		_t;
		// 論理ログファイルに関する情報を記憶するクラス
		Trans::Log::File*		_file;
	};
#endif

	//	CLASS
	//	$$$::_EntryInfo::_Directory --
	//		破棄する OS ディレクトリに関する情報を表すクラス
	//
	//	NOTES

	class _Directory
	{
	public:
		// デフォルトコンストラクター
		_Directory();
		// コンストラクター
		_Directory(Schema::ObjectID::Value dbID,
				   const Os::Path& path, bool onlyDir);
		// デストラクター
		~_Directory();

		// 破棄する
		bool
		destroy(const Trans::TimeStamp& t, bool force);

		// 登録対象のディレクトリーが属するデータベースID
		Schema::ObjectID::Value _dbID;
		// 登録時点での直前のチェックポイント処理の終了時タイムスタンプ
		Trans::TimeStamp		_t;
		// 絶対パス名
		Os::Path				_path;
		// 子として OS ファイルを持つ OS ディレクトリは削除しないか
		bool					_onlyDir;

	private:
		// ある OS ディレクトリ以下の OS ディレクトリをすべて破棄する
		static bool
		destroy(const Os::Path& path);
	};

	// コンストラクター
	_EntryInfo(const Trans::Transaction::ID& id);
	// デストラクター
	~_EntryInfo();

	// クラスを生成する
	static _EntryInfo*
	attach(const Trans::Transaction::ID& id, bool bCreate_ = true);
	// クラスを破棄する
	static void
	detach(_EntryInfo*& info);

	// 破棄対象の登録を行ったトランザクションのトランザクション識別子
	Trans::Transaction::ID	_id;

	// 登録済の破棄する論理ファイルを管理するリスト
	ModVector<_LogicalFile>		_logicalFile;
#ifdef OBSOLETE
	// 登録済の破棄する物理ファイルを管理するリスト
	ModVector<_PhysicalFile>	_physicalFile;
	// 登録済の破棄する論理ログファイルを管理するリスト
	ModVector<_LogicalLog>		_logicalLog;
#endif
	// 登録済の破棄するディレクトリを管理するリスト
	ModVector<_Directory>		_directory;

	// 登録情報リストでの直前の要素へのポインタ
	_EntryInfo*				_prev;
	// 登録情報リストでの直後の要素へのポインタ
	_EntryInfo*				_next;
};

//	TYPEDEF
//	$$$::_EntryInfoList --
//		あるトランザクションについて破棄するオブジェクトの
//		登録に関する情報を表すクラスを管理するためのリスト
//
//	NOTES

typedef	Common::DoubleLinkedList<_EntryInfo>		_EntryInfoList;

namespace _FileDestroyer
{
	// 以下の情報を保護するためのラッチ
	Os::CriticalSection		_latch;
	// 登録情報を管理するリスト
	_EntryInfoList*			_infoList = 0;
}

//	FUNCTION public
//	$$$::_EntryInfo::_EntryInfo --
//		あるトランザクションについて破棄するオブジェクトの
//		登録に関する情報を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction::ID&	id
//			このトランザクション識別子の表すトランザクションについて
//			破棄するオブジェクトの登録に関する情報を表すクラスを生成する
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
//		あるトランザクションについて破棄するオブジェクトの
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
//		あるトランザクションについて破棄するオブジェクトの
//		登録に関する情報を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction::ID&	id
//			このトランザクション識別子の表すトランザクションについて
//			破棄するオブジェクトの登録に関する情報を表すクラスを生成する
//		bool bCreate = true
//			trueのときリスト中になかったら新規作成する
//			falseのときリスト中になかったら0を返す
//
//	RETURN
//		クラスを格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
_EntryInfo*
_EntryInfo::attach(const Trans::Transaction::ID& id, bool bCreate_)
{
	//【注意】	登録情報を管理するリストを保護するためにラッチ済であること

	if (!_FileDestroyer::_infoList) {

		// 登録情報を管理するリストが存在しないので、生成する

		_FileDestroyer::_infoList =
			new _EntryInfoList(&_EntryInfo::_prev, &_EntryInfo::_next);
		; _SYDNEY_ASSERT(_FileDestroyer::_infoList);
	}

	// 情報を登録しようとしているトランザクションについて、
	// すでに情報が登録されていないか調べる
	//
	//【注意】	リストはトランザクション識別子の昇順にソートされている

	_EntryInfoList::Iterator		ite(_FileDestroyer::_infoList->begin());
	const _EntryInfoList::Iterator&	end = _FileDestroyer::_infoList->end();

	for (; ite != end && (*ite)._id <= id; ++ite)
		if (id == (*ite)._id)

			// 情報を登録しようとしている
			// トランザクションについての情報がすでに登録されている

			return &*ite;

	if (!bCreate_) return 0;

	// 情報を登録しようとしているトランザクションについて、
	// 登録情報を表すクラスを生成する

	_EntryInfo*	info = new _EntryInfo(id);
	; _SYDNEY_ASSERT(info);

	// 生成した登録情報を表すクラスを
	// 登録情報を管理するリストの先頭に登録する

	(void) _FileDestroyer::_infoList->insert(ite, *info);

	return info;
}

//	FUNCTION public
//	$$$::_EntryInfo::detach --
//		あるトランザクションについて破棄するオブジェクトの
//		登録に関する情報を表すクラスを破棄する
//
//	NOTES
//
//	ARGUMENTS
//		$$$::_EntryInfo*&	info
//			このトランザクション識別子の表すトランザクションについて
//			破棄するオブジェクトの登録に関する情報を表すクラスを
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

		; _SYDNEY_ASSERT(_FileDestroyer::_infoList);
		_FileDestroyer::_infoList->erase(*info);

		// 指定された登録情報を破棄する

		delete info, info = 0;
	}
}

//	FUNCTION public
//	$$$::_EntryInfo::_LogicalFile::_LogicalFile --
//		破棄する論理ファイルに関する情報を表すクラスの
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
	  _file(0),
	  _dbID(Schema::ObjectID::Invalid)
{}

//	FUNCTION public
//	$$$::_EntryInfo::_LogicalFile::_LogicalFile --
//		破棄する論理ファイルに関する情報を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		LogicalFile::FileDriver&	driver
//			破棄する論理ファイルの論理ファイルドライバー記述子
//		LogicalFile::File*	file
//			破棄する論理ファイルの論理ファイル記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
_EntryInfo::_LogicalFile::_LogicalFile(
	Schema::ObjectID::Value dbID,
	const LogicalFile::FileDriver& driver, LogicalFile::File* file)
	: _t(TimeStamp::getMostRecent()),
	  _driver(&driver),
	  _file(file),
	  _dbID(dbID)
{}

//	FUNCTION public
//	$$$::_EntryInfo::_LogicalFile::~_LogicalFile --
//		破棄する論理ファイルに関する情報を表すクラスのデストラクター
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
//	$$$::_EntryInfo::_LogicalFile::destroy --
//		論理ファイルを破棄できれば、実際に破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			論理ファイルを破棄するトランザクションのトランザクション記述子
//		Trans::TimeStamp&	t
//			このタイムスタンプより前に情報が登録されたもののみ破棄する
//		bool			force
//			true
//				無条件に破棄する
//			false
//				指定されたタイムスタンプより前に
//				情報が登録されたもののみ、破棄する
//
//	RETURN
//		true
//			破棄した
//		false
//			破棄しなかった
//
//	EXCEPTIONS

inline
bool
_EntryInfo::_LogicalFile::destroy(
	const Trans::Transaction& trans, const Trans::TimeStamp& t, bool force)
{
	if (_file && (force || _t < t)) {
		if (Checkpoint::Database::isAvailable(_dbID))

			// 論理ファイルを破棄する
			_file->destroy(trans);

		// not availableでもエントリーからは除外する
		return true;
	}

	return false;
}

#ifdef OBSOLETE

//	FUNCTION public
//	$$$::_EntryInfo::_PhysicalFile::_PhysicalFile --
//		破棄する物理ファイルに関する情報を表すクラスの
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
//		破棄する物理ファイルに関する情報を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		PhysicalFile::File*	file
//			破棄する物理ファイルの物理ファイル記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
_EntryInfo::_PhysicalFile::_PhysicalFile(PhysicalFile::File* file)
	: _t(TimeStamp::getMostRecent()),
	  _file(file)
{}

//	FUNCTION public
//	$$$::_EntryInfo::_PhysicalFile::~_PhysicalFile --
//		破棄する物理ファイルに関する情報を表すクラスのデストラクター
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
//	$$$::_EntryInfo::_PhysicalFile::destroy --
//		物理ファイルを破棄できれば、実際に破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			物理ファイルを破棄するトランザクションのトランザクション記述子
//		Trans::TimeStamp&	t
//			このタイムスタンプより前に情報が登録されたもののみ破棄する
//		bool			force
//			true
//				無条件に破棄する
//			false
//				指定されたタイムスタンプより前に
//				情報が登録されたもののみ、破棄する
//
//	RETURN
//		true
//			破棄した
//		false
//			破棄しなかった
//
//	EXCEPTIONS

inline
bool
_EntryInfo::_PhysicalFile::destroy(
	const Trans::Transaction& trans, const Trans::TimeStamp& t, bool force)
{
	if (_file && (force || _t < t)) {

		// 物理ファイルを破棄する

		_file->destroy(trans);

		return true;
	}

	return false;
}

//	FUNCTION public
//	$$$::_EntryInfo::_LogicalLog::_LogicalLog --
//		破棄する論理ログファイルに関する情報を表すクラスの
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
//		破棄する論理ログファイルに関する情報を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::File*	file
//			破棄する論理ログファイルに関する情報を記憶するクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
_EntryInfo::_LogicalLog::_LogicalLog(Trans::Log::File* file)
	: _t(TimeStamp::getMostRecent()),
	  _file(file)
{}

//	FUNCTION public
//	$$$::_EntryInfo::_LogicalLog::~_LogicalLog --
//		破棄する論理ログファイルに関する情報を表すクラスのデストラクター
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
//	$$$::_EntryInfo::_LogicalLog::destroy --
//		論理ログファイルを破棄できれば、実際に破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp&	t
//			このタイムスタンプより前に情報が登録されたもののみ破棄する
//		bool			force
//			true
//				無条件に破棄する
//			false
//				指定されたタイムスタンプより前に
//				情報が登録されたもののみ、破棄する
//
//	RETURN
//		true
//			破棄した
//		false
//			破棄しなかった
//
//	EXCEPTIONS

inline
bool
_EntryInfo::_LogicalLog::destroy(const Trans::TimeStamp& t, bool force)
{
	if (_file && (force || _t < t)) {

		// 論理ログファイルを破棄する

		_file->destroy();

		return true;
	}

	return false;
}

#endif // OBSOLETE

//	FUNCTION public
//	$$$::_EntryInfo::_Directory::_Directory --
//		破棄する OS ディレクトリに関する情報を表すクラスの
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
	: _onlyDir(false),
	  _dbID(Schema::ObjectID::Invalid)
{}

//	FUNCTION public
//	$$$::_EntryInfo::_Directory::_Directory --
//		破棄する OS ディレクトリに関する情報を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			破棄する OS ディレクトリの絶対パス名
//		bool				onlyDir
//			true
//				OS ファイルを格納する OS ディレクトリは削除しない
//			false
//				指定された OS ディレクトリ以下はすべて削除する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
_EntryInfo::_Directory::_Directory(Schema::ObjectID::Value dbID,
								   const Os::Path& path, bool onlyDir)
	: _t(TimeStamp::getMostRecent()),
	  _path(path),
	  _onlyDir(onlyDir),
	  _dbID(dbID)
{}

//	FUNCTION public
//	$$$::_EntryInfo::_Directory::~_Directory --
//		破棄する OS ディレクトリに関する情報を表すクラスのデストラクター
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
//	$$$::_EntryInfo::_Directory::destroy --
//		OS ディレクトリを破棄できれば、実際に破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp&	t
//			このタイムスタンプより前に情報が登録されたもののみ破棄する
//		bool				force
//			true
//				無条件に破棄する
//			false
//				指定されたタイムスタンプより前に
//				情報が登録されたもののみ、破棄する
//
//	RETURN
//		true
//			破棄した
//		false
//			破棄しなかった
//
//	EXCEPTIONS

bool
_EntryInfo::_Directory::destroy(const Trans::TimeStamp& t, bool force)
{
	bool	ret = false;

	if (!Checkpoint::Database::isAvailable(_dbID))
		ret = true;
	else if (force || _t < t)
	{
		if (!Os::Directory::access(_path, Os::Directory::AccessMode::File))
		{
			// OS ディレクトリが存在しない場合は
			// 破棄したこととする
			
			ret = true;
		}
		else
		{
			try {
				// 依頼された絶対パス名の表すものが
				// OS ディレクトリであることを確認する

				if (ret = ModOsDriver::File::isDirectory(_path))

					// 依頼された絶対パス名の表すものは
					// OS ディレクトリである

					if (_onlyDir)

						// OS ディレクトリおよび
						// それ以下の OS ディレクトリをすべて破棄する

						ret = destroy(_path);
					else {

						// OS ディレクトリ以下をすべて破棄する

						ModOsDriver::File::rmAll(_path, ModTrue);
						ret = true;
					}
			}
#ifdef NO_CATCH_ALL
			catch (Exception::Object&)
#else
			catch (...)
#endif
			{
				
				// エラーが起きても無視する
				
				Common::Thread::resetErrorCondition();
			}
		}
	}
	
	return ret;
}

//	FUNCTION private
//	$$$::_EntryInfo::_Directory::destroy --
//		ある OS ディレクトリ以下のすべての OS ディレクトリを破棄する
//
//	NOTES
//		指定された OS ディレクトリ以下の OS ディレクトリのうち、
//		その直下に OS ファイルが存在しないものをすべて削除する
//
//
//	ARGUMENTS
//		Os::Path&			path
//			削除する OS ディレクトリの絶対パス名
//
//	RETURN
//		true
//			指定された OS ディレクトリ以下のすべてを削除した
//		false
//			指定されたのは、OS ディレクトリでなかったか、
//			指定された OS ディレクトリ以下のすべてを削除できなかった
//
//	EXCEPTIONS

// static
bool
_EntryInfo::_Directory::destroy(const Os::Path& path)
{
	; _SYDNEY_ASSERT(ModOsDriver::File::isDirectory(path));

	bool ret = true;

	Os::Path parent(path);
	parent += ModOsDriver::File::getPathSeparator();

	// 指定された OS ディレクトリの直下に存在するものを得る

	ModUnicodeString** children = 0;
	int n = 0;

	ModOsDriver::File::getDirectoryEntry(path, &children, &n);
	; _SYDNEY_ASSERT(!n || children);

	try {
		// 得られたそれぞれについて、同様に処理していく

		for (int i = 0; i < n; ++i) {
			; _SYDNEY_ASSERT(children[i]);

			if (!destroy(parent + *children[i]))

				// これは処理しなかった

				ret = false;

			delete children[i], children[i] = 0;
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		for (int i = 0; i < n; ++i)
			delete children[i];
		ModOsManager::free(children, sizeof(ModUnicodeString*) * n);

		_SYDNEY_RETHROW;
	}

	ModOsManager::free(children, sizeof(ModUnicodeString*) * n);

	if (ret)

		// 指定された OS ディレクトリの直下に存在するものを
		// すべて削除していれば、指定された OS ディレクトリも削除する

		Os::Directory::remove(path);

	return ret;
}

}

//	FUNCTION private
//	Checkpoint::Manager::FileDestroyer::initialize --
//		マネージャーの初期化のうち、
//		チェックポイント処理のファイル破棄関連の初期化を行う
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
Manager::FileDestroyer::initialize()
{}

//	FUNCTION private
//	Checkpoint::Manager::FileDestroyer::terminate --
//		マネージャーの後処理のうち、
//		チェックポイント処理のファイル破棄関連の後処理を行う
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
Manager::FileDestroyer::terminate()
{
	if (_FileDestroyer::_infoList) {

		// 登録情報を管理するリストは空である必要がある

		; _SYDNEY_ASSERT(_FileDestroyer::_infoList->isEmpty());

		// 登録情報を管理するリストを破棄する

		delete _FileDestroyer::_infoList, _FileDestroyer::_infoList = 0;
	}
}

//	FUNCTION public
//	Checkpoint::FileDestroyer::enter --
//		ある論理ファイルをチェックポイント処理時に実際に破棄するように依頼する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			依頼するトランザクションのトランザクション記述子
//		LogicalFile::FileDriver&	driver
//			破棄が依頼された論理ファイルの論理ファイルドライバー記述子
//		LogicalFile::File&	file
//			破棄が依頼された論理ファイルの論理ファイル記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
FileDestroyer::enter(const Trans::Transaction& trans,
					 Schema::ObjectID::Value dbID,
					 const LogicalFile::FileDriver& driver,
					 const LogicalFile::File& file)
{
	// 登録情報を管理するリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(_FileDestroyer::_latch);

	// 情報を登録しようとしているトランザクションに関する
	// 登録情報を表すクラスを得る

	_EntryInfo*	info = _EntryInfo::attach(trans.getID());
	; _SYDNEY_ASSERT(info);

	// 得られた登録情報に指定された論理ファイルを加える

	info->_logicalFile.pushBack(
		_EntryInfo::_LogicalFile(
			dbID, driver, driver.attachFile(&file)));
}

#ifdef OBSOLETE

//	FUNCTION public
//	Checkpoint::FileDestroyer::enter --
//		ある物理ファイルをチェックポイント処理時に実際に破棄するように依頼する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			依頼するトランザクションのトランザクション記述子
//		PhysicalFile::File&	file
//			破棄が依頼された物理ファイルの物理ファイル記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
FileDestroyer::enter(
	const Trans::Transaction& trans, const PhysicalFile::File& file)
{
	// 登録情報を管理するリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(_FileDestroyer::_latch);

	// 情報を登録しようとしているトランザクションに関する
	// 登録情報を表すクラスを得る

	_EntryInfo*	info = _EntryInfo::attach(trans.getID());
	; _SYDNEY_ASSERT(info);

	// 得られた登録情報に指定された物理ファイルを加える

	info->_physicalFile.pushBack(
		_EntryInfo::_PhysicalFile(
			PhysicalFile::Manager::attachFile(&file)));
}

//	FUNCTION public
//	Checkpoint::FileDestroyer::enter --
//		ある論理ログファイルをチェックポイント処理時に
//		実際に破棄するように依頼する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			依頼するトランザクションのトランザクション記述子
//		Trans::Log::File&	file
//			破棄が依頼された論理ログファイルに関する情報を記憶するクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
FileDestroyer::enter(
	const Trans::Transaction& trans, const Trans::Log::File& file)
{
	// 登録情報を管理するリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(_FileDestroyer::_latch);

	// 情報を登録しようとしているトランザクションに関する
	// 登録情報を表すクラスを得る

	_EntryInfo*	info = _EntryInfo::attach(trans.getID());
	; _SYDNEY_ASSERT(info);

	// 得られた登録情報に指定された論理ログファイルを加える

	info->_logicalLog.pushBack(
		_EntryInfo::_LogicalLog(Trans::Log::File::attach(file)));
}

#endif // OBSOLETE

//	FUNCTION public
//	Checkpoint::FileDestroyer::enter --
//		ある OS ディレクトリをチェックポイント処理時に
//		実際に破棄するように依頼する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			破棄が依頼された OS ディレクトリの絶対パス名
//		bool				onlyDir
//			true
//				OS ファイルを格納する OS ディレクトリは削除しない
//			false
//				指定された OS ディレクトリ以下はすべて削除する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
FileDestroyer::enter(Schema::ObjectID::Value dbID,
					 const Os::Path& path, bool onlyDir)
{
	// 登録情報を管理するリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(_FileDestroyer::_latch);

	// トランザクションの指定のない登録情報を表すクラスを得る

	_EntryInfo*	info = _EntryInfo::attach(Trans::IllegalID);
	; _SYDNEY_ASSERT(info);

	// 得られた登録情報に指定された OS ディレクトリを加える

	info->_directory.pushBack(_EntryInfo::_Directory(dbID, path, onlyDir));
}

// 論理ファイルの破棄依頼を取り消す
//static
void
FileDestroyer::
erase(const Trans::Transaction& trans,
	  const LogicalFile::FileDriver& driver,
	  const LogicalFile::File& file)
{
	// 登録情報を管理するリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(_FileDestroyer::_latch);

	// 情報を登録しようとしているトランザクションに関する
	// 登録情報を表すクラスを得る

	_EntryInfo*	info = _EntryInfo::attach(trans.getID(), false /* not create */);

	if (info) {

		// 登録情報からDriverとFileが一致するエントリーを削除する

		if (info->_logicalFile.getSize()) {
			ModVector<_EntryInfo::_LogicalFile>::Iterator
				ite(info->_logicalFile.end());
			const ModVector<_EntryInfo::_LogicalFile>::Iterator&
				begin = info->_logicalFile.begin();

			do {
				_EntryInfo::_LogicalFile& object = *--ite;

				; _SYDNEY_ASSERT(object._driver);
				; _SYDNEY_ASSERT(object._file);
				if (object._driver->getDriverID() == driver.getDriverID()) {
					const LogicalFile::FileID& fileID1 = object._file->getFileID();
					const LogicalFile::FileID& fileID2 = file.getFileID();
					
					if (fileID1.getInteger(LogicalFile::Parameter::Key(LogicalFile::FileID::KeyNumber::SchemaDatabaseID))
						== fileID2.getInteger(LogicalFile::Parameter::Key(LogicalFile::FileID::KeyNumber::SchemaDatabaseID))
						&&
						fileID1.getInteger(LogicalFile::Parameter::Key(LogicalFile::FileID::KeyNumber::SchemaTableID))
						== fileID2.getInteger(LogicalFile::Parameter::Key(LogicalFile::FileID::KeyNumber::SchemaTableID))
						&&
						fileID1.getInteger(LogicalFile::Parameter::Key(LogicalFile::FileID::KeyNumber::SchemaFileObjectID))
						== fileID2.getInteger(LogicalFile::Parameter::Key(LogicalFile::FileID::KeyNumber::SchemaFileObjectID))) {

						// ファイルをデタッチする
						object._driver->detachFile(object._file);

						// その破棄の指示の記録を抹消する

						info->_logicalFile.erase(ite);
					}
				}
			} while (ite != begin) ;
		}
	}
}

// OS ディレクトリの破棄依頼を取り消す
//static
void
FileDestroyer::
erase(const Os::Path& path)
{
	// 登録情報を管理するリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(_FileDestroyer::_latch);

	// トランザクションの指定のない登録情報を表すクラスを得る

	_EntryInfo*	info = _EntryInfo::attach(Trans::IllegalID, false /* not create */);

	if (info) {
		if (info->_directory.getSize()) {
			ModVector<_EntryInfo::_Directory>::Iterator
				ite(info->_directory.end());
			const ModVector<_EntryInfo::_Directory>::Iterator&
				begin = info->_directory.begin();

			do {
				_EntryInfo::_Directory& object = *--ite;

				// 同じパス名のエントリーを削除する
				// enterとeraseで大文字小文字が変わることはないはずなので
				// compareではなく通常の文字列比較で比較する

				if (object._path == path)
					info->_directory.erase(ite);

			} while (ite != begin) ;
		}
	}
}

//	FUNCTION private
//	Checkpoint::FileDestroyer::execute --
//		破棄が依頼されているオブジェクトを可能な限り実際に破棄する
//
//	NOTES
//		チェックポイントスレッドから呼び出される
//
//	ARGUMENTS
//		bool				force
//			true
//				破棄が依頼されているオブジェクトをすべて実際に破棄する
//			false
//				直前のチェックポイント以前に破棄が依頼されたもので、
//				それを参照中の参照トランザクションが存在しなければ、破棄する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
FileDestroyer::execute(bool force)
{
	// 登録情報を管理するリストを保護するためにラッチする

	Os::AutoCriticalSection	latch(_FileDestroyer::_latch);

	if (!_FileDestroyer::_infoList || _FileDestroyer::_infoList->isEmpty())

		// 登録情報を管理するリストが存在しない、
		// または空なので、なにもしない

		return;

	// 登録情報を管理するリストの
	// 先頭からひとつひとつ登録情報を調べていく
	//
	//【注意】	リストの先頭から調べることにより、
	//			OS ディレクトリを最後に処理することができる

	_EntryInfoList::Iterator		ite(_FileDestroyer::_infoList->begin());
	const _EntryInfoList::Iterator&	end = _FileDestroyer::_infoList->end();

	do {
		_EntryInfo* info = &*ite;

		// 登録情報を表すクラスが破棄されると、
		// その登録情報を指す反復子は不正になるので、先に次を求めておく

		++ite;

		// 直前のチェックポイント処理の終了時タイムスタンプを求める

		Trans::TimeStamp	last(TimeStamp::getMostRecent());
		{
		// 論理ファイル、物理ファイルを破棄するために、
		// 更新トランザクションを開始する
		//
		//【注意】	他から参照されることはまったくないので
		//			ロックしないトランザクションを開始する
		//
		//【注意】	ページ操作はしないので、データベースIDとしてシステム表
		//			のデータベースIDを利用する

		Trans::AutoTransaction	trans(Trans::Transaction::attach());
		trans->begin(Schema::ObjectID::SystemTable,
					 Trans::Transaction::Mode(
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

				// このデータベースの連続したトランザクションのうち、
				// 最初に開始したトランザクションより、
				// 情報を登録した更新トランザクションが新しい場合は、
				// スキップする
				
				if (!force && !info->_id.isIllegal())
				{
					Trans::ID bid
						= Trans::Transaction::getBeginningID(object._dbID);
					if (!bid.isIllegal() && info->_id > bid)
						continue;
				}

				// 論理ファイルの破棄が指示されてから、
				// 初めてのチェックポイント処理でなければ、
				// 論理ファイルを実際に破棄する
				//
				//【注意】	論理ファイルの破棄が指示されてから、
				//			初めてのチェックポイント処理であれば、
				//			破棄する論理ファイルに関する更新内容が
				//			フラッシュされていない可能性があるので、
				//			破棄できない

				if (object.destroy(*trans, last, force)) {

					// 論理ファイルを破棄した

					// 破棄した論理ファイルの論理ファイル記述子を破棄する

					; _SYDNEY_ASSERT(object._driver);
					object._driver->detachFile(object._file);

					// その破棄の指示の記録を抹消する

					info->_logicalFile.erase(ite);
				}
			} while (ite != begin) ;
		}

#ifdef OBSOLETE
		if (info->_physicalFile.getSize()) {
			ModVector<_EntryInfo::_PhysicalFile>::Iterator
				ite(info->_physicalFile.end());
			const ModVector<_EntryInfo::_PhysicalFile>::Iterator&
				begin = info->_physicalFile.begin();

			do {
				_EntryInfo::_PhysicalFile& object = *--ite;

				// 物理ファイルの破棄が指示されてから、
				// 初めてのチェックポイント処理でなければ、
				// 物理ファイルを実際に破棄する
				//
				//【注意】	物理ファイルの破棄が指示されてから、
				//			初めてのチェックポイント処理であれば、
				//			破棄する物理ファイルに関する更新内容が
				//			フラッシュされていない可能性があるので、
				//			破棄できない

				if (object.destroy(*trans, last, force)) {

					// 物理ファイルを破棄した

					// 破棄した物理ファイルの物理ファイル記述子を破棄する

					PhysicalFile::Manager::detachFile(object._file);

					// その破棄の指示の記録を抹消する

					info->_physicalFile.erase(ite);
				}
			} while (ite != begin) ;
		}
#endif

		// 論理ファイル、物理ファイルを破棄した
		// 更新トランザクションを終了する

		trans->commit();
		}
#ifdef OBSOLETE
		if (info->_logicalLog.getSize()) {
			ModVector<_EntryInfo::_LogicalLog>::Iterator
				ite(info->_logicalLog.end());
			const ModVector<_EntryInfo::_LogicalLog>::Iterator&
				begin = info->_logicalLog.begin();

			do {
				_EntryInfo::_LogicalLog& object = *--ite;

				// 論理ログファイルの破棄が指示されてから、
				// 初めてのチェックポイント処理でなければ、
				// 論理ログファイルを実際に破棄する
				//
				//【注意】	論理ログファイルの破棄が指示されてから、
				//			初めてのチェックポイント処理であれば、
				//			論理ログファイルに記録される操作の対象となる
				//			論理ファイル、物理ファイルを破棄する前に
				//			論理ログファイルを破棄してしまう可能性があるので、
				//			破棄できない

				if (object.destroy(last, force)) {

					// 論理ログファイルを破棄した

					// 破棄した論理ログファイルの
					// 論理ログファイルの情報を表すクラスを破棄する

					Trans::Log::File::detach(object._file);

					// その破棄の指示の記録を抹消する

					info->_logicalLog.erase(ite);
				}
			} while (ite != begin) ;
		}
#endif

		if (info->_directory.getSize()) {
			ModVector<_EntryInfo::_Directory>::Iterator
				ite(info->_directory.end());
			const ModVector<_EntryInfo::_Directory>::Iterator&
				begin = info->_directory.begin();

			do {
				_EntryInfo::_Directory& object = *--ite;

				// OS ディレクトリの破棄が指示されてから、
				// 初めてのチェックポイント処理でなければ、
				// OS ディレクトリを実際に破棄する
				//
				//【注意】	OS ディレクトリの破棄が指示されてから、
				//			初めてのチェックポイント処理であれば、
				//			破棄する OS ディレクトリ以下の論理ファイル、
				//			物理ファイル、論理ログファイルが
				//			破棄されていない可能性があるので、破棄できない

				if (object.destroy(last, force))

					// OS ディレクトリ以下をすべて破棄したので、
					// その破棄の指示の記録を抹消する

					info->_directory.erase(ite);

			} while (ite != begin) ;
		}

		if (info->_logicalFile.isEmpty()
#ifdef OBSOLETE
			&&	info->_physicalFile.isEmpty()
			&& info->_logicalLog.isEmpty()
#endif
			&& info->_directory.isEmpty()
		)

			// 登録情報に破棄対象がひとつも記録されていないので、
			// 登録情報を表すクラスを破棄する

			_EntryInfo::detach(info);

	} while (ite != end) ;
}

//
// Copyright (c) 2001, 2002, 2005, 2006, 2013, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
