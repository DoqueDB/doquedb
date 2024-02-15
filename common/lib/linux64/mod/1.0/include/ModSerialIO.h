// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4
//
// ModSerialIO.h -- シリアル化IOの基底クラス
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

#ifndef __ModSerialIO_H__
#define __ModSerialIO_H__

#include "ModTypes.h"

//
// CLASS
// ModSerialIO -- シリアルIOの基底クラス
//
// NOTES
// シリアルの対象となるオブジェクト(Memory,File,Socket)などの入出力を行う
// 基底クラス。メソッドreadSerial(), writeSerial()が純粋仮想関数なので
// 必ず実装する。
//
// 抽象クラスなので、ModObjectのサブクラスとはしない。

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

//【注意】	long や unsigned long は64ビットと32ビットとで互換性がないので、
//			注意すること

class ModSerialIO
{
public:
    // 
    // ENUM
    // DataType -- シリアルIOを行うデータ型
    //
    // NOTES
    // 引数に渡すデータ型を指定する
	//
	enum DataType {
		dataTypeVariable = 0,
		dataTypeCompressedVariable,
		dataTypeCharacter,
		dataTypeShort,
		dataTypeInteger,
		dataTypeLong,
		dataTypeFloat,
		dataTypeDouble,
		dataTypeCharacterArray,
		dataTypeShortArray,
		dataTypeIntegerArray,
		dataTypeLongArray,
		dataTypeFloatArray,
		dataTypeDoubleArray,
		// Windows用 (未使用)
		dataTypeInt64,
		dataTypeInt64Array,
	    // Solaris用 (未使用)
		dataTypeLongLong,
		dataTypeLongLongArray
	};

	//
	// ENUM
	// ModSerialIO::SeekWhence -- シークの種類
	// NOTES
	// シークの種類を指定する。先頭からの絶対オフセット、
	// 現在位置からの相対オフセット、メモリ領域の最後を指定できる。
	// 
	enum SeekWhence {
		seekSet         = 0,	// 先頭からの絶対オフセット
		seekCurrent     = 1,	// 現在位置からの相対オフセット
		seekEnd         = 2 	// メモリ領域の最後からのオフセット
    };

    virtual int 	readSerial(void* buffer_, ModSize byte_, DataType type_) = 0;
    virtual int 	writeSerial(const void* buffer_, ModSize byte_, DataType type_) = 0;
    virtual void 	resetSerial();
    virtual void 	readFlushSerial();
    virtual void 	writeFlushSerial();

	// 以下、ものによっては返り値をなくし、例外送出するかどうか要検討
	virtual	ModFileOffset 	seekSerial(ModFileOffset offset_, 
									   SeekWhence whence_);
	
	virtual	ModFileOffset	getCurrentPosition();

	virtual	void*	getCurrentAddress();
	virtual	void*	getHeadAddress();
	virtual	int		getSerialSize();
	virtual	int		getCompressSize();

	virtual int		rawRead(void* buffer_, ModSize byte_);
	virtual int		rawRead(void* buffer_, ModSize byte_, ModSize min_);
	virtual int		rawWrite(const void* buffer_, ModSize byte_);
	virtual int		rawWrite(const void* buffer_, ModSize byte_, ModSize min_);
};

//
// FUNCTION public
// ModSerialIO::resetSerial -- シリアル化のリセット
//
// NOTES
// 通常は何もしない。メモリなどに書き込み、読み込みを行う場合の
// 操作の切換時にポインタなどをリセットするために用意したメソッド。
// 必要な場合、本メソッドをオーバーライトして実装する。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// 実装依存
//
inline
void
ModSerialIO::resetSerial() 
{ 
	return; 
}

//
// FUNCTION public
// ModSerialIO::readFlushSerial -- 読み込みデータのフラッシュ
//
// NOTES
// 通常は何もしない。バッファリングを行った場合、読み込みデータのフラッシュを
// 行うために用意したメソッド。
// 必要な場合、本メソッドをオーバーライトして実装する。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// 実装依存
//
inline
void
ModSerialIO::readFlushSerial() 
{ 
	return; 
}

//
// FUNCTION public
// ModSerialIO::writeFlushSerial -- 書き込みデータのフラッシュ
//
// NOTES
// 通常は何もしない。バッファリングを行った場合、書き込みデータのフラッシュを
// 行うために用意したメソッド。
// 必要な場合、本メソッドをオーバーライトして実装する。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// 実装依存
//
inline
void
ModSerialIO::writeFlushSerial() 
{ 
	return; 
}

// 以下、ものによっては返り値をなくし、例外送出するかどうか要検討

//
// FUNCTION public
// ModSerialIO::seekSerial -- メモリポインターのシーク
//
// NOTES
// 通常は何もしない。メモリ領域へのアーカイブの場合に
// 書き込み、読み込み位置を指定するために用意したメソッド。
// 必要な場合、本メソッドをオーバーライトして実装する。
//
// ARGUMENTS
// ModFileOffset offset_
//	シークオフセット
// SeekWhence whence_
//  シーク位置
//
// RETURN
//  シーク長、ただし未実装では-1を返す
//
// EXCEPTIONS
// 実装依存
//
inline
ModFileOffset
ModSerialIO::seekSerial(ModFileOffset offset_, SeekWhence whence_) 
{ 
	return (-1); 
}

//
// FUNCTION public
// ModSerialIO::getCurrentPosition -- 現在のメモリポインタの位置を得る
//
// NOTES
// 通常は何もしない。メモリ領域へのアーカイブの場合に
// 現在のメモリポインタの位置を得るために用意したメソッド。
// 必要な場合、本メソッドをオーバーライトして実装する。
//
// ARGUMENTS
// なし
//
// RETURN
// メモリポインタの位置、ただし未実装では-1を返す。
//
// EXCEPTIONS
// 実装依存
//
inline
ModFileOffset
ModSerialIO::getCurrentPosition() 
{ 
	return (-1); 
}

//
// FUNCTION public
// ModSerialIO::getCurrentAddress -- 現在のメモリポインタを得る
//
// NOTES
// 通常は何もしない。メモリ領域へのアーカイブの場合に
// 現在のメモリポインタを得るために用意したメソッド。
// 必要な場合、本メソッドをオーバーライトして実装する。
//
// ARGUMENTS
// なし
//
// RETURN
// 現在のメモリポインタ、ただし未実装では0を返す。
//
// EXCEPTIONS
// 実装依存
//
inline
void*
ModSerialIO::getCurrentAddress() 
{ 
	return 0; 
}

//
// FUNCTION public
// ModSerialIO::getHeadAddress -- メモリ先頭ポインタを得る
//
// NOTES
// 通常は何もしない。メモリ領域へのアーカイブの場合に
// メモリの先頭ポインタを得るために用意したメソッド。
// 必要な場合、本メソッドをオーバーライトして実装する。
//
// ARGUMENTS
// なし
//
// RETURN
// メモリの先頭ポインタ、ただし未実装では0を返す。
//
// EXCEPTIONS
// 実装依存
//
inline
void*
ModSerialIO::getHeadAddress() 
{ 
	return 0; 
}

//
// FUNCTION public
// ModSerialIO::getSerialSize -- メモリ領域のサイズを得る
//
// NOTES
// 通常は何もしない。メモリ領域へのアーカイブの場合に
// メモリ領域のサイズを得るために用意したメソッド。
// 必要な場合、本メソッドをオーバーライトして実装する。
//
// ARGUMENTS
// なし
//
// RETURN
// メモリ領域サイズ、ただし未実装では-1を返す。
//
// EXCEPTIONS
// 実装依存
//
inline
int
ModSerialIO::getSerialSize() 
{ 
	return(-1); 
}

//
// FUNCTION public
// ModSerialIO::getCompressSize -- メモリ領域の圧縮サイズを得る
//
// NOTES
// 通常は何もしない。メモリ領域へのアーカイブの場合に
// メモリ領域の圧縮サイズを得るために用意したメソッド。
// 必要な場合、本メソッドをオーバーライトして実装する。
//
// ARGUMENTS
// なし
//
// RETURN
// メモリ領域の圧縮サイズ、ただし未実装では-1を返す。
//
// EXCEPTIONS
// 実装依存
//
inline
int
ModSerialIO::getCompressSize() 
{ 
	return(-1); 
}

//
// FUNCTION public
// ModSerialIO::rawRead -- 実際の読み込みを行う
//
// NOTES
// 通常は何もしない。サブクラスとして実装したクラスで
// 読み込みを行うために用意したメソッド。
// 必要な場合、本メソッドをオーバーライトして実装する。
//
// ARGUMENTS
// void* buffer_
//	読み込みバッファへのポインタ
// ModSize byte_
//  読み込みサイズ
// ModSize min_
//	無視される
//
// RETURN
//  読み込みサイズ、ただし未実装ではサイズをそのままを返す。
//
// EXCEPTIONS
// 実装依存
//

inline
int
ModSerialIO::rawRead(void* buffer_, ModSize byte_)
{ 
	return (int) byte_; 
}

inline
int
ModSerialIO::rawRead(void* buffer_, ModSize byte_, ModSize min_)
{
	return this->rawRead(buffer_, byte_);
}

//
// FUNCTION public
// ModSerialIO::rawWrite -- 実際の書き込みを行う
//
// NOTES
// 通常は何もしない。サブクラスとして実装したクラスで
// 書き込みを行うために用意したメソッド。
// 必要な場合、本メソッドをオーバーライトして実装する。
//
// ARGUMENTS
// void* buffer_
//	書き込みバッファへのポインタ
// ModSize byte_
//  書き込みサイズ
// ModSize min_
//	無視される
//
// RETURN
//  書き込みサイズ、ただし未実装ではサイズをそのままを返す。
//
// EXCEPTIONS
// 実装依存
//

inline
int
ModSerialIO::rawWrite(const void* buffer_, ModSize byte_)
{ 
	return (int) byte_;
}

inline
int
ModSerialIO::rawWrite(const void* buffer_, ModSize byte_, ModSize min_)
{
	return this->rawWrite(buffer_, byte_);
}

#endif	// __ModSerialIO_H__

//
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

