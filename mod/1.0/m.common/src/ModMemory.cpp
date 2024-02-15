// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4
//
// ModMemory.cpp -- シリアル化可能メモリのメソッド定義
// 
// Copyright (c) 1997, 1999, 2023 Ricoh Company, Ltd.
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


#include "ModMemory.h"

//	FUNCTION public
//	ModPureMemory::renewalMemory -- 管理するメモリーを交換する
//
//	NOTES
//		内部で管理するメモリーと与えられたメモリーを交換し、
//		内部で管理するメモリーとそのサイズを返す
//
//	ARGUMENTS
//		void*				p
//			新しく管理するメモリーの先頭アドレス
//		ModSize				size
//			新しく管理するメモリーのサイズ(B 単位)
//		ModSize*			oldSize
//			これまで管理していたメモリーのサイズを返す
//			ModSize へのポインター
//
//	RETURN
//		これまで管理していたメモリーの先頭アドレス
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			これまで管理していたメモリーのサイズを返す
//			ModSize へのポインターが 0 である	

void*
ModPureMemory::renewalMemory(void* p, ModSize size, ModSize* oldSize)
{
	if (oldSize == 0)
		ModThrow(ModModuleOs, ModCommonErrorBadArgument, ModErrorLevelError);

	void*	old = this->headAddress;
	*oldSize = this->totalSize;

	this->renewalMemory(p, size);

	return old;
}

// FUNCTION public
// ModPureMemory::readSerial -- メモリからデータを読み出す
//
// NOTES
//	メモリからデータを読み出し、読み出しに成功したサイズを返す。
//	最後の引数はSerialIOのI/Fを引き継ぐために用意されている。
//	モードがhouseAbandonの場合、残りサイズの不足がわかった時点で、
//	何も読み込まずに0を返す。
//	この場合はオーバーフロー関数は呼び出さず、例外も送出しない。
//
// ARGUMENTS
//	void* buffer
//		読みだした結果を格納するバッファ
//	ModSize size
//		読み出すサイズ
//	ModSerialIO::DataType dummy
//		使用しないダミー引数。
//
// RETURN
// 読みだしたサイズ
//
// EXCEPTIONS
//	その他
//	サブクラスのreadOverFlowの例外か、ModPureMemory::readOverFlowの例外参照

int
ModPureMemory::readSerial(void* buf, ModSize size, ModSerialIO::DataType dummy)
{
	ModSize	rest = this->totalSize - this->currentPosition;
	if (size <= rest) {

		// メモリー中には指定された
		// サイズより多くのデータが格納されている

		ModOsDriver::Memory::copy(buf, this->currentAddress, size);
		this->currentPosition += size;
		this->currentAddress = (char*) this->currentAddress + size;
	} else {

		// メモリー中のデータでは指定されたサイズに足りない

		if (this->houseMode == houseAbandon)
			return 0;

		// まず、メモリー中のデータを読み出す

		ModOsDriver::Memory::copy(buf, this->currentAddress, rest);
		this->currentPosition += rest;
		this->currentAddress = (char*) this->currentAddress + rest;

		// 残りのぶんを登録されている
		// オーバーフロー関数を呼び出して、読み出す

		this->readOverFlow((char*) buf + rest, size - rest);
	}

	return (int) size;
}

// FUNCTION public
// ModPureMemory::writeSerial -- メモリにデータを格納する
//
// NOTES
//	メモリにデータを格納し、書き込みに成功したサイズを返す。
//	最後の引数はSerialIOのI/Fを引き継ぐために用意されている。
//	モードがhouseAbandonの場合、残りサイズの不足がわかった時点で、
//	何も書き込まずに0を返す。
//	この場合はオーバーフロー関数は呼び出さず、例外も送出しない。
//
// ARGUMENTS
//	const void* buffer
//		コピー元のバッファ
//	ModSize size
//		書き込むサイズ
//	ModSerialIO::DataType dummy
//		使用しないダミー引数
//
// RETURN
// 書き込んだサイズ
//
// EXCEPTIONS
//	その他
//		サブクラスのwriteOverFlowか、ModPureMemory::writeOverFlowの例外参照

int	
ModPureMemory::writeSerial(const void* buf, ModSize size,
						   ModSerialIO::DataType dummy)
{
	ModSize	rest = this->totalSize - this->currentPosition;
	if (size <= rest) {

		// 指定されたサイズのデータをメモリーの残りで格納できる

		this->copy(buf, size);
	} else {

		// メモリーの残りでは指定されたサイズに足りない

		if (this->houseMode == houseAbandon)
			return 0;

		// まず、メモリー中へ書き出す

		this->copy(buf, rest);

		// 残りのぶんを登録されている
		// オーバーフロー関数を呼び出して、書き出す

		this->writeOverFlow((char*) (const_cast<void*>(buf)) + rest,
							size - rest);
	}

	return (int) size;
}

// FUNCTION public
// ModPureMemory::seekSerial -- 現在位置をシークする
//
// NOTES
//	シークして現在位置を移動させ、新たな現在位置を返す。
//	現在はオーバーフローを呼び出していないが、
//	要望があれば呼び出すように変更も可能。
//	引数、返り値は、親クラスとの関係上ModFileOffsetを利用しているが、
//	値の範囲はModOffset型の範囲で利用しなければならない。
//
// ARGUMENTS
//	ModFileOffset ofs
//		シークするオフセット(ModOffset型の範囲)
//	ModPureMemory::SeekWhence whence
//		シークのタイプを指定する。seekSetは先頭からの絶対オフセット
//
// RETURN
//	シークに成功の場合現在位置を返す。不成功の場合は例外を送出。
//
// EXCEPTIONS
// 	ModCommonErrorOutOfRange
//		メモリの範囲を超えてアクセス
//

ModFileOffset
ModPureMemory::seekSerial(ModFileOffset ofs, SeekWhence whence)
{
	switch (whence) {
	case seekCurrent:
		if ((ofs < 0 && -ofs > (ModFileOffset) this->currentPosition) ||
			(ofs > 0 && ofs >
			 (ModFileOffset) (this->totalSize - this->currentPosition))) {
error:
			ModThrow(ModModuleOs,
					 ModCommonErrorOutOfRange, ModErrorLevelError);
		}
		this->currentPosition += (ModOffset) ofs;	break;

	case seekSet:
		if (ofs < 0 || ofs > (ModFileOffset) this->totalSize)
			goto error;
		this->currentPosition = (ModOffset) ofs;	break;

	case seekEnd:
		if (ofs > 0 || -ofs > (ModFileOffset) this->totalSize)
			goto error;
		this->currentPosition = this->totalSize + (ModOffset) ofs;
	}

	this->currentAddress = (char*) this->headAddress + this->currentPosition;

	return this->currentPosition;
}

//
// Copyright (c) 1997, 1999, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
