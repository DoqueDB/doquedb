// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AsyncStatus.cpp -- 非同期操作の実行状況を表すクラスの関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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
#endif
#ifdef SYD_OS_POSIX
#include <assert.h>
#endif
}

#include "Os/Assert.h"
#include "Os/AsyncStatus.h"
#include "Os/Unicode.h"

#include "Exception/SystemCall.h"

#include "ModTypes.h"

_TRMEISTER_USING
_TRMEISTER_OS_USING

namespace 
{

namespace _Literal
{
	#define	U(literal)	const UnicodeString	literal(#literal);

	// 関数名関係

#ifdef SYD_OS_WINDOWS
	U(CreateEvent);
	U(GetOverlappedResult);
#endif

	#undef U
}

}

//	FUNCTION public
//	Os::AsyncStatus::AsyncStatus --
//		非同期操作の実行状況を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Os::File&			file
//			非同期操作するファイルを表すクラス
//		Os::File::Offset	offset
//			非同期操作する領域のファイルの先頭からのオフセット(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

AsyncStatus::AsyncStatus(File& file, File::Offset offset)
#ifdef SYD_OS_WINDOWS
	: _handle(file._handle)
#endif
{
#ifdef SYD_OS_WINDOWS
	// オーバーラップ構造体を初期化する

	// 手動リセットイベントのハンドルを生成し、設定する

	_overlapped.hEvent = ::CreateEvent(0, true, false, 0);
	if (!_overlapped.hEvent) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall, _Literal::CreateEvent, osErrno);
	}

	// 操作する領域のファイルの先頭からのオフセットを設定する

	ModStructuredInt64	position;
	position.full = offset;

	_overlapped.Offset = position.halfs.low;
	_overlapped.OffsetHigh = position.halfs.high;
#endif
}

//	FUNCTION public
//	Os::AsyncStatus::wait -- 非同期操作が終了するまで待つ
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		終了した非同期操作が実際に操作したデータ量(B 単位)
//
//	EXCEPTIONS

Memory::Size
AsyncStatus::wait()
{
#ifdef SYD_OS_WINDOWS

	// 非同期操作の実行が終了するまで待ち、
	// 実際に操作したデータ量を得る

	DWORD n;
	if (!::GetOverlappedResult(_handle, &_overlapped, &n, true)) {
		const DWORD osErrno = ::GetLastError();
		switch (osErrno) {
		case ERROR_HANDLE_EOF:

			// 読み出し中にファイルの末尾に達した

			n = 0;
			break;

		default:

			// システムコールのエラーを表す例外を投げる

			_TRMEISTER_THROW2(
				Exception::SystemCall, _Literal::GetOverlappedResult, osErrno);
		}
	}

	// 得られた実際に操作したデータ量を返す

	return n;
#endif
#ifdef SYD_OS_POSIX
	return 0;
#endif
}

#ifdef OBSOLETE
//	FUNCTION public
//	Os::AsyncStatus::isCompleted --	非同期操作が終了しているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			終了している
//		false
//			終了していない
//
//	EXCEPTIONS
//		なし

bool
AsyncStatus::isCompleted()
{
#ifdef SYD_OS_WINDOWS
	
	// 非同期操作の実行が終了しているか調べる
	//
	//【注意】	HasOverlappedIoCompleted はマクロなので、
	//			スコープ解決演算子は使えない

	return HasOverlappedIoCompleted(&_overlapped);
#endif
}

//	FUNCTION public
//	Os::AsyncStatus::isCompleted --
//		非同期操作が終了しているか調べ、
//		終了していれば、実際に操作したデータ量を得る
//
//	NOTES
//
//	ARGUMENTS
//		Os::Memory::Size&	n
//			非同期操作が終了していれば、実際に操作したデータ量が代入される
//
//	RETURN
//		true
//			終了している
//		false
//			終了していない
//
//	EXCEPTIONS

bool
AsyncStatus::isCompleted(Memory::Size& n)
{
#ifdef SYD_OS_WINDOWS

	// 非同期操作の実行が終了しているか調べ、
	// 終了していれば、実際に操作したデータ量を得る

	if (!::GetOverlappedResult(_handle, &_overlapped, &n, false)) {
		const DWORD osErrno = ::GetLastError();
		if (osErrno != ERROR_IO_INCOMPLETE)

			// システムコールのエラーを表す例外を投げる

			_TRMEISTER_THROW2(
				Exception::SystemCall, _Literal::GetOverlappedResult, osErrno);

		// 非同期操作は終了していなかった

		return false;
	}

	// 非同期操作は終了していた

	return true;
#endif
}
#endif

//
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
