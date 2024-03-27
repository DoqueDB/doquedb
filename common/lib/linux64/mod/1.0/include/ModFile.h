// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4;	
//
// ModFile.h -- シリアル化可能ファイルのクラス定義
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

#ifndef	__ModFile_H__
#define __ModFile_H__

#include "ModCommonDLL.h"
#include "ModOsDriver.h"
#include "ModCharString.h"
#include "ModUnicodeString.h"
#include "ModSerialIO.h"
#include "ModCodec.h"

//
// モジュールは汎用OSに属する。
// したがって、ModOsObjectのサブクラスとして作成し、エラーはModOsXXX
// である。
//

//
// CLASS
// ModPureFile -- シリアライズ可能なファイル機能クラス
//
// NOTES
//	ファイルを表すクラスであり、アーカイバによってシリアイズしながら読み書き
//	することができる機能をもつ。
//	シリアライズ可能とするため、ModSerialIOクラスの派生クラスである。
//	実際にはメモリハンドルを明示したクラスModFileを使う。
//
//	バッファリング、圧縮伸長を行なう場合はコーデックを用いる。
// 
//	** オープンできるファイルディスクリプタの個数の制限を越えないように
//	** 利用していないときにクローズするなどの処理を行う必要が多分ある。
//	** その実装は物理ファイル担当者にまかせる。

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModPureFile
	: public	ModSerialIO
{
public:
	// ENUM
	// openMode -- オープンのモード
	//
	// NOTES
	//	ファイルのオープンモードを表す。openの引数に指定する。
	// アーカイバからのreadWriteModeの利用には注意が必要である。
	enum OpenMode {
		readMode = 0,
		writeMode,
		readWriteMode,
		appendMode
	};
	// ENUM
	// permissionMode -- アクセス権のモード
	//
	// NOTES
	//	作成されるファイルのアクセス権を表す。openの引数に指定する。
	enum PermissionMode {
		ownerReadPermission		= ModOs::ownerReadMode,
		ownerWritePermission	= ModOs::ownerWriteMode,
		ownerExecutePermission	= ModOs::ownerExecuteMode,

		groupReadPermission		= ModOs::groupReadMode,
		groupWritePermission	= ModOs::groupWriteMode,
		groupExecutePermission	= ModOs::groupExecuteMode,

		otherReadPermission		= ModOs::otherReadMode,
		otherWritePermission	= ModOs::otherWriteMode,
		otherExecutePermission	= ModOs::otherExecuteMode
	};

	// 互換性のため。
	enum
	{
		readPermission =		ownerReadPermission,
		writePermission =		ownerWritePermission,
		executePermission =		ownerExecutePermission
	};

	// ENUM
	// ControlFlag -- その他の制御モード
	//
	// NOTES
	//	作成されるファイルのその他の指定を表す。
	//	createFlagは、ファイルを作成するかどうかを表す。
	//	ファイルが存在している場合は、意味を為さない。
	//	truncateFlagはファイルが存在している場合にファイルサイズを0に設定
	//	するかどうかを表す。 上書きの場合はこれを指定する必要がある。
	//	exclusiveFlagは同時にcreateFlagが指定されている場合にファイルが存在
	//	すればエラーとするかどうかを指定する。
	//	いずれも指定した場合はそれが実行される。
	//	

	enum ControlFlag
	{
		writeThroughFlag =		ModOs::writeThroughFlag,
		createFlag =			ModOs::createFlag,
		truncateFlag =			ModOs::truncateFlag,
		exclusiveFlag =			ModOs::exclusiveFlag
	};

	//
	// File独自のメソッド
	//

	// コンストラクタ
	// インスタンス化すると自動的にオープンされ、
	//	デストラクトされると自動的にクローズされる。
	ModCommonDLL
	ModPureFile(const ModUnicodeString& path, OpenMode mode_, 
				int control_=createFlag, int permission_=0666, 
				ModCodec* codec = 0);

	ModCommonDLL
	ModPureFile(const ModCharString& path, OpenMode mode_, 
				int control_=createFlag, int permission_=0666, 
				ModCodec* codec = 0);

	virtual ~ModPureFile();						// デストラクター

	// 明示的にオープンされるまでは何もしない
	ModCommonDLL
	ModPureFile(const ModUnicodeString& path, ModCodec* codec = 0);
	ModCommonDLL
	ModPureFile(const ModCharString& path, ModCodec* codec = 0);
	// 明示的にオープン。ファイルを作成、truncateの有無は引数control_で指定
	ModCommonDLL
	void open(OpenMode mode_, int control_=createFlag, 
			  int permission_=0666, unsigned int blockSize = 1024);

	const ModUnicodeString&	getFullPathNameW() const;
	ModCommonDLL
	const ModCharString		getFullPathName() const;
												// ファイルの絶対パス名を得る

	// ファイルのシーク
	ModCommonDLL
	ModFileOffset seek(ModFileOffset offset, SeekWhence whence);

	ModFileSize				getFileSize();
	ModCommonDLL
	static ModFileSize		getFileSize(const ModUnicodeString& path);
	static ModFileSize		getFileSize(const ModCharString& path);
												// ファイルサイズを得る

	// ファイルから読みこむ
	ModCommonDLL
	int read(void* buffer, ModSize size);
	// ファイルに書き込む
	ModCommonDLL
	int write(const void* buffer, ModSize size);

	void					close();			// ファイルをクローズする

	ModBoolean				isOpened() const;	// ファイルが
												// オープンされているか調べる

	OpenMode				getMode() const;	// オープンモードを得る

	ModBoolean				isTapeDevice() const;
												// ファイルが
												// テープデバイスか調べる

	void					readFlushCodec();	// 符号化クラスのバッファへ
												// ファイルからデータを読み出す
    void					writeFlushCodec();	// 符号化クラスのバッファから
												// ファイルへデータを書き込む
    void					resetCodec();		// 符号化クラスのバッファ上の
												// 入出力位置を先頭へ戻す

	int						readSerial(void* buffer, ModSize byte, 
									   ModSerialIO::DataType type);
												// ファイルを
												// 復号化しながら読み出す
	int						writeSerial(const void* buffer, ModSize byte, 
										ModSerialIO::DataType type);
												// ファイルへ
												// 符号化しながら書き出す
	ModFileOffset			seekSerial(ModFileOffset offset, 
									   SeekWhence whence);
												// ファイルカーソルを移動する

	void					readFlushSerial();	// 符号化クラスのバッファへ
												// ファイルからデータを読み出す
    void					writeFlushSerial();	// 符号化クラスのバッファから
												// ファイルへデータを書き込む
    void					resetSerial();		// 符号化クラスのバッファ上の
												// 入出力位置を先頭へ戻す

	// 現在位置を返す
	ModCommonDLL
	ModFileOffset getCurrentPosition();
	ModCommonDLL
	int getCompressSize();

	int						rawRead(void* buffer, ModSize bytes);
	int						rawRead(void* buffer, ModSize bytes, ModSize min);
												// ファイルを読み出す
	int						rawWrite(const void* buffer, ModSize bytes);
	int						rawWrite(const void* buffer,
									 ModSize bytes, ModSize min);
												// ファイルへ書き出す

	ModCommonDLL
	static ModBoolean		doesExist(const ModUnicodeString& path);
	ModCommonDLL
	static ModBoolean		doesExist(const ModCharString& path);
												// ファイルの存在を調べる

	void					truncate(ModFileSize length);
	static void				truncate(const ModUnicodeString& path,
									 ModFileSize length);
	static void				truncate(const ModCharString& path,
									 ModFileSize length);
												// ファイルをトランケートする
	static void				unlink(const ModUnicodeString& path);
	static void				unlink(const ModCharString& path);
												// ファイルを削除する

	ModCommonDLL
	static ModUnicodeString	getCurrentDirectoryW();
	ModCommonDLL
	static ModCharString	getCurrentDirectory();
												// カレントワーキング
												// ディレクトリーを求める

	static ModFileSize		getFileSizeLimit();	// ファイルサイズの上限値を得る

	static void 			setNumberOfFileUnlimited();
												// ファイルディスクリプターの
												// 制限数を最大にする
protected:
	// サブクラスが以下のメンバーをいじれるようにする
	ModUnicodeString pathName;
	OpenMode mode;
	int control;
	int permission;

private:
	ModOsDriver::File		_file;				// 仮想 OS のファイル
	ModCodec*				_codec;				// コーデック
};

//	FUNCTION public
//	ModPureFile::~ModPureFile --
//		シリアル化可能ファイルを表すクラスのデストラクター
//
//	NOTES
//		仮想デストラクターを定義するだけで、なにもしない
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
ModPureFile::~ModPureFile()
{ }

//	FUNCTION public
//	ModPureFile::close -- シリアル化可能ファイルをクローズする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
ModPureFile::close()
{
	_file.close();
}

//
// FUNCTION
// ModPureFile::isOpened -- ファイルがオープンされているかどうかチェックする
// 
// NOTES
//	ファイルがオープンされているか、クローズされていない状態かどうかを
// チェックし、返す。
//
// ARGUMENTS
// なし
//
// RETURN
//	オープンされていればModTrue, クローズ済みならばModFalse
//
// EXCEPTIONS
//	なし
//
inline 
ModBoolean
ModPureFile::isOpened() const
{
	return _file.isOpened();
}

//	FUNCTION
//	ModPureFile::isTapeDevice --
//		シリアル化可能ファイルの実体である OS ファイルがテープデバイスか調べる
// 
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModTrue
//			テープデバイスである
//		ModFalse
//			テープデバイスでない
//
//	EXCEPTIONS
//		なし

inline 
ModBoolean
ModPureFile::isTapeDevice() const
{
	return (this->isOpened()) ?
		_file.isTapeDevice() :
		ModOsDriver::File::isTapeDevice(this->pathName);
}

//	FUNCTION public
//	ModPureFile::rawRead -- シリアル化可能ファイルを読み出す
// 
//	NOTES
//		シリアル化可能ファイルから指定された量のデータを読み出す
//		読み出し時に復号化は行わない
//
//	ARGUMENTS
//		void*				buffer
//			読み出したデータを格納する領域の先頭アドレス
//		ModSize				byte
//			読み出すデータのサイズ(B 単位)
//		ModSize				min
//			無視される
//
//	RETURN
//		読み出したデータのサイズ(B 単位)
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			与えられた領域の先頭アドレスや読み出すサイズに誤りがある
//			(ModOsDriver::File::read より)
//		ModOsErrorBadFileDescriptor
//			オープンされていないシリアル化可能ファイルを読み出そうとしている
//			(ModOsDriver::File::read より)
//		ModOsErrorIsDirectory
//			シリアル化可能ファイルの実体である OS ファイルが
//			ディレクトリーである
//			(ModOsDriver::File::read より)

inline
int
ModPureFile::rawRead(void* buffer, ModSize bytes)
{
	return _file.read(buffer, bytes);
}

inline
int
ModPureFile::rawRead(void* buffer, ModSize bytes, ModSize min)
{
	// ModPureSocket::rawRead にあわせた定義

	return this->rawRead(buffer, bytes);
}

//	FUNCTION public
//	ModPureFile::rawWrite -- シリアル化可能ファイルへ書き出す
// 
//	NOTES
//		シリアル化可能ファイルへ指定された量のデータを書き出す
//		書き出し時に符号化は行わない
//
//	ARGUMENTS
//		void*				buffer
//			書き出すデータが格納されている領域の先頭アドレス
//		ModSize				byte
//			書き出すデータのサイズ(B 単位)
//		ModSize				min
//			無視される
//
//	RETURN
//		書き出したデータのサイズ(B 単位)
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			与えられた領域の先頭アドレスや書き出すサイズに誤りがある
//			(ModOsDriver::File::write より)
//		ModOsErrorBadFileDescriptor
//			オープンされていないシリアル化可能ファイルへ書き出そうとしている
//			(ModOsDriver::File::write より)
//		ModOsErrorNotSpace
//			シリアル化可能ファイルの存在するファイルシステムに
//			空き領域が存在しない
//			(ModOsDriver::File::write より)
//		ModOsErrorTooBigFile
//			シリアル化可能ファイルのファイルサイズの上限を超えて
//			書き出そうとしている
//			(ModOsDriver::File::write より)

inline
int
ModPureFile::rawWrite(const void* buffer, ModSize bytes)
{
	return _file.write(buffer, bytes);
}

inline
int
ModPureFile::rawWrite(const void* buffer, ModSize bytes, ModSize min)
{
	// ModPureSocket::rawWrite にあわせた定義

	return this->rawWrite(buffer, bytes);
}

//	FUNCTION public
//	ModFile::truncate -- シリアル化可能ファイルをトランケートする
//
//	NOTES
//
//	ARGUMENTS
//		ModCharString&		path
//			トランケートするシリアル化可能ファイルの実体である
//			OS ファイルの絶対パス名
//		ModFileSize			length
//			トランケート後のシリアル化可能ファイルの実体である
//			OS ファイルのファイルサイズ(B 単位)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
ModPureFile::truncate(ModFileSize length)
{
	_file.truncate(length);
}

// static
inline
void
ModPureFile::truncate(const ModUnicodeString& path, ModFileSize length)
{
	ModOsDriver::File::truncate(path, length);
}

// static
inline
void
ModPureFile::truncate(const ModCharString& path, ModFileSize length)
{
	ModOsDriver::File::truncate(path.getString(), length);
}

//	FUNCTION public
//	ModPureFile::unlink -- シリアル化可能ファイルを削除する
// 
//	NOTES
//
//	ARGUMENTS
//		ModCharString&		path
//			削除するシリアル化可能ファイルの実体である OS ファイルの絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			OS ファイルの絶対パス名に誤りがある
//			(ModOsDriver::File::unlink より)
//		ModOsErrorFileNotFound
//			OS ファイルの絶対パス名が存在しない
//			(ModOsDriver::File::unlink より)
//		ModOsErrorPermissionDenied
//			OS ファイルにアクセスか、書き込み許可が与えられていない
//			OS ファイルとその親ディレクトリーの所有者でない
//			OS ファイルがディレクトリーで、スーパーユーザーでない
//			(ModOsDriver::File::unlink より)
//		ModOsErrorTooLongFilename
//			OS ファイルの絶対パス名が長すぎる
//			(ModOsDriver::File::unlink より)

inline
void
ModPureFile::unlink(const ModUnicodeString& path)
{
	ModOsDriver::File::unlink(path);
}

inline
void
ModPureFile::unlink(const ModCharString& path)
{
	ModOsDriver::File::unlink(path.getString());
}

//	FUNCTION public
//	ModPureFile::readSerial -- シリアル化可能ファイルを復号化しながら読み出す
// 
//	NOTES
//		シリアル化可能ファイルから指定された量のデータを復号化しながら読み出す
//		指定された量だけ読み出せないときも、例外は発生しない
//
//	ARGUMENTS
//		void*				buffer
//			読み出したデータを格納する領域の先頭アドレス
//		ModSize				byte
//			読み出すデータのサイズ(B 単位)
//		ModSerialIO::DataType type
//			無視される
//
//	RETURN
//		読み出したデータのサイズ(B 単位)
//
//	EXCEPTIONS

inline
int
ModPureFile::readSerial(void* buffer, ModSize byte,	ModSerialIO::DataType type)
{
	// 復号化しながら読み出す

	return this->read(buffer, byte);
}

//	FUNCTION public
//	ModPureFile::writeSerial -- シリアル化可能ファイルへ符号化しながら書き出す
// 
//	NOTES
//		シリアル化可能ファイルへ指定された量のデータを符号化しながら書き出す
//		指定された量だけ書き出せないときも、例外は発生しない
//
//	ARGUMENTS
//		void*				buffer
//			書き出すデータが格納されている領域の先頭アドレス
//		ModSize				byte
//			書き出すデータのサイズ(B 単位)
//		ModSerialIO::DataType type
//			無視される
//
//	RETURN
//		書き出したデータのサイズ(B 単位)
//
//	EXCEPTIONS

inline
int
ModPureFile::writeSerial(const void* buffer,
						 ModSize byte, ModSerialIO::DataType type)
{
	// 符号化しながら書き出す

	return this->write(buffer, byte);
}

//	FUNCTION public
//	ModPureFile::seekSerial --
//		シリアル化可能ファイルのファイルカーソルを移動する
// 
//	NOTES
//
//	ARGUMENTS
//		ModFileOffset		offset
//			移動後のファイルカーソルのオフセット(B 単位)
//		ModPureFile::SeekWhence	whence
//			指定されたオフセットがどこからのものかを表す
//
//	RETURN
//		移動後のファイルカーソルのファイルの先頭からのオフセット(B 単位)
//
//	EXCEPTIONS
//		ModOsErrorBadArgument
//			指定されたオフセット値に誤りがある
//			(ModOsDriver::File::seek より)
//		ModOsErrorBadFileDescriptor
//			オープンされていないシリアル化可能ファイルの
//			ファイルカーソルを移動しようとしている
//			(ModOsDriver::File::seek より)

inline
ModFileOffset
ModPureFile::seekSerial(ModFileOffset offset, SeekWhence whence)
{
	return this->seek(offset, whence);
}

//	FUNCTION public
//	ModPureFile::readFlushSerial --
//		符号化クラスのバッファへファイルからデータを読み出す
// 
//	NOTES
//		符号化クラスのバッファにデータがなければ、
//		新たにバッファへファイルからデータを読み出す
//		このとき、バッファにファイルへ未書き込みのデータがあれば、破棄される
//		読み出し後、バッファ上の入出力の開始位置は、バッファの先頭に戻される
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
ModPureFile::readFlushSerial()
{
	if (_codec)
		_codec->decodeFlush(this);
}

//	FUNCTION public
//	ModPureFile::writeFlushSerial --
//		符号化クラスのバッファからファイルへデータを書き込む
// 
//	NOTES
//		符号化クラスのバッファに未書き込みのデータがあれば、
//		バッファからファイルへデータを書き込む
//		書き込み後、バッファ上の入出力の開始位置は、バッファの先頭へ戻される
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
ModPureFile::writeFlushSerial()
{
	if (_codec)
		_codec->encodeFlush(this);
}

//	FUNCTION public
//	ModPureFile::resetSerial -- 入出力位置を符号化クラスのバッファの先頭に戻す
// 
//	NOTES
//		符号化クラスを使用しているときのみ、
//		符号化クラスのバッファ上の入出力を開始する位置を、
//		バッファの先頭に戻す
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
ModPureFile::resetSerial()
{
	if (_codec)
		_codec->reset();
}

//	FUNCTION public
//	ModPureFile::readFlushCodec --
//		符号化クラスのバッファへファイルからデータを読み出す
// 
//	NOTES
//		ModPureFile::readFlushSerial を参照のこと
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
ModPureFile::readFlushCodec()
{
	this->readFlushSerial();
}

//	FUNCTION public
//	ModPureFile::writeFlushCodec --
//		符号化クラスのバッファからファイルへデータを書き込む
// 
//	NOTES
//		ModPureFile::writeFlushSerial を参照のこと
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
ModPureFile::writeFlushCodec()
{
	this->writeFlushSerial();
}

//	FUNCTION public
//	ModPureFile::resetCodec -- 入出力位置を符号化クラスのバッファの先頭に戻す
// 
//	NOTES
//		ModPureFile::resetSerial を参照のこと
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
ModPureFile::resetCodec()
{
	this->resetSerial();
}

//	FUNCTION public
//	ModPureFile::getFullPathName --
//		シリアル化可能ファイルの実体である OS ファイルの絶対パス名を得る
//
//	NOTES
//		日本語のファイル、ディレクトリー名でも ModCharString であつかって、
//		特に問題はない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた実体である OS ファイルの絶対パス名
//
//	EXCEPTIONS
//		なし

inline
const ModUnicodeString&
ModPureFile::getFullPathNameW() const
{ 
    return this->pathName; 
}

//	FUNCTION public
//	ModPureFile::getMode -- シリアル化可能ファイルのオープンモードを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたオープンモード
//
//	EXCEPTIONS
//		なし

inline
ModPureFile::OpenMode	
ModPureFile::getMode() const
{ 
    return this->mode; 
}

//	FUNCTION public
//	ModPureFile::getFileSize -- シリアル化可能ファイルのファイルサイズを得る
// 
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたシリアル化可能ファイルのファイルサイズ
//
//	EXCEPTIONS
//		ModOsErrorBadFileDescriptor
//			オープンされていないシリアル化可能ファイルの
//			ファイルサイズを得ようとしている
//			(ModOsDriver::File::getFileSize より)

inline
ModFileSize
ModPureFile::getFileSize()
{
	return _file.getFileSize();
}

// static
inline
ModFileSize
ModPureFile::getFileSize(const ModCharString& path)
{
	return ModOsDriver::File::getFileSize(path.getString());
}

//	FUNCTION public
//	ModFile::getFileSizeLimit --
//		シリアル化可能ファイルのファイルサイズの上限値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたシリアル化可能ファイルのファイルサイズの上限値
//
//	EXCEPTIONS

inline
ModFileSize
ModPureFile::getFileSizeLimit()
{
	return ModOsDriver::File::getFileSizeLimit();
}

//	FUNCTION public
//	ModFile::setNumberOfFileUnlimited --
//		シリアル化可能ファイルの実体である OS ファイルディスクリプターの
//		制限数を最大にする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
ModPureFile::setNumberOfFileUnlimited()
{
	ModOsDriver::File::setNumberOfFileUnlimited();
}

//
// CLASS
//	ModFile -- ModPureFileクラスのメモリハンドル明示クラス
// NOTES
//	ModPureFileクラスをデフォルトメモリハンドルの管理下のクラスとして
//	利用するためのクラスである。ユーザは通常本クラスを利用する。
//

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModFile
	: public	ModObject<ModDefaultManager>,
	  public	ModPureFile
{
public:
	// コンストラクター
	ModFile(const ModUnicodeString& path, OpenMode mode_, 
			int control_=createFlag, int permission_=0666, 
			ModCodec* codec = 0)
		: ModPureFile(path, mode_, control_, permission_, codec)
	{}
	ModFile(const ModCharString& path, OpenMode mode_, 
			int control_=createFlag, int permission_=0666, 
			ModCodec* codec = 0)
		: ModPureFile(path, mode_, control_, permission_, codec)
	{}
	ModFile(const ModUnicodeString& path, ModCodec* codec = 0)
		: ModPureFile(path, codec)
	{}
	ModFile(const ModCharString& path, ModCodec* codec = 0)
		: ModPureFile(path, codec)
	{}
	// デストラクター
	~ModFile()
	{}
};

#endif	// __ModFile_H__

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
