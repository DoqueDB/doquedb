// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4
//
// ModArchive.cpp -- アーカイブ
// 
// Copyright (c) 1997, 2000, 2023 Ricoh Company, Ltd.
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


#include "ModArchive.h"
#include "ModSerial.h"
#include "ModOsException.h"
#include "ModCommonInitialize.h"

// 静的クラスメンバーの定義

ModArchive::funcIO		ModArchive::_funcTable[ModArchive::ModeNum] =
{
	&ModArchive::dummyIOArchive,		// ModeUndefined のとき使用する
	&ModArchive::writeIOArchive,		// ModeStoreArchive のとき使用する
	&ModArchive::readIOArchive,			// ModeLoadArchive のとき使用する
	&ModArchive::dummyIOArchive			// ModeLoadStoreArchive のとき使用する
};

//	FUNCTION public
//	ModArchive::ModArchive -- アーカイバーを表すクラスのコンストラクター
//
//	NOTES
//		与えられたアーカイブモードは () 演算子の使用時に読み出しと
//		書き出しのどちらを行うかを決定する
//		アーカイブモードに ModArchive::ModeLoadStoreArchive を与えたとき、
//		() 演算子を使用できない
//
//	ARGUMENTS
//		ModSerialIO&		io
//			シリアル化入出力先
//		ModArchive::Mode 	mode
//			アーカイブモード
//
//	RETURN
//		なし 
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			指定されたモードがおかしい

ModArchive::ModArchive(ModSerialIO& io, Mode mode)
	: _io(io),
	  _mode(mode),
	  _size(0)
{
	// 必要ならば汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	switch (this->getMode()) {
	case ModeStoreArchive:
	case ModeLoadArchive:
	case ModeLoadStoreArchive:
		break;
	default:
		ModThrowOsError(ModCommonErrorBadArgument);
	}

	// シリアル化入出力先を初期化する

	_io.resetSerial();
}

#if MOD_CONF_BOOL_TYPE == 1
//	FUNCTION public
//	ModArchive::operator () -- 基本型を引数とする () 演算子
//
//	NOTES
//
//	ARGUMENTS
//		bool&				data
//			読み出した、または書き出した bool 型
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorEndOfFile
//			EOF まで読み出してしまった
//			(ModArchive::readIOArchive より)
//		ModOsErrorWriteNoSpace
//			EOF を超えて書き出そうとしている
//			(ModArchive::writeIOArchive より)
//		ModOsErrorOutOfMode
//			アーカイブモードが ModeLoadStoreArchive のアーカイブである
//			(ModArchive::dummyIOArchive より)

void
ModArchive::operator ()(bool& data)
{
	switch (this->getMode()) {
	case ModeStoreArchive:
		(void) this->writeArchive(data);	break;
	case ModeLoadArchive:
		(void) this->readArchive(data);		break;
	default:
		ModThrowOsError(ModOsErrorOutOfMode);
	}
}
#endif

//	FUNCTION public
//	ModArchive::operator () -- シリアライザーを引数にする () 演算子
//
//	NOTES
//		シリアル化入出力先からシリアル化可能なオブジェクトを読み出す
//		または、書き出す
//
//		ModSerial.h をインクルードしたくないので、
//		ModArchive.h でインライン化できない
//
//	ARGUMENTS
//		ModSerializer&		serializer
//			シリアライザー(シリアル化するオブジェクト)
//
//	RETURN
//		なし 
//
//	EXCEPTIONS

void
ModArchive::operator ()(ModSerializer& serializer)
{
    serializer.serialize(*this);
}

//	FUNCTION public
//	ModArchive::readIOArchive -- シリアル化入出力先からの読み込み
//
//	NOTES
//
//	ARGUMENTS
//		void*				buf
//			読み込んだデータを格納する領域の先頭アドレス
//		ModSize				size
//			読み込んだデータを格納する領域のサイズ(B 単位)
//		ModSerialIO::DataType	type
//			読み込むデータの型
//
//	RETURN
//		実際に読み込んだデータのサイズ(B 単位)
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			読み込むサイズが 0 より大きいのに
//			読み込んだデータを格納する領域の先頭アドレスが指定されていない
//		ModOsErrorEndOfFile
//			EOF まで読み出してしまい、指定された型のデータを読み出せなかった

ModSize
ModArchive::readIOArchive(void* buf, ModSize size, ModSerialIO::DataType type)
{
	if (size == 0)

		// 読み込むサイズは 0 なので、なにもしない

		return 0;
	if (buf == 0)

		// 読み込むサイズが 0 より大きいのに
		// 読み込んだデータを格納する領域の先頭アドレスが指定されていない

		ModThrowOsError(ModCommonErrorBadArgument);

	ModSize len = _io.readSerial(buf, size, type);
	if (!len && size)

		// EOF を検出した

		ModThrowOsWarning(ModOsErrorEndOfFile);

	_size += len;
	return len;
}

//	FUNCTION public
//	ModArchive::writeIOArchive -- シリアル化入出力先への書き出し
//
//	NOTES
//
//	ARGUMENTS
//		void*				buf
//			書き出すデータを格納する領域の先頭アドレス
//		ModSize				size
//			書き出すデータを格納する領域のサイズ(B 単位)
//		ModSerialIO::DataType	type
//			書き出すデータの型
//
//	RETURN
//		実際に書き出したデータのサイズ(B 単位)
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			書き出すサイズが 0 より大きいのに、
//			書き出すデータが格納される領域の先頭アドレスが指定されていない
//		ModOsErrorWriteNoSpace
//			EOF を超えて書き出そうとして、
//			指定された型のデータを読み出せなかった

ModSize
ModArchive::writeIOArchive(void* buf, ModSize size, ModSerialIO::DataType type)
{
	if (size == 0)

		// 書き出すサイズは 0 なので、なにもしない

		return 0;
	if (buf == 0)

		// 書き出すサイズが 0 より大きいのに、
		// 書き出すデータが格納される領域の先頭アドレスが指定されていない

		ModThrowOsError(ModCommonErrorBadArgument);

	ModSize len = _io.writeSerial(buf, size, type);
	if (!len && size)

		// EOF を超えて書き出そうとしている

		ModThrowOsError(ModOsErrorWriteNoSpace);

	_size += len;
	return len;
}

//	FUNCTION public
//	ModArchive::dummyIOArchive -- () 演算子実装用の例外発生関数
//
//	NOTES
//		例外 ModOsErrorOutOfMode が必ず発生する
//
//	RETURN
//		0
//
//	EXCEPTIONS
//		ModOsErrorOutOfMode
//			アーカイブモードが ModeLoadStoreArchive のアーカイブで
//			() 演算子を使用した

ModSize
ModArchive::dummyIOArchive(void* buffer, ModSize size,
						   ModSerialIO::DataType type)
{
	ModThrowOsError(ModOsErrorOutOfMode);
	return 0;
}

//
// Copyright (c) 1997, 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
