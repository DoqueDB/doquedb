// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp -- ファイル、ディレクトリ関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2012, 2016, 2023 Ricoh Company, Ltd.
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
#ifdef SYD_OS_WINNT4_0
#include <accctrl.h>
#include <aclapi.h>
#endif
#include <assert.h>
#include <io.h>
#endif
#ifdef SYD_OS_POSIX
#define NORMAL_WRITEV
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>
#ifndef NORMAL_WRITEV
#include <sys/mman.h>
#endif
#endif
}

#include "Os/AutoCriticalSection.h"
#include "Os/Assert.h"
#include "Os/AsyncStatus.h"
#include "Os/FakeError.h"
#include "Os/File.h"
#include "Os/Limits.h"
#include "Os/Process.h"
#include "Os/SysConf.h"
#include "Os/Unicode.h"

#include "Common/Message.h"
#include "Common/SystemParameter.h"

#include "Exception/BadArgument.h"
#include "Exception/DiskFull.h"
#include "Exception/FileAlreadyExisted.h"
#include "Exception/FileAlreadyOpened.h"
#include "Exception/FileNotFound.h"
#include "Exception/PermissionDenied.h"
#include "Exception/TooManyOpenFiles.h"
#include "Exception/SystemCall.h"
#include "Exception/Unexpected.h"

_TRMEISTER_USING
_TRMEISTER_OS_USING

//	MACRO
//	_TRMEISTER_OS_THROW -- システムコールのエラーを表す例外を投げる
//
//	NOTES

#define	_TRMEISTER_OS_THROW(path, func, osErrno)	\
	_File::throwOsError(path, func, osErrno, srcFile, __LINE__)

namespace 
{
#ifdef SYD_OS_WINDOWS
namespace _Parameter
{
	//	CLASS
	//	_Parameter::_Int
	//
	//	NOTES
	//	Integer型のパラメータを取得する

	class _Int
	{
	public:
		// コンストラクタ
		_Int(const char* key_, int default_)
			: m_cKey(key_), m_iDefault(default_), m_bRead(false)
			{}

		// 値を取得する
		int get() {
			if (m_bRead == false) {
				AutoCriticalSection cAuto(m_cLatch);
				if (m_bRead == false) {
					if (Common::SystemParameter::getValue(m_cKey, m_iValue)
						== false)
						m_iValue = m_iDefault;
					if (m_iValue < 1) m_iValue = 1;
					m_bRead = true;
				}
			}
			return m_iValue;
		}

	private:
		// キー
		ModUnicodeString m_cKey;
		// 値
		int m_iValue;
		// デフォルト値
		int m_iDefault;
		// 排他用
		CriticalSection m_cLatch;
		// パラメータを読み込んだかどうか
		bool m_bRead;
	};

	// ファイルIO時のリトライ回数
	_Int	_FileRetry("Os_FileIORetryCount", 30);
}
#endif

namespace _File
{
#ifdef SYD_OS_WINDOWS
	// あるファイルの属性を得る
	DWORD					getAttribute(const Path& path);
	// あるファイルの属性を設定する
	void					setAttribute(const Path& path, DWORD v);
#endif
#ifdef SYD_OS_POSIX
	// あるファイルをロックする
	bool
	lock(const Os::Path& path, int descriptor, int function);
#endif
	// システムコールのエラーを表す例外を投げる
	void
	throwOsError(const Path& path, const UnicodeString& func,
#ifdef SYD_OS_WINDOWS
				 DWORD osErrno,
#endif
#ifdef SYD_OS_POSIX
				 int osErrno,
#endif
				 const char* srcFile, int line);
}

namespace _Literal
{
	// 擬似エラー関係

#ifdef SYD_FAKE_ERROR
	const char func_write_error_DiskFull[] = "Os::File::write_DiskFull";
#endif

	#define	U(literal)	const UnicodeString	literal(#literal)

	// 関数名関係

#ifdef SYD_OS_WINDOWS
	U(CloseHandle);
	U(CreateDirectory);
	U(CreateFile);
	U(DeleteFile);
	U(FlushFileBuffers);
	U(GetFileAttributes);
	U(GetFileSize);
#ifdef SYD_OS_WINNT4_0
	U(LockFileEx);
#else
	U(LockFile);
#endif
	U(MoveFile);
	U(MoveFileEx);
	U(OpenProcessToken);
	U(ReadFile);
	U(ReadFileScatter);
	U(RemoveDirectory);
	U(SetEndOfFile);
	U(SetFileAttributes);
	U(SetFilePointer);
#ifdef SYD_OS_WINNT4_0
	U(UnlockFileEx);
#else
	U(UnlockFile);
#endif
	U(WriteFile);
	U(WriteFileGather);
#ifdef SYD_OS_WINNT4_0
	U(AdjustTokenPrivileges);
	U(AllocateAndInitializeSid);
	U(GetNamedSecurityInfo);
	U(GetSecurityDescriptorDacl);
	U(GetSecurityDescriptorGroup);
	U(GetSecurityDescriptorOwner);
	U(GetTokenInformation);
	U(InitializeSecurityDescriptor);
	U(LookupPrivilegeValue);
	U(SetEntriesInAcl);
	U(SetNamedSecurityInfo);
	U(SetSecurityDescriptorDacl);
	U(SetSecurityDescriptorGroup);
	U(SetSecurityDescriptorOwner);
#endif
#ifdef SYD_OS_WINNT5_0
#else
	U(GetVersionEx);
#endif
#endif
#ifdef SYD_OS_POSIX
	U(access);
	U(chmod);
	U(close);
	U(fchmod);
	U(fstat);
	U(fsync);
	U(fdatasync);
	U(ftruncate);
	U(getrlimit);
	U(lockf);
	U(lseek);
	U(mkdir);
	U(open);
	U(pread);
	U(pwrite);
	U(read);
	U(readv);
	U(rename);
	U(rmdir);
	U(stat);
#ifdef OBSOLETE
	U(truncate);
#endif
	U(unlink);
	U(write);
#ifdef NORMAL_WRITEV
	U(writev);
#else
	U(mmap);
	U(munmap);
#endif
#endif

	#undef U
}

#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WINNT5_0
#else
namespace _Version
{
	// OS のバージョン情報を得る
	const OSVERSIONINFO&	get();

	// 以下の情報を保護するためのラッチ
	CriticalSection			_latch;

	// OS のバージョン情報を取得済みか
	bool					_initialized = false;
	// OS のバージョン情報
	OSVERSIONINFO			_info;

}
#endif

#ifdef SYD_OS_WINNT4_0
namespace _AccessControl
{
	namespace _Sid
	{
		//	STRUCT
		//	$$$::_AccessControl::_Sid::_Type --
		//		UNIX 形式の許可モードでのユーザーに対応する
		//		SID の種類を表すクラス
		//
		//	NOTES

		struct _Type
		{
			//	ENUM
			//	$$$::_AccessControl::_Sid::_Type::Value --
			//		UNIX 形式の許可モードでのユーザーに対応する
			//		SID の種類を表す値
			//
			//	NOTES

			enum Value
			{
				// 実行ユーザー
				User = 0,
				// ユーザー BUILTIN\Administrators
				Admins,
				// プライマリグループ
				Group,
				// ユーザー EveryOne
				EveryOne,
				// 値の数
				ValueNum
			};
		};
	}

	namespace _Info
	{
		//	CLASS
		//	$$$::_AccessControl::_Info -- セキュリティ情報の種類を表すクラス
		//
		//	NOTES

		struct _Type
		{
			//	ENUM
			//	$$$::_AccessControl::_Info::_Type --
			//		セキュリティ情報の種類を表す値の型
			//
			//	NOTES

#ifdef SYD_OS_WINNT5_0
#else
#ifndef PROTECTED_DACL_SECURITY_INFORMATION
#define	PROTECTED_DACL_SECURITY_INFORMATION		0x80000000L
#endif
#ifndef	UNPROTECTED_DACL_SECURITY_INFORMATION
#define	UNPROTECTED_DACL_SECURITY_INFORMATION	0x20000000L
#endif
#ifndef PROTECTED_SACL_SECURITY_INFORMATION
#define	PROTECTED_SACL_SECURITY_INFORMATION		0x40000000L
#endif
#ifndef	UNPROTECTED_SACL_SECURITY_INFORMATION
#define	UNPROTECTED_SACL_SECURITY_INFORMATION	0x10000000L
#endif
#endif
			typedef unsigned int	Value;
			enum
			{
				// なし
				None =				0x00000000,
				// 所有者
				Owner =				OWNER_SECURITY_INFORMATION,
				// グループ
				Group =				GROUP_SECURITY_INFORMATION,
				// 随意アクセス制御リスト
				Dacl =				DACL_SECURITY_INFORMATION,
				// システムアクセス制御リスト
				Sacl =				SACL_SECURITY_INFORMATION,
				// 継承される ACE から随意アクセス制御リストを守る
				ProtectedDacl =		PROTECTED_DACL_SECURITY_INFORMATION,
				// 継承される ACE から随意アクセス制御リストを守らない
				UnprotectedDacl =	UNPROTECTED_DACL_SECURITY_INFORMATION,
				// 継承される ACE からシステムアクセス制御リストを守る
				ProtectedSacl =		PROTECTED_SACL_SECURITY_INFORMATION,
				// 継承される ACE からシステムアクセス制御リストを守らない
				UnprotectedSacl =	UNPROTECTED_SACL_SECURITY_INFORMATION,
				// マスク
				Mask =				(None | Owner | Group | Dacl | Sacl |
									 ProtectedDacl | UnprotectedDacl |
									 ProtectedSacl | UnprotectedSacl)
			};
		};
	};

	// 与えられた SID を複写する
	PSID					copySid(PSID src);
	// ユーザー EveryOne の SID を求める
	PSID					getEveryOneSid();
	// ユーザー BUILTIN\Administrators の SID を求める
	PSID					getAdministratorsSid();
	// UNIX の許可モード 3 ユーザーの SID を破棄する
	void					freeUnixSids(PSID* sids);
	// 呼び出しプロセスに関する UNIX の許可モード 3 ユーザーの SID を得る
	void					getCurrentUnixSids(PSID* sids);

	// 与えられた UNIX の許可モードから対応する ACES を生成し、
	// 与えられたアクセス制御リストに設定する
	ACL*
	setUnixAcesInAcl(ACL* src, File::Permission::Value mode, PSID* sids);

	// 与えられたセキュリティ記述子を初期化する
	void
	initializeSecurityDescriptor(SECURITY_DESCRIPTOR& sd,
								 PSID owner, PSID group, ACL* dacl);

#ifdef OBSOLETE
	// あるファイルのセキュリティ情報を得る
	PSECURITY_DESCRIPTOR
	getSecurityInfo(const Path& path, _Info::_Type::Value& type,
					PSID& owner, PSID& group, ACL*& dacl);
	// あるファイルのセキュリティ情報を変更する
	void
	setSecurityInfo(const Path& path, _Info::_Type::Value type,
					PSID owner, PSID group, ACL* dacl);
#endif

	// ファイルのセキュリティ情報を変更を排他するためのラッチ
	CriticalSection			_latch;
}

#ifdef OBSOLETE
namespace _Luid
{
	// 以下の情報を保護するためのラッチ
	CriticalSection		_latch;

	//	NAMESPACE
	//	$$$::_Luid::_TakeOwnership --
	//		所有権を取得する特権の LUID 関連の名前空間
	//
	//	NOTES

	namespace _TakeOwnership
	{
		// LUID を得る
		const LUID&			get();

		// LUID を取得済か
		bool				_initialized = false;
		// 取得済の LUID
		LUID				_value;
	}

	//	NAMESPACE
	//	$$$::_Luid::_Restore --
	//		ファイルとディレクトリを復元する特権の LUID 関連の名前空間
	//
	//	NOTES

	namespace _Restore
	{
		// LUID を得る
		const LUID&			get();

		// LUID を取得済か
		bool				_initialized = false;
		// 取得済の LUID
		LUID				_value;
	}
}
#endif // OBSOLETE
#endif

//	FUNCTION
//	$$$::_File::getAttribute -- あるファイルの属性を得る
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			属性を得るファイルの絶対パス名
//
//	RETURN
//		得られた属性を表す値
//
//	EXCEPTIONS

DWORD
_File::getAttribute(const Path& path)
{
	// 指定されたファイルの属性を得る

	const DWORD v = ::GetFileAttributes(path);
	if (v == 0xffffffff) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_OS_THROW(path, _Literal::GetFileAttributes, osErrno);
	}

	return v;
}

//	FUNCTION
//	$$$::_File::setAttribute -- あるファイルの属性を変更する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			属性を変更するファイルの絶対パス名
//		DWORD				v
//			変更後のファイルの属性
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_File::setAttribute(const Path& path, DWORD v)
{
	// 指定されたファイルの属性を与えられたものに変更する

	if (!::SetFileAttributes(path, v)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_OS_THROW(path, _Literal::SetFileAttributes, osErrno);
	}
}

#ifdef SYD_OS_WINNT5_0
#else
//	FUNCTION
//	$$$::_Version::get -- OS のバージョン情報を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた OS のバージョン情報
//
//	EXCEPTIONS

const OSVERSIONINFO&
_Version::get()
{
	AutoCriticalSection	latch(_latch);

	if (!_initialized) {
		_info.dwOSVersionInfoSize = sizeof(_info);
		if (!::GetVersionEx(&_info)) {

			// システムコールのエラーを表す例外を投げる

			const DWORD osErrno = ::GetLastError();
			_TRMEISTER_THROW2(
				Exception::SystemCall, _Literal::GetVersionEx, osErrno);
		}
		_initialized = true;
	}

	return _info;
}
#endif

#ifdef SYD_OS_WINNT4_0
//	FUNCTION
//	$$$::_AccessControl::copySid -- 与えられた SID を複写する
//
//	NOTES
//		複写して求められた SID は ::FreeSid で破棄しなければならない
//
//	ARGUMENTS
//		PSID				src
//			複写する SID を格納する領域の先頭アドレス
//
//	RETURN
//		複写して得られた SID を格納する領域の先頭アドレス
//
//	EXCEPTIONS

PSID
_AccessControl::copySid(PSID src)
{
	; _TRMEISTER_ASSERT(::IsValidSid(src));

	BYTE	n = *::GetSidSubAuthorityCount(src);

	PSID	dst;
	if (!::AllocateAndInitializeSid(
			::GetSidIdentifierAuthority(src),
			n,
			(n > 0) ? *::GetSidSubAuthority(src, 0) : 0,
			(n > 1) ? *::GetSidSubAuthority(src, 1) : 0,
			(n > 2) ? *::GetSidSubAuthority(src, 2) : 0,
			(n > 3) ? *::GetSidSubAuthority(src, 3) : 0,
			(n > 4) ? *::GetSidSubAuthority(src, 4) : 0,
			(n > 5) ? *::GetSidSubAuthority(src, 5) : 0,
			(n > 6) ? *::GetSidSubAuthority(src, 6) : 0,
			(n > 7) ? *::GetSidSubAuthority(src, 7) : 0,
			&dst)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall,
					   _Literal::AllocateAndInitializeSid, osErrno);
	}
	; _TRMEISTER_ASSERT(::IsValidSid(dst));

	return dst;
}

//	FUNCTION
//	$$$::_AccessControl::getAdministratorsSid --
//		ユーザー BUILTIN\Administratos の SID を求める
//
//	NOTES
//		求めたユーザー BUILTIN\Administrators の SID は
//		::FreeSid で破棄しなければならない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求めたユーザー BUILTIN\Administrators の SID を
//		格納する領域の先頭アドレス
//
//	EXCEPTIONS

PSID
_AccessControl::getAdministratorsSid()
{
	PSID	sid;

	SID_IDENTIFIER_AUTHORITY	idAuth = SECURITY_NT_AUTHORITY;
	if (!::AllocateAndInitializeSid(
			&idAuth, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
			0, 0, 0, 0, 0, 0, &sid)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall,
					   _Literal::AllocateAndInitializeSid, osErrno);
	}
	; _TRMEISTER_ASSERT(::IsValidSid(sid));

	return sid;
}

//	FUNCTION
//	$$$::_AccessControl::getEveryOneSid -- ユーザー EveryOne の SID を求める
//
//	NOTES
//		求めたユーザー EveryOne の SID は ::FreeSid で破棄しなければならない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求めたユーザー EveryOne の SID を格納する領域の先頭アドレス
//
//	EXCEPTIONS

PSID
_AccessControl::getEveryOneSid()
{
	PSID	sid;

	SID_IDENTIFIER_AUTHORITY	idAuth = SECURITY_WORLD_SID_AUTHORITY;
	if (!::AllocateAndInitializeSid(
			&idAuth, 1,	SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &sid)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall,
					   _Literal::AllocateAndInitializeSid, osErrno);
	}
	; _TRMEISTER_ASSERT(::IsValidSid(sid));

	return sid;
}

//	FUNCTION
//	$$$::_AccessControl::freeUnixSids --
//		UNIX の許可モード 3 ユーザーの SID を破棄する
//
//	NOTES
//		UNIX の許可モード 3 ユーザーである
//
//		* 所有者
//		* グループ
//		* その他
//
//		のそれぞれに対応するユーザーの SID の配列を破棄する
//
//	ARGUMENTS
//		PSID*				sids
//			破棄する SID の配列であり、
//			要素数は $$$::_AccessControl::_Sid::_Type::ValueNum である
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
_AccessControl::freeUnixSids(PSID* sids)
{
	unsigned int i = 0;
	do {
		if (sids[i])
			::FreeSid(sids[i]), sids[i] = 0;
	} while (++i < _Sid::_Type::ValueNum) ;
}

//	FUNCTION
//	$$$::_AccessControl::getCurrentUnixSids --
//		呼び出しプロセスに関する UNIX の許可モード 3 ユーザーの SID を得る
//
//	NOTES
//		呼び出しプロセスのトークンから
//		実行ユーザーとプライマリグループの SID を得る
//		また、ユーザー BUILTIN\Administrators と EveryOne の SID を得る
//		これらはそれぞれ UNIX の許可モード 3 ユーザーに以下のように対応する
//
//		実行ユーザー -> 所有者
//		BUILTIN\Administrators -> 所有者
//		プライマリグループ -> グループ
//		EveryOne -> その他
//
//	ARGUMENTS
//		PSID*				sids
//			求めた UNIX の許可モード 3 ユーザーを格納する配列であり、
//			要素数は $$$::_AccessControl::_Sid::_Type::ValueNum である
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_AccessControl::getCurrentUnixSids(PSID* sids)
{
	// 呼び出しプロセスのアクセストークンのハンドルを得る
	//
	//【注意】	MSDN Library には OpenProcessToken の
	//			第 3 引数を参照するとは書いていないが、
	//			Purify で UMR していると指摘されるので、
	//			初期化したものを与える

	HANDLE	handle = 0;
	if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &handle)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(
			Exception::SystemCall, _Literal::OpenProcessToken, osErrno);
	}

	Memory::reset(sids, sizeof(PSID) * _Sid::_Type::ValueNum);
	void*	buf = 0;
	DWORD size;

	try {
		// アクセストークンからユーザー情報を得る

		::GetTokenInformation(handle, TokenUser, 0, 0, &size);

		buf = Memory::allocate(size);
		; _TRMEISTER_ASSERT(buf);

		if (!::GetTokenInformation(handle, TokenUser, buf, size, &size) ||
			!::IsValidSid(static_cast<TOKEN_USER*>(buf)->User.Sid)) {

			// システムコールのエラーを表す例外を投げる

			const DWORD osErrno = ::GetLastError();
			_TRMEISTER_THROW2(Exception::SystemCall,
						   _Literal::GetTokenInformation, osErrno);
		}

		// ユーザー情報中のユーザーの SID をコピーする

		sids[_Sid::_Type::User] =
			copySid(static_cast<TOKEN_USER*>(buf)->User.Sid);
		; _TRMEISTER_ASSERT(sids[_Sid::_Type::User]);

		Memory::free(buf);

		// BUILTIN\Administratos の SID を生成する

		sids[_Sid::_Type::Admins] = getAdministratorsSid();
		; _TRMEISTER_ASSERT(sids[_Sid::_Type::Admins]);

		// アクセストークンからプライマリグループ情報を得る

		::GetTokenInformation(handle, TokenPrimaryGroup, 0, 0, &size);

		buf = Memory::allocate(size);
		; _TRMEISTER_ASSERT(buf);

		if (!::GetTokenInformation(
				handle, TokenPrimaryGroup, buf, size, &size) ||
			!::IsValidSid(
				static_cast<TOKEN_PRIMARY_GROUP*>(buf)->PrimaryGroup)) {

			// システムコールのエラーを表す例外を投げる

			const DWORD osErrno = ::GetLastError();
			_TRMEISTER_THROW2(
				Exception::SystemCall, _Literal::GetTokenInformation, osErrno);
		}

		// プライマリグループ情報中の
		// プライマリグループの SID をコピーする

		sids[_Sid::_Type::Group] = copySid(
			static_cast<TOKEN_PRIMARY_GROUP*>(buf)->PrimaryGroup);
		; _TRMEISTER_ASSERT(sids[_Sid::_Type::Group]);

		Memory::free(buf);

		// 全ユーザーを表す EveryOne の SID を生成する

		sids[_Sid::_Type::EveryOne] = getEveryOneSid();
		; _TRMEISTER_ASSERT(sids[_Sid::_Type::EveryOne]);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		Memory::free(buf);
		freeUnixSids(sids);
		(void) ::CloseHandle(handle);
		_TRMEISTER_RETHROW;
	}

	// 最後に呼び出しプロセスのアクセストークンを破棄する

	(void) ::CloseHandle(handle);
}

//	FUNCTION private
//	_AccessControl::setUnixAcesInAcl --
//		与えられた UNIX の許可モードから対応する ACES を生成し、
//		与えられたアクセス制御リストに設定する
//
//	NOTES
//		生成されたアクセス制御リストは ::LocalFree で破棄しなければならない
//
//	ARGUMENTS
//		ACL*				src
//			0 以外の値
//				指定された UNIX の許可モードから生成された ACES を設定する
//				アクセス制御リストを格納する領域の先頭アドレス
//			0
//				指定された UNIX の許可モードから生成された
//				ACES のみからなるアクセス制御リストを生成する
//		Os::File::Permission::Value	mode
//			ACES を生成するための UNIX の許可モードを表す値
//			Os::File::Permission::Value の論理和を指定する
//		PSID*				sids
//			指定された UNIX の許可モードにおける
//			所有者、グループ、その他のユーザーに対応する
//			SID を格納する配列であり、要素数は
//			$$$::_AccessControl::_Sid::_Type::ValueNum である
//
//	RETURN
//		生成したアクセス制御リストを格納する領域の先頭アドレス
//
//	EXCEPTIONS

ACL*
_AccessControl::setUnixAcesInAcl(
	ACL* src, File::Permission::Value mode, PSID* sids)
{
	// ファイルのアクセスマスク
	//
	//【注意】	ディレクトリーのものを共用している

	struct AccessMask
	{
		enum Value
		{
			None =		READ_CONTROL | SYNCHRONIZE |
						FILE_READ_ATTRIBUTES | FILE_READ_EA,
			Owner =		DELETE | WRITE_DAC | WRITE_OWNER |
						FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA,
			Read =		FILE_LIST_DIRECTORY,
//						FILE_READ_DATA,
			Write =		FILE_ADD_FILE | FILE_ADD_SUBDIRECTORY |
						FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA |
						FILE_DELETE_CHILD,
//						FILE_WRITE_DATA | FILE_APPEND_DATA |
//						FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA,
			Execute =	FILE_TRAVERSE
//						FILE_EXECUTE
		};
	};

	// まず、所有者、グループ、その他のユーザーの
	// ACE を生成するための初期化を行う

	EXPLICIT_ACCESS		ea[_Sid::_Type::ValueNum];
	Memory::reset(ea, sizeof(ea));

	unsigned int i = 0;
	do {
		ea[i].grfAccessPermissions = AccessMask::None; 
		ea[i].grfAccessMode = (sids[i] ? SET_ACCESS : NOT_USED_ACCESS);
		ea[i].grfInheritance= NO_INHERITANCE;
		ea[i].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[i].Trustee.TrusteeType = TRUSTEE_IS_UNKNOWN;
		ea[i].Trustee.ptstrName = (LPTSTR) sids[i];
	} while (++i < _Sid::_Type::ValueNum) ;

	// 指定された許可モードから
	// 所有者、グループ、その他のユーザーのアクセスマスクを求める

	mode &= File::Permission::Mask;

	ea[_Sid::_Type::User].grfAccessPermissions |= AccessMask::Owner;

	if (mode & File::Permission::OwnerRead) {
		ea[_Sid::_Type::User].grfAccessPermissions |= AccessMask::Read;
		ea[_Sid::_Type::Admins].grfAccessPermissions |= AccessMask::Read;
	}
	if (mode & File::Permission::OwnerWrite) {
		ea[_Sid::_Type::User].grfAccessPermissions |= AccessMask::Write;
		ea[_Sid::_Type::Admins].grfAccessPermissions |= AccessMask::Write;
	}
	if (mode & File::Permission::OwnerExecute) {
		ea[_Sid::_Type::User].grfAccessPermissions |= AccessMask::Execute;
		ea[_Sid::_Type::Admins].grfAccessPermissions |= AccessMask::Execute;
	}

	if (mode & File::Permission::GroupRead)
		ea[_Sid::_Type::Group].grfAccessPermissions |= AccessMask::Read;
	if (mode & File::Permission::GroupWrite)
		ea[_Sid::_Type::Group].grfAccessPermissions |= AccessMask::Write;
	if (mode & File::Permission::GroupExecute)
		ea[_Sid::_Type::Group].grfAccessPermissions |= AccessMask::Execute;

	if (mode & File::Permission::OtherRead)
		ea[_Sid::_Type::EveryOne].grfAccessPermissions |= AccessMask::Read;
	if (mode & File::Permission::OtherWrite)
		ea[_Sid::_Type::EveryOne].grfAccessPermissions |= AccessMask::Write;
	if (mode & File::Permission::OtherExecute)
		ea[_Sid::_Type::EveryOne].grfAccessPermissions |= AccessMask::Execute;

	// 与えられたアクセス制御リストに所有者、グループ、その他のユーザーの
	// ACE を設定した新しいアクセス制御リストを生成する
	//
	//【注意】	::SetEntriesInAcl が与えた EXPLICIT_ACCESS の先頭から
	//			ACE を設定していくことを仮定している

	ACL*	dst;
	if (::SetEntriesInAcl(sizeof(ea) / sizeof(EXPLICIT_ACCESS),
						  ea, src, &dst) != ERROR_SUCCESS) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(
			Exception::SystemCall, _Literal::SetEntriesInAcl, osErrno);
	}

	return dst;
}

//	FUNCTION private
//	$$$::_AccessControl::initializeSecurityDescriptor --
//		与えられたセキュリティ記述子を初期化する
//
//	NOTES
//
//	ARGUMENTS
//		SECURITY_DESCRIPTOR& sd
//			初期化するセキュリティ記述子
//		PSID				owner
//			0 以外の値
//				与えられたセキュリティ記述子に設定する
//				所有者の SID を格納する領域の先頭アドレス
//			0
//				なにもしない
//		PSID				group
//			0 以外の値
//				与えられたセキュリティ記述子に設定する
//				プライマリグループの SID を格納する領域の先頭アドレス
//			0
//				なにもしない
//		ACL*				dacl
//			0 以外の値
//				与えられたセキュリティ記述子に設定する
//				随意アクセス制御リストを格納する領域の先頭アドレス
//			0
//				なにもしない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_AccessControl::initializeSecurityDescriptor(
	SECURITY_DESCRIPTOR& sd, PSID owner, PSID group, ACL* dacl)
{
	// 与えられたセキュリティディスクリプターを初期化する

	if (!::InitializeSecurityDescriptor(
			&sd, SECURITY_DESCRIPTOR_REVISION) ||
		!::IsValidSecurityDescriptor(&sd)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall,
					   _Literal::InitializeSecurityDescriptor, osErrno);
	}

	// 与えられた所有者、プライマリグループの SID を
	// セキュリティ記述子に設定する

	if (owner && !::SetSecurityDescriptorOwner(&sd, owner, 0)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall,
					   _Literal::SetSecurityDescriptorOwner, osErrno);
	}

	if (group && !::SetSecurityDescriptorGroup(&sd, group, 0)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall,
					   _Literal::SetSecurityDescriptorGroup, osErrno);
	}

	// 与えられた随意アクセス制御リストをセキュリティ記述子に設定する

	if (dacl && !::SetSecurityDescriptorDacl(&sd, true, dacl, false)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall,
					   _Literal::SetSecurityDescriptorDacl, osErrno);
	}
}

#ifdef OBSOLETE

//	FUNCTION
//	$$$::_AccessControl::getSecurityInfo --
//		あるファイルのセキュリティ情報を得る
//
//	NOTES
//		得られたセキュリティ記述子は ::LocalFree で破棄しなければならない
//		また、得られたセキュリティ情報は
//		このセキュリティ記述子を破棄すると無効になる
//
//	ARGUMENTS
//		Os::Path&			path
//			セキュリティ情報を求めるファイルの絶対パス名
//		$$$::_AccessControl::_Info::_Type::Value&	type
//			得られたセキュリティ情報の種類を表す値で、
//			$$$::_AccessControl::_Info::_Type::Value の論理和が設定される
//		PSID&				owner
//			type & $$$::_AccessControl::_Info::_Type::Owner が真のとき
//				得られたセキュリティ記述子中の所有者の
//				SID を格納する領域の先頭アドレスが設定される
//			偽のとき
//				なにもしない
//		PSID&				group
//			type & $$$::_AccessControl::_Info::_Type::Group が真のとき
//				得られたセキュリティ記述子中のプライマリグループの
//				SID を格納する領域の先頭アドレスが設定される
//			偽のとき
//				なにもしない
//		ACL*&				dacl
//			type & $$$::_AccessControl::_Info::_Type::Dacl が真のとき
//				得られたセキュリティ記述子中の
//				随意アクセス制御リストを格納する領域の先頭アドレスが設定される
//			偽のとき
//				なにもしない
//
//	RETURN
//		得られたセキュリティ記述子を格納する領域の先頭アドレス
//
//	EXCEPTIONS

PSECURITY_DESCRIPTOR
_AccessControl::getSecurityInfo(const Path& path, _Info::_Type::Value& type,
								PSID& owner, PSID& group, ACL*& dacl)
{
	// 指定されたファイルのセキュリティ記述子を得る

	PSECURITY_DESCRIPTOR sd = 0;

	const DWORD osErrno =
		::GetNamedSecurityInfo(
			const_cast<TCHAR*>(static_cast<const TCHAR*>(path)),
			SE_FILE_OBJECT,
			_Info::_Type::Owner | _Info::_Type::Group | _Info::_Type::Dacl,
			0, 0, 0, 0, &sd);
	if (osErrno != ERROR_SUCCESS ||	!::IsValidSecurityDescriptor(sd))

		// システムコールのエラーを表す例外を投げる

		_TRMEISTER_OS_THROW(path, _Literal::GetNamedSecurityInfo, osErrno);

	type = _Info::_Type::None;

	BOOL defaulted;
	BOOL present;

	// 得られたセキュリティ記述子から所有者に関する情報を得る

	if (!::GetSecurityDescriptorOwner(sd, &owner, &defaulted)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall,
					   _Literal::GetSecurityDescriptorOwner, osErrno);
	}
	if (owner && !defaulted)

		// 所有者に関する情報が得られた
		//
		//【注意】	デフォルトの情報が得られても無視する

		type |= _Info::_Type::Owner;

	// 得られたセキュリティ記述子からプライマリグループに関する情報を得る

	if (!::GetSecurityDescriptorGroup(sd, &group, &defaulted)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall,
					   _Literal::GetSecurityDescriptorGroup, osErrno);
	}
	if (group && !defaulted)

		// プライマリグループに関する情報が得られた
		//
		//【注意】	デフォルトの情報が得られても無視する

		type |= _Info::_Type::Group;

	// 得られたセキュリティ記述子から随意アクセス制御リストに関する情報を得る

	if (!::GetSecurityDescriptorDacl(sd, &present, &dacl, &defaulted)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall,
					   _Literal::GetSecurityDescriptorDacl, osErrno);
	}
	if (present && !defaulted)

		// 随意アクセス制御リストに関する情報が得られた
		//
		//【注意】	デフォルトの情報が得られても無視する

		type |= _Info::_Type::Dacl;

	// 得られたセキュリティ記述子を返す

	return sd;
}
#endif // OBSOLETE

#ifdef OBSOLETE
//	FUNCTION
//	$$$::_AccessControl::setSecurityInfo --
//		あるファイルのセキュリティ情報を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			セキュリティ情報を設定するファイルの絶対パス名
//		$$$::_AccessControl::_Info::_Type::Value	type
//			設定するセキュリティ情報の種類を表す値で、
//			$$$::_AccessControl::_Info::_Type::Value の論理和が指定される
//		PSID				owner
//			type & $$$::_AccessControl::_Info::_Type::Owner が真のとき
//				設定する所有者の SID を格納する領域の先頭アドレス
//			偽のとき
//				なにもしない
//		PSID				group
//			type & $$$::_AccessControl::_Info::_Type::Group が真のとき
//				設定するプライマリグループの SID を格納する領域の先頭アドレス
//			偽のとき
//				なにもしない
//		ACL*&				dacl
//			type & $$$::_AccessControl::_Info::_Type::Dacl が真のとき
//				設定する随意アクセス制御リストを格納する領域の先頭アドレス
//			偽のとき
//				なにもしない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_AccessControl::setSecurityInfo(const Path& path, _Info::_Type::Value type,
								PSID owner, PSID group, ACL* dacl)
{
#ifdef SYD_OS_WINNT5_0
#else
	if (type & _Info::_Type::Dacl && _Version::get().dwMajorVersion <= 4) {

		// 親から継承される ACE によりアクセス制御リストを守るか設定する
		//
		//【注意】	ただし、Windows 2000/XP 以降でないと
		//			PROTECTED_DACL_SECURITY_INFORMATION や
		//			UNPROTECTED_DACL_SECURITY_INFORMATION を指定できない
		//
		//			NT と 2000/XP でオブジェクトを共通にする必要があるとき、
		//			仕方ないので OS のバージョン番号を求めて
		//			動的に設定するようにする

		if (type & _Info::_Type::ProtectedDacl)
			type ^= _Info::_Type::ProtectedDacl;
		if (type & _Info::_Type::UnprotectedDacl)
			type ^= _Info::_Type::UnprotectedDacl;
	}
#endif
	DWORD osErrno;

	if (type & _Info::_Type::Owner) {

		// ファイルの所有者を変更しようとしている

		AutoCriticalSection	latch(_latch);

		// 呼び出しプロセスのアクセストークンのハンドルを得る
		//
		//【注意】	MSDN Library には OpenProcessToken の
		//			第 3 引数を参照するとは書いていないが、
		//			Purify で UMR していると指摘されるので、
		//			初期化したものを与える

		HANDLE	handle = 0;
		if (!::OpenProcessToken(::GetCurrentProcess(),
								TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
								&handle)) {

			// システムコールのエラーを表す例外を投げる

			const DWORD osErrno = ::GetLastError();
			_TRMEISTER_THROW2(
				Exception::SystemCall, _Literal::OpenProcessToken, osErrno);
		}

		TOKEN_PRIVILEGES* tp = 0;
		TOKEN_PRIVILEGES* prev = 0;
		void* p;

		try {
			// 所有権を取得する特権と
			// ファイルやディレクトリを復元する特権を有効にする

			DWORD size =
				sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES);

			tp = static_cast<TOKEN_PRIVILEGES*>(Memory::allocate(size));
			; _TRMEISTER_ASSERT(tp);

			tp->PrivilegeCount = 2;
			tp->Privileges[0].Luid = _Luid::_TakeOwnership::get();
			tp->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			tp->Privileges[1].Luid = _Luid::_Restore::get();
			tp->Privileges[1].Attributes = SE_PRIVILEGE_ENABLED;

			prev = static_cast<TOKEN_PRIVILEGES*>(Memory::allocate(size));
			; _TRMEISTER_ASSERT(prev);

			if (!::AdjustTokenPrivileges(
					handle, false, tp, size, prev, &size)) {

				// システムコールのエラーを表す例外を投げる

				const DWORD osErrno = ::GetLastError();
				_TRMEISTER_THROW2(Exception::SystemCall,
							   _Literal::AdjustTokenPrivileges, osErrno);
			}

			// 所有権を取得する特権を実際に変更したか調べる

			const bool adjusted = (::GetLastError() == ERROR_SUCCESS);

			// 指定されたファイルのセキュリティ記述子中に
			// 指定された種類のセキュリティ情報を設定する

			osErrno = ::SetNamedSecurityInfo(
				const_cast<TCHAR*>(static_cast<const TCHAR*>(path)),
				SE_FILE_OBJECT, type, owner, group, dacl, 0);

			if (adjusted)

				// 所有者を取得する特権を実際に変更しているので、元に戻す
				//
				//【注意】	エラーが起きても無視する

				(void) ::AdjustTokenPrivileges(
					handle, false, prev, size, 0, 0);

		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{

			Memory::free(p = tp);
			Memory::free(p = prev);
			(void) ::CloseHandle(handle);

			_TRMEISTER_RETHROW;
		}

		// 必要なくなった領域を開放する

		Memory::free(p = tp);
		Memory::free(p = prev);

		// 最後に呼び出しプロセスのアクセストークンを破棄する
		//
		//【注意】	エラーは無視する

		(void) ::CloseHandle(handle);
	} else

		// 指定されたファイルのセキュリティ記述子中に
		// 指定された所有者以外のセキュリティ情報を設定する

		osErrno = ::SetNamedSecurityInfo(
			const_cast<TCHAR*>(static_cast<const TCHAR*>(path)),
			SE_FILE_OBJECT, type, 0, group, dacl, 0);

	if (osErrno != ERROR_SUCCESS)

		// システムコールのエラーを表す例外を投げる

		_TRMEISTER_OS_THROW(path, _Literal::SetNamedSecurityInfo, osErrno);
}
#endif // OBSOLETE

#ifdef OBSOLETE

//	FUNCTION
//	$$$::_Luid::_TakeOwnership::get --
//		所有権を取得する特権を表す LUID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた LUID へのリファレンス
//
//	EXCEPTIONS

const LUID&
_Luid::_TakeOwnership::get()
{
	AutoCriticalSection	latch(_latch);

	if (!_initialized) {
		if (!::LookupPrivilegeValue(0, SE_TAKE_OWNERSHIP_NAME, &_value)) {

			// システムコールのエラーを表す例外を投げる

			const DWORD	osErrno = ::GetLastError();
			_TRMEISTER_THROW2(Exception::SystemCall,
						   _Literal::LookupPrivilegeValue, osErrno);
		}
		_initialized = true;
	}

	return _value;
}

#endif // OBSOLETE

#ifdef OBSOLETE

//	FUNCTION
//	$$$::_Luid::_Restore::get --
//		ファイルやディレクトリを復元する特権を表す LUID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた LUID へのリファレンス
//
//	EXCEPTIONS

const LUID&
_Luid::_Restore::get()
{
	AutoCriticalSection	latch(_latch);

	if (!_initialized) {
		if (!::LookupPrivilegeValue(0, SE_RESTORE_NAME, &_value)) {

			// システムコールのエラーを表す例外を投げる

			const DWORD	osErrno = ::GetLastError();
			_TRMEISTER_THROW2(Exception::SystemCall,
						   _Literal::LookupPrivilegeValue, osErrno);
		}
		_initialized = true;
	}

	return _value;
}
#endif // OBSOLETE
#endif
#endif
#ifdef SYD_OS_POSIX
//	FUNCTION
//	$$$::_File::lock --
//		あるファイルをロックする
//
//	NOTES
//		現在のファイルポインタの位置以降の領域をロックする
//
//	ARGUMENTS
//		Os::Path&			path
//			ロックするファイルの絶対パス名
//		int					descriptor
//			ロックするファイルのファイルディスクリプタ
//		int					function
//			どのようにロックするかを表す以下の値のいずれか
//
//			F_ULOCK				領域のロックをはずす
//			F_LOCK				領域をロックする
//			F_TLOCK				領域のロックを試みる
//
//	RETURN
//		true
//			操作が成功した
//		false
//			操作に失敗した
//
//	EXCEPTIONS

bool
_File::lock(const Os::Path& path, int descriptor, int function)
{
	// サイズとして 0 を指定することで、
	// 現在のファイルポインタの位置から
	// 将来におけるファイルの末尾までロックできる

	if (::lockf(descriptor, function, 0) == -1) {
		const int osErrno = errno;
		switch (osErrno) {
		case EAGAIN:
#ifdef SYD_OS_SOLARIS
		case EACCES:
#endif
			// 別のプロセスによってロックされている

			return false;
		}

		// システムコールのエラーを表す例外を投げる

		_TRMEISTER_OS_THROW(path, _Literal::lockf, osErrno);
	}

	// ロックできた

	return true;
}
#endif

//	FUNCTION
//	$$$::_File::throwOsError -- システムコールのエラーを表す例外を投げる
//
//	NOTES
//		与えられたエラー番号が NO_ERROR のとき、
//		例外を投げないことに注意する必要がある
//
//	ARGUMENTS
//		Os::Path&			path
//			エラーになったシステムコールの操作対象であるファイルの絶対パス名
//		Os::UnicodeString&	func
//			エラーになったシステムコールの名前
#ifdef SYD_OS_WINDOWS
//		DWORD				osErrno
#endif
#ifdef SYD_OS_POSIX
//		int					osErrno
#endif
//			エラー番号
//		char*				srcFile
//			エラーになったシステムコールを呼び出したソースファイルの名前
//		int					line
//			エラーになったシステムコールを呼び出したソースファイルの行
//
//	RETURN
//		なし
//
//	EXCETIONS

void
_File::throwOsError(const Path& path, const UnicodeString& func,
#ifdef SYD_OS_WINDOWS
					DWORD osErrno,
#endif
#ifdef SYD_OS_POSIX
					int osErrno,
#endif
					const char* srcFile, int line)
{
	switch (osErrno) {
#ifdef SYD_OS_WINDOWS
//  0 :		The operation completed successfully.
	case NO_ERROR:

		// エラーが起きていない

		break;
#endif

#ifdef SYD_OS_WINDOWS
//	2 :		The system cannot find the file specified.
	case ERROR_FILE_NOT_FOUND:
//	3 :		The system cannot find the path specified.
	case ERROR_PATH_NOT_FOUND:
//	53 :	The network path was not found.
	case ERROR_BAD_NETPATH:
//	67 :	The network name cannot be found.
	case ERROR_BAD_NET_NAME:
//	123 :	The file name, directory name, or volume label syntax is incorrect.
	case ERROR_INVALID_NAME:
#endif
#ifdef SYD_OS_POSIX
//	No such file or directory
	case ENOENT:
//	Not a directory
	case ENOTDIR:
#endif
		// 与えられた名前のファイルは存在しない

		throw Exception::FileNotFound(moduleName, srcFile, line, path);

#ifdef SYD_OS_WINDOWS
//	5 :     Access is denied.
	case ERROR_ACCESS_DENIED:
#endif
#ifdef SYD_OS_POSIX
//	Permission denied
	case EACCES:
#endif
		// 与えられた名前のファイルにアクセスする権限がない

		throw Exception::PermissionDenied(moduleName, srcFile, line, path);

#ifdef SYD_OS_WINDOWS
//	80 :	The file exists.
	case ERROR_FILE_EXISTS:
#endif
#ifdef SYD_OS_POSIX
//	File exists
	case EEXIST:
#endif
		// 与えられた名前のファイルは既に存在する

		throw Exception::FileAlreadyExisted(moduleName, srcFile, line, path);

#ifdef SYD_OS_WINDOWS
//	112 :	There is not enough space on the disk.
	case ERROR_DISK_FULL:
#endif
#ifdef SYD_OS_POSIX
//	No space left on device
	case ENOSPC:
#endif
		// ディスクに空き領域がなくなった

		throw Exception::DiskFull(moduleName, srcFile, line, path);

#ifdef SYD_OS_WINDOWS
// 4 :		Ths system cannnot open the file.
	case ERROR_TOO_MANY_OPEN_FILES:
#endif
#ifdef SYD_OS_POSIX
//	Too many open files
	case EMFILE:
	case ENFILE:
#endif
		// オープンしているファイルが多すぎる

		throw Exception::TooManyOpenFiles(moduleName, srcFile, line, path);

	default:

		// システムコールでエラーが起きた
		
		// パスが例外に設定されないので、ここで出力する

		SydErrorMessage << path << ModEndl;

		throw Exception::SystemCall(moduleName, srcFile, line, func, osErrno);
	}
}

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

void
File::open(OpenMode::Value mode, Permission::Value permission)
{
	if (!getPath().getLength()) {

		// ファイルの絶対パス名を記憶していない

		; _TRMEISTER_ASSERT(false);
		_TRMEISTER_THROW0(Exception::Unexpected);
	}
	if (isOpened())

		// ファイルはすでにオープンされている

		_TRMEISTER_THROW1(Exception::FileAlreadyOpened, getPath());

#ifdef SYD_OS_WINDOWS

	// 現在の UMASK を求めて、
	// 指定されたファイルの許可モードに適用する

	unsigned int	mask = Process::umask(0);
	(void) Process::umask(mask);

	permission &= Permission::Mask;
	permission &= ~mask;

	// 指定されたオープンモードから
	// どのようなアクセスをするかについての指定を取り出す

	DWORD	access;
	switch (mode & OpenMode::MaskAccess) {
	case OpenMode::Read:
		access = GENERIC_READ;					break;
	case OpenMode::Write:
		access = GENERIC_WRITE;					break;
	default:
		access = GENERIC_READ | GENERIC_WRITE;	break;
	}

	// 指定されたオープンモードから生成に関する指定を取り出す

	DWORD	create;
	switch (mode & OpenMode::MaskCreate) {
	case OpenMode::Create:
		create = OPEN_ALWAYS;		break;
	case OpenMode::Create | OpenMode::Truncate:

		// WINDOWS で存在するファイルを上書きすると、
		// セキュリティ情報は更新されないが、
		// ファイル属性は指定したものに更新されてしまう
		//
		// SOLARIS の ::creat では存在するファイルを上書きしても
		// ファイル許可モードは指定されたものに更新されない
		//
		// そこで、ファイルの生成を試みて、存在していれば、
		// そのファイルのトランケートを試みることにより、
		// 存在しているファイルのファイル属性を更新しないようにする

	case OpenMode::Create | OpenMode::Exclusive:
	case OpenMode::Create | OpenMode::Truncate | OpenMode::Exclusive:
		create = CREATE_NEW;		break;
	case OpenMode::Truncate:
	case OpenMode::Truncate | OpenMode::Exclusive:
		create = TRUNCATE_EXISTING;	break;
	default:
		create = OPEN_EXISTING;		break;
	}

	// オープンまたは生成するファイルの属性を求める
	//
	//【注意】	可能であれば、ファイル操作は非同期で行うようにする
	//
	//【注意】	ファイル属性のファイルの許可モードには、
	//			アプリケーションが書き込み可能かそうでないかしかない
	//
	//【注意】	FILE_ATTRIBUTE_READONLY は、
	//			書き込み不可なファイルであることを表し、
	//			実行不可であることを表していない

	DWORD attribute =
#ifdef SYD_OS_WINNT4_0
		FILE_FLAG_OVERLAPPED |
#endif
		FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
	if ((mode & OpenMode::Create) && !(permission & Permission::MaskWrite))
		attribute |= FILE_ATTRIBUTE_READONLY;

	// 指定されたオープンモードから
	// バッファリングに関するファイル属性を取り出す

	switch (mode & OpenMode::MaskBuffering) {
	case OpenMode::WriteThrough:
		attribute |= FILE_FLAG_WRITE_THROUGH;	break;
	default:
		attribute |= FILE_FLAG_NO_BUFFERING;	break;
	}

	// 生成したファイルハンドルは、子プロセスへ継承するようにする

	SECURITY_ATTRIBUTES	security;
	security.nLength = sizeof(security);
	security.bInheritHandle = true;
#ifdef SYD_OS_WINNT4_0
	PSID	sids[_AccessControl::_Sid::_Type::ValueNum];
	Memory::reset(sids, sizeof(sids));
	ACL*	dacl = 0;

	try {
		// 呼び出しプロセスの所有者、プライマリーグループと、
		// 全ユーザーの SID を得る

		_AccessControl::getCurrentUnixSids(sids);

		// 与えられたファイルの許可モードを
		// 求めた SID に関するアクセス制御リストへ変換する

		dacl = _AccessControl::setUnixAcesInAcl(0, permission, sids);
		; _TRMEISTER_ASSERT(dacl);

		// 生成するファイルに設定する
		// 絶対形式のセキュリティー記述子を生成する

		SECURITY_DESCRIPTOR		sd;
		_AccessControl::initializeSecurityDescriptor(
			sd, sids[_AccessControl::_Sid::_Type::User],
			sids[_AccessControl::_Sid::_Type::Group], dacl);

		security.lpSecurityDescriptor = &sd;
#else
		security.lpSecurityDescriptor = 0;
#endif
		// ファイルをオープンする

		int i = _Parameter::_FileRetry.get();
retry:
		_handle = ::CreateFile(
			getPath(),
			access,	FILE_SHARE_READ | FILE_SHARE_WRITE,
			&security, create, attribute, 0);

		if (_handle == INVALID_HANDLE_VALUE) {
			const DWORD osErrno = ::GetLastError();
			switch (osErrno) {
			case ERROR_FILE_NOT_FOUND:
				if ((mode & OpenMode::MaskCreate) ==
					(OpenMode::Create | OpenMode::Truncate) &&
					create == TRUNCATE_EXISTING) {

					// 先ほど調べたときに存在したファイルが
					// オープンしたときには削除されていたので、
					// 今度は、ファイルの生成を試みる

					create = CREATE_NEW;
					goto retry;
				}
				break;

			case ERROR_FILE_EXISTS:
			case ERROR_ALREADY_EXISTS:
				if ((mode & OpenMode::MaskCreate) ==
					(OpenMode::Create | OpenMode::Truncate) &&
					create == CREATE_NEW) {

					// 先ほど調べたときには存在しなかったファイルが
					// 生成したときには存在していたようなので、
					// 今度は、ファイルのトランケートを試みる

					create = TRUNCATE_EXISTING;
					goto retry;
				}
				break;
				
			case ERROR_SHARING_VIOLATION:

				// 別のアプリがファイルを開いているので、
				// ちょっと待って、再実行する

				if (--i) {

					// バックアップのアプリなどがファイルを
					// オープンしていることもあるので、再実行する

					SydMessage << "Sharing Violation (CreateFile) "
							   << getPath() << ModEndl;
					
					::Sleep(1000);
					goto retry;
				}
				break;
				
			}

			// システムコールのエラーを表す例外を投げる

			_TRMEISTER_OS_THROW(getPath(), _Literal::CreateFile, osErrno);
		}
#ifdef SYD_OS_WINNT4_0
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		::LocalFree(dacl);
		_AccessControl::freeUnixSids(sids);
		// マウント時に存在チェックをせずに呼ばれるのでそのまま再送する
		throw;
	}

	// 必要のなくなったアクセス制御リストと SID を破棄する

	::LocalFree(dacl);
	_AccessControl::freeUnixSids(sids);
#endif
#endif
#ifdef SYD_OS_POSIX

	// 指定されたオープンモードから
	// どのようなアクセスをするかについての指定を取り出す

	unsigned int	flag = 0;

	switch (mode & OpenMode::MaskAccess) {
	case OpenMode::Read:
		flag |= O_RDONLY;	break;
	case OpenMode::Write:
		flag |= O_WRONLY;	break;
	default:
		flag |= O_RDWR;		break;
	}

	// 書き込みは常にファイルの末尾に行うかを求める

	if (mode & OpenMode::Append)
		flag |= O_APPEND;

	// 指定されたオープンモードから生成に関する指定を取り出す

	switch (mode & OpenMode::MaskCreate) {
	case OpenMode::Create:
		flag |= O_CREAT;			break;
	case OpenMode::Create | OpenMode::Truncate:
		flag |= O_CREAT | O_TRUNC;	break;
	case OpenMode::Create | OpenMode::Exclusive:
	case OpenMode::Create | OpenMode::Truncate | OpenMode::Exclusive:
		flag |= O_CREAT | O_EXCL;	break;
	case OpenMode::Truncate:
	case OpenMode::Truncate | OpenMode::Exclusive:
		flag |= O_TRUNC;			break;
	}
#ifdef USE_OPEN_SYNC

	// 指定されたオープンモードから同期に関する指定を取り出す

	switch (mode & OpenMode::MaskSync) {
	case OpenMode::WriteDataSync:
		flag |= O_DSYNC;			break;
	case OpenMode::WriteInodeSync | OpenMode::WriteDataSync:
		flag |= O_SYNC;				break;
	case OpenMode::ReadSync | OpenMode::WriteDataSync:
		flag |= O_RSYNC | O_DSYNC;	break;
	case OpenMode::ReadSync |
			OpenMode::WriteInodeSync | OpenMode::WriteDataSync:
		flag |= O_RSYNC | O_SYNC;	break;
	}
#else
	//【注意】	LINUX で IDE と(Adaptec 越しの) SCSI ハードディスクに対して
	//			writev で書き込みを行ったとき、
	//			O_SYNC や O_DSYNC が立っていると、
	//			どうもひとつの iovec を書き込むごとに
	//			ディスクへのフラッシュが行われるらしく、
	//			ものすごく時間がかかる
	//
	//			USE_OPEN_SYNC を定義しないことで、
	//			O_SYNC や O_DSYNC を立てずにファイルをオープンし、
	//			write のたびに fsync するようになる

	//【注意】	O_RSYNC は実現できないし、
	//			::fdatasync を使えば O_DSYNC は実現できるが、
	//			現状は実装していない
#endif
	// 求めた指定を使って、ファイルをオープンする

	_descriptor = ::open(getPath(), flag, permission & Permission::Mask);
	if (_descriptor == -1) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(getPath(), _Literal::open, osErrno);
	}
#endif
	// オープンモードを記憶する

	_openMode = mode;
}

//	FUNCTION public
//	Os::File::close -- ファイルをクローズする
//
//	NOTES
//		クローズするファイルはオープンされている必要がある
//
//		クローズするファイルがロックされていれば、まず、アンロックされる
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
#ifdef SYD_OS_WINDOWS
	if (_openMode & OpenMode::Write && !(_openMode & OpenMode::WriteThrough))

		// オープンしているファイルをクローズしても、
		// OS は自動的にバッファリング内容を
		// フラッシュしないので、フラッシュする

		flush();

	if (_locked)

		// ファイルがロックされているので、アンロックする
		//
		//【注意】	MSDN Library によれば、
		//			ファイルのクローズ時にアンロックしなくても、
		//			Windows によりアンロックされるはずであるが、
		//			アンロックされるタイミングが利用可能な
		//			システムリソースに依存するため、
		//			明示的にアンロックすることが推奨されるとのことである

		unlock();

	// オープンしているファイルをクローズする

	if (!::CloseHandle(_handle)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_OS_THROW(getPath(), _Literal::CloseHandle, osErrno);
	}

	// オープンしているファイルに関する情報を初期化する

	_handle = INVALID_HANDLE_VALUE;
#endif
#ifdef SYD_OS_POSIX

	// バッファリングしている内容をフラッシュする

	if (_openMode & OpenMode::Write)
		
		if (::fdatasync(_descriptor) == -1) {
			const int osErrno = errno;
			_TRMEISTER_OS_THROW(getPath(), _Literal::fdatasync, osErrno);
		}
	
	// オープンしているファイルをクローズする

	if (::close(_descriptor) == -1) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(getPath(), _Literal::close, osErrno);
	}

	// オープンしているファイルに関する情報を初期化する

	_descriptor = -1;
#endif
	_openMode = OpenMode::None;
}

//	FUNCTION public
//	Os::File::seek -- ファイルポインタを移動する
//
//	NOTES
//		WINDOWS で、ファイルサイズを
//		Os::File::seek(0, Os::File::SeekWhence::End)
//		で求めると、ファイルポインタが不正になる可能性がある
//
//	ARGUMENTS
//		Os::File::Offset	offset
//			ファイルポインタの移動量(B 単位)
//		Os::File::SeekWhence::Value	whence
//			ファイルポインタをどこから移動するかを表す
//
//	RETURN
//		ファイルの先頭から移動後のファイルポインタのオフセット(B 単位)
//
//	EXCEPTIONS

File::Offset
File::seek(Offset offset, SeekWhence::Value whence)
{
	// 指定された位置にファイルポインタを移動する

#ifdef SYD_OS_WINDOWS
	ModStructuredInt64	position;
	LONG				high;

	position.full = offset;
	high = position.halfs.high;

	static const DWORD	table[] = { FILE_BEGIN, FILE_CURRENT, FILE_END };

	position.halfs.low =
		::SetFilePointer(_handle, position.halfs.low, &high, table[whence]);

	if (position.halfs.low == 0xffffffff) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_OS_THROW(getPath(), _Literal::SetFilePointer, osErrno);
	}

	position.halfs.high = high;

	return position.full;
#endif
#ifdef SYD_OS_POSIX

	static const int	table[] = { SEEK_SET, SEEK_CUR, SEEK_END };

	Offset position =
		::lseek(_descriptor, static_cast<off_t>(offset), table[whence]);

	if (position == -1) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(getPath(), _Literal::lseek, osErrno);
	}

	return position;
#endif
}

#ifdef SYD_OS_POSIX // Linux版のみで使用される

//	FUNCTION public
//	Os::File::read --
//		ファイルの現在のファイルポインタの指す位置から
//		あるサイズのデータを読み取り、ひとつのバッファに格納する
//
//	NOTES
//
//	ARGUMENTS
//		Os::File::IOBuffer&	buf
//			読み取ったデータを格納するバッファを表すクラス
//		Os::Memory::Size	size
//			読み取るデータのサイズ(B 単位)
//
//	RETURN
//		実際に読み取ったデータのサイズ(B 単位)
//
//	EXCEPTIONS
//		Exception::BadArgument
//			読み取った内容を格納する領域の先頭アドレスとして 0 が指定されている
//			または、読み取るサイズとして 0 が指定されている

Memory::Size
File::read(IOBuffer& buf, Memory::Size size)
{
#ifdef SYD_OS_WINDOWS

	// ファイルの現在のファイルポインタの位置を求め、
	// その位置から指定されたサイズのデータを
	// 指定された領域へ読み取る

	return read(buf, size, seek(0, SeekWhence::Current));
#endif
#ifdef SYD_OS_POSIX
	if (!static_cast<void*>(buf) || !size)

		// 読み取った内容を格納する領域の
		// 先頭アドレスとして 0 が指定されている
		//
		// または、
		//
		// 読み取るサイズとして 0 が指定されている

		_TRMEISTER_THROW0(Exception::BadArgument);

	// ファイルの現在のファイルポインタの位置から
	// 指定されたサイズのデータを指定された領域に読み取る
	//
	//【注意】	ファイルポインタは読み取り終えた位置に移動する

	ssize_t n = ::read(_descriptor, buf, size);

	if (n == -1) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(getPath(), _Literal::read, osErrno);
	}

	// 実際に読み取ったサイズを返す

	return n;
#endif
}

#endif // OBSOLETE

//	FUNCTION public
//	Os::File::read --
//		ファイルの指定された位置から
//		あるサイズのデータを読み取り、ひとつのバッファに格納する
//
//	NOTES
//
//	ARGUMENTS
//		Os::File::IOBuffer&	buf
//			読み取ったデータを格納するバッファを表すクラス
//		Os::Memory::Size	size
//			読み取ったデータのサイズ(B 単位)
//		Os::File::Offset	offset
//			読み取りを開始する位置のファイルの先頭からのオフセット(B 単位)
//
//	RETURN
//		実際に読み取ったデータのサイズ(B 単位)
//
//	EXCEPTIONS
//		Exception::BadArgument
//			読み取った内容を格納する領域の先頭アドレスとして 0 が指定されている
//			または、読み取るサイズとして 0 が指定されている

Memory::Size
File::read(IOBuffer& buf, Memory::Size size, Offset offset)
{
	if (!static_cast<void*>(buf) || !size)

		// 読み取った内容を格納する領域の
		// 先頭アドレスとして 0 が指定されている
		//
		// または、
		//
		// 読み取るサイズとして 0 が指定されている

		_TRMEISTER_THROW0(Exception::BadArgument);

#ifdef SYD_OS_WINDOWS
	OVERLAPPED* overlapped;
	int i = _Parameter::_FileRetry.get();
	DWORD	n;
#ifdef SYD_OS_WINNT4_0

	// ファイルの指定された位置からの
	// 非同期操作の実行状況を表すオブジェクトを生成する

	AsyncStatus	async(*this, offset);
	overlapped = &async._overlapped;
 retry:
#else
 retry:
	overlapped = 0;

	// 指定された位置へファイルポインタを移動する

	seek(offset, SeekWhence::Set);
#endif
	if (!::ReadFile(_handle, buf, size, &n, overlapped)) {

		const DWORD osErrno = ::GetLastError();
		switch (osErrno) {
#ifdef SYD_OS_WINNT4_0
		case ERROR_IO_PENDING:

			// 指定されたサイズのデータをすべて読み取る前に
			// 呼び出しから戻ったので、
			// すべての読み取るまで待つことにする

			n = async.wait();	break;
#endif
		case ERROR_HANDLE_EOF:

			// ファイルの末尾に達した

			n = 0;				break;

		case ERROR_SHARING_VIOLATION:
		case ERROR_LOCK_VIOLATION:

			// 別アプリがアクセスしていてアクセスできない
			// 少し待って再実行する

			if (--i) {

				SydMessage << "Sharing Violation (ReadFile) : " << osErrno
						   << ModEndl;
				::Sleep(1000);

				goto retry;
			}

		default:

			// システムコールのエラーを表す例外を投げる

			_TRMEISTER_OS_THROW(getPath(), _Literal::ReadFile, osErrno);
		}
	}
#endif
#ifdef SYD_OS_POSIX

	// ファイルの先頭から指定されたオフセットの位置から
	// 指定されたサイズのデータを指定された領域に読み取る
	//
	//【注意】	::pread を呼び出してもファイルポインタは移動しないが、
	//			WINDOWS と仕様を合わせるために
	//			ファイルポインタの位置は不定になることにする

	ssize_t n = ::pread(_descriptor, buf, size, static_cast<off_t>(offset));

	if (n == -1) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(getPath(), _Literal::pread, osErrno);
	}
#endif
	// 実際に読み取ったサイズを返す

	return n;
}

//	FUNCTION public
//	Os::File::read --
//		ファイルの現在のファイルポインタの指す位置から
//		あるサイズのデータを読み取り、複数のバッファに格納する
//
//	NOTES
//		WINDOWS の場合、オープンモードに Os::File::OpenMode::NoBuffering を
//		与えて、読み取るファイルをオープンしている必要がある
//
//		WINDOWS の場合、ファイルの現在のファイルポインタの指す位置は、
//		システムのメモリページサイズの倍数である必要がある
//
//	ARGUMENTS
//		Os::File::IOBuffer	bufs[]
//			読み取ったデータを格納するバッファを表すクラスの配列で、
//			読み取ったデータは、配列の先頭から順に格納されていく
//
//			配列の末尾のバッファの実体である
//			領域の先頭アドレスは 0 である必要がある
//
//			WINDOWS の場合、配列の末尾以外のバッファの実体である
//			領域の先頭アドレスは Os::SysConf::PageSize::get で得られる
//			システムのメモリページサイズの境界であり、
//			そのサイズはシステムのメモリページサイズである必要がある
//		Os::Memory::Size	size
//			読み取るデータのサイズ(B 単位)
//
//			WINDOWS の場合、システムのメモリページサイズの倍数である必要がある
//		unsigned int		count
//			引数 bufs に指定する配列の長さ
//
//	RETURN
//		実際に読み取ったデータのサイズ(B 単位)
//
//	EXCEPTIONS
//		Exception::BadArgument
//			読み取った内容を格納する領域の先頭アドレスを
//			記憶する配列へのポインタとして 0 が指定されている
//			または、読み取った内容を格納する領域の先頭アドレスを
//			記憶する配列のサイズとして 0 が指定されている
//			または、読み取るサイズとして 0 か、
//			読み取った内容を格納する領域の先頭アドレスを
//			記憶する配列のサイズで割り切れない値が指定されている

Memory::Size
File::read(IOBuffer bufs[], Memory::Size size, unsigned int count)
{
#ifdef SYD_OS_WINDOWS

	// ファイルの現在のファイルポインタの位置を求め、
	// その位置から指定されたサイズのデータを読み取り、
	// 指定された複数の領域へ格納する

	return read(bufs, size, count, seek(0, SeekWhence::Current));
#endif
#ifdef SYD_OS_POSIX
	if (!bufs || !count || !size || size % count) 

		// 読み取った内容を格納する領域の先頭アドレスを
		// 記憶する配列へのポインタとして 0 が指定されている
		//
		// または
		//
		// 読み取った内容を格納する領域の先頭アドレスを
		// 記憶する配列のサイズとして 0 が指定されている
		//
		// または
		//
		// 読み取るサイズとして 0 か、
		// 読み取った内容を格納する領域の先頭アドレスを
		// 記憶する配列のサイズで割り切れない値が指定されている

		_TRMEISTER_THROW0(Exception::BadArgument);

	// 読み込んだ内容を格納する領域のサイズを求める

	size /= count;

	Memory::Size total = 0;
	unsigned int i = 0, m;

	do {
		// 一度にいくつの読み込んだ内容を格納する領域へ
		// データを読み込むか求める
		//
		//【注意】	一度に最大で IOV_MAX の領域に対してデータを読み込める

		if ((m = count - i) > IOV_MAX)
			m = IOV_MAX;

		// ファイルから読みかんだデータを格納するバッファに対して
		// バッファのサイズを設定する

		unsigned int j = 0;
		do {
			bufs[i + j]._iovec.iov_len = size;
		} while (++j < m) ;

		// ファイルの現在のファイルポインタの位置から
		// データを読み込み、先ほど求めた個数の領域へ格納する
		//
		//【注意】	ファイルポインタは読み込み終えた位置に移動するが、
		//			WINDOWS と仕様を合わせるために
		//			ファイルポインタの位置は不定になることにする

		ssize_t n = ::readv(
			_descriptor, syd_reinterpret_cast<const struct iovec*>(bufs + i), m);
		
		if (n == -1) {

			// システムコールのエラーを表す例外を投げる

			const int osErrno = errno;
			_TRMEISTER_OS_THROW(getPath(), _Literal::readv, osErrno);
		}

		// これまでに読み込んだサイズを計算する

		total += n;

	} while ((i += m) < count) ;

	// 実際に読み込んだサイズを返す

	return total;
#endif
}

//	FUNCTION public
//	Os::File::read --
//		ファイルの指定された位置から
//		あるサイズのデータを読み込み、複数のバッファに格納する
//
//	NOTES
//		WINDOWS の場合、オープンモードに Os::File::OpenMode::NoBuffering を
//		与えて、読み込むファイルをオープンしている必要がある
//
//	ARGUMENTS
//		Os::File::IOBuffer	bufs[]
//			読み込んだデータを格納するバッファを表すクラスの配列で、
//			読み込んだデータは、配列の先頭から順に格納されていく
//
//			配列の末尾のバッファの実体である
//			領域の先頭アドレスは 0 である必要がある
//
//			WINDOWS の場合、配列の末尾以外のバッファの実体である
//			領域の先頭アドレスは Os::SysConf::PageSize::get で得られる
//			システムのメモリページサイズの境界であり、
//			そのサイズはシステムのメモリページサイズである必要がある
//		Os::Memory::Size	size
//			読み込むデータのサイズ(B 単位)
//
//			WINDOWS の場合、システムのメモリページサイズの倍数である必要がある
//		unsigned int		count
//			引数 bufs に指定する配列の長さ
//		Os::File::Offset	offset
//			読み込みを開始する位置のファイルの先頭からのオフセット(B 単位)
//
//			WINDOWS の場合、システムのメモリページサイズの倍数である必要がある
//
//	RETURN
//		実際に読み込んだデータのサイズ(B 単位)
//
//	EXCEPTIONS
//		Exception::BadArgument
//			読み込んだ内容を格納する領域の先頭アドレスを
//			記憶する配列へのポインタとして 0 が指定されている
//			または、読み込んだ内容を格納する領域の先頭アドレスを
//			記憶する配列のサイズとして 0 が指定されている
//			または、読み込むサイズとして 0 か、
//			読み込んだ内容を格納する領域の先頭アドレスを
//			記憶する配列のサイズで割り切れない値が指定されている

Memory::Size
File::read(IOBuffer bufs[], Memory::Size size,
		   unsigned int count, Offset offset)
{
#ifdef SYD_OS_WINDOWS
	if (!bufs || !count || !size || size % count) 

		// 読み込んだ内容を格納する領域の先頭アドレスを
		// 記憶する配列へのポインタとして 0 が指定されている
		//
		// または
		//
		// 読み込んだ内容を格納する領域の先頭アドレスを
		// 記憶する配列のサイズとして 0 が指定されている
		//
		// または
		//
		// 読み込むサイズとして 0 か、
		// 読み込んだ内容を格納する領域の先頭アドレスを
		// 記憶する配列のサイズで割り切れない値が指定されている

		_TRMEISTER_THROW0(Exception::BadArgument);

#ifdef SYD_OS_WINNT4_0

	// ファイルの指定された位置への
	// 非同期操作の実行状況を表すオブジェクトを生成する

	AsyncStatus	async(*this, offset);

	// ファイルの指定された位置から
	// 指定されたサイズのデータを非同期に読み込み、
	// 指定された複数の領域へ格納する
	//
	//【注意】	::ReadFileScatter を呼び出しても
	//			ファイルポインタは移動しないが、
	//			POSIX と仕様を合わせるために
	//			ファイルポインタの位置は不定になることにする

	int i = _Parameter::_FileRetry.get();
	Memory::Size total;
 retry:
	if (::ReadFileScatter(_handle,
						  syd_reinterpret_cast<FILE_SEGMENT_ELEMENT*>(bufs),
						  size, 0, &async._overlapped))

		// エラーが起きなかったので、
		// 指定されたサイズのデータを実際に読み込んでいるはず

		total = size;
	else {
		const DWORD osErrno = ::GetLastError();
		switch (osErrno) {
		case ERROR_IO_PENDING:

			// 指定されたサイズのデータをすべて読み込む前に
			// 呼び出しから戻ったので、
			// すべて読み込むまで待つことにする

			total = async.wait();	break;

		case ERROR_HANDLE_EOF:

			// ファイルの末尾に達した

			total = size;			break;

		case ERROR_SHARING_VIOLATION:
		case ERROR_LOCK_VIOLATION:

			// 別アプリがアクセスしていてアクセスできない
			// 少し待って再実行する

			if (--i) {

				SydMessage << "Sharing Violation (ReadFileScatter) : "
						   << osErrno << ModEndl;
				::Sleep(1000);

				goto retry;
			}

		default:

			// システムコールのエラーを表す例外を投げる

			_TRMEISTER_OS_THROW(getPath(), _Literal::ReadFileScatter, osErrno);
		}
	}

	// 実際に読み込んだサイズを返す

	return total;
#else
	// ファイルの指定された位置から
	// 指定された複数の領域のそれぞれに対して読み込んでいく

	const Memory::Size unit = size / count;
	const Offset start = offset;

	for (unsigned int i = 0; i < count; ++i) {
		; _TRMEISTER_ASSERT(bufs[i]);
		if (!read(bufs[i], unit, offset))

			// ファイルの末尾に達した

			break;

		offset += unit;
	}

	// 実際に読み込んだサイズを計算し、返す

	return static_cast<Memory::Size>(offset - start);
#endif
#endif
#ifdef SYD_OS_POSIX

	// ファイルの先頭から指定されたオフセットの位置に
	// ファイルポインタを移動する

	(void) seek(offset, SeekWhence::Set);

	// 移動後のファイルポインタの位置から
	// 指定されたサイズのデータを読み込み、
	// 指定された複数の領域へ格納する

	return read(bufs, size, count);
#endif
}

//	FUNCTION public
//	Os::File::write --
//		ひとつのバッファからあるサイズのデータを
//		ファイルの現在のファイルポインタの指す位置へ書き込む
//
//	NOTES
//		オープンモードに Os::File::OpenMode::Append を与えて、
//		書き込むファイルをオープンしている場合、
//		ファイルポインタをファイルの末尾に移動してから、
//		書き込みが行われる
//
//	ARGUMENTS
//		Os::File::IOBuffer& buf
//			書き込むデータを格納するバッファを表すクラス
//		Os::Memory::Size	size
//			書き込むデータのサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			書き込む内容を格納する領域の先頭アドレスとして 0 が指定されている
//			または、書き込むサイズとして 0 が指定されている

void
File::write(const IOBuffer& buf, Memory::Size size)
{
#ifdef SYD_OS_WINDOWS

	// ファイルの現在のファイルポインタの位置を求め、
	// その位置へ指定されたサイズのデータを書き込む

	write(buf, size,
		  (_openMode & OpenMode::Append) ? 0 : seek(0, SeekWhence::Current));
#endif
#ifdef SYD_OS_POSIX

	if (!static_cast<const void*>(buf) || !size)

		// 書き込む内容を格納する領域の先頭アドレスとして
		// 0 が指定されている
		//
		// または、
		//
		// 書き込むサイズとして 0 が指定されている

		_TRMEISTER_THROW0(Exception::BadArgument);

	// 指定された領域から指定されたサイズのデータを
	// ファイルの現在のファイルポインタの位置へ書き込む
	//
	//【注意】	ファイルポインタは書き込み終えた位置へ移動する

	const unsigned char* p =
		static_cast<unsigned char*>(
			static_cast<void*>(const_cast<IOBuffer&>(buf)));
	Memory::Size n;
retry:
	n = ::write(_descriptor, p, size);

	if (size != n) {
		if (n == -1) {

			// システムコールのエラーを表す例外を投げる

			const int osErrno = errno;
			_TRMEISTER_OS_THROW(getPath(), _Literal::write, osErrno);
		}

		// なんらかの理由ですべてを書き込めなかったので、
		// 残りを再度書き込んでみる
		//
		//【注意】	0 が返ったときでも、もう一度書き込むと -1 が返り、
		//			ENOSPC のエラーになる

		p += n;
		size -= n;
		goto retry;
	}
#ifndef USE_OPEN_SYNC
	if (_openMode & OpenMode::WriteDataSync)

		// ファイルのオープン時に O_SYNC や O_DSYNC を使わない場合
		// 書き込みのたびに明示的にフラッシュする

		flush();
#endif
#endif
}

//	FUNCTION public
//	Os::File::write --
//		ひとつのバッファからあるサイズのデータを
//		ファイルの指定された位置へ書き込む
//
//	NOTES
//		オープンモードに Os::File::OpenMode::Append を与えて、
//		書き込むファイルをオープンしている場合、
//		指定されたオフセットは無視されて、
//		必ずファイルの末尾にデータは書き込まれる
//
//	ARGUMENTS
//		Os::File::IOBuffer& buf
//			書き込むデータを格納するバッファを表すクラス
//		Os::Memory::Size	size
//			書き込むデータのサイズ(B 単位)
//		Os::File::Offset	offset
//			書き込みを開始する位置のファイルの先頭からのオフセット(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			書き込む内容を格納する領域の先頭アドレスとして 0 が指定されている
//			または、書き込むサイズとして 0 が指定されている

void
File::write(const IOBuffer& buf, Memory::Size size, Offset offset)
{
	if (!static_cast<const void*>(buf) || !size)

		// 書き込む内容を格納する領域の先頭アドレスとして
		// 0 が指定されている
		//
		// または、
		//
		// 書き込むサイズとして 0 が指定されている

		_TRMEISTER_THROW0(Exception::BadArgument);

#ifdef SYD_OS_WINDOWS

	// 条件を満たせば、システムコールエラーの例外を発生させる
	//
	//【注意】	ERROR_HADNLE_DISK_FULL を返さないらしい

	_TRMEISTER_OS_FAKE_ERROR_SYSTEMCALL(
		_Literal::func_write_error_DiskFull,
		_Literal::WriteFile, ERROR_DISK_FULL);

	OVERLAPPED* overlapped;
#ifdef SYD_OS_WINNT4_0

	// ファイルの指定された位置への
	// 非同期操作の実行状況を表すオブジェクトを生成する
	//
	//【注意】	常にファイルの末尾に書き込むときは、
	//			ファイルサイズを求めて、
	//			ファイルの先頭からその位置へ書き込むことにする

	AsyncStatus	async(
		*this, (_openMode & OpenMode::Append) ? getSize() : offset);
	overlapped = &async._overlapped;
#else
	overlapped = 0;

	// 指定された位置へファイルポインタを移動する

	seek((_openMode & OpenMode::Append) ? getSize() : offset, SeekWhence::Set);
#endif
	// 指定された領域から指定されたサイズのデータを
	// ファイルの現在のファイルポインタの位置へ書き込む
	//
	//【注意】	ファイルポインタは書き込み終えた位置へ移動する

	unsigned char* p =
		static_cast<unsigned char*>(
			static_cast<void*>(const_cast<IOBuffer&>(buf)));

	//【注意】	Windows 95/98 では ::WriteFile の
	//			pNumberOfBytesWritten に 0 を指定できない

	int i = _Parameter::_FileRetry.get();
	DWORD	n;
 retry:
	if (!::WriteFile(_handle, p, size, &n, overlapped)) {

		const DWORD osErrno = ::GetLastError();
		switch (osErrno) {
#ifdef SYD_OS_WINNT4_0
		case ERROR_IO_PENDING:

			// 指定されたサイズのデータをすべて書き込む前に
			// 呼び出しから戻ったので、
			// すべて書き込むまで待つことにする

			(void) async.wait();		break;
#endif
		case ERROR_NO_SYSTEM_RESOURCES:
		{
			// 書き込もうとしている領域のサイズが大きすぎるので、
			// 半分づつ書き込んでみる

			const Memory::Size half_size = size >> 1;
			
			write(buf, half_size, offset);
			write(IOBuffer(p + half_size),
				  size - half_size, offset + half_size);

			// 結局、書き込めた

			break;
		}
		
		case ERROR_SHARING_VIOLATION:
		case ERROR_LOCK_VIOLATION:

			// 別アプリがアクセスしていてアクセスできない
			// 少し待って再実行する

			if (--i) {

				SydMessage << "Sharing Violation (WriteFile) : "
						   << osErrno << ModEndl;
				::Sleep(1000);

				goto retry;
			}

		default:

			// システムコールのエラーを表す例外を投げる

			_TRMEISTER_OS_THROW(getPath(), _Literal::WriteFile, osErrno);
		}
	}

	//【注意】	エラーが起きたなかったときは、
	//			指定されたサイズのデータを書き込んだこととみなしている
#endif
#ifdef SYD_OS_POSIX

	// 指定された領域から指定されたサイズのデータを
	// ファイルの先頭から指定されたオフセットの位置へ書き込む
	//
	//【注意】	::pwrite を呼び出してもファイルポインタは移動しないが、
	//			WINDOWS と仕様を合わせるために
	//			ファイルポインタは不定になることとする

	const unsigned char* p =
		static_cast<unsigned char*>(
			static_cast<void*>(const_cast<IOBuffer&>(buf)));
retry:
	ssize_t n = ::pwrite(_descriptor, p, size, static_cast<off_t>(offset));

	if (size != n) {
		if (n == -1) {

			// システムコールのエラーを表す例外を投げる

			const int osErrno = errno;
			_TRMEISTER_OS_THROW(getPath(), _Literal::pwrite, osErrno);
		}

		// なんらかの理由ですべてを書き込めなかったので、
		// 残りを再度書き込んでみる
		//
		//【注意】	0 が返ったときでも、もう一度書き込むと -1 が返り、
		//			ENOSPC のエラーになる

		p += n;
		size -= n;
		offset += n;
		goto retry;
	}
#ifndef USE_OPEN_SYNC
	if (_openMode & OpenMode::WriteDataSync)

		// ファイルのオープン時に O_SYNC や O_DSYNC を使わない場合
		// 書き込みのたびに明示的にフラッシュする

		flush();
#endif
#endif
}

//	FUNCTION public
//	Os::File::write --
//		複数のバッファからあるサイズのデータを
//		ファイルの現在のファイルポインタの指す位置へ書き込む
//
//	NOTES
//		オープンモードに Os::File::OpenMode::Append を与えて、
//		書き込むファイルをオープンしている場合、
//		ファイルポインタをファイルの末尾に移動してから、
//		書き込みが行われる
//
//		WINDOWS の場合、オープンモードに Os::File::OpenMode::NoBuffering を
//		与えて、書き込むファイルをオープンしている必要がある
//
//		WINDOWS の場合、ファイルの現在のファイルポインタの指す位置は、
//		システムのメモリページサイズの倍数である必要がある
//
//	ARGUMENTS
//		Os::File::IOBuffer	bufs[]
//			書き込むデータを格納するバッファを表すクラスの配列で、
//			配列の先頭から順にファイルへ書き込まれる
//
//			配列の末尾のバッファの実体である
//			領域の先頭アドレスは 0 である必要がある
//
//			WINDOWS の場合、配列の末尾以外のバッファの実体である
//			領域の先頭アドレスは Os::SysConf::PageSize::get で得られる
//			システムのメモリページサイズの境界であり、
//			そのサイズはシステムのメモリページサイズである必要がある
//		Os::Memory::Size	size
//			書き込むデータのサイズ(B 単位)
//
//			WINDOWS の場合、システムのメモリページサイズの倍数である必要がある
//		unsigned int		count
//			引数 bufs に指定する配列の長さ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			書き込む内容を格納する領域の先頭アドレスを
//			記憶する配列へのポインタとして 0 が指定されている
//			または、書き込む内容を格納する領域の先頭アドレスを
//			記憶する配列のサイズとして、0 が指定されている
//			または、書き込みサイズとして 0 か、
//			書き込み内容を格納する領域の先頭アドレスを
//			記憶する配列のサイズで割り切れない値が指定されている

void
File::write(const IOBuffer bufs[], Memory::Size size, unsigned int count)
{
#ifdef SYD_OS_WINDOWS

	// 現在のファイルポインタの位置を求め、
	// 指定された複数の領域から指定されたサイズのデータを
	// その位置へ書き込む
	//
	//【注意】	常にファイルの末尾に書き込むときは、
	//			現在のファイルポインタの位置を求めずに
	//			たんに 0 を与えておく

	write(bufs, size, count,
		  (_openMode & OpenMode::Append) ? 0 : seek(0, SeekWhence::Current));
#endif
#ifdef SYD_OS_POSIX
	if (!bufs || !count || !size || size % count)

		// 書き込む内容を格納する領域の先頭アドレスを
		// 記憶する配列へのポインタとして 0 が指定されている
		//
		// または、
		//
		// 書き込む内容を格納する領域の先頭アドレスを
		// 記憶する配列のサイズとして、0 が指定されている
		//
		// または、
		//
		// 書き込むサイズとして 0 か、
		// 書き込む内容を格納する領域の先頭アドレスを
		// 記憶する配列のサイズで割り切れない値が指定されている

		_TRMEISTER_THROW0(Exception::BadArgument);
#ifdef NORMAL_WRITEV

	// 書き込む内容を格納する領域のサイズを求める

	const Os::Memory::Size unit = size / count;

	Memory::Size total = 0;
	unsigned int i = 0, m;

	do {
		// 一度にいくつの書き込む内容を格納する領域から
		// データを書き込むか求める
		//
		//【注意】	一度に最大で IOV_MAX の領域に対してデータを読み込める

		if ((m = count - i) > IOV_MAX)
			m = IOV_MAX;

		// ファイルへ書き込むデータを格納するバッファに対して
		// バッファのサイズを設定する

		unsigned int j = 0;
		do {
			bufs[i + j]._iovec.iov_len = unit;
		} while (++j < m) ;

		// 先ほど求めた個数の領域から
		// ファイルの現在のファイルポインタの位置へ
		// データを書き込む
		//
		//【注意】	ファイルポインタは書き込み終えた位置に移動するが、
		//			WINDOWS と仕様を合わせるために
		//			ファイルポインタの位置は不定になることにする
retry:
		ssize_t n = ::writev(
			_descriptor, syd_reinterpret_cast<const struct iovec*>(bufs + i), m);

		if (unit * m != n) {
			if (n == -1) {

				// システムコールのエラーを表す例外を投げる

				const int osErrno = errno;
				_TRMEISTER_OS_THROW(getPath(), _Literal::writev, osErrno);
			}

			// なんらかの理由ですべて書き込めなかったので、
			// 残りを再度書き込んでみる
			//
			//【注意】	0 が返ったときでも、もう一度書き込むと -1 が返り、
			//			ENOSPC のエラーになる
			//【注意】	static_cast<char*>(bufs[i]._iovec.iov_base) += (n % unit)
			//			と書くと、Intel Compiler でエラーになる

			i += n / unit;
			m -= n / unit;
			bufs[i]._iovec.iov_base
				= static_cast<char*>(bufs[i]._iovec.iov_base) + (n % unit);
			bufs[i]._iovec.iov_len -= n % unit;
			goto retry;
		}
	} while ((i += m) < count) ;
#ifndef USE_OPEN_SYNC
	if (_openMode & OpenMode::WriteDataSync)

		// ファイルのオープン時に O_SYNC や O_DSYNC を使わない場合
		// 書き込みのたびに明示的にフラッシュする

		flush();
#endif
#else
	// 書き込みを行うファイル領域をマップする

	void* p = ::mmap(0, size, PROT_WRITE, MAP_SHARED,
					 _descriptor, seek(0, SeekWhence::Current));

	if (p == MAP_FAILED) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(getPath(), _Literal::mmap, osErrno);
	}

	// 書き込む内容を格納する領域のサイズを求める

	const Os::Memory::Size unit = size / count;

	// マップされた領域に与えられた書き込み内容をコピーする

	char* q = static_cast<char*>(p);
	unsigned int i = 0;

	do {
		(void) Os::Memory::copy(q, bufs[i]._iovec.iov_base, unit);
		q += unit;

	} while (++i < count);

#ifdef SYD_OS_SOLARIS
	// Solaris requires char* as the 1st argument
	if (::munmap(syd_reinterpret_cast<char*>(p), size) == -1) {
#else
	if (::munmap(p, size) == -1) {
#endif

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(getPath(), _Literal::munmap, osErrno);
	}
#endif
#endif
}

//	FUNCTION public
//	Os::File::write --
//		複数のバッファからあるサイズのデータを
//		ファイルの指定された位置へ書き込む
//
//	NOTES
//		オープンモードに Os::File::OpenMode::Append を与えて、
//		書き込むファイルをオープンしている場合、
//		指定されたオフセットは無視されて、
//		必ずファイルの末尾にデータは書き込まれる
//
//		WINDOWS の場合、オープンモードに Os::File::OpenMode::NoBuffering を
//		与えて、書き込むファイルをオープンしている必要がある
//
//	ARGUMENTS
//		Os::File::IOBuffer	bufs[]
//			書き込むデータを格納するバッファを表すクラスの配列で、
//			配列の先頭から順にファイルへ書き込まれる
//
//			配列の末尾のバッファの実体である
//			領域の先頭アドレスは 0 である必要がある
//
//			WINDOWS の場合、配列の末尾以外のバッファの実体である
//			領域の先頭アドレスは Os::SysConf::PageSize::get で得られる
//			システムのメモリページサイズの境界であり、
//			そのサイズはシステムのメモリページサイズである必要がある
//		Os::Memory::Size	size
//			書き込むデータのサイズ(B 単位)
//
//			WINDOWS の場合、システムのメモリページサイズの倍数である必要がある
//		unsigned int		count
//			引数 bufs の末尾を除いた要素数
//		Os::File::Offset	offset
//			書き込みを開始する位置のファイルの先頭からのオフセット(B 単位)
//
//			WINDOWS の場合、システムのメモリページサイズの倍数である必要がある
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			書き込む内容を格納する領域の先頭アドレスを
//			記憶する配列へのポインタとして 0 が指定されている
//			または、書き込む内容を格納する領域の先頭アドレスを
//			記憶する配列のサイズとして、0 が指定されている
//			または、書き込みサイズとして 0 か、
//			書き込む内容を格納する領域の先頭アドレスを
//			記憶する配列のサイズで割り切れない値が指定されている

void
File::write(const IOBuffer bufs[], Memory::Size size,
			unsigned int count, Offset offset)
{
#ifdef SYD_OS_WINDOWS
	if (!bufs || !count || !size || size % count)

		// 書き込む内容を格納する領域の先頭アドレスを
		// 記憶する配列へのポインタとして 0 が指定されている
		//
		// または、
		//
		// 書き込む内容を格納する領域の先頭アドレスを
		// 記憶する配列のサイズとして、0 が指定されている
		//
		// または、
		//
		// 書き込みサイズとして 0 か、
		// 書き込む内容を格納する領域の先頭アドレスを
		// 記憶する配列のサイズで割り切れない値が指定されている

		_TRMEISTER_THROW0(Exception::BadArgument);

#ifdef SYD_OS_WINNT4_0

	// 条件を満たせば、システムコールエラーの例外を発生させる
	//
	//【注意】	ERROR_HADNLE_DISK_FULL を返さないらしい

	_TRMEISTER_OS_FAKE_ERROR_SYSTEMCALL(
		_Literal::func_write_error_DiskFull,
		_Literal::WriteFileGather, ERROR_DISK_FULL);

	// ファイルの指定された位置への
	// 非同期操作の実行状況を表すオブジェクトを生成する
	//
	//【注意】	常にファイルの末尾に書き込むときは、
	//			ファイルサイズを求めて、
	//			ファイルの先頭からその位置へ書き込むことにする

	AsyncStatus async(
		*this, (_openMode & OpenMode::Append) ? getSize() : offset);

	// 指定された複数の領域から指定されたサイズのデータを
	// ファイルの指定された位置へ非同期に書き込む
	//
	//【注意】	::WriteFileGather を呼び出しても
	//			ファイルポインタは移動しないが、
	//			POSIX と仕様を合わせるために
	//			ファイルポインタの位置は不定になることとする

	int i = _Parameter::_FileRetry.get();
 retry:
	if (!::WriteFileGather(_handle,
						   syd_reinterpret_cast<FILE_SEGMENT_ELEMENT*>(
							   const_cast<IOBuffer*>(bufs)),
						   size, 0, &async._overlapped)) {

		const DWORD osErrno = ::GetLastError();
		switch (osErrno) {
		case ERROR_IO_PENDING:

			// 指定されたサイズのデータをすべて書き込む前に
			// 呼び出しから戻ったので、
			// すべて書き込むまで待つことにする

			(void) async.wait();		break;

		case ERROR_NO_SYSTEM_RESOURCES:
		case ERROR_WORKING_SET_QUOTA:
			if (count > 1) {

				// 一度に書き込もうとしている領域の数が多すぎるので、
				// 半分づつ書き込んでみる
				//
				//【注意】	Windows NT 4.0 と 2000 で試したところ、
				//			両方とも count == 16376 以上で
				//			ERROR_NO_SYSTEM_RESOURCES になった
				//
				//			また、この数値以下でも場合によって
				//			ERROR_WORKING_SET_QUOTA になることがある
				//
				//			この数値を埋め込みたくないので、半々に処理を行う

				const unsigned int	half_count = count >> 1;
				const Memory::Size	half_size = size / count * half_count;

				const IOBuffer saved(bufs[half_count]);
				const_cast<IOBuffer*>(bufs)[half_count] = 0;

				write(bufs, half_size, half_count, offset);

				const_cast<IOBuffer*>(bufs)[half_count] = saved;

				write(bufs + half_count, size - half_size,
					  count - half_count, offset + half_size);

				// 結局、書き込めた

				break;
			}

			// システムコールのエラーを表す例外を投げる

			_TRMEISTER_OS_THROW(getPath(), _Literal::WriteFileGather, osErrno);
			break;

		case ERROR_SHARING_VIOLATION:
		case ERROR_LOCK_VIOLATION:

			// 別アプリがアクセスしていてアクセスできない
			// 少し待って再実行する

			if (--i) {

				SydMessage << "Sharing Violation (WriteFileGather) : "
						   << osErrno << ModEndl;
				::Sleep(1000);

				goto retry;
			}

		default:

			// システムコールのエラーを表す例外を投げる

			_TRMEISTER_OS_THROW(getPath(), _Literal::WriteFileGather, osErrno);
		}
	}
#else
	// ファイルの指定された位置から
	// 指定された複数の領域のそれぞれを書き込んでいく

	const Memory::Size unit = size / count;

	for (unsigned int i = 0; i < count; ++i) {
		; _TRMEISTER_ASSERT(bufs[i]);
		write(bufs[i], unit, offset);
		
		offset += unit;
	}
#endif
	//【注意】	エラーが起きたなかったときは、
	//			指定されたサイズのデータを書き込んだこととみなしている
#endif
#ifdef SYD_OS_POSIX

	// ファイルの先頭から指定されたオフセットの位置に
	// ファイルポインタを移動する

	(void) seek(offset, SeekWhence::Set);

	// 指定された複数の領域から指定されたサイズのデータを
	// 移動後のファイルポインタの位置へ書き込む

	write(bufs, size, count);
#endif
}

//	FUNCTION public
//	Os::File::flush --
//		バッファリングされている書き込み内容をファイルへフラッシュする
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
File::flush()
{
#ifdef SYD_OS_WINDOWS
	if (!::FlushFileBuffers(_handle)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_OS_THROW(getPath(), _Literal::FlushFileBuffers, osErrno);
	}
#endif
#ifdef SYD_OS_POSIX

	// OS が管理しているバッファをすべてフラッシュする
	//
	//【注意】
	//	ファイルのメタデータを更新する必要はないので、
	//	fsyncではなくfdatasyncを利用する

	if (::fdatasync(_descriptor) == -1) {
		const int osErrno = errno;
		_TRMEISTER_OS_THROW(getPath(), _Literal::fdatasync, osErrno);
	}
#endif
}

//	FUNCTION public
//	Os::File::remove -- 指定されたファイルを削除する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			削除するファイルの絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			削除するファイルの絶対パス名が指定されていない

// static
void
File::remove(const Path& path)
{
	if (!path.getLength())

		// 削除するファイルの絶対パス名が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

#ifdef SYD_OS_WINDOWS

	// POSIX と仕様を合わせて、
	// ファイル属性が読み取り専用でも削除できるように
	// 削除するファイルのファイル属性を読み取り専用でなくする
	//
	//【注意】	ファイルの所有者であれば、ファイル属性を変更できるはず
	//
	//【注意】	削除するファイルのセキュリティは変更しない

	chmodAttribute(path, Permission::MaskWrite);

	// ファイルを削除する

	int i = _Parameter::_FileRetry.get();
retry:
	if (!::DeleteFile(path)) {

		const DWORD osErrno = ::GetLastError();
		if (!((osErrno == ERROR_SHARING_VIOLATION ||
			   osErrno == ERROR_ACCESS_DENIED) && --i))

			// システムコールのエラーを表す例外を投げる
			
			_TRMEISTER_OS_THROW(path, _Literal::DeleteFile, osErrno);

		// ほかの誰かが削除を許可せずに
		// ファイルハンドルを生成しているので、1 秒待って再実行する
		//
		//【注意】	少なくとも Windows XP だと、
		//			削除しようとしているファイルの親ディレクトリのうち、
		//			いずれかに対して move コマンドなどでファイルを移動すると、
		//			なぜか ERROR_SHARING_VIOLATION になるので、
		//			再実行するようにした
		//
		//			追加:
		//			ERROR_ACCESS_DENIED になることもあるようなので、
		//			再実行するようにした

		SydMessage << "Sharing Violation (DeleteFile)" << ModEndl;
		
		::Sleep(1000);
		goto retry;
	}
#endif
#ifdef SYD_OS_POSIX

	// ファイルを削除する

	if (::unlink(path) == -1) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(path, _Literal::unlink, osErrno);
	}
#endif
}

//	FUNCTION public
//	Os::File::truncate -- ファイルのサイズを縮める
//
//	NOTES
//		サイズを縮めるファイルはオープンされている必要がある
//
//	ARGUMENTS
//		Os::File::Size		length
//			縮めた後のファイルのサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			トランケート後のファイルサイズが最大値を超える

void
File::truncate(Size length)
{
#ifdef SYD_OS_WINDOWS

	// トランケート後のファイルサイズの位置にファイルポインタを移動する

	if (length > static_cast<Size>(getOffsetMax())) {

		// ファイルポインタの位置は Offset で指定するので、
		// Offset の最大値より大きなサイズにトランケートするとき、
		// ファイルの末尾から、現在のファイルサイズ -
		// トランケート後のファイルサイズぶんの位置に移動する

		Offset	offset = static_cast<Offset>(getSize() - length);
		(void) seek(-offset, SeekWhence::End);
	} else
		(void) seek(static_cast<Offset>(length), SeekWhence::Set);
	
	// 移動したファイルポインタが指している場所をファイルの末尾にする

	int i = _Parameter::_FileRetry.get();
 retry:
	if (!::SetEndOfFile(_handle)) {

		const DWORD osErrno = ::GetLastError();
		switch (osErrno) {
		case ERROR_SHARING_VIOLATION:
		case ERROR_LOCK_VIOLATION:
		case ERROR_USER_MAPPED_FILE:

			// 別アプリがアクセスしていてアクセスできない
			// 少し待って再実行する

			if (--i) {

				SydMessage << "Sharing Violation (SetEndOfFile) : " << osErrno
						   << ModEndl;
				::Sleep(1000);

				goto retry;
			}

		default:

			// システムコールのエラーを表す例外を投げる

			_TRMEISTER_OS_THROW(getPath(), _Literal::SetEndOfFile, osErrno);
		}
	}
#endif
#ifdef SYD_OS_POSIX
	if (length > getSizeMax())

		// トランケート後のファイルサイズが最大値を超える

		_TRMEISTER_THROW0(Exception::BadArgument);

	// ファイルをトランケートする

	if (::ftruncate(_descriptor, length) == -1) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(getPath(), _Literal::ftruncate, osErrno);
	}
#endif
}

#ifdef OBSOLETE
//	FUNCTION public
//	Os::File::truncate -- 指定されたファイルのサイズを縮める
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			サイズを縮めるファイルの絶対パス名
//		Os::File::Size		length
//			指定されたとき
//				縮めた後のファイルのサイズ(B 単位)
//			指定されないとき
//				0 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			トランケートするファイルの絶対パス名が指定されていない
//			または、トランケート後のファイルサイズが最大値を超える

// static
void
File::truncate(const Path& path, Size length)
{
#ifdef SYD_OS_WINDOWS

	// 指定された絶対パス名のファイルをオープンする

	File	file(path);
	file.open(OpenMode::Read | OpenMode::Write);

	// オープンしたファイルをトランケートする

	file.truncate(length);
#endif
#ifdef SYD_OS_POSIX
	if (!path.getLength() || length > getSizeMax())

		// トランケートするファイルの絶対パス名が指定されていない
		//
		// または
		//
		// トランケート後のファイルサイズが最大値を超える

		_TRMEISTER_THROW0(Exception::BadArgument);

	// 指定された絶対パス名のファイルをトランケートする

	if (::truncate(path, length) == -1) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(path, _Literal::truncate, osErrno);
	}
#endif
}
#endif

#ifdef SYD_OS_POSIX // Linux版のみで使用される

//	FUNCTION public
//	Os::File::copy -- 指定されたファイルを指定されたものとしてコピーする
//
//	NOTES
#ifdef SYD_OS_POSIX
//		複写元のアクセス権に関する情報は複写先へまったくコピーされない
#endif
//
//	ARGUMENTS
//		Os::Path&			srcPath
//			コピーするファイルの絶対パス名で、
//			存在するファイルのものである必要がある
//		Os::Path&			dstPath
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
//		Exception::BadArgument
//			コピー元またはコピー先のファイルの絶対パス名が指定されていない

// static
void
File::copy(const Path& srcPath, const Path& dstPath, bool force)
{
	if (!srcPath.getLength() || !dstPath.getLength())

		// コピー元またはコピー先の
		// ファイルの絶対パス名が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

#ifdef SYD_OS_WINDOWS

	//【注意】	WIN32API には ::CopyFile というファイルをコピーする
	//			便利な関数があるが、コピー元の属性はコピーできるが、
	//			セキュリティがコピーできないので、使用しない

#endif
	// 指定されたコピー元をオープンする

	File src(srcPath);
	src.open(OpenMode::Read);

	// 指定されたコピー先を生成する

	const OpenMode::Value mode =
		OpenMode::Write | OpenMode::Create |
		((force) ? OpenMode::Truncate : OpenMode::Exclusive);

	File dst(dstPath);
	dst.open(mode);

	try {
		// システムのメモリページサイズを求めて、
		// コピー元のファイルを読み取った内容を格納する領域を
		// そのサイズで確保する

		const Memory::Size size = Os::SysConf::PageSize::get();
		void* p = Memory::allocate(size);
		; _TRMEISTER_ASSERT(p);

		try {
			// コピー元のファイルを先頭から読み取りながら、
			// コピー先のファイルへ書き込んでいく

			IOBuffer	buf(p);

			while (Memory::Size n = src.read(buf, size))
				dst.write(buf, n);

		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{

			Memory::free(p);
			_TRMEISTER_RETHROW;
		}

		Memory::free(p);

#ifdef SYD_OS_WINDOWS

		// コピー元の属性を得て、コピー先の属性として設定する

		_File::setAttribute(dstPath, _File::getAttribute(srcPath));

#ifdef SYD_OS_WINNT4_0

		// コピー元のセキュリティ情報を得る

		_AccessControl::_Info::_Type::Value	type;
		PSID	owner;
		PSID	group;
		ACL*	dacl;

		PSECURITY_DESCRIPTOR sd =
			_AccessControl::getSecurityInfo(srcPath, type, owner, group, dacl);

		try {
			// コピー先にコピー元から得たセキュリティ情報を設定する
			//
			//【注意】	このとき、親のセキュリティ情報が継承されないようにする

			_AccessControl::setSecurityInfo(
				dstPath, type | _AccessControl::_Info::_Type::ProtectedDacl,
				owner, group, dacl);

		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{

			::LocalFree(sd);
			_TRMEISTER_RETHROW;
		}

		// 必要のなくなったセキュリティ記述子を破棄する

		::LocalFree(sd);
#endif
#endif
#ifdef SYD_OS_POSIX

		// コピー元の詳細情報を求める

		struct stat buf;

		if (::fstat(src._descriptor, &buf) == -1) {

			// システムコールのエラーを表す例外を投げる

			const int osErrno = errno;
			_TRMEISTER_OS_THROW(srcPath, _Literal::fstat, osErrno);
		}

		// コピー先のファイルのアクセス権をコピー元と同じにする

		dst.chmod(buf.st_mode);
#endif
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		dst.remove();
		_TRMEISTER_RETHROW;
	}
}
#endif

//	FUNCTION public
//	Os::File::rename -- 指定されたファイルの名前を指定されたものに変更する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			srcPath
//			名前を変更するファイルの絶対パス名で、
//			存在するファイルのものである必要がある
//		Os::Path&			dstPath
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
//		Exception::BadArgument
//			変更するまたは変更後のファイルの絶対パス名が指定されていない

// static
void
File::rename(const Path& srcPath, const Path& dstPath, bool force)
{
	if (!srcPath.getLength() || !dstPath.getLength())

		// 変更するまたは変更後の
		// ファイルの絶対パス名が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WINNT4_0

	// 指定された変更するファイルの名前を
	// 指定された絶対パス名に変更する
	//
	//【注意】	::MoveFileEx はボリュームをまたがって名前を変更できる

	if (!::MoveFileEx(srcPath, dstPath,
					  MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH |
					  ((force) ? MOVEFILE_REPLACE_EXISTING : 0))) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall, _Literal::MoveFileEx, osErrno);
	}
#else
	if (force && access(dstPath, AccessMode::File))

		// 変更後のファイルが存在するので、破棄する

		remove(dstPath);

	// 指定された変更するファイルの名前を
	// 指定された絶対パス名に変更する

	if (!::MoveFile(srcPath, dstPath)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall, _Literal::MoveFile, osErrno);
	}
#endif
#endif		
#ifdef SYD_OS_POSIX

	// 指定された絶対パス名に名前を変更する

	if (::rename(srcPath, dstPath) == -1) {
		const int osErrno = errno;
		if (osErrno != EXDEV)

			// システムコールのエラーを表す例外を投げる

			_TRMEISTER_THROW2(Exception::SystemCall, _Literal::rename, osErrno);

		// ファイルシステムをまたがって名前を変更しようとしているので、
		// ファイルをコピーしてから、削除する

		copy(srcPath, dstPath, force);
		try {
			File(srcPath).remove();

		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{

			File(dstPath).remove();
			_TRMEISTER_RETHROW;
		}
	}
#endif
}

#ifdef SYD_OS_POSIX // Linux版のみで使用される

//	FUNCTION public
//	Os::File::chmod -- ファイルの許可モードを変更する
//
//	NOTES
//		WINDOWS のファイルには許可モードという概念は存在しないので、
//		指定された許可モードにほぼ同義のファイル属性とセキュリティを設定する
//
//	ARGUMENTS
//		Os::File::Permission::Value	mode
//			ファイルの変更後の許可モードを表す値で、
//			Os::File::Permission::Value の論理和を指定する
//
//	RERURN
//		なし
//
//	EXCEPTIONS

void
File::chmod(Permission::Value mode)
{
#ifdef SYD_OS_WINDOWS

	// ファイルのファイル属性およびセキュリティを
	// 指定された許可モードに合わせて変更する

	chmod(getPath(), mode);
#endif
#ifdef SYD_OS_POSIX

	// ファイルの許可モードを指定されたものに変更する

	if (::fchmod(_descriptor, mode & Permission::Mask) == -1) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(getPath(), _Literal::fchmod, osErrno);
	}
#endif
}
#endif

#ifdef OBSOLETE

//	FUNCTION public
//	Os::File::chmod -- 指定されたファイルの許可モードを変更する
//
//	NOTES
//		WINDOWS のファイルには許可モードという概念は存在しないので、
//		指定された許可モードにほぼ同義のファイル属性とセキュリティを設定する
//
//	ARGUMENTS
//		Os::Path&			path
//			許可モードを変更するファイルの絶対パス名
//		Os::File::Permission::Value	mode
//			ファイルの変更後の許可モードを表す値で、
//			Os::File::Permission::Value の論理和を指定する
//
//	RERURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			許可モードを変更するファイルの絶対パス名が指定されていない

// static
void
File::chmod(const Path& path, Permission::Value mode)
{
	if (!path.getLength())

		// 許可モードを変更するファイルの絶対パス名が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

#ifdef SYD_OS_WINDOWS
	mode &= Permission::Mask;

	// まず、ファイル属性を指定された
	// POSIX 形式の許可モードに合わせて変更する
	//
	//【注意】	所有者であれば、ファイル属性を変更できるはず
	//
	//【注意】	先にセキュリティーを変更すると、
	//			この関数の呼び出し時にこのファイルのセキュリティーが
	//			ファイル属性を変更可能であっても、できなくなる可能性がある

	chmodAttribute(path, mode);
#ifdef SYD_OS_WINNT4_0

	// 次にセキュリティを指定された
	// POSIX 形式の許可モードに合わせて変更する
	//
	//【注意】	この関数で例外が発生すると、
	//			すでに変更済みのファイル属性をもとに戻せない

	chmodSecurity(path, mode);
#endif
#endif
#ifdef SYD_OS_POSIX

	// 指定された絶対パス名のファイルの許可モードを変更する

	if (::chmod(path, mode & Permission::Mask) == -1) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(path, _Literal::chmod, osErrno);
	}
#endif
}

#endif

#ifdef SYD_OS_WINDOWS
//	FUNCTION private
//	Os::File::chmodAttribute --
//		指定されたファイルのファイル属性を
//		与えられた許可モードに合わせて変更する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			ファイル属性を変更するファイルの絶対パス名
//		Os::File::Permission::Value	mode
//			変更後のファイル属性を求めるための許可モードで、
//			Os::File::Permission の論理和を指定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			ファイル属性を変更するファイルの絶対パス名が指定されていない

// static
void
File::chmodAttribute(const Path& path, Permission::Value mode)
{
	; _TRMEISTER_ASSERT(path.getLength());

	// 現在のファイルの属性を求める

	DWORD v = _File::getAttribute(path);

	// ファイルの属性をどう変更すればいいか求める

	if (v & FILE_ATTRIBUTE_READONLY) {

		// 現在のファイルは読み取り専用のとき

		if (mode & Permission::MaskWrite)

			// 与えられたファイルの権利に
			// 書き込み可の権利があれば、読み取り専用でなくする

			v &= ~FILE_ATTRIBUTE_READONLY;
		else
			return;
	} else

		// 現在のファイルは読み取り専用でないとき

		if (mode & Permission::MaskWrite)
			return;
		else
			// 与えられたファイルの権利に
			// 書き込み可の権利がなければ、読み取り専用にする

			v |= FILE_ATTRIBUTE_READONLY;

	// ファイル属性を求めたものに変更する

	_File::setAttribute(path, v);
}

#ifdef SYD_OS_WINNT4_0
#ifdef OBSOLETE
//	FUNCTION private
//	Os::File::chmodSecurity --
//		指定されたファイルのセキュリティを
//		与えられた許可モードに合わせて変更する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			セキュリティを変更するファイルの絶対パス名
//		Os::File::Permission::Value	mode
//			変更後のセキュリティを求めるための許可モードで
//			Os::File::Permission の論理和を指定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			セキュリティを変更するファイルの絶対パス名が指定されていない

// static
void
File::chmodSecurity(const Path& path, Permission::Value mode)
{
	; _TRMEISTER_ASSERT(path.getLength());

	// 指定されたファイルの現在のセキュリティ情報を得る

	_AccessControl::_Info::_Type::Value	type;
	PSID	owner;
	PSID	group;
	ACL*	dacl0;
	
	PSECURITY_DESCRIPTOR sd =
		_AccessControl::getSecurityInfo(path, type, owner, group, dacl0);

	PSID	sids[_AccessControl::_Sid::_Type::ValueNum];
	Memory::reset(sids, sizeof(sids));

	try {
		// 求めたセキュリティ情報から所有者とプライマリーグループの SID を得る

		if (type & _AccessControl::_Info::_Type::Owner) {
			sids[_AccessControl::_Sid::_Type::User] =
				_AccessControl::copySid(owner);
			; _TRMEISTER_ASSERT(sids[_AccessControl::_Sid::_Type::User]);
		}
		if (type & _AccessControl::_Info::_Type::Group) {
			sids[_AccessControl::_Sid::_Type::Group] =
				_AccessControl::copySid(group);
			; _TRMEISTER_ASSERT(sids[_AccessControl::_Sid::_Type::Group]);
		}

		// ユーザー BUILTIN\Administrators と EveryOne の SID を生成する

		sids[_AccessControl::_Sid::_Type::Admins] =
			_AccessControl::getAdministratorsSid();
		; _TRMEISTER_ASSERT(sids[_AccessControl::_Sid::_Type::Admins]);
		sids[_AccessControl::_Sid::_Type::EveryOne] =
			_AccessControl::getEveryOneSid();
		; _TRMEISTER_ASSERT(sids[_AccessControl::_Sid::_Type::EveryOne]);

		// 求めたセキュリティ情報と与えられた UNIX 形式の許可モードから、
		// 求めた SID に関するアクセス制御リストを求める

		ACL* dacl1 = _AccessControl::setUnixAcesInAcl(dacl0, mode, sids);

		try {
			// 指定されたファイルのセキュリティ情報に
			// 求めた随意アクセス制御リストを設定する
			//
			//【注意】	このとき、親のセキュリティ情報が継承されないようにする

			_AccessControl::setSecurityInfo(
				path, _AccessControl::_Info::_Type::Dacl |
				_AccessControl::_Info::_Type::ProtectedDacl, 0, 0, dacl1);

		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{

			::LocalFree(dacl1);
			_TRMEISTER_RETHROW;
		}

		// 必要のなくなった随意アクセス制御リストを破棄する

		::LocalFree(dacl1);

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		::LocalFree(sd);
		_AccessControl::freeUnixSids(sids);
		_TRMEISTER_RETHROW;
	}

	// 必要のなくなったセキュリティー記述子と SID を破棄する

	::LocalFree(sd);
	_AccessControl::freeUnixSids(sids);
}
#endif
#endif
#endif

//	FUNCTION public
//	Os::File::access --
//		指定されたファイルにある操作を行う権利を持っているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			権利があるか調べるファイルの絶対パス名
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
//		Exception::BadArgument
//			権利があるか調べるファイルの絶対パス名が指定されていない

// static
bool
File::access(const Path& path, AccessMode::Value mode)
{
	if (!path.getLength())

		// 権利があるか調べるファイルの絶対パス名が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

	// 与えられたパス名を OS が解釈可能な文字コードに変換する

#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WINNT4_0
	if (mode == AccessMode::File) {

		// ::GetFileAttributes は遅いので、
		// ファイルの存在を確認すればよいときは ::CreateFile を使う
		//
		//【注意】	ディレクトリが指定されてもエラーにならないように
		//			FILE_FLAG_BACKUP_SEMANTICS を指定する
		//
		//【注意】	Windows 95/98 ではディレクトリを ::CreateFile できない

		const HANDLE handle =
			::CreateFile(path,
						 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
						 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL |
						 FILE_FLAG_BACKUP_SEMANTICS, 0);

		if (handle == INVALID_HANDLE_VALUE) {
			const DWORD osErrno = ::GetLastError();
			switch (osErrno) {
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
			case ERROR_BAD_NETPATH:
			case ERROR_BAD_NET_NAME:
			case ERROR_INVALID_NAME:

				// 存在確認したファイルは存在しない

				return false;
			}

			// システムコールのエラーを表す例外を投げる

			_TRMEISTER_OS_THROW(path, _Literal::CreateFile, osErrno);
		}

		// 生成されたハンドルは必要ないので破棄する

		(void) ::CloseHandle(handle);
	} else
#endif
	{
		// 指定された絶対パス名のファイルのファイル属性を求める

		const DWORD attribute = ::GetFileAttributes(path);

		if (attribute == 0xffffffff) {

			// システムコールのエラーを表す例外を投げる

			const DWORD osErrno = ::GetLastError();
#ifdef SYD_OS_WINNT4_0
#else
			if (mode == AccessMode::File)
				switch (osErrno) {
				case ERROR_FILE_NOT_FOUND:
				case ERROR_PATH_NOT_FOUND:
				case ERROR_BAD_NETPATH:
				case ERROR_BAD_NET_NAME:
				case ERROR_INVALID_NAME:

					// 存在確認したファイルは存在しない

					return false;
				}
#endif
			_TRMEISTER_OS_THROW(path, _Literal::GetFileAttributes, osErrno);

		} else if (attribute & FILE_ATTRIBUTE_READONLY &&
				   mode & AccessMode::Write)

			// 読み込み専用のファイルなので、書き込む権利はない

			return false;
	}
#endif
#ifdef SYD_OS_POSIX

	// 権利の有無を調べる

	if (::access(path, mode) == -1) {
		const int osErrno = errno;

		switch (osErrno) {
		case EACCES:
		case ENOENT:

			// 指定された操作の権利はない

			return false;
		}
		
		// システムコールのエラーを表す例外を投げる

		_TRMEISTER_OS_THROW(path, _Literal::access, osErrno);
	}
#endif
	return true;
}

//	FUNCTION public
//	Os::File::lock -- ファイルをロックする
//
//	NOTES
//		ある領域をロックすると、他のプロセスがその領域に対して
//		読み書きを行ったとき、ロック待ちする
//
//		ロックされたファイルは Os::File::unlock でアンロックできる
//
//		ロックしたファイルがクローズしたり、されると、ロックははずされる
//
//		同じプロセスがロックを重ねがけできない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::lock()
{
	//【注意】	同じプロセスによるロックの重ねがけに対応するには、
	//			ロック回数を管理するようにすればよい
	//			が、必要なく、無駄に遅くなるので実装しない

#ifdef SYD_OS_WINDOWS

	// ファイルの最大サイズを求める

	ModStructuredUInt64 size;
	size.full = getSizeMax();

#ifdef SYD_OS_WINNT4_0

	// ファイルの先頭からの非同期操作の実行状況を表すオブジェクトを作成する

	AsyncStatus async(*this, 0);
	OVERLAPPED* overlapped = &async._overlapped;

	// ファイル全体を排他ロックする

	if (!::LockFileEx(_handle, LOCKFILE_EXCLUSIVE_LOCK, 0,
					  size.halfs.low, size.halfs.high, overlapped)) {
		const DWORD osErrno = ::GetLastError();
		if (osErrno != ERROR_IO_PENDING)

			// システムコールのエラーを表す例外を投げる

			_TRMEISTER_OS_THROW(getPath(), _Literal::LockFileEx, osErrno);

		// ファイルをロックする前に呼び出しから戻ったので、
		// ロックが終了するまで待つ

		(void) async.wait();
	}

	// ロックできた

	_locked = true;
#else
	// ファイル全体を排他ロックする
	//
	//【注意】	Windows 95/98 では ::LockFileEx をサポートしていない
retry:
	_locked = ::LockFile(_handle, 0, 0, size.halfs.low, size.halfs.high);
	if (!_locked) {
		const DWORD osErrno = ::GetLastError();
		if (osErrno != ERROR_LOCK_VIOLATION)

			// システムコールのエラーを表す例外を投げる

			_TRMEISTER_OS_THROW(getPath(), _Literal::LockFile, osErrno);

		// 他プロセスがロックしているので、1 秒待って再実行する
		//
		//【注意】	::LockFile はロック待ちできない

		::Sleep(1000);
		goto retry;
	}
#endif
#endif
#ifdef SYD_OS_POSIX

	// ファイルの先頭へファイルポインタを移動する

	seek(0, SeekWhence::Set);

	// ファイル全体をロックする

	(void) _File::lock(getPath(), _descriptor, F_LOCK);
#endif
}

//	FUNCTION public
//	Os::File::trylock -- ファイルをロックしてみる
//
//	NOTES
//		ある領域をロックすると、他のプロセスがその領域に対して
//		読み書きを行ったとき、ロック待ちする
//
//		ロックされたファイルは Os::File::unlock でアンロックできる
//
//		ロックしたファイルがクローズしたり、されると、ロックははずされる
//
//		同じプロセスがロックを重ねがけできない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			ロックできた
//		false
//			ロックできなかった
//
//	EXCEPTIONS

bool
File::trylock()
{
#ifdef SYD_OS_WINDOWS

	// ファイルの最大サイズを求める

	ModStructuredUInt64 size;
	size.full = getSizeMax();

#ifdef SYD_OS_WINNT4_0

	// ファイル全体の排他ロックを試みる

	OVERLAPPED overlapped;
	overlapped.Offset = 0;
	overlapped.OffsetHigh = 0;
	overlapped.hEvent = 0;

	_locked = ::LockFileEx(
		_handle, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY,
		0, size.halfs.low, size.halfs.high, &overlapped);
	if (!_locked) {
		const DWORD osErrno = ::GetLastError();
		if (osErrno != ERROR_LOCK_VIOLATION)

			// システムコールのエラーを表す例外を投げる

			_TRMEISTER_OS_THROW(getPath(), _Literal::LockFileEx, osErrno);
	}
#else
	// ファイル全体の排他ロックを試みる
	//
	//【注意】	Windows 95/98 では ::LockFileEx をサポートしていない

	_locked = ::LockFile(_handle, 0, 0, size.halfs.low, size.halfs.high);
	if (!_locked) {
		const DWORD osErrno = ::GetLastError();
		if (osErrno != ERROR_LOCK_VIOLATION)

			// システムコールのエラーを表す例外を投げる

			_TRMEISTER_OS_THROW(getPath(), _Literal::LockFile, osErrno);
	}
#endif
	return _locked;
#endif
#ifdef SYD_OS_POSIX

	// ファイルの先頭へファイルポインタを移動する

	seek(0, SeekWhence::Set);

	// ファイル全体をロックしてみる

	return _File::lock(getPath(), _descriptor, F_TLOCK);
#endif
}

//	FUNCTION public
//	Os::File::unock -- ファイルをアンをロックする
//
//	NOTES
//		アンロックするファイルはロックされている必要がある
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::unlock()
{
#ifdef SYD_OS_WINDOWS

	// ファイルの最大サイズを求める

	ModStructuredUInt64 size;
	size.full = getSizeMax();

#ifdef SYD_OS_WINNT4_0

	// ファイル全体の排他ロックをはずす

	OVERLAPPED overlapped;
	overlapped.Offset = 0;
	overlapped.OffsetHigh = 0;
	overlapped.hEvent = 0;

	if (!::UnlockFileEx(
		_handle, 0, size.halfs.low, size.halfs.high, &overlapped)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_OS_THROW(getPath(), _Literal::UnlockFileEx, osErrno);
	}
#else
	// ファイル全体の排他ロックをはずす
	//
	//【注意】	Windows 95/98 では ::UnlockFileEx をサポートしていない

	if (!::UnlockFile(_handle, 0, 0, size.halfs.low, size.halfs.high)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_OS_THROW(getPath(), _Literal::UnlockFileEx, osErrno);
	}
#endif
	// ロックがはずれた

	_locked = false;
#endif
#ifdef SYD_OS_POSIX

	// ファイルの先頭へファイルポインタを移動する

	seek(0, SeekWhence::Set);

	// ファイル全体をロックしてみる

	(void) _File::lock(getPath(), _descriptor, F_ULOCK);
#endif
}

//	FUNCTION public
//	Os::File::setPath -- ファイルの絶対パス名を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			設定するファイルの絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::setPath(const Path& path)
{
	if (isOpened())

		// ファイルはすでにオープンされている

		_TRMEISTER_THROW1(Exception::FileAlreadyOpened, getPath());

	// 指定された絶対パス名を記憶する

	_path = path;
}

//	FUNCTION public
//	Os::File::getSize -- ファイルサイズを得る
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

File::Size
File::getSize() const
{
#ifdef SYD_OS_WINDOWS

	// ファイルサイズを求める

	ModStructuredUInt64	size;
	DWORD				high;

	size.halfs.low = ::GetFileSize(_handle, &high);

	if (size.halfs.low == 0xffffffff) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_OS_THROW(getPath(), _Literal::GetFileSize, osErrno);
	}

	size.halfs.high = high;

	return size.full;
#endif
#ifdef SYD_OS_POSIX

	// ファイルの詳細情報を求める

	struct stat buf;

	if (::fstat(_descriptor, &buf) == -1) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(getPath(), _Literal::fstat, osErrno);
	}

	// 求めた詳細情報中のファイルサイズを返す

	return buf.st_size;
#endif
}

//	FUNCTION public
//	Os::File::getSize -- 指定されたファイルのファイルサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			ファイルサイズを求めるファイルの絶対パス名
//
//	RETURN
//		得られたファイルサイズ(B 単位)
//
//	EXCEPTIONS
//		Exception::BadArgument
//			ファイルサイズを求めるファイルの絶対パス名が指定されていない

// static
File::Size
File::getSize(const Path& path)
{
#ifdef SYD_OS_WINDOWS

	// 指定された絶対パス名のファイルをオープンする

	File	file(path);
	file.open(OpenMode::Read);

	// オープンしたファイルのサイズを求める

	return file.getSize();
#endif
#ifdef SYD_OS_POSIX
	if (!path.getLength())

		// ファイルサイズを求めるファイルの絶対パス名が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

	// 指定された絶対パス名のファイルの詳細情報を求める

	struct stat buf;

	if (::stat(path, &buf) == -1) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(path, _Literal::stat, osErrno);
	}

	// 求めた詳細情報中のファイルサイズを返す

	return buf.st_size;
#endif
}

//	FUNCTION public
//	Os::File::getSizeMax -- システムが許可するファイルサイズの最大値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイルサイズの最大値(B 単位)
//
//	EXCEPTIONS

// static
File::Size
File::getSizeMax()
{
#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WINNT4_0

	// システムが許可するファイルサイズの上限を求める手段を
	// システムは提供しない
	//
	// NTFS での理論上の最大ファイルサイズは
	//
	// Limits<Size>::getMax() (現在、16 EXA バイト)
	//
	// である(WIndows NT Server 4.0 Resource Kit より)が、
	// インタフェース関数の入出力となるファイルオフセットは
	// 符号付整数である必要があるため、
	//
	// Limits<Offset>::getMax() (現在、8 EXA バイト)
	//
	// に制限する

	return Limits<Offset>::getMax();
#else
	// システムが許可するファイルサイズの上限を求める手段を
	// システムは提供しない
	//
	// FAT32 での最大ファイルサイズは 4 G バイト - 2 バイト である

	return Limits<ModUInt32>::getMax() - 2;
#endif
#endif
#ifdef SYD_OS_POSIX

	// システムが許可するファイルサイズの上限を求める
	//
	//【注意】	struct stat のメンバーである st_size の型は off_t なので、
	//			得られる値を負数にしたものをファイルオフセットにしても
	//			オーバーフローしないはず

	struct rlimit	buf;

	if (::getrlimit(RLIMIT_FSIZE, &buf) == -1) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_THROW2(Exception::SystemCall, _Literal::getrlimit, osErrno);
	}

	return buf.rlim_max;
#endif
}

//	FUNCTION public
//	Os::Directory::create -- ディレクトリを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			指定された絶対パス名のディレクトリを生成する
//		Os::Directory::Permission::Value	permission
//			指定されたとき
//				生成するディレクトリに対して許可される操作を表す値で、
//				Os::Directory::Permission::Value の論理和を指定する
//			指定されないとき
//				Permission::OwnerMask が指定されたものとみなす
//		bool				recursive
//			true
//				生成するディレクトリの親ディレクトリが存在しないとき、
//				まず、親ディレクトリを permission |
//				Os::Directory::Permission::OwnerWrite |
//				Os::Directory::Permission::OwnerExecute の許可モードで生成する
//			false または指定されないとき
//				生成するディレクトリの親ディレクトリが
//				存在しなければ、エラーになる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Directory::create(
	const Path& path, Permission::Value permission, bool recursive)
{
	if (!path.getLength())

		// 生成するディレクトリの絶対パス名が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

	if (recursive) {

		// 指定されたディレクトリの親ディレクトリが存在するか調べる

		Path	parent;
		if (path.getParent(parent) && !access(parent, AccessMode::File))

			// 親ディレクトリが存在しないので、生成する
			//
			//【注意】	生成する親ディレクトリの許可モードは、
			//			与えられた許可モードに所有者が書き込み、
			//			実行可を加えたものにする
			//
			//			そうしないと、
			//			その中にディレクトリを生成できない可能性がある

			create(parent, permission |
				   Permission::OwnerWrite | Permission::OwnerExecute, true);
	}
#ifdef SYD_OS_WINDOWS

	// 現在の UMASK を求めて、
	// 指定されたディレクトリの許可モードに適用する

	unsigned int	mask = Process::umask(0);
	(void) Process::umask(mask);

	permission &= Permission::Mask;
	permission &= ~mask;

	SECURITY_ATTRIBUTES	security;
	security.nLength = sizeof(security);
	security.bInheritHandle = false;
#ifdef SYD_OS_WINNT4_0

	// ::CreateDirectory によるディレクトリの生成時には
	// セキュリティは設定できるが、ファイル属性は設定できない
	//
	// ファイル属性の設定には FILE_WRITE_ATTRIBUTES の権利が必要なので、
	// 先に与えられた許可モードからセキュリティだけを設定してしまうと、
	// ファイル属性を設定できなくなる可能性がある
	//
	// そこで、まずファイル属性を設定可能な権利による
	// セキュリティを設定したディレクトリを生成してから、
	// 与えられた許可モードによりセキュリティとファイル属性を設定する

	PSID	sids[_AccessControl::_Sid::_Type::ValueNum];
	Memory::reset(sids, sizeof(sids));
	ACL*	dacl = 0;

	try {
		// 呼び出しプロセスの所有者、プライマリーグループと、
		// 全ユーザーの SID を得る

		_AccessControl::getCurrentUnixSids(sids);

		// 与えられたディレクトリの許可モードを
		// 求めた SID に関するアクセス制御リストへ変換する

		dacl = _AccessControl::setUnixAcesInAcl(0, permission, sids);
		; _TRMEISTER_ASSERT(dacl);

		// 生成するディレクトリーに設定する
		// 絶対形式のセキュリティー記述子を生成する

		SECURITY_DESCRIPTOR		sd;
		_AccessControl::initializeSecurityDescriptor(
			sd, sids[_AccessControl::_Sid::_Type::User],
			sids[_AccessControl::_Sid::_Type::Group], dacl);
		
		security.lpSecurityDescriptor = &sd;
#else
		security.lpSecurityDescriptor = 0;
#endif
		// 指定されたパス名のディレクトリを生成する

		if (!::CreateDirectory(path, &security)) {

			const DWORD osErrno = ::GetLastError();

			if (osErrno != ERROR_ALREADY_EXISTS)

				// 複数のスレッドが同じディレクトリ配下のファイルを
				// 同時に作成する場合があり、作成しようとしたディレクトリが
				// すでに存在している場合がある
				// それ以外の場合は、システムコールのエラーを表す例外を投げる
				
				_TRMEISTER_OS_THROW(path, _Literal::CreateDirectory, osErrno);
		}
#ifdef SYD_OS_WINNT4_0
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{

		::LocalFree(dacl);
		_AccessControl::freeUnixSids(sids);
		_TRMEISTER_RETHROW;
	}

	// 必要のなくなったアクセス制御リストと SID を破棄する

	::LocalFree(dacl);
	_AccessControl::freeUnixSids(sids);
#endif
	// 生成したディレクトリのファイル属性を許可モードに合わせて変更する
	//
	//【注意】	生成されたディレクトリに
	//			どのようなセキュリティが設定されていても
	//			所有者は必ずファイル属性を操作できるはず

	File::chmodAttribute(path, permission);
#endif
#ifdef SYD_OS_POSIX

	// 指定されたパス名のディレクトリを生成する

	if (::mkdir(path, permission & Permission::Mask) == -1) {

		const int osErrno = errno;
		
		if (osErrno != EEXIST)

			// 複数のスレッドが同じディレクトリ配下のファイルを
			// 同時に作成する場合があり、作成しようとしたディレクトリが
			// すでに存在している場合がある
			// それ以外の場合は、システムコールのエラーを表す例外を投げる

			_TRMEISTER_OS_THROW(path, _Literal::mkdir, osErrno);
	}
#endif
}

//	FUNCTION public
//	Os::Directory::remove -- ディレクトリを削除する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			指定された絶対パス名のディレクトリを削除する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Directory::remove(const Path& path)
{
	if (!path.getLength())

		// 削除するディレクトリの絶対パス名が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

#ifdef SYD_OS_WINDOWS
	// POSIX と仕様を合わせて、
	// ファイル属性が読み取り専用でも削除できるように
	// 削除するディレクトリのファイル属性を読み取り専用でなくする
	//
	//【注意】	ディレクトリの所有者であれば、ファイル属性を変更できるはず
	//
	//【注意】	削除するディレクトリのセキュリティーは変更しない

	File::chmodAttribute(path, Permission::MaskWrite);

	// ディレクトリを削除する

	if (!::RemoveDirectory(path)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_OS_THROW(path, _Literal::RemoveDirectory, osErrno);
	}
#endif
#ifdef SYD_OS_POSIX

	// ディレクトリを削除する

	if (::rmdir(path) == -1) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(path, _Literal::rmdir, osErrno);
	}
#endif
}

//	FUNCTION public
//	Os::Directory::flush -- ディレクトリの内容をフラッシュする
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			指定された絶対パス名のディレクトリのする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Directory::flush(const Path& path)
{
	if (!path.getLength())

		// 削除するディレクトリの絶対パス名が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

#ifdef SYD_OS_WINDOWS
	// NTFSは内容をフラッシュする必要はない？
#endif
#ifdef SYD_OS_POSIX

	// ディレクトリの内容をフラッシュする
	// フラッシュは open -> fdatasync -> close で行う

	// ディレクトリをオープンする
	
	int fd = ::open(path, O_RDONLY);
	if (fd == -1) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(path, _Literal::open, osErrno);
	}

	// 内容をフラッシュする
	
	if (::fdatasync(fd) == -1) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		// 例外を送出する前にクローズする
		::close(fd);
		_TRMEISTER_OS_THROW(path, _Literal::fdatasync, osErrno);
	}

	// クローズする

	if (::close(fd) == -1) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_OS_THROW(path, _Literal::close, osErrno);
	}
#endif
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2012, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
