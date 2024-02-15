// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Limits.h -- 上限、下限を定義するクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2004, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_OS_LIMITS_H
#define	__TRMEISTER_OS_LIMITS_H

// #include <limits.h>
// #include <float.h>

#include "Os/Module.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	TEMPLATE CLASS
//	Os::Limits -- 上限、下限値を定義するテンプレートクラス
//
//	TEMPLATE ARGUMENTS
//		class _Type		対象の型
//
//	NOTES
//		STL の numeric_limits の真似だが、
//		必要最小限のメソッドしか定義していない

template <class _Type>
class Limits
{
public:
	enum { IsSpecialized = false };

	static _Type			getMin() { return _Type(0); }
	static _Type			getMax() { return _Type(0); }
	static _Type			getEpsilon() { return _Type(0); }
	static int				getDig() { return _Type(0); }
};

//	TEMPLATE CLASS
//	Os::Limits<> -- Limits の特別バージョン
//
//	NOTES
//		定数は <limits.h>, <float.h> で定義されている

template <>
class Limits<short>
{
public:
	enum { IsSpecialized = true };

	static short			getMin() { return SHRT_MIN; }
	static short			getMax() { return SHRT_MAX; }
	static short			getEpsilon() { return 0; }
	static int				getDig() {
		return int(((sizeof(short) << 3) - 1) * 301L / 1000);
	}
};

template <>
class Limits<unsigned short>
{
public:
	enum { IsSpecialized = true };

	static unsigned short	getMin() { return 0; }
	static unsigned short	getMax() { return USHRT_MAX; }
	static unsigned short	getEpsilon() { return 0; }
	static int				getDig() {
		return int((sizeof(unsigned short) << 3) * 301L / 1000);
	}
};

template <>
class Limits<int>
{
public:
	enum { IsSpecialized = true };

	static int				getMin() { return INT_MIN; }
	static int				getMax() { return INT_MAX; }
	static int				getEpsilon() { return 0; }
	static int				getDig() {
		return int(((sizeof(int) << 3) - 1) * 301L / 1000);
	}
};

template <>
class Limits<unsigned int>
{
public:
	enum { IsSpecialized = true };

	static unsigned int		getMin() { return 0; }
	static unsigned int		getMax() { return UINT_MAX; }
	static unsigned int		getEpsilon() { return 0; }
	static int				getDig() {
		return int((sizeof(unsigned int) << 3) * 301L / 1000);
	}
};

template <>
class Limits<long>
{
public:
	enum { IsSpecialized = true };

	static long				getMin() { return LONG_MIN; }
	static long				getMax() { return LONG_MAX; }
	static long				getEpsilon() { return 0; }
	static int				getDig() {
		return int(((sizeof(long) << 3) - 1)* 301L / 1000);
	}
};

template <>
class Limits<unsigned long>
{
public:
	enum { IsSpecialized = true };

	static unsigned long	getMin() { return 0; }
	static unsigned long	getMax() { return ULONG_MAX; }
	static unsigned long	getEpsilon() { return 0; }
	static int
	getDig() {
		return int((sizeof(unsigned long) << 3) * 301L / 1000);
	}
};

#ifdef SYD_OS_WINDOWS
template <>
class Limits<__int64>
{
public:
	enum { IsSpecialized = true };

	static __int64			getMin() { return _I64_MIN; }
	static __int64			getMax() { return _I64_MAX; }
	static __int64			getEpsilon() { return 0; }
	static int				getDig() {
		return int(((sizeof(__int64) << 3) - 1) * 301L / 1000);
	}
};

template <>
class Limits<unsigned __int64>
{
public:
	enum { IsSpecialized = true };

	static unsigned __int64	getMin() { return 0; }
	static unsigned __int64	getMax() { return _UI64_MAX; }
	static unsigned __int64	getEpsilon() { return 0; }
	static int				getDig() {
		return int((sizeof(unsigned __int64) << 3) * 301L / 1000);
	}
};
#endif
#ifdef SYD_OS_POSIX
#ifdef SYD_C_GCC2_8
#ifndef LLONG_MIN
#define LLONG_MIN	LONG_LONG_MIN
#endif
#ifndef LLONG_MAX
#define LLONG_MAX	LONG_LONG_MAX
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX	ULONG_LONG_MAX
#endif
#endif

template <>
class Limits<long long>
{
public:
	enum { IsSpecialized = true };

	static long long		getMin() { return LLONG_MIN; }
	static long long		getMax() { return LLONG_MAX; }
	static long long		getEpsilon() { return 0; }
	static int				getDig() {
		return int(((sizeof(long long) << 3) - 1) * 301L / 1000);
	}
};

template <>
class Limits<unsigned long long>
{
public:
	enum { IsSpecialized = true };

	static unsigned long long	getMin() { return 0; }
	static unsigned long long	getMax() { return ULLONG_MAX; }
	static unsigned long long	getEpsilon() { return 0; }
	static int					getDig() {
		return int((sizeof(unsigned long long) << 3) * 301L / 1000);
	}
};
#endif

template <>
class Limits<float>
{
public:
	enum { IsSpecialized = true };

	static float			getMin() { return FLT_MIN; }
	static float			getMax() { return FLT_MAX; }
	static float			getEpsilon() { return FLT_EPSILON; }
	static int				getDig() { return FLT_DIG; }
};

template <>
class Limits<double>
{
public:
	enum { IsSpecialized = true };

	static double			getMin() { return DBL_MIN; }
	static double			getMax() { return DBL_MAX; }
	static double			getEpsilon() { return DBL_EPSILON; }
	static int				getDig() { return DBL_DIG;}
};

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_LIMITS_H

//
// Copyright (c) 2001, 2002, 2004, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
