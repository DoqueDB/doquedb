// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModSerial.h -- シリアル
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

#ifndef __ModSerial_H__
#define __ModSerial_H__

#include "ModArchive.h"
#include "ModSerialSize.h"

//
// CLASS
// ModSerializer -- シリアル化の基底クラス
//
// NOTES
// シリアルを行うための基底クラス。シリアル化を行うクラスは、
// 本クラスのサブクラスとして実装し、純粋仮想関数であるserialize
// を必ず実装する。実装は、引数で渡される archiver_がストアーか
// ロードかを判断し、シリアル化を行うメンバをModArchiveのオペレータ
// (), <<, >> いずれかを用いてarchiver_にセットする。
//
// 抽象クラスであるので、ModObjectのサブクラスとはしない。

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModSerializer
{
public:
    virtual ~ModSerializer();

    // このメソッドを純粋仮想関数にする。
    virtual void serialize(ModArchive& archiver_) = 0;
    virtual ModSize sizeOfSerialized();

    // これで、<<, >> を使えるようにする。
    friend ModArchive&
		operator <<(ModArchive& archiver_, ModSerializer& serial_);
    friend ModArchive&
		operator >>(ModArchive& archiver_, ModSerializer& serial_);
};

//
// FUNCTION public
// ModSerializer::~ModSerializer -- ModSerializerのでィストラクタ
//
// NOTES
// なにもしない
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline 
ModSerializer::~ModSerializer()
{
    // do nothing
}

//
// FUNCTION public
// ModSerializer::sizeOfSerialized -- アーカイブサイズを得る
//
// NOTES
// アーカイバデータを格納する際のデータサイズを得る。
//
// ARGUMENTS
// なし
//
// RETURN
// 格納するオブジェクトのデータサイズ
//
// EXCEPTIONS
// なし
//

inline
ModSize 
ModSerializer::sizeOfSerialized()
{
    // ModSerialSizeを使って計算する。

	ModSerialSize	io;
	ModArchive		archiver(io, ModArchive::ModeStoreArchive);
	archiver << *this;
	return archiver.getSize();
};

//
// FUNCTION public
// ModSerializer::operator << -- << オペレータでアーカイブを実現する
//
// NOTES
// 本クラスのサブクラスにおける<<マニピュレータを実現する。
//
// ARGUMENTS
// ModArchive& archiver_
//	アーカイバへの参照
// ModSerializer& serial_
//	本クラス自身への参照
//
// RETURN
// アーカイバへの参照を返す。
//
// EXCEPTIONS
//  その他
//      ModArchiveのコンストラクタで設定されたModSerialIOクラスのサブクラス
//      が発生させる例外を送出する。
//

inline
ModArchive& 
operator <<(ModArchive& archiver_, ModSerializer& serial_)
{
    ModArchive::Mode	m = archiver_.getMode();
    if (m == ModArchive::ModeLoadStoreArchive) {
		archiver_.setMode(ModArchive::ModeStoreArchive);
    } 	

	try {
		serial_.serialize(archiver_);
	} catch (ModException& exception) {
		archiver_.setMode(m);
		ModRethrow(exception);
	}

	archiver_.setMode(m);
    return archiver_;
}

//
// FUNCTION public
// ModSerializer::operator >> -- >> オペレータでアーカイブを実現する
//
// NOTES
// 本クラスのサブクラスにおける>>マニピュレータを実現する。
//
// ARGUMENTS
// ModArchive& archiver_
//	アーカイバへの参照
// ModSerializer& serial_
//	本クラス自身への参照
//
// RETURN
// アーカイバへの参照を返す。
//
// EXCEPTIONS
//  その他
//      ModArchiveのコンストラクタで設定されたModSerialIOクラスのサブクラス
//      が発生させる例外を送出する。
//

inline
ModArchive& 
operator >>(ModArchive& archiver_, ModSerializer& serial_)
{
    ModArchive::Mode	m = archiver_.getMode();
    if (m == ModArchive::ModeLoadStoreArchive) {
		archiver_.setMode(ModArchive::ModeLoadArchive);
    }

	try {
		serial_.serialize(archiver_);
	} catch (ModException& exception) {
		archiver_.setMode(m);
		ModRethrow(exception);
	}
 
    archiver_.setMode(m);
    return archiver_;
}

#endif	// __ModSerial_H__

//
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
