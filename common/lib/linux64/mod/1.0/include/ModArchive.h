// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4
//
// ModArchive.h -- アーカイブ関連のクラス定義
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

#ifndef __ModArchive_H__
#define __ModArchive_H__

#include "ModCommonDLL.h"
#include "ModTypes.h"
#include "ModSerialIO.h"
#include "ModCharTrait.h"
#include "ModDefaultManager.h"

class ModSerializer;

//	CLASS
//	ModArchive -- アーカイブを表すクラス
//
//	NOTES
//		シリアル化入出力を使用して、シリアル化可能な
//		MOD のオブジェクトのアーカイブを作成するためのクラス
//
//		シリアル化入出力先は ModSerialIO の子クラスとして提供される
//		また、シリアル化可能な MOD のオブジェクトは、
//		ModSerializer の子クラスのオブジェクトである
//
//		メモリーハンドルの明示、非明示クラスに分けるには、
//		演算子関数や serialize 関数をそれぞれに分けて定義する必要があるので、
//		現在は分けていない

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

//【注意】	long や unsigned long は64ビットと32ビットとで互換性がないので、
//			注意すること

class ModArchive
	: public	ModDefaultObject
{
public:
    //	ENUM
    //	ModArchive::Mode -- アーカイブモード
    //
    //	NOTES
	//		アーカイブに対する可能な操作を表す

    enum Mode
	{
		ModeUndefined =			0x00,			// 未定義
		ModeStoreArchive =		0x01,			// 書き出し専用
        ModeLoadArchive =		0x02,			// 読み出し専用
		ModeLoadStoreArchive =	0x03,			// 読み書き可能
		ModeNum									// モード数
    };

	//	TYPEDEF
	//	ModArchive::funcIO -- シリアル化可能な入出力先への操作関数の型
	//
	//	NOTES

	typedef	ModSize			(ModArchive::*funcIO)(void*, ModSize,
												  ModSerialIO::DataType);

	ModCommonDLL
    ModArchive(ModSerialIO& io, Mode mode);		// コンストラクター
    virtual ~ModArchive();						// デストラクター

	ModBoolean				isStore() const;	// 書き出し専用か
    ModBoolean				isLoad() const;		// 読み込み専用か

    Mode					getMode() const;	// アーカイブモードを得る
	void					setMode(Mode mode);	// アーカイブモードを設定する

    void					resetSize();		// アーカイブサイズの初期化
    ModSize					getSize() const;	// アーカイブサイズを得る
	ModSize					getCompressSize();	// 圧縮アーカイブサイズを得る

	void*					getHeadAddress();	// アーカイブの
												// 先頭アドレスを得る
	void*					getCurrentAddress();
												// アーカイブの
												// 現在のアドレスを得る
	ModSize					getBufferSize();	// シリアル化可能
												// 最大サイズを得る

	ModFileOffset			getCurrentPosition();
												// アーカイブの
												// 現在のオフセットを得る

	ModFileOffset			seek(ModFileOffset offset, 
								 ModSerialIO::SeekWhence whence);
												// アーカイブの入出力の
												// 開始位置を移動する

    void					flush(Mode mode = ModeUndefined);
												// シリアル化入出力先の
												// フラッシュ

    ModSize					readArchive(void* address, ModSize size);
												// 自由記憶領域へ読み出す
    ModSize					writeArchive(void* address, ModSize size);
												// 自由記憶領域から書き出す
#if MOD_CONF_BOOL_TYPE == 1
	ModSize					readArchive(bool& data);
#endif
    ModSize					readArchive(char& data);
    ModSize 				readArchive(short& data);
    ModSize 				readArchive(int& data);
    ModSize 				readArchive(long& data);
	ModSize					readArchive(ModInt64& data);
    ModSize 				readArchive(unsigned char& data);
    ModSize 				readArchive(unsigned short& data);
    ModSize 				readArchive(unsigned int& data);
    ModSize 				readArchive(unsigned long& data);
	ModSize					readArchive(ModUInt64& data);
    ModSize 				readArchive(float& data);
    ModSize 				readArchive(double& data);
												// 基本型を読み出す
#if MOD_CONF_BOOL_TYPE == 1
	ModSize					writeArchive(bool data);
#endif
    ModSize 				writeArchive(char data);
    ModSize 				writeArchive(short data);
    ModSize 				writeArchive(int data);
    ModSize 				writeArchive(long data);
	ModSize					writeArchive(ModInt64 data);
    ModSize 				writeArchive(unsigned char data);
    ModSize 				writeArchive(unsigned short data);
    ModSize 				writeArchive(unsigned int data);
    ModSize 				writeArchive(unsigned long data);
	ModSize					writeArchive(ModUInt64 data);
    ModSize 				writeArchive(float data);
    ModSize 				writeArchive(double data);
												// 基本型を読み出す

    ModSize 				readArchive(char* data, ModSize n);
    ModSize 				readArchive(short* data, ModSize n);
    ModSize 				readArchive(int* data, ModSize n);
    ModSize 				readArchive(long* data, ModSize n);
    ModSize					readArchive(ModInt64* data, ModSize n);
    ModSize					readArchive(unsigned char* data, ModSize n);
    ModSize					readArchive(unsigned short* data, ModSize n);
    ModSize					readArchive(unsigned int* data, ModSize n);
    ModSize					readArchive(unsigned long* data, ModSize n);
    ModSize					readArchive(ModUInt64* data, ModSize n);
    ModSize					readArchive(float* data, ModSize n);
    ModSize					readArchive(double* data, ModSize n);
												// 基本型の配列を読み出す
    ModSize 				writeArchive(const char* data);
    ModSize 				writeArchive(const char* data, ModSize n);
    ModSize 				writeArchive(const short* data, ModSize n);
    ModSize 				writeArchive(const int* data, ModSize n);
    ModSize 				writeArchive(const long* data, ModSize n);
    ModSize					writeArchive(const ModInt64* data, ModSize n);
    ModSize					writeArchive(const unsigned char* data, ModSize n);
    ModSize					writeArchive(const unsigned short* data,
										 ModSize n);
    ModSize					writeArchive(const unsigned int* data, ModSize n);
    ModSize					writeArchive(const unsigned long* data, ModSize n);
    ModSize					writeArchive(const ModUInt64* data, ModSize n);
    ModSize 				writeArchive(const float* data, ModSize n);
    ModSize 				writeArchive(const double* data, ModSize n);
												// 基本型の配列を書き出す

    ModSize					readCompressedArchive(void* address, ModSize size);
												// 自由記憶領域へ
												// 圧縮データを読み出す
    ModSize					writeCompressedArchive(void* address,
												   ModSize size);
												// 自由記憶領域から
												// 圧縮データを書き出す

    void 					closeArchive();		// アーカイブをクローズする

	ModCommonDLL
    ModSize					readIOArchive(void* buffer, ModSize size,
										  ModSerialIO::DataType type);
												// シリアル化可能な
												// 入出力先から読み出す
	ModCommonDLL
    ModSize					writeIOArchive(void* buffer, ModSize size,
										   ModSerialIO::DataType type);
												// シリアル化可能な
												// 入出力先から読み出す
	ModCommonDLL
	ModSize					dummyIOArchive(void* buffer, ModSize size,
										   ModSerialIO::DataType type);
												// 例外発生関数

	ModCommonDLL
    void 					operator ()(ModSerializer& serializer);
    void					operator ()(void* address, ModSize size);
    void					operator ()(ModStatus& data);
    void					operator ()(ModBoolean& data);
#if MOD_CONF_BOOL_TYPE == 1
	ModCommonDLL
	void					operator ()(bool& data);
#endif
    void					operator ()(char& data);
    void 					operator ()(short& data);
    void 					operator ()(int& data);
    void 					operator ()(long& data);
    void 					operator ()(ModInt64& data);
    void 					operator ()(unsigned char& data);
    void 					operator ()(unsigned short& data);
    void 					operator ()(unsigned int& data);
    void 					operator ()(unsigned long& data);
    void 					operator ()(ModUInt64& data);
    void 					operator ()(float& data);
    void 					operator ()(double& data);
    void 					operator ()(char* data, ModSize n);
    void 					operator ()(short* data, ModSize n);
    void 					operator ()(int* data, ModSize n);
    void 					operator ()(long* data, ModSize n);
    void 					operator ()(ModInt64* data, ModSize n);
    void					operator ()(unsigned char* data, ModSize n);
    void					operator ()(unsigned short* data, ModSize n);
    void					operator ()(unsigned int* data, ModSize n);
    void					operator ()(unsigned long* data, ModSize n);
    void					operator ()(ModUInt64* data, ModSize n);
    void					operator ()(float* data, ModSize n);
    void					operator ()(double* data, ModSize n);
												// () 演算子
    ModArchive&				operator >>(ModStatus& data);
    ModArchive&				operator >>(ModBoolean& data);
#if MOD_CONF_BOOL_TYPE == 1
	ModArchive&				operator >>(bool& data);
#endif
    ModArchive&				operator >>(char& data);
    ModArchive& 			operator >>(short& data);
    ModArchive& 			operator >>(int& data);
    ModArchive& 			operator >>(long& data);
    ModArchive& 			operator >>(ModInt64& data);
    ModArchive& 			operator >>(unsigned char& data);
    ModArchive& 			operator >>(unsigned short& data);
    ModArchive& 			operator >>(unsigned int& data);
    ModArchive& 			operator >>(unsigned long& data);
    ModArchive& 			operator >>(ModUInt64& data);
    ModArchive& 			operator >>(float& data);
    ModArchive& 			operator >>(double& data);
												// >> 演算子
    ModArchive&				operator <<(ModStatus& data);
    ModArchive&				operator <<(ModBoolean& data);
#if MOD_CONF_BOOL_TYPE == 1
	ModArchive&				operator <<(bool& data);
#endif
    ModArchive&				operator <<(char& data);
    ModArchive& 			operator <<(short& data);
    ModArchive& 			operator <<(int& data);
    ModArchive& 			operator <<(long& data);
    ModArchive& 			operator <<(ModInt64& data);
    ModArchive& 			operator <<(unsigned char& data);
    ModArchive& 			operator <<(unsigned short& data);
    ModArchive& 			operator <<(unsigned int& data);
    ModArchive& 			operator <<(unsigned long& data);
    ModArchive& 			operator <<(ModUInt64& data);
    ModArchive& 			operator <<(float& data);
    ModArchive& 			operator <<(double& data);
												// << 演算子
private:
	ModCommonDLL
	static funcIO			_funcTable[ModeNum];
												// 入出力関数テーブル

    Mode					_mode;				// アーカイブモード
    ModSerialIO&			_io;				// シリアル化可能な入出力先
    ModSize					_size;				// アーカイブサイズ(B 単位)
};

//	FUNCTION public
//	ModArchive::~ModArchive -- アーカイブを表すクラスのデストラクター
//
//	NOTES
//		今のところ、なにもしない
//		仮想デストラクターにするために定義している
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし 
//
//	EXCEPTIONS
//		なし

inline
ModArchive::~ModArchive()
{ }

//	FUNCTION public
//	ModArchive::isStore -- 書き出し専用のアーカイブか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			書き出し専用のアーカイブである
//		ModFalse
//			書き出し専用のアーカイブでない
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModArchive::isStore() const
{ 
	return (this->getMode() == ModeStoreArchive) ? ModTrue : ModFalse; 
}

//	FUNCTION public
//	ModArchive::isLoad -- 読み込み専用のアーカイブか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			読み込み専用のアーカイブである
//		ModFalse
//			読み込み専用のアーカイブでない
//
//	EXCEPTIONS
//		なし

inline
ModBoolean
ModArchive::isLoad() const
{ 
    return (this->getMode() == ModeLoadArchive) ? ModTrue : ModFalse; 
}

//	FUNCTION public
//	ModArchive::getMode -- アーカイブモードを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたアーカイブモード
//
//	EXCEPTIONS
//		なし

inline
ModArchive::Mode
ModArchive::getMode() const
{ 
    return _mode; 
}

//	FUNCTION public
//	ModArchive::setMode -- アーカイブモードを設定する
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive::Mode	mode
//			設定するアーカイブモード
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModArchive::setMode(Mode mode) 
{ 
    _mode = mode; 
}

//	FUNCTION public
//	ModArchive::resetSize -- アーカイブサイズの初期化
//
//	NOTES
//		アーカイブサイズを 0 に戻す
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModArchive::resetSize() 
{ 
    _size = 0; 
}

//	FUNCTION public
//	ModArchive::getSize -- アーカイブサイズを得る
//
//	NOTES
//		アーカイブが生成されてから、
//		もしくは最後にアーカイブサイズが初期化されてから、
//		アーカイブへ(もしくはアーカイブから)シリアル化された
//		データの総サイズを得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたアーカイブサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

inline
ModSize
ModArchive::getSize() const
{ 
    return _size; 
}

//	FUNCTION public
//	ModArchive::getCompressSize -- 圧縮アーカイブサイズを得る
//
//	NOTES
//		アーカイブが生成されてから、
//		もしくは最後に圧縮アーカイブサイズが初期化されてから、
//		アーカイブへ(もしくはアーカイブから)シリアル化され、
//		圧縮されたデータの総サイズを得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた圧縮アーカイブサイズ(B 単位)
//
//	EXCEPTIONS
//		ModOsErrorNotSetCodec
//			圧縮しないシリアル化入出力先を使用している
//			(ModSerialIO::getCompressSize より)

inline
ModSize
ModArchive::getCompressSize() 
{ 
    return _io.getCompressSize(); 
}

//	FUNCTION public
//	ModArchive::getHeadAddress -- アーカイブの先頭アドレスを得る
//
//	NOTES
//		シリアル化入出力先が ModMemory などの自由記憶領域のとき、
//		アーカイブの先頭アドレスを得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		0 以外の値
//			アーカイブの先頭アドレス
//		0
//			シリアル化入出力先が自由記憶領域でない
//
//	EXCEPTIONS

inline
void*	
ModArchive::getHeadAddress()
{
	return _io.getHeadAddress();
}

//	FUNCTION public
//	ModArchive::getCurrentAddress -- アーカイブの現在のアドレスを得る
//
//	NOTES
//		シリアル化入出力先が ModMemory などの自由記憶領域のとき、
//		アーカイブの入出力を開始するアドレスを得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		0 以外の値
//			アーカイブの現在のアドレス
//		0
//			シリアル化入出力先が自由記憶領域でない
//
// EXCEPTIONS

inline
void*	
ModArchive::getCurrentAddress()
{
	return _io.getCurrentAddress();
}

//	FUNCTION public
//	ModArchive::getBufferSize -- シリアル化可能な最大サイズを得る
//
//	NOTES
//		シリアル化入出力先が ModMemory などの自由記憶領域のとき、
//		シリアル化可能な最大サイズを得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		0 以外の値
//			シリアル化可能な最大サイズ(B 単位)
//		0
//			シリアル化できない
//			または、シリアル化入力先が自由記憶領域でない
//
// EXCEPTIONS

inline
ModSize
ModArchive::getBufferSize()
{
	int	size = _io.getSerialSize();
	return (size < 0) ? 0 : size;
}

//	FUNCTION public
//	ModArchive::getCurrentPosition -- アーカイブの現在のオフセットを得る
//
//	NOTES
//		アーカイブの入出力を開始する位置の
//		アーカイブの先頭からのオフセットを得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		アーカイブの現在のオフセット(B 単位)
//
//	EXCEPTIONS

inline
ModFileOffset
ModArchive::getCurrentPosition()
{
	return _io.getCurrentPosition();
}

//	FUNCTION public
//	ModArchive::seek -- アーカイブの入出力開始位置の移動
//
//	NOTES
//
//	ARGUMENTS
//		ModFileOffset		offset
//			新しいアーカイブの入出力開始位置のオフセット(B 単位)
//		ModSerialIO::SeekWhence	whence
//			指定されたオフセットがどこからのものかを表す
//
//	RETURN
//		移動後のアーカイブの現在のオフセット(B 単位)
//
//	EXCEPTIONS

inline
ModFileOffset		
ModArchive::seek(ModFileOffset offset, ModSerialIO::SeekWhence whence)
{
	return _io.seekSerial(offset, whence);
}

//	FUNCTION public
//	ModArchive::flush -- シリアル化入出力先をフラッシュする
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive::Mode	mode
//			ModArchive::ModeStoreArchive
//				書き出しのためにフラッシュする
//			ModArchive::ModeLoadArchive
//				読み込みのためにフラッシュする
//			ModArchive::ModeUndefined または指定されないとき
//				アーカイブモードが与えられたものとする
//			その他
//				なにもしない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
ModArchive::flush(Mode mode) 
{
	switch ((mode == ModeUndefined) ? this->getMode() : mode) {
	case ModeStoreArchive:
		_io.writeFlushSerial();		break;
	case ModeLoadArchive:
		_io.readFlushSerial();		break;
	}
}

//	FUNCTION public
//	ModArchive::readArchive -- あるサイズの自由記憶領域へ読み出す
//
//	NOTES
//
//	ARGUMENTS
//		void*				address
//			読み出した内容を格納する領域の先頭アドレス
//		ModSize				size
//			領域のサイズ(B 単位)
//
//	RETURN
//		実際に読み出したサイズ(B 単位)
//
//	EXCEPTIONS
//		ModOsErrorEndOfFile
//			EOF まで読み出してしまった
//			(ModArchive::readIOArchive より)

inline
ModSize
ModArchive::readArchive(void* address, ModSize size) 
{ 
    return this->readIOArchive(address, size, ModSerialIO::dataTypeVariable);
}

//	FUNCTION public
//	ModArchive::writeArchive -- あるサイズの自由記憶領域から書き出す
//
//	NOTES
//
//	ARGUMENTS
//		void*				address
//			書き出す内容を格納する領域の先頭アドレス
//		ModSize				size
//			領域のサイズ(B 単位)
//
//	RETURN
//		実際に書き出したサイズ(B 単位)
//
//	EXCEPTIONS
//		ModOsErrorWriteNoSpace
//			EOF を超えて書き出そうとしている
//			(ModArchive::writeIOArchive より)

inline
ModSize
ModArchive::writeArchive(void* address, ModSize size) 
{ 
    return this->writeIOArchive(address, size, ModSerialIO::dataTypeVariable);
}

//	FUNCTION public
//	ModArchive::readArchive -- 基本型を読み出す
//
//	NOTES
//
//	ARGUMENTS
//		bool&				data
//			読み出した bool 型
//		char&				data
//			読み出した char 型
//		short&				data
//			読み出した short 型
//		int&				data
//			読み出した int 型
//		long&				data
//			読み出した long 型
//		ModInt64&			data
//			読み出した 64 ビット整数型
//		unsigned char&		data
//			読み出した unsigned char 型
//		unsigned short&		data
//			読み出した unsigned short 型
//		unsigned int&		data
//			読み出した unsigned int 型
//		unsigned long&		data
//			読み出した unsigned long 型
//		ModUInt64&			data
//			読み出した非負 64 ビット整数型
//		float&				data
//			読み出した float 型
//		double&				data
//			読み出した double 型
//
//	RETURN
//		実際に読み出したサイズ(B 単位)
//
//	EXCEPTIONS
//		ModOsErrorEndOfFile
//			EOF まで読み出してしまった
//			(ModArchive::readIOArchive より)

#if MOD_CONF_BOOL_TYPE == 1
inline
ModSize
ModArchive::readArchive(bool& data)
{
	//【注意】	bool 型のサイズは、コンパイラーの実装によって異なるので、
	//			true を '1'、false を '0' で表現することにして、
	//			char で読み出す

	char	tmp;
	ModSize	size = this->readArchive(tmp);
	data = (tmp == '\0') ? false : true;
	return size;
}
#endif

inline
ModSize
ModArchive::readArchive(char& data)
{ 
	return this->readIOArchive(&data, sizeof(data),
							   ModSerialIO::dataTypeCharacter);
}

inline
ModSize
ModArchive::readArchive(short& data)
{ 
	return this->readIOArchive(&data, sizeof(data),
							   ModSerialIO::dataTypeShort);
}

inline
ModSize
ModArchive::readArchive(int& data)
{
	return this->readIOArchive(&data, sizeof(data),
							   ModSerialIO::dataTypeInteger);
}

inline
ModSize
ModArchive::readArchive(long& data)
{ 
	return this->readIOArchive(&data, sizeof(data),
							   ModSerialIO::dataTypeLong);
}

inline
ModSize
ModArchive::readArchive(ModInt64& data)
{
	return this->readIOArchive(&data, sizeof(data),
							   ModSerialIO::dataTypeInt64);
}

inline
ModSize
ModArchive::readArchive(unsigned char& data) 
{ 
	return this->readIOArchive(&data, sizeof(data),
							   ModSerialIO::dataTypeCharacter);
}

inline
ModSize
ModArchive::readArchive(unsigned short& data) 
{ 
	return this->readIOArchive(&data, sizeof(data),
							   ModSerialIO::dataTypeShort);
}

inline
ModSize
ModArchive::readArchive(unsigned int& data) 
{
	return this->readIOArchive(&data, sizeof(data),
							   ModSerialIO::dataTypeInteger);
}

inline
ModSize
ModArchive::readArchive(unsigned long& data) 
{
	return this->readIOArchive(&data, sizeof(data),
							   ModSerialIO::dataTypeLong);
}

inline
ModSize
ModArchive::readArchive(ModUInt64& data)
{
	return this->readIOArchive(&data, sizeof(data),
							   ModSerialIO::dataTypeInt64);
}

inline
ModSize
ModArchive::readArchive(float& data) 
{
	return this->readIOArchive(&data, sizeof(data),
							   ModSerialIO::dataTypeFloat); 
}

inline
ModSize
ModArchive::readArchive(double& data) 
{
	return this->readIOArchive(&data, sizeof(data),
							   ModSerialIO::dataTypeDouble);
}

//	FUNCTION public
//	ModArchive::writeArchive -- 基本型を書き出す
//
//	NOTES
//
//	ARGUMENTS
//		bool&				data
//			書き出した bool 型
//		char&				data
//			書き出した char 型
//		short&				data
//			書き出した short 型
//		int&				data
//			書き出した int 型
//		long&				data
//			書き出した long 型
//		ModInt64&			data
//			書き出した 64 ビット整数型
//		unsigned char&		data
//			書き出した unsigned char 型
//		unsigned short&		data
//			書き出した unsigned short 型
//		unsigned int&		data
//			書き出した unsigned int 型
//		unsigned long&			data
//			書き出した unsigned long 型
//		ModUInt64&			data
//			書き出した 非負 64 ビット整数型
//		float&				data
//			書き出した float 型
//		double&				data
//			書き出した double 型
//
//	RETURN
//		実際に書き出したサイズ(B 単位)
//
//	EXCEPTIONS
//		ModOsErrorWriteNoSpace
//			EOF を超えて書き出そうとしている
//			(ModArchive::writeIOArchive より)

#if MOD_CONF_BOOL_TYPE == 1
inline
ModSize
ModArchive::writeArchive(bool data)
{
	//【注意】	bool 型のサイズは、コンパイラーの実装によって異なるので、
	//			true を '\1'、false を '\0' で表現することにして、
	//			char で書き込む

	char	tmp = (data) ? '\1' : '\0';
	return this->writeArchive(tmp);
}
#endif

inline
ModSize
ModArchive::writeArchive(char data) 
{
	return this->writeIOArchive(&data, sizeof(data),
								ModSerialIO::dataTypeCharacter);
}

inline
ModSize
ModArchive::writeArchive(short data) 
{	
	return this->writeIOArchive(&data, sizeof(data),
								ModSerialIO::dataTypeShort);
}

inline
ModSize
ModArchive::writeArchive(int data)
{
	return this->writeIOArchive(&data, sizeof(data),
								ModSerialIO::dataTypeInteger);
}

inline
ModSize
ModArchive::writeArchive(long data) 
{
	return this->writeIOArchive(&data, sizeof(data),
								ModSerialIO::dataTypeLong);
}

inline
ModSize 
ModArchive::writeArchive(ModInt64 data)
{
	return this->writeIOArchive(&data, sizeof(data),
								ModSerialIO::dataTypeInt64);
}

inline
ModSize
ModArchive::writeArchive(unsigned char data)
{	
	return this->writeIOArchive(&data, sizeof(data),
								ModSerialIO::dataTypeCharacter);
}

inline
ModSize
ModArchive::writeArchive(unsigned short data) 
{
	return this->writeIOArchive(&data, sizeof(data),
								ModSerialIO::dataTypeShort);
}

inline
ModSize
ModArchive::writeArchive(unsigned int data) 
{
	return this->writeIOArchive(&data, sizeof(data),
								ModSerialIO::dataTypeInteger); 
}

inline
ModSize
ModArchive::writeArchive(unsigned long data) 
{
	return this->writeIOArchive(&data, sizeof(data),
								ModSerialIO::dataTypeLong);
}

inline
ModSize 
ModArchive::writeArchive(ModUInt64 data)
{
	return this->writeIOArchive(&data, sizeof(data),
								ModSerialIO::dataTypeInt64);
}

inline
ModSize
ModArchive::writeArchive(float data) 
{
	return this->writeIOArchive(&data, sizeof(data),
								ModSerialIO::dataTypeFloat);
}

inline
ModSize
ModArchive::writeArchive(double data) 
{
	return this->writeIOArchive(&data, sizeof(data),
								ModSerialIO::dataTypeDouble);
}

//	FUNCTION public
//	ModArchive::readArchive -- 基本型の配列を読み出す
//
//	NOTES
//
//	ARGUMENTS
//		char*				data
//			読み出した char 型の配列
//		short*				data
//			読み出した short 型の配列
//		int*				data
//			読み出した int 型の配列
//		long*				data
//			読み出した long 整数型の配列
//		ModInt64*			data
//			読み出した 64 ビット整数型の配列
//		unsigned char*		data
//			読み出した unsigned char 型の配列
//		unsigned short*		data
//			読み出した unsigned short 型の配列
//		unsigned int*		data
//			読み出した unsigned int 型の配列
//		unsigned long*		data
//			読み出した unsigned long 型の配列
//		ModUInt64*			data
//			読み出した非負 64 ビット整数型の配列
//		float*				data
//			読み出した float 型の配列
//		double*				data
//			読み出した double 型の配列
//		ModSize				n
//			読み出す配列の長さ
//
//	RETURN
//		実際に読み出したサイズ(B 単位)
//
//	EXCEPTIONS
//		ModOsErrorEndOfFile
//			EOF まで読み出してしまった
//			(ModArchive::readIOArchive より)

inline
ModSize
ModArchive::readArchive(char* data, ModSize n) 
{
	return this->readIOArchive(data, sizeof(char) * n,
							   ModSerialIO::dataTypeCharacterArray);
}

inline
ModSize
ModArchive::readArchive(short* data, ModSize n) 
{
	return this->readIOArchive(data, sizeof(short) * n,
							   ModSerialIO::dataTypeShortArray);
}

inline
ModSize
ModArchive::readArchive(int* data, ModSize n) 
{
	return this->readIOArchive(data, sizeof(int) * n,
							   ModSerialIO::dataTypeIntegerArray);
}

inline
ModSize
ModArchive::readArchive(long* data, ModSize n) 
{
	return this->readIOArchive(data, sizeof(long) * n,
							   ModSerialIO::dataTypeLongArray);
}

inline
ModSize
ModArchive::readArchive(ModInt64* data, ModSize n) 
{
	return this->readIOArchive(data, sizeof(ModInt64) * n,
							   ModSerialIO::dataTypeInt64Array);
}

inline
ModSize
ModArchive::readArchive(unsigned char* data, ModSize n) 
{
	return this->readArchive((char*) data, n);
}

inline
ModSize
ModArchive::readArchive(unsigned short* data, ModSize n) 
{
	return this->readArchive((short*) data, n);
}

inline
ModSize
ModArchive::readArchive(unsigned int* data, ModSize n) 
{
	return this->readArchive((int*) data, n);
}

inline
ModSize
ModArchive::readArchive(unsigned long* data, ModSize n) 
{	
	return this->readArchive((long*) data, n);
}

inline
ModSize 
ModArchive::readArchive(ModUInt64* data, ModSize n) 
{
	return this->readArchive((ModInt64*) data, n);
}

inline
ModSize
ModArchive::readArchive(float* data, ModSize n) 
{
	return this->readIOArchive(data, sizeof(float) * n,
							   ModSerialIO::dataTypeFloatArray);
}

inline
ModSize
ModArchive::readArchive(double* data, ModSize n) 
{
	return this->readIOArchive(data, sizeof(double) * n,
							   ModSerialIO::dataTypeDoubleArray);
}

//	FUNCTION public
//	ModArchive::writeArchive -- 基本型の配列を書き出す
//
//	NOTES
//
//	ARGUMENTS
//		char*				data
//			書き出した char 型の配列
//		short*				data
//			書き出した short 型の配列
//		int*				data
//			書き出した int 型の配列
//		long*				data
//			書き出した long 型の配列
//		ModInt64*			data
//			書き出した 64 ビット整数型の配列
//		unsigned char*		data
//			書き出した unsigned char 型の配列
//		unsigned short*		data
//			書き出した unsigned short 型の配列
//		unsigned int*		data
//			書き出した unsigned int 型の配列
//		unsigned long*		data
//			書き出した unsigned long 型の配列
//		ModUInt64*			data
//			書き出した 非負 64 ビット整数型の配列
//		float*				data
//			書き出した float 型の配列
//		double*				data
//			書き出した double 型の配列
//		ModSize				n
//			書き出す配列の長さ
//
//	RETURN
//		実際に書き出したサイズ(B 単位)
//
//	EXCEPTIONS
//		ModOsErrorWriteNoSpace
//			EOF を超えて書き出そうとしている
//			(ModArchive::writeIOArchive より)

inline
ModSize
ModArchive::writeArchive(const char* data)
{
	return this->writeIOArchive(
		const_cast<char*>(data),
		sizeof(char) * ModCharTrait::length(data),
		ModSerialIO::dataTypeCharacterArray);
}

inline
ModSize
ModArchive::writeArchive(const char* data, ModSize n) 
{
	return this->writeIOArchive(
		const_cast<char*>(data),
		sizeof(char) * n, ModSerialIO::dataTypeCharacterArray);
}

inline
ModSize
ModArchive::writeArchive(const short* data, ModSize n) 
{
	return this->writeIOArchive(
		const_cast<short*>(data),
		sizeof(short) * n, ModSerialIO::dataTypeShortArray);
}

inline
ModSize
ModArchive::writeArchive(const int* data, ModSize n) 
{
	return this->writeIOArchive(
		const_cast<int*>(data),
		sizeof(int) * n, ModSerialIO::dataTypeIntegerArray);
}

inline
ModSize
ModArchive::writeArchive(const long* data, ModSize n) 
{
	return this->writeIOArchive(
		const_cast<long*>(data),
		sizeof(long) * n, ModSerialIO::dataTypeLongArray);
}

inline
ModSize
ModArchive::writeArchive(const ModInt64* data, ModSize n) 
{
	return this->writeIOArchive(
		const_cast<ModInt64*>(data),
		sizeof(ModInt64) * n, ModSerialIO::dataTypeInt64Array);
}

inline
ModSize
ModArchive::writeArchive(const unsigned char* data, ModSize n)
{
	return this->writeArchive((const char*) data, n);
}

inline
ModSize
ModArchive::writeArchive(const unsigned short* data, ModSize n)
{
	return this->writeArchive((const short*) data, n);
}

inline
ModSize
ModArchive::writeArchive(const unsigned int* data, ModSize n)
{
	return this->writeArchive((const int*) data, n);
}

inline
ModSize
ModArchive::writeArchive(const unsigned long* data, ModSize n)
{
	return this->writeArchive((const long*) data, n);
}

inline
ModSize 
ModArchive::writeArchive(const ModUInt64* data, ModSize n) 
{
	return this->writeArchive((const ModInt64*) data, n);
}

inline
ModSize
ModArchive::writeArchive(const float* data, ModSize n) 
{
	return this->writeIOArchive(
		const_cast<float*>(data),
		sizeof(float) * n, ModSerialIO::dataTypeFloatArray);
}

inline
ModSize
ModArchive::writeArchive(const double* data, ModSize n) 
{
	return this->writeIOArchive(
		const_cast<double*>(data),
		sizeof(double) * n,	ModSerialIO::dataTypeDoubleArray);
}

//	FUNCTION public
//	ModArchive::readCompressedArchive --
//		あるサイズの自由記憶領域へ圧縮データを読み出す
//
//	NOTES
//
//	ARGUMENTS
//		void*				address
//			読み出した内容を格納する領域の先頭アドレス
//		ModSize				size
//			領域のサイズ(B 単位)
//
//	RETURN
//		実際に読み出したサイズ(B 単位)
//
//	EXCEPTIONS
//		ModOsErrorEndOfFile
//			EOF まで読み出してしまった
//			(ModArchive::readIOArchive より)

inline
ModSize
ModArchive::readCompressedArchive(void* address, ModSize size) 
{ 
    return this->readIOArchive(address, size,
							   ModSerialIO::dataTypeCompressedVariable);
}

//	FUNCTION public
//	ModArchive::writeCompressedArchive --
//		あるサイズの自由記憶領域から圧縮データを書き出す
//
//	NOTES
//
//	ARGUMENTS
//		void*				address
//			書き出す内容を格納する領域の先頭アドレス
//		ModSize				size
//			領域のサイズ(B 単位)
//
//	RETURN
//		実際に書き出したサイズ(B 単位)
//
//	EXCEPTIONS
//		ModOsErrorWriteNoSpace
//			EOF を超えて書き出そうとしている
//			(ModArchive::writeIOArchive より)

inline
ModSize
ModArchive::writeCompressedArchive(void* address, ModSize size) 
{
    return this->writeIOArchive(address, size,
								ModSerialIO::dataTypeCompressedVariable); 
}

//	FUNCTION public
//	ModArchive::closeArchive -- アーカイブをクローズする
//
//	NOTES
//		今のところ、なにもしない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModArchive::closeArchive()
{ 
    return;
}

//	FUNCTION public
//	ModArchive::operator () -- あるサイズの自由記憶領域を引数とする () 演算子
//
//	NOTES
//
//	ARGUMENTS
//		void*				address
//			読み出した内容、または書き出す内容を格納する領域の先頭アドレス
//		ModSize				size
//			領域のサイズ(B 単位)
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

inline
void
ModArchive::operator ()(void* address, ModSize size)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(address, size, ModSerialIO::dataTypeVariable);
}

//	FUNCTION public
//	ModArchive::operator () -- 基本型を引数とする () 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModStatus&			data
//			読み出した、または書き出した状態を表す型
//		ModBoolean&			data
//			読み出した、または書き出した真偽を表す型
//		char&				data
//			読み出した、または書き出した char 型
//		short&				data
//			読み出した、または書き出した short 型
//		int&				data
//			読み出した、または書き出した int 型
//		long&				data
//			読み出した、または書き出した long 型
//		ModInt64&			data
//			読み出した、または書き出した 64 ビット整数型
//		unsigned char&		data
//			読み出した、または書き出した unsigned char 型
//		unsigned short&		data
//			読み出した、または書き出した unsigned short 型
//		unsigned int&		data
//			読み出した、または書き出した unsigned int 型
//		unsigned long&		data
//			読み出した、または書き出した unsigned long 型
//		ModUInt64&			data
//			読み出した、または書き出した 非負 64 ビット整数型
//		float&				data
//			読み出した、または書き出した float 型
//		double&				data
//			読み出した、または書き出した double 型
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

inline
void
ModArchive::operator ()(ModStatus& data)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(&data, sizeof(int), ModSerialIO::dataTypeInteger);
}

inline
void
ModArchive::operator ()(ModBoolean& data)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(&data, sizeof(int), ModSerialIO::dataTypeInteger);
}

inline
void
ModArchive::operator ()(char& data)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(&data, sizeof(data), ModSerialIO::dataTypeCharacter);
}

inline
void 
ModArchive::operator ()(short& data)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(&data, sizeof(data), ModSerialIO::dataTypeShort);
}

inline
void 
ModArchive::operator ()(int& data)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(&data, sizeof(data), ModSerialIO::dataTypeInteger);
}

inline
void 
ModArchive::operator ()(long& data)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(&data, sizeof(data), ModSerialIO::dataTypeLong);
}

inline
void
ModArchive::operator ()(ModInt64& data)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(&data, sizeof(data), ModSerialIO::dataTypeInt64);
}

inline
void 
ModArchive::operator ()(unsigned char& data)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(&data, sizeof(data), ModSerialIO::dataTypeCharacter);
}

inline
void 
ModArchive::operator ()(unsigned short& data)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(&data, sizeof(data), ModSerialIO::dataTypeShort);
}

inline
void 
ModArchive::operator ()(unsigned int& data)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(&data, sizeof(data), ModSerialIO::dataTypeInteger);
}

inline
void 
ModArchive::operator ()(unsigned long& data)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(&data, sizeof(data), ModSerialIO::dataTypeLong);
}

inline
void 
ModArchive::operator ()(ModUInt64& data)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(&data, sizeof(data), ModSerialIO::dataTypeInt64);
}

inline
void 
ModArchive::operator ()(float& data)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(&data, sizeof(data), ModSerialIO::dataTypeFloat);
}

inline
void 
ModArchive::operator ()(double& data)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(&data, sizeof(data), ModSerialIO::dataTypeDouble);
}

//	FUNCTION public
//	ModArchive::operator () -- 基本型の配列を引数とする () 演算子
//
//	NOTES
//
//	ARGUMENTS
//		char*				data
//			読み出した、または書き出した char 型の配列
//		short*				data
//			読み出した、または書き出した short 型の配列
//		int*				data
//			読み出した、または書き出した int 型の配列
//		long*				data
//			読み出した、または書き出した long 型の配列
//		ModInt64*			data
//			読み出した、または書き出した 64 ビット整数型の配列
//		unsigned char*		data
//			読み出した、または書き出した unsigned char 型の配列
//		unsigned short*		data
//			読み出した、または書き出した unsigned short 型の配列
//		unsigned int*		data
//			読み出した、または書き出した unsigned int 型の配列
//		unsigned long*		data
//			読み出した、または書き出した unsigned long 型の配列
//		ModUInt64*			data
//			読み出した、または書き出した 非負 64 ビット整数型の配列
//		float*				data
//			読み出した、または書き出した float 型の配列
//		double*				data
//			読み出した、または書き出した double 型の配列
//		ModSize				n
//			読み出せる、または書き出す配列の長さ
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

inline
void
ModArchive::operator ()(char* data, ModSize n)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(data, sizeof(char) * n, ModSerialIO::dataTypeCharacterArray);
}

inline
void
ModArchive::operator ()(short* data, ModSize n)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(data, sizeof(short) * n, ModSerialIO::dataTypeShortArray);
}

inline
void
ModArchive::operator ()(int* data, ModSize n)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(data, sizeof(int) * n, ModSerialIO::dataTypeIntegerArray);
}

inline
void
ModArchive::operator ()(long* data, ModSize n)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(data, sizeof(long) * n, ModSerialIO::dataTypeLongArray);
}

inline
void
ModArchive::operator ()(ModInt64* data, ModSize n)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(data, sizeof(ModInt64) * n, ModSerialIO::dataTypeInt64Array);
}

inline
void
ModArchive::operator ()(unsigned char* data, ModSize n)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(data, sizeof(unsigned char) * n, ModSerialIO::dataTypeCharacterArray);
}

inline
void
ModArchive::operator ()(unsigned short* data, ModSize n)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(data, sizeof(unsigned short) * n, ModSerialIO::dataTypeShortArray);
}

inline
void
ModArchive::operator ()(unsigned int* data, ModSize n)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(data, sizeof(unsigned int) * n, ModSerialIO::dataTypeIntegerArray);
}

inline
void
ModArchive::operator ()(unsigned long* data, ModSize n)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(data, sizeof(unsigned long) * n, ModSerialIO::dataTypeLongArray);
}

inline
void
ModArchive::operator ()(ModUInt64* data, ModSize n)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(data, sizeof(ModUInt64) * n, ModSerialIO::dataTypeInt64Array);
}

inline
void
ModArchive::operator ()(float* data, ModSize n)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(data, sizeof(float) * n, ModSerialIO::dataTypeFloatArray);
}

inline
void
ModArchive::operator ()(double* data, ModSize n)
{
	(void) (this->*ModArchive::_funcTable[this->getMode()])
		(data, sizeof(double) * n, ModSerialIO::dataTypeDoubleArray);
}

//	FUNCTION public
//	ModArchive::operator >> -- 基本型を引数とする >> 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModStatus&			data
//			読み出した状態を表す型
//		ModBoolean&			data
//			読み出した真偽を表す型
//		bool&				data
//			読み出した bool 型
//		char&				data
//			読み出した char 型
//		short&				data
//			読み出した short 型
//		int&				data
//			読み出した int 型
//		long&				data
//			読み出した long 型
//		ModInt64&			data
//			読み出した 64 ビット整数型
//		unsigned char&		data
//			読み出した unsigned char 型
//		unsigned short&		data
//			読み出した unsigned short 型
//		unsigned int&		data
//			読み出した unsigned int 型
//		unsigned long&		data
//			読み出した unsigned long 型
//		ModUInt64&			data
//			読み出した非負 64 ビット整数型
//		float&				data
//			読み出した float 型
//		double&				data
//			読み出した double 型
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorEndOfFile
//			EOF まで読み出してしまった
//			(ModArchive::readIOArchive より)

inline
ModArchive& 
ModArchive::operator >>(ModStatus& data) 
{ 
	(void) this->readArchive((int&) data); return *this; 
}

inline
ModArchive& 
ModArchive::operator >>(ModBoolean& data) 
{ 
	(void) this->readArchive((int&) data); return *this; 
}

#if MOD_CONF_BOOL_TYPE == 1
inline
ModArchive&
ModArchive::operator >>(bool& data)
{
	(void) this->readArchive(data); return *this;
}
#endif

inline
ModArchive& 
ModArchive::operator >>(char& data) 
{ 
	(void) this->readArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator >>(short& data) 
{
	(void) this->readArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator >>(int& data) 
{ 
	(void) this->readArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator >>(long& data) 
{ 
	(void) this->readArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator >>(ModInt64& data) 
{ 
	(void) this->readArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator >>(unsigned char& data) 
{ 
	(void) this->readArchive(data); return *this; 
}

inline
ModArchive&
ModArchive::operator >>(unsigned short& data) 
{ 
	(void) this->readArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator >>(unsigned int& data) 
{ 
	(void) this->readArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator >>(unsigned long& data) 
{ 
	(void) this->readArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator >>(ModUInt64& data) 
{ 
    (void) this->readArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator >>(float& data) 
{ 
    (void) this->readArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator >>(double& data) 
{ 
    (void) this->readArchive(data); return *this; 
}

//	FUNCTION public
//	ModArchive::operator << -- 基本型を引数とする << 演算子
//
//	NOTES
//
//	ARGUMENTS
//		ModStatus&			data
//			書き出した状態を表す型
//		ModBoolean&			data
//			書き出した真偽を表す型
//		bool&				data
//			書き出した bool 型
//		char&				data
//			書き出した char 型
//		short&				data
//			書き出した short 型
//		int&				data
//			書き出した int 型
//		long&				data
//			書き出した long 型
//		ModInt64&			data
//			書き出した 64 ビット整数型
//		unsigned char&		data
//			書き出した unsigned char 型
//		unsigned short&		data
//			書き出した unsigned short 型
//		unsigned int&		data
//			書き出した unsigned int 型
//		unsigned long&		data
//			書き出した unsigned long 型
//		ModUInt64&			data
//			書き出した 非負 64 ビット整数型
//		float&				data
//			書き出した float 型
//		double&				data
//			書き出した double 型
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorWriteNoSpace
//			EOF を超えて書き出そうとしている
//			(ModArchive::writeIOArchive より)

inline
ModArchive&
ModArchive::operator <<(ModStatus& data)
{
	(void) this->writeArchive((int&) data); return *this;
}

inline
ModArchive& 
ModArchive::operator <<(ModBoolean& data) 
{ 
    (void) this->writeArchive((int&) data); return *this; 
}

#if MOD_CONF_BOOL_TYPE == 1
inline
ModArchive&
ModArchive::operator <<(bool& data)
{
	(void) this->writeArchive(data); return *this;
}
#endif

inline
ModArchive& 
ModArchive::operator <<(char& data) 
{ 
    (void) this->writeArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator <<(short& data) 
{ 
    (void) this->writeArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator <<(int& data) 
{ 
    (void) this->writeArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator <<(long& data) 
{ 
    (void) this->writeArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator <<(ModInt64& data) 
{ 
    (void) this->writeArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator <<(unsigned char& data) 
{ 
    (void) this->writeArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator <<(unsigned short& data) 
{ 
    (void) this->writeArchive(data); return *this; 
}

inline
ModArchive&
ModArchive::operator <<(unsigned int& data) 
{ 
    (void) this->writeArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator <<(unsigned long& data) 
{
    (void) this->writeArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator <<(ModUInt64& data) 
{ 
    (void) this->writeArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator <<(float& data) 
{ 
    (void) this->writeArchive(data); return *this; 
}

inline
ModArchive& 
ModArchive::operator <<(double& data) 
{ 
    (void) this->writeArchive(data); return *this; 
}

#endif	// __ModArchive_H__

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
