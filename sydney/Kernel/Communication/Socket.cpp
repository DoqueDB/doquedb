// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Socket.cpp -- ソケットクラス
// 
// Copyright (c) 1999, 2002, 2004, 2005, 2006, 2007, 2008, 2011, 2012, 2013, 2014, 2017, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Communication";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Communication/Socket.h"
#include "Communication/CryptCodec.h"	// 暗号化対応

#include "Common/Configuration.h"
#include "Common/ExceptionObject.h"
#include "Common/Externalizable.h"
#include "Common/Message.h"
#include "Common/Thread.h"

#include "Exception/ConnectionRanOut.h"
#include "Exception/ConnectionClosed.h"

#include "ModAutoPointer.h"

_TRMEISTER_USING

using namespace Communication;

namespace {
	//
	//	VARIABLE local
	//	_cFamily -- 利用するプロトコル
	//
	Common::Configuration::ParameterString _cFamily(
		"Communication_Family", "", false);
	
	//
	// VARIABLE local
	//	_cIPv4, _cIPv6 -- Familyの文字列
	//
	ModUnicodeString _cIPv4("IPv4");
	ModUnicodeString _cIPv6("IPv6");
}

//
//	FUNCTION protected
//	Communication::Socket::Socket -- コンストラクタ(1)
//
//	NOTES
//	サーバ側が使用するコンストラクタ。引数のソケットとコーデックは
//	デストラクタでdeleteされる。
//
//	ARGUMENTS
//	ModServerSocket* pServerSocket_
//		サーバソケット
//		バッファサイズ4096のコーデックを持つものとする
//
//	CryptCodec* pCodec_(暗号化対応)
//		サーバソケットに渡されたコーデック
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Socket::Socket(ModServerSocket* pServerSocket_, CryptCodec* pCodec_)
	: SerialIO(SerialIO::Remote), m_pSocket(pServerSocket_),
	  m_bServer(true), m_iBufferSize(0), m_pCodec(pCodec_)
{
}

//
//	FUNCTION public
//	Communication::Socket::Socket -- コンストラクタ(2)
//
//	NOTES
//	クライアント側が使用するコンストラクタ。
//	ソケットのオープンとサーバへのコネクトを行う。
//
//	ARGUMENTS
//	const ModString& cstrHostName_
//		接続先ホスト名
//	int iPort_
//		接続先ポート番号
//	int iBufferSize_
//		バッファサイズ(default 4096)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Socket::Socket(const ModString& cstrHostName_, int iPort_,
			   Family::Value eFamily, int iBufferSize_)
	: SerialIO(SerialIO::Remote), m_bServer(false), m_iBufferSize(iBufferSize_),
	  m_pSocket(0), m_pCodec(0)
{
	int option = 0;
	if (eFamily == Family::Only_v4)
		option = ModOs::only_v4;
	else if (eFamily == Family::Only_v6)
		option = ModOs::only_v6;
	ModAutoPointer<CryptCodec> pCodec// 暗号化対応
		= new CryptCodec(m_iBufferSize);
	ModAutoPointer<ModClientSocket> pClientSocket
		= new ModClientSocket(pCodec.get());
	
	pClientSocket->connect(cstrHostName_, iPort_,
						   option | ModOs::reuseAddress);
	
	m_pSocket = pClientSocket.release();
	m_pCodec = pCodec.release();
}

//
//	FUNCTION public
//	Communication::Socket::~Socket -- デストラクタ
//
//	NOTES
//	デストラクタ
//
// 	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Socket::~Socket()
{
	delete m_pSocket;
	delete m_pCodec;
}

//
//	FUNCTION public
//	Communication::Socket::open -- オープンする
//
//	NOTES
//	入出力アーカイブを作成するだけで、ソケットをオープンするわけではない。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Socket::open()
{
	allocateArchive(*m_pSocket, *m_pSocket);
}

//
//	FUNCTION public
//	Communication::Socket::close -- クローズする
//
//	NOTES
//	入出力アーカイブを破棄し、ソケットをクローズする
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Socket::close()
{
	deallocateArchive();
	m_pSocket->close();
}

//
//	FUNCTION public
//	Communication::Socket::readObject -- オブジェクトを読み込む
//
//	NOTES
//	オブジェクトを読み込む
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Externalizable*
//		読み込んだオブジェクトへのポインタ
//
//	EXCEPTIONS
//	Exception::ConnectionRanOut
//		Modの例外が発生した場合
//	その他
//		下位の例外はそのまま再送
//
Common::Externalizable*
Socket::readObject()
{
	try
	{
		while (this->wait(500) != true)
		{
			if (ModThisThread::isAborted() == ModTrue)
			{
				_TRMEISTER_THROW0(Exception::ConnectionRanOut);
			}
		}

		return SerialIO::readObject();
	}
	catch (Exception::Object& e)
	{
		if (m_bServer) SydErrorMessage << e << ModEndl;
		// 例外はConnectionRanOutにする
		_TRMEISTER_THROW0(Exception::ConnectionRanOut);
	}
	catch (ModException& e)
	{
		Common::Thread::resetErrorCondition();
		if (e.getErrorNumber() == ModOsErrorEndOfFile)
			// EOFなのでConnectionClosedにする
			_TRMEISTER_THROW0(Exception::ConnectionClosed);
		
		if (m_bServer) SydErrorMessage << e.setMessage() << ModEndl;
		// 例外はConnectionRanOutにする
		_TRMEISTER_THROW0(Exception::ConnectionRanOut);
	}
#ifndef NO_CATCH_ALL
	catch (...)
	{
		if (m_bServer) SydErrorMessage << "Unexpected Error." << ModEndl;
		// 例外はConnectionRanOutにする
		_TRMEISTER_THROW0(Exception::ConnectionRanOut);
	}
#endif
}

//
//	FUNCTION public
//	Communication::Socket::writeObject -- オブジェクトを書き込む
//
//	NOTES
//	オブジェクトを書き込む。
//
//	ARGUMENTS
//	const Common::Externalizable* pObject_
//		書き込むオブジェクトへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::ConnectionRanOut
//		Modの例外が発生した場合
//	その他
//		下位の例外はそのまま再送
//
void
Socket::writeObject(const Common::Externalizable* pObject_)
{
	try
	{
		SerialIO::writeObject(pObject_);
	}
	catch (Exception::Object& e)
	{
		if (m_bServer) SydErrorMessage << e << ModEndl;
		// 例外はConnectionRanOutにする
		_TRMEISTER_THROW0(Exception::ConnectionRanOut);
	}
	catch (ModException& e)
	{
		Common::Thread::resetErrorCondition();
		if (m_bServer) SydErrorMessage << e.setMessage() << ModEndl;
		// 例外はConnectionRanOutにする
		_TRMEISTER_THROW0(Exception::ConnectionRanOut);
	}
#ifndef NO_CATCH_ALL
	catch (...)
	{
		if (m_bServer) SydErrorMessage << "Unexpected Error." << ModEndl;
		// 例外はConnectionRanOutにする
		_TRMEISTER_THROW0(Exception::ConnectionRanOut);
	}
#endif
}

//
//	FUNCTION public
//	Communication::Socket::readInteger -- 32ビット整数を読み込む
//
//	NOTES
//	32ビット整数を読み込む
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		読み込んだ値
//
//	EXCEPTIONS
//	Exception::ConnectionRanOut
//		Modの例外が発生した場合
//	その他
//		下位の例外はそのまま再送
//
int
Socket::readInteger()
{
	try
	{
		return SerialIO::readInteger();
	}
	catch (Exception::Object& e)
	{
		if (m_bServer) SydErrorMessage << e << ModEndl;
		// 例外はConnectionRanOutにする
		_TRMEISTER_THROW0(Exception::ConnectionRanOut);
	}
	catch (ModException& e)
	{
		Common::Thread::resetErrorCondition();
		if (e.getErrorNumber() == ModOsErrorEndOfFile)
			// EOFなのでConnectionClosedにする
			_TRMEISTER_THROW0(Exception::ConnectionClosed);
		
		if (m_bServer) SydErrorMessage << e.setMessage() << ModEndl;
		// その他の例外はConnectionRanOutにする
		_TRMEISTER_THROW0(Exception::ConnectionRanOut);
	}
#ifndef NO_CATCH_ALL
	catch (...)
	{
		if (m_bServer) SydErrorMessage << "Unexpected Error." << ModEndl;
		// その他の例外はConnectionRanOutにする
		_TRMEISTER_THROW0(Exception::ConnectionRanOut);
	}
#endif
}

//
//	FUNCTION public
//	Communication::Socket::writeInteger -- 32ビット整数を書き込む
//
//	NOTES
//	32ビット整数を書き込む。
//
//	ARGUMENTS
//	int iValue_
//		値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Socket::writeInteger(int iValue_)
{
	try
	{
		SerialIO::writeInteger(iValue_);
	}
	catch (Exception::Object& e)
	{
		if (m_bServer) SydErrorMessage << e << ModEndl;
		// 例外はConnectionRanOutにする
		_TRMEISTER_THROW0(Exception::ConnectionRanOut);
	}
	catch (ModException& e)
	{
		Common::Thread::resetErrorCondition();
		if (m_bServer) SydErrorMessage << e.setMessage() << ModEndl;
		//Modの例外はConnectionRanOutにする
		_TRMEISTER_THROW0(Exception::ConnectionRanOut);
	}
#ifndef NO_CATCH_ALL
	catch (...)
	{
		if (m_bServer) SydErrorMessage << "Unexpected Error." << ModEndl;
		// その他の例外はConnectionRanOutにする
		_TRMEISTER_THROW0(Exception::ConnectionRanOut);
	}
#endif
}

//
//	FUNCTION public
//	Communication::Socket::flush -- 出力をフラッシュする
//
//	NOTES
//	出力をフラッシュする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Socket::flush()
{
	try
	{
		SerialIO::flush();
	}
	catch (Exception::Object& e)
	{
		if (m_bServer) SydErrorMessage << e << ModEndl;
		// 例外はConnectionRanOutにする
		_TRMEISTER_THROW0(Exception::ConnectionRanOut);
	}
	catch (ModException& e)
	{
		Common::Thread::resetErrorCondition();
		if (m_bServer) SydErrorMessage << e.setMessage() << ModEndl;
		//Modの例外はConnectionRanOutにする
		_TRMEISTER_THROW0(Exception::ConnectionRanOut);
	}
#ifndef NO_CATCH_ALL
	catch (...)
	{
		if (m_bServer) SydErrorMessage << "Unexpected Error." << ModEndl;
		// その他の例外はConnectionRanOutにする
		_TRMEISTER_THROW0(Exception::ConnectionRanOut);
	}
#endif
}

//
//	FUNCTION public
//	Communication::Socket::wait -- 入力があるまで待つ
//
//	NOTES
//	入力があるまで指定時間待つ。
//
//	ARGUMETNS
//	int iMilliseconds_
//		待つ時間(ミリ秒)
//
//	RETURN
//	入力があった場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
bool
Socket::wait(int iMilliseconds_)
{
	bool result = false;
	if (iMilliseconds_ < 0)
	{
		//永遠に待つのがModSocketにないので、とりあえず10分づつまつ
		ModTimeSpan span(0, 0, 10, 0);
		while (m_pSocket->select(span) != ModTrue)
			{;}//nop
		result = true;
	}
	else
	{
		ModTimeSpan span(iMilliseconds_ / 1000, iMilliseconds_ % 1000);
		result = (m_pSocket->select(span) == ModTrue)
			? true : false;
	}
	return result;
}

//
//	FUNCTION public
//	Communication::Socket::setKey -- 共通鍵設定
//
//	NOTES
//	共通鍵設定(暗号化対応)
//
//	ARGUMETNS
//	CryptKey::Pointer pKey_
//		共通鍵
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Socket::setKey(const CryptKey::Pointer& pKey_)
{
	m_pCodec->setKey(pKey_);
}

//
//	FUNCTION public
//	Communication::Socket::getKey -- 共通鍵取得
//
//	NOTES
//	共通鍵取得(暗号化対応)
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	CryptKey::Pointer
//		共通鍵
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
const CryptKey::Pointer&
Socket::getKey()
{
	return m_pCodec->getKey();
}

//
//	FUNCTION public static
//	Communication::Socket::getFamily -- パラメータから利用プロトコルを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Communication::Socket::Family::Value
//		パラメータ値
//
//	EXCEPTIONS
//
Socket::Family::Value
Socket::getFamily()
{
	Family::Value family = Family::Unspec;
	
	if (_cFamily.get().compare(_cIPv4, ModFalse) == 0)
		family = Family::Only_v4;
	else if (_cFamily.get().compare(_cIPv6, ModFalse) == 0)
		family = Family::Only_v6;

	return family;
}

//
//	FUNCTION public static
//	Communication::Socket::getCryptFamily -- パラメータから利用プロトコルを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Communication::Socket::Family::Value
//		パラメータ値
//
//	EXCEPTIONS
//
Socket::Family::Value
Socket::getCryptFamily()
{
	Family::Value family = Family::Unspec;

	return family;
}

//
//	Copyright (c) 1999, 2002, 2004, 2005, 2006, 2007, 2008, 2011, 2012, 2013, 2014, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
