// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4
//
// ModSerialSize.c -- シリアル化 Sizeを得る
// 
// Copyright (c) 1997, 2023 Ricoh Company, Ltd.
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

#include "ModSerialSize.h"

//
// FUNCTION public
//	ModPureSerialSize::ModPureSerialSize -- ModPureSerialSizeのコンストラクタ
//
// NOTES
//	ModPureSerialSizeをコンストラクトする。
//
// ARGUMENTS
//	なし
//
// RETURN
//  なし
//
// EXCEPTIONS
// 	なし
//
ModPureSerialSize::ModPureSerialSize()
{

}

//
// FUNCTION public
//	ModPureSerialSize::~ModPureSerialSize -- ModPureSerialSizeのディストラクタ
//
// NOTES
//	ModPureSerialSizeをディストラクトする。
//
// ARGUMENTS
//	なし
//
// RETURN
//  なし
//
// EXCEPTIONS
// 	なし
//
ModPureSerialSize::~ModPureSerialSize()
{

}

//
// FUNCTION public
//	ModPureSerialSize::readSerial -- アーカイブ読み込みサイズを得る
//
// NOTES
//	アーカイブの読み込みサイズを得る。
//
// ARGUMENTS
//	void* buffer_
//		読み込みバッファへのポインタ
//	ModSize byte_
//		読み込みバッファサイズ
//	ModSerialIO::DataType type_
// 		IOデータタイプ
//
// RETURN
//  読み込みサイズ
//
// EXCEPTIONS
// 	なし
//
int
ModPureSerialSize::readSerial(void* buffer_, ModSize byte_, ModSerialIO::DataType type_)
{
    return byte_;
}

//
// FUNCTION public
//	ModPureSerialSize::writeSerial -- アーカイブ書き込みサイズを得る
//
// NOTES
//	アーカイブの書き込みサイズを得る。
//
// ARGUMENTS
//	void* buffer_
//		読み込みバッファへのポインタ
//	ModSize byte_
//		読み込みバッファサイズ
//	ModSerialIO::DataType type_
// 		IOデータタイプ
//
// RETURN
//  書き込みサイズ
//
// EXCEPTIONS
// 	なし
//
int
ModPureSerialSize::writeSerial(const void* buffer_, ModSize byte_, ModSerialIO::DataType type_)
{
    return byte_;
}

//
// FUNCTION public
//	ModPureSerialSize::resetSerial -- アーカイブIOのリセット
//
// NOTES
//	本メソッドは何もしない。
//
// ARGUMENTS
//	なし
//
// RETURN
//  なし
//
// EXCEPTIONS
// 	なし
//
void 
ModPureSerialSize::resetSerial()
{
    // do nothig;
}

//
// Copyright (c) 1997, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

