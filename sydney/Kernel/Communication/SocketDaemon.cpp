// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SocketDaemon.cpp -- ソケットデーモン
// 
// Copyright (c) 1999, 2001, 2005, 2006, 2009, 2011, 2013, 2014, 2016, 2017, 2023 Ricoh Company, Ltd.
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

#include "Communication/ConnectionKeeper.h"
#include "Communication/ServerSocket.h"
#include "Communication/SocketDaemon.h"
#include "Communication/CryptCodec.h"	// 暗号化対応

#include "Common/Configuration.h"
#include "Common/Message.h"

#include "Buffer/File.h"

#include "Exception/BadArgument.h"
#include "Exception/ModLibraryError.h"
#include "Exception/UserLevel.h"

#include "ModAutoPointer.h"
#include "ModVector.h"

_TRMEISTER_USING

using namespace Communication;

namespace {
	//
	//	VARIABLE local
	//	_cPortNumber -- ポート番号のパラメータ
	//
	Common::Configuration::ParameterInteger	_cPortNumber(
		"Communication_PortNumber", 0, false);

	//
	//	VARIABLE local
	//	_cBindHost -- バインドするホスト名
	//
	Common::Configuration::ParameterString _cBindHost(
		"Communication_BindHostName", "", false);

	//
	//	VARIABLE local
	//	_cTcpKeepAlive -- TCP/IPのKeepAliveを有効にするか否か
	//
	Common::Configuration::ParameterBoolean _cTcpKeepAlive(
		"Communication_TcpKeepAlive", false, false);

	//
	// FUNCTION local
	//	_split -- カンマで分かれている文字列を分割して返す
	//
	ModVector<ModUnicodeString> _split(const ModUnicodeString& src_)
	{
		ModVector<ModUnicodeString> ret;
		const ModUnicodeChar* p = src_;
		const ModUnicodeChar* b = p;

		while (*p != 0)
		{
			if (b == p && (*p == ' ' || *p == '\t'))
			{
				++b;
				++p;
				continue;
			}
			
			switch (*p)
			{
			case ',':
				if (b != p) {
					const ModUnicodeChar* e = p;
					--e;
					while (b < e)
					{
						if (*e != ' ' && *e != '\t')
							break;
						--e;
					}
					++e;
					ModUnicodeString s(b, ModSize(e - b));
					ret.pushBack(s);
				}
				++p;
				b = p;
				break;
			default:
				++p;
				break;
			}
		}
		
		if (b != p) {
			const ModUnicodeChar* e = p;
			--e;
			while (b < e)
			{
				if (*e != ' ' && *e != '\t')
					break;
				--e;
			}
			++e;
			ModUnicodeString s(b, ModSize(e - b));
			ret.pushBack(s);
		}
			
		return ret;
	}

	//
	//	_bind
	//
	void _bind(ModServerSocket* socket_,
			   int portNumber_,
			   int mark_,
			   int option_,
			   ModVector<ModUnicodeString>& hostnames_)
	{
		if (hostnames_.getSize() == 0)
		{
			// ホストが指定されてないので、ワールドワイドにバインドする
			socket_->bind(portNumber_,
						  mark_,
						  option_,
						  0);
		}
		else
		{
			// ホストを指定する
			ModVector<ModUnicodeString>::Iterator i = hostnames_.begin();
			for (; i != hostnames_.end(); ++i)
			{
				socket_->bind(portNumber_,
							  mark_,
							  option_,
							  (*i).getString());
			}
		}
	}
}

#define _MOD_EXCEPTION(e) \
	Exception::ModLibraryError(moduleName, srcFile, __LINE__, e)

//
//	FUNCTION public
//	Communication::SocketDaemon::SocketDaemon -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	Communication::ConnectionKeeper* pKeeper_
//		接続プールクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
SocketDaemon::SocketDaemon(ConnectionKeeper* pKeeper_)
	: m_pKeeper(pKeeper_), m_iBufferSize(4096)
{
}

//
//	FUNCTION public
//	Communication::SocketDaemon::~SocketDaemon -- デストラクタ
//
//	NTOES
//	デストラクタ
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
SocketDaemon::~SocketDaemon()
{
}

//
//	FUNCTION public
//	Communication::SocketDaemon::initialize -- 初期化
//
//	NOTES
//	初期化を行う。Modのサーバソケットを確保する。
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
SocketDaemon::initialize()
{
	m_pCodec = allocateCodec();
	m_pServerSocket = new ModServerSocket(m_pCodec);

	try
	{
		bool isBind = false;

		// Family IPv4 or IPv6 or 空文字列
		int option = 0;
		Socket::Family::Value family = Socket::getFamily();
		switch (family)
		{
		case Socket::Family::Only_v4:
			option = ModOs::only_v4;
			break;
		case Socket::Family::Only_v6:
			option = ModOs::only_v6;
			break;
		default:
			;
		}

		// keepalive
		if (_cTcpKeepAlive.get() == true)
		{
			option |= ModOs::keepAlive;
		}

		// reuse
		option |= ModOs::reuseAddress;

		// まずは、通常のポートをbindする
	
		int iPortNumber = getPortNumber();
		if (iPortNumber != 0)
		{
			// バインドホスト
			ModVector<ModUnicodeString> vecHostName
				= _split(_cBindHost.get());

			// バインドする
			_bind(m_pServerSocket,
				  iPortNumber,
				  0,	// 暗号化なし
				  option,
				  vecHostName);

			isBind = true;
		}

		if (isBind == false)
		{
			// 1つもbindしていないので、エラー
			_TRMEISTER_THROW0(Exception::BadArgument);
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		delete m_pServerSocket, m_pServerSocket = 0;
		delete m_pCodec, m_pCodec = 0;
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Communication::SocketDaemon::terminate -- 後処理
//
//	NOTES
//	後処理を行う。Modのサーバソケットを解放する。
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
SocketDaemon::terminate()
{
	//スレッドを終了する
	abort();
	join();
	//ソケット
	m_pServerSocket->close();
	delete m_pServerSocket;
	delete m_pCodec;
}

//
//	FUNCTION public static
//	Communication::SocketDaemon::getPortNumber -- ポート番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		ポート番号
//
//	EXCEPTIONS
//
int
SocketDaemon::getPortNumber()
{
	return _cPortNumber.get();
}

//
//	FUNCTION public static
//	Communication::SocketDaemon::getCryptPortNumber -- ポート番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		ポート番号
//
//	EXCEPTIONS
//
int
SocketDaemon::getCryptPortNumber()
{
	int p = 0;
	return p;
}

//
//	FUNCTION private
//	Communication::SocketDaemon::runnable -- スレッドとして起動される関数
//
//	NOTES
//	スレッドとして起動される関数。acceptする。
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
SocketDaemon::runnable()
{
	try
	{
		while (1)
		{
			while (m_pServerSocket->select(ModTimeSpan(0,500)) != ModTrue)
			{
				if (isAborted() == true)
				{
					//終了指示
					setStatus(Common::Thread::Aborted);
					m_pKeeper->execute(0, 0);
					return;
				}
			}

			bool reserved = true;
		
			try
			{
				// ソケットを確保するとファイルディスクリプタを消費するので
				// バッファモジュールに1つ利用することを知らせる
				reserved = Buffer::File::reserveDescriptor();
				
				//クライアントからの接続要求が来た
				ModAutoPointer<CryptCodec> pCodec = allocateCodec();

				int mark;
				ModServerSocket* pServerSocket
					= m_pServerSocket->accept(mark, pCodec.get());
				
				if (reserved == false)
				{
					// 一旦受けつけたが、
					// ファイルディスクリプターを予約できなかったので、
					// ここでクローズしてしまう。

					SydMessage << "Too Many Open Socket" << ModEndl;
					
					pServerSocket->close();
					delete pServerSocket;

					continue;
				}
				
				Socket* pSocket = new ServerSocket(
					pServerSocket, pCodec.release());

				// ここから先でエラーが発生しても ServerSocket が close される
				// ので、ファイルディスクリプターの開放を行う必要はない
				
				reserved = false;
				
				m_pKeeper->execute(pSocket, (mark == 1 ? true : false));
				
			}
			catch (Exception::UserLevel&)
			{
				// メッセージは出力しない
				if (reserved)
					Buffer::File::returnDescriptor();
			}
			catch (Exception::Object& e)
			{
				// acceptでエラーが発生した。リトライする
				SydErrorMessage << e << ModEndl;
				SydErrorMessage << "accept failed. retry" << ModEndl;
				if (reserved)
					Buffer::File::returnDescriptor();
			}
			catch (ModException& e)
			{
				// acceptでエラーが発生した。リトライする
				SydErrorMessage << _MOD_EXCEPTION(e) << ModEndl;
				SydErrorMessage << "accept failed. retry" << ModEndl;
				Common::Thread::resetErrorCondition();
				if (reserved)
					Buffer::File::returnDescriptor();
			}
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// もうどうしようもないので、サーバを停止する
		SydErrorMessage << "fatal socket error." << ModEndl;
		setStatus(Common::Thread::Aborted);
		m_pKeeper->execute(0, 0);
		throw;
	}
}

//
//	FUNCTION private
//	Communication::SocketDaemon::allocateCodec -- コーデックを確保する
//
//	NOTES
//	コーデックを確保する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	CryptCodec*
//		コーデック
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
CryptCodec*
SocketDaemon::allocateCodec()
{
	// 暗号化対応
	return new CryptCodec(m_iBufferSize);
}

//
//	Copyright (c) 1999, 2001, 2005, 2006, 2009, 2011, 2013, 2014, 2016, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
