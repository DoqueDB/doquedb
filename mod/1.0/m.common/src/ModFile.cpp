// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4
//
// ModFile.cpp -- シリアル化可能ファイルのメソッド定義
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


#include "ModFile.h"
#include "ModOsException.h"

//
// FUNCTION public
// ModPureFile::ModPureFile -- ファイルクラスのコンストラクタ(1)
//
// NOTES
//	ファイルクラスのコンストラクタである。
//	引数で指定したパス名をフルパスとして内部に保持し、初期化する。
//	明示的にopen()が呼び出されるとオープンされる。
//	そのとき、オープンモード、アクセス権、ファイルを作成するかどうかなど
//	その他の指定を行なう。
//	バッファリング、圧縮伸長を行う場合にはコーデックを最後の引数に
//	指定する。
//
// ARGUMENTS
//	const ModUnicodeString&/ModCharString& path
//		対象のファイル名
//	ModCodec* codec = 0
//		利用するコーデック
//
// RETURN
// なし
//
// EXCEPTIONS
//	その他
//		ModOsDriver::File::getFullPathName, ModCharString::ModCharStringの例外参照。メモリエラー以外の主なものは以下に書き下す。
//	ModOsErrorPermissionDenied
//		親ディレクトリを読むアクセス権がない	(ModOsDriver::File::getcwd)
//

ModPureFile::ModPureFile(const ModUnicodeString& path, ModCodec* codec)
	: mode(readMode),
	  permission(0),
	  control(0),
	  _codec(codec)
{
	// モードとパーミッション、制御フラグは0に初期化
	// (明示的なopenでデフォルトが設定されるから)
	// 絶対パスを設定
	ModOsDriver::File::getFullPathName(path, this->pathName);

	// 与えられた符号化クラスをリセットしておく

	this->resetSerial();

	// あとは明示的にオープンされるまで何もしない。
}

ModPureFile::ModPureFile(const ModCharString& path, ModCodec* codec)
	: mode(readMode),
	  permission(0),
	  control(0),
	  _codec(codec)
{
	// モードとパーミッション、制御フラグは0に初期化
	// (明示的なopenでデフォルトが設定されるから)
	// 絶対パスを設定
	ModOsDriver::File::getFullPathName(path, this->pathName);

	// 与えられた符号化クラスをリセットしておく

	this->resetSerial();

	// あとは明示的にオープンされるまで何もしない。
}

//
// FUNCTION public
// ModPureFile::ModPureFile -- ファイルクラスのコンストラクタ(2)
//
// NOTES
//	ファイルクラスのコンストラクタである。引数で指定したパスのフルパス名、
//	オープンモード、アクセス権、コーデックが内部に設定され、
//	コンストラクト時に自動的にオープンされる。
//	オープンの動作はメソッドopenに準ずる。
//	バッファリング、圧縮伸長を行う場合にはコーデックを最後の引数に
//	指定する。デフォルトのアクセス権は0666、ファイルは存在しなければ作成するが
//	トランケートはしない。
//
//	明示的にオープンする場合には、ファイルクラスのコンストラクタ(1)と
//	メソッドopenを利用する。
//
// ARGUMENTS
//	const ModUnicodeString&/ModCharString& path
//		対象のファイル名
//	ModPureFile::OpenMode mode_
//		オープンモード。
//	int control_
//		ファイルの制御をModPureFile::ControlFlagの値のORで指定する。デフォルトはcreateFlagのみ指定されている。
//	int permission_
//		アクセス権をModPureFile::PermissionModeの値のORで指定する。デフォルトは0666
//	ModCodec* codec = 0
//		利用するコーデック
//
// RETURN
// なし
//
// EXCEPTIONS
//	その他
//	    ModOsDriver::File::getFullPathName, open, ModCharString::ModCharStringの例外参照。メモリエラー、openのエラー以外の主なものは以下に書き下す。
//	ModOsErrorPermissionDenied
//		親ディレクトリを読むアクセス権がない	(ModOsDriver::File::getcwd)
//
ModPureFile::ModPureFile(const ModUnicodeString& path, OpenMode mode_, 
				 int control_, int permission_, ModCodec* codec)
	: mode(mode_),
	  permission(permission_),
	  control(control_),
	  _codec(codec)
{
	// モードとパーミッション、制御フラグは:xxx()で設定する
	// 絶対パスを設定
	ModOsDriver::File::getFullPathName(path, this->pathName);

	if (_codec) {
		_codec->reset();
		if (_codec->getMode() == ModCodec::blockMode) {
			this->open(mode_, control_, permission_, _codec->getBufferSize());
		} else {
			this->open(mode_, control_, permission_);
		}
	} else {
		// ModPureFileのコンストラクタ(1)で設定されないのはモードとアクセス権。
		// それは以下で設定される。
		this->open(mode_, control_, permission_);
	}
}

ModPureFile::ModPureFile(const ModCharString& path, OpenMode mode_, 
				 int control_, int permission_, ModCodec* codec)
	: mode(mode_),
	  permission(permission_),
	  control(control_),
	  _codec(codec)
{
	// モードとパーミッション、制御フラグは:xxx()で設定する
	// 絶対パスを設定
	ModOsDriver::File::getFullPathName(path, this->pathName);

	if (_codec) {
		_codec->reset();
		if (_codec->getMode() == ModCodec::blockMode) {
			this->open(mode_, control_, permission_, _codec->getBufferSize());
		} else {
			this->open(mode_, control_, permission_);
		}
	} else {
		// ModPureFileのコンストラクタ(1)で設定されないのはモードとアクセス権。
		// それは以下で設定される。
		this->open(mode_, control_, permission_);
	}
}

//
// FUNCTION public
// ModPureFile::open -- ファイルの明示的なオープン
//
// NOTES
//	ファイルをオープンする。引数にオープンモードとアクセス権を指定する。
//	デフォルトのアクセス権は、0666である。
//	ファイルが存在しなかった場合に作成するかどうか、
//	ファイルをトランケートするかどうか、ファイル作成が指定されているのに
//	ファイルが存在した場合のエラーチェックについては、引数control_に
//	ModPureFile::ControlFlagの値のORで指定する。デフォルトでは、ファイルが存在
//	しなかったときに作成するフラグ(createFlag)のみ指定される。
//	readModeが指定された場合、createFlagの指定は無視される。
//	(OsDriverの場合はエラーとなる)
//
// ARGUMENTS
//	ModPureFile::OpenMode mode_
//		オープンモードをModPureFile::OpenModeの値で指定する。
//	int control_
//		ファイル制御用のその他のフラグをModPureFile::ControlFlagの値のORで指定する。
//	int permission_
//		アクセス権をModPureFile::PermissionModeの値のORで指定する。
//  unsigned int blockSize_
//		ブロックモードのコーデックを使うときに用いるブロックサイズ
//
// RETURN
//	なし
//
// EXCEPTIONS
//	ModOsErrorFileAlreadyOpened
//			既にオープンされている	
//	その他
//		ModOsDriver::File::openの例外参照、主なものは以下に書き下す
//	ModOsErrorBadArgument			(ModOsDriver::File::open)
//		引数エラー
//	ModOsErrorPermissionDenied		(ModOsDriver::File::open)
//		ファイルは存在しないが作成許可なし、トランケート指定時に書き込み許可なし、指定パスにアクセスできない、既存ファイルに指定オープンフラグの許可なし
//	ModOsErrorFileExist				(ModOsDriver::File::open)
//		createFlagかつexclusiveFlag指定時にファイルが既に存在する
//	ModOsErrorIsDirectory			(ModOsDriver::File::open)
//		writeOnlyFlagもしくはreadWriteFlag指定時に、既存のディレクトリを指定
//	ModOsErrorOpenTooManyFiles		(ModOsDriver::File::open)
//		オープン中のファイルが多すぎる
//	ModOsErrorFileNotFound			(ModOsDriver::File::open)
//		createFlagが指定されていなくてファイルが存在しない、createFlag指定時に指定パスプレフィックスが存在しない、指定パスプレフィックスがディレクトリでない
//	ModOsErrorNotSpace				(ModOsDriver::File::open)
//		ファイルシステムのInodeが不足もしくはシステムファイルテーブルが満杯
//	ModOsErrorTooLongFilename		(ModOsDriver::File::open)
//		ファイル名が長すぎる
//
void
ModPureFile::open(OpenMode mode_, int control_,
				  int permission_, unsigned int blockSize_)
{
	// すでにオープンされていたらエラー。
	if (this->isOpened() == ModTrue) {
		ModThrowOsError(ModOsErrorFileAlreadyOpened);
	}

	this->mode = mode_;
	this->permission = permission_;
	this->control = control_;

	// readOnlyFileは、createFlagが立っているとオープンできない。
	// readModeの場合は、意味のない指定を無視するようにする。
	int osControl = this->control;

	// openを呼び出す準備
	// createについては明示的に指定できるように変更した
	int osflag;
	switch (this->mode) {
	case readMode:
		osflag = ModOs::readOnlyFlag;
		osControl = 0;
		break;
	case writeMode:
		// truncate, createについては引数control_で直接指定される
		osflag = ModOs::writeOnlyFlag;
		break;
	case readWriteMode:
		// readWriteMode でトランケートしてしまっていた。
		// truncate, createについては引数control_で直接指定される。
		// readWriteModeでtruncateをたてると何も残らないので注意
		osflag = ModOs::readWriteFlag;
		break;
	case appendMode:
		// appendFlag を指定してもポインタが最後に移動するだけで、
		// 明示的にwriteOnlyFlagを指定しないと書き込み不可能になる。
		// truncateFlagがあると、ファイルポインタが0の位置にきて
		// 後ろを切ってしまう。
		osflag = ModOs::writeOnlyFlag | ModOs::appendFlag;
		break;
	}
	// control_の値はENUMでModOsと同じ値を設定してあるので、
	// そのままORすればよい。ここでのエラーチェックはしない
	// contrl_のデフォルト値は以前と同じでcreateFlagのみである。
	
	// permission はENUMで同じ値を設定してあるのでそのまま
	// ORの解析は仮想OSの方で行う

	_file.open(this->pathName, osflag | osControl, permission_, blockSize_);
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
//
const ModCharString
ModPureFile::getFullPathName() const
{ 
	ModUnicodeString	tmp = this->pathName;
	return tmp.getString(ModOs::Process::getEncodingType()); 
}

//
// FUNCTION public
// ModPureFile::getCompressSize -- 全圧縮サイズを得る
//
// NOTES
//	全圧縮サイズを得る。コーデックが設定されていない場合は例外を送出する。
//
// ARGUMENTS
//	なし
//
// RETURN
//	圧縮サイズ
//
// EXCEPTIONS
//	ModOsErrorNotSetCodec
//		コーデックが指定されていない
//	その他
//		コーデックのgetTotalCompressSizeで送出された例外参照
//

int
ModPureFile::getCompressSize() 
{ 
	if (_codec == 0)
		ModThrowOsWarning(ModOsErrorNotSetCodec);

	return (int) _codec->getTotalCompressSize();
}

//
// FUNCTION public
// ModMemory::getCurrentPosition -- ファイルでの現在位置を得る
//
// NOTES
// ファイルでの現在位置を、ファイル先頭からのオフセットで返す。
// シリアル化I/Fに従って作成されている。
//
// ARGUMENTS
//		なし
//
// RETURN
// メモリ内で現在指しているオフセット位置
//
// EXCEPTIONS
//	その他
//		ModOsDriver::File::seekの例外参照。主なものは以下に書き下す
//	ModOsErrorBadFileDescriptor		(ModOsDriver::File::seek)
//		オープンしていないファイルディスクリプタである
//

ModFileOffset
ModPureFile::getCurrentPosition()
{
	ModFileOffset	offset = _file.seek(0, ModOs::seekCurrent);

	if (_codec)
		if (_codec->getEncodedSize() > 0)
			offset += _codec->getEncodedSize();
		else if (_codec->getDecodedSize() > 0)
			offset -= _codec->getDecodedSize();

	return offset;
}

//
// FUNCTION 
// ModPureFile::read -- コーデックを通してファイルからデータを読み込む
// 
// NOTES
//	ファイルからデータを指定バイト読み込む。
//	指定されている場合はコーデックを利用する。
//
// ARGUMENTS
//	void* buffer
//		読み込み先のバッファへのポインタ
//	ModSize bytes
//		読み込むサイズ
//
// RETURN
//	実際に読み込んだサイズ
//
// EXCEPTIONS
//	その他
//		ModOsDriver::File::read、コーデックのdecode参照。デフォルトコーデックの場合で主なものは以下に書き下す
//	ModOsErrorBadFileDescriptor			(ModOsDriver::File::read)
//		ファイルディスクリプタが無効
//	ModOsErrorIsDirectory				(ModOsDriver::File::read)
//		対象がディレクトリである
//	ModCommnErrorBadArgument			(ModOsDriver::File::read)
//		引数エラー
//	ModOsErrorReadProtocolInCodec		(ModPureCodec::decodeFlush)
//		コーデックのプロトコルによるreadでエラーが起きた(Fatal)
//	ModOsErrorReadDataInCodec			(ModPureCodec::decodeFlush)
//		コーデックによるデータのreadでエラーが起きた
//

int
ModPureFile::read(void* buffer, ModSize byte)
{
	return (_codec) ?
		_codec->decode(this, buffer, byte) : this->rawRead(buffer, byte);
}

//
// FUNCTION 
// ModPureFile::write -- コーデックを通してファイルにデータを書き込む
// 
// NOTES
//	ファイルにデータを指定バイト書き込む。
//	指定されている場合はコーデックを利用する。
//
// ARGUMENTS
//	void* buffer
//		書き込むデータが格納されているバッファへのポインタ
//	ModSize bytes
//		書き込むサイズ
//
// RETURN
//	実際に書き込んだサイズ
//
// EXCEPTIONS
//	その他
//		ModOsDriver::File::write、コーデックのencode参照。デフォルトコーデックの場合で主なものは以下に書き下す
//	ModOsErrorBadFileDescriptor			(ModOsDriver::File::write)
//		ファイルディスクリプタが無効
//	ModCommnErrorBadArgument			(ModOsDriver::File::write)
//		引数エラー
//	ModOsErrorNotSpace					(ModOsDriver::File::write)
//		空き領域がなく、ファイルシステムに書き込めない
//	ModOsErrorTooBigFile				(ModOsDriver::File::write)
//		ファイルサイズの制限を超える
//	ModOsErrorWriteProtocolInCodec		(ModPureCodec::encodeFlush)
//		コーデックのプロトコルによるwriteでエラーが起きた(Fatal)
//	ModOsErrorWriteDataInCodec			(ModPureCodec::encodeFlush)
//		コーデックによるデータ部分のwriteでエラーが起きた
//

int
ModPureFile::write(const void* buffer, ModSize byte)
{
	return (_codec) ?
		_codec->encode(this, buffer, byte) : this->rawWrite(buffer, byte);
}

//
// FUNCTION
// ModPureFile::seek -- 現在位置をシークする
// 
// NOTES
//	シークして現在位置を移動させる。
//
// ARGUMENTS
//	ModFileOffset offset
//		シークするオフセット
//	ModPureFile::SeekWhence whence
//		シークのタイプを指定する。seekSetは先頭からの絶対オフセット
//
// RETURN
//	シークに成功の場合現在位置を返す。不成功の場合は例外を送出。
//
// EXCEPTIONS
//	その他
//		ModOsDriver::File::seekの例外参照、主なものは以下に書き下す
//	ModOsErrorBadFileDescriptor		(ModOsDriver::File::seek)
//		オープンしていないファイルディスクリプタである
//	ModOsErrorBadArgument			(ModOsDriver::File::seek)
//		オフセットが無効
//

ModFileOffset
ModPureFile::seek(ModFileOffset offset, SeekWhence whence)
{
	if (_codec) {
		if (whence == ModSerialIO::seekCurrent)
			if (_codec->getEncodedSize() > 0)
				offset += _codec->getEncodedSize();
			else if (_codec->getDecodedSize() > 0)
				offset -= _codec->getDecodedSize();

		_codec->reset();
	}

	static const ModOs::SeekWhence	osWhence[] =
		{ ModOs::seekSet, ModOs::seekCurrent, ModOs::seekEnd };

	return _file.seek(offset, osWhence[whence]);
}

//	FUNCTION public
//	ModPureFile::doesExist -- ファイルが存在するか調べる
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&/ModCharString&		path
//			存在するか調べるファイルのパス名
//
//	RETURN
//		ModTrue
//			ファイルは存在する
//		ModFalse
//			ファイルは存在しない
//
//	EXCEPTIONS
//		なし

// static
ModBoolean
ModPureFile::doesExist(const ModUnicodeString& path)
{
	try {
		return ModOsDriver::File::access(path, ModOs::accessFile);

	} catch (ModException) {

		// 例外が発生したとき無視して、
		// 指定されたファイルが存在しないことにする

		ModErrorHandle::reset();
		return ModFalse;
	}
}

// static
ModBoolean
ModPureFile::doesExist(const ModCharString& path)
{
	try {
		return ModOsDriver::File::access(path.getString(), ModOs::accessFile);

	} catch (ModException) {

		// 例外が発生したとき無視して、
		// 指定されたファイルが存在しないことにする

		ModErrorHandle::reset();
		return ModFalse;
	}
}

//	FUNCTION public
//	ModFile::getCurrentDirectory -- カレントワーキングディレクトリーを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたカレントワーキングディレクトリーのパス名
//
//	EXCEPTIONS
//		ModOsErrorPermissionDenied
//			カレントワーキングディレクトリーの
//			親ディレクトリーの読み出し許可がない
//			(ModOsDriver::Process::getcwd より)
//		ModOsErrorOtherReason
//			上記以外のエラーが起きた
//			(ModOsDriver::Process::getcwd より)

ModUnicodeString
ModPureFile::getCurrentDirectoryW()
{
	ModUnicodeString	buf;
	ModOsDriver::Process::getcwd(buf);
	return buf;
}

ModCharString
ModPureFile::getCurrentDirectory()
{
	char	buf[ModPathMax + 1];
	ModOsDriver::Process::getcwd(buf, sizeof(buf));
	return ModCharString(buf);
}

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
