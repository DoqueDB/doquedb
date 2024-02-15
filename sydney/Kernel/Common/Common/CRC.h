// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CRC.h --	CRC (Cyclic Redundancy Check) 関連の
//			テンプレートクラス定義、関数宣言
// 
// Copyright (c) 2001, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__TRMEISTER_COMMON_CRC_H
#define	__TRMEISTER_COMMON_CRC_H

#include "Common/Module.h"
#include "Common/Object.h"

#include "Os/Memory.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	NAMESPACE
//	Common::CRC32 -- 32 ビット CRC を扱うための名前空間
//
//	NOTES
//		RFC1662 の 32 ビット FCS を元にしている

namespace CRC32
{
	//	TYPEDEF
	//	Common::CRC32::Value --	32 ビット CRC の型
	//
	//	NOTES

	typedef	unsigned int	Value;

	// ある領域から 32 ビット CRC を生成する
	SYD_COMMON_FUNCTION
	Value					generate(const void* buf, Os::Memory::Size n);

	// ある領域を検証する
	SYD_COMMON_FUNCTION
	bool					verify(const void* buf, Os::Memory::Size n);
}

//	NAMESPACE
//	Common::CRC16 -- 16 ビット CRC を扱うための名前空間
//
//	NOTES
//		RFC1662 の 16 ビット FCS を元にしている

namespace CRC16
{
	//	TYPEDEF
	//	Common::CRC16::Value --	16 ビット CRC の型
	//
	//	NOTES

	typedef	unsigned short	Value;

	// ある領域から 16 ビット CRC を生成する
	SYD_COMMON_FUNCTION
	Value					generate(const void* buf, Os::Memory::Size n);

	// ある領域を検証する
	SYD_COMMON_FUNCTION
	bool					verify(const void* buf, Os::Memory::Size n);
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif	// __TRMEISTER_COMMON_CRC_H

//
// Copyright (c) 2001, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
