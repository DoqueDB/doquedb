// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.h -- ファイル、ディレクトリ関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2008, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_OS_FILE_H
#define	__TRMEISTER_OS_FILE_H

#if !defined(SYD_OS_WINDOWS) && !defined(SYD_OS_POSIX)
#error require #include "SyDefault.h"
#endif

#ifdef SYD_OS_WINDOWS
// #include <windows.h>
#endif
#ifdef SYD_OS_POSIX
// #include <sys/uio.h>
#endif

#include "Os/Module.h"
#include "Os/Memory.h"
#include "Os/Path.h"

#include "ModTypes.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

class AsyncStatus;
class Directory;

//	CLASS
//	Os::File -- ファイルを表すクラス
//
//	NOTES
//		POSIX のように、ファイルといっても
//		ディレクトリーやテープデバイスを操作できるわけでない

class File
{
	friend class AsyncStatus;
	friend class Directory;
public:
	//	TYPEDEF
	//	Os::File::Offset -- ファイル内のオフセット値を表す型
	//
	//	NOTES

	typedef	ModFileOffset	Offset;

	//	TYPEDEF
	//	Os::File::Size -- ファイルのサイズ値を表す型
	//
	//	NOTES

	typedef	ModFileSize		Size;

	//	CLASS
	//	Os::File::OpenMode -- オープンモードを表すクラス
	//
	//	NOTES

	struct OpenMode
	{
		//	ENUM
		//	Os::File::OpenMode::Value -- オープンモードを表す値の型
		//
		//	NOTES

		typedef	unsigned int	Value;
		enum
		{
			// なし
			None =			0x000,

			// 読み取り可
			Read =			0x001,
			// 書き込み可
			Write =			0x002,

			// 必ずファイルの末尾に書き込まれる
			Append =		0x004,

			// データを書き込むまで、呼び出しから返らない
			WriteDataSync =	0x010,
			// I ノードを書き込むまで、呼び出しから返らない
			WriteInodeSync = 0x020,
			// 書き込みが完全に終わるまで、読み取りを行わない
			ReadSync =		0x040,

			// 書き込みをバッファリングしない
			WriteThrough =	WriteDataSync | WriteInodeSync,
			// 読み書きをバッファリングしない
			NoBuffering =	WriteDataSync | WriteInodeSync | ReadSync,

			// ファイルがなければ、作る
			Create =		0x100,
			// ファイルがあれば、サイズを 0 にする
			Truncate =		0x200,
			// すでにファイルがあると、作れない
			Exclusive =		0x400,

			// アクセスマスク
			MaskAccess =	0x003,
			// 同期マスク
			MaskSync =		0x070,
			// バッファリングマスク
			MaskBuffering =	MaskSync,
			// 作成マスク
			MaskCreate =	0x700,
			// マスク
			Mask =			0x7ff
		};
	};

	//	CLASS
	//	Os::File::Permission --
	//		ファイルに対して許可される操作を表すクラス
	//
	//	NOTES
	
	struct Permission
	{
		//	ENUM
		//	Os::File::Permission::Value --
		//		ファイルに対して許可される操作を表す値の型
		//
		//	NOTES

		typedef	unsigned int	Value;
		enum
		{
			// なし
			None =			0000000,

			// 所有者が読み取り可
			OwnerRead =		0000400,
			// 所有者が書き込み可
			OwnerWrite =	0000200,
			// 所有者が実行可
			OwnerExecute =	0000100,

			// グループが読み取り可
			GroupRead =		0000040,
			// グループが書き込み可
			GroupWrite =	0000020,
			// グループが実行可
			GroupExecute =	0000010,

			// その他のユーザーが読み取り可
			OtherRead =		0000004,
			// その他のユーザーが書き込み可
			OtherWrite =	0000002,
			// その他のユーザーが実行可
			OtherExecute =	0000001,

			// 読み取りマスク
			MaskRead =		0000444,
			// 書き込みマスク
			MaskWrite =		0000222,
			// 実行マスク
			MaskExecute =	0000111,

			// 所有者マスク
			MaskOwner =		0000700,
			// グループマスク
			MaskGroup =		0000070,
			// その他のユーザーマスク
			MaskOther =		0000007,

			// マスク
			Mask =			0000777
		};
	};

	//	CLASS
	//	Os::File::SeekWhence --
	//		シーク位置として指定された値の解釈の方法を表すクラス
	//
	//	NOTES

	struct SeekWhence
	{
		//	ENUM
		//	Os::File::SeekWhence::Value --
		//		シーク位置として指定された値の解釈の方法を表す値の列挙型
		//
		//	NOTES

		enum Value
		{
			// ファイルの先頭から
			Set =			0,
			// 同上
			Begin =			Set,
			// 現在位置から
			Current,
			// ファイルの末尾から
			End,
			// 値の数
			Count
		};
	};

	//	CLASS
	//	Os::File::AccessMode --
	//		どのようなアクセスの可能性を確認するかを表すクラス
	//
	//	NOTES

	struct AccessMode
	{
		//	ENUM
		//	Os::File::AccessMode::Value --
		//		どのようなアクセスの可能性を確認するかを表す値の型
		//
		//	NOTES

		typedef unsigned int	Value;
		enum
		{
			// 存在確認のみを行う
			File =			0x0,
			// 実行する権利
			Execute =		0x1,
			// 書き込む権利
			Write =			0x2,
			// 読み取る権利
			Read =			0x4
		};
	};

	//	CLASS
	//	Os::File::IOBuffer --
	//		ファイルから読み取ったデータを格納したり、
	//		書き込むデータを格納するバッファを表すクラス
	//
	//	NOTES

	class IOBuffer
	{
		friend class File;
	public:
		// デフォルトコンストラクター
		IOBuffer();
		// コンストラクター
		explicit IOBuffer(void* p);
		// デストラクター
		~IOBuffer();

		// = 演算子
		IOBuffer&
		operator =(void* p);
		// void* へのキャスト演算子
		operator void*();
		// const void* へのキャスト演算子
		operator const void*() const;
	private:
#ifdef SYD_OS_WINDOWS
		// 入出力するデータを格納する領域の先頭アドレス
		FILE_SEGMENT_ELEMENT _ptr;
#endif
#ifdef SYD_OS_POSIX
		// 入出力するデータを格納する領域に関する情報を管理する構造体
		mutable struct iovec	_iovec;
#endif
	};

	// デフォルトコンストラクター
	File();
	// コンストラクター
	File(const Path& path);
	// デストラクター
	~File();
#ifdef OBSOLETE
	// ファイルを生成し、オープンする
	void
	create(Permission::Value permission =
		   Permission::OwnerRead | Permission::OwnerWrite);
	void
	create(const Path& path, Permission::Value permission =
		   Permission::OwnerRead | Permission::OwnerWrite);
#endif
	// ファイルをオープンする
	SYD_OS_FUNCTION
	void
	open(OpenMode::Value mode, Permission::Value permission =
		 Permission::OwnerRead | Permission::OwnerWrite);
#ifdef OBSOLETE
	void
	open(const Path& path,
		 OpenMode::Value mode, Permission::Value permission =
		 Permission::OwnerRead | Permission::OwnerWrite);
#endif
	// ファイルをクローズする
	SYD_OS_FUNCTION
	void
	close();
	// ファイルがオープンされているか
	bool
	isOpened() const;

	// ファイルの入出力位置を移動する
	SYD_OS_FUNCTION
	Offset
	seek(Offset offset, SeekWhence::Value whence);

	// ファイルからバッファ領域へ読み取る
#ifdef SYD_OS_POSIX // Linux版のみで使用される
	SYD_OS_FUNCTION
	Memory::Size
	read(IOBuffer& buf, Memory::Size size);
#endif
	SYD_OS_FUNCTION
	Memory::Size
	read(IOBuffer& buf, Memory::Size size, Offset offset);
#ifdef YET
	SYD_OS_FUNCTION
	Memory::Size
	read(IOBuffer& buf, Memory::Size size, Offset offset, AsyncStatus& stat);
#endif
	SYD_OS_FUNCTION
	Memory::Size
	read(IOBuffer bufs[], Memory::Size size, unsigned int count);
	SYD_OS_FUNCTION
	Memory::Size
	read(IOBuffer bufs[], Memory::Size size,
		 unsigned int count, Offset offset);
#ifdef YET
	SYD_OS_FUNCTION
	Memory::Size
	read(IOBuffer bufs[], Memory::Size size,
		 unsigned int count, Offset offset, AsyncStatus& stat);
#endif
	// ファイルへバッファ領域を書き込む
	SYD_OS_FUNCTION
	void
	write(const IOBuffer& buf, Memory::Size size);
	SYD_OS_FUNCTION
	void
	write(const IOBuffer& buf, Memory::Size size, Offset offset);
#ifdef YET
	SYD_OS_FUNCTION
	void
	write(const IOBuffer& buf, Memory::Size size,
		  Offset offset, AsyncStatus& stat);
#endif
	SYD_OS_FUNCTION
	void
	write(const IOBuffer bufs[], Memory::Size size, unsigned int count);
	SYD_OS_FUNCTION
	void
	write(const IOBuffer bufs[], Memory::Size size,
		  unsigned int count, Offset offset);
#ifdef YET
	SYD_OS_FUNCTION
	void
	write(const IOBuffer bufs[], Memory::Size size,
		  unsigned int count, Offset offset, AsyncStatus& stat);
#endif
	// ファイルへフラッシュする
	SYD_OS_FUNCTION
	void
	flush();

	// ファイルを削除する
	void
	remove();
	SYD_OS_FUNCTION
	static void
	remove(const Path& path);

	// ファイルサイズを変更する
	SYD_OS_FUNCTION
	void
	truncate(Size length = 0);
	SYD_OS_FUNCTION
	static void
	truncate(const Path& path, Size length = 0);
#ifdef OBSOLETE
	// ファイルをコピーする
	void
	copy(const Path& path, bool force = false) const;
#endif
#ifdef SYD_OS_POSIX // Linux版のみで使用される
	SYD_OS_FUNCTION
	static void
	copy(const Path& srcPath, const Path& dstPath, bool force = false);
#endif

	// ファイル名を変更する
	void
	rename(const Path& path, bool force = false);
	SYD_OS_FUNCTION
	static void
	rename(const Path& srcPath, const Path& dstPath, bool force = false);

#ifdef SYD_OS_POSIX // Linux版のみで使用される
	// 許可モードを変更する
	SYD_OS_FUNCTION
	void
	chmod(Permission::Value mode);
#endif
#ifdef OBSOLETE
	SYD_OS_FUNCTION
	static void
	chmod(const Path& path, Permission::Value mode);
#endif

	// 操作を行う権利の有無を調べる
	bool
	access(AccessMode::Value mode) const;
	SYD_OS_FUNCTION
	static bool
	access(const Path& path, AccessMode::Value mode);

	// ファイルをロックする
	SYD_OS_FUNCTION
	void
	lock();
	// ファイルをロックしてみる
	SYD_OS_FUNCTION
	bool
	trylock();
	// ファイルをアンロックする
	SYD_OS_FUNCTION
	void
	unlock();

	// ファイルの絶対パス名を得る
	const Path&
	getPath() const;
	// ファイルの絶対パス名を設定する
	SYD_OS_FUNCTION
	void
	setPath(const Path& path);
	// ファイルサイズを得る
	SYD_OS_FUNCTION
	Size
	getSize() const;
	SYD_OS_FUNCTION
	static Size
	getSize(const Path& path);

	// ファイルサイズの最大値を得る
	SYD_OS_FUNCTION
	static Size
	getSizeMax();
	// ファイルサイズの最小値を得る
	static Size
	getSizeMin();
	// ファイルのオフセットの最大値を得る
	static Offset
	getOffsetMax();
	// ファイルのオフセットの最小値を得る
	static Offset
	getOffsetMin();

private:
#ifdef SYD_OS_WINDOWS
	// 許可モードに合わせてファイル属性を変更する
	static void
	chmodAttribute(const Path& path, Permission::Value mode);
#ifdef SYD_OS_WINNT4_0
#ifdef OBSOLETE
	// 許可モードに合わせてセキュリティを変更する
	static void
	chmodSecurity(const Path& path, Permission::Value mode);
#endif
#endif
#endif
	// 絶対パス名
	Path					_path;
#ifdef SYD_OS_WINDOWS
	// ファイルハンドル
	HANDLE					_handle;
#endif
#ifdef SYD_OS_POSIX
	// ファイルディスクリプター
	int						_descriptor;
#endif
	// オープンモード
	OpenMode::Value			_openMode;
#ifdef SYD_OS_WINDOWS
	// ロックされているか
	bool					_locked;
#endif
};

//	CLASS
//	Os::Directory -- ディレクトリを表すクラス
//
//	NOTES
//		POSIX のように、ディレクトリといっても
//		ファイルやテープデバイスを操作できるわけでない

class Directory
{
public:
	//	TYPEDEF
	//	Os::Directory::Permission -- 
	//		ディレクトリに対して許可される操作を表す型
	//	NOTES

	typedef	File::Permission	Permission;

	//	TYPEDEF
	//	Os::Directory::AccessMode --
	//		どのようなアクセスの可能性を確認するかを表す型
	//
	//	NOTES

	typedef	File::AccessMode	AccessMode;

	// ディレクトリを生成する
	SYD_OS_FUNCTION
	static void
	create(const Path& path, Permission::Value permission =
		   Permission::MaskOwner, bool recursive = false);
	// ディレクトリを削除する
	SYD_OS_FUNCTION
	static void
	remove(const Path& path);
#ifdef OBSOLETE
	// ディレクトリ名を変更する
	static void
	rename(const Path& srcPath, const Path& dstPath, bool force = false);
	// 許可モードを変更する
	static void
	chmod(const Path& path, Permission::Value mode);
#endif
	// 操作を行う権利の有無を調べる
	static bool
	access(const Path& path, AccessMode::Value mode);
	// ディレクトリの内容をフラッシュする
	SYD_OS_FUNCTION
	static void
	flush(const Path& path);
};

//	FUNCTION public
//	Os::File::File -- ファイルを表すクラスのデフォルトコンストラクター
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
File::File()
#ifdef SYD_OS_WINDOWS
	: _handle(INVALID_HANDLE_VALUE),
	  _openMode(OpenMode::None),
	  _locked(false)
#endif
#ifdef SYD_OS_POSIX
	: _descriptor(-1),
	  _openMode(OpenMode::None)
#endif
{}

//	FUNCTION public
//	Os::File::File -- ファイルを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			操作するファイルの絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
File::File(const Path& path)
	: _path(path),
#ifdef SYD_OS_WINDOWS
	  _handle(INVALID_HANDLE_VALUE),
	  _openMode(OpenMode::None),
	  _locked(false)
#endif
#ifdef SYD_OS_POSIX
	  _descriptor(-1),
	  _openMode(OpenMode::None)
#endif
{}

//	FUNCTION public
//	Os::File::~File -- ファイルを表すクラスのデストラクター
//
//	NOTES
//		ファイルがオープンされたままであれば、まず、クローズされる
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
File::~File()
{
	if (isOpened())

		// ファイルがオープンされているので、クローズしておく

		close();
}

#ifdef OBSOLETE
//	FUNCTION public
//	Os::File::create -- ファイルを生成し、オープンする
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			指定されたとき
//				指定された絶対パス名のファイルを、
//				this の生成時に指定されている絶対パス名の
//				ファイルの代わりに生成し、オープンする
//			指定されないとき
//				this の生成時に指定されている絶対パス名の
//				ファイルを生成し、オープンする
//		Os::File::Permission::Value	permission
//			指定されたとき
//				生成するファイルに対して許可される操作を表す値で、
//				Os::File::Permission::Value の論理和を指定する
//			指定されないとき
//				Permission::OwnerRead | Permission::OwnerWrite が
//				指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
File::create(Permission::Value permission)
{
	open(OpenMode::Write | OpenMode::Create | OpenMode::Truncate,
		 permission);
}

inline
void
File::create(const Path& path, Permission::Value permission)
{
	open(path,
		 OpenMode::Write | OpenMode::Create | OpenMode::Truncate,
		 permission);
}

//	FUNCTION public
//	Os::File::open -- ファイルをオープンする
//
//	NOTES
//		オープンされたファイルは Os::File::close でクローズできる
//
//		mode に Os::File::OpenMode::Create を与えると、
//		ファイルを生成してから、オープンすることができる
//
//	ARGUMENTS
//		Os::Path&			path
//			オープンするファイルの絶対パス名
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
//		なし
//
//	EXCEPTIONS

inline
void
File::open(const Path& path,
		   OpenMode::Value mode, Permission::Value permission)
{
	// 指定された絶対パス名を記憶し、そのファイルをオープンする

	setPath(path);
	open(mode, permission);
}
#endif

//	FUNCTION public
//	Os::File::isOpened -- ファイルがオープンされているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			オープンされている
//		false
//			オープンされていない
//
//	EXCEPTIONS
//		なし

inline
bool
File::isOpened() const
{
#ifdef SYD_OS_WINDOWS
	return _handle != INVALID_HANDLE_VALUE;
#endif
#ifdef SYD_OS_POSIX
	return _descriptor >= 0;
#endif
}

//	FUNCTION public
//	Os::File::remove -- ファイルを削除する
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
File::remove()
{
	if (isOpened())

		// ファイルがオープンされたままであれば、クローズしておく

		close();

	// 自分自身の表すファイルを削除する

	File::remove(getPath());
}

#ifdef OBSOLETE
//	FUNCTION public
//	Os::File::copy -- 指定されたファイルへコピーする
//
//	NOTES
//		コピーするファイルはオープンされている必要はない
//
//		Os::Path&			path
//			コピーによって生成されるファイルの絶対パス名で、
//			存在するファイルのものか、
//			存在しないがその親ディレクトリが存在する必要がある
//		bool				force
//			true
//				コピーによって生成されるファイルがすでに存在するとき、
//				そのファイルを上書きする
//			false または指定されないとき
//				コピーによって生成されるファイルがすでに存在するとき、
//				コピーできない
//			
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
File::copy(const Path& path, bool force) const
{
	// 自分自身を指定されたものにコピーする

	File::copy(getPath(), path, force);
}
#endif

//	FUNCTION public
//	Os::File::rename -- 指定された名前に変更する
//
//	NOTES
//		名前を変更するファイルはオープンされている必要はない
//
//	ARGUMENTS
//		Os::Path&			path
//			新しい絶対パス名で、存在するファイルのものか、
//			存在しないがその親ディレクトリが存在する必要がある
//		bool				force
//			true
//				新しい名前のファイルが存在するとき、そのファイルを上書きする
//			false または指定されないとき
//				新しい名前のファイルが存在するとき、名前を変更できない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
File::rename(const Path& path, bool force)
{
	// 自分自身の名前を指定されたものに変更する

	File::rename(getPath(), path, force);

	// 自分が記憶している変更前の絶対パス名を新しいものに変更する
	//
	//【注意】	ここで失敗すると元に戻せない

	_path = path;
}

//	FUNCTION public
//	Os::File::access --
//		ファイルにある操作を行う権利を持っているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Os::File::AccessMode::Value	mode
//			調べる権利を表す値で、
//			Os::File::AccessMode の論理和を指定する
//
//	RETURN
//		true
//			指定された操作を行う権利が存在する
//		false
//			存在しない
//
//	EXCEPTIONS

inline
bool
File::access(AccessMode::Value mode) const
{
	return File::access(getPath(), mode);
}

//	FUNCTION public
//	Os::File::getPath -- ファイルの絶対パス名を得る
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
//		なし

inline
const Path&
File::getPath() const
{
	return _path;
}

//	FUNCTION public
//	Os::File::getSizeMin -- システムが許可するファイルサイズの最小値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイルサイズの最小値(B 単位)
//
//	EXCEPTIONS
//		なし

// static
inline
File::Size
File::getSizeMin()
{
	return 0;
}

//	FUNCTION public
//	Os::File::getOffsetMax --
//		システムが許可するファイルオフセットの最大値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイルオフセットの最大値(B 単位)
//
//	EXCEPTIONS

// static
inline
File::Offset
File::getOffsetMax()
{
	// システムが許可するファイルサイズの上限を求めて、
	// それをそのまま返す

	return File::getSizeMax();
}

//	FUNCTION public
//	Os::File::getOffsetMin --
//		システムが許可するファイルオフセットの最小値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイルオフセットの最小値(B 単位)
//
//	EXCEPTIONS

// static
inline
File::Offset
File::getOffsetMin()
{
	// システムが許可するファイルサイズの上限を求めて
	// それを負数にして返す

	return -static_cast<File::Offset>(File::getSizeMax());
}

//	FUNCTION public
//	Os::File::IOBuffer::IOBuffer --
//		ファイルから読み取ったデータを格納したり、
//		書き込むデータを格納するバッファを表すクラスの
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
File::IOBuffer::IOBuffer()
{
#ifdef SYD_OS_WINDOWS
	_ptr.Alignment = 0;
#endif
}

//	FUNCTION public
//	OS::File::IOBuffer::IOBuffer --
//		ファイルから読み取ったデータを格納したり、
//		書き込むデータを格納するバッファを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		void*				p
//			ファイルから読み取ったデータを格納したり、
//			書き込むデータを格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
File::IOBuffer::IOBuffer(void* p)
{
#ifdef SYD_OS_WINDOWS
	_ptr.Alignment = 0;
#endif
	(void) operator =(p);
}

//	FUNCTION public
//	Os::File::IOBuffer::~IOBuffer --
//		ファイルから読み取ったデータを格納したり、
//		書き込むデータを格納するバッファを表すクラスのデストラクター
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
File::IOBuffer::~IOBuffer()
{}

//	FUNCTION public
//	Os::File::IOBuffer::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		void*				p
//			ファイルから読み取ったデータを格納したり、
//			書き込むデータを格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
File::IOBuffer&
File::IOBuffer::operator =(void* p)
{
#ifdef SYD_OS_WINDOWS
	_ptr.Buffer = p;
#endif
#ifdef SYD_OS_POSIX
#ifdef SYD_OS_LINUX
	_iovec.iov_base = p;
#endif
#ifdef SYD_OS_SOLARIS
	_iovec.iov_base = static_cast<caddr_t>(p);
#endif
#endif
	return *this;
}

//	FUNCTION public
//	Os::File::IOBuffer::operator void* -- void* へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ファイルから読み取ったデータを格納したり、
//		書き込むデータを格納する領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

inline
File::IOBuffer::operator void*()
{
#ifdef SYD_OS_WINDOWS
	return _ptr.Buffer;
#endif
#ifdef SYD_OS_POSIX
	return _iovec.iov_base;
#endif
}

//	FUNCTION public
//	Os::File::IOBuffer::operator const void* -- const void* へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ファイルから読み取ったデータを格納したり、
//		書き込むデータを格納する領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

inline
File::IOBuffer::operator const void*() const
{
#ifdef SYD_OS_WINDOWS
	return _ptr.Buffer;
#endif
#ifdef SYD_OS_POSIX
	return _iovec.iov_base;
#endif
}

#ifdef OBSOLETE
//	FUNCTION public
//	Os::Directory::rename --
//		指定されたディレクトリの名前を指定されたものに変更する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			srcPath
//			名前を変更するディレクトリの絶対パス名で、
//			存在するディレクトリのものである必要がある
//		Os::Path&			dstPath
//			新しい絶対パス名で、存在するディレクトリのものか、
//			存在しないがその親ディレクトリが存在する必要がある
//		bool				force
//			true
//				新しい名前のディレクトリが存在するとき、
//				そのファイルを上書きする
//			false または指定されないとき
//				新しい名前のディレクトリが存在するとき、名前を変更できない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			変更するまたは変更後のディレクトリの絶対パス名が指定されていない

// static
inline
void
Directory::rename(const Path& srcPath, const Path& dstPath, bool force)
{
	File::rename(srcPath, dstPath, force);
}

//	FUNCTION public
//	Os::Directory::chmod -- 指定されたディレクトリの許可モードを変更する
//
//	NOTES
//		WINDOWS のディレクトリには許可モードという概念は存在しないので、
//		指定された許可モードにほぼ同義のファイル属性とセキュリティを設定する
//
//	ARGUMENTS
//		Os::Path&			path
//			許可モードを変更するディレクトリの絶対パス名
//		Os::Directory::Permission::Value	mode
//			ディレクトリの変更後の許可モードを表す値で、
//			Os::Directory::Permission::Value の論理和を指定する
//
//	RERURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			許可モードを変更するディレクトリの絶対パス名が指定されていない

// static
inline
void
Directory::chmod(const Path& path, Permission::Value mode)
{
	File::chmod(path, mode);
}
#endif

//	FUNCTION public
//	Os::Directory::access --
//		指定されたディレクトリにある操作を行う権利を持っているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			権利があるか調べるディレクトリの絶対パス名
//		Os::Directory::AccessMode::Value	mode
//			調べる権利を表す値で、
//			Os::Directory::AccessMode の論理和を指定する
//
//	RETURN
//		true
//			指定された操作を行う権利が存在する
//		false
//			存在しない
//
//	EXCEPTIONS
//		Exception::BadArgument
//			権利があるか調べるディレクトリの絶対パス名が指定されていない

// static
inline
bool
Directory::access(const Path& path, AccessMode::Value mode)
{
	return File::access(path, mode);
}

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_FILE_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2008, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
