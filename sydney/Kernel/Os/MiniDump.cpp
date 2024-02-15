// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiniDump.cpp -- ミニダンプ関連の関数定義
// 
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Os";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

extern "C"
{
#ifdef SYD_OS_WINDOWS
#include <windows.h>
#include <assert.h>
#include <DbgHelp.h>
#endif
}

#include "Os/Assert.h"
#include "Os/Manager.h"
#ifdef SYD_OS_WINDOWS
#include "Os/File.h"
#include "Os/Library.h"
#include "Os/MiniDump.h"
#include "Os/Process.h"
#include "Os/Thread.h"
#include "Os/Unicode.h"

#include "ModError.h"
#include "ModIos.h"
#include "ModTime.h"
#include "ModUnicodeOstrStream.h"
#endif

_TRMEISTER_USING
_TRMEISTER_OS_USING

namespace
{
#ifdef SYD_OS_WINDOWS
namespace _Literal
{

	// ファイル名関係

	const Path			DbgHelp_Dll("DBGHELP.DLL");
	const Path			_Dmp(".DMP");

	#define	U(literal)	const UnicodeString	literal(#literal);

	// 関数名関係

	U(MiniDumpWriteDump);

	#undef U
}

namespace _MiniDumper
{
	// ミニダンプを生成するディレクトリの絶対パス名
	Path			_parent;
	// ミニダンプファイルの許可モード
	File::Permission::Value	_permission;
}

namespace _MiniDump
{
	// ミニダンプを生成する関数へのポインタ

	typedef BOOL (WINAPI* WriteFunc)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, MINIDUMP_EXCEPTION_INFORMATION*, MINIDUMP_USER_STREAM_INFORMATION*, MINIDUMP_CALLBACK_INFORMATION*);

	WriteFunc			_createFunc = 0;

	// あるファイルにミニダンプを実際に記録する
	bool
	create(HANDLE handle, EXCEPTION_POINTERS* ep);
}

//	FUNCTION
//	$$$::_MiniDump::create -- あるファイルにミニダンプを実際に記録する
//
//	NOTES
//
//	ARGUMENTS
//		HANDLE			handle
//			ミニダンプを記録するファイルのハンドル
//		EXCEPTION_POINTERS*	ep
//			記録するミニダンプに含める例外情報を格納する領域の先頭アドレス
//
//	RETURN
//		true
//			ミニダンプを記録した
//		false
//			ミニダンプを記録しなかった
//
//	EXCEPTIONS
//		なし

bool
_MiniDump::create(HANDLE handle, EXCEPTION_POINTERS* ep)
{
	if (!_createFunc)
		try {
			// ミニダンプを生成するためのシステムが提供する DLL をロードする

			Library::load(_Literal::DbgHelp_Dll);

			// ロードした DLL 中のミニダンプを生成するための
			// 関数のアドレスを求める

			_createFunc = static_cast<_MiniDump::WriteFunc>(
				Library::getFunction(
					_Literal::DbgHelp_Dll, _Literal::MiniDumpWriteDump));

		} catch (...) {

			// エラーは無視する
		}

	if (_createFunc) {

		// 求めた関数を使ってミニダンプを生成する

		MINIDUMP_EXCEPTION_INFORMATION* p = 0;
		MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;

		if (ep) {
			p = &exceptionInfo;
			exceptionInfo.ThreadId = Thread::self();
			exceptionInfo.ExceptionPointers = ep;
			exceptionInfo.ClientPointers = false;
		}

		return _createFunc(::GetCurrentProcess(), Process::self(), handle,
						   MiniDumpWithDataSegs, p, 0, 0);
	}

	return false;
}
#endif
}

//	FUNCTION private
//	Os::Manager::MiniDumper::initialize --
//		マネージャーの初期化のうち、ミニダンプの生成関連のものを行う
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			ミニダンプを記録するファイルの親ディレクトリの絶対パス名
//		Os::File::Permission::Value	permission
//			ミニダンプを記録するファイルの許可モード
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Manager::MiniDumper::initialize(
	const Path& path, File::Permission::Value permission)
{
#ifdef SYD_OS_WINDOWS
	if (path.getLength()) {

		// 絶対パス名として空文字列が指定されていないとき、
		// ミニダンプを必要に応じて生成できるようにする

		_MiniDumper::_parent = path;
		_MiniDumper::_permission = permission;

		// 未処理の例外を処理するための関数として、
		// ミニダンプを生成する関数を登録する
		//
		//【注意】	以前に登録されている関数があれば、上書きされてしまう

		(void) ::SetUnhandledExceptionFilter(
			Manager::MiniDumper::topLevelExceptionFilter);
	}
#endif
}

//	FUNCTION private
//	Os::Manager::MiniDumper::terminate --
//		マネージャーの後処理のうち、ミニダンプの生成関連のものを行う
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
Manager::MiniDumper::terminate()
{
#ifdef SYD_OS_WINDOWS
	_MiniDumper::_parent.clear();
	_MiniDump::_createFunc = 0;
#endif
}

//	FUNCTION public
//	Os::Manager::MiniDumper::execute --
//		初期化時に指定されたディレクトリの下に
//		適当な名前の例外情報を含まないミニダンプを生成する
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
Manager::MiniDumper::execute()
{
#ifdef SYD_OS_WINDOWS
	execute(0);
#endif
}

//	FUNCTION public
//	Os::Manager::MiniDumper::execute --
//		初期化時に指定されたディレクトリの下に
//		指定された名前の例外情報を含まないミニダンプを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Os::UnicodeString&	name
//			ミニダンプを記録するファイルの名前
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
Manager::MiniDumper::execute(const UnicodeString& name)
{
#ifdef SYD_OS_WINDOWS
	execute(name, 0);
#endif
}

#ifdef SYD_OS_WINDOWS
//	FUNCTION private
//	Os::Manager::MiniDumper::execute --
//		初期化時に指定されたディレクトリの下に
//		適当な名前で例外情報を含むミニダンプを生成する
//
//	NOTES
//
//	ARGUMENTS
//		EXCEPTION_POINTERS*	ep
//			発生し、未処理の例外に関する情報を格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
Manager::MiniDumper::execute(EXCEPTION_POINTERS* ep)
{
	// ミニダンプを記録するファイルの絶対パス名を生成する
	//
	//【注意】	ファイル名の形式は
	//
	//			YYMMDDNN.DMP
	//
	//			で、YY は年、MM は月、DD は日、
	//			NN はその日に生成されたうちのなん個目かを表す
	//
	//			100 個以上生成しようとすると、
	//			すべて NN は 99 になり、直前に生成したものが上書きされる

	UnicodeString name;

	try {
		ModTime t = ModTime::getCurrentTime();

		unsigned int i = 0;
		const unsigned int	n = 100;

		do {
			ModUnicodeOstrStream	buf;

			buf << ModIosSetW(2) << ModIosSetFill('0') << (t.getYear() % 100)
				<< ModIosSetW(2) << ModIosSetFill('0') << t.getMonth()
				<< ModIosSetW(2) << ModIosSetFill('0') << t.getDay()
				<< ModIosSetW(2) << ModIosSetFill('0') << i
				<< _Literal::_Dmp;

			name = buf.getString();

		} while (++i < n &&
				 File::access(Path(_MiniDumper::_parent).addPart(name),
							  File::AccessMode::File)) ;
	} catch (...) {

		// エラーが起きても無視する

		ModErrorHandle::reset();
	}

	// この生成した名前で、必要かつ可能であれば、ミニダンプを生成する

	execute(name, ep);
}

//	FUNCTION private
//	Os::Manager::MiniDumper::execute --
//		初期化時に指定されたディレクトリの下に
//		例外情報を含むミニダンプを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Os::UnicodeString&	name
//			ミニダンプを記録するファイルの名前
//		EXCEPTION_POINTERS*	ep
//			発生し、未処理の例外に関する情報を格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
Manager::MiniDumper::execute(const UnicodeString& name, EXCEPTION_POINTERS* ep)
{
	if (name.getLength())
		(void) MiniDump::create(Path(_MiniDumper::_parent).addPart(name),
								_MiniDumper::_permission, ep);
}

//	FUNCTION private
//	Os::Manager::MiniDumper::topLevelExceptionFilter --
//		未処理の例外を処理する関数で、ミニダンプを生成する
//
//	NOTES
//
//	ARGUMENTS
//		EXCEPTION_POINTERS*	ep
//			発生し、未処理の例外に関する情報を格納する領域の先頭アドレス
//
//	RETURN
//		EXCEPTION_CONTINUE_SEARCH
//			この例外処理関数が設定されていないときと同様の動作を続ける
//
//	EXCEPTIONS
//		なし

LONG
Manager::MiniDumper::topLevelExceptionFilter(EXCEPTION_POINTERS* ep)
{
	// 適当な名前で、必要かつ可能であれば、ミニダンプを生成する

	execute(ep);

	// この例外処理関数が設定されていないときと同様の動作を続ける

	return EXCEPTION_CONTINUE_SEARCH;
}

//	FUNCTION public
//	Os::MiniDump::create -- 例外情報を含まないミニダンプを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			ミニダンプを記録するファイルの絶対パス名
//		Os::File::Permission::Value	permission
//			ミニダンプを記録するファイルの許可モード
//
//	RETURN	
//		true
//			ミニダンプを記録した
//		false
//			ミニダンプを記録しなかった
//
//	EXCEPTIONS
//		なし

// static
bool
MiniDump::create(const Path& path, File::Permission::Value permission)
{
	return create(path, permission, 0);
}

//	FUNCTION private
//	Os::MiniDump::create -- 例外情報を含むミニダンプを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&		path
//			ミニダンプを記録するファイルの絶対パス名
//		Os::File::Permission::Value	permission
//			ミニダンプを記録するファイルの許可モード
//		EXCEPTION_POINTERS*	ep
//			記録するミニダンプに含める例外情報を格納する領域の先頭アドレス
//
//	RETURN
//		true
//			ミニダンプを記録した
//		false
//			ミニダンプを記録しなかった
//
//	EXCEPTIONS
//		なし

// static
bool
MiniDump::create(const Path& path, File::Permission::Value permission,
				 EXCEPTION_POINTERS* ep)
{
	// まず、ミニダンプを記録するファイルを生成する
	//
	//【注意】	Os::File を使いたいところだが、
	//			必ず非同期モードでオープンするので使えない
	//
	//【注意】	そのため、現状では与えられた許可モードはまったく使っていない

	HANDLE handle = ::CreateFile(
		path,
		GENERIC_WRITE, FILE_SHARE_WRITE,
		0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	bool created = false;

	if (handle != INVALID_HANDLE_VALUE) {

		// ファイルが生成できたので、そこにミニダンプを記録する

		created = _MiniDump::create(handle, ep);

		// ミニダンプを記録したファイルのハンドルを破棄する

		(void) ::CloseHandle(handle);

		if (!created)
			try {

				// ミニダンプを記録できなかったので、
				// ミニダンプの記録用に生成したファイルを破棄する

				File::remove(path);

			} catch (...) {

				// エラーは無視する

				ModErrorHandle::reset();
			}
	}

	return created;
}
#endif

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
