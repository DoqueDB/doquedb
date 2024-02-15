// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4
//
// ModSocket.cpp -- シリアル化可能ソケットのメソッド定義
// 
// Copyright (c) 1997, 2011, 2023 Ricoh Company, Ltd.
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


#include "ModSocket.h"
#include "ModCommonInitialize.h"
#include "ModAutoPointer.h"
#include "ModOsException.h"

//
// FUNCTION
// ModSocketBase::ModSocketBase -- ソケット基底クラスのコンストラクタ
// 
// NOTES
//	仮想OSのソケットを作成し、ソケット基底クラスを作成する。
//	デフォルトの場合、コーデックは利用されない。したがってバッファリング
//	も行われない。バッファリングが必要な場合、圧縮、伸長が必要な場合には
//	指定する。
//
// ARGUMENTS
//	ModCodec* codec
//		コーデックを行なうオブジェクト
//
// RETURN
// なし
//
// EXCEPTIONS
//	その他
//		ModCommonInitialize::checkAndInitialize(初期化前のみ)、コーデックのresetの例外参照
//	ModOsErrorSystemMemoryExhaust		(::new)
//		システムメモリが不足
//

ModSocketBase::ModSocketBase(ModCodec* codec) 
	: _codec(codec)
{
	// 必要ならば、汎用ライブラリーを初期化する

	ModCommonInitialize::checkAndInitialize();

	// 与えられた符号化クラスをリセットしておく

	this->resetSerial();

	// 仮想 OS のソケットを生成する

	_socket = new ModOsDriver::Socket();
	; ModAssert(_socket != 0);
	_socket->open();
}

//
// FUNCTION
// ModSocketBase::close -- ソケットのクローズ
// 
// NOTES
//	ソケットをクローズする。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		ModOsDriver::Socket::closeの例外参照
//
void
ModSocketBase::close()
{
	_socket->close();
}

//
// FUNCTION
// ModSocketBase::select -- 同期をとり、指定時間待つ。
// 
// NOTES
//	指定モードで同期をとり、指定時間待つ。
//	読み、書きのどちらかのモードをModSocketBase::SynchronousModeで
//	指定する。待つ時間はミリ秒単位で指定する。
//	同期がとれた場合にはModTrue、時間が満了した場合はModFalseを返す。
//
// ARGUMENTS
//	ModSize millisecond
//		待つ時間をミリ秒単位で指定
//
// RETURN
//	同期がとれた場合はModTrue、時間が満了した場合はModFalseを返す
//
// EXCEPTIONS
//	その他
//		ModOsDriver::Socket::selectの例外参照(主なものは以下に書き下す)
//	ModCommonErrorOutOfRange
//		引数が値の範囲を超えている
//

ModBoolean
ModSocketBase::select(const ModTimeSpan& waitTime) const
{
	if (_codec && _codec->hasBufferedData())
		return ModTrue;

	return (_socket->select(0,
							(ModSize)waitTime.getTotalMilliSeconds())) ?
		ModTrue : ModFalse;
}

//
// FUNCTION
// ModSocketBase::getCompressSize -- 全圧縮サイズを得る
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
//		コーデックのgetTotalCompressSizeの例外参照
//

int  
ModSocketBase::getCompressSize() 
{ 
	if (_codec == 0)
		ModThrowOsWarning(ModOsErrorNotSetCodec);

	return (int) _codec->getTotalCompressSize();
}

// 
//
// FUNCTION
// ModSocketBase::readSerial -- ソケットからデータを読む
// 
// NOTES
//	ソケットからデータをバッファに読み込む。コーデックが設定されていれば
//	それを利用し、バイトオーダーも変換しながら必要なだけ読み進む。
//	すなわちここで htons, ntohs, htonl, ntohl を行なう。
//
// ARGUMENTS
//	void* buffer
//		読み込み先のバッファへのポインタ
//	ModSize byte
//		読み込むバイト数
//	ModSerialIO::DataType type
//		対象データのタイプ
//
// RETURN
//	実際に読みこんだサイズ
//
// EXCEPTIONS
//	その他
//		ModSocketBase::readNetworkの例外参照
//

int 
ModSocketBase::readSerial(void* buf, ModSize size, 
						  ModSerialIO::DataType type)
{
	ModSize	total = 0;

	switch(type) {
	case ModSerialIO::dataTypeShort:
	{
		unsigned short	n;
		if ((total = this->readNetwork(&n, size)) == size)
			*((unsigned short*) buf) = ModOsDriver::Socket::networkToHost(n);
		break;
	}
	case ModSerialIO::dataTypeInteger:
#if defined(__LP64__) || defined(_LP64)
#else
	case ModSerialIO::dataTypeLong:
#endif
	{
		unsigned int	n;
		if ((total = this->readNetwork(&n, size)) == size)
			*((unsigned int*) buf) = ModOsDriver::Socket::networkToHost(n);
		break;
	}
	case ModSerialIO::dataTypeFloat:
	{
		float	n;
		if ((total = this->readNetwork(&n, size)) == size)
			*((float*) buf) = ModOsDriver::Socket::networkToHost(n);
		break;
	}
	case ModSerialIO::dataTypeDouble:
	{
		double	n;
		if ((total = this->readNetwork(&n, size)) == size)
			*((double*) buf) = ModOsDriver::Socket::networkToHost(n);
		break;
	}
	case ModSerialIO::dataTypeInt64:
	case ModSerialIO::dataTypeLongLong:
#if defined(__LP64__) || defined(_LP64)
	case ModSerialIO::dataTypeLong:
#else
#endif
	{
		ModUInt64	n;
		if ((total = this->readNetwork(&n, size)) == size)
			*((ModUInt64*) buf) = ModOsDriver::Socket::networkToHost(n);
		break;
	}
	case ModSerialIO::dataTypeShortArray:
	{
		unsigned short	n;
		for (ModSize i = size / sizeof(n); i--; ) {
			ModSize	len = this->readNetwork(&n, sizeof(n));
			total += len;

			if (len != sizeof(n))
				break;

			*((unsigned short*) buf) =
				ModOsDriver::Socket::networkToHost(n);
			buf = (char*) buf + sizeof(n);
		}
		break;
	}
	case ModSerialIO::dataTypeIntegerArray:
#if defined(__LP64__) || defined(_LP64)
#else
	case ModSerialIO::dataTypeLongArray:
#endif
	{
		unsigned int	n;
		for (ModSize i = size / sizeof(n); i--; ) {
			ModSize	len = this->readNetwork(&n, sizeof(n));
			total += len;

			if (len != sizeof(n))
				break;

			*((unsigned int*) buf) = ModOsDriver::Socket::networkToHost(n);
			buf = (char*) buf + sizeof(n);
		}
		break;
	}
	case ModSerialIO::dataTypeFloatArray:
	{
		float	n;
		for (ModSize i = size / sizeof(n); i--; ) {
			ModSize	len = this->readNetwork(&n, sizeof(n));
			total += len;

			if (len != sizeof(n))
				break;

			*((float*) buf) = ModOsDriver::Socket::networkToHost(n);
			buf = (char*) buf + sizeof(n);
		}
		break;
	}
	case ModSerialIO::dataTypeDoubleArray:
	{
		double	n;
		for (ModSize i = size / sizeof(n); i--; ) {
			ModSize	len = this->readNetwork(&n, sizeof(n));
			total += len;

			if (len != sizeof(n))
				break;

			*((double*) buf) = ModOsDriver::Socket::networkToHost(n);
			buf = (char*) buf + sizeof(n);
		}
		break;
	}
	case ModSerialIO::dataTypeInt64Array:
	case ModSerialIO::dataTypeLongLongArray:
#if defined(__LP64__) || defined(_LP64)
	case ModSerialIO::dataTypeLongArray:
#else
#endif
	{
		ModUInt64	n;
		for (ModSize i = size / sizeof(n); i--; ) {
			ModSize	len = this->readNetwork(&n, sizeof(n));
			total += len;

			if (len != sizeof(n))
				break;

			*((ModUInt64*) buf) = ModOsDriver::Socket::networkToHost(n);
			buf = (char*) buf + sizeof(n);
		}
		break;
	}
	default:
		total = this->readNetwork(buf, size);
	}

	return (int) total;
}

//
// FUNCTION
// ModSocketBase::writeSerial -- ソケットにデータを書き込む
// 
// NOTES
//	バッファからソケットにデータを書き込む。コーデックが設定されていれば
//	それを利用し、バイトオーダーも変換しながら必要なだけ書き進む。
//	すなわちここで htons, ntohs, htonl, ntohl を行なう
//
// ARGUMENTS
//	const void* 		buf
//		書き込むデータが格納されたバッファへのポインタ
//	ModSize				size
//		書き込むバイト数
//	ModSerialIO::DataType type
//		対象データのタイプ
//
// RETURN
//	実際に書き込んだサイズ
//
// EXCEPTIONS
//	その他
//		ModSocketBase::writeNetworkの例外参照
//

int
ModSocketBase::writeSerial(const void* buf, ModSize size, 
						   ModSerialIO::DataType type)
{
	ModSize	total = 0;

	switch(type) {
	case ModSerialIO::dataTypeShort:
	{
		unsigned short	n =
			ModOsDriver::Socket::hostToNetwork(*((const unsigned short*) buf));
		total = this->writeNetwork(&n, size);
		break;
	}
	case ModSerialIO::dataTypeInteger:
#if defined(__LP64__) || defined(_LP64)
#else
	case ModSerialIO::dataTypeLong:
#endif
	{
		unsigned int n =
			ModOsDriver::Socket::hostToNetwork(*((const unsigned int*) buf));
		total = this->writeNetwork(&n, size);
		break;
	}
	case ModSerialIO::dataTypeFloat:
	{
		float	n = ModOsDriver::Socket::hostToNetwork(*((const float*) buf));
		total = this->writeNetwork(&n, size);
		break;
	}
	case ModSerialIO::dataTypeDouble:
	{
		double	n = ModOsDriver::Socket::hostToNetwork(*((const double*) buf));
		total = this->writeNetwork(&n, size);
		break;
	}
	case ModSerialIO::dataTypeInt64:
	case ModSerialIO::dataTypeLongLong:
#if defined(__LP64__) || defined(_LP64)
	case ModSerialIO::dataTypeLong:
#else
#endif
	{
		ModUInt64 n =
			ModOsDriver::Socket::hostToNetwork(*((const ModUInt64*) buf));
		total = this->writeNetwork(&n, size);
		break;
	}
	case ModSerialIO::dataTypeShortArray:
	{
		unsigned short	n;
		for (ModSize i = size / sizeof(n); i--; ) {
			n = ModOsDriver::Socket::hostToNetwork(
				*((const unsigned short*) buf));

			ModSize	len = this->writeNetwork(&n, sizeof(n));
			total += len;

			if (len != sizeof(n))
				break;

			buf = (const char*) buf + sizeof(n);
		}
		break;
	}
	case ModSerialIO::dataTypeIntegerArray:
#if defined(__LP64__) || defined(_LP64)
#else
	case ModSerialIO::dataTypeLongArray:
#endif
	{
		unsigned int	n;
		for (ModSize i = size / sizeof(n); i--; ) {
			n = ModOsDriver::Socket::hostToNetwork(
				*((const unsigned int*) buf));

			ModSize	len = this->writeNetwork(&n, sizeof(n));
			total += len;

			if (len != sizeof(n))
				break;

			buf = (const char*) buf + sizeof(n);
		}
		break;
	}
	case ModSerialIO::dataTypeFloatArray:
	{
		float	n;
		for (ModSize i = size / sizeof(n); i--; ) {
			n = ModOsDriver::Socket::hostToNetwork(*((const float*) buf));

			ModSize	len = this->writeNetwork(&n, sizeof(n));
			total += len;

			if (len != sizeof(n))
				break;

			buf = (const char*) buf + sizeof(n);
		}
		break;
	}
	case ModSerialIO::dataTypeDoubleArray:
	{
		double	n;
		for (ModSize i = size / sizeof(n); i--; ) {
			n = ModOsDriver::Socket::hostToNetwork(*((const double*) buf));

			ModSize	len = this->writeNetwork(&n, sizeof(n));
			total += len;

			if (len != sizeof(n))
				break;

			buf = (const char*) buf + sizeof(n);
		}
		break;
	}
	case ModSerialIO::dataTypeInt64Array:
	case ModSerialIO::dataTypeLongLongArray:
#if defined(__LP64__) || defined(_LP64)
	case ModSerialIO::dataTypeLongArray:
#else
#endif
	{
		ModUInt64	n;
		for (ModSize i = size / sizeof(n); i--; ) {
			n = ModOsDriver::Socket::hostToNetwork(*((const ModUInt64*) buf));

			ModSize	len = this->writeNetwork(&n, sizeof(n));
			total += len;

			if (len != sizeof(n))
				break;

			buf = (const char*) buf + sizeof(n);
		}
		break;
	}
	default:
		total = this->writeNetwork(buf, size);
	}

	return (int) total;
}

//
// FUNCTION
// ModSocketBase::rawRead -- ソケットからデータを読み込む
// 
// NOTES
//	ソケットからデータを指定バイト読み込む。一回で読みきれない場合は
//	(少なくとも1バイトは読める場合)全体が読み終わるまでループする。
//	途中読めたバイト数が0だったらデータ終了のEOFとみなし、そこまで読みこめた
//	サイズを返す。途中でエラーが起きた場合は例外が送出される。
//	ソケットが有効かどうかのチェックは行なわない。必要に応じて上位で
//	ModSocket::isOpened()を使ってチェックする。
//
// ARGUMENTS
//	void* buffer
//		読み込み先のバッファへのポインタ
//	ModSize bytes
//		読み込むサイズ
//	ModSize	min
//		このサイズを読み込むまでは、なんども再読み込みを行う
//
// RETURN
//	実際に読み込んだサイズ
//
// EXCEPTIONS
//	その他
//		ModOsDriver::Socket::readの例外参照。主なものは以下に書き下す
//	ModOsErrorResourceExhaust				(ModOsDriver::Socket::read)
//		処理に必要なストリームリソースが不足
//	ModOsErrorSystemMemoryExhaust			(ModOsDriver::Socket::read)
//		実行に必要なシステムメモリが不足
//	ModOsErrorIOError						(ModOsDriver::Socket::read)
//		IOエラーが起きた
//	ModOsErrorSocketInvalid					(ModOsDriver::Socket::read)
//		無効なソケットである(バインドされていない)
//	(ModOsErrorWinSockConnectAborted)	*** NTのみ	(ModOsDriver::Socket::read)
//		接続が何かの原因で切れ、使えなくなった。(ソケットの作り直しが必要)
//

int
ModSocketBase::rawRead(void* buffer, ModSize bytes, ModSize min)
{
	if (min > bytes)
		min = bytes;

	ModSize	rest = bytes, total = 0;
	do {
		ModSize	n = _socket->read(buffer, rest);
		if (n == 0)
			break;		// ソケットが切れている

		buffer = (char*) buffer + n;
		rest -= n;
		total = bytes - rest;

	} while (rest > 0 && min > total);

	return (int) total;
}

//
// FUNCTION
// ModSocketBase::rawWrite -- ソケットにデータを書き込む
// 
// NOTES
//	ソケットにデータを指定バイト書き込む。一回で書ききれない場合
//	(少なくとも1バイトは書ける場合)全体を書き終わるまでループする。
//	途中で書けるバイト数が0になったら失敗とみなし、そこまで書き込んだ
//	サイズを返す。途中でエラーが起きた場合は例外が送出される。
//	ソケットが有効かどうかのチェックは行なわない。必要に応じて上位で
//	ModSocket::isOpened()を使ってチェックする。
//
// ARGUMENTS
//	void* buffer
//		書き込むデータが格納されているバッファへのポインタ
//	ModSize bytes
//		書き込むサイズ
//	ModSize	min
//		このサイズを書き込むまでは、なんども再書き込みを行う
//
// RETURN
//	実際に書き込んだサイズ
//
// EXCEPTIONS
//	その他
//		ModOsDriver::Socket::writeの例外参照(主なものは以下に書き下す)
//	ModOsErrorResourceExhaust					(ModOsDriver::Socket::write)
//		処理に必要なストリームリソースが不足
//	ModOsErrorSocketInvalid						(ModOsDriver::Socket::write)
//		ソケットが無効(バインドされていないのも含む)
//	ModOsErrorSystemMemoryExhaust				(ModOsDriver::Socket::write)
//		実行に必要なシステムメモリが不足
//	(ModCommonErrorBadArgument)		*** NTのみ	(ModOsDriver::Socket::write)
//		引数エラー
//	(ModOsErrorNotConnect)			*** NTのみ	(ModOsDriver::Socket::write)
//		コネクトされていない
//	(ModOsErrorWinSockConnectAborted)*** NTのみ	(ModOsDriver::Socket::write)
//		接続が何かの原因で切れ、使えなくなった。(ソケットの作り直しが必要)
//

int
ModSocketBase::rawWrite(const void* buffer, ModSize bytes, ModSize min)
{
	ModSize	rest = bytes, total = 0;
	do {
		ModSize	n = _socket->write(buffer, rest);
		if (n == 0)
			break;		// ソケットが切れている

		buffer = (const char*) buffer + n;
		rest -= n;
		total = bytes - rest;

	} while (rest > 0 && min > total);

	return (int) total;
}

// **** 以下はサーバ用

//
// FUNCTION
// ModServerSocket::bind -- サーバーソケットをbindする
// 
// NOTES
//	サービスポートIDを指定し、サーバーソケットをオープンする。基本クラスの
//	オープンの後、サーバーソケットクラスとして、bindからlistenまでを行い、
//	acceptに備える。
//	なお、bind は複数実行可能である。accept した時に、どの bind だったのかを
//	識別するために、mark を指定する
//
// ARGUMENTS
//	int port_
//		ポートID
//	int mark
//		呼出し元が指定する数値
//
// RETURN
// 	なし
//
// EXCEPTIONS
//	その他
//		ModOsDriver::Socket::open, setsockopt、bind, getsockname, listenの例外参照。主なものは以下に書き下す
// ModOsErrorResourceExhaust		   	(ModOsDriver::Socket::open)
//		処理に必要なストリームリソースが不足
// ModOsErrorOpenTooManyFiles		   	(ModOsDriver::Socket::open, ...)
//		プロセスごとのディスクリプタテーブルがいっぱい
// ModOsErrorSystemMemoryExhaust		(ModOsDriver::Socket::open, ...)
//		実行に必要なシステムメモリが不足
//	ModOsErrorIOError							(ModOsDriver::Socket::bind)
//		IOエラーが起きた
//	ModOsErrorResourceExhaust					(ModOsDriver::Socket::bind,...)
//		処理に必要なストリームリソースが不足
//	ModOsErrorInvalidAddress					(ModOsDriver::Socket::bind)
//		アドレスが無効
//	ModOsErrorAddressInUse						(ModOsDriver::Socket::bind,...)
//		指定アドレスは既に使用されている
//	(ModOsErrordIsConnected)	*** NTのみ		(ModOsDriver::Socket::listen)
//		既にコネクトされている
//

void
ModServerSocket::bind(int port, int mark, int option, const char* hostname)
{
	_socket->bind(port, mark, option, hostname);
}

//
// FUNCTION
// ModServerSocket::accept -- コネクションのアクセプト
// 
// NOTES
//	コネクションをアクセプトし、新たなサーバーソケットオブジェクトを
//	生成して返す。
//	新たなソケットオブジェクトには、引数に指定されたコーデックが設定される。
//
// ARGUMENTS
//	int& mark
//		どのbindをacceptしたか
//	ModCodec* codec
//		新たなソケットに設定するコーデック
//
// RETURN
// 	生成されたサーバーソケットへのポインタ
//
// EXCEPTIONS
//	その他
//		ModServerSocket::ModServerSocket, ModOsDriver::Socket::acceptの例外参照。主なものは以下に書き下す
//	ModOsErrorSystemMemoryExhaust		(::new)
//		システムメモリが不足
//	ModOsErrorResourceExhaust
//		処理に必要なストリームリソースが不足
//	(ModOsErrorTooOpenFiles)		*** NTのみ	 (ModOsDriver::Socket::accept)
//		キューが空ではなく、これ以上ディスクリプタが作れない
//

ModServerSocket*
ModServerSocket::accept(int& mark, ModCodec* codec)
{
	// 要求を受け取り、
	// 受け取り先と通信するための仮想 OS のソケットを得る

	ModAutoPointer<ModOs::Socket>	osSock(_socket->accept(mark));

	// このソケットを使うサーバーソケットを生成する

	ModAutoPointer<ModServerSocket>	srvSock(
		new ModServerSocket(osSock.get(), codec));
	(void) osSock.release();

	return srvSock.release();
}

//
// Copyright (c) 1997, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
