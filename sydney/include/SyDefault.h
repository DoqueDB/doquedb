// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SyDefault.h -- 
// 
// Copyright (c) 1999, 2000, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

// !!!!! WARNING !!!!!!
// You must write only an ASCII character in this file.
// !!!!! WARNING !!!!!!

#ifndef __SY_DEFAULT_H
#define __SY_DEFAULT_H

#define SYD_ORDER_LH		0x00
#define SYD_ORDER_HL		0x20
#define SYD_FLOAT_IEEE_LH	0x00
#define	SYD_FLOAT_IEEE_HL	0x20

#ifdef SYD_CPU_SPARC
#define SYD_BYTEORDER		SYD_ORDER_HL
#define SYD_FLOATING		SYD_FLOAT_IEEE_HL
#endif
#ifdef SYD_CPU_INTEL
#define	SYD_BYTEORDER		SYD_ORDER_LH
#define	SYD_FLOATING		SYD_FLOAT_IEEE_LH
#endif

#ifdef SYD_OS_SOL10
#define SYD_OS_SOL9
#endif
#ifdef SYD_OS_SOL9
#define SYD_OS_SOL8
#endif
#ifdef SYD_OS_SOL8
#define SYD_OS_SOL7
#endif
#ifdef SYD_OS_SOL7
#define SYD_OS_SOL2_6
#endif
#ifdef SYD_OS_SOL2_6
#define SYD_OS_SOL2_5
#endif
#ifdef SYD_OS_SOL2_5
#define SYD_OS_SOLARIS
#endif

// Windows 7/Server2008 R2			v6.1
// Windows Vista/Server 2008		v6.0
// Windows Server 2003				v5.2
// Windows XP						v5.1
// Windows 2000						v5.0
// Windows NT4.0					v4.0
#ifdef SYD_OS_WINNT6_1
#define SYD_OS_WINNT6_0
#endif
#ifdef SYD_OS_WINNT6_0
#define SYD_OS_WINNT5_2
#endif
#ifdef SYD_OS_WINNT5_2
#define SYD_OS_WINNT5_1
#endif
#ifdef SYD_OS_WINNT5_1
#define SYD_OS_WINNT5_0
#endif
#ifdef SYD_OS_WINNT5_0
#define SYD_OS_WINNT4_0
#endif
#ifdef SYD_OS_WIN98
#define SYD_OS_WIN95
#endif
#if defined(SYD_OS_WINNT4_0) || defined(SYD_OS_WIN95)
#define SYD_OS_WINDOWS
#endif

#if defined(SYD_OS_LINUX) || defined(SYD_OS_SOLARIS)
#define SYD_OS_POSIX
#endif

#ifdef SYD_C_SC5_9
#define SYD_C_SC5_8
#endif
#ifdef SYD_C_SC5_8
#define SYD_C_SC5_5
#endif
#ifdef SYD_C_SC5_5
#define SYD_C_SC5_4
#endif
#ifdef SYD_C_SC5_4
#define SYD_C_SC5_3
#endif
#ifdef SYD_C_SC5_3
#define SYD_C_SC5_2
#endif
#ifdef SYD_C_SC5_2
#define SYD_C_SC5_1
#endif
#ifdef SYD_C_SC5_1
#define SYD_C_SC5_0
#endif
#ifdef SYD_C_SC5_0
#define SYD_C_SC4_2
#endif
#ifdef SYD_C_SC4_2
#define SYD_C_SC4_0
#endif

#ifdef SYD_C_GCC4_8
#define SYD_C_GCC4_4
#endif
#ifdef SYD_C_GCC4_4
#define SYD_C_GCC4_1
#endif
#ifdef SYD_C_GCC4_1
#define SYD_C_GCC3_4
#endif
#ifdef SYD_C_GCC3_4
#define SYD_C_GCC3_3
#endif
#ifdef SYD_C_GCC3_3
#define SYD_C_GCC3_2
#endif
#ifdef SYD_C_GCC3_2
#define SYD_C_GCC3_1
#endif
#ifdef SYD_C_GCC3_1
#define SYD_C_GCC3_0
#endif
#ifdef SYD_C_GCC3_0
#define SYD_C_GCC2_9
#endif
#ifdef SYD_C_GCC2_9
#define SYD_C_GCC2_8
#endif

#ifdef SYD_C_MS10_0
#define SYD_C_MS9_0
#endif
#ifdef SYD_C_MS9_0
#define SYD_C_MS8_0
#endif
#ifdef SYD_C_MS8_0
#define SYD_C_MS7_1
#endif
#ifdef SYD_C_MS7_1
#define SYD_C_MS7_0
#endif

#ifdef SYD_C_MS
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)
#pragma warning(disable: 4786)
#pragma warning(disable: 4800)
#ifdef SYD_C_MS8_0
#pragma warning(disable: 4996)
#endif
#endif

#ifdef SYD_C_MS6_0
#define SYD_VOID_NOT_RETURN
#endif
#ifdef SYD_C_MS7_0
#define SYD_CC_TYPENAME
#define	SYD_CC_FOR_INSIDE_SCOPE
#endif
#ifdef SYD_C_GCC2_8
#define SYD_CC_TYPENAME
#define	SYD_CC_FOR_INSIDE_SCOPE
#endif
#ifdef SYD_C_SC5_0
#define SYD_CC_TYPENAME
#define	SYD_CC_FOR_INSIDE_SCOPE
#define SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
#endif

#if defined(SYD_OS_LINUX) || defined(SYD_OS_SOL2_6)

// use 64 bit file interface

#ifdef _FILE_OFFSET_BITS
#undef _FILE_OFFSET_BITS
#endif
#define _FILE_OFFSET_BITS	64
#endif

#if defined(SYD_OS_SOL2_6)
#define SYD_SCHEMA_BTREE2
#define SYD_SCHEMA_VECTOR2
#endif

// TODO: remove old versioning V10-V15
#ifdef SYD_USE_UNA_1_0
#define SYD_USE_UNA_V15
#endif

#ifdef SYD_USE_UNA_V15
#define SYD_USE_UNA_V14
#endif

#ifdef SYD_USE_UNA_V14
#define SYD_USE_UNA_V13
#endif

#ifdef SYD_USE_UNA_V13
#define SYD_USE_UNA_V12
#endif

#ifdef SYD_USE_UNA_V12
#define SYD_USE_UNA_V11
#endif

#ifdef SYD_USE_UNA_V11
#define SYD_USE_UNA_V10
#endif

#include "SyInclude.h"

#endif

//
//	Copyright (c) 1999, 2000, 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
