// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TimeStamp.cpp -- タイムスタンプ関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
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

#include "Trans/AutoLatch.h"
#include "Trans/AutoTransaction.h"
#include "Trans/Configuration.h"
#include "Trans/LogData.h"
#include "Trans/Manager.h"
#include "Trans/TimeStamp.h"
#include "Trans/PathParts.h"

#include "Buffer/AutoFile.h"
#include "Buffer/AutoPool.h"
#include "Buffer/Memory.h"
#include "Buffer/Page.h"
#include "Common/Assert.h"
#include "Common/InputArchive.h"
#include "Common/OutputArchive.h"
#include "Common/Message.h"
#include "Common/UnicodeString.h"
#include "Common/UnsignedInteger64Data.h"
#include "Exception/BadDataPage.h"
#include "Exception/DuplicateServer.h"
#include "Exception/TimeStampFileCorrupted.h"
#include "Os/AutoCriticalSection.h"
#include "Os/SysConf.h"

#include "ModArchive.h"

_SYDNEY_USING
_SYDNEY_TRANS_USING

namespace
{

namespace _TimeStamp
{
	namespace _VersionNumber
	{
		//	ENUM
		//	$$$::_TimeStamp::_VersionNumber::Value --
		//		タイムスタンプファイルのバージョン番号を表す値の列挙型
		//
		//	NOTES

		enum Value
		{
			// 最初
			First =				0,
			// 2番目
			Second,
			// 現在
			Current =			Second,
			// 値の数
			Count,
			// 不明
			Unknown =			Count
		};
	}

	//	STRUCT
	//	$$$::_TimeStamp::_FileHeader --
	//		タイムスタンプ値を永続化するファイルのヘッダ
	//
	//	NOTES

	struct _FileHeader
	{
		// バージョン番号
		_VersionNumber::Value	_versionNumber;
#ifdef SYD_C_GCC
		// Microsoft Visual C++ のデフォルトのアライメントに合わせる
		char					_dummy[4];
#endif
		// 永続化するタイムスタンプ値
		TimeStamp::Value		_value;
	};

#ifdef SYD_C_GCC
	// バージョン番号がFirstのLinux版はWindows版とアライメントが合っていない
	// その古いバージョンを読むときに使用する
	struct _FileHeader_First
	{
		// バージョン番号
		_VersionNumber::Value	_versionNumber;
		// 永続化するタイムスタンプ値
		TimeStamp::Value		_value;
	};
#endif

	// システム専用のタイムスタンプファイルの絶対パス名を得る
	Os::Path
	getPath();
	// システム用の論理ログファイルに
	// タイムスタンプ値の上位 32 ビットを新たに生成したことを記録する
	void
	storeLog(const TimeStamp& t);

	// タイムスタンプ値を永続化するバッファファイルのページサイズ(B 単位)
	const Os::Memory::Size	PageSize = Os::SysConf::PageSize::get();
	// ファイルヘッダをいくつ多重化して記憶するか
	const unsigned int		MultiplexCount = 2;

	// 以下の情報を保護するためのラッチ
	Os::CriticalSection		_latch;
	// システムを初期化したときのタイムスタンプ
	TimeStamp				_systemInitialized;
	// 最後に生成したタイムスタンプ
	TimeStamp				_last;
	
	// 排他ロックするロックファイル
	Os::File				_lockfile;
	// 排他ロックするロックファイルの絶対パス名を得る
	Os::Path				getLockPath();
	// システムをインストールしたかどうか
	bool					_isInstalled = false;
}

//	FUNCTION
//	$$$::_TimeStamp::getPath --
//		システム専用のタイムスタンプファイルの絶対パス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイルの絶対パス名
//
//	EXCEPTIONS

inline
Os::Path
_TimeStamp::getPath()
{
	return Os::Path(
		Configuration::TimeStampPath::get()).addPart(PathParts::TimeStamp);
}

//	FUNCTION
//	$$$::_TimeStamp::storeLog --
//		システム用の論理ログファイルに
//		タイムスタンプ値の上位 32 ビットを新たに生成したことを記録する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp&	t
//			割り当てられたタイムスタンプ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_TimeStamp::storeLog(const TimeStamp& t)
{
	//【注意】	論理ログを記録するためだけに
	//			トランザクション記述子を生成する

	AutoTransaction	trans(Transaction::attach());
	AutoLatch latch(
		*trans,	trans->getLogInfo(Log::File::Category::System).getLockName());

	(void) trans->storeLog(
		Log::File::Category::System, Log::TimeStampAssignData(t));
}

//	FUNCTION
//	$$$::_TimeStamp::getLockPath --
//		二重立ち上げ防止ロックファイルの絶対パス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイルの絶対パス名
//
//	EXCEPTIONS

inline
Os::Path
_TimeStamp::getLockPath()
{
	return Os::Path(
		Configuration::TimeStampPath::get()).addPart(PathParts::LockFile);
}

}

//	FUNCTION private
//	Trans::Manager::TimeStamp::initialize --
//		マネージャーのうち、タイムスタンプ関連を初期化する
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
Manager::TimeStamp::initialize()
{
	// システム専用のタイムスタンプファイルの絶対パス名を求める

	const Os::Path& path = _TimeStamp::getPath();

#ifndef OBSOLETE_OLIVE_SW2

	//【注意】	下記のコードは Olive-SW2 が廃止された時点で削除できる

	Trans::TimeStamp t;
	if (Os::File::getSize(path) == sizeof(t._value.halfs.high)) {

		// Olive-SW2 で使用している古い形式のタイムスタンプファイルである

		// 古い形式のタイムスタンプファイルから
		// タイムスタンプ値の上位 32 ビットを読み出し、
		// 下位 32 ビットは 0 にする
		{
		Os::File file(path);
		file.open(Os::File::OpenMode::Read);
		Os::File::IOBuffer buf(&t._value.halfs.high);
		file.read(buf, sizeof(t._value.halfs.high), 0);
		t._value.halfs.low = 0;
		}
		// 置き換えるための一時名を生成しておく

		Os::Path oldPath(path);
		oldPath.append(_TRMEISTER_U_STRING("0"));
		Os::Path newPath(path);
		newPath.append(_TRMEISTER_U_STRING("1"));

		try {
			// 新しい形式のタイムスタンプファイルを生成し、
			// 先ほど取り出したタイムスタンプ値を記録する

			Trans::TimeStamp::createFile(newPath);
			t.store(newPath);

			// 古い形式のタイムスタンプファイルを一時名に変える

			Os::File::rename(path, oldPath, true);

			try {

				// 新しい形式のタイムスタンプファイルを正式名に変える

				Trans::TimeStamp::renameFile(newPath, path);

			} catch (...) {

				Os::File::rename(oldPath, path, true);
				_SYDNEY_RETHROW;
			}
		} catch (...) {

			Trans::TimeStamp::destroyFile(newPath);
			_SYDNEY_RETHROW;
		}

		// 古い形式のタイムスタンプファイルを削除する

		Os::File::remove(oldPath);
	}
#endif
	// タイムスタンプパスをログに出力する

	SydMessage << "TimeStamp: " << path << ModEndl;

	if (_TimeStamp::_isInstalled == false) {
		
		// 他のプロセスが起動していないことを保障するために、
		// 二重立ち上げ防止用のファイルをロックする
		//
		//【注意】
		// タイムスタンプのパスがネットワークドライブの時、
		// Sydneyのインストール時にロックしてしまうと、
		// その後の VirtualAlloc でなぜか ERROR_NOT_ENOUGH_MEMORY が返ってくる。
		// よって、インストール時にはロックは行わないようにする

		_TimeStamp::_lockfile.setPath(_TimeStamp::getLockPath());
		_TimeStamp::_lockfile.open(Os::File::OpenMode::Read |
								   Os::File::OpenMode::Write |
								   Os::File::OpenMode::NoBuffering |
								   Os::File::OpenMode::Create);

		if (!_TimeStamp::_lockfile.trylock())

			// 同じタイムスタンプファイルを使用する他のプロセスが起動されている

			_SYDNEY_THROW0(Exception::DuplicateServer);
	}
}

//	FUNTION private
//	Trans::Manager::TimeStamp::terminate --
//		マネージャーのうち、タイムスタンプ関連を終了処理する
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
Manager::TimeStamp::terminate()
{
	if (_TimeStamp::_isInstalled == false) {
		
		// 二重立ち上げ防止用ロックファイルをクローズする
		//
		//【注意】	自動的に排他ロックははずされる

		_TimeStamp::_lockfile.close();
	}

	_TimeStamp::_systemInitialized = IllegalTimeStamp;
	_TimeStamp::_last = IllegalTimeStamp;
}

//	FUNCTION private
//	Trans::Manager::TimeStamp::install --
//		マネージャーのうち、タイムスタンプ関連のインストールを行う
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
Manager::TimeStamp::install()
{
	// 情報を保護するためにラッチする

	Os::AutoCriticalSection	latch(_TimeStamp::_latch);

	// システム専用のタイムスタンプファイルを生成する

	Trans::TimeStamp::createFile(_TimeStamp::getPath());

	// 記録した初期値を最後に求めたタイムスタンプとして記憶しておく

	_TimeStamp::_last = _TimeStamp::_systemInitialized = 0;

	// このプロセスでインストールしたかどうか覚えておく

	_TimeStamp::_isInstalled = true;
}

//	FUNCTION private
//	Trans::Manager::TimeStamp::uninstall --
//		マネージャーのうち、タイムスタンプ関連のアンインストールを行う
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
Manager::TimeStamp::uninstall()
{
	// 情報を保護するためにラッチする

	Os::AutoCriticalSection	latch(_TimeStamp::_latch);

	// システム専用のタイムスタンプファイルを破棄する

	Trans::TimeStamp::destroyFile(_TimeStamp::getPath());
}

#ifndef SYD_COVERAGE
//	FUNCTION public
//	Trans::TimeStamp::toString -- 文字列で取り出す
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた文字列
//
//	EXCEPTIONS

ModUnicodeString
TimeStamp::toString() const
{
	return Common::UnsignedInteger64Data(operator Value()).toString();
}
#endif

//	FUNCTION public
//	Trans::TimeStamp::serialize -- クラスをシリアル化する
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&			archiver
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
TimeStamp::serialize(ModArchive& archiver)
{
	if (archiver.isStore())
		archiver << _value.full;
	else
		archiver >> _value.full;
}

//	FUNCTION public
//	Trans::TimeStamp::assign -- 新しいタイムスタンプを割り当てる
//
//	NOTES
//
//	ARGUMENTS
//		bool				isNoLog
//			true
//				タイムスタンプ値の上位 32 ビットを
//				割り当てたことを表す論理ログを記録しない
//			false または指定されいないとき
//				タイムスタンプ値の上位 32 ビットを
//				割り当てたことを表す論理ログを記録する
//
//	RETURN
//		割り当てられたタイムスタンプ
//
//	EXCEPTIONS

// static
TimeStamp
TimeStamp::assign(bool isNoLog)
{
	// 情報を保護するためにラッチする

	Os::AutoCriticalSection	latch(_TimeStamp::_latch);

	// システムを起動してから
	// タイムスタンプを求めたことがあるか調べる

	if (_TimeStamp::_last.isIllegal())

		// システムが初期化されたときのタイムスタンプを求める

		return TimeStamp::getSystemInitialized();

	// 最後に求めたタイムスタンプに 1 加えたものを求める

	TimeStamp t(_TimeStamp::_last);
	++t;

	if (!t._value.halfs.low) {

		// その結果、タイムスタンプ値の下位 32 ビットが繰り上がり、
		// 上位 32 ビットが 1 増えた

		if (!isNoLog)

			// 論理ログを記録する

			_TimeStamp::storeLog(t);

		// 上位 32 ビットが新たに生成されたタイムスタンプ値を永続化する

		t.store(_TimeStamp::getPath());
	}

	// 新たに求めたタイムスタンプを最後に求めたタイムスタンプとする

	return _TimeStamp::_last = t;
}

//	FUNCTION public
//	Trans::TimeStamp::assign -- 既存のタイムスタンプを割り当てる
//
//	NOTES
//		指定されたタイムスタンプが最後に求めたタイムスタンプより大きければ、
//		最後に求めたタイムスタンプを指定したものにする
//
//	ARGUMENTS
//		Trans::TimeStamp&	src
//			割り当てるタイムスタンプ
//		bool				isNoLog
//			true
//				タイムスタンプ値の上位 32 ビットを
//				割り当てたことを表す論理ログを記録しない
//			false または指定されいないとき
//				タイムスタンプ値の上位 32 ビットを
//				割り当てたことを表す論理ログを記録する
//
//	RETURN
//		割り当てられたタイムスタンプ
//
//	EXCEPTIONS

// static
TimeStamp
TimeStamp::assign(const TimeStamp& src, bool isNoLog)
{
	// 情報を保護するためにラッチする

	Os::AutoCriticalSection	latch(_TimeStamp::_latch);

	// システムを起動してから
	// タイムスタンプを求めたことがあるか調べる

	if (_TimeStamp::_last.isIllegal())

		// システムが初期化されたときのタイムスタンプを求める

		(void) TimeStamp::getSystemInitialized();

	// 最後に求めたタイムスタンプより
	// 指定されたタイムスタンプが大きいか調べる

	if (!src.isIllegal() && _TimeStamp::_last < src) {
		if (!isNoLog)

			// 論理ログを記録する

			_TimeStamp::storeLog(src);

		// 指定されたものを新しいタイムスタンプ値として永続化する

		src.store(_TimeStamp::getPath());

		// 指定されたものを最後に求めたタイムスタンプとする

		_TimeStamp::_last = src;
	}

	return _TimeStamp::_last;
}

//	FUNCTION public
//	Trans::TimeStamp::redo -- 障害回復のために REDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::TimeStampAssignData&	data
//			新たなタイムスタンプ値の上位 32 ビットの生成の論理ログデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
TimeStamp::redo(const Log::TimeStampAssignData& data)
{
	; _SYDNEY_ASSERT(!data.getAssigned().isIllegal());

	// 情報を保護するためにラッチする

	Os::AutoCriticalSection	latch(_TimeStamp::_latch);

	if (_TimeStamp::_last.isIllegal() ||
		_TimeStamp::_last > data.getAssigned())

		// システムを起動してからタイムスタンプを求めたことがないか、
		// 論理ログデータに記録されているタイムスタンプが
		// システムが初期化されたときのタイムスタンプより小さければ、
		// それをシステムが初期化されたときのタイムスタンプとする

		_TimeStamp::_systemInitialized = data.getAssigned();

	// 論理ログデータに記録されている
	// 過去に割り当てたタイムスタンプ値を永続化する

	data.getAssigned().store(_TimeStamp::getPath());

	// 論理ログデータに記録されている過去に割り当てた
	// タイムスタンプを最後に求めたタイムスタンプとする

	_TimeStamp::_last = data.getAssigned();
}

//	FUNCTION public
//	Trans::TimeStamp::getSystemInitialized --
//		システムが初期化されたときのタイムスタンプを得る
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
//		なし

// static
const TimeStamp&
TimeStamp::getSystemInitialized()
{
	// 情報を保護するためにラッチする

	Os::AutoCriticalSection	latch(_TimeStamp::_latch);

	// システムを起動時のタイムスタンプを
	// これまでに求めているか調べる

	if (_TimeStamp::_systemInitialized.isIllegal()) {

		// システム専用のタイムスタンプファイルの絶対パス名を求める

		const Os::Path& path = _TimeStamp::getPath();

		// 永続化されているタイムスタンプを取り出す

		TimeStamp t;
		t.load(path);

		// タイムスタンプ値の上位 32 ビットは、
		// インストールされてからのシステムの起動回数を意味する
		//
		// そこで、起動回数を 1 増やす

		++t._value.halfs.high;

		// 論理ログを記録する

		_TimeStamp::storeLog(t);

		// 新しいタイムスタンプ値を永続化する

		t.store(path);

		// システムを起動してから初めてタイムスタンプを求めたので、
		// 最後に求めたタイムスタンプとして記憶しておく

		_TimeStamp::_last = _TimeStamp::_systemInitialized = t;
	}

	return _TimeStamp::_systemInitialized;
}

//	FUNCTION public
//	Trans::TimeStamp::getPersisted -- 永続化されているタイムスタンプを得る
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

// static
TimeStamp
TimeStamp::getPersisted()
{
	// 情報を保護するためにラッチする

	Os::AutoCriticalSection	latch(_TimeStamp::_latch);

	// 永続化されているタイムスタンプを取り出す

	TimeStamp t;
	t.load(_TimeStamp::getPath());

	return t;
}

//	FUNCTION public
//	Trans::TimeStamp::getLast -- 最後に割り当てたタイムスタンプを得る
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

// static
TimeStamp
TimeStamp::getLast()
{
	// 情報を保護するためにラッチする

	Os::AutoCriticalSection	latch(_TimeStamp::_latch);

	// システムを起動してから
	// タイムスタンプを求めたことがなければ、
	// システムが初期化されたときのタイムスタンプを、
	// そうでなければ、最後に求めたタイムスタンプを返す

	return (_TimeStamp::_last.isIllegal()) ?
		TimeStamp::getSystemInitialized() : _TimeStamp::_last;
}

//	FUNCTION private
//	Trans::TimeStamp::createFile --
//		タイムスタンプ値を永続化するファイルを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			タイムスタンプ値を永続化する
//			バッファファイルの実体である OS ファイルの絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
TimeStamp::createFile(const Os::Path& path)
{
	// タイムスタンプ値を永続化するバッファファイルの記述子を得る

	Buffer::AutoPool pool(
		Buffer::Pool::attach(Buffer::Pool::Category::Normal));
	Buffer::AutoFile file(
		Buffer::File::attach(
			*pool, path, _TimeStamp::PageSize, false, false, false));

	// バッファファイルを生成する

	file->create(false);

	try {
		// ファイルヘッダを記憶する領域を確保し、初期化する

		file->extend(_TimeStamp::PageSize * _TimeStamp::MultiplexCount);

		//【注意】	store時にAllocateでfixしているので、
		//			ここではAllocateでfixしない

		// 初期値として 0 を永続化する

		TimeStamp(0).store(path);

	} catch (...) {

		file->destroy();
		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Trans::TimeStamp::mountFile --
//		タイムスタンプ値を永続化するファイルをマウントする
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			タイムスタンプ値を永続化する
//			バッファファイルの実体である OS ファイルの絶対パス名
//		bool				readOnly
//			true
//				マウントしようとしているファイルは読取専用である
//			false
//				マウントしようとしているファイルは読取専用でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
TimeStamp::mountFile(const Os::Path& path, bool readOnly)
{
	// タイムスタンプ値を永続化するバッファファイルの記述子を得て、
	// バッファファイルをマウントする

	Buffer::AutoPool pool(
		Buffer::Pool::attach(Buffer::Pool::Category::Normal));
	Buffer::AutoFile file(
		Buffer::File::attach(
			*pool, path, _TimeStamp::PageSize, false, readOnly, false));
	file->mount(
#ifndef OBSOLETE_OLIVE_SW2

			//【注意】	下記のコードは Olive-SW2 が廃止された時点で削除できる

			false
#else
			true
#endif
			);
}

//	FUNCTION private
//	Trans::TimeStamp::destroyFile --
//		タイムスタンプ値を永続化するファイルを破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			タイムスタンプ値を永続化する
//			バッファファイルの実体である OS ファイルの絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
TimeStamp::destroyFile(const Os::Path& path)
{
	// タイムスタンプ値を永続化するバッファファイルの記述子を得て、
	// バッファファイルを破棄する

	Buffer::AutoPool pool(
		Buffer::Pool::attach(Buffer::Pool::Category::Normal));
	Buffer::AutoFile file(
		Buffer::File::attach(
			*pool, path, _TimeStamp::PageSize, true, false, false));
	file->destroy();
}

//	FUNCTION private
//	Trans::TimeStamp::unmountFile --
//		タイムスタンプ値を永続化するファイルをアンマウントする
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			タイムスタンプ値を永続化する
//			バッファファイルの実体である OS ファイルの絶対パス名
//		bool				readOnly
//			true
//				アンマウントしようとしているファイルは読取専用である
//			false
//				アンマウントしようとしているファイルは読取専用でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
TimeStamp::unmountFile(const Os::Path& path, bool readOnly)
{
	// タイムスタンプ値を永続化するバッファファイルの記述子を得て、
	// バッファファイルをアンマウントする

	Buffer::AutoPool pool(
		Buffer::Pool::attach(Buffer::Pool::Category::Normal));
	Buffer::AutoFile file(
		Buffer::File::attach(
			*pool, path, _TimeStamp::PageSize, true, readOnly, false));
	file->unmount();
}

//	FUNCTION private
//	Trans::TimeStamp::renameFile --
//		タイムスタンプ値を永続化するファイルの名前を変更する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			名前を変更するタイムスタンプ値を永続化する
//			バッファファイルの実体である OS ファイルの絶対パス名
//		Os::Path&			newPath
//			変更後の絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
TimeStamp::renameFile(const Os::Path& path, const Os::Path& newPath)
{
	// タイムスタンプ値を永続化するバッファファイルの記述子を得て、
	// バッファファイルを新しい名前に変更する

	Buffer::AutoPool pool(
		Buffer::Pool::attach(Buffer::Pool::Category::Normal));
	Buffer::AutoFile file(
		Buffer::File::attach(
			*pool, path, _TimeStamp::PageSize, true, false, false));
	file->rename(newPath);
}

//	FUNCTION private
//	Trans::TimeStamp::store -- タイムスタンプ値をファイルに永続化する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			タイムスタンプ値を永続化する
//			バッファファイルの実体である OS ファイルの絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
TimeStamp::store(const Os::Path& path) const
{
	// タイムスタンプ値を永続化するバッファファイルの記述子を得る

	Buffer::AutoPool pool(
		Buffer::Pool::attach(Buffer::Pool::Category::Normal));
	Buffer::AutoFile file(
		Buffer::File::attach(
			*pool, path, _TimeStamp::PageSize, true, false, false));

#ifndef OBSOLETE_OLIVE_SW2

	//【注意】	下記のコードは Olive-SW2 が廃止された時点で削除できる

	if (!file->isAccessible(true))

		// Olive-SW2 で作成されたデータベースをアップグレードしたときは、
		// タイムスタンプファイルは生成されていないので、まず生成する

		createFile(path);
#endif
	unsigned int i = 0;
	do {
		// タイムスタンプ値を記録しているバッファページをフィックスする
		//
		//【注意】	読み出し時には、多重化されているページのうち、１ページ
		//			読めればOKである。しかし、書き出し時にFixModeをWriteで、
		//			fixすると、現在のページを読み込んでしまい、結果として、
		//			多重化されているページすべてが正しく読み込める必要があった
		//			
		//			そこで、FixModeをWriteからAllocateに変更し、
		//			現在のページの内容は読み込まずに上書きするように変更した

		Buffer::Memory memory(
			Buffer::Page::fix(*file, _TimeStamp::PageSize * i,
							  Buffer::Page::FixMode::Allocate));

		// バッファページ上のファイルヘッダ中のタイムスタンプ値を更新する

		_TimeStamp::_FileHeader& header =
			*static_cast<_TimeStamp::_FileHeader*>(static_cast<void*>(memory));

		Os::Memory::reset(&header, sizeof(header));

		header._versionNumber = _TimeStamp::_VersionNumber::Current;
		header._value = _value.full;

		// 同期的にアンフィックスする
		//
		//【注意】	多重化しているファイルヘッダを、
		//			ひとつひとつ完全にディスクへ書き出してから、
		//			次のファイルヘッダをディスクへ書き出す
		//
		//			そのため、少なくともひとつは整合性の保障された
		//			ファイルヘッダが残る(最新でない可能性はある)

		memory.unfix(true, false);

	} while (++i < _TimeStamp::MultiplexCount) ;
}

//	FUNCTION private
//	Trans::TimeStamp::load -- タイムスタンプ値をファイルから読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			タイムスタンプ値を永続化する
//			バッファファイルの実体である OS ファイルの絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
TimeStamp::load(const Os::Path& path)
{
	// タイムスタンプ値を永続化するバッファファイルの記述子を得る

	Buffer::AutoPool pool(
		Buffer::Pool::attach(Buffer::Pool::Category::Normal));
	Buffer::AutoFile file(
		Buffer::File::attach(
			*pool, path, _TimeStamp::PageSize, true, false, false));

#ifndef OBSOLETE_OLIVE_SW2

	//【注意】	下記のコードは Olive-SW2 が廃止された時点で削除できる

	if (!file->isAccessible(true)) {

		// Olive-SW2 で作成されたデータベースをアップグレードしたときは、
		// タイムスタンプファイルは生成されていないので、
		// 不正なタイムスタンプ値を得たことにする

		_value.full = IllegalTimeStamp;
		return;
	}
#endif
	unsigned int i = 0;
	do {
		try {
			// 多重化されているファイルヘッダを読み出す

			const Buffer::Memory& memory =
				Buffer::Page::fix(*file, _TimeStamp::PageSize * i,
								  Buffer::Page::FixMode::ReadOnly);
			const _TimeStamp::_FileHeader& header =
				*static_cast<const _TimeStamp::_FileHeader*>(
					static_cast<const void*>(memory));

#ifdef SYD_C_GCC
			// バージョンをチェックする

			if (header._versionNumber == _TimeStamp::_VersionNumber::First)
			{
				// 古いバージョンなので、古い構造体で読み直す

				const _TimeStamp::_FileHeader_First& old =
					*static_cast<const _TimeStamp::_FileHeader_First*>(
						static_cast<const void*>(memory));

				// 値を取得する
				
				_value.full = old._value;
			}
			else
			{
#endif
				// 正常なファイルヘッダを読み出したので、値を取得する

				_value.full = header._value;
#ifdef SYD_C_GCC
			}
#endif
			return;

		} catch (Exception::BadDataPage&) {

			// このファイルヘッダは正しく記録されていないので、
			// 次のファイルヘッダを読み出せるか調べてみる

			continue;
		}
	} while (++i < _TimeStamp::MultiplexCount) ;

	// 多重化されているファイルヘッダがすべて正しく記録されていなかった

	_SYDNEY_THROW1(Exception::TimeStampFileCorrupted, path);
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
