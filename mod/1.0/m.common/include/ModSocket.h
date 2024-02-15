// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4
//
// ModSocket.h -- シリアル化可能ソケットのクラス定義
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

#ifndef __ModSocket_H__
#define __ModSocket_H__

#include "ModCommonDLL.h"
#include "ModOsDriver.h"
#include "ModDefaultManager.h"
#include "ModSerialIO.h"
#include "ModCodec.h"
#include "ModString.h"
#include "ModTimeSpan.h"

//
// モジュールは汎用OSに属する。
// したがって、エラーはModOsXXXである。
//

//
// CLASS
// ModSocketBase -- すべてのソケットの基底クラス(シリアライズ可能)
//
// NOTES
// ソケットを保持し、ソケットに関する処理を行うクラスであり、
// 読み書きする内容をアーカイバによってシリアライズすることが
// できるクラスである。
// シリアル化可能とするため、ModSerialIOの派生クラスである。
// 実際には、派生クラスであるModServerSocket, ModClientSocketのどちらかを
// 利用する。
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModSocketBase
	: public	ModSerialIO
{
public:
	// ソケット独自メソッド

	// コンストラクタ
	ModCommonDLL
    ModSocketBase(ModCodec* codec = 0);

    virtual ~ModSocketBase();					// デストラクター

	// ソケットをクローズ
	ModCommonDLL
	void close();

	ModBoolean isOpened() const;
	
	// 同期をとるために待つ。
	ModCommonDLL
	ModBoolean select(const ModTimeSpan& waitTime) const;

	// IPv6か否か
	ModBoolean isIPv6() const;
		
	// シリアライズに必要な関数群
	// コーデックを通し、オーダーも変換して読み書きする
	ModCommonDLL
    int readSerial(void* buffer_, ModSize byte_, 
				   ModSerialIO::DataType type_);
	ModCommonDLL
    int writeSerial(const void* buffer_, ModSize byte_, 
					ModSerialIO::DataType type_);
	// コーデックの先頭に戻る
    void resetSerial();
	// 内容をフラッシュ
    void readFlushSerial();
    void writeFlushSerial();
	// 以下はソケットにはない
	// int seekSerial(); 		// ModSocketBaseでは不要
	// getCurrentPosition();	// ModSocketBaseでは不要
	// getCurrentAddress();		// ModSocketBaseでは不要
	// getHeadAddress();		// ModSocketBaseでは不要
	// getSize();				// ModSocketBaseでは不要
	ModCommonDLL
	int	getCompressSize();

	// コーデックも、オーダー変換も通さず、読み書きする
	int	rawRead(void* buffer, ModSize bytes);
	ModCommonDLL
	int	rawRead(void* buffer, ModSize bytes, ModSize min);
	int	rawWrite(const void* buffer, ModSize bytes);
	ModCommonDLL
	int	rawWrite(const void* buffer, ModSize bytes, ModSize min);

protected:
    ModSocketBase(ModOs::Socket* socket, ModCodec* codec = 0);
												// 既存の仮想 OS ソケットからの
												// コンストラクター

	ModOsDriver::Socket*	_socket;			// 仮想 OS のソケット
	ModCodec*				_codec;				// 圧縮、伸長に使用する
												// 符号化クラス

private:
	// コーデックを通して読み書きするが、変換はしない
    int	readNetwork(void* buf, ModSize nbytes);
    int writeNetwork(const void* buf, ModSize nbytes);
};

//	FUNCTION protected
//	ModSocketBase::ModSocketBase --
//		既に生成されている仮想 OS のソケットを使用する
//		シリアル化可能ソケットを表すクラスのコンストラクター
// 
//	NOTES
//		与えられた仮想 OS のソケットは、デストラクト時に破棄される
//
//	ARGUMENTS
//		ModOs::Socket*		socket
//			既に生成済みの仮想 OS のソケット
//			通常は、ModOsDriver::accept で生成されたソケットを与える
//		ModCodec*			codec
//			0 以外の値
//				圧縮、伸長に使用する符号化クラスを格納する領域の先頭アドレス
//			0 または指定されないとき
//				圧縮、伸長を行わない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ModSocketBase::ModSocketBase(ModOs::Socket* socket, ModCodec* codec) 
	: _socket((ModOsDriver::Socket*) socket),
	  _codec(codec)
{
	this->resetSerial();
}

//	FUNCTION public
//	ModSocketBase::~ModSocketBase --
//		シリアル化可能ソケットを表すクラスのデストラクター
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
ModSocketBase::~ModSocketBase()
{
	delete _socket, _socket = 0;
}

//
// FUNCTION
// ModSocketBase::isOpened -- ソケットが有効かどうかをチェックする
// 
// NOTES
//	ソケットが有効かどうか(オープンされているか、クローズ済みでないか)を
//	返す。
//
// ARGUMENTS
//	なし
//
// RETURN
// 	有効な場合はModTrue, 無効な場合はModFalse
//
// EXCEPTIONS
//	なし
//

inline
ModBoolean
ModSocketBase::isOpened() const
{
	return (_socket) ? _socket->isOpened() : ModFalse;
}

//
// FUNCTION
// ModSocketBase::isIPv6 -- IPv6か否か
// 
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
// 	IPv6の場合はModTrue, 無効な場合はModFalse
//
// EXCEPTIONS
//	なし
//

inline
ModBoolean
ModSocketBase::isIPv6() const
{
	return (_socket) ? _socket->isIPv6() : ModFalse;
}

//	FUNCTION public
// ModSocketBase::resetSerial -- コーデックのバッファの先頭に戻る
// 
// NOTES
//	コーデックが設定されている場合、コーデックのバッファの先頭に戻る。
//	コーデックが設定されていないときには何もしない。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		コーデックのresetの例外参照

inline
void 
ModSocketBase::resetSerial()
{
	if (_codec)
		_codec->reset();
}

//	FUNCTION public
// ModSocketBase::readFlushSerial -- コーデックの読み込みバッファをフラッシュ
// 
// NOTES
//	コーデックが設定されている場合、コーデックの読み込みバッファを
//	フラッシュする。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		コーデックがあれば、そのdecodeFlushの例外参照(デフォルトコーデックの場合を以下に書き下す)
//	ModOsErrorReadProtocolInCodec		(ModCodec::decodeFlush)
//		コーデックのプロトコルによるreadでエラーが起きた(Fatal)
//	ModOsErrorReadDataInCodec			(ModCodec::decodeFlush)
//		コーデックによるデータのreadでエラーが起きた
//  その他								(ModCodec::decodeFlush)
//		ModSocket::rawReadの例外参照

inline
void 
ModSocketBase::readFlushSerial()
{
	if (_codec)
		_codec->decodeFlush(this);
}

//	FUNCTION public
// ModSocketBase::writeFlushSerial -- コーデックの書き込みバッファをフラッシュ
// 
// NOTES
//	コーデックが設定されている場合、コーデックの書き込みバッファを
//	フラッシュする。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	その他
//		コーデックがあれば、そのencodeFlushの例外参照(デフォルトコーデックの場合を以下に書き下す)
//	ModOsErrorWriteProtocolInCodec		(ModCodec::encodeFlush)
//		コーデックのプロトコルによるwriteでエラーが起きた(Fatal)
//	ModOsErrorWriteDataInCodec			(ModCodec::encodeFlush)
//		コーデックによるデータ部分のwriteでエラーが起きた
//  その他								(ModCodec::encodeFlush)
//		ModSocket::rawWriteの例外参照

inline
void 
ModSocketBase::writeFlushSerial()
{
	if (_codec)
		_codec->encodeFlush(this);
}

// 説明は ModSocket.cpp を参照のこと

inline
int
ModSocketBase::rawRead(void* buffer, ModSize bytes)
{
	return this->rawRead(buffer, bytes, bytes);
}

// 説明は ModSocket.cpp を参照のこと

inline
int
ModSocketBase::rawWrite(const void* buffer, ModSize bytes)
{
	return this->rawWrite(buffer, bytes, bytes);
}

//	FUNCTION private
// ModSocketBase::readNetwork -- ソケットからオーダー変換なしでデータを読む
// 
// NOTES
//	コーデックが設定されている場合、それを利用してソケットからデータを必要な
//	だけ読む。ただしバイトオーダーの変換はしない。
//
// ARGUMENTS
//	void* buffer
//		読み込み先のバッファへのポインタ
//	ModSize bytes
//		読み込みサイズ
//
// RETURN
//	実際に読み込んだサイズ
//
// EXCEPTIONS
//	その他
//		ModOsDriver::Socket::read、コーデックのdecodeFlushの例外参照、主なものは以下に書き下す
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
//	ModOsErrorReadProtocolInCodec		(ModCodec::decodeFlush)
//		コーデックのプロトコルによるreadでエラーが起きた(Fatal)
//	ModOsErrorReadDataInCodec			(ModCodec::decodeFlush)
//		コーデックによるデータのreadでエラーが起きた

inline
int	
ModSocketBase::readNetwork(void* buffer, ModSize bytes)
{
	return (_codec) ?
		_codec->decode(this, buffer, bytes) : this->rawRead(buffer, bytes);
}

//	FUNCTION private
// ModSocketBase::writeNetwork -- ソケットにオーダー変換なしでデータを書く
// 
// NOTES
//	コーデックが設定されている場合、それを利用してソケットにデータを必要な
//	だけ書く。ただしバイトオーダーの変換はしない。
//
// ARGUMENTS
//	void* buffer
//		書き込むデータが格納されているバッファへのポインタ
//	ModSize bytes
//		書き込みサイズ
//
// RETURN
//	実際に書き込んだサイズ
//
// EXCEPTIONS
//	その他
//		ModOsDriver::Socket::write、コーデックのencodeFlushの例外参照(主なものは以下に書き下す)
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
//	ModOsErrorWriteProtocolInCodec		(ModCodec::encodeFlush)
//		コーデックのプロトコルによるwriteでエラーが起きた(Fatal)
//	ModOsErrorWriteDataInCodec			(ModCodec::encodeFlush)
//		コーデックによるデータ部分のwriteでエラーが起きた

inline
int
ModSocketBase::writeNetwork(const void* buffer, ModSize bytes)
{
	return (_codec) ?
		_codec->encode(this, buffer, bytes) : this->rawWrite(buffer, bytes);
}

//
// CLASS
// ModServerSocket -- サーバ側で利用するソケットクラス
//
// NOTES
//	ModSocketBaseから派生し、サーバとして必要な処理部分のみ追加したもので、
//  シリアル化可能なクラスである。
//

//	メモリハンドルを明示しないModPureServerSocketとして作成しようとしたが、
//	以下の理由により断念し、直接ModDefaultObjectのサブクラスとして作成する。
//  [理由]
//	accept()では新たなクラスModPureXXXを返すが、そこで
//	「エラー: ModPureServerSocket* を ModServerSocket* に代入
//	することはできません。」とユーザプログラム作成時にエラーになるため。
//	(libCommon.aは作成できるので注意)
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModServerSocket
	: public	ModSocketBase,
	  public	ModDefaultObject
{
public:
	ModServerSocket(ModCodec* codec = 0);
	virtual ~ModServerSocket();

	// サーバ用のオープン
	//	option は ModOs::SocketOption の論理和
	ModCommonDLL
	void bind(int port, int mark, int option, const char* hostname = 0);
	// コネクションを受けて新たなソケットを返す。
	ModCommonDLL
	ModServerSocket* accept(int& mark, ModCodec* codec = 0);
	// socket の接続先のアドレスを得る
	ModCharString getPeerName() const;
private:
	// 特別なaccept用のコンストラクタ
	ModServerSocket(ModOs::Socket* socket, ModCodec* codec = 0);
};

//
// FUNCTION
// ModServerSocket::ModServerSocket -- サーバーソケットクラスのコンストラクタ
// 
// NOTES
//	サーバーソケットクラスを作成し、コーデックを設定する。
//	必要な初期化が行われる。
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

inline
ModServerSocket::ModServerSocket(ModCodec* codec)
	: ModSocketBase(codec)
{ }

//
// FUNCTION private
// ModServerSocket::ModServerSocket -- サーバーソケットクラスのコンストラクタ(2)
// 
// NOTES
//	サーバーソケットクラスを作成し、コーデックを設定する。
//	必要な初期化が行われる。
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

inline
ModServerSocket::ModServerSocket(ModOs::Socket* socket, ModCodec* codec)
	: ModSocketBase(socket, codec)
{ }

//
// FUNCTION
// ModServerSocket::~ModServerSocket -- サーバーソケットクラスのデストラクタ
// 
// NOTES
//	サーバーソケットクラスの後処理をする。サーバーソケットクラスとしては特に
//	何もしないが、クローズされていなければ基本クラスであるModSocketの後処理で
//	ソケットがクローズされる。
//
// ARGUMENTS
//	なし
//
// RETURN
// 	なし
//
// EXCEPTIONS
//	その他
//		ModOsDriver::Socket::closeの例外参照
//

inline
ModServerSocket::~ModServerSocket()
{ }

//
// FUNCTION
// ModServerSocket::getPeerName -- 接続先のアドレス名を得る
// 
// NOTES
//	ソケットの接続先のアドレス名を得る
//
// ARGUMENTS
// なし
//
// RETURN
//	接続先のアドレス名を文字列として返す
//
// EXCEPTIONS
//	その他
//		ModOsDriver::Socket::getpeernameの例外参照
//

inline
ModCharString
ModServerSocket::getPeerName() const
{
	return _socket->getpeername();
}

//
// CLASS
// ModPureClientSocket -- クライアント側のソケット機能クラス
//
// NOTES
//	ModSocketBaseから派生し、クライアントとして必要な処理のみ追加したもので、
//  シリアル化可能である。
//	実際にはメモリハンドルを明示したModClientSocketを利用する。

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModPureClientSocket
	: public	ModSocketBase
{
public:
	// コンストラクター
	ModPureClientSocket(ModCodec* codec = 0);
	// デストラクター
	virtual ~ModPureClientSocket();

	// 指定したホストにコネクトする
	//	option は ModOs::SocketOption の論理和
	void connect(const ModCharString& hostname, int port, int option);
};

// ****** クライアントソケット

//
// FUNCTION
// ModPureClientSocket::ModPureClientSocket -- クライアントソケットクラスのコンストラクタ
// 
// NOTES
//	クライアントソケットクラスを作成し、コーデックを設定する。
//
// ARGUMENTS
//	ModCodec* codec
//		コーデックを行うオブジェクト
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

inline
ModPureClientSocket::ModPureClientSocket(ModCodec* codec)
	: ModSocketBase(codec)
{ }

//
// FUNCTION
// ModPureClientSocket::~ModPureClientSocket -- クライアントソケットクラスのデストラクタ
// 
// NOTES
//	クライアントソケットクラスの後処理をする。サーバーソケットクラスとしては特に
//	何もしないが、クローズされていなければ基本クラスであるModSocketの後処理で
//	ソケットがクローズされる。
//
// ARGUMENTS
//	なし
//
// RETURN
// 	なし
//
// EXCEPTIONS
//	その他
//		ModOsDriver::Socket::closeの例外参照
//

inline
ModPureClientSocket::~ModPureClientSocket()
{ }

//
// FUNCTION
// ModPureClientSocket::connect -- 指定したホストにコネクト
// 
// NOTES
// 指定したホストにコネクトする。
//
// ARGUMENTS
//	ModCharString& hostname
//		コネクト先のホスト名
//	int port
//		コネクト先のポート番号
//	int option
//		ModOs::SocketOptionの論理和
//
// RETURN
// なし
//
// EXCEPTIONS
//	その他
//		ModOsDriver::Socket::connectの例外参照(主なものは以下に書き下す)
//	ModOsErrorResourceExhaust		(ModOsDriver::Socket::connect)
//		処理に必要なストリームリソースが不足
//	ModOsErrorIOError				(ModOsDriver::Socket::connect)
//		IOエラーが起きた
//	ModOsErrorHostNotFound			(ModOsDriver::Socket::connect)
//		ホストがみつからない
//	ModOsErrorServerFailed			(ModOsDriver::Socket::connect)
//		サーバーエラーもしくはNon-Authoritive Host not found
//	ModOsErrorNetworkUnreach		(ModOsDriver::Socket::connect)
//		ネットワークにつながらない
//	ModOsErrorAddressInUse			(ModOsDriver::Socket::connect)
//		アドレスは既に使用されている
//	ModOsErrorConnectAlready		(ModOsDriver::Socket::connect)
//		既にコネクト進行中
//	ModOsErrorConnectRefused		(ModOsDriver::Socket::connect)
//		コネクトが拒否された
//	ModOsErrorIsConnected			(ModOsDriver::Socket::connect)
//		既にコネクトされている
//	ModOsErrorConnectTimeout		(ModOsDriver::Socket::connect)
//		コネクション成立までにタイムアウト
//	(ModOsErrorInvalidAddress)	   *** NTのみ	(ModOsDriver::Socket::connect)
//		指定アドレスは無効
//	(ModOsErrorSystemMemoryExhaust)	 *** NTのみ	(ModOsDriver::Socket::connect)
//		ソケット用バッファが確保できない
//

inline
void
ModPureClientSocket::connect(const ModCharString& hostname,
							 int port, int option)
{
	_socket->connect(hostname.getString(), port, option);
}

//
// CLASS
//	ModClientSocket -- ModPureClientSocketクラスのメモリハンドル明示クラス
// NOTES
//	ModPureClientSocketクラスをデフォルトメモリハンドルの管理下のクラスとして
//	利用するためのクラスである。ユーザは通常本クラスを利用する。
//

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModClientSocket
	: public	ModObject<ModDefaultManager>, 
	  public	ModPureClientSocket
{
public:
	// コンストラクター
	ModClientSocket(ModCodec* codec = 0)
		: ModPureClientSocket(codec)
	{}
	// デストラクター
	~ModClientSocket()
	{}
};

#endif	// __ModSocket_H__

//
// Copyright (c) 1997, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
