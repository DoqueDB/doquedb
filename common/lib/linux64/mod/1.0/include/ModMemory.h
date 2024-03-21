// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4
//
// ModMemory.h -- シリアル化可能メモリのクラス定義
// 
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __ModMemory_H__
#define __ModMemory_H__

#include "ModCommonDLL.h"
#include "ModOsDriver.h"
#include "ModDefaultManager.h"
#include "ModSerialIO.h"

//
// モジュールは汎用OSに属する。
// したがって、専用エラーはModOsXXXである。
//

//
// CLASS
// ModPureMemory -- シリアライズ可能なメモリ機能クラス
//
// NOTES
// メモリを保持し、メモリ内容をアーカイバによってシリアライズすることが
// できる機能クラスである。
// シリアライズ可能とするため、ModSerialIOクラスの派生クラスである。
// プレーンなメモリを確保し、コンストラクタに渡して生成することもできるが、
// 各モジュールのマネージャで用意されるメモリ確保の関数でも直接生成することが
// できる。
// ModPureMemory自身のメモリは、ModDefaultManagerのメモリハンドルから獲得される。
// 残りのメモリサイズが不足し、指定サイズのデータが格納でき
// ない、あるいは読み込めないような場合の動作は、コンストラクタの引数houseMode
// で指定する。
// 実際にはメモリハンドルを明示したクラスModMemoryを用いる。

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModPureMemory
	: public	ModSerialIO
{
public:
	//
	// ENUM
	// ModPureMemory::HouseMode -- 残りサイズ不足の場合の動作指定
	// NOTES
	// 書き込むデータがメモリに格納しきれない場合、あるいは読み込み対象の
	// 残りのメモリサイズが指定サイズに対して不足している場合の動作を指定する。
	// houseAbandonの場合、残りサイズの不足がわかった時点で関数がエラーに
	// なり、何も処理(書き込んだり読み込んだり)せずに0を返す。
	// この場合はオーバーフロー関数は呼び出されない。
	// houseFillの場合は、メモリ領域の最後まで
	// 詰めて処理し、格納もしくは読み込んだ後にオーバーフロー関数を呼び出す。
	// houseAbandonがデフォルトである。
	//
	enum HouseMode {
		houseFill       = 0,
		houseAbandon    = 1
    };
	// コンストラクタ、デストラクタ
    ModPureMemory(void* address, ModSize size, 
			  HouseMode houseMode_ = houseAbandon);
    virtual ~ModPureMemory();
	//
	// シリアライズに必要な関数群
	//
	ModCommonDLL
    int	readSerial(void* buffer_, ModSize byte_, 
				   ModSerialIO::DataType type_);
	ModCommonDLL
    int	writeSerial(const void* buffer_, ModSize byte_, 
					ModSerialIO::DataType type_);
    // 先頭に戻る
	void 	resetSerial();	
	// readFlushSerial();	// ModPureMemoryでは不要。
	// writeFlushSerial();	// ModPureMemoryでは不要。

	// 現在位置を移動し、新たな現在位置を返す
	// メモリ上ではModOffsetの範囲の値を利用すべきだが、ModSerialIOを
	// 継承しているため、型はこのように定義される。ただし、ModOffsetの範囲
	// の値を渡す必要がある。
	ModCommonDLL
	ModFileOffset seekSerial(ModFileOffset offset_, 
							 ModSerialIO::SeekWhence whence_);
	// アクセサ
	ModFileOffset	getCurrentPosition();
    void*  	getCurrentAddress();
    void* 	getHeadAddress();
    int		getSerialSize();
	// getCompressSize();	// ModPureMemoryでは不要。
	// rawRead();			// ModPureMemoryでは不要。
	// rawWrite();			// ModPureMemoryでは不要。

	void					renewalMemory(void* p, ModSize size);
	ModCommonDLL
	void*					renewalMemory(void* p, ModSize size,
										  ModSize* oldSize);
												// 管理するメモリーを交換する

	// memset, memcpyに相当するメソッド
	// ★残りサイズ、オーバーフローチェックをしないので注意。必要な
	// 場合はwriteSerialを用いる。
	void set(int value, ModSize count);
	void copy(const void* source, ModSize count);

protected:
    ModSize					checkSize(ModSize size)	const;
												// 空き領域サイズを得る

	// 98/03/12 protectedに変更
	// 「ModPureMemory::readSerial()のオーバーフロー処理の中で
	//   readSerial() がやってるのとほとんど同じ処理をしているのです。
	//   その中で、上に挙げた２つのメンバ変数の値も更新しないと
	//   うまく処理ができないという事に気づいた。」ので
    void* currentAddress;		// 現在位置へのポインタ
    ModOffset currentPosition;	// 現在位置オフセット
	                            // (あえてModFileOffsetとしない)

	HouseMode houseMode;		// メモリサイズ不足動作モード
	ModSize totalSize;			// メモリ全体のサイズ

private:
	// デフォルトのオーバーフロー関数。メモリサイズ不足がわかった時点で
	// 例外を送出する。
	virtual void	writeOverFlow(void* address_, ModSize size_);
	virtual void	readOverFlow(void* address_, ModSize size_);

	// 以下の2データメンバを protected に変更
//	HouseMode houseMode;		// メモリサイズ不足動作モード
//  ModSize totalSize;			// メモリ全体のサイズ
    void* headAddress;			// メモリの先頭へのポインタ
};

//
// FUNCTION public
// ModPureMemory::ModPureMemory -- メモリクラスのコンストラクタ
//
// NOTES
//	メモリクラスのコンストラクタである。引数で指定したメモリ領域、
//	メモリサイズ、残りサイズ不足の場合の動作が内部に設定される。
//	現在位置はメモリ領域の先頭を指すように初期化される。
//	各モジュールのマネージャのメモリ確保関数allocateMemoryでも、
//	ModPureMemoryを生成して返すI/Fが用意されている。
//
// ARGUMENTS
//	void* address
//		確保したメモリのアドレス
//	ModSize size
//		メモリサイズ
//	ModPureMemory::HouseMode houseMode_
//		残りサイズ不足の動作モード
//
// RETURN
// なし
//
// EXCEPTIONS
//	なし
//

inline
ModPureMemory::
ModPureMemory(void* address, ModSize size, HouseMode houseMode_) 
	: headAddress(address),
	  totalSize(size),
	  currentAddress(address),
	  currentPosition(0),
	  houseMode(houseMode_)
{ }

//
// FUNCTION public
// ModPureMemory::~ModPureMemory -- メモリクラスのデストラクタ
//
// NOTES
//	メモリクラスのデストラクタであるが、何もしない。仮想関数のためにある。
//	保持しているメモリの解放は別に行う必要がある。これに対して、
//	ModXXXManagerが提供する関数freeMemoryは、メモリハンドルから確保した
//	内部メモリと、メモリクラスの両方を解放する。
//
// ARGUMENTS
//	void* address
//		確保したメモリのアドレス
//	ModSize size
//		メモリサイズ
//	ModPureMemory::HouseMode houseMode_
//		残りサイズ不足の動作モード
//
// RETURN
// なし
//
// EXCEPTIONS
//	なし
//

inline
ModPureMemory::~ModPureMemory()
{ }

//
// FUNCTION public
// ModPureMemory::getSerialSize -- メモリサイズを返す
//
// NOTES
// 確保しているメモリのメモリサイズを返す。
// シリアル化I/Fに従って作成されている。
//
// ARGUMENTS
//		なし
//
// RETURN
// 確保しているメモリのサイズ
//
// EXCEPTIONS
//	なし
//
inline int 	
ModPureMemory::getSerialSize()
{ 
    return (int)this->totalSize; 
}

//
// FUNCTION public
// ModPureMemory::getHeadAddress -- 確保しているメモリの先頭アドレスを返す
//
// NOTES
// 確保しているメモリの先頭アドレスを返す。
// シリアル化I/Fに従って作成されている。
//
// ARGUMENTS
//		なし
//
// RETURN
// 確保しているメモリの先頭アドレス
//
// EXCEPTIONS
//	なし
//
inline void*
ModPureMemory::getHeadAddress() 
{ 
    return this->headAddress; 
}

//
// FUNCTION public
// ModPureMemory::getCurrentAddress -- メモリ内で現在指しているポインタを得る
//
// NOTES
// メモリ内で現在指しているポインタを得る。
// シリアル化I/Fに従って作成されている。
//
// ARGUMENTS
//		なし
//
// RETURN
// メモリ内で現在指しているポインタ
//
// EXCEPTIONS
//	なし
//
inline void*
ModPureMemory::getCurrentAddress()
{ 
    return this->currentAddress; 
}

//
// FUNCTION public
// ModPureMemory::getCurrentPosition -- メモリ内で現在指している位置を得る
//
// NOTES
//	メモリ内で現在指している先頭からの位置を返す。
//	シリアル化I/Fに従って作成されているため、ModFileOffset型で値を返す。
//	ただし、返り値はメモリ内部の位置を示すもので、
//	返す値の範囲はModOffset型の範囲に限られる。
//
// ARGUMENTS
//		なし
//
// RETURN
// メモリ内で現在指しているオフセット位置
//
// EXCEPTIONS
//	なし
//
inline ModFileOffset
ModPureMemory::getCurrentPosition()
{ 
    return (ModFileOffset)this->currentPosition; 
}

//
// FUNCTION public
// ModPureMemory::resetSerial -- 指している位置をメモリの先頭に戻す
//
// NOTES
// メモリ内で指している位置を先頭に戻す。
// シリアル化I/Fに従って作成されている。
//
// ARGUMENTS
//		なし
//
// RETURN
// なし
//
// EXCEPTIONS
//	なし
//
inline void 
ModPureMemory::resetSerial()
{
    this->currentAddress = this->headAddress; 
    this->currentPosition = 0;
}

//
// FUNCTION public
// ModPureMemory::set -- メモリ内容の設定
//
// NOTES
// 現在位置から、メモリ内容を指定値で塗りつぶす。
// OSで供給されるmemsetに相当し、一切のチェックやオーバーフロー処理をしない。
// 残りサイズのチェックが必要な場合にはwriteSerialを利用する。
//
// ARGUMENTS
//	int value
//		指定値
//	ModSize count
//		設定するサイズ
//
// RETURN
// なし
//
// EXCEPTIONS
//	なし
//

inline
void
ModPureMemory::set(int value, ModSize count)
{
	ModOsDriver::Memory::set(this->currentAddress,
							 (unsigned char) value, count);
	this->currentPosition += count;
	this->currentAddress = (char*) this->currentAddress + count;
}

//
// FUNCTION public
// ModPureMemory::copy -- メモリの現在位置に対して内容をコピーする
//
// NOTES
// 現在位置に対して、メモリ内容をコピーする。
// OSで供給されるmemcopyに相当し、一切のチェックやオーバーフロー処理をしない。
// 残りサイズのチェックが必要な場合にはwriteSerialを利用する。
//
// ARGUMENTS
//	const void* source
//		コピー元のアドレス
//	ModSize count
//		コピーするサイズ
//
// RETURN
// なし
//
// EXCEPTIONS
//	なし
//
inline
void
ModPureMemory::copy(const void* source, ModSize count)
{
	ModOsDriver::Memory::copy(this->currentAddress, source, count);
	this->currentPosition += count;
	this->currentAddress = (char*) this->currentAddress + count;
}

//
// FUNCTION protected
// ModPureMemory::checkSize -- メモリ中の残りサイズを得る
//
// NOTES
// 指定サイズに対して、メモリの残りサイズと比較し、
// 実際にアクセス可能なサイズを得る。
//
// ARGUMENTS
//	ModSize size
//		アクセス要求サイズ
//
// RETURN
// アクセス可能なサイズ
//
// EXCEPTIONS
//	なし
//

inline
ModSize
ModPureMemory::checkSize(ModSize size) const
{
	ModSize	rest = this->totalSize - this->currentPosition;
	return (size > rest) ? rest : size;
}

//
// FUNCTION private
// ModPureMemory::readOverFlow -- メモリからの読み込み用オーバーフロー関数
//
// NOTES
//	読み込み時に、残りサイズが不足したときに呼び出されるデフォルト関数。
//	houseFillが指定されている場合には、例外を送出する。
// 	実際には派生クラスでその動作を実装する。
//	あらかじめ設定されているHouseMode によって呼び出される前の処理が異なる。
//
// ARGUMENTS
//	void* address
//		ユーザ指定バッファ内での現在位置へのポインタ
//	ModSize size
//		処理しきれなかった残りサイズ
//
// RETURN
//	なし
//
// EXCEPTIONS
// ModCommonErrorNotOverLoad
//		必要な関数がオーバーロードされていない
//

inline
void 	
ModPureMemory::readOverFlow(void* address, ModSize size)
{ 
	if (this->houseMode == houseFill) {
		ModThrow(ModModuleOs, ModCommonErrorNotOverLoad, ModErrorLevelWarning);
	}
}

//
// FUNCTION private
// ModPureMemory::writeOverFlow -- メモリへの書き込み用オーバーフロー関数
//
// NOTES
//	メモリへの書き込み時に、残りサイズが不足したときに呼び出される
//	デフォルト関数。
//	houseFillが指定されている場合には、例外を送出する。
// 	実際には派生クラスでその動作を実装する。
//	あらかじめ設定されているHouseMode によって呼び出される前の処理が異なる。
//
// ARGUMENTS
//	void* address
//		ユーザ指定バッファ内で処理しきれなかった位置へのポインタ
//	ModSize size
//		処理しきれなかった残りサイズ
//
// RETURN
//	なし
//
// EXCEPTIONS
// ModCommonErrorNotOverLoad
//		必要な関数がオーバーロードされていない
//

inline
void 	
ModPureMemory::writeOverFlow(void* address_, ModSize size_)
{ 
	if (this->houseMode == houseFill) {
		ModThrow(ModModuleOs, ModCommonErrorNotOverLoad, ModErrorLevelWarning);
	}
    return;
}

//	FUNCTION public
//	ModPureMemory::renewalMemory -- 管理するメモリーを交換する
//
//	NOTES
//		内部で管理するメモリーと与えられたメモリーを交換する
//
//	ARGUMENTS
//		void*				p
//			新しく管理するメモリーの先頭アドレス
//		ModSize				size
//			新しく管理するメモリーのサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModPureMemory::renewalMemory(void* p, ModSize size)
{
	this->headAddress = p;
	this->totalSize = size;
	this->resetSerial();
}

//
// CLASS
//	ModMemory -- ModPureMemoryクラスのメモリハンドル明示クラス
// NOTES
//	ModPureMemoryクラスをデフォルトメモリハンドルの管理下のクラスとして
//	利用するためのクラスである。ユーザは通常本クラスを利用する。

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModMemory
	: public	ModDefaultObject,
	  public	ModPureMemory
{
public:
	// コンストラクター
    ModMemory(void* address, ModSize size, HouseMode houseMode_ = houseAbandon)
		: ModPureMemory(address, size, houseMode_)
	{}
	// デストラクター
	~ModMemory()
	{}
};

#endif	// __ModMemory_H__

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
