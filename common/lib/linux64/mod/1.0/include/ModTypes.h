// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModTypes.h -- 型の定義
// 
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModTypes_H__
#define __ModTypes_H__

#include "ModConfig.h"

//	TYPEDEF
//	ModInt32 -- 32 ビット長整数を表す型
//
//	NOTES

typedef	int					ModInt32;

//	TYPEDEF
//	ModUInt32 -- 32 ビット長非負整数を表す型
//
//	NOTES

typedef unsigned int		ModUInt32;

//	TYPEDEF
//	ModInt64 -- 64 ビット長整数を表す型
//
//	NOTES

#if defined(OS_RHLINUX6_0)
typedef long long			ModInt64;
#endif

//	TYPEDEF
//	ModUInt64 -- 64 ビット長非負整数を表す型
//
//	NOTES

#if defined(OS_RHLINUX6_0)
typedef unsigned long long	ModUInt64;
#endif

//	TYPEDEF
//	ModStructuredInt64 --
//		ModInt64 の上位、下位 32 ビットにアクセスしやすくするための共用体
//
//	NOTES

typedef union
{
	ModInt64	full;
	struct
	{
#if MOD_CONF_BYTEORDER == 0
		ModInt32	high;
		ModUInt32	low;
#endif
#if MOD_CONF_BYTEORDER == 1
		ModUInt32	low;
		ModInt32	high;
#endif
	} halfs;
} ModStructuredInt64;

//	TYPEDEF
//	ModStructuredUInt64 --
//		ModUInt64 の上位、下位 32 ビットにアクセスしやすくするための共用体
//
//	NOTES

typedef union
{
	ModUInt64	full;
	struct
	{
#if MOD_CONF_BYTEORDER == 0
		ModUInt32	high;
		ModUInt32	low;
#endif
#if MOD_CONF_BYTEORDER == 1
		ModUInt32	low;
		ModUInt32	high;
#endif
	} halfs;
} ModStructuredUInt64;

//	TYPEDEF
//	ModPtr, ModUPtr --
//		任意のデータ型を指すポインターを格納可能な整数または非負整数
//
//	NOTES
//		intptr_t および uintptr_t と同じ定義とするが、
//		これらの型が存在しない環境もあるので、独自に定義する

#if defined(OS_RHLINUX6_0)
#if defined(__LP64__) || defined(_LP64)
typedef	long				ModPtr;
typedef	unsigned long		ModUPtr;
#else
typedef	int					ModPtr;
typedef	unsigned int		ModUPtr;
#endif
#endif

//
// TYPEDEF
//	ModSize -- 32ビットのサイズを表す型
// NOTES
// 	非負の値を表す32ビットの型。主にメモリ上のサイズを表すのに用いる。
//	MODでのサイズ型は32ビットをデフォルトとする。
//	64ビットの方がコーディングが容易かもしれないが、まだ64bitをデフォルトと
//	するには性能の面でリスクが高いと予想されるため、ModFileSizeと使い分ける。
typedef ModUInt32	ModSize;

//
// TYPEDEF
//	ModFileSize -- 64ビットのサイズを表す型
// NOTES
// 	非負の値を表す64ビットの型。主にファイルのサイズを表すのに用いる。
typedef ModUInt64	ModFileSize;

//
// TYPEDEF
//	ModOffset -- 32ビットのオフセットを表す型
// NOTES
// 	32ビットの整数型。主にメモリ上のオフセット値を表すのに用いる。
//	MODでのサイズ型は32ビットをデフォルトとする。
//	64ビットの方がコーディングが容易かもしれないが、まだ64bitをデフォルトと
//	するには性能の面でリスクが高いと予想されるため、ModFileOffsetと
//	使い分ける。
typedef ModInt32	ModOffset;

//
// TYPEDEF
//	ModFileOffset -- 64ビットのオフセットを表す型
// NOTES
// 	64ビットの整数型。主にファイル上のオフセット値を表すのに用いる。
// 	サイズが2G以上のファイルでも使用できる。
//	本当は、OS非依存に定義したい (__int64 は使わないようにしたい)
typedef ModInt64	ModFileOffset;

//
// VARIABLE
//	ModUndefinedOffset -- 無効な ModOffset の値
// NOTES
//	無効な32ビットオフセット型の値を示す。
//
const ModOffset ModUndefinedOffset = (ModOffset)(-1);

//
// VARIABLE
//	ModUndefinedFileOffset -- 無効な ModFileOffset の値
// NOTES
//	無効な64ビットオフセット型の値を示す。
//
const ModFileOffset ModUndefinedFileOffset = (ModFileOffset)(-1);

//	TYPEDEF
//	ModProcessId -- プロセス ID 型
//
//	NOTES
// 		プロセス ID を表す

#if defined(OS_RHLINUX6_0)
// pid_t
typedef int					ModProcessId;
#endif

//	TYPEDEF
//	ModThreadId -- スレッド ID 型
//
//	NOTES
// 		スレッド ID を表す

#if defined(OS_RHLINUX6_0)
#ifdef MOD_NO_THREAD
typedef ModProcessId		ModThreadId;
#else
// pthread_t
typedef	unsigned long		ModThreadId;
#endif
#endif

// 以下、将来廃止します
//	CONST
//	ModUndefinedProcessId -- 無効なプロセス ID
//
//	NOTES
//		無効なプロセス ID を表す

const ModProcessId	ModUndefinedProcessId = (ModProcessId) -1;

//	CONST
//	ModUndefinedThreadId -- 無効なスレッド ID
//
//	NOTES
//		無効なスレッド ID を表す

#if defined(OS_RHLINUX6_0)
#ifdef MOD_NO_THREAD
const ModThreadId	ModUndefinedThreadId = ModUndefinedProcessId;
#else
const ModThreadId	ModUndefinedThreadId = (ModThreadId) 0xfffffff;
#endif
#endif
// 以上、将来廃止します

//
// VARIABLE
//	ModUInt32, ModInt32, ModUInt64, ModInt64 -- 32ビット非負整数、32ビット整数、64ビット非負整数、64ビット整数の最大値
// NOTES
//	limits.hからの抜粋で定義している。
//
#if defined(OS_RHLINUX6_0)
const ModUInt32	ModUInt32Max	= 4294967295UL;
const ModInt32	ModInt32Max		= 2147483647L;
const ModUInt64	ModUInt64Max	= 18446744073709551615ULL;
const ModInt64	ModInt64Max		= 9223372036854775807LL;

const ModInt32	ModInt32Min		= (-2147483647L-1L);
#endif

//
// VARIABLE
//	ModUnsignedLongMax -- unsigned long の最大値
// NOTES
//	unsigned longの最大値を示す。
//	limits.hから抜粋している値を元に定義しているので、
//	OSによりunsigned longのサイズが変わったときには定義の変更が必要。
//
#if defined(__LP64__) || defined(_LP64)
const unsigned long ModUnsignedLongMax = ModUInt64Max;
#else
const unsigned long ModUnsignedLongMax = ModUInt32Max;
#endif

//
// VARIABLE
//	ModSizeMax -- ModSizeの最大値
// NOTES
//	ModSizeの最大値を示す。
//
const ModSize ModSizeMax = ModUInt32Max;

//
// VARIABLE
//	ModOffsetMax -- ModOffsetの最大値
// NOTES
//	ModOffsetの最大値を示す。
//
const ModOffset ModOffsetMax = ModInt32Max;
//
// VARIABLE
//	ModOffsetMin -- ModOffsetの最小値
// NOTES
//	ModOffsetの最小値を示す。
//	(ModSizeの最小値は0なので定義しない)
const ModOffset ModOffsetMin = ModInt32Min;

//
// VARIABLE
//	ModFileSizeMax -- ModFileSizeの最大値
// NOTES
//	ModFileSizeの最大値を示す。
//
const ModFileSize ModFileSizeMax = ModUInt64Max;

//
// VARIABLE
//	ModFileOffsetMax -- ModFileOffsetの最大値
// NOTES
//	ModFileOffsetの最大値を示す。
//
const ModFileOffset ModFileOffsetMax = ModInt64Max;

//
// CONST
//  ModPathMax -- パス名の最大値
// NOTES
//  パス名の最大値を示す。
//
#if defined(OS_RHLINUX6_0)
const ModSize ModPathMax = 1024;
#endif

//
// ENUM
//	ModStatus -- 状態を示す型の定義
// NOTES
//	正常か、異常かの状態を示す型。	
//
enum ModStatus {
	ModOk		= 0,
	ModError	= -1
};

//
// ENUM
//	ModBoolean -- ブール型の定義
// NOTES
//	真か、偽かの値を示す型。	
//
enum ModBoolean {
	ModTrue = 1,
	ModFalse = 0
};

#endif	// __ModTypes_H__

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
