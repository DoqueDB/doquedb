// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4
//
// ModSerialSize.h -- シリアル化 Sizeを得る
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

#ifndef __ModSerialSize_H__
#define __ModSerialSize_H__

#include "ModCommonDLL.h"
#include "ModSerialIO.h"
#include "ModDefaultManager.h"

//
// モジュールは汎用OSに属する。
// したがって、エラーはModOsXXXである。

//
// CLASS
// ModPureSerialSize -- アーカイブサイズを得るための機能クラス
// 
// NOTES
// アーカイブを行う時のデータサイズを算出するための
// 機能クラスで、ModSerialIOクラスのサブクラスとして実装する。
//
// 実際には、メモリハンドルを明示したModSerialSizeクラスを利用する。

class ModCommonDLL ModPureSerialSize
	: public	ModSerialIO
{
public:
    ModPureSerialSize();
    virtual ~ModPureSerialSize();

    int	 	readSerial(void* buffer_, ModSize byte_, ModSerialIO::DataType type_);
    int 	writeSerial(const void* buffer_, ModSize byte_, ModSerialIO::DataType type_);
    void 	resetSerial();
};

//
// CLASS
//	ModSerialSize -- ModPureSerialSizeクラスのメモリハンドル明示クラス
// NOTES
//	ModPureSerialSizeクラスをデフォルトメモリハンドルの管理下のクラスとして
//	利用するためのクラスである。ユーザは通常本クラスを利用する。
//

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModSerialSize
	: public	ModObject<ModDefaultManager>, 
	  public	ModPureSerialSize
{
public:
	// コンストラクター
	ModSerialSize()
	{}
	// デストラクター
	~ModSerialSize()
	{}
};

#endif	// __ModSerialSize_H__

//
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
