// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModCompress.cpp -- ModCompress のメンバ定義
// 
// Copyright (c) 1998, 2011, 2023 Ricoh Company, Ltd.
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

#include "ModCommon.h"
#include "ModCompress.h"
#include "zlib.h"

//
// FUNCTION
// ModCompress::compress -- 圧縮を行なうスタティック関数
//
// NOTES
// この関数はバイト列を圧縮するために用いる。
//
// ARGUMENTS
// char* destination
//		圧縮したバイト列を格納するアドレス、領域は呼出側が確保する
// ModSize& destinationLength
//		destinationに確保されているバイト数。
//		実行後は格納されたバイト数が入る。
// const char* source
//		圧縮元のバイト列
// const ModSize sourceLength
//		sourceのバイト数
//
// RETURN VALUE
// なし
//
// EXCEPTIONS
// ModCommonErrorMemoryExhaust	zlibがZ_MEM_ERROR(メモリ不足)を返した
// ModCommonErrorBadArgument	zlibがZ_BUF_ERROR(destinationLengthの不足)を返した
// ModCommonErrorUnexpected		zlibが上記以外のエラーを返した
//
void
ModCompress::compress(char* destination, ModSize& destinationLength,
					  const char* source, const ModSize sourceLength)
{
	int error;
	uLongf destLen = destinationLength;
	error = ::compress((Byte*)destination, &destLen,
					   (const Bytef*)source, (uLong)sourceLength);
	switch(error) {
	case Z_OK:
		break;
	case Z_MEM_ERROR:
		ModErrorMessage << "compress failed: not enough memory.";
		ModErrorMessage << ModEndl;
		ModThrow(ModModuleStandard, ModCommonErrorMemoryExhaust,
				 ModErrorLevelError);
		break;
	case Z_BUF_ERROR:
		ModErrorMessage << "compress failed: not enough space in out buffer.";
		ModErrorMessage << ModEndl;
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument,
				 ModErrorLevelError);
		break;
	default:
		ModErrorMessage << "compress failed because of reason #";
		ModErrorMessage << error;
		ModErrorMessage << ModEndl;
		ModThrow(ModModuleStandard, ModCommonErrorUnexpected,
				 ModErrorLevelError);
		break;
	}
	destinationLength = (ModSize)destLen;
}

//
// FUNCTION
// ModCompress::uncompress -- 解凍を行なうスタティック関数
//
// NOTES
// この関数はバイト列を解凍するために用いる。
//
// ARGUMENTS
// char* destination
//		解凍したバイト列を格納するアドレス、領域は呼出側が確保する
// ModSize& destinationLength
//		destinationに確保されているバイト数。
//		実行後は格納されたバイト数が入る。
// const char* source
//		解凍元のバイト列
// const ModSize sourceLength
//		sourceのバイト数
//
// RETURN VALUE
// なし
//
// EXCEPTIONS
// ModCommonErrorMemoryExhaust	zlibがZ_MEM_ERROR(メモリ不足)を返した
// ModCommonErrorBadArgument	zlibがZ_BUF_ERROR(destinationLengthの不足)かZ_DATA_ERROR(入力データがcompressedでない)を返した
// ModCommonErrorUnexpected		zlibが上記以外のエラーを返した
//
void
ModCompress::uncompress(char* destination, ModSize& destinationLength,
						const char* source, const ModSize sourceLength)
{
	int error;
	uLongf destLen = destinationLength;
	error = ::uncompress((Byte*)destination, &destLen,
						 (const Bytef*)source, (uLong)sourceLength);
	switch(error) {
	case Z_OK:
		break;
	case Z_MEM_ERROR:
		ModErrorMessage << "uncompress failed: not enough memory.";
		ModErrorMessage << ModEndl;
		ModThrow(ModModuleStandard, ModCommonErrorMemoryExhaust,
				 ModErrorLevelError);
		break;
	case Z_BUF_ERROR:
		ModErrorMessage << "uncompress failed: not enough space in out buffer.";
		ModErrorMessage << ModEndl;
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument,
				 ModErrorLevelError);
		break;
	case Z_DATA_ERROR:
		ModErrorMessage << "uncompress failed: input data corrupted.";
		ModErrorMessage << ModEndl;
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument,
				 ModErrorLevelError);
		break;
	default:
		ModErrorMessage << "compress failed because of reason #";
		ModErrorMessage << error;
		ModErrorMessage << ModEndl;
		ModThrow(ModModuleStandard, ModCommonErrorUnexpected,
				 ModErrorLevelError);
		break;
	}
	destinationLength = (ModSize)destLen;
}


//
// Copyright (c) 1998, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
