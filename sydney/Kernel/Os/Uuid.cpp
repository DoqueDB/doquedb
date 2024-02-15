// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Uuid.cpp -- UUIDを表すクラス
// 
// Copyright (c) 2017, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Os";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Os/Uuid.h"
#include "Os/Memory.h"
#include "Os/Unicode.h"

#include "Exception/SystemCall.h"

#include "ModCharTrait.h"

_TRMEISTER_USING
_TRMEISTER_OS_USING

namespace
{
}

//
//	FUNCTION public
//	Os::Uuid::Uuid -- コンストラクタ
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
Uuid::Uuid()
{
#ifdef SYD_OS_WINDOWS

	// 初期化する
	Os::Memory::reset(&m_cUUID, sizeof(m_cUUID));

#endif
#ifdef SYD_OS_POSIX

	// 初期化する
	uuid_clear(m_cUUID);

#endif
}

//
//	FUNCTION public
//	Os::Uuid::~Uuid -- デストラクタ
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
Uuid::~Uuid()
{
}

//
//	FUNCTION public
//	Os::Uuid::generate -- UUIDを生成し自身に設定する
//
//	NOTES
//
//	ARGUNMENTS
//	なし
//
//	RERURN
//	なし
//
//	EXCEPTIONS
//
void
Uuid::generate()
{
#ifdef SYD_OS_WINDOWS

	if (UuidCreate(&m_cUUID) != RPC_S_OK)
	{
		const DWORD e = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall,
						  Os::UnicodeString("UuidCreate"), e);
	}

#endif
#ifdef SYD_OS_POSIX

	uuid_generate(m_cUUID);
	// man を見る限り、エラーは発生しないようだ

#endif
}

//
//	FUNCTION public
//	Os::Uuid::compare -- 比較する
//
//	NOTES
//
//	ARGUMENTS
//	const Os::Uuid& cOther_
//		比較対象のUUID
//
//	RETURN
//	int
//		大小関係を判定した結果を返す
//			0	等しい
//			-1	自身 < cOther_
//			1	自身 > cOther_
//
//	EXCEPTIONS
//
int
Uuid::compare(const Uuid& cOther_)
{
	int ret = 0;
	
#ifdef SYD_OS_WINDOWS

	RPC_STATUS status;
	ret = ::UuidCompare(&m_cUUID,
						const_cast<UUID*>(&(cOther_.m_cUUID)), &status);
	// statusがエラーを返すことはないので、確認しない

#endif
#ifdef SYD_OS_POSIX

	ret = ::uuid_compare(m_cUUID, cOther_.m_cUUID);

#endif
	
	return ret;
}

//
//	FUNCTION public
//	Os::Uuid::parse -- 文字列からUUIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const char* pBuffer_
///		xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx 形式のUUID文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Uuid::parse(const char* pBuffer_)
{
	// xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx 形式のUUID文字列をパースし、
	// メンバー変数 m_cUUID に設定する
	
#ifdef SYD_OS_WINDOWS
	
	// UuidFronStringはUNICODE版とchar版があるが、
	// TRMeisterでは常にcharなので、char版を直に指定する

	if (UuidFromStringA(reinterpret_cast<unsigned char*>(
							const_cast<char*>(pBuffer_)),
					   &m_cUUID) != RPC_S_OK)
	{
		const DWORD e = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall,
						  Os::UnicodeString("UuidFromString"), e);
	}

#endif
#ifdef SYD_OS_POSIX

	if (uuid_parse(pBuffer_, m_cUUID) != 0)
	{
		_TRMEISTER_THROW2(Exception::SystemCall,
						  Os::UnicodeString("uuid_parse"), errno);
	}

#endif
}

//
//	FUNCTION public
//	Os::Uuid::unparse -- UUIDを文字列で取り出す
//
//	NOTES
//
//	ARGUMENTS
//	char* pBuffer_
///		xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx 形式のUUID文字列を設定する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Uuid::unparse(char* pBuffer_)
{
	// xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx 形式のUUID文字列を
	// 引数 pBuffer_ に設定する。(全部小文字)
	// pBuffer_ のメモリ領域(37バイト)は呼び出し側で確保する必要がある
	
#ifdef SYD_OS_WINDOWS

	// UuidToStringはUNICODE版とchar版があるが、
	// TRMeisterでは常にcharなので、char版を直に指定する
	//
	// 関数内部で格納領域が確保されるので、開放する必要がある
	
	unsigned char* tmp = 0;
	if (UuidToStringA(&m_cUUID, &tmp) != RPC_S_OK)
	{
		const DWORD e = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall,
						  Os::UnicodeString("UuidToString"), e);
	}
	
	// 関数リファレンスに大文字なのか小文字なのか記述がない
	// なので、小文字にしながらコピーする
	
	unsigned char* s = tmp;
	unsigned char* d = reinterpret_cast<unsigned char*>(pBuffer_);
	while (*s)
	{
		*d = ModCharTrait::toLower(*s);
		
		++s;
		++d;
	}
	*d = 0;

	// UuidToStringで確保された領域を解放する

	RpcStringFreeA(&tmp);

#endif
#ifdef SYD_OS_POSIX

	// すべて小文字で生成する
	
	uuid_unparse_lower(m_cUUID, pBuffer_);

#endif
}

//
// Copyright (c) 2017, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
