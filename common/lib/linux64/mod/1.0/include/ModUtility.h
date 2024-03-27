// -*-Mode: C; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModUtility.h -- 便利な関数
// 
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModUtility_H__
#define __ModUtility_H__

#include "ModCommon.h"
#include "ModAlgorithm.h"

//【注意】	private でないメソッドがすべて inline なので dllexport しない

class ModUtility
{
public:
	static ModSize byteToKbyte(ModSize byte);
	static ModOffset byteToKbyte(ModOffset byte);
	static ModFileSize byteToKbyte(ModFileSize byte);
	static ModFileOffset byteToKbyte(ModFileOffset byte);

	static ModSize kbyteToByte(ModSize byte);
	static ModOffset kbyteToByte( ModOffset byte);
	static ModFileSize kbyteToByte(ModFileSize byte);
	static ModFileOffset kbyteToByte( ModFileOffset byte);
};

//
// FUNCTION
// ModUtility::byteToKbyte -- バイトからキロバイトへの変換
//
// NOTES
// 	この関数はバイト単位の値をキロバイト単位に変換するために用いる。
//	1024で割った値が返される。
//
// ARGUMENTS
// const ModSize byte, const ModOffset byte, const ModFileSize byte, const ModFileOffset byte
//		バイト単位の値
//
// RETURN
// byte をキロバイト単位の値にした値
//
// EXCEPTIONS
// なし
//
inline
ModSize
ModUtility::byteToKbyte(ModSize byte)
{
	return ModByteToKbyte(byte);
}

inline
ModOffset
ModUtility::byteToKbyte(ModOffset byte)
{
	return ModByteToKbyte(byte);
}

inline
ModFileSize
ModUtility::byteToKbyte(ModFileSize byte)
{
	return ModByteToKbyte(byte);
}

inline
ModFileOffset
ModUtility::byteToKbyte(ModFileOffset byte)
{
	return ModByteToKbyte(byte);
}

//
// FUNCTION
// ModUtility::kbyteTobyte -- キロバイトからバイトへの変換
//
// NOTES
// 	この関数はキロバイト単位の値をバイト単位に変換するために用いる。
//	1024をかけた値が返される。値がオーバーフローして型の範囲を超える値に
//	なる場合には例外を送出する。
//
// ARGUMENTS
// const ModSize kbyte, const ModOffset kbyte, const ModFileSize kbyte, const ModFileOffset kbyte
//		キロバイト単位の値
//
// RETURN
// kbyte をバイト単位の値にした値
//
// EXCEPTIONS
// ModCommonErrorOutOfRange
//	範囲を超えている
//
inline
ModSize
ModUtility::kbyteToByte(ModSize kbyte)
{
 	if (kbyte > (ModSizeMax >> 10)) {
		ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
				 ModErrorLevelError);
	}
	return ModKbyteToByte(kbyte);
}

inline
ModOffset
ModUtility::kbyteToByte(ModOffset kbyte)
{
 	if (kbyte > (ModOffsetMax >> 10)) {
		ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
				 ModErrorLevelError);
	}
	return ModKbyteToByte(kbyte);
}

inline
ModFileSize
ModUtility::kbyteToByte(ModFileSize kbyte)
{
 	if (kbyte > (ModFileSizeMax >> 10)) {
		ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
				 ModErrorLevelError);
	}
	return ModKbyteToByte(kbyte);
}

inline
ModFileOffset
ModUtility::kbyteToByte(ModFileOffset kbyte)
{
 	if (kbyte > (ModFileOffsetMax >> 10)) {
		ModThrow(ModModuleStandard, ModCommonErrorOutOfRange,
				 ModErrorLevelError);
	}
	return ModKbyteToByte(kbyte);
}

#endif	// __ModUtility_H__

//
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
