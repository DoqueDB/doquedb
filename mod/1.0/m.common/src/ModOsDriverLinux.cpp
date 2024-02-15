// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModOsDriverLinux.cpp -- 仮想 OS ドライバーに関するメンバーの定義
// 
// Copyright (c) 1997, 2016, 2023 Ricoh Company, Ltd.
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

static const char srcFile[] = __FILE__;

//
// どうしても必要なCのヘッダをインクルードする。
// #include ... の右側にコメントで目的を明記すること。
//

extern "C" {
#include <fcntl.h>		// open
#include <sys/stat.h>	// open, stat
#include <unistd.h>		// lseek, chdir

#include <stdlib.h>		// atoi, atof, exit, abort
#include <ctype.h>		// tolower, toupper, isascii, isspace
#include <errno.h>		// errnoの利用のため

#include <sys/socket.h>	// socket
#include <netdb.h>		// gethostbyname
#include <time.h>		// nanosleep

#include <arpa/inet.h>  // inet_addr利用のため yohji

#include <signal.h>		// signal

#include <sys/utsname.h>	// uname
#include <pwd.h>			// getpwuid_r
#include <dirent.h>		// opendir, readdir_r

#include <sys/time.h>	// getrlimit
#include <sys/resource.h> // getrlimit
#ifdef OS_RHLINUX6_0
#include <asm/param.h>  // MAXHOSTNAMELEN
#include <stdio.h>
#include <netinet/tcp.h>	// TCP_KEEPIDLE, TCP_KEEPINTVL, TCP_KEEPCNT
#endif  // OS_RHLINUX6_0
}

#include <new>		// set_new_handlerのため

#include "ModCommon.h"
#include "ModOsDriverLinux.h"	// new はだめだけど、allocateMemoryはする。
// ModExceptionが使えるかどうか、スレッドの初期化チェックをするため。
#include "ModThread.h"
#include "ModParameter.h"
#include "ModOsException.h"

#include "ModCharTrait.h"	// ファイル名のフルパスチェック用文字処理
#include "ModCharString.h"	// 同上
#include "ModUnicodeCharTrait.h"	// 同上
#include "ModUnicodeString.h"		// 同上
#include "ModAutoPointer.h"
#include "ModAutoMutex.h"	// エラーハンドラ登録時自動ロック解除のため。

#include "ModVector.h"


#ifdef MOD_DEBUG
#include "ModFakeError.h"				// 疑似エラー用
#endif

namespace _Error
{
	// OSエラーに対するエラーレベルを得る
	ModErrorLevel	getErrorLevel(int error_)
	{
		switch (error_) {
////////////////////////////
// WARNING
////////////////////////////
		// Socket関係
		case EADDRINUSE:				//	Address already in use
		case EADDRNOTAVAIL:				//	Can't assign requested address
		case EALREADY:					//	operation already in progress
		case ECONNREFUSED:				//	Connection refused
		case EISCONN:					//	Socket is already connected
		case ENETUNREACH:				//	Network is unreachable
		case ETIMEDOUT:					//	Connection timed out
		case EWOULDBLOCK:				//	Resource temporarily unavailable
		// Thread, File関係
		// ---- file not found系 ----
		case ENOENT:					//	No such file or directory
		// ---- permission denied系 ----
		case EACCES:					//	Permission denied
		case EROFS:						//	Read only file system
		case EPERM:						//	Not super-user / Operation not permitted
		case EISDIR:					//	Is a directory
		case ENOTDIR:					//	Not a directory
		case EEXIST:					//	File exists
		// ---- その他 ----
		case ESRCH:						//	No such process
		case ERANGE:					//	Math result not representable / Result too large
		case EINTR:						//	interrupted system call / Interrupted function call
		case EFBIG:						//	File too large
		case ENAMETOOLONG:				//	path name is too long / Filename too long
			{
				return ModErrorLevelWarning;
			}
////////////////////////////
// ERROR
////////////////////////////
		case ENOTEMPTY:					//	Directory not empty
		case EPIPE:						//	Broken pipe
			{
				return ModErrorLevelError;
			}
////////////////////////////
// FATAL
////////////////////////////
		// Socket関係
		case ENOTSOCK:					//	Socket operation on non-socket
		case EPROTONOSUPPORT:			//	Protocol not supported
		case ENOSR:						//	out of streams resources

		// Thread, File関係
		// ---- bad argument系 ----
		case EINVAL:					//	Invalid argument
		case EBADF:						//	Bad file number
		// ---- disk障害系 ----
		/* case EIO:	= EPROTONOSUPPORT //	I/O error / Input/output error */
		// ---- その他 ----
		case EFAULT:					//	Bad address
		case EMFILE:					//	Too many open files
		case ENFILE:					//	File table overflow / Too many open files in system
		case ENOSPC:					//	No space left on device
		case ENOMEM:					//	Not enough core / Not enough memory
			{
				return ModErrorLevelFatal;
			}

		// 列挙されていないエラーはERRORレベルとする
		default:
			{
				return ModErrorLevelError;
			}
		}
	}

	// getaddrinfo のエラーメッセージを出力する

	ModErrorNumber printOtherMessage(int error, const char* func,
									 const char* file, int line)
	{
		ModMessageStreamWrapper message = ModMessageSelection::error(file, line);
		message.getStream() << "Uncategorized Os Error: " << error << " occured ";
		if (func)
			message.getStream() << "(" << func << ")";
		message.getStream() << ModEndl;
		return ModOsErrorOtherReason;
	}
};

//
// ** 64bit Large File対応は考慮しているが、基本的には
//	コンパイルオプションで対応する予定なのでまだ実施していない。
//	もちろんテストもしていない。
//

// ****** ファイル関連 ******

//	FUNCTION public
//	ModOsDriver::File::File -- 仮想 OS のファイルクラスのコンストラクター
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

ModOsDriver::File::File()
	: _path(0),
	  _descriptor(-1),
	  _isTape(ModFalse)
{
	_path = new ModCharString();
}

//	FUNCTION public
//	ModOsDriver::File::~File -- 仮想 OS のファイルクラスのデストラクター
//
//	NOTES
//		ファイルがオープンされたままならば、クローズされる
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

ModOsDriver::File::~File()
{
	if (this->isOpened())

		// ファイルがオープンされたままなので、クローズしておく

		this->close();

	delete _path, _path = 0;
}

void
ModOsDriver::File::create(const ModUnicodeString& path, int mode, ModSize block)
{
	ModOsDriver::File::create(const_cast<ModUnicodeString&>(path).getString(ModOs::Process::getEncodingType()), mode, block);
}

//	FUNCTION public
//	ModOsDriver::File::open -- ファイルをオープンする
//
//	NOTES
//		指定されたパス名の表すファイルをオープンする
//		ディレクトリーはオープンできないが、テープデバイスはオープンできる
//		オープンされたファイルは ModOsDriver::File::close でクローズできる
//
//		flag に ModOs::createFlag を与えると、
//		ファイルを生成してからオープンすることができる
//
//	ARGUMENTS
//		char*				path
//			オープンするファイルのパス名
//		int					flag
//			どのようにオープンするかを表す値
//			ModOs::OpenFlag の論理積を指定する
//		int					mode
//			指定されたとき
//				生成するファイルのアクセス権を表す値
//				ModOs::PermissionMode の論理積を指定する
//			指定されないとき
//				ModOs::ownerReadMode | ModOs::ownerWriteMode が
//				指定されたものとみなす
//		ModSize				block
//			指定されたとき
//				指定されたパス名のファイルがテープデバイスのとき、
//				読み書きを行うブロックのサイズ(B 単位)
//			指定されないとき
//				ModOsBlockSizeDefault が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			パス名として 0 が指定されている
//		ModOsErrorFileAlreadyOpened
//			オープンしようとしているファイルはすでにオープンされている
//		ModOsErrorPermissionDenied
//			指定されたパス名にアクセスできない
//			または指定されたパス名の親ディレクトリーは書き込み不可である
//			または指定されたパス名のファイルが存在し、書き込み不可である
//		ModOsErrorFileExist
//			ModOs::exclusiveFlag を指定したとき、
//			生成しようとしているファイルは既に存在する
//		ModOsErrorInterrupt
//			シグナルにより中断された
//		ModOsErrorIsDirectory
//			指定されたパス名が既に存在するディレクトリーである
//		ModOsErrorOpenTooManyFiles
//			すでにオープンしているファイルが多すぎる
//		ModOsErrorNotSpace
//			ファイルシステムのリソースが足りない
//		ModOsErrorFileNotFound
//			指定されたパス名の親ディレクトリーが存在しない、
//			またはディレクトリーでない
//		ModOsErrorTooLongFilename
//			指定されたパス名が長すぎる
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

void
ModOsDriver::File::open(const ModUnicodeString& path, int flag, int mode, ModSize block)
{
	ModOsDriver::File::open(const_cast<ModUnicodeString&>(path).getString(ModOs::Process::getEncodingType()), flag, mode, block);
}

void
ModOsDriver::File::open(const char* path, int flag, int mode,
						unsigned int block)
{
	if (this->isOpened())

		// すでにオープンされている

		ModThrowOsError(ModOsErrorFileAlreadyOpened);

	if (path == 0)

		// オープンするファイルのパス名が指定されていない

		ModThrowOsError(ModCommonErrorBadArgument);

	// 実際にオープンする

#if 0 	// Linux用にフラグを変換するようにしたが、
		// ModOs.hでフラグを定義し直して対応する事にした

	// OS依存の open flag を設定する
	int osFlag = O_RDONLY;
	if (flag & ModOs::writeOnlyFlag) {
		osFlag |= O_WRONLY;
	}
	if (flag & ModOs::readWriteFlag) {
		osFlag |= O_RDWR;
	}
	if (flag & ModOs::appendFlag) {
		osFlag |= O_APPEND;
	}
	if (flag & ModOs::createFlag) {
		osFlag |= O_CREAT;
	}
	if (flag & ModOs::truncateFlag) {
		osFlag |= O_TRUNC;
	}
	if (flag & ModOs::exclusiveFlag) {
		osFlag |= O_EXCL;
	}
	if (flag & ModOs::writeThroughFlag) {
		osFlag |= O_SYNC;
	}

#endif

	// OS依存の open 関数を呼び出す
	// ModOs::XXXModeはsys/stat.h互換である。そのままの値を使ってよい。

	static const char	func_open[] = "open";

	_descriptor =
#ifdef MOD_DEBUG
		(ModFakeError::check(func_open)) ? -1 :
#endif

#if 0	// Linux用にフラグを変換するようにしたが、
		// ModOs.hでフラグを定義し直して対応する事にした
		::open(path, osFlag ,mode & ModOs::PermissionModeMask);
#else //
		::open(path, flag & ModOs::OpenFlagMask,
			   mode & ModOs::PermissionModeMask);
#endif //

	if (_descriptor == -1) {
		int	saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_open, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}

	// 与えられたパス名を絶対パス名に変換しておく

	ModOsDriver::File::getFullPathName(path, *_path);

	// オープンしたファイルがテープデバイスか調べておく

	_isTape = ModOsDriver::File::isTapeDevice(*_path);
}

//	FUNCTION public
//	ModOsDriver::File::close -- ファイルをクローズする
//
//	NOTES
//		クローズするファイルが実際にオープンされているかは検査しない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorBadFileDescriptor
//			オープンされていないファイルをクローズしようとした
//		ModOsErrorInterrupt
//			シグナルにより中断された
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

void
ModOsDriver::File::close()
{
	static const char	func_close[] = "close";

	if (::close(_descriptor) == -1) {
		int	saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_close, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}

	// オープンしていたファイルに関する情報を初期化しておく

	_path->clear();
	_descriptor = -1;
	_isTape = ModFalse;
}

//	FUNCTION public
//	ModOsDriver::File::read -- ファイルからデータを読み出す
//
//	NOTES
//
//	ARGUMENTS
//		void*				buf
//			ファイルから読み出したデータを格納する領域の先頭アドレス
//		ModSize				size
//			ファイルから読み出したデータを格納する領域のサイズ(B 単位)
//
//	RETURN
//		実際に読み出したデータのサイズ(B 単位)
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			読み出したデータを格納する領域の先頭アドレス、
//			またはそのサイズとして 0 が指定された
//		ModOsErrorBadFileDescriptor
//			オープンされていないファイルからデータを読み出そうとした
//		ModOsErrorIsDirectory
//			ディレクトリーからデータを読み出そうとした
//		ModOsErrorInterrupt
//			シグナルにより中断された
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

ModSize
ModOsDriver::File::read(void* buf, ModSize size)
{
	if (buf == 0 || size == 0)

		// 読み出した内容を格納する領域の先頭アドレスや
		// そのサイズとして 0 が指定されている

	    ModThrowOsError(ModCommonErrorBadArgument);

	static const char	func_read[] = "read";

	ssize_t	n =
#ifdef MOD_DEBUG
		(ModFakeError::check(func_read)) ? -1 :
#endif
		::read(_descriptor, buf, (size_t) size);

	if (n < 0) {
		int saved = errno;

		//【注意】	マルチプレクサーからの読み出し時に、
		//			EINVAL になる

		ModThrowOs((saved == EINVAL) ?
				   ModOs::getOsErrorOtherReason(
						saved, func_read, srcFile, __LINE__) :
				   ModOsDriver::File::getErrorNumber(
						saved, func_read, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}

	return (ModSize) n;
}

//	FUNCTION public
//	ModOsDriver::File::write -- ファイルへデータを書き込む
//
//	NOTES
//
//	ARGUMENTS
//		void*				buf
//			ファイルへ書き込むデータを格納した領域の先頭アドレス
//		ModSize				size
//			ファイルへ書き込むデータのサイズ(B 単位)
//
//	RETURN
//		実際に書き出したデータのサイズ(B 単位)
//		ただし、必ず size と等しい
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			書き出すデータを格納する領域の先頭アドレス、
//			または書き出すデータのサイズとして 0 が指定された
//		ModOsErrorBadFileDescriptor
//			オープンされていないファイルへデータを書き込もうとした
//		ModOsErrorInterrupt
//			シグナルにより中断された
//		ModOsErrorNotSpace
//			空き領域がなく、データをすべて書き込めない
//		ModOsErrorTooBigFile
//			ファイルサイズの上限を超えて書き込みを行おうとした
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

ModSize
ModOsDriver::File::write(const void* buf, ModSize size)
{
	if (buf == 0 || size == 0)

		// 書き込む内容を格納する領域の先頭アドレスや
		// 書き込む内容のサイズとして 0 が指定されている

		ModThrowOsError(ModCommonErrorBadArgument);

	static const char	func_write[] = "write";
	const unsigned char*	p = (const unsigned char*) buf;
	ssize_t	rest = size;
	ssize_t	n;
retry:
	n =
#ifdef MOD_DEBUG
		(ModFakeError::check(func_write)) ? -1 :
#endif
		::write(_descriptor, p, rest);

	if (rest != n) {
		if (n != -1) {

			// なんらかの理由ですべてを書き込めなかったので、
			// 残りを再度書き込んでみる
			//
			//【注意】	0 のときでももう一度書き込むことにより、
			//			-1 が返り、ENOSPC のエラーになる

			p += n;
			rest -= n;
			goto retry;
		}

		int saved = errno;

		//【注意】	マルチプレクサーへの書き込み時に、
		//			EINVAL になる

		ModThrowOs((saved == EINVAL) ?
				   ModOs::getOsErrorOtherReason(
						saved, func_write, srcFile, __LINE__) :
					ModOsDriver::File::getErrorNumber(
						saved, func_write, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}

	return size;
}

//	FUNCTION public
//	ModOsDriver::File::seek -- ファイルポインターを移動する
//
//	NOTES
//
//	ARGUMENTS
//		ModFileOffset		offset
//			ファイルポインターの移動量(B 単位)
//		ModOs::SeekWhence	whence
//			ファイルポインターをどこから移動するかを表す
//
//	RETURN
//		ファイルの先頭から移動後のファイルポインターのオフセット(B 単位)
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			このファイルには無効なオフセットが指定された
//		ModOsErrorBadFileDescriptor
//			オープンされていないファイルのファイルポインターを移動しようとした
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

ModFileOffset
ModOsDriver::File::seek(ModFileOffset offset, ModOs::SeekWhence whence)
{
	static const char	func_llseek[] = "llseek";
	static const int	osWhence[] = { SEEK_SET, SEEK_CUR, SEEK_END };

#ifdef OS_RHLINUX6_0
	// これはとりあえずOS_* のままにしておく
	// Linuxにはllseekはない
	ModFileOffset   position = (ModFileOffset)
		::lseek(_descriptor, (off_t) offset, osWhence[whence]);

#else //  OS_RHLINUX6_0
	ModFileOffset	position = (ModFileOffset)
		::llseek(_descriptor, (offset_t) offset, osWhence[whence]);
#endif //  OS_RHLINUX6_0

	if (position == -1) {
		int	saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_llseek, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}

	return position;
}

//	FUNCTION public
//	ModOsDriver::File::getFileSize -- ファイルサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイルサイズ(B 単位)
//
//	EXCEPTIONS
//		ModOsErrorBadFileDescriptor
//			オープンされていないファイルのファイルサイズを求めようとした
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

ModFileSize
ModOsDriver::File::getFileSize()
{
	static const char	func_fstat[] = "fstat";

	struct stat	buf;
	if (::fstat(_descriptor, &buf) == -1) {
		int saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_fstat, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}

	return (ModFileSize) (buf.st_size);
}

//static
ModFileSize
ModOsDriver::File::getFileSize(const ModUnicodeString& path)
{
	return ModOsDriver::File::getFileSize(const_cast<ModUnicodeString&>(path).getString(ModOs::Process::getEncodingType()));
}

// static
ModFileSize
ModOsDriver::File::getFileSize(const char* path)
{
	if (path == 0)

		// ファイルサイズを求めるファイルのパス名が指定されていない

		ModThrowOsError(ModCommonErrorBadArgument);

	static const char	func_stat[] = "stat";

	struct stat	buf;
	if (::stat(path, &buf) == -1) {
		int saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_stat, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}

	return (ModFileSize) (buf.st_size);
}

//	FUNCTION public
//	ModOsDriver::File::truncate -- ファイルのサイズを変更する
//
//	NOTES
//
//	ARGUMENTS
//		char*				path
//			ファイルサイズを変更するファイルのパス名
//		ModFileSize			length
//			指定されたとき
//				変更後のファイルサイズ(B 単位)
//			指定されないとき
//				0 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			パス名として 0 が指定されている
//			または、ファイルサイズを変更できないファイルのパス名が指定された
//		ModOsErrorPermissionDenied
//			指定されたパス名にアクセスできない
//			または指定されたパス名のファイルが存在し、書き込み不可である
//		ModOsErrorIsDirectory
//			指定されたパス名が既に存在するディレクトリーである
//		ModOsErrorOpenTooManyFiles
//			すでにオープンしているファイルが多すぎる
//		ModOsErrorFileNotFound
//			指定されたパス名の親ディレクトリーが存在しない、
//			またはディレクトリーでない
//		ModOsErrorNotSpace
//			ファイルシステムのリソースが足りない
//		ModOsErrorTooLongFilename
//			指定されたパス名が長すぎる
//		ModOsErrorTooBigFile
//			ファイルサイズの上限を超えるファイルサイズに変更しようとした
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

void
ModOsDriver::File::truncate(ModFileSize length)
{
	//【注意】	ほんとうは ModOsDriver::File::getFileSizeLimit で
	//			得られる上限と比較すべきだが、
	//			遅そうなので ModInt32Max と比較する

	if (length > ModInt32Max)
		ModThrowOsError(ModOsErrorTooBigFile);

	static const char	func_ftruncate[] = "ftruncate";

	if (::ftruncate(_descriptor, (off_t) length) == -1) {
		int saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_ftruncate, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}
}

//static
void
ModOsDriver::File::truncate(const ModUnicodeString& path, ModFileSize length)
{
	ModOsDriver::File::truncate(const_cast<ModUnicodeString&>(path).getString(ModOs::Process::getEncodingType()), length);
}

// static
void
ModOsDriver::File::truncate(const char* path, ModFileSize length)
{
	if (path == 0)

		// ファイルサイズを変更するファイルのパス名が指定されていない

		ModThrowOsError(ModCommonErrorBadArgument);

	if (length > ModInt32Max)
		ModThrowOsError(ModOsErrorTooBigFile);

	static const char	func_truncate[] = "truncate";

	if (::truncate(path, (off_t) length) == -1) {
		int saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_truncate, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}
}

//	FUNCTION public
//	ModOsDriver::File::unlink -- ファイルを削除する
//
//	NOTES
//
//	ARGUMENTS
//		char*				path
//			削除するファイルのパス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			パス名として 0 が指定されている
//		ModOsErrorPermissionDenied
//			指定されたパス名にアクセスできない
//			指定されたパス名のファイルを削除可能な権利がない
//		ModOsErrorInterrupt
//			シグナルにより中断された
//		ModOsErrorIsDirectory
//			指定されたパス名はディレクトリーである
//		ModOsErrorFileNotFound
//			指定されたパス名のファイルが存在しない、
//			または、指定されたパス名の親ディレクトリーが存在しない
//		ModOsErrorTooLongFilename
//			指定されたパス名が長すぎる
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

//static
void
ModOsDriver::File::unlink(const ModUnicodeString& path)
{
	ModOsDriver::File::unlink(const_cast<ModUnicodeString&>(path).getString(ModOs::Process::getEncodingType()));
}

// static
void
ModOsDriver::File::unlink(const char* path)
{
	if (path == 0)

		// 削除するファイルのパス名が指定されていない

		ModThrowOsError(ModCommonErrorBadArgument);

	static const char	func_unlink[] = "unlink";

	if (::unlink(path) == -1) {
		int saved = errno;
		ModErrorNumber	n = ModOsDriver::File::getErrorNumber(
			saved, func_unlink, srcFile, __LINE__);

		if (n == ModOsErrorPermissionDenied &&
			ModOsDriver::File::isDirectory(path))

			// 指定されたパス名がディレクトリーだった

			n = ModOsErrorIsDirectory;

		ModThrowOsError(n);
	}
}

//	FUNCTION public
//	ModOsDriver::File::chmod -- ファイルのアクセス権を変更する
//
//	NOTES
//
//	ARGUMENTS
//		char*				path
//			アクセス権を変更するファイルのパス名
//		int					mode
//			ファイルの変更後のアクセス権を表す値
//				ModOs::PermissionMode の論理積を指定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			パス名として 0 が指定されている
//		ModOsErrorPermissionDenied
//			指定されたパス名にアクセスできない
//			指定されたパス名のファイルのアクセス権を変更可能な権利がない
//		ModOsErrorInterrupt
//			シグナルにより中断された
//		ModOsErrorFileNotFound
//			指定されたパス名のファイルが存在しない、
//			または、指定されたパス名の親ディレクトリーが存在しない
//		ModOsErrorTooLongFilename
//			指定されたパス名が長すぎる
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

void
ModOsDriver::File::chmod(int mode)
{
	static const char	func_fchmod[] = "fchmod";

	if (::fchmod(_descriptor, mode) == -1) {
		int saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_fchmod, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}
}

//static
void
ModOsDriver::File::chmod(const ModUnicodeString& path, int mode)
{
	ModOsDriver::File::chmod(const_cast<ModUnicodeString&>(path).getString(ModOs::Process::getEncodingType()), mode);
}

// static
void
ModOsDriver::File::chmod(const char* path, int mode)
{
	if (path == 0)

		// アクセス権を変更するファイルのパス名が指定されていない

		ModThrowOsError(ModCommonErrorBadArgument);

	static const char	func_chmod[] = "chmod";

	if (::chmod(path, mode) == -1) {
		int saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_chmod, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}
}

//	FUNCTION public
//	ModOsDriver::File::access -- ファイルに権利を持っているか調べる
//
//	NOTES
//		あるファイルが指定された権利を持つか調べる
//		指定された権利がなければ、例外 ModOsErrorPermissionDenied を発生する
//
//	ARGUMENTS
//		char*				path
//			権利があるか調べるファイルのパス名
//		int					amode
//			調べる権利を表す値
//			ModOs::AccessMode の論理積を指定する
//
//	RETURN
//		ModTrue
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			パス名として 0 が指定されている
//		ModOsErrorPermissionDenied
//			指定されたパス名にアクセスできない
//			指定されたパス名のファイルに調査した権利がない
//		ModOsErrorInterrupt
//			シグナルにより中断された
//		ModOsErrorFileNotFound
//			指定されたパス名のファイルが存在しない、
//			または、指定されたパス名の親ディレクトリーが存在しない
//		ModOsErrorTooLongFilename
//			指定されたパス名が長すぎる
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

//static
ModBoolean
ModOsDriver::File::access(const ModUnicodeString& path, int amode)
{
	return ModOsDriver::File::access(const_cast<ModUnicodeString&>(path).getString(ModOs::Process::getEncodingType()), amode);
}

// static
ModBoolean
ModOsDriver::File::access(const char* path, int amode)
{
	if (path == 0)

		// 権利があるか調べるファイルのパス名が指定されていない

		ModThrowOsError(ModCommonErrorBadArgument);

	// 権利の有無を調べる

	static const char	func_access[] = "access";

	if (::access(path, amode) == -1) {
		int saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_access, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}

	return ModTrue;
}

//	FUNCTION public
//	ModOsDriver::File::isNotFound -- ファイルが存在しないか調べる
//
//	NOTES
//
//	ARGUMENTS
//		char*				path
//			存在しないか調べるファイルのパス名
//
//	RETURN
//		ModTrue
//			ファイルは存在しない
//		ModFalse
//			ファイルは存在しないかわからない
//
//	EXCEPTIONS
//		なし

//static
ModBoolean
ModOsDriver::File::isNotFound(const ModUnicodeString& path)
{
	return ModOsDriver::File::isNotFound(const_cast<ModUnicodeString&>(path).getString(ModOs::Process::getEncodingType()));
}

// static
ModBoolean
ModOsDriver::File::isNotFound(const char* path)
{
	try {

		(void) ModOsDriver::File::access(path, ModOs::accessFile);

	} catch (ModException& exception) {

		ModErrorHandle::reset();

		switch (exception.getErrorNumber()) {
		case ModOsErrorFileNotFound:
			return ModTrue;
		}
	}
	return ModFalse;
}

//	FUNCTION public
//	ModOsDriver::File::rename -- ファイル名を変更する
//
//	NOTES
//
//	ARGUMENTS
//		const char*				old
//			変更される元のファイル名
//		const char*				new
//			新しいファイル名
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			パス名として 0 が指定されている
//		ModOsErrorPermissionDenied
//			指定されたパス名にアクセスできない
//			指定されたパス名のファイルを削除可能な権利がない
//		ModOsErrorFileExist
//			
//		ModOsErrorInterrupt
//			シグナルにより中断された
//		ModOsErrorIsDirectory
//			指定された新しい名前はディレクトリーであるが
//			元の名前はファイルである
//		ModOsErrorFileNotFound
//			指定された元のファイルが存在しない、
//		ModOsErrorTooLongFilename
//			指定されたパス名が長すぎる
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

//static
void
ModOsDriver::File::rename(const ModUnicodeString& oldpath,
						  const ModUnicodeString& newpath)
{
	ModOsDriver::File::rename(
		const_cast<ModUnicodeString&>(oldpath).getString(ModOs::Process::getEncodingType()),
		const_cast<ModUnicodeString&>(newpath).getString(ModOs::Process::getEncodingType()));
}

// static
void
ModOsDriver::File::rename(const char* oldpath, const char* newpath)
{
	if (oldpath == 0 || newpath == 0)

		// 削除するファイルのパス名が指定されていない

		ModThrowOsError(ModCommonErrorBadArgument);

	static const char	func_rename[] = "rename";

	if (::rename(oldpath, newpath) == -1) {
		int saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_rename, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}
}

//	FUNCTION public
//	ModOsDriver::File::getFullPathName --
//		与えられたパス名を絶対パス名に変換する
//
//	NOTES
//
//	ARGUMENTS
//		char*				src
//			絶対パス名へ変換するパス名
//		ModCharString&		dst
//		ModUnicodeString&	dst
//			得られた絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			指定したパス名が不正である
//		ModOsErrorPermissionDenied
//			カレントワーキングディレクトリーの
//			親ディレクトリーの読み出し許可がない
//			(ModOsDriver::Process::getcwd より)
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた
//			(ModOsDriver::Process::getcwd より)
//		その他
//			ModKanjiCode::jjTransfer の例外参照

// static
void
ModOsDriver::File::getFullPathName(const char* src, ModCharString& dst)
{
	ModUnicodeString s;
	ModOsDriver::File::getFullPathName(src, s);
	dst = s.getString(ModOs::Process::getEncodingType());
}

//static
void
ModOsDriver::File::getFullPathName(const ModUnicodeString& src, ModUnicodeString& dst)
{
	if (isFullPathName(src)) {
		// 絶対パス名が与えられているので、
		// 与えられたものをそのまま返す
		dst = src;

	} else {

		// カレントワーキングディレクトリーを得て、
		// それをベースとした絶対パス名を得る

		ModUnicodeString cCwd;
		ModOsDriver::Process::getcwd(cCwd);

		getFullPathName(cCwd, src, dst);
	}
}

void
ModOsDriver::File::getFullPathName(const char* src, ModUnicodeString& dst)
{
	if (src == 0) {
		ModThrowOsError(ModCommonErrorBadArgument);
	}
	ModUnicodeString cPath(src, ModOs::Process::getEncodingType());
	getFullPathName(cPath, dst);
}

//static
void
ModOsDriver::File::
getFullPathName(const ModUnicodeString& cBase_,
				const ModUnicodeString& cPath_,
				ModUnicodeString& cResult_)
{
	// cBase_ は絶対パスが渡されていなければならない
	; ModAssert(isFullPathName(cBase_));
	// cPath_ は相対パスが渡されていなければならない
	; ModAssert(!isFullPathName(cPath_));

	if (cBase_.getLength() > ModPathMax - cPath_.getLength() - 1)

		// 最終的に得られる絶対パス名が長すぎる

		ModThrowOsError(ModCommonErrorBadArgument);

	cResult_.reallocate(cBase_.getLength() + cPath_.getLength() + 1);
	cResult_.clear();
	cResult_.append(cBase_);
	cResult_.append(getPathSeparator());
	cResult_.append(cPath_);
}

// FUNCTION public
//	src::ModOsDriver::File::isFullPathName -- 文字列が絶対パス名を表すか得る
//
// NOTES
//
// ARGUMENTS
//	const char* pszPath_
//	
// RETURN
//	ModBoolean
//
// EXCEPTIONS

//static
ModBoolean
ModOsDriver::File::
isFullPathName(const char* pszPath_)
{
	if (pszPath_ == 0) {
		ModThrowOsError(ModCommonErrorBadArgument);
	}
	ModUnicodeString cPath(pszPath_, ModOs::Process::getEncodingType());
	return isFullPathName(cPath);
}

// FUNCTION public
//	src::ModOsDriver::File::isFullPathName -- 文字列が絶対パス名を表すか得る
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cPath_
//	
// RETURN
//	ModBoolean
//
// EXCEPTIONS

//static
ModBoolean
ModOsDriver::File::
isFullPathName(const ModUnicodeString& cPath_)
{
	ModSize	iLength = cPath_.getLength();
	if (iLength == 0 || iLength > ModPathMax) {
		// 指定されたパス名が空文字列か長すぎる
		ModThrowOsError(ModCommonErrorBadArgument);
	}

	// 先頭がセパレーターならパス名
	return (cPath_[0] == getPathSeparator()) ? ModTrue : ModFalse;
}


//	FUNCTION public
//	ModOsDriver::File::getParentPathName --
//		与えられたパス名のファイルまたはディレクトリーの
//		親ディレクトリーの絶対パス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		char*				src
//			親ディレクトリーの絶対パス名を取得する
//			ファイルまたはディレクトリーのパス名
//		ModUnicodeString&	dst
//			得られた親ディレクトリーの絶対パス名
//
//	RETURN
//		true
//			指定したファイルまたはディレクトリーには
//			親ディレクトリーが存在する
//			ただし、パス名上存在するだけで実在するか未確認
//		false
//			指定したファイルまたはディレクトリーには
//			親ディレクトリーが存在しない
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			指定したパス名が不正である
//			(ModOsDriver::File::getFullPathName より)
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた
//			(ModOsDriver::File::getFullPathName より)

//static
ModBoolean
ModOsDriver::File::getParentPathName(const ModUnicodeString& src, ModUnicodeString& dst)
{
	return ModOsDriver::File::getParentPathName(const_cast<ModUnicodeString&>(src).getString(ModOs::Process::getEncodingType()), dst);
}

// static
ModBoolean
ModOsDriver::File::
getParentPathName(const char* path, ModUnicodeString& parent)
{
	// 与えられたパス名を絶対パス名に変換する

	ModOsDriver::File::getFullPathName(path, parent);

	// パス名の末尾に区切り文字があれば、
	// すべて取り除く

	const char	sep = ModOsDriver::File::getPathSeparator();
	ModUnicodeChar*	slash;
	while ((slash = parent.rsearch(sep)) != 0 &&
		   slash + 1 == parent.getTail())
		parent.truncate(slash);

	if (!slash || (const ModUnicodeChar*) parent == slash)

		// * パス名に区切り文字がひとつもなくなったとき
		// * パス名の先頭の区切り文字を見つけたとき
		//
		// のいずれかの場合、与えられたパス名には
		// 親ディレクトリーが含まれていなかったことになる
		
		return ModFalse;

	// パス名中の親ディレクトリーより後の部分を抹消する

	parent.truncate(slash);

	return ModTrue;
}		

//	FUNCTION public
//	ModOsDriver::File::mkdir -- ディレクトリーを生成する
//
//	NOTES
//
//	ARGUMENTS
//		char*				path
//			生成するディレクトリーのパス名
//		int					mode
//			指定されたとき
//				生成するディレクトリーのアクセス権を表す値
//				ModOs::PermissionMode の論理積を指定する
//			指定されないとき
//				ModOs::ownerModeMask が指定されたものとみなす
//		ModBoolean			recursive
//			ModTrue
//				生成するディレクトリーの親ディレクトリーが存在しないとき、
//				親ディレクトリーも生成する
//				このとき、生成される親ディレクトリーのアクセス権は、
//				mode | ModOs::ownerWriteMode | ModOs::ownerExecuteMode になる
//			ModFalse または指定されないとき
//				生成するディレクトリーの親ディレクトリーが存在しなければ、
//				例外 ModOsErrorFileNotFound が発生する
//
//	RETURN
//		なし	
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			パス名として 0 が指定されている
//		ModOsErrorPermissionDenied
//			指定されたパス名にアクセスできない
//			または指定されたパス名の親ディレクトリーは書き込み不可である
//		ModOsErrorFileExist
//			生成しようとしているディレクトリーは既に存在する
//		ModOsErrorNotSpace
//			ファイルシステムのリソースが足りない
//		ModOsErrorFileNotFound
//			指定されたパス名の親ディレクトリーが存在しない、
//			またはディレクトリーでない
//		ModOsErrorTooLongFilename
//			指定されたパス名が長すぎる
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

//static
void
ModOsDriver::File::mkdir(const ModUnicodeString& path, int mode, ModBoolean recursive)
{
	ModOsDriver::File::mkdir(const_cast<ModUnicodeString&>(path).getString(ModOs::Process::getEncodingType()), mode, recursive);
}

// static
void
ModOsDriver::File::mkdir(const char* path, int mode, ModBoolean recursive)
{
	if (path == 0)

		// 生成するディレクトリーを表すパス名が指定されていない

		ModThrowOsError(ModCommonErrorBadArgument);

	if (recursive) {

		// 与えられたディレクトリーの親ディレクトリーのパス名を得て、
		// 親ディレクトリーが実在するか調べる

		ModUnicodeString	parent;
		if (ModOsDriver::File::getParentPathName(path, parent) &&
			ModOsDriver::File::isNotFound(
				parent.getString(ModOs::Process::getEncodingType())))

			// 親ディレクトリーが存在しないとき、
			// 親ディレクトリーを生成する
			//
			//【注意】	親ディレクトリーは、
			//			与えられたアクセス権に所有者が書き込み、
			//			実行可を加えたもので生成しないと、
			//			その中にディレクトリーを生成できない

			ModOsDriver::File::mkdir(
				parent.getString(ModOs::Process::getEncodingType()),
				mode | ModOs::ownerWriteMode | ModOs::ownerExecuteMode,
				ModTrue);
	}

	// 指定されたパス名のディレクトリーを生成する

	static const char	func_mkdir[] = "mkdir";

	if (::mkdir(path, mode) == -1) {
		int	saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_mkdir, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}
}

//	FUNCTION public
//	ModOsDriver::File::rmdir -- ディレクトリーを削除する
//
//	NOTES
//
//	ARGUMENTS
//		char*				path
//			削除するディレクトリーのパス名
//
//	RETURN
//		なし	
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			パス名として 0 が指定されている
//		ModOsErrorNotDirectory
//			指定されたパス名はディレクトリーでない
//		ModOsErrorNotEmpty
//			指定されたディレクトリーが空でない
//			または、カレントディレクトリーである
//			または、指定されたパス名の最後が '.' で終わっている
//		ModOsErrorFileNotFound
//			指定されたパス名のファイルが存在しない、
//			または、指定されたパス名の親ディレクトリーが存在しない
//		ModOsErrorPermissionDenied
//			指定されたパス名にアクセスできない
//			または指定されたパス名の親ディレクトリーは書き込み不可である
//			または指定されたパス名の親ディレクトリーに
//			S ビットが立ち、所有者でない
//			または指定されたパス名のディレクトリーが存在し、
//			所有者でなく書き込み不可である
//			または実行者がスーパーユーザーでない
//		ModOsErrorTooLongFilename
//			指定されたパス名が長すぎる
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

//static
void
ModOsDriver::File::rmdir(const ModUnicodeString& path)
{
	ModOsDriver::File::rmdir(const_cast<ModUnicodeString&>(path).getString(ModOs::Process::getEncodingType()));
}

// static
void
ModOsDriver::File::rmdir(const char* path)
{
	if (path == 0)

		// 削除するディレクトリーのパス名が指定されていない

		ModThrowOsError(ModCommonErrorBadArgument);

	static const char	func_rmdir[] = "rmdir";

	if (::rmdir(path) == -1) {
		int	saved = errno;
		ModErrorNumber	n;
		switch (saved) {
		case EINVAL:		//【注意】	ModCommonErrorBadArgument にしない
		case EEXIST:		//【注意】	ModOsErrorFileExist にしない

			n = ModOsErrorNotEmpty;			break;

		case ENOTDIR:		//【注意】	ModOsErrorFileNotFound にしない

			n = ModOsErrorNotDirectory;		break;

		default:
			n = ModOsDriver::File::getErrorNumber(
				saved, func_rmdir, srcFile, __LINE__);
		}
		ModThrowOs(n, _Error::getErrorLevel(saved),
				   saved);
	}
}

//
// FUNCTION 
// ModOs::File::rmAll -- 指定されたファイル以下をすべて削除する (static)
//
// NOTES
//	指定されたファイルがファイルであってもディレクトリであっても、
//	とにかく全部削除する。もしディレクトリならばそれ以下の
//	ファイル、サブディレクトリをすべて削除する。
//	本関数はファイル、ディレクトリのどちらでも引数にとれるが、
//	プライベート関数rmAllFilesInDirectory()は、ディレクトリだけを引数に
//	とる。
//	本関数の実行はコマンドrm -rf の実行にほぼ相当する。
//
// ARGUMENTS
//	const char* path
//		対象ディレクトリ名
//  ModBoolean forceFlag
//		ModTrueなら対象のディレクトリが存在しなくてもエラーにしない
//
// RETURN
//	なし	
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		引数エラー
//	その他
//		ModOsDriver::File::rmAllFilesInDirectory()と全く同じなので参照のこと。
//

//static
void
ModOsDriver::File::rmAll(const ModUnicodeString& path, ModBoolean forceFlag)
{
	ModOsDriver::File::rmAll(const_cast<ModUnicodeString&>(path).getString(ModOs::Process::getEncodingType()), forceFlag);
}

void
ModOsDriver::File::rmAll(const char* path, ModBoolean forceFlag)
{
	if (path == 0)
		ModThrowOsError(ModCommonErrorBadArgument);

	// ここで errno を ENOENT にしておかないと続けて rmAll したときに
	// readdir_r でエラー番号が不定になることがある。
	// 98/08/06
	errno = ENOENT;

	// ファイルかどうかを調べる
	ModBoolean result;
	try {
		result = ModOsDriver::File::isDirectory(path);
	} catch (ModException& exception) {
		if (forceFlag == ModTrue
			&& exception.getErrorNumber() == ModOsErrorFileNotFound) {
			// ディレクトリがない場合、forceFlag が True ならこれで終り
			ModErrorHandle::reset();
			return;
		}
		ModRethrow(exception);
	}
	if (result == ModTrue) {
		// ディレクトリだったらそれ以下のファイルを調べて全部消してから
		// ディレクトリを消す
		ModOsDriver::File::rmAllFilesInDirectory(path);
		ModOsDriver::File::rmdir(path);
	} else {
		// もしファイルだったら消すだけで返る
		ModOsDriver::File::unlink(path);
	}
}

//	FUNCTION public
//	ModOsDriver::File::copy -- ファイルをコピーする
//
//	NOTES
//
//	ARGUMENTS
//		char*				srcPath
//			コピー元のファイルのパス名
//		char*				dstPath
//			コピー先のファイルのパス名
//		int					flag
//			指定されたとき
//				コピー先のファイルをどのようにオープンするかを表す値
//				ModOs::OpenFlag のうち、以下の値の論理積を指定する
//
//				* ModOs::writeThroughFlag
//				* ModOs::truncateFlag
//				* ModOs::exclusiveFlag
//
//			指定されないとき
//				ModOs::truncateFlag が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			コピー元またはコピー先のファイルのパス名として 0 が指定されている
//		ModCommonErrorNotSupported
//			コピー元のファイルがシンボリックリンクである
//		ModOsErrorPermissionDenied
//			コピー元またはコピー先のパス名にアクセスできない
//			またはコピー先の親ディレクトリーは書き込み不可である
//			またはコピー元のファイルは読み出し不可である
//			またはコピー先のファイルは書き込み不可である
//			(ModOsDriver::File::open より)
//		ModOsErrorIsDirectory
//			コピー元またはコピー先のファイルがディレクトリーである
//		ModOsErrorFileNotFound
//			コピー元のファイルが存在しない
//		ModOsErrorNotSpace
//			ファイルシステムのリソースが足りない
//			(ModOsDriver::File::open または ModOsDriver::File::write より)
//		ModOsErrorTooLongFilename
//			コピー元またはコピー先のパス名が長すぎる
//		ModOsErrorTooBigFile
//			コピー先のファイルのファイルサイズが上限を超えた
//			(ModOsDriver::File::write より)
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

//static
void
ModOsDriver::File::copy(const ModUnicodeString& srcPath, const ModUnicodeString& dstPath, int flag)
{
	ModOsDriver::File::copy(
		const_cast<ModUnicodeString&>(srcPath).getString(ModOs::Process::getEncodingType()),
		const_cast<ModUnicodeString&>(dstPath).getString(ModOs::Process::getEncodingType()),
		flag);
}

// static
void
ModOsDriver::File::copy(const char* srcPath, const char* dstPath, int flag)
{
	if (srcPath == 0 || dstPath == 0)

		// コピー元またはコピー先のファイルのパス名が指定されていない

		ModThrowOsError(ModCommonErrorBadArgument);

	int	saved;

	// コピー元のファイルの種類を調べる
	//
	//【注意】	指定されたパス名がシンボリックリンクのときでも、
	//			参照先でなくそのものの情報を求めるようにする

	static const char	func_lstat[] = "lstat";
	struct stat srcBuf;

	if (::lstat(srcPath, &srcBuf) == -1) {
		saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_lstat, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}

	switch (srcBuf.st_mode & S_IFMT) {
	case S_IFDIR:

		// コピー元がディレクトリーのときはコピーできない

		ModThrowOs(ModOsErrorIsDirectory,
				   _Error::getErrorLevel(EISDIR),
				   0);

	case S_IFLNK:

		// コピー元がシンボリックリンクのときは
		// 未実装である

		ModThrowOsError(ModCommonErrorNotSupported);
	}

	//【注意】	コピー元のファイルを読み出す権利があるかは
	//			オープン時に分かるので、ここでは調べない

	flag &= ModOs::writeThroughFlag |
		    ModOs::truncateFlag | ModOs::exclusiveFlag;

	// コピー先のファイルの種類を調べる
	//
	//【注意】	指定されたパス名がシンボリックリンクのときでも、
	//			参照先でなくそのものの情報を求めるようにする

	struct stat dstBuf;

	if (::lstat(dstPath, &dstBuf) == -1) {
		saved = errno;
		ModErrorNumber	n = ModOsDriver::File::getErrorNumber(
			saved, func_lstat, srcFile, __LINE__);

		if (n != ModOsErrorFileNotFound)
			ModThrowOs(n, _Error::getErrorLevel(saved),
					   saved);

		// コピー先のファイルが存在しないときは、
		// 生成するのでエラーにしない

	} else {

		// コピー先のファイルが存在する

		if (flag & ModOs::exclusiveFlag)

			// コピー先のファイルが存在するとき、
			// エラーにする必要がある

			ModThrowOs(ModOsErrorFileExist,
					   _Error::getErrorLevel(EEXIST),
					   0);

		switch (dstBuf.st_mode & S_IFMT) {
		case S_IFDIR:

			// コピー先がディレクトリーのときはコピーできない

			ModThrowOs(ModOsErrorIsDirectory,
					   _Error::getErrorLevel(EISDIR),
					   0);
		}

		//【注意】	コピー先のファイルへ書き出す権利があるかは
		//			オープン時に分かるので、ここでは調べない
	}

	// コピー元のファイルをオープンする

	ModOsDriver::File	src;
	src.open(srcPath, ModOs::readOnlyFlag);

	// コピー先のファイルをオープンする
	//
	//【注意】	コピー先のファイルが存在しなければ、
	//			書き出し可能なアクセス権でまず生成する

	ModOsDriver::File	dst;
	dst.open(dstPath,
			 ModOs::createFlag | ModOs::writeOnlyFlag | flag,
			 ModOs::ownerReadMode | ModOs::ownerWriteMode);

	// コピー元のファイルの内容を先頭から読み込みながら、
	// コピー先のファイルへ書き出していく

	const ModSize	size = 1024 * 8;
	char			buf[size];
	ModSize			n;

	while ((n = src.read(buf, size)) > 0)
		(void) dst.write(buf, n);

	// コピー先のファイルのアクセス権をコピー元と同じにする

	ModOsDriver::File::chmod(dstPath, srcBuf.st_mode);
}

// static
void
ModOsDriver::File::copy(
	const ModUnicodeString& srcPath,
	const ModUnicodeString& dstPath, int mode, int flag)
{
	ModThrowOsError(ModCommonErrorNotSupported);
}

//
// FUNCTION 
// ModOs::File::copyAll -- 指定されたディレクトリをすべてコピーする (static)
//
// NOTES
//	第一引数に指定されたディレクトリ以下のファイルやディレクトリを
//  第二引数に指定されたディレクトリ以下にコピーする。
//  どちらの引数もディレクトリでなければエラーとなる。
//	本関数の実行はコマンドcp -r p1/* p2/ の実行にほぼ相当する。
//
// ARGUMENTS
//	const char* sourcePath
//		コピー元ディレクトリ名
//  const char* targetPath
//		コピー先ディレクトリ名
//
// RETURN
//	なし	
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		引数エラー
//	ModCommonErrorNotSupported
//		シンボリックリンクがあった
//	その他
//		ModOsDriver::File::copyAllFilesInDirectory()と全く同じなので参照
//

//static
void
ModOsDriver::File::copyAll(const ModUnicodeString& sourcePath, const ModUnicodeString& targetPath)
{
	ModOsDriver::File::copyAll(
		const_cast<ModUnicodeString&>(sourcePath).getString(ModOs::Process::getEncodingType()),
		const_cast<ModUnicodeString&>(targetPath).getString(ModOs::Process::getEncodingType()));
}

// static
void
ModOsDriver::File::copyAll(
	const ModUnicodeString& srcPath,
	const ModUnicodeString& dstPath, int mode)
{
	ModThrowOsError(ModCommonErrorNotSupported);
}

void
ModOsDriver::File::copyAll(const char* sourcePath, const char* targetPath)
{
	if (sourcePath == 0 || targetPath == 0)
		ModThrowOsError(ModCommonErrorBadArgument);

	// ファイルかどうかを調べる

	static const char	func_stat[] = "stat";
	struct stat statBuffer;

	if (::stat(sourcePath, &statBuffer) == -1) {

		// コピー先が存在しない => 作るからいい

		int saved = errno;
		if (saved != ENOENT)
			ModThrowOs(ModOsDriver::File::getErrorNumber(
				saved, func_stat, srcFile, __LINE__),
					   _Error::getErrorLevel(saved),
					   saved);
	}

	if ((statBuffer.st_mode & S_IFMT) == S_IFDIR) {
		// ディレクトリだったらコピー先のディレクトリがあるか調べて、
		// なければディレクトリを作る

		ModBoolean created = ModFalse;

		try {
			(void) ModOsDriver::File::access(targetPath, ModOs::accessFile);

		} catch (ModException& exception) {
			if (exception.getErrorNumber() == ModOsErrorFileNotFound) {
				// ディレクトリがない
				ModErrorHandle::reset();
				ModOsDriver::File::mkdir(targetPath, ModOs::ownerModeMask);
				created = ModTrue;
			} else {
				// その他のエラーはそのまま再送出
				ModRethrow(exception);
			}
		}
		try {
			// ディレクトリ以下のファイルを全部コピーする
			ModOsDriver::File::copyAllFilesInDirectory(sourcePath, targetPath);

		} catch (ModException& exception) {

			if (created)
				rmdir(targetPath);
			ModRethrow(exception);
		}

		if (created)
			try {

				// ディレクトリのモードをコピー元と同じにする

				ModOsDriver::File::chmod(targetPath, statBuffer.st_mode);

			} catch (ModException& exception) {

				rmAll(targetPath);
				ModRethrow(exception);
			}
	} else {
		// もしファイルだったら単にコピーする
		ModOsDriver::File::copy(sourcePath, targetPath);
	}
}

//
// FUNCTION private
// ModOs::File::rmAllFilesInDirectory -- ディレクトリ以下をすべて削除する (static)
//
// NOTES
//	指定ディレクトリ以下のファイル、サブディレクトリをすべて削除する。
//	本関数の引数はディレクトリを示すパスでなければならない。
//	ディレクトリかファイルかわからない場合はrmAll()を使用すること。
//
// ARGUMENTS
//	const char* path
//		対象ディレクトリ名
//
// RETURN
//	なし	
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		引数エラー
//	ModOsErrorPermissionDenied
//		指定ディレクトリの読み込み許可がない、ファイルサーチパスにアクセス権がない、書き込み不可などで指定ディレクトリ以下のファイルが削除できない
//	ModOsErrorOpenTooManyFiles
//		オープン中のファイルが多すぎる
//	ModOsErrorFileNotFound
//		指定ファイルは存在しない
//	ModOsErrorFileNotDirectory
//		指定ファイルはディレクトリでない
//	ModOsErrorNotSpace
//		システムファイルテーブルが満杯
//	ModOsErrorBadFileDescriptor
//		ファイルディスクリプタが無効
//	ModOsErrorInterrupt
//		シグナルによって処理中止
//	ModOsErrorTooLongFilename
//		ファイル名が長すぎる
//	ModOsErrorOtherReason
//		呼び出したOS提供関数のその他のエラー
//	その他
//		ModOsDriver::File::rmdir()、ModOsManager::allocateの例外参照
//	
void
ModOsDriver::File::rmAllFilesInDirectory(const char* path)
{
	DIR* information = 0;
	// ディレクトリであることは疑わないものとする
	// pathのNULLチェックも既に済んでいるはず
	try {
		int saved;

		// path以下のファイルを調べる

		static const char	func_opendir[] = "opendir";

		information =
#ifdef MOD_DEBUG
			(ModFakeError::check(func_opendir, -1, EACCES)) ? NULL :
#endif
			::opendir(path);

		if (information == NULL) {

			// ENOENTはコンポーネントがないか、pathがNULLのときなので除外
			// EFAULT, ENOTDIRはこの場合本当は起こらないはずだが

			saved = errno;
			if (saved != ENOENT)
				ModThrowOs((saved == ENOTDIR) ?
							ModOsErrorNotDirectory :
							ModOsDriver::File::getErrorNumber(
								saved, func_opendir, srcFile, __LINE__),
						   _Error::getErrorLevel(saved),
						   saved);
		}

		static const char	func_readdir[] = "readdir";
		struct dirent*	entry;
		const char	sep = ModOsDriver::File::getPathSeparator();
#ifndef MOD_NO_THREAD
		ModSize entrySize =
			sizeof(struct dirent) + pathconf(path, _PC_NAME_MAX) + 1;
		entry =	(struct dirent*) ModOsManager::allocate(entrySize);

		try {
			struct dirent* result;
			while (::readdir_r(information, entry, &result) == 0 && result != 0)
#else
			while ((entry = ::readdir(information)) != NULL)
#endif
			{
				// . と ..は無視する
				const ModUnicodeString	oneDot(".");
				const ModUnicodeString	twoDot("..");

				ModUnicodeString unicodeStr(entry->d_name,
											ModOs::Process::getEncodingType());
				if (unicodeStr == oneDot || unicodeStr == twoDot) {
					continue;
				}
				ModCharString filename(path);
				filename += sep;
				filename += entry->d_name;

				// 更に深く先を再帰的に探して消す

				ModOsDriver::File::rmAll(filename.getString(), ModFalse);
			}
#ifndef MOD_NO_THREAD
		} catch (ModException& exception) {

			ModOsManager::free(entry, entrySize);
			ModRethrow(exception);
		}

		ModOsManager::free(entry, entrySize);
#endif
		// 絶対に最後はエラーになって上記のループを抜ける。普通はENOENT
		// ENOENTは最後という意味

		saved = errno;
		if (saved != ENOENT) {
			; ModAssert(saved != EFAULT);

			//【注意】	マルチプレクサーからの読み出し時に、
			//			EINVAL になる

			ModThrowOs((saved == EINVAL) ?
					   ModOs::getOsErrorOtherReason(
							saved, func_readdir, srcFile, __LINE__) :
						ModOsDriver::File::getErrorNumber(
							saved, func_readdir, srcFile, __LINE__),
					   _Error::getErrorLevel(saved),
					   saved);
		}

		// 先にクローズする
		if (information != NULL) {
			if (::closedir(information) < 0) {
				// 絶対成功するはず。万が一エラーでも無視する。
				; ModAssert(0);
			}
			information = 0;
		}

	} catch (ModException& exception) {
		if (information != 0) {
			::closedir(information);
		}
		ModRethrow(exception);
	}
}
//
// FUNCTION private
// ModOs::File::copyAllFilesInDirectory -- 指定されたディレクトリ以下をすべてコピーする (static)
//
// NOTES
//	第一引数に指定されたディレクトリ以下のファイルやディレクトリを
//  第二引数に指定されたディレクトリ以下にコピーする。
//	本関数の実行はコマンドcp -r dir1/* dir2 の実行にほぼ相当する。
//  cp -r dir1 dir2 ではないことに注意。
//
// ARGUMENTS
//	const char* sourcePath
//		コピー元ディレクトリ名
//  const char* targetPath
//		コピー先ディレクトリ名
//
// RETURN
//	なし	
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		引数エラー
//	ModOsErrorPermissionDenied
//		指定ディレクトリの読み込み許可がない、ファイルサーチパスにアクセス権がない、書き込み不可などで指定ディレクトリ以下のファイルがコピーできない
//	ModOsErrorOpenTooManyFiles
//		オープン中のファイルが多すぎる
//	ModOsErrorFileNotFound
//		指定ファイルは存在しない
//	ModOsErrorFileNotDirectory
//		指定ファイルはディレクトリでない
//	ModOsErrorNotSpace
//		システムファイルテーブルが満杯
//	ModOsErrorBadFileDescriptor
//		ファイルディスクリプタが無効
//	ModOsErrorInterrupt
//		シグナルによって処理中止
//	ModOsErrorTooLongFilename
//		ファイル名が長すぎる
//	ModOsErrorOtherReason
//		呼び出したOS提供関数のその他のエラー
//	その他
//		ModOsDriver::File::rmdir()、ModOsManager::allocateの例外参照
//
void
ModOsDriver::File::copyAllFilesInDirectory(const char* sourcePath, const char* targetPath)
{
	DIR* information = 0;
	// ディレクトリであることは疑わないものとする
	// pathのNULLチェックも既に済んでいるはず
	try {
		int saved;

		// path以下のファイルを調べる

		static const char	func_opendir[] = "opendir";

		information =
#ifdef MOD_DEBUG
			(ModFakeError::check(func_opendir, -1, EACCES)) ? NULL :
#endif
			::opendir(sourcePath);

		if (information == NULL) {

			// ENOENTはコンポーネントがないか、pathがNULLのときなので除外
			// EFAULT, ENOTDIRはこの場合本当は起こらないはずだが

			saved = errno;
			if (saved != ENOENT)
				ModThrowOs((saved == ENOTDIR) ?
							ModOsErrorNotDirectory :
							ModOsDriver::File::getErrorNumber(
								saved, func_opendir, srcFile, __LINE__),
						   _Error::getErrorLevel(saved),
						   saved);
		}

		static const char	func_readdir[] = "readdir";
		struct dirent*	entry;
		const char	sep = ModOsDriver::File::getPathSeparator();
#ifndef MOD_NO_THREAD
		ModSize entrySize =
			sizeof(struct dirent) + pathconf(sourcePath, _PC_NAME_MAX) + 1;
		entry = (struct dirent*) ModOsManager::allocate(entrySize);

		try {
			struct dirent* result;
			while (::readdir_r(information, entry, &result) == 0 && result !=0)
#else
			while ((entry = ::readdir(information)) != NULL)
#endif
			{
				// . と ..は無視する
				const ModUnicodeString	oneDot(".");
				const ModUnicodeString	twoDot("..");
				ModUnicodeString unicodeStr(entry->d_name,
											ModOs::Process::getEncodingType());
				if (unicodeStr == oneDot || unicodeStr == twoDot) {
					continue;
				}
				ModCharString	targetFileName(targetPath);
				if (targetFileName == entry->d_name) {
					// 自分を自分自身の下にコピーしない
					// 		(ユーザーが指定した文字コードと OS が返した
					//  	d_name は同じ文字コードのはずなので
					//  	operator== で比較可能)
					continue;
				}
				targetFileName += sep;
				targetFileName += entry->d_name;

				ModCharString	sourceFileName(sourcePath);
				sourceFileName += sep;
				sourceFileName += entry->d_name;

				// 更に深く先を再帰的に探してコピーする

				ModOsDriver::File::copyAll(sourceFileName.getString(),
										   targetFileName.getString());
			}
#ifndef MOD_NO_THREAD
		} catch (ModException& exception) {

			ModOsManager::free(entry, entrySize);
			ModRethrow(exception);
		}

		ModOsManager::free(entry, entrySize);
#endif
		// 絶対に最後はエラーになって上記のループを抜ける。普通はENOENT
		// ENOENTは最後という意味

		saved = errno;
		if (saved != ENOENT) {
			; ModAssert(saved != EFAULT);

			//【注意】	マルチプレクサーからの読み出し時に、
			//			EINVAL になる

			ModThrowOs((saved == EINVAL) ?
						ModOs::getOsErrorOtherReason(
							saved, func_readdir, srcFile, __LINE__) :
						ModOsDriver::File::getErrorNumber(
							saved, func_readdir, srcFile, __LINE__),
					   _Error::getErrorLevel(saved),
					   saved);
		}

		// クローズする
		if (information != NULL) {
			if (::closedir(information) < 0) {
				// 絶対成功するはず。万が一エラーでも無視する。
				; ModAssert(0);
			}
			information = 0;
		}
	} catch (ModException& exception) {

		if (information != 0) {
			::closedir(information);
		}
		rmAllFilesInDirectory(targetPath);
		ModRethrow(exception);
	}
}

//	FUNCTION public
//	ModOsDriver::File::isDirectory -- パス名がディレクトリーを表すものか調べる
//
//	NOTES
//
//	ARGUMENTS
//		char*				path
//			調べるパス名
//
//	RETURN
//		ModTrue
//			指定されたパス名はディレクトリーを表す
//		ModFalse
//			指定されたパス名はディレクトリーを表さない
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			パス名として 0 が指定されている
//		ModOsErrorPermissionDenied
//			指定されたパス名にアクセスできない
//		ModOsErrorFileNotFound
//			指定されたパス名のファイルが存在しない、
//			または、指定されたパス名の親ディレクトリーが存在しない
//		ModOsErrorTooLongFilename
//			指定されたパス名が長すぎる
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

//static
ModBoolean
ModOsDriver::File::isDirectory(const ModUnicodeString& path)
{
	return ModOsDriver::File::isDirectory(const_cast<ModUnicodeString&>(path).getString(ModOs::Process::getEncodingType()));
}

// static
ModBoolean
ModOsDriver::File::isDirectory(const char* path)
{
	if (path == 0)

		// ディレクトリーかを調べるファイルのパス名が指定されていない

		ModThrowOsError(ModCommonErrorBadArgument);

	// ファイルの情報を求める
	//
	//【注意】	指定されたパス名がシンボリックリンクのときでも、
	//			その参照先の情報を求めるようにする

	static const char	func_stat[] = "stat";
	struct stat buf;

	if (::stat(path, &buf) == -1) {
		int saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_stat, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}

	return ((buf.st_mode & S_IFMT) == S_IFDIR) ? ModTrue : ModFalse;
}

// FUNCTION 
// ModOs::File::isTapeDevice -- テープデバイスかどうかを調べる (static)
//
// NOTES
//	指定ファイルがテープデバイスであるかどうかを調べ、テープデバイスならば
//	ModTrueを返す。
//
// ARGUMENTS
//	const char* path
//		対象デバイス名
//
// RETURN
//	テープデバイスならばModTrue、テープデバイスでなければModFalseを返す
//
// EXCEPTIONS
//	ModOsErrorPermissionDenied
//		指定パスがアクセスできない
//	ModCommonErrorBadArgument
//		引数エラー
//	ModOsErrorTooLongFilename
//		ファイル名が長すぎる
//	ModOsErrorOtherReason
//		呼び出した関数のその他のエラー

//static
ModBoolean
ModOsDriver::File::isTapeDevice(const ModUnicodeString& path)
{
	return ModOsDriver::File::isTapeDevice(const_cast<ModUnicodeString&>(path).getString(ModOs::Process::getEncodingType()));
}

// static
ModBoolean
ModOsDriver::File::isTapeDevice(const char* path)
{
	if (path == 0)

		// 調べるパス名が指定されていない

		ModThrowOsError(ModCommonErrorBadArgument);

	// 指定されたパス名のファイルの情報を得る
	// ただし、指定されたパス名のファイルがシンボリックリンクのときは、
	// 参照先を調べるために ::lstat(2) でなく ::stat(2) をつかう

	static const char	func_stat[] = "stat";
	struct stat	buf;

	if (::stat(path, &buf) == -1) {
		int	saved = errno;
		ModErrorNumber	n = ModOsDriver::File::getErrorNumber(
			saved, func_stat, srcFile, __LINE__);

		if (n != ModOsErrorFileNotFound)
			ModThrowOs(n, _Error::getErrorLevel(saved),
					   saved);

		// 指定されたパス名のファイルが存在しないときは、
		// テープデバイスでないことにする

		return ModFalse;
	}

	return ((buf.st_mode & S_IFMT) == S_IFCHR) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModOsDriver::File::getFileSizeLimit -- ファイルサイズの最大値を得る
//
// NOTES
// この関数はOSで定められているファイルサイズの最大値を得るために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// ファイルサイズの最大値
//
// EXCEPTIONS
// コードとしてModThrowは入っているが例外は発生しないはずである
//

ModFileSize
ModOsDriver::File::getFileSizeLimit()
{
	static const char	func_getrlimit[] = "getrlimit";
	struct rlimit		rl;

	if (::getrlimit(RLIMIT_FSIZE, &rl) == -1) {
		int saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_getrlimit, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}

	return (ModFileSize) rl.rlim_max;
}

//
// FUNCTION
// ModOsDriver::File::setNumberOfFileUnlimited -- ディスクリプタを無制限にする
//
// NOTES
// この関数は同時にopenできるディスクリプタの制限数を上限値にするために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// コードとしてModThrowは入っているが例外は発生しないはずである
//

void
ModOsDriver::File::setNumberOfFileUnlimited()
{
	static const char	func_getrlimit[] = "getrlimit";
	static const char	func_setrlimit[] = "setrlimit";
	struct rlimit		rl;
	int					saved;

	// 現在の制限値を得る
	if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
		saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_getrlimit, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}

	// 最大値にする
	rl.rlim_cur = rl.rlim_max;
	if (setrlimit(RLIMIT_NOFILE, &rl) == -1) {
		saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_setrlimit, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}
}

//
// FUNCTION
// ModOsDriver::File::getDirectoryEntry -- ディレクトリ内のファイル名を求める
//
// NOTES
// 指定されたディレクトリ内のファイル名の一覧を求める
// 呼出し側で entry の各要素および entry の解放を行う必要がある
//
// ARGUMENTS
//	const char* path
//		対象ディレクトリ名
//  ModCharString*** entrys	[OUT]
//		path 内のファイルの名前
//	int* n_entrys			[OUT]
//		entrys の件数
//
// RETURN
// なし
//
// EXCEPTIONS
//	ModOsErrorNotDirectory
//

// static
void
ModOsDriver::File::getDirectoryEntry(const char* path,
									 ModCharString*** entrys, int* n_entrys)
{
	if (path == 0)

		// ディレクトリのパス名が指定されていない

		ModThrowOsError(ModCommonErrorBadArgument);

	// UnicodeString版を実行する
	ModUnicodeString	wpath(path, ModOs::Process::getEncodingType());
	ModUnicodeString**	files;
	int	n_files;
	ModOsDriver::File::getDirectoryEntry(wpath, &files, &n_files);
	*n_entrys = n_files;
	if (n_files == 0)
		return;

	// files を entrys へ

	ModCharString**	ep = 0;
	try {
		ep = (ModCharString**) ModOsManager::allocate(sizeof(ModCharString*) * n_files);
		*entrys = ep;

		for (ModUnicodeString** fp = files; fp < files + n_files; ++fp) {
			ModCharString*	tmpp = new ModCharString((*fp)->getString(ModOs::Process::getEncodingType()));
			*ep++ = tmpp;
		}

	} catch (ModException& exception) {
		//
		if (ep != 0) {
			ModOsManager::free(ep, sizeof(ModCharString*) * n_files);
			while (ep > *entrys)
				delete *(--ep);
		}

		// files の解放
		for (ModUnicodeString** fp = files; fp < files + n_files; ++fp)
			delete *fp;
		ModOsManager::free(files, sizeof(ModUnicodeString*) * n_files);

		ModRethrow(exception);
	}

	// files の解放
	for (ModUnicodeString** fp = files; fp < files + n_files; ++fp)
		delete *fp;
	ModOsManager::free(files, sizeof(ModUnicodeString*) * n_files);

	return;
}

//static
void
ModOsDriver::File::getDirectoryEntry(const ModUnicodeString& path, ModUnicodeString*** entrys, int* n_entrys)
{
	const char* cpath = const_cast<ModUnicodeString&>(path).getString(ModOs::Process::getEncodingType());
	if (ModOsDriver::File::isDirectory(cpath) != ModTrue)
		ModThrowOs(ModOsErrorNotDirectory,
				   _Error::getErrorLevel(ENOTDIR),
				   0);

	DIR*	dirp = ::opendir(cpath);
	if (dirp == 0) {
		int		saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
						saved, "opendir", srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}

	ModVector<ModUnicodeString*>	files;
	struct dirent* entry = 0;
#ifndef MOD_NO_THREAD
	ModSize entrySize =
				sizeof(struct dirent) + pathconf(cpath, _PC_NAME_MAX) + 1;
#endif
	try {
#ifndef MOD_NO_THREAD
		entry = (struct dirent*) ModOsManager::allocate(entrySize);

		struct dirent* result;
		while (::readdir_r(dirp, entry, &result) == 0 && result !=0)
#else
		while ((entry = ::readdir(dirp)) != 0)
#endif
		{
			// . と ..は無視する
			const ModUnicodeString	oneDot(".");
			const ModUnicodeString	twoDot("..");

			ModAutoPointer<ModUnicodeString> unicodeStr =
				new ModUnicodeString(entry->d_name,
									 ModOs::Process::getEncodingType());
			if (*unicodeStr == oneDot || *unicodeStr == twoDot)
				continue;

			// ファイル名を記録する
			files.pushBack(unicodeStr.release());
		}
	} catch (ModException& exception) {
#ifndef MOD_NO_THREAD
		if (entry != 0)
			ModOsManager::free(entry, entrySize);
#endif
		::closedir(dirp);
		for (ModVector<ModUnicodeString*>::Iterator i = files.begin();
			 i != files.end(); ++i)
			delete *i, *i = 0;
		ModRethrow(exception);
	}

#ifndef MOD_NO_THREAD
	ModOsManager::free(entry, entrySize);
#endif

	::closedir(dirp);

	*n_entrys = files.getSize();
	if (*n_entrys > 0) {
		// ModVector -> char** する
		ModUnicodeString**	ep;
		ep = (ModUnicodeString**) ModOsManager::allocate(sizeof(ModUnicodeString*) * files.getSize());
		*entrys = ep;
		for (ModVector<ModUnicodeString*>::Iterator i = files.begin();
			 i != files.end(); ++i)
			*ep++ = *i;
	}

	return;
}

// ****** ソケット関連 ******

//
// VARIABLE
//	ModOs::Socket::hostnameLengthMax -- ホスト名の最大長さ
// NOTES
//
const ModSize ModOs::Socket::hostnameLengthMax = NI_MAXHOST;

//
// FUNCTION
// ModOsDriver::Socket::Socket -- 仮想OSソケットクラスのコンストラクタ
// 
// NOTES
//	仮想OSのソケットのコンストラクタ。必要な初期化を行なう。
//	インターネットドメイン、ストリームタイプ、TCP/IPプロトコルとして
//	初期化する。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		初期化前に限りModCommonInitialize::checkAndInitializeの例外参照
//

ModOsDriver::Socket::Socket()
	: socket(-1), acceptSocket(0), acceptSocketNum(0), fds(0), ipv6(ModFalse)
{
	// 初期化が必要
	ModCommonInitialize::checkAndInitialize();
}

//	FUNCTION public
//	ModOsDriver::Socket::~Socket -- 仮想 OS のソケットクラスのデストラクター
// 
//	NOTES
//		ソケットがオープンされたままならば、クローズされる
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

ModOsDriver::Socket::~Socket()
{
	// 念のため、クローズしておく

	this->close();
}

//
// FUNCTION
// ModOsDriver::Socket::open -- ソケットのオープン
// 
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし

void
ModOsDriver::Socket::open()
{
}

//
// FUNCTION
// ModOsDriver::Socket::close -- ソケットのクローズ
// 
// NOTES
//	仮想OSのソケットをクローズする。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModOsErrorSocketInvalid
//		ソケットが有効でない
//	ModOsErrorInterrupt
// 		シグナルによって処理中止(Solaris), キャンセルされている(NT)
//	(MosOsErrorWouldBlock)			*** NTのみ
// 		ノンブロック設定時にブロックされる場合(将来)
//	(ModOsErrorWinSockNetworkDown)	*** NTのみ
// 	ネットワークがダウン
//	(ModOsErrorInProgress)			*** NTのみ
// 		現在処理中
//	ModOsErrorOtherReason
//		OSが提供する関数が設定したその他のエラー
//

void
ModOsDriver::Socket::close()
{
	static const char	func_close[] = "close";

	if (this->acceptSocket)
	{
		for (int i = 0; i < getSocketSize(); ++i) {
			if (::close(this->acceptSocket[i].socket) == -1) {
				int	saved = errno;
				ModThrowOs(ModOsDriver::Socket::getErrorNumber(
							   saved, func_close, srcFile, __LINE__),
						   _Error::getErrorLevel(saved),
						   saved);
			}
		}
		delete[] this->acceptSocket, this->acceptSocket = 0;
		delete[] this->fds, this->fds = 0;
		this->acceptSocketNum = 0;
	}
	else if (this->socket != -1)
	{
		if (::close(this->socket) == -1) {
			int	saved = errno;
			ModThrowOs(ModOsDriver::Socket::getErrorNumber(
						   saved, func_close, srcFile, __LINE__),
					   _Error::getErrorLevel(saved),
					   saved);
		}
		this->socket = -1;
	}
}

//
// FUNCTION
// ModOsDriver::Socket::bind -- 名前をソケットにバインドする
// 
// NOTES
//	名前を仮想OSのソケットにバインドする。
//
// ARGUMENTS
//	const char* hostname
//		バインドするホスト名(default 0)
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModOsErrorPermissionDenied
//		ソケットを作成する権利がない
//	ModOsErrorIOError
//		IOエラーが起きた
//	ModOsErrorResourceExhaust
//		処理に必要なストリームリソースが不足
//	ModOsErrorSocketInvalid
//		無効なソケットである
//	ModOsErrorInvalidAddress
//		アドレスが無効
//	ModOsErrorAddressInUse
//		指定アドレスは既に使用されている
//	ModOsErrorAlreadyBound
//		既にバインドされている
//	ModOsErrorNotSocket
//		ソケットではない
//	(ModCommonErrorBadArgument)		*** NTのみ
//		引数エラー
//	(ModOsErrorInProgress)			*** NTのみ
// 		現在処理中
//	(ModOsErrorWinSockNetworkDown)	*** NTのみ
//		ネットワークがダウン
//	(ModOsErrorSystemMemoryExhaust)	*** NTのみ
//		ソケット用バッファが確保できない
//	ModOsErrorOtherReason
//		bindが設定したその他のエラー
//

void
ModOsDriver::Socket::bind(int port, int mark, int option,  const char* hostname)
{
	struct addrinfo hint;
	::memset(&hint, 0, sizeof(hint));
	hint.ai_family = PF_UNSPEC;	// デフォルトはv4,v6の両方
	if (option & ModOs::only_v4)
		hint.ai_family = PF_INET;
	else if (option & ModOs::only_v6)
		hint.ai_family = PF_INET6;
	hint.ai_socktype = SOCK_STREAM;	// TCP/IPのみ
	hint.ai_flags = AI_PASSIVE;

	char p[16];
	::sprintf(p, "%d", port);
	struct addrinfo* addrinfo = 0;
	int saved = 0;
	if ((saved = ::getaddrinfo(hostname, p, &hint, &addrinfo)) != 0) {
		ModThrowOs(_Error::printOtherMessage(
					   saved, "getaddrinfo", srcFile, __LINE__),
				   ModErrorLevelError, saved);
	}

	// IPv6があるかどうかチェックする
	bool isv6 = false;
	for (struct addrinfo* ai = addrinfo; ai != 0; ai = ai->ai_next) {
		if (ai->ai_family == PF_INET6 && ai->ai_socktype == SOCK_STREAM) {
			isv6 = true;
			break;
		}
	}

	int i = 0;
	
	try {

		struct addrinfo* ai = addrinfo;
		while (ai != 0) {
		
			if ((ai->ai_family != PF_INET && ai->ai_family != PF_INET6) ||
				ai->ai_socktype != SOCK_STREAM) {
				// TCP/IPのみ
				ai = ai->ai_next;
				continue;
			}

			// UNIX系の場合、IPv6かIPv4のどちらかしかバインドしない
			if (isv6 == true && ai->ai_family != PF_INET6) {
				ai = ai->ai_next;
				continue;
			}
			if (isv6 == false && ai->ai_family != PF_INET) {
				ai = ai->ai_next;
				continue;
			}
		
			int s = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
			if (s == -1) {
				saved = errno;
				
				if (isv6 == true) {

					// RHEL5でIPv4のシングルスタックの場合でも、
					// なぜか getaddrinfo で IPv6 も得られてしまう。
					// 当然 socket を実行するとエラーになるので、
					// それを想定したコードにする

					isv6 = false;	// IPv4限定
					ai = addrinfo;	// はじめから
					continue;
				}
				
				ModThrowOsError(ModOsDriver::Socket::getErrorNumber(
									saved, "socket", srcFile, __LINE__));
			}

			try {
#ifdef OS_RHLINUX6_0
				// RHEL4.0 以上ならOK
				if ((option & ModOs::only_v6) && ai->ai_family == PF_INET6) {
					int on = 1;
					if (::setsockopt(s, IPPROTO_IPV6,
									 IPV6_V6ONLY, &on, sizeof(on)) == -1) {
						saved = errno;
						ModThrowOsError(ModOsDriver::Socket::getErrorNumber(
											saved, "setsockopt",
											srcFile, __LINE__));
					}
				}
#endif

				if (option & ModOs::reuseAddress) {
					int on = 1;
					if (::setsockopt(s, SOL_SOCKET,
									 SO_REUSEADDR, &on, sizeof(on)) == -1) {
						saved = errno;
						ModThrowOsError(ModOsDriver::Socket::getErrorNumber(
											saved, "setsockopt",
											srcFile, __LINE__));
					}
				}

				if (option & ModOs::keepAlive) {
					int option = 1;
					if (::setsockopt(s, SOL_SOCKET, SO_KEEPALIVE,
									 &option, sizeof(option)) == -1) {
						saved = errno;
						ModThrowOsError(ModOsDriver::Socket::getErrorNumber(
											saved, "setsockopt",
											srcFile, __LINE__));
					}

#ifdef OS_RHLINUX6_0
					option = 600;	// 10分間隔
					ModParameter parameter(ModFalse);
					int idle = 0;
					if (parameter.getInteger(idle, "TcpKeepIdleTime")
						== ModTrue) {
						option = idle;
					}
					if (::setsockopt(s, IPPROTO_TCP, TCP_KEEPIDLE,
									 &option, sizeof(option)) == -1) {
						saved = errno;
						ModThrowOsError(ModOsDriver::Socket::getErrorNumber(
											saved, "setsockopt",
											srcFile, __LINE__));
					}

					option = 10;
					if (::setsockopt(s, IPPROTO_TCP, TCP_KEEPINTVL,
									 &option, sizeof(option)) == -1) {
						saved = errno;
						ModThrowOsError(ModOsDriver::Socket::getErrorNumber(
											saved, "setsockopt",
											srcFile, __LINE__));
					}

					option = 5;
					if (::setsockopt(s, IPPROTO_TCP, TCP_KEEPCNT,
									 &option, sizeof(option)) == -1) {
						saved = errno;
						ModThrowOsError(ModOsDriver::Socket::getErrorNumber(
											saved, "setsockopt",
											srcFile, __LINE__));
					}
#endif
				}

				if (option & ModOs::readTimeout) {
					struct timeval tv;
					tv.tv_sec = 24 * 60 * 60; // 24 hours
					tv.tv_usec = 0;
					
					ModParameter parameter(ModFalse);
					int timeout = 0;
					if (parameter.getInteger(timeout, "TcpReadTimeoutInterval")
						== ModTrue) {
						tv.tv_sec = timeout;
					}

					if (tv.tv_sec > 0)
					{
						if (::setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,
									 &tv, sizeof(tv)) == -1) {
							saved = errno;
							ModThrowOsError(ModOsDriver::Socket::getErrorNumber(
											saved, "setsockopt",
											srcFile, __LINE__));
						}
					}
				}
				
				if (::bind(s, (struct sockaddr*)ai->ai_addr,
						   ai->ai_addrlen) == -1) {
					saved = errno;
					ModThrowOsError((saved == EINVAL) ?
									ModOsErrorAlreadyBound :
									ModOsDriver::Socket::getErrorNumber(
										saved, "bind", srcFile, __LINE__));
				}

				if (::listen(s, 5) == -1) {
					saved = errno;
					ModThrowOsError(ModOsDriver::Socket::getErrorNumber(
										saved, "listen", srcFile, __LINE__));
				}
			
			} catch (ModException& exception) {

				::close(s);
				ModRethrow(exception);
			}

			if (ai->ai_family == AF_INET6)
				this->ipv6 = ModTrue;

			// bind できたソケットを acceptSocket に追加する
			int n = this->acceptSocketNum + 1;
			AcceptSocket* tmp = new AcceptSocket[n];
			if (this->acceptSocket) {
				Memory::copy(tmp, this->acceptSocket,
							 sizeof(AcceptSocket) * this->acceptSocketNum);
			}
	
			AcceptSocket* t = tmp + this->acceptSocketNum;
			t->socket = s;
			t->mark = mark;

			if (this->acceptSocket) {
				delete[] this->acceptSocket;
			}
	
			this->acceptSocket = tmp;
			this->acceptSocketNum = n;

			ai = ai->ai_next;
			++i;
		}
		
	} catch (ModException& exception) {
		::freeaddrinfo(addrinfo);
		ModRethrow(exception);
	}

	::freeaddrinfo(addrinfo);

	if (i == 0) {
		// 1つも bind できなかった
		ModThrowOsError(ModOsDriver::Socket::getErrorNumber(
							saved, "ModOsDriver::Socket::bind",
							srcFile, __LINE__));
	}

	// bind できた分の struct pollfd を確保する
	this->fds = new struct pollfd[this->acceptSocketNum];
	Memory::reset(this->fds,
				  sizeof(struct pollfd) * this->acceptSocketNum);
}

//
// FUNCTION
// ModOsDriver::Socket::getpeername -- 相手のアドレス名の取得
// 
// NOTES
//	ソケット接続先のアドレス名を返す
//
// ARGUMENTS
//	なし
//
// RETURN
//	接続先のアドレスを表す文字列を返す
//
// EXCEPTIONS
//	ModOsErrorSystemMemoryExhaust
//		実行に必要なシステムメモリが不足
//	ModOsErrorResourceExhaust
//		処理に必要なストリームリソースが不足
//	ModOsErrorSocketInvalid
//		無効なソケットである
//	ModOsErrorNotSocket
//		ソケットではない
//	(ModOsErrorWinSockNetworkDown)	*** NTのみ
//		ネットワークがダウン
//	(ModOsErrorInProgress)			*** NTのみ
// 		現在処理中
//	ModOsErrorOtherReason
//		getpeernameのその他のエラー
//

ModCharString
ModOsDriver::Socket::getpeername()
{
	struct sockaddr_storage	buffer;
#if MOD_CONF_LIB_POSIX == 2
	// Linux
	socklen_t size = sizeof(buffer);
#else
	// Linux(MOD_CONF_LIB_POSIX == 1)
	int size = sizeof(buffer);
#endif

	if (::getpeername(this->socket, (struct sockaddr*)&buffer, &size)
		== -1) {
		int saved = errno;
		ModThrowOsError(ModOsDriver::Socket::getErrorNumber(
							saved, "getpeername", srcFile, __LINE__));
	}

	char hostname[NI_MAXHOST];
	size = sizeof(buffer);
	if (::getnameinfo((struct sockaddr*)&buffer, size, hostname, NI_MAXHOST,
					  0, 0, NI_NUMERICHOST) != 0) {
		int saved = errno;
		ModThrowOsError(ModOsDriver::Socket::getErrorNumber(
							saved, "getnameinfo", srcFile, __LINE__));
	}

	return ModCharString(hostname);
}

// FUNCTION
// ModOsDriver::Socket::accept -- ソケットにコネクションを受ける
// 
// NOTES
//	ソケットにコネクションを受けとり、新たなソケットを返す。
//
// ARGUMENTS
//	なし
//
// RETURN
//	新たなソケットオブジェクトへのポインタ
//
// EXCEPTIONS
//	ModOsErrorResourceExhaust
//		処理に必要なストリームリソースが不足
//	ModOsErrorSocketInvalid
//		無効なソケットである
//	ModOsErrorInterrupt
//		シグナルによって処理中止、またはキャンセルされている
//	ModOsErrorSystemMemoryExhaust
//		実行に必要なシステムメモリが不足
//	ModOsErrorNotSocket
//		ディスクリプタがソケットではない
//	(ModOsErrorTooOpenFiles)		*** NTのみ
//		キューが空ではなく、これ以上ディスクリプタが作れない
//	(ModOsErrorWinSockNetworkDown)	*** NTのみ
//		ネットワークがダウン
//	(ModCommonErrorBadArgument)		*** NTのみ
//		引数エラー
//	(ModOsErrorInProgress)	*** 	NTのみ
// 		現在処理中でブロック
//	(MosOsErrorWouldBlock)			*** NTのみ
// 		ノンブロック設定時にブロックされる場合(将来)
//	ModOsErrorOtherReason
//		システムコールacceptのその他のエラー
//		

ModOs::Socket*
ModOsDriver::Socket::accept(int& mark)
{
	int i = 0;
	int n = getSocketSize();
	for (i = 0; i < n; ++i) {
		if (this->fds[i].revents & POLLIN) {
			// すでに select 済のソケットがあるので、それを accept する
			this->fds[i].revents = 0;
			break;
		}
	}

	if (i == n) {
		// select 済のソケットがないので、ここで無限に接続を待つ

		for (i = 0; i < n; ++i) {
			this->fds[i].fd = this->acceptSocket[i].socket;
			this->fds[i].events = POLLIN;
			this->fds[i].revents = 0;
		}
		for (;;) {
			// timeout に -1 を指定すると無限の意味になる
			
			if (::poll(this->fds, n, -1) == -1) {
				int	saved = errno;
				ModErrorNumber	n =
					ModOsDriver::Socket::getErrorNumber(
						saved, "select", srcFile, __LINE__);
				if (n == ModOsErrorInterrupt)
					continue;
				ModThrowOsError(n);
			}
			break;
		}

		for (i = 0; i < n; ++i) {
			// 読み出せるソケットの内、最初のものを accept する
			// accept するもののフラグをクリアする
			
			if (this->fds[i].revents & POLLIN) {
				this->fds[i].revents = 0;
				break;
			}
		}
	}

	if (i == n) {
		// ありえないけど、念のためチェックする
		ModUnexpectedThrowOs();
	}
	
	// 接続要求を待ち、要求を受けると、
	// 要求元と接続されたソケットオブジェクトを生成する

	ModAutoPointer<ModOsDriver::Socket>	sock(new ModOsDriver::Socket());
	struct sockaddr_storage buffer;
	socklen_t size = sizeof(buffer);

	for (;;) {
		sock->socket = 
#ifdef MOD_DEBUG
			(ModFakeError::check("accept")) ? -1 :
#endif
			::accept(this->acceptSocket[i].socket,
					 (struct sockaddr*)&buffer, &size);

		if (sock->socket == -1) {
			int	saved = errno;
			ModErrorNumber	n =	ModOsDriver::Socket::getErrorNumber(
				saved, "accept", srcFile, __LINE__);
			if (n == ModOsErrorInterrupt)
				continue;
			ModThrowOs(n, _Error::getErrorLevel(saved), saved);
		}
		break;
	}

	mark = acceptSocket[i].mark;

	if (buffer.ss_family == AF_INET6)
		sock->ipv6 = ModTrue;

	return (ModOs::Socket*) sock.release();
}

//
// FUNCTION
// ModOsDriver::Socket::connect -- コネクトする
// 
// NOTES
//	ホスト名を指定してコネクションをはり、初期化する。
//	情報は内部変数serviceに格納される。
//
// ARGUMENTS
//	const char*	name
//		コネクトするホスト名
//	int port
//		ポート番号
//	int option
//		ModOs::SocketOptionの論理和
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModOsErrorPermissionDenied
//		アクセス失敗
//	ModOsErrorResourceExhaust
//		処理に必要なストリームリソースが不足
//	ModOsErrorIOError
//		IOエラーが起きた
//	ModOsErrorHostNotFound
//		ホストがみつからない
//	ModOsErrorServerFailed
//		サーバーエラーもしくはNon-Authoritive Host not found
//	ModOsErrorNetworkUnreach
//		ネットワークにつながらない
//	ModOsErrorAddressInUse
//		アドレスは既に使用されている
//	ModOsErrorInterrupt
//		シグナルを受け処理中止, またはキャンセルされている
//	ModOsErrorConnectAlready
//		既にコネクト進行中
//	ModOsErrorSocketInvalid
//		無効なソケットである
//	ModOsErrorConnectRefused
//		コネクトが拒否された
//	ModOsErrorInProgress
//		現在処理中
//	ModOsErrorIsConnected
//		既にコネクトされている
//	ModOsErrorConnectTimeout
//		コネクション成立までにタイムアウト
//	(ModOsErrorWinSockNetworkDown)	   *** NTのみ
//		ネットワークがダウン
//	(ModOsErrorInvalidAddress)	   *** NTのみ
//		指定アドレスは無効
//	(ModOsErrorSystemMemoryExhaust	   *** NTのみ)
//		ソケット用バッファが確保できない
//	(ModOsErrorNotSocket	   *** NTのみ)
//		ソケットではない
//	(MosOsErrorWouldBlock	   *** NTのみ)
// 		ノンブロック設定時にブロックされる場合(将来)
//	ModOsErrorOtherReason
//		呼び出した関数のその他のエラー
//	その他
//		ModOsManager::allocateの例外参照
//

void 
ModOsDriver::Socket::connect(const char* hostname, int port, int option)
{
	struct addrinfo hint;
	::memset(&hint, 0, sizeof(hint));
	hint.ai_family = PF_UNSPEC;	// デフォルトはv4,v6の両方
	if (option & ModOs::only_v4)
		hint.ai_family = PF_INET;
	else if (option & ModOs::only_v6)
		hint.ai_family = PF_INET6;
	hint.ai_socktype = SOCK_STREAM;	// TCP/IPのみ

	struct addrinfo* addrinfo;
	char p[16];
	::sprintf(p, "%d", port);
	int saved = 0;
	if ((saved = ::getaddrinfo(hostname, p, &hint, &addrinfo)) != 0) {
		ModThrowOs(_Error::printOtherMessage(
					   saved, "getaddrinfo", srcFile, __LINE__),
				   ModErrorLevelError, saved);
	}

	int i = 0;
	for (struct addrinfo* ai = addrinfo; ai != 0; ai = ai->ai_next) {
		
		if ((ai->ai_family != PF_INET && ai->ai_family != PF_INET6) ||
			ai->ai_socktype != SOCK_STREAM)
			// TCP/IPのみ
			continue;

		// ソケットを作成する
		this->socket
			= ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (this->socket == -1) {
			saved = errno;
			ModMessageSelection::error(srcFile, __LINE__).getStream()
				<< "(ERROR) socket (errno=" << saved << ")" << ModEndl;
			continue;
		}

		if (option & ModOs::reuseAddress) {
			
			// オプションを設定する
			
			int on = 1;
			if (::setsockopt(this->socket, SOL_SOCKET,
							 SO_REUSEADDR, &on, sizeof(on)) == -1) {
				saved = errno;
				ModMessageSelection::error(srcFile, __LINE__).getStream()
					<< "(ERROR) setsockopt (errno=" << saved << ")" << ModEndl;
				::close(this->socket);
				this->socket = -1;
				continue;
			}
		}

		// コネクトする
		if (::connect(this->socket, ai->ai_addr, ai->ai_addrlen)
			== -1) {
			saved = errno;
			ModMessageSelection::error(srcFile, __LINE__).getStream()
				<< "(ERROR) connect (errno=" << saved << ")" << ModEndl;
			::close(this->socket);
			this->socket = -1;
			continue;
		}

		if (ai->ai_family == AF_INET6)
			this->ipv6 = ModTrue;

		// 1つ connect できればそれでいい
		++i;
		break;
	}

	::freeaddrinfo(addrinfo);

	if (i == 0) {
		// 1つも connect できなかった
		ModThrowOs(ModOsDriver::Socket::getErrorNumber(
					   saved, "ModOsDriver::Socket::connect",
					   srcFile, __LINE__),
				   _Error::getErrorLevel(saved), saved);
	}
}

//
// FUNCTION
// ModOsDriver::Socket::read -- ソケットからデータを読む
// 
// NOTES
//	ソケットからデータを1回読む。実際に読みこめたサイズを返す。
//
// ARGUMENTS
//	void* buffer
//		読み込み先のバッファへのポインタ
//	ModSize size
//		読み込むサイズ
//
// RETURN
//	実際に読み込んだサイズ
//
// EXCEPTIONS
//	ModOsErrorResourceExhaust
//		処理に必要なストリームリソースが不足
//	ModOsErrorSystemMemoryExhaust
//		実行に必要なシステムメモリが不足
//	ModOsErrorIOError
//		IOエラーが起きた
//	ModOsErrorSocketInvalid
//		無効なソケットである(バインドされていない)
//	ModOsErrorInterrupt
//		シグナルによって処理中止、またはキャンセルされている
//	ModOsErrorNotSocket
//		ディスクリプタがソケットではない
//	ModOsErrorWouldBlock
// 		ノンブロック設定時にブロックされる場合(将来)
//	(ModOsErrorWinSockNetworkDown)		*** NTのみ
//		ネットワークがダウン)			*** NTのみ
//	(ModOsErrorNotConnect)				*** NTのみ
//		コネクトされていない
//	(ModOsErrorInProgress)				*** NTのみ
//		現在処理中
//	(ModOsErrorWinSockConnectAborted)	*** NTのみ
//		接続が何かの原因で切れ、使えなくなった。(ソケットの作り直しが必要)
//	ModOsErrorOtherReason
//		recvのその他のエラー
//

ModSize
ModOsDriver::Socket::read(void* buffer, ModSize size)
{
	// quantifyで調べると::recvはgetMessageを呼び出しており、
	// サイクル数も多くて遅い。readを使うように変更する。
	// int realsize = ::recv(this->socket, (char*)buffer, size, 0);

	static const char	func_read[] = "read";
retry:
	int	realsize =
#ifdef MOD_DEBUG
		(ModFakeError::check(func_read)) ? -1 :
#endif
		::read(this->socket, (char*) buffer, size);

	if (realsize == -1) {
		int	saved = errno;
		ModErrorNumber	n =
			ModOsDriver::Socket::getErrorNumber(
				saved, func_read, srcFile, __LINE__);
		if (n == ModOsErrorInterrupt)
			goto retry;
		if (n == ModOsErrorWinSockConnectAborted)
		{
			// Windowsで、このエラーが発生した場合は接続先がcloseしたということ
			// なので、EOFをあらわす 0 をリターンする
			
			return 0;
		}
			
		ModThrowOs(n, _Error::getErrorLevel(saved),
				   saved);
	}
	return (ModSize) realsize;
}

//
// FUNCTION
// ModOsDriver::Socket::write -- ソケットにデータを書き込む
// 
// NOTES
//	ソケットにデータを1回書き込む。実際に書き込んだサイズを返す。
//
// ARGUMENTS
//	void* buffer
//		書き込むデータが格納されているバッファへのポインタ
//	ModSize size
//		書き込むサイズ
//
// RETURN
//	実際に書き込んだサイズ
//
// EXCEPTIONS
//	ModOsErrorResourceExhaust
//		処理に必要なストリームリソースが不足
//	ModOsErrorSocketInvalid
//		ソケットが無効(バインドされていないのも含む)
//	ModOsErrorInterrupt
//		シグナルによって処理中止、またはキャンセルされている
//	ModOsErrorNotSocket
//		ソケットではない
//	MosOsErrorWouldBlock
// 		ノンブロック設定時にブロックされる場合(将来)
//	ModOsErrorSystemMemoryExhaust
//		実行に必要なシステムメモリが不足
//	(ModOsErrorInProgress)			*** NTのみ
// 		前の処理中でブロック
//	(ModCommonErrorBadArgument)		*** NTのみ
//		引数エラー
//	(ModOsErrorNotConnect)			*** NTのみ
//		コネクトされていない
//	(ModOsErrorWinSockConnectAborted)	*** NTのみ
//		接続が何かの原因で切れ、使えなくなった。(ソケットの作り直しが必要)
//	(ModOsErrorWinSockNetworkDown)		*** NTのみ
//		ネットワークがダウン
//	ModOsErrorOtherReason
//		send()が設定したその他のエラー
//

ModSize
ModOsDriver::Socket::write(const void* buffer, ModSize size)
{
	// OS依存の send 関数を呼び出す
	// 最後の引数は何にすればいいのか
	// MSG_OOB, MSG_DONTROUTE

	// quantifyで調べると::sendは遅い。writeを使うように変更する。
	// int realsize = ::send(this->socket, (const char*)buffer, size, 0);

	static const char	func_write[] = "write";
retry:
	int	realsize =
#ifdef MOD_DEBUG
		(ModFakeError::check(func_write)) ? -1 :
#endif
		::write(this->socket, (const char*) buffer, size);

	if (realsize == -1) {
		int	saved = errno;
		ModErrorNumber	n =
			ModOsDriver::Socket::getErrorNumber(
				saved, func_write, srcFile, __LINE__);
		if (n == ModOsErrorInterrupt)
			goto retry;
		ModThrowOs(n, _Error::getErrorLevel(saved),
				   saved);
	}
	return (ModSize)realsize;
}

//
// FUNCTION
// ModOsDriver::Socket::select -- ソケットの同期を設定する
// 
// NOTES
//	ソケットの同期をとるため、指定時間待つ。同期がとれた場合には
//	1(ファイルディスクリプタの数)、時間が満了した場合は0を返す。
//	(10 ** 8)秒+1000ミリ秒未満まで指定できる。
//
// ARGUMENTS
//	ModSize second
//		同期を待つ時間を秒単位で指定する。
// 	ModSize millisecond
//		同期を待つ時間をミリ秒単位で指定する。
//
// RETURN
//	時間満了ならば0、そうでなければ1を返す。
//
// EXCEPTIONS
//	ModOsErrorSocketInvalid
//		無効なソケットである
//	ModCommonErrorOutOfRange
//		引数が値の範囲を超えている
//	ModOsErrorInterrupt
//		シグナルによって処理中止、またはキャンセルされている
//	(ModOsErrorWinSockNetworkDown)	*** NTのみ
//		ネットワークがダウン
//	(ModOsErrorInProgress)			*** NTのみ
// 		現在処理中
//	(ModOsErrorNotSocket)			*** NTのみ
//		ソケットではない
//	ModOsErrorOtherReason
//		関数selectが設定したその他のエラー
//

int
ModOsDriver::Socket::
select(ModSize second, ModSize milliSecond)
{
	// readのみ
	
	// 待ち時間の設定

	int timeout = (int) second * 1000 + milliSecond;
	int stat = 1;

	if (this->acceptSocket)
	{
		int i = 0;
		int n = getSocketSize();
		for (i = 0; i < n; ++i) {
			if (this->fds[i].revents & POLLIN)
				// すでに select 済のソケットがあるので、終わり
				break;
		}

		if (i == n) {
			// select 済のソケットがないので、ここで指定時間接続を待つ

			for (i = 0; i < n; ++i) {
				this->fds[i].fd = this->acceptSocket[i].socket;
				this->fds[i].events = POLLIN;
				this->fds[i].revents = 0;
			}

			for (;;) {
			
				stat =
#ifdef MOD_DEBUG
					(ModFakeError::check("poll")) ? -1 :
#endif
					::poll(this->fds, n, timeout);
				if (stat == -1) {
					int	saved = errno;
					ModErrorNumber	n =
						ModOsDriver::Socket::getErrorNumber(
							saved, "poll", srcFile, __LINE__);
					if (n == ModOsErrorInterrupt)
						continue;
					ModThrowOsError(n);
				}

				break;
			}
		}
	}
	else
	{
		// 操作対象となるソケットの
		// ファイルディスクリプターの集合を生成しておく

		struct pollfd fds;
		fds.fd = this->socket;
		fds.events = POLLIN;
		fds.revents = 0;

		for (;;) {

			stat =
#ifdef MOD_DEBUG
				(ModFakeError::check("poll")) ? -1 :
#endif
				::poll(&fds, 1, timeout);

			if (stat == -1) {
				int	saved = errno;
				ModErrorNumber	n =
					ModOsDriver::Socket::getErrorNumber(
						saved, "poll", srcFile, __LINE__);
				if (n == ModOsErrorInterrupt)
					continue;
				ModThrowOsError(n);
			}

			break;
		}
	}
	
	return stat;
}

//
// FUNCTION public
// ModOsDriver::Socket::getHostname -- 仮想OSでホスト名を得る
//
// NOTES
//	staticメソッドであり、直接呼び出す。
//	実行マシンのホスト名を得て、引数hostnameに格納する。
//	バッファhostnameにはホスト名を格納するに十分な
//	サイズをもつ領域を指定する。領域が足りない場合は例外が送出される。
//	ModOs::Socket::hostnameLengthMax+1以上あれば十二分である。
//
// ARGUMENTS
// 	char* hostname
//		ホスト名を格納するバッファ
//	ModSize size
//		バッファサイズ
//
// RETURN
// 	なし
//
// EXCEPTIONS
//	ModCommonErrorBadArgument
//		引数エラー
//	(ModOsErrorInProgress)			*** NTのみ。
//		現在処理中
//	(ModOsErrorWinSockNetworkDown)	*** NTのみ。
//		ネットワークがダウン
//	(ModOsErrorOtherReason)			*** NTのみ。
//		gethostnameが設定したその他のエラー
//

// static
void
ModOsDriver::Socket::getHostname(char* buf, ModSize size)
{
	if (buf == 0 || size == 0) {

		// 求めたホスト名を格納する領域の先頭アドレスや
		// そのサイズとして 0 が指定されている
error:
		ModThrowOsError(ModCommonErrorBadArgument);
	}

	// ホスト名を求める
	//
	//【注意】	::gethostname でなく、システムコールである ::uname を使う

	static const char	func_uname[] = "uname";
	struct utsname		name;

	if (uname(&name) == -1) {
		int saved = errno;
		ModThrowOs(ModOsDriver::Socket::getErrorNumber(
			saved, func_uname, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}

	// 指定された領域に得られたホスト名をコピーする

	ModSize	len = ModCharTrait::length(name.nodename);
	if (len + 1 > size)

		// 得られたホスト名を格納するには
		// 指定された領域では足りない

		goto error;

	ModOsDriver::Memory::copy(buf, name.nodename, len + 1);
}

//	FUNCTION public
//	ModOsDriver::Socket::getErrorNumber --
//		ソケットに関する C の errno を MOD のエラー番号に変換する
// 
//	NOTES
//
//	ARGUMENTS
//		unsigned int	error
//			MOD のエラー番号を得たいソケットに関する C の errno
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
ModOsDriver::Socket::getErrorNumber(unsigned int error)
{
	ModErrorNumber	n;

	switch (error) {

//	Bad file number
	case EBADF:
		n = ModOsErrorSocketInvalid;			break;

//	out of streams resources
	case ENOSR:
		n = ModOsErrorResourceExhaust;			break;

//	Socket operation on non-socket
	case ENOTSOCK:
		n = ModOsErrorNotSocket;				break;

//	Protocol not supported
	case EPROTONOSUPPORT:
		n = ModCommonErrorNotSupported;			break;

	// 以下のエラー番号は ::bind(3N) で発生する

//	Address already in use
	case EADDRINUSE:
		n = ModOsErrorAddressInUse;				break;

//	Can't assign requested address
	case EADDRNOTAVAIL:
		n = ModOsErrorInvalidAddress;			break;

	// 以下のエラー番号は ::connect(3N) で発生する

//	operation already in progress
	case EALREADY:
		n = ModOsErrorConnectAlready;			break;

//	Connection refused
	case ECONNREFUSED:
		n = ModOsErrorConnectRefused;			break;

//	Socket is already connected
	case EISCONN:
		n = ModOsErrorIsConnected;				break;

//	Network is unreachable
	case ENETUNREACH:
		n = ModOsErrorNetworkUnreach;			break;

//	Connection timed out
	case ETIMEDOUT:
		n = ModOsErrorConnectTimeout;			break;

	// 以下のエラー番号は ::recv(3N) で発生する

//	Resource temporarily unavailable
	case EWOULDBLOCK:
		n = ModOsErrorWouldBlock;				break;

// Connection reset by peer
	case ECONNRESET:
		n = ModOsErrorWinSockConnectAborted;	break;
		
	default:
		n = ModOs::getErrorNumber(error);
	}

	return n;
}

// static
ModErrorNumber
ModOsDriver::Socket::getErrorNumber(unsigned int error, const char* func,
									const char* file, int line)
{
	ModErrorNumber	n = ModOsDriver::Socket::getErrorNumber(error);
	switch (n) {
	case ModOsErrorOtherReason:
		ModOs::printOsErrorNumber(error, func, file, line);
		ModOs::printOsErrorMessage(error, file, line);
	}
	return n;
}

// ****** スレッド固有ストレージ関連 ******

//	FUNCTION public
//	ModOsDriver::ThreadSpecificKey::ThreadSpecificKey --
//		仮想 OS のスレッド固有ストレージを表すクラスのコンストラクター
//
//	NOTES
//		POSIX が提供するスレッド固有データ用キーを初期化する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorResourceExhaust
//			システムリソースが不足しているか、
//			プロセスあたりのキー数の上限に達している
//		ModOsErrorSystemMemoryExhaust
//			キーの生成に必要なメモリーがシステムに存在しない
//		ModOsErrorOtherReason
//			その他のエラーが起きた

ModOsDriver::ThreadSpecificKey::ThreadSpecificKey()
{
#ifndef MOD_NO_THREAD
	static const char	func_pthread_key_create[] = "pthread_key_create";

	int stat =
#ifdef MOD_DEBUG
		(ModFakeError::check(func_pthread_key_create, -1, EAGAIN)) ? errno :
#endif
		::pthread_key_create(&_key, NULL);

	if (stat)
		ModThrowOs(ModOs::getErrorNumber(
			stat, func_pthread_key_create, srcFile, __LINE__),
				   _Error::getErrorLevel(stat),
				   stat);
#else
	_value = 0;
#endif
}

//	FUNCTION public
//	ModOsDriver::ThreadSpecificKey::setValue --
//		仮想 OS のスレッド固有ストレージに値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		void*				value
//			設定する値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			値の設定に必要なメモリーがシステムに存在しない

void
ModOsDriver::ThreadSpecificKey::setValue(void* value)
{
#ifndef MOD_NO_THREAD

	// スレッド固有データに値を設定する
	//
	//【注意】	メッセージ出力に影響するので、
	//			この関数に ModFakeError::check() はかけれない

	if (::pthread_setspecific(_key, value))
		ModThrowOs(ModOsErrorSystemMemoryExhaust,
				   _Error::getErrorLevel(ENOMEM),
				   0);
#else
	_value = value;
#endif
}

// ****** スレッド関連 ******

// 静的クラスメンバーの定義

#ifndef MOD_NO_THREAD
int	ModOsDriver::Thread::_signalNumber = SIGUSR2;
ModOsDriver::ThreadSpecificKey*		ModOsDriver::Thread::_selfKey = 0;
#endif

//	FUNCTION public
//	ModOsDriver::Thread::Thread -- 仮想 OS のスレッドクラスのコンストラクター
//
//	NOTES
//		スレッドクラスがコンストラクトされても、
//		スレッドが生成されているわけでない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			必要なメモリーがシステムに存在しない

ModOsDriver::Thread::Thread()
#ifndef MOD_NO_THREAD
	: _joinable(ModFalse),
	  _wrapper(0),
	  _exitCode(0)
{
	// 仮想 OS のスレッド環境の初期化のために
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	// POSIX のスレッド属性を初期化する

	static const char	func_pthread_attr_init[] = "pthread_attr_init";

	int	stat =
#ifdef MOD_DEBUG
		(ModFakeError::check(func_pthread_attr_init, ENOMEM)) ? errno :
#endif
		::pthread_attr_init(&_attribute);

	if (stat) {
		; ModAssert(stat == ENOMEM);
		ModThrowOs(ModOsErrorSystemMemoryExhaust,
				   _Error::getErrorLevel(ENOMEM),
				   0);
	}

	// LWP 結合するスレッドを生成するようにスレッド属性を設定する
	//
	//【注意】	本当は非結合スレッドにしたいのであるが、
	//			そうするとほとんどの場合呼び出しスレッドが
	//			ブロックするまで生成されたスレッドが動かない
	//
	//			引数はおかしくないはずなので、
	//			EINVAL のエラーにならないはず

	(void) ::pthread_attr_setscope(&_attribute, PTHREAD_SCOPE_SYSTEM);
}
#else
{ }
#endif

//	FUNCTION public
//	ModOsDriver::Thread::~Thread -- 仮想 OS のスレッドクラスのデストラクター
//
// 	NOTES
//
//	ARGUMENTS
//		なし
//
// 	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

ModOsDriver::Thread::~Thread()
{
#ifndef MOD_NO_THREAD
	if (_joinable) {

		// 自分自身を介して実行したスレッドの
		// 終了待ちをしていないので、スレッドの終了を待つ
		//
		//【注意】	スレッドの終了待ちをしていないと、
		//			スレッド用の領域が再利用されない
		//
		//【注意】	スレッドが終了していないと、
		//			ラッパーを破棄できないので、
		//			::pthread_detach の呼び出しでは駄目
		//
		//【注意】	エラーは無視する

		(void) ::pthread_join(_id, 0);
		_joinable = ModFalse;
	}

	// 登録されているラッパーがあれば、破棄する

	delete _wrapper, _wrapper = 0;

	// POSIX のスレッド属性を後処理しておく
	//
	//【注意】	引数に与えるスレッド属性はおかしくないはずなので、
	//			EINVAL のエラーにならないはず

	(void) ::pthread_attr_destroy(&_attribute);
#endif
}

//	FUNCTION public
//	ModOsDriver::Thread::initialize --
//		仮想 OS のスレッドクラスを利用するための初期化を行う
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
//		ModOsErrorSystemMemoryExhaust
//			必要なメモリーがシステムに存在しない
//		ModOsErrorResourceExhaust
//			システムリソースが不足しているか、
//			プロセスあたりのキー数の上限に達している
//			(ModOsDriver::ThreadSpecificKey::ThreadSpecficKey より)
//		ModOsErrorOtherReason
//			その他のエラーが起きた
//			(ModOsDriver::ThreadSpecificKey::ThreadSpecficKey より)

// static
void
ModOsDriver::Thread::initialize()
{
#ifndef MOD_NO_THREAD

	// スレッドごとに自分自身を表す
	// 仮想 OS のスレッドを記憶するための
	// スレッド固有ストレージを初期化する

	_selfKey = new ModOsDriver::ThreadSpecificKey();
	; ModAssert(_selfKey);

	// パラメータから kill 用のシグナル番号を得る
	ModParameter	parameter(ModFalse);
	int sig;
	if (parameter.getInteger(sig, "ThreadKillSignal") == ModTrue) {
		// 値の確認は行わない
		_signalNumber = sig;
	}

	if (_signalNumber > 0) {
		// スレッドを強制終了するためのシグナルハンドラーを設定する
		//
		//【注意】	引数はおかしくないはずなので、
		//			EINVAL のエラーにならないはず

#ifdef OS_RHLINUX6_0
		(void) ::signal(_signalNumber, ModOsDriver::Thread::handlerPerProcess);
#else
		(void) ::sigset(_signalNumber, ModOsDriver::Thread::handlerPerProcess);
#endif
	}

#endif
}

//	FUNCTION public
//	ModOsDriver::Thread::terminate --
//		仮想 OS のスレッドクラスを利用した後処理を行う
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
ModOsDriver::Thread::terminate()
{
#ifndef MOD_NO_THREAD

	// スレッドごとに自分自身を表す
	// 仮想 OS のスレッドを記憶するための
	// スレッド固有ストレージを破棄する

	delete _selfKey, _selfKey = 0;

	//【注意】	今のところ、初期化時に設定した
	//			シグナルハンドラーの登録を抹消していない
#endif
}

//	FUNCTION public
//	ModOsDriver::Thread::create -- 与えた関数を実行するスレッドを生成する
//
//	ARGUMENTS
//		ModOs::Thread::Routine	routine
//			生成するスレッドで実行する関数
//		void*				argument
//			生成するスレッドで実行する関数に与える引数
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorSystemMemoryExhaust
//			必要なメモリーがシステムに存在しない
//		ModOsErrorTooManyThread
//			プロセスあたりのスレッド数の上限に達している

void
ModOsDriver::Thread::create(Routine routine, void* argument)
{
#ifndef MOD_NO_THREAD
	if (this->isAlive())

		// 自分自身を介して実行したスレッドがまだ終了していない

		ModThrowOsError(ModOsErrorThreadStillAlive);

	if (_joinable) {

		// 自分自身を介して実行したスレッドは終了しているが、
		// 終了待ちをしていないので、
		// スレッド用の領域が再利用されるようにする
		//
		//【注意】	エラーは無視する

		(void) ::pthread_detach(_id);
		_joinable = ModFalse;
	}
		
	// 登録されているラッパーがあれば、破棄する
	//
	//【注意】	スレッドが終了していないと、
	//			ラッパーは破棄できない

	delete _wrapper, _wrapper = 0;

	// 生成するスレッドで実行する関数と
	// その引数を記録するラッパーを生成する

	_wrapper = new Wrapper(routine, argument, this);
	; ModAssert(_wrapper);

	// スレッドの終了状態を不明にする

	ExitType	saved = this->getExitType();
	_exitType = unknown;

	// スレッドを生成する

	static const char	func_pthread_create[] = "pthread_create";

	int	stat =
#ifdef MOD_DEBUG
		(ModFakeError::check(func_pthread_create, -1, ENOMEM)) ? errno :
#endif
		::pthread_create(&_id, &_attribute, 
						 ModOsDriver::Thread::cover, (void*) _wrapper);

	if (stat) {

		// スレッドの終了状態をもとに戻す

		_exitType = saved;

		ModThrowOs(ModOsDriver::Thread::getErrorNumber(
			stat, func_pthread_create, srcFile, __LINE__),
				   _Error::getErrorLevel(stat),
				   stat);
	}

	// スレッドが生成されたので、
	// スレッドの終了待ちを可能にする

	_joinable = ModTrue;
#else
	ModThrowOsError(ModCommonErrorNotSupported);
#endif
}

//	FUNCTION public
//	ModOsDriver::Thread::join -- スレッドが終了するまで待つ
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		スレッドで実行した関数の返り値
//		または、スレッドの終了コード
//
//	EXCEPTIONS
//		ModOsErrorThreadNotFound
//			存在しないスレッドの終了を待とうとしている
//		ModOsErrorDeadLockInJoin
//			そのスレッドの終了を待つと、デッドロックが起きる
//			例えば、自スレッドの終了を待とうとしている

void*
ModOsDriver::Thread::join()
{
#ifndef MOD_NO_THREAD
	if (_joinable == ModFalse)

		// 自分自身を介してスレッドが生成されていないか、
		// すでに終了待ちをしたので、終了待ちできない

		ModThrowOsError(ModOsErrorThreadNotFound);

	// スレッドの終了を待つ

	static const char	func_pthread_join[] = "pthread_join";

	void*	thread_status = 0;
	int	stat =
#ifdef MOD_DEBUG
		(ModFakeError::check(func_pthread_join, -1, EDEADLK)) ? errno :
#endif
		::pthread_join(_id, (void**) &thread_status);

	if (stat)
		ModThrowOs(ModOsDriver::Thread::getErrorNumber(
			stat, func_pthread_join, srcFile, __LINE__),
				   _Error::getErrorLevel(stat),
				   stat);

	// 一度終了待ちすると、もう終了待ちはできない

	_joinable = ModFalse;

	switch (this->getExitType()) {
#ifdef MOD_DEBUG
	case normally:

		// スレッドで実行した関数の返り値がラッパーにも設定されており、
		// スレッドの終了コードと同じ値のはず

		; ModAssert(_wrapper->getExitStatus() == thread_status);
		break;

	case exited:
	case killed:

		// スレッドを ModOsDriver::Thread::exit() で終了すると、
		// ラッパーでなく自分自身に終了コードが設定され、
		// スレッドの終了コードの格納領域の先頭アドレスと等しいはず

		; ModAssert(&_exitCode == thread_status);
		break;

	case except:

		// スレッドで実行した関数で例外が発生したとき、
		// スレッドの終了コードは 0 であるはず

		; ModAssert(thread_status == 0);
		break;
#endif
	case unknown:

		//【注意】	スレッドが実際には終了しているにもかかわらず
		//			終了状態が不明であることはないはず

		thread_status = 0;
	}

	return thread_status;
#else
	ModThrowOsError(ModCommonErrorNotSupported);
	return 0;
#endif
}

//	FUNCTION public
//	ModOsDriver::Thread::sleep -- 呼び出しスレッドの実行を任意時間停止する
//
//	NOTES
//
//	ARGUMENTS
//		ModSize				milliSecond
//			スレッドの実行を停止する時間(ミリ秒単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorInterrupt
//			シグナルを捕捉したため、中断した

// static
void
ModOsDriver::Thread::sleep(ModSize milliSecond)
{
#ifndef MOD_NO_THREAD
	// スレッドの実行を停止する
	// ただし、その間もシグナルを捕捉することはできる

	static const char	func_nanosleep[] = "nanosleep";

	struct timespec	tv;
	tv.tv_sec = milliSecond / 1000;
	tv.tv_nsec = (milliSecond % 1000) * 1000000;

	if (::nanosleep(&tv, 0) == -1) {
		int	saved = errno;
		ModThrowOs(ModOsDriver::Thread::getErrorNumber(
			saved, func_nanosleep, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}
#else
	ModThrowOsError(ModCommonErrorNotSupported);
#endif
}

//	FUNCTION public
//	ModOsDriver::Thread::exit -- 呼び出しスレッドを終了する
//
//	NOTES
//		呼び出したスレッドを終了させる
//		終了するスレッドが保持する自動オブジェクトのデストラクトは行われない
//
//	ARGUMENTS
//		unsigned int		status
//			終了するスレッドの終了コード
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
ModOsDriver::Thread::exit(unsigned int status)
{
#ifndef MOD_NO_THREAD

	// スレッド固有ストレージに記録してあるこのスレッドを取り出す

	ModOsDriver::Thread*	thread =
		(ModOsDriver::Thread*) _selfKey->getValue();

	void*	p;
	if (thread) {
		p = &(thread->_exitCode = status);
		if (thread->getExitType() != killed)
			thread->_exitType = exited;
	} else {

		// スレッドの実行がはじまって、
		// ModOsDriver::Thread::cover のシグナルハンドラーの設定から
		// スレッド固有ストレージへの記録が終わらない間にシグナルを捕捉した

		// 終了コードは 0 を返す

		p = 0;
	}
	::pthread_exit(p);
#else
	ModThrowOsError(ModCommonErrorNotSupported);
#endif
}

//	FUNCTION public
//	ModOsDriver::Thread::kill -- スレッドを強制終了する
//
//	NOTES
//		スレッドを強制終了させる
//		実行中の処理が正常に終了するかはまったく保証できない
//		ただし、強制終了するスレッドが保持するミューテックスロックは
//		すべてはずされる
//
//		現在、ミューテックスロック待ちしているスレッドを終了させると、
//		異常動作する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTION
//		ModOsErrorKillSelfThread
//			自分自身を強制終了しようとした
//		ModOsErrorThreadNotFound
//			存在しないスレッドを強制終了しようとした
//		ModOsErrorOtherReason
//			その他のエラーが起きた

void
ModOsDriver::Thread::kill()
{
#ifndef MOD_NO_THREAD
	if (_signalNumber == 0)
		// ModOsDriver::Thread::kill() を使用しない設定である
		ModThrowOsError(ModCommonErrorNotSupported);

	if (_joinable == ModFalse)

		// 終了待ちできないスレッドは強制終了できない

		ModThrowOsError(ModOsErrorThreadNotFound);

	if (this->getThreadId() == ModOsDriver::Thread::self())

		// 自分自身を強制終了しようとしている

		ModThrowOsError(ModOsErrorKillSelfThread);

	// スレッドの終了状態を強制終了にしておく
	//
	//【注意】	シグナルハンドラーからは
	//			このメンバーを参照できないのでここで設定する

	ExitType	saved = this->getExitType();
	_exitType = killed;

	// スレッドの強制終了はスレッドに対して _signalNumber を送り、
	// そのシグナルハンドラーの ModOsDriver::Thread::handlerPerProcess で
	// スレッドを終了させるようにする
	// このとき、そのスレッドの保持するミューテックスロックはすべてはずされる
	//
	//【注意】	スレッドの強制終了の方法として、
	//			::pthread_cancel(3T) を使うことができるが、
	//			以下の理由により、シグナルハンドラーを使用した
	//
	//			* ミューテックスロック中でもスレッドが停止するため、
	//			  残りのスレッドの処理ができなくなったり、メッセージ
	//			  ストリームに対する出力中に止まってしまうことがある
	//
	//			* ::pthread_cancel(3T) でも、スレッド終了ハンドラーの
	//			  登録や除去が可能だが、::pthread_cleanup_push(3T) などが
	//			  実はマクロであるために、登録と除去を別関数にすることが
	//			  できない

	static const char	func_pthread_kill[] = "pthread_kill";

	int	stat =
#ifdef MOD_DEBUG
		(ModFakeError::check(func_pthread_kill, -1, ESRCH)) ? errno :
#endif
		::pthread_kill(_id, _signalNumber);

	if (stat) {

		// スレッドの終了状態をもとに戻す

		_exitType = saved;

		ModThrowOs(ModOsDriver::Thread::getErrorNumber(
			stat, func_pthread_kill, srcFile, __LINE__),
				   _Error::getErrorLevel(stat),
				   stat);
	}
#else
	ModThrowOsError(ModCommonErrorNotSupported);
#endif
}

#ifndef MOD_NO_THREAD
//	FUNCTION private
//	ModOsDriver::Thread::cover -- スレッドとして起動される関数
//
//	NOTES
//		OS によりスレッドとして起動できる関数の型が異なるので、
//		この関数をスレッドとして起動し、この関数から与えられた関数に
//		引数を与えて実行することにする
//
//		与えられた関数を実行した結果、返り値やスレッドの終了状態は
//		すべてスレッドの生成側で参照できるようにする
//		同様に、与えられた関数で発生した例外はすべて捕捉され、
//		その例外の種類を生成側で参照できるようにする
//
//	ARGUMENTS
//		void*				arg
//			スレッドで起動する関数とそれに与える引数が格納されたラッパー
//
//	RETURN
//		スレッドで起動した関数の返り値、または終了状態
//
//	EXCEPTIONS
//		なし

// static
void*
ModOsDriver::Thread::cover(void* arg)
{
	if (_signalNumber > 0) {
		// ModOsDriver::Thread::kill で
		// このスレッドを強制終了できるように、
		// _signalNumber を捕捉可能にしておく

		sigset_t signal;
		::sigemptyset(&signal);
		::sigaddset(&signal, _signalNumber);
	}

	// 自分自身を表す仮想 OS のスレッドを得る
	//
	//【注意】	スレッドの終了状態は
	//			ModOsDriver::Thread::create で実行中に設定済みのはず

	Wrapper* wrapper = (Wrapper*) arg;
	ModOsDriver::Thread*	thread =
		(ModOsDriver::Thread*) wrapper->getThread();
	; ModAssert(thread != 0);
	; ModAssert(thread->getExitType() == unknown);

	void*	code = 0;		// 0 は例外が発生したことを表す

	try {
		// このスレッドをスレッド固有ストレージに記録しておく
		//
		//【注意】	この記録が終わる前に、シグナルを受け取る可能性がある
		//			そのときはハンドラーから
		//			ModOsDriver::Thread::exit が呼び出され、
		//			この記録しようとしていたスレッドの
		//			取り出しに失敗するので、そちらで対応する

		_selfKey->setValue((void*) thread);

		// 実行するユーザーの定義した関数と引数を取り出し、
		// 取り出した関数に引数を与えて実行する

		Routine routine = wrapper->getRoutine();
		code = (*routine)(wrapper->getArgument());

		// ユーザー定義関数の返り値をラッパーに設定する

		wrapper->setExitStatus(code);

		// ここまできたときに、
		// スレッドが正常終了したとみなす

		thread->_exitType = normally;

	} catch (ModException& exception) {

		// 例外が発生した

		thread->_exitType = except;

		ModException*	threadException = ModErrorHandle::getException();
		if (threadException && threadException != &exception)

			// 捕捉した例外が格納されている ModException の実体と、
			// このスレッド用の ModException が違う可能性があるので、
			// その場合は、例外の内容を反映する

			*threadException = exception;

		//【注意】	エラー状態は解除しない

	} catch (...) {

		// 理由が分からない例外が発生した
		//
		//【注意】	::pthread_exit を呼び出すと、なぜかここを通る
		//			さらにここで再送すると、以下のようになってしまう
		//
		//			signal fault in critical section
		//			signal number: 11, signal code: 1,                  
		//			fault address: 0x0, pc: 0xef654184, sp: 0xef20b9d0
	    //			ABORT: core dump

		switch (thread->getExitType()) {
		case exited:
		case killed:

			// ::pthread_exit を呼び出したための例外であるはず
			// この場合は、ラッパーに返り値は設定されない

			ModErrorHandle::reset();
			return &thread->_exitCode;
		}

		thread->_exitType = except;

		ModException*	threadException = ModErrorHandle::getException();
		if (threadException) {
			threadException->setError(ModModuleOs,
									  ModCommonErrorUnexpected,
									  ModErrorLevelError,
									  0);
		}

		//【注意】	エラー状態は解除しない
	}

	return code;
}

//	FUNCTION private
//	ModOsDriver::Thread::handlerPerProcess -- MOD のシグナルハンドラー
//
//	NOTES
//		MOD のシグナルハンドラーとして設定する関数で、
//		ModOsDriver::Thread::handlerPerThread を呼び出すことにより、
//		強制終了を指定されたスレッドを実際に終了する
//
//	ARGUMENTS
//		int					signal
//			捕捉したシグナル
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
ModOsDriver::Thread::handlerPerProcess(int sig)
{
	; ModAssert(_signalNumber > 0);

	// シグナルハンドラーの再設定

	(void) ::signal(sig, ModOsDriver::Thread::handlerPerProcess);

	if (sig == _signalNumber) {
		//【注意】	現在はどのスレッドもまったくおなじ
		//			シグナルハンドラーを呼び出す
		//			スレッドごとに変えたいときは、
		//			スレッド固有データにハンドラーを登録し、
		//			それを呼び出すような仕様にすべきである

		ModOsDriver::Thread::handlerPerThread(sig);
	}
}

//	FUNCTION private
//	ModOsDriver::Thread::handlerPerThread -- MOD のシグナルハンドラー下位関数
//
//	NOTES
//		MOD のシグナルハンドラーである
//		ModOsDriver::Thread::handlerPerProcess から呼び出される関数である
//		強制終了の指示を受けて、呼び出したスレッドを終了する
//
//	ARGUMENTS
//		int					signal
//			捕捉したシグナル
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
ModOsDriver::Thread::handlerPerThread(int sig)
{
	; ModAssert(_signalNumber > 0);

	try {
		ModThread*	thread = ModThisThread::getThread();
		if (thread)

			// 自分自身がかけた
			// ミューテックスへのロックがあれば、はずす

			thread->unlockLockingMutex();

	} catch (...) {

		// エラーが起きても無視する

		ModErrorHandle::reset();
	}

	// 自分自身を終了する

	ModOsDriver::Thread::exit(0);
}
#endif

//	FUNCTION public
//	ModOsDriver::Thread::getErrorNumber --
//		スレッドに関する C の errno を MOD のエラー番号に変換する
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int	error
//			MOD のエラー番号を得たいスレッドに関する C の errno
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
ModOsDriver::Thread::getErrorNumber(unsigned int error)
{
	ModErrorNumber	n;

	switch (error) {

//	Resource temporarily unavailable
	case EAGAIN:
		n = ModOsErrorTooManyThreads;			break;

//	Invalid argument
	case EINVAL:
		n = ModCommonErrorOutOfRange;			break;

//	Deadlock condition
	case EDEADLK:
		n = ModOsErrorDeadLockInJoin;			break;

//	No such process
	case ESRCH:
		n = ModOsErrorThreadNotFound;			break;

	default:
		n = ModOs::getErrorNumber(error);
	}

	return n;
}

// static
ModErrorNumber
ModOsDriver::Thread::getErrorNumber(unsigned int error, const char* func,
									const char* file, int line)
{
	ModErrorNumber	n = ModOsDriver::Thread::getErrorNumber(error);
	switch (n) {
	case ModOsErrorOtherReason:
		ModOs::printOsErrorNumber(error, func, file, line);
		ModOs::printOsErrorMessage(error, file, line);
	}
	return n;
}

// ****** クリティカルセクション関連 ******

// ****** ミューテックス関連 ******

//	FUNCTION public
//	ModOsDriver::Mutex::Mutex --
//		仮想 OS のミューテックスクラスのコンストラクター
//
//	NOTES
// 		POSIX が提供する異プロセススレッド間ミューテックスを表すクラスを
//		コンストラクトする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

ModOsDriver::Mutex::Mutex()
{
#ifndef MOD_NO_THREAD
#if MOD_CONF_LIB_PTHREAD == 1
	static const char
		func_pthread_mutexattr_init[] = "pthread_mutexattr_init";
	pthread_mutexattr_t		attr;

	int	stat = ::pthread_mutexattr_init(&attr);
	if (stat)
		ModThrowOs(ModOs::getErrorNumber(
			stat, func_pthread_mutexattr_init, srcFile, __LINE__),
				   _Error::getErrorLevel(stat),
				   stat);

#ifndef OS_RHLINUX6_0
	(void) ::pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
#endif //  OS_RHLINUX6_0
	// ModOsDriver::Mutex は
	// LINUX では ModOsDriver::CriticalSection と同じになる

	//【注意】	引数に与えるミューテックスはおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_init(&_mutex, &attr);

	(void) ::pthread_mutexattr_destroy(&attr);
#endif
#if MOD_CONF_LIB_PTHREAD == 2

	//【注意】	過去の経緯から ModCommonErrorNotSupported にせずに、
	//			プロセス間で同プロセススレッド間ミューテックスを生成する

	(void) ::pthread_mutex_init(&_mutex, 0);
#endif
#endif
}

//	FUNCTION public
//	ModOsDriver::Mutex::lock -- 仮想 OS のミューテックスのロック
//
//	NOTES
//		POSIX が提供する異プロセススレッド間ミューテックスをロックする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

#ifndef MOD_NO_THREAD

//	【注意】	ヘッダーファイルで定義される

#else
void
ModOsDriver::Mutex::lock()
{
	ModThrowOsError(ModCommonErrorNotSupported);
}
#endif

//	****** 条件変数関連 ******

//	FUNCTION public
//	ModOsDriver::ConditionVariable::ConditionVariable --
//		仮想 OS の条件変数クラスのコンストラクター
//
//	NOTES
//		POSIX が提供する同プロセススレッド間条件変数を初期化する
//
//		WIN32API の仕様とあわせるために、
//		シグナル化されたときに待ちスレッドのうち 1 つしか起きなく、
//		かつ、自動的に非シグナル化されない条件変数は初期化できない
//
//	ARGUMENTS
//		ModBoolean			doWakeUpAll
//			ModTrue
//				シグナル化されたときに待ちスレッドがすべて起きる
//			ModFalse または指定されないとき
//				シグナル化されたときに待ちスレッドのうち 1 つだけ起きる
//		ModBoolean			doManualReset
//			ModTrue
//				doWakeUpAll が ModTrue のとき、
//				条件変数が一度シグナル化されると、
//				ModOsDriver::ConditionVariable::reset を
//				呼び出すまで非シグナル化されない
//			ModFalse または指定されないとき
//				ModOsDriver::ConditionVariable::wait を呼び出して
//				シグナル化を待っていたスレッドが起きた時点で、
//				自動的に非シグナル化される
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

ModOsDriver::ConditionVariable::ConditionVariable(ModBoolean doWakeUpAll,
												  ModBoolean doManualReset)
	: _count(0),
	  _waiter(0),
	  _behavior(((doWakeUpAll) ? behaviorWakeUpAll : behaviorDefault) |
				((doWakeUpAll && doManualReset) ?
				 behaviorManualReset : behaviorDefault))
{
#ifndef MOD_NO_THREAD

	//【注意】	引数に与える条件変数はおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_cond_init(&_condition, 0);
#endif
}

//	FUNCTION public
//	ModOsDriver::ConditionVariable::signal --
//		仮想 OS の条件変数をシグナル状態にする
//
//	NOTES
//		POSIX の提供する条件変数をシグナル状態にする
//		すでにシグナル状態の条件変数の場合、
//		シグナル状態のままで、なにもおきない
//
//		この条件変数がシグナル状態になることを
//		複数のスレッドが待っている場合、
//		条件変数の生成時に doWakeUpAll に
//
//		ModTrue を与えると		すべてのスレッド
//		ModFalse を与えると		ある 1 つのスレッド
//
//		が待ち状態から復帰する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ModOsDriver::ConditionVariable::signal()
{
#ifndef MOD_NO_THREAD

	// 内部ミューテックスをロックする

	_mutex.lock();

	// 処理されていないシグナル化の回数を増やした後、
	// 条件変数をシグナル化する
	//
	//【注意】	引数に与える条件変数はおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	if (_behavior & behaviorWakeUpAll) {
		if ((_behavior & behaviorManualReset) == 0)
			_count += (_waiter) ? _waiter : 1;
		else
			_count = 1;
		(void) ::pthread_cond_broadcast(&_condition);
	} else {
		_count++;
		(void) ::pthread_cond_signal(&_condition);
	}

	// 内部ミューテックスのロックをはずす
	//
	//【注意】	シグナル化を待っていたスレッドがあれば、
	//			内部ミューテックスをロックし、復帰することになる

	_mutex.unlock();
#endif
}

//	FUNCTION public
//	ModOsDriver::ConditionVariable::wait --
//		仮想 OS の条件変数がシグナル状態になるまで(一定時間)待つ
//
//	NOTES
//		POSIX の提供する条件変数がシグナル状態になるまで(一定時間)待ち、
//		シグナル状態になると、呼び出しスレッドは待ち状態から復帰する
//
//		待ち状態からの復帰後の条件変数の状態は、
//		条件変数の生成時に doWakeupAll に ModTrue を与え、doManualReset に
//
//		ModTrue を与えると		シグナル状態
//		ModFalse を与えると		非シグナル状態
//
//		にそれぞれなる
//
//	ARGUMENTS
//		ModUInt32			milliSecond
//			指定されたとき
//				シグナル状態になるまで待つ最大待ち時間(ミリ秒単位)
//			指定されないとき
//				シグナル状態になるまで永久に待つ
//
//	RETURN
//		ModTrue
//			シグナル状態になった
//		ModFalse
//			シグナル状態にならなかった
//
//	EXCEPTIONS
//		ModOsErrorInterrupt
//			シグナルによりシグナル化を待ちが中断された

ModBoolean
ModOsDriver::ConditionVariable::wait()
{
#ifndef MOD_NO_THREAD

	// 内部ミューテックスをロックする

	_mutex.lock();

	// 待ちスレッド数を 1 増やす

	_waiter++;

	static const char	func_pthread_cond_wait[] = "pthread_cond_wait";

	while (_count == 0) {

		// 処理されていないシグナル化がないので、
		// 実際にシグナル化を待つ
		//
		//【注意】	シグナル化を待っている間、
		//			内部ミューテックスのロックは自動的にはずれている

		int stat = ::pthread_cond_wait(&_condition,
									   &_mutex.getInternalMutex());

		// 再び、内部ミューテックスはロックされている

		if (stat) {

			// 待ちスレッド数を 1 減らす

			_waiter--;

			// 内部ミューテックスのロックをはずす

			_mutex.unlock();

			ModThrowOs(ModOs::getErrorNumber(
				stat, func_pthread_cond_wait, srcFile, __LINE__),
					   _Error::getErrorLevel(stat),
					   stat);
		}

		// シグナル化された
		//
		// ただし、内部ミューテックスのロックを得るまでに
		// 他のスレッドがシグナル化を待ちにきて、
		// 処理されていないシグナル化の回数が
		// ふたたび 0 になる可能性があるので、
		// 必ず、処理されていないシグナル化の回数を調べる
	}

	// 待ちスレッド数を 1 減らす

	if ((_behavior & behaviorManualReset) == 0)
		if (--_waiter)

			// 処理されていないシグナル化を 1 減らす

			_count--;
		else
			// 待ちスレッドがなくなったので、
			// 処理されていないシグナル化はなくなる

			_count = 0;

	// 内部ミューテックスのロックをはずす

	_mutex.unlock();

	return ModTrue;
#else
	return ModFalse;
#endif
}

ModBoolean
ModOsDriver::ConditionVariable::wait(ModUInt32 milliSecond)
{
#ifndef MOD_NO_THREAD

	// 内部ミューテックスをロックする

	_mutex.lock();

	// 待ちスレッドを 1 増やす

	_waiter++;

	if (_count == 0) {

		// 処理されていないシグナル化がないので、
		// 実際に指定された時間、シグナル化を待つ
		//
		//【注意】	シグナル化を待っている間、
		//			内部ミューテックスのロックは自動的にはずれている

		static const char
			func_pthread_cond_timedwait[] = "pthread_cond_timedwait";

		struct timeval	tv;
		::gettimeofday(&tv, 0);

		struct timespec	abs;
		abs.tv_sec = tv.tv_sec + milliSecond / 1000;
		milliSecond = milliSecond % 1000 + tv.tv_usec / 1000;
		abs.tv_sec += milliSecond / 1000;
		abs.tv_nsec =
			(milliSecond % 1000) * 1000000 + (tv.tv_usec % 1000) * 1000;

		int	stat = ::pthread_cond_timedwait(&_condition,
											&_mutex.getInternalMutex(), &abs);

		// 再び、内部ミューテックスはロックされている

		if (stat || _count == 0) {

			// エラーが起きた
			// または、シグナル化されたときに、
			// 内部ミューテックスのロックを得るまでに
			// 他のスレッドがシグナル化を待ちにきて、
			// 処理されていないシグナル化の回数がふたたび 0 になった

			// 待ちスレッド数を 1 減らし、
			// 内部ミューテックスのロックをはずす

			_waiter--;
			_mutex.unlock();

			switch (stat) {
			case 0:
			case ETIME:
			case ETIMEDOUT:

				// 指定された時間待ったが、シグナル化されなかった
				//
				//【注意】	ロックを得るまでに処理されていない
				//			シグナル化の回数が 0 になったときも、
				//			同様とみなす

				return ModFalse;
			}

			ModThrowOs(ModOs::getErrorNumber(
				stat, func_pthread_cond_timedwait, srcFile, __LINE__),
					   _Error::getErrorLevel(stat),
					   stat);
		}
	}

	// 待ちスレッド数を 1 減らす

	if ((_behavior & behaviorManualReset) == 0)
		if (--_waiter)

			// 処理されていないシグナル化を 1 減らす

			_count--;
		else
			// 待ちスレッドがなくなったので、
			// 処理されていないシグナル化はなくなる

			_count = 0;

	// 内部ミューテックスのロックをはずす

	_mutex.unlock();

	return ModTrue;
#else
	return ModFalse;
#endif
}

//	FUNCTION public
//	ModOsDriver::Memory::alloc -- メモリーを確保する
//
//	NOTES
//		自由記憶領域から指定されたサイズのメモリーを確保する
//
//	ARGUMENTS
//		ModSize				size
//			確保するメモリーのサイズ(B 単位)
//		ModBoolean			noError
//			ModTrue
//				メモリーを確保できなくてもエラー状態にしない
//			ModFalse または指定されないとき
//				メモリーが確保できないときエラー状態になる
//
//	RETURN
//		確保されたメモリーの先頭アドレス
//
//	EXCEPTIONS
//		ModCommonErrorOutOfRange
//			確保しようとしているメモリーのサイズが、
//			システムの物理的な制限を越えているため、メモリーを確保できない
//		ModOsErrorSystemMemoryExhaust
//			現時点では充分なメモリーが存在しないため、メモリーを確保できない
//			再実行すれば、確保できるかもしれない
//		ModOsErrorOtherReason
//			それ以外の原因でメモリーを確保できない

void* 
ModOsDriver::Memory::alloc(ModSize size, ModBoolean noError)
{
	static const char	func_malloc[] = "malloc";
	int	retryCount = 5;
retry:
	void*	address =
#ifdef MOD_DEBUG
		(ModFakeError::check(func_malloc, -1, ENOMEM)) ? 0 :
#endif
		::malloc(size);

	if (!address) {
		int	saved = errno;
		ModErrorNumber	n;
		ModErrorLevel l;
		switch (saved) {
		case ENOMEM:
			n = ModCommonErrorOutOfRange;		break;
		case EAGAIN:
			if (retryCount--)
				goto retry;

			n = ModOsErrorSystemMemoryExhaust;
			l = ModErrorLevelFatal;				break;
		default:
			n = ModOs::getErrorNumber(saved, func_malloc, srcFile, __LINE__);
			l = _Error::getErrorLevel(saved);
		}
		if (noError) {

			// メモリー確保の試みの前のエラーを残すために
			// エラー状態を設定しない

			ModMemoryErrorThrow(ModModuleOs, n, ModErrorLevelError, saved);
		} else
			ModThrowOs(n, l, saved);
	}
	return address;
}

//	FUNCTION public
//	ModOsDriver::Process::getUserName -- 実ユーザーのユーザー名を求める
//
//	NOTES
//		この関数を呼び出したプロセスの実ユーザーのユーザー名を求める
//
//	ARGUMENTS
//		char*		buf
//			求めたユーザー名を格納する領域の先頭アドレス
//		ModSize		size
//			求めたユーザー名を格納する領域のサイズ(B 単位)
//
//	RETURN
// 		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			求めたユーザー名を格納する領域の先頭アドレス、
//			またはそのサイズとして 0 が指定された
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

//static
void
ModOsDriver::Process::getUserName(ModUnicodeString& buf)
{
	char	tmp[ModPathMax + 1];
	ModOsDriver::Process::getUserName(tmp, sizeof(tmp));
	buf = tmp;
}

// static
void
ModOsDriver::Process::getUserName(char* buf, ModSize size)
{
	if (buf == 0 || size == 0) {

		// 求めたユーザー名を格納する領域の先頭アドレスや
		// そのサイズとして 0 が指定されている
error:
		ModThrowOsError(ModCommonErrorBadArgument);
	}

	// まず、実ユーザー ID を得る

	uid_t	uid = ::getuid();

	// その実ユーザー ID の表すユーザーの名前を得る

	struct passwd*	p;
	static const char	func_getpwuid[] = "getpwuid";

#ifndef	MOD_NO_THREAD
	char			tmp[256];
	struct passwd	entry;

	//【注意】	POSIX SEMANTICS を使っていない

#if MOD_CONF_LIB_POSIX == 2 || defined(_POSIX_PTHREAD_SEMANTICS)
	if (::getpwuid_r(uid, &entry, tmp, sizeof(tmp), &p) != 0 || p==0)
#else
	if (::getpwuid_r(uid, p = &entry, tmp, sizeof(tmp)) == NULL)
#endif

#else
	if ((p = ::getpwuid(uid)) == NULL)
#endif
	{
		int	saved = errno;
#if MOD_CONF_LIB_POSIX == 1
		; ModAssert(saved == ERANGE);
#endif
		ModThrowOs(ModOs::getErrorNumber(
			saved, func_getpwuid, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}

	// 指定された領域に得られたユーザー名をコピーする

	ModSize	len = ModCharTrait::length(p->pw_name);
	if (len + 1 > size)

		// 得られたユーザー名を格納するには
		// 指定された領域では足りない

		goto error;

	ModOsDriver::Memory::copy(buf, p->pw_name, len + 1);
}

// 以下、将来廃止します
// static
void
ModOsDriver::getUsername(char* buf, ModSize size)
{
	ModOsDriver::Process::getUserName(buf, size);
}
// 以上、将来廃止します

//	FUNCTION public
//	ModOsDriver::Process::getcwd -- カレントディレクトリーのパス名を求める
//
//	NOTES
//
//	ARGUMENTS
//		char*		buf
//			求めたカレントワーキングディレクトリーの
//			パス名を格納する領域の先頭アドレス
//		ModSize		size
//			求めたカレントワーキングディレクトリーの
//			パス名を格納する領域のサイズ(B 単位)
//			パス名は最大でも ModPathMax 以下のサイズしかない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			求めたパス名を格納する領域の先頭アドレス、
//			またはそのサイズとして 0 が指定された
//		ModOsErrorPermissionDenied
//			カレントワーキングディレクトリーの
//			親ディレクトリーの読み出し許可がない
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

//static
void
ModOsDriver::Process::getcwd(ModUnicodeString& buf)
{
	char	tmp[ModPathMax + 1];
	ModOsDriver::Process::getcwd(tmp, sizeof(tmp));
	buf.allocateCopy(tmp, sizeof(tmp), ModOs::Process::getEncodingType());
}

// static
void
ModOsDriver::Process::getcwd(char* buf, ModSize size)
{
	if (buf == 0 || size == 0)

		// 求めたパス名を格納する領域の先頭アドレスや
		// そのサイズとして 0 が指定されている

		ModThrowOsError(ModCommonErrorBadArgument);

	static const char	func_getcwd[] = "getcwd";

	if (::getcwd(buf, size) == NULL) {
		int	saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_getcwd, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}
}

// 以下、将来廃止します
// static
void
ModOsDriver::File::getcwd(char* buf, ModSize size)
{
	ModOsDriver::Process::getcwd(buf, size);
}
// 以上、将来廃止します

//	FUNCTION public
//	ModOsDriver::Process::chdir -- カレントワーキングディレクトリーを変更する
//
//	NOTES
//
//	ARGUMENTS
//		char*				path
//			新しいカレントワーキングディレクトリーを表すパス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorPermissionDenied
//			新しいカレントワーキングディレクトリーの探索許可がない
//		ModCommonErrorBadArgument
//			引数が不正なアドレスを指している
//		ModOsErrorInterrupt
//			シグナルを受けて、処理を中止した
//		ModOsErrorFileNotFound
//			新しいカレントワーキングディレクトリーは存在しない
//			または、それはディレクトリーでない
//		ModOsErrorTooLongFilename
//			新しいカレントワーキングディレクトリーのパス名が長過ぎる
//		ModOsErrorOtherReason
//			その他のエラーが起きた
//			例えば、ファイルシステムの読み出しエラーが起きた、
//			新しいカレントワーキングディレクトリーのパス名が
//			シンボリックリンクを含んでいてループになっている、など

//static
void
ModOsDriver::Process::chdir(const ModUnicodeString& path)
{
	ModOsDriver::Process::chdir(const_cast<ModUnicodeString&>(path).getString(ModOs::Process::getEncodingType()));
}

// static
void
ModOsDriver::Process::chdir(const char* path)
{
	static const char	func_chdir[] = "chdir";

	if (::chdir(path) == -1) {
		int	saved = errno;
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_chdir, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}
}

//	FUNCTION public
//	ModOsDriver::Process::getenv -- ある環境変数の値を求める
//
//	NOTES
//
//	ARGUMENTS
//		char*		name
//			値を求める環境変数の名前が格納された領域の先頭アドレス
//		char*		buf
//			求めた値を格納する領域の先頭アドレス
//		ModSize		size
//			求めた値を格納する領域のサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			値を求める環境変数の名前が格納された領域の先頭アドレス、
//			または求めた値を格納する領域の先頭アドレス、
//			またはそのサイズとして 0 が指定された
//		ModCommonErrorEntryNotFound
//			指定された名前の環境変数は見あたらない
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

//static
void
ModOsDriver::Process::getenv(const ModUnicodeString& name, ModUnicodeString& buf)
{
	char	tmp[ModPathMax + 1];
	ModOsDriver::Process::getenv(const_cast<ModUnicodeString&>(name).getString(ModOs::Process::getEncodingType()), tmp, sizeof(tmp));
	buf = tmp;
}

// static
void
ModOsDriver::Process::getenv(const char* name, char* buf, ModSize size)
{
	if (name == 0 || buf == 0 || size == 0) {

		// 値を求める環境変数の名前を格納する領域の先頭アドレスや
		// 求めた値を格納する領域の先頭アドレスや
		// そのサイズとして 0 が指定されている
error:
		ModThrowOsError(ModCommonErrorBadArgument);
	}

	// プロセスの環境から
	// 指定された名前の環境変数の値を含む文字列を得て、
	// この文字列は "名前=値" の形式なので、値の部分だけ取り出す

	static const char	func_getenv[] = "getenv";

	char*	value = ::getenv(name);
	if (value == 0 || (value = ModCharTrait::find(value, '=')) == 0)

		// 指定された名前の環境変数は見あたらないか、
		// そんなはずはないはずだが、
		// 文字列中に '=' が含まれていない

		ModThrowOsError(ModCommonErrorEntryNotFound);

	// 指定された領域に得られた値をコピーする

	ModSize	len = ModCharTrait::length(++value);
	if (len + 1 > size)

		// 得られた値を格納するには
		// 指定された領域では足りない

		goto error;

	ModOsDriver::Memory::copy(buf, value, len + 1);
}

//	FUNCTION public
//	ModOsDriver::Process::setenv -- ある環境変数の値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		char*		name
//			値を求める環境変数の名前が格納された領域の先頭アドレス
//		char*		value
//			設定する値を格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			値を設定する環境変数の名前が格納された領域の先頭アドレス、
//			設定する値の格納された領域の先頭アドレスとして 0 が指定された
//		ModCommonErrorOutOfRange
//			確保しようとしているメモリーのサイズが、
//			システムの物理的な制限を越えているため、メモリーを確保できない
//		ModOsErrorSystemMemoryExhaust
//			現時点では充分なメモリーが存在しないため、メモリーを確保できない
//			再実行すれば、確保できるかもしれない
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた

//static
void
ModOsDriver::Process::setenv(const ModUnicodeString& name, const ModUnicodeString& value)
{
	ModOsDriver::Process::setenv(
		const_cast<ModUnicodeString&>(name).getString(ModOs::Process::getEncodingType()),
		const_cast<ModUnicodeString&>(value).getString(ModOs::Process::getEncodingType()));
}

// static
void
ModOsDriver::Process::setenv(const char* name, const char* value)
{
	if (name == 0 || value == 0)

		// 値を設定する環境変数の名前を格納する領域の
		// 先頭アドレスとして 0 が指定されている

		ModThrowOsError(ModCommonErrorBadArgument);

	// 指定された環境変数の名前と値を使って
	// "名前=値" の形式の文字列を生成する

	ModSize	nLen = ModCharTrait::length(name);
	ModSize vLen = ModCharTrait::length(value);

	char*	buf = (char*) ModOsDriver::Memory::alloc(nLen + 1 + vLen + 1);
	; ModAssert(buf != 0);

	ModOsDriver::Memory::copy(buf, name, nLen);
	buf[nLen] = '=';
	ModOsDriver::Memory::copy(buf + nLen + 1, value, vLen + 1);

	// 生成した文字列をプロセスの環境へ登録する
	//
	//【注意】	登録に成功すると、
	//			登録された文字列の格納されている領域は
	//			システムにより管理されるため、解放してはいけない

	static const char	func_putenv[] = "putenv";

	if (::putenv(buf)) {
		int	saved = errno;
		ModOsDriver::Memory::free(buf);
		ModThrowOs(ModOsDriver::File::getErrorNumber(
			saved, func_putenv, srcFile, __LINE__),
				   _Error::getErrorLevel(saved),
				   saved);
	}
}

//
// FUNCTION public
// ModOsDriver::newErrorHandler -- ::newのエラーハンドル関数
//
// NOTES
//	new()のエラーでModException例外を送出するためのハンドラ関数。
//	NTと仕様が異なるので、仮想OSレベルに用意する。
//	ModOsErrorSystemMemoryExhaustのエラーを送出する。
//	グローバルなnewでメモリを確保できないときに呼び出すため、
//	ModErrorHandle::newSettingHandlerで登録する。
//	グローバルなnewが失敗した場合、デフォルトでは0が返されるが、
//	他のハンドラが設定されているとその動作は保証されない。また、コンストラクタ
//	など、返り値がチェックできない場合もある。
//	そこで、グローバルなnewを使うクラスでは、本エラーハンドラを
//	operator new()で設定してからグローバルなnewを
//	呼び出し、メモリ獲得失敗時には本関数が呼び出されるようにする。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModOsErrorSystemMemoryExhaust
//		システムのメモリが確保できない(::newに失敗)
//
void
ModOsDriver::newErrorHandler()
{
	ModThrowOsFatal(ModOsErrorSystemMemoryExhaust);

	// 例外を送出したときにはハンドラが戻されなくなるとまずい。
	// 上位で対処する
}

//
// TYPEDEF
//	ModNewErrorHandlerFunction -- ::newのエラーハンドラ関数を表す型
// NOTES
//	::newが失敗した場合に呼び出されるエラーハンドラ関数は、
//	::set_new_handlerで設定できる。この関数の型をわかりやすくするために
//	定義したもの。
//
typedef void (*ModNewErrorHandlerFunction)();

//
// FUNCTION public
// ModOsDriver::newSetHandler -- エラーハンドラを設定してから::newを呼び出す
//
// NOTES
//	newのMOD用のエラーハンドラModOsDriver::newErrorHandlerを一時的に設定し、
//	グローバルなnewを呼び出した後、元のハンドラを戻す。
//	ロックして作業を行ない、newの実行後すぐに元に戻すので他のライブラリで
//	もし別のハンドラを設定していても他のクラスへの悪影響はないはずである。
//	この方法は、「Effective C++ (C++ の50の急所)」ソフトバンク社、
//	スコット・マイヤーズ著、p.28,項目8を参考にした。
//	ModObjectのサブクラスにできないクラスのoperator newから呼びだし、
//	グローバルなnewでエラーがおきてもModException例外のチェックだけで済む
//	ようにする。
//	set_new_handlerとエラーハンドル関数の仕様がNTと異なるため、
//	仮想OSで用意する。
//
// ARGUMENTS
//	ModSize size
//		必要なサイズ
//
// RETURN
//	確保できたメモリの先頭アドレスを返す
//
// EXCEPTIONS
//	ModOsErrorSystemMemoryExhaust
//		システムのメモリが確保できない(::newに失敗)
//
void* 
ModOsDriver::newSetHandler(ModSize size)
{
	using namespace std;
	
	// ロック
	ModAutoMutex<ModOsMutex> autoMutex(ModCommonMutex::getMutex());
	ModNewErrorHandlerFunction current = 0;
	void* memory;
	// ハンドラ中で初期化が始まるとnewが呼び出されたりするので困る
	ModCommonInitialize::checkAndInitialize();

	autoMutex.lock();
	try {
		current = set_new_handler(ModOsDriver::newErrorHandler);
		// 指定されたサイズを確保する
		memory = (void*)new char[size];
	} catch (ModException& exception) {
		// ハンドラを元に戻す
		set_new_handler(current);
		ModRethrow(exception);
	}
	// ハンドラを元に戻す
	set_new_handler(current);
	return(memory);
}


//
// WIN32API用 (未使用)
// Linux版は ModCommonErrorNotSupported としておく
//

//static
const ModUnicodeString&
ModOsDriver::File::getFullPathNameW() const
{
	ModThrowOsError(ModCommonErrorNotSupported);
}

//
// Copyright (c) 1997, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
