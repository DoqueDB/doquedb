// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4
//
// ModCodec.h -- 圧縮、暗号化
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

#ifndef __ModCodec_H__
#define __ModCodec_H__

#include "ModCommonDLL.h"
#include "ModDefaultManager.h"

//
// モジュールは汎用OSに属する。
// したがって、ModOsObjectのサブクラスとして作成し、エラーはModOsXXX
// である。
//

class ModSerialIO;

//
// CLASS
// ModPureCodec -- （圧縮・伸長）、暗号化、バッファリングを行う基底クラス
//
// NOTES
// （圧縮・伸長）、暗号化、バッファリングを行う機能クラス。
// 実際にはメモリハンドルを明示した型ModCodecを使う。
// ModFile, ModSocketで利用される。本クラスは、指定された
// エンコードバッファ領域にデータを格納し、バッファがオーバーフローしたら
// デコードしデコードバッファに圧縮、暗号化を行い格納する。
// 実際の圧縮・伸長、暗号化アルゴリズムは、仮想関数rawEncode, rawDecodeを
// オーバーライトし実装する。何も実装しないとバッファリングのみを行う。
// 【注意】 本クラスを使用して作成したファイルは、本クラスの同じバッファ
// サイズのインスタンスにより読み込むことしかできない。

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModPureCodec
{
public:
	//
	// ENUM
	// ModPureCodec::CodecMode -- 圧縮・伸長、バッファリングを区別する
	//
	// NOTES
	// ModPureCodec のバッファの利用方法を（圧縮・伸長）、暗号化、バッファリング
	// のモードとして指定する値
	//
    enum CodecMode 
    {
		bufferMode	= 0x00,		// なにもせずにバッファリング機能を提供
		compressMode = 0x01,
		cryptMode = 0x02,
		blockMode = 0x04		// 決まったブロックサイズしか読み書きしない
	};

	ModCommonDLL
	ModPureCodec(ModSize bufferSize_, CodecMode mode_ = bufferMode);
	ModCommonDLL
    virtual ~ModPureCodec();

	ModCommonDLL
	void	reset();
	ModCommonDLL
	virtual void	encodeFlush(ModSerialIO* io_);						// 暗号化対応の為 virtual
	ModCommonDLL
	virtual void	decodeFlush(ModSerialIO* io_, ModSize min_ = 1);	// 暗号化対応の為 virtual
    ModCommonDLL
	virtual bool    hasBufferedData() const;                            // 暗号化対応の為 virtual
	ModCommonDLL
    int		encode(ModSerialIO* io_, const void* buffer_, ModSize size_);
	ModCommonDLL
    int		decode(ModSerialIO* io_, void* buffer_, ModSize size_);

	// アクセサ
	CodecMode getMode() const;
	ModSize getBufferSize() const;
	ModSize	getEncodedSize() const;
	ModSize	getDecodedSize() const;
	ModSize	getTotalCompressSize() const;

protected:																// 暗号化対応の為 private から変更
	void	destruct();							// デストラクター下位関数

    // 以下のメソッドを実装し、圧縮機能を提供する。
	ModCommonDLL														// 暗号化対応の為エクスポート
	int	rawEncode(void* in_, ModSize inSize_, void* out_, ModSize* outSize_);
	ModCommonDLL														// 暗号化対応の為エクスポート
	int	rawDecode(int encodeStatus_, void* in_, ModSize inSize_, void* out_, ModSize* outSize_);

    CodecMode	mode;
	ModSize		bufferSize;

	char*		buffer;
	char*		current;
	ModSize		encodedSize;
	ModSize		decodedSize;

	char*		compressBuffer;
	ModSize		compressedSize;
};

//
// FUNCTION public
// ModPureCodec::getTotalCompressSize -- 圧縮・伸長サイズを得る
//
// NOTES
// 圧縮・伸長されたデータのサイズを得る。
//
// ARGUMENTS
// なし
//
// RETURN
// 圧縮データのサイズ
//
// EXCEPTIONS
// なし
//

inline
ModSize
ModPureCodec::getTotalCompressSize() const
{
	return this->compressedSize;
}

//
// FUNCTION public
// ModPureCodec::getBufferSize -- コーデックバッファのサイズを得る
//
// NOTES
// コーデックバッファのサイズを得る。
//
// ARGUMENTS
// なし
//
// RETURN
// コーデックバッファのサイズ
//
// EXCEPTIONS
// なし
//

inline
ModSize
ModPureCodec::getBufferSize() const
{
	return this->bufferSize;
}

//
// FUNCTION public
// ModPureCodec::getMode -- コーデックのモードを得る
//
// NOTES
// コーデックのモードを得る。
//
// ARGUMENTS
// なし
//
// RETURN
// コーデックのモード
//
// EXCEPTIONS
// なし
//

inline
ModPureCodec::CodecMode
ModPureCodec::getMode() const
{
	return this->mode;
}

//	FUNCTION public
//	ModPureCodec::getEncodedSize -- 符号化待ちのデータのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		符号化待ちのデータのサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

inline
ModSize
ModPureCodec::getEncodedSize() const
{
	return this->encodedSize;
}

//	FUNCTION public
//	ModPureCodec::getDecodedSize -- 復号化済みのデータのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		復号化済みのデータのサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

inline
ModSize
ModPureCodec::getDecodedSize() const
{
	return this->decodedSize;
}

//	CLASS
//	ModCodec -- ModPureCodecクラスのメモリハンドル明示クラス
//
//	NOTES
//		ModPureCodecクラスをデフォルトメモリハンドルの管理下のクラスとして
//		利用するためのクラスである。ユーザは通常本クラスを利用する。

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModCodec
	: public	ModObject<ModDefaultManager>,
	  public	ModPureCodec
{
public:
	// コンストラクター
	ModCodec(ModSize bufferSize_, CodecMode mode_ = bufferMode)
		: ModPureCodec(bufferSize_, mode_)
	{}
	// デストラクター
	~ModCodec()
	{}
};

#endif	// __ModCodec_H__

//
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
