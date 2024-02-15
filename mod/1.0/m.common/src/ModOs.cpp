// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModOs.cpp -- 仮想 OS に関するメンバーの定義
// 
// Copyright (c) 1997, 2010, 2023 Ricoh Company, Ltd.
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


extern "C" {
#include <string.h>
#include <errno.h>
}

#include "ModOs.h"
#include "ModOsDriver.h"
#include "ModException.h"
#include "ModCommonException.h"
#include "ModCharTrait.h"
#include "ModMessage.h"

//
// VARIABLE
// ModOs::Process::_encodeingType -- 現在のエンコーディング方式(漢字コード)
//
// NOTES
// 現在のエンコーディング方式(漢字コード)を表す。
//
ModKanjiCode::KanjiCodeType
ModOs::Process::_encodeingType = ModKanjiCode::unknown;

//
// FUNCTION private
// ModOs::initialize -- 初期化関数をまとめたもの
//
// NOTES
//	仮想OSレベルを初期化する関数。プライベート関数であり、ModCommonInitialize
//	から一回だけ呼び出されることが保証される。
//	仮想OSレベルで必要となるスレッド、ソケットの初期化を行う。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModOsDriver::Thread::initialize, ModOsDriver::Socket::initializeの例外参照
//
void
ModOs::initialize()
{
	// スレッドまわりの初期化関数
	ModOsDriver::Thread::initialize();

//	LoadLibraryでロードされるときにソケット関係の関数は呼んではいけない
//	よって、ここではソケットの初期化は行わない
//	ソケットの初期化は必要に応じて、ModOsDriver::Socket の中で呼ばれる
}

//
// FUNCTION private
// ModOs::terminate -- 仮想OSレベルの後処理関数をまとめたもの
//
// NOTES
//	仮想OSレベルの後処理を行う関数
//	プライベート関数であり、ModCommonInitialize
//	から一回だけ呼び出されることが保証される。
//	仮想OSレベルで必要となるスレッド、ソケットの後処理を行う。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし	
//
// EXCEPTIONS
//	その他
//		ModOsDriver::Socket::terminate()の例外参照
//
void
ModOs::terminate()
{
	// ソケットまわりの後処理
	ModOsDriver::Socket::terminate();
	// スレッドまわりの後処理
	ModOsDriver::Thread::terminate();
}

//	FUNCTION public
//	ModOs::getErrorNumber -- C の errno から MOD のエラー番号を得る
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		error
//			MOD のエラー番号を得たい C の errno
//		char*		func
//			指定されたとき
//				得られた MOD のエラー番号が ModOsErrorOtherReason のとき、
//				与えられた関数名を表す文字列を含むエラーメッセージを出力する
//			指定されないとき
//				エラーメッセージを出力しない
//		char*		file
//			指定されたとき
//				与えられた関数を呼び出したファイル名
//			指定されないとき
//				エラーメッセージを出力しない
//		int			line
//			指定されたとき
//				与えられた関数を呼び出した行数
//			指定されないとき
//				エラーメッセージを出力しない
//
//	RETURN
//		得られた MOD のエラー番号
//
//	EXCEPTIONS
//		なし

// static
ModErrorNumber
ModOs::getErrorNumber(unsigned int error)
{
	ModErrorNumber	n;
	switch (error) {

//	Permission denied
	case EACCES:
//	Read only file system
	case EROFS:
//	Not super-user / Operation not permitted
	case EPERM:
		n = ModOsErrorPermissionDenied;			break;

//	Bad address
	case EFAULT:
//	Invalid argument
	case EINVAL:
//	Math result not representable / Result too large
	case ERANGE:
		n = ModCommonErrorBadArgument;			break;

//	interrupted system call / Interrupted function call
	case EINTR:
		n = ModOsErrorInterrupt;				break;

//	Is a directory
	case EISDIR:
		n = ModOsErrorIsDirectory;				break;

//	Too many open files
	case EMFILE:
		n = ModOsErrorOpenTooManyFiles;			break;

//	Resource temporarily unavailable
	case EAGAIN:
		n = ModOsErrorResourceExhaust;			break;

//	File table overflow / Too many open files in system
	case ENFILE:
//	No space left on device
	case ENOSPC:
		n = ModOsErrorNotSpace;					break;

//	Not enough core / Not enough memory
	case ENOMEM:

		//【注意】	MOD のメモリー管理モジュールが生成するものと
		//			別のエラー番号にする

		n = ModOsErrorSystemMemoryExhaust;		break;

//	No such file or directory
	case ENOENT:
//	Not a directory
	case ENOTDIR:
		n = ModOsErrorFileNotFound;				break;

//	File exists
	case EEXIST:
		n = ModOsErrorFileExist;				break;

//	Directory not empty
	case ENOTEMPTY:
		n = ModOsErrorNotEmpty;					break;

//	Bad file number
	case EBADF:
		n = ModOsErrorBadFileDescriptor;		break;

//	File too large
	case EFBIG:
		n = ModOsErrorTooBigFile;				break;

//	path name is too long / Filename too long
	case ENAMETOOLONG:
		n = ModOsErrorTooLongFilename;			break;

//	I/O error / Input/output error
	case EIO:
		n = ModOsErrorIOError;					break;

//	Broken pipe
	case EPIPE:
		n = ModOsErrorBrokenPipe;				break;

//	No such process
//	case ESRCH:
//	No such device or address
//	case ENXIO:
//	Arg list too long
//	case E2BIG:
//	Exec format error
//	case ENOEXEC:
//	No children / No child processes
//	case ECHILD:
//	Block device required / Unknown error
//	case ENOTBLK:
//	Mount device busy / Resource device
//	case EBUSY:
//	Cross-device link / Improper link
//	case EXDEV:
//	No such device
//	case ENODEV:
//	Inappropriate ioctl for device / Inappropriate I/O control operation
//	case ENOTTY:
//	Text file busy / Unknown error
//	case ETXTBSY:
//	Illegal seek / Invalid seek
//	case ESPIPE:
//	Too many links
//	case EMLINK:
//	Math arg out of domain of func / Domain error
//	case EDOM:
//	file locking deadlock error / Resource deadlock would occur
//	case EDEADLOCK:
//	Deadlock condition / Resource deadlock avoided
//	case EDEADLK:
//	No record locks available / No locks available
//	case ENOLCK:
//	Unsupported file system operation / Function not implemented
//	case ENOSYS:
//	Illegal byte sequence
//	case EILSEQ:
	default:
		n = ModOsErrorOtherReason;
	}

	return n;
}

// static
ModErrorNumber
ModOs::getErrorNumber(unsigned int error, const char* func,
					  const char* file, int line)
{
	ModErrorNumber	n = ModOs::getErrorNumber(error);
	switch (n) {
	case ModOsErrorOtherReason:
		ModOs::printOsErrorNumber(error, func, file, line);
		ModOs::printOsErrorMessage(error, file, line);
	}
	return n;
}

//	FUNCTION public
//	ModOs::getOsErrorOtherReason --
//		C の errno から MOD のその他のエラーを表すエラー番号を得る
//
//	NOTES
//		与えられた C の errno を表すエラーメッセージを出力し、
//		MOD のその他のエラーを表すエラー番号である ModOsErrorOtherReason を得る
//
//	ARGUMENTS
//		unsigned int		error
//			エラーメッセージを出力する errno
//		char*				func
//			エラーを起こした関数の名前
//		char*				file
//			エラーが発生したファイルの名前
//		int					line
//			エラーが発生したファイルの行数
//
//	RETURN
//		ModOsErrorOtherReason
//
//	EXCEPTIONS
//		なし

// static
ModErrorNumber
ModOs::getOsErrorOtherReason(unsigned int error, const char* func,
							 const char* file, int line)
{
	ModOs::printOsErrorNumber(error, func, file, line);
	ModOs::printOsErrorMessage(error, file, line);
	return ModOsErrorOtherReason;
}

//	FUNCTION public
//	ModOs::printOsErrorNumber -- C の errno を含むエラーメッセージを出力する
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		error
//			エラーメッセージを出力する C の errno
//		char*				func
//			エラーが発生した関数の名前
//		char*				file
//			エラーが発生したファイルの名前
//		int					line
//			エラーが発生したファイルの行数
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
ModOs::printOsErrorNumber(unsigned int error, const char* func,
						  const char* file, int line)
{
	ModMessageStreamWrapper message = ModMessageSelection::error(file, line);
	message.getStream() << "Uncategorized Os Error: " << error << " occured ";
	if (func)
		message.getStream() << "(" << func << ")";
	message.getStream() << ModEndl;
}

//	FUNCTION public
//	ModOs::printOsErrorMessage -- C の errno を表すエラーメッセージを出力する
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		error
//			エラーメッセージを出力する C の errno
//		char*				file
//			エラーが発生したファイルの名前
//		int					line
//			エラーが発生したファイルの行数
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
ModOs::printOsErrorMessage(unsigned int error, const char* file, int line)
{
	ModException exception;
	char*	p = exception.getMessageBuffer();
	const char*	m = ::strerror(error);
	if (m == 0)
	{
		// strerrorでエラーメッセージが得られなかった
		
		m = "Not include error message.";
	}
	ModCharTrait::copy(p, m);
	ModMessageSelection::error(file, line).getStream()
		<< "(ERROR) " << p << ModEndl;
}

#ifndef MOD_SELF_MEMORY_MANAGEMENT_OFF
//	FUNCTION public
//	ModOs::File::operator new -- 自由記憶領域を確保する
//
//	NOTES
//		::operator new で自由記憶領域を確保する
//
//	ARGUMENTS
//		size_t		size
//			確保する自由記憶領域のサイズ(B 単位)
//
//	RETURN
//		確保した自由記憶領域の先頭アドレス
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			必要なメモリーがシステムに存在しない

void*
ModOs::File::operator new(size_t size)
{
	return ModOsDriver::newSetHandler((ModSize)size);
}

//	FUNCTION public
//	ModOs::Socket::operator new -- 自由記憶領域を確保する
//
//	NOTES
//		::operator new で自由記憶領域を確保する
//
//	ARGUMENTS
//		size_t		size
//			確保する自由記憶領域のサイズ(B 単位)
//
//	RETURN
//		確保した自由記憶領域の先頭アドレス
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			必要なメモリーがシステムに存在しない

void*
ModOs::Socket::operator new(size_t size)
{
	return ModOsDriver::newSetHandler((ModSize)size);
}

//	FUNCTION public
//	ModOs::ThreadSpecificKey::operator new -- 自由記憶領域を確保する
//
//	NOTES
//		::operator new で自由記憶領域を確保する
//
//	ARGUMENTS
//		size_t		size
//			確保する自由記憶領域のサイズ(B 単位)
//
//	RETURN
//		確保した自由記憶領域の先頭アドレス
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			必要なメモリーがシステムに存在しない

void*
ModOs::ThreadSpecificKey::operator new(size_t size)
{
	return ModOsDriver::newSetHandler((ModSize)size);
}

//	FUNCTION public
//	ModOs::Thread::operator new -- 自由記憶領域を確保する
//
//	NOTES
//		::operator new で自由記憶領域を確保する
//
//	ARGUMENTS
//		size_t		size
//			確保する自由記憶領域のサイズ(B 単位)
//
//	RETURN
//		確保した自由記憶領域の先頭アドレス
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			必要なメモリーがシステムに存在しない

void*
ModOs::Thread::operator new(size_t size)
{
	return ModOsDriver::newSetHandler((ModSize)size);
}

//	FUNCTION public
//	ModOs::Thread::Wrapper::operator new -- 自由記憶領域を確保する
//
//	NOTES
//		::operator new で自由記憶領域を確保する
//
//	ARGUMENTS
//		size_t		size
//			確保する自由記憶領域のサイズ(B 単位)
//
//	RETURN
//		確保した自由記憶領域の先頭アドレス
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			必要なメモリーがシステムに存在しない

void*
ModOs::Thread::Wrapper::operator new(size_t size)
{
	return ModOsDriver::newSetHandler((ModSize)size);
}

//	FUNCTION public
//	ModOs::ConditionVariable::operator new -- 自由記憶領域を確保する
//
//	NOTES
//		::operator new で自由記憶領域を確保する
//
//	ARGUMENTS
//		size_t		size
//			確保する自由記憶領域のサイズ(B 単位)
//
//	RETURN
//		確保した自由記憶領域の先頭アドレス
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			必要なメモリーがシステムに存在しない

void*
ModOs::ConditionVariable::operator new(size_t size)
{
	return ModOsDriver::newSetHandler((ModSize)size);
}
#endif

//
// Copyright (c) 1997, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
