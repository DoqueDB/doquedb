// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ServerConnection.cpp -- サーバコネクション
// 
// Copyright (c) 1999, 2001, 2004, 2006, 2012, 2023 Ricoh Company, Ltd.
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

#include "Communication/ConnectionSupplier.h"
#include "Communication/SerialIO.h"
#include "Communication/ServerConnection.h"
#include "Common/Request.h"
#include "Common/Message.h"
#include "Common/Thread.h"

#include "Exception/ConnectionRanOut.h"
#include "Exception/ModLibraryError.h"

#include "ModAutoPointer.h"

#define _SERVER_MOD_EXCEPTION(e) \
      Exception::ModLibraryError(moduleName, srcFile, __LINE__, e)

_TRMEISTER_USING

using namespace Communication;

//
//	FUNCTION public
//	Communication::ServerConnection::ServerConnection
//													-- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	int iMasterID_
//		マスターID
//	int iSlaveID_
//		スレーブID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ServerConnection::ServerConnection(int iMasterID_, int iSlaveID_)
: Connection(iMasterID_, iSlaveID_)
{
}

//
//	FUNCTION public
//	Communication::ServerConnection::~ServerConnection
//														-- デストラクタ
//
//	NOTES
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
ServerConnection::~ServerConnection()
{
	close();
}

//
//	FUNCTION public
//	Communication::ServerConnection -- オープンする
//
//	NOTES
//	オープンする
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
ServerConnection::open()
{
	if (m_pSerialIO == 0)
	{
		ConnectionSupplier::consumeConnection(this);

		try
		{
				// getMasterID()は暗号Bitが落ちてくるので完全IDを取得して渡す(暗号化対応)
				m_pSerialIO->writeInteger(getFullMasterID());// 暗号化対応
				m_pSerialIO->writeInteger(getSlaveID());
				m_pSerialIO->flush();
				m_fOpen = true;
		}
		catch (Exception::Object& e)
		{
				SydErrorMessage << e << ModEndl;
				_SYDNEY_THROW0(Exception::ConnectionRanOut);
		}
		catch (ModException& e)
		{
			Common::Thread::resetErrorCondition();
			SydErrorMessage << _SERVER_MOD_EXCEPTION(e) << ModEndl;
			_SYDNEY_THROW0(Exception::ConnectionRanOut);
		}
		catch (...)
		{
			SydErrorMessage << "Unexpected error occured." << ModEndl;
			_SYDNEY_RETHROW;
		}
	}
}

//
//	FUNCTION public
//	Communication::ServerConnection -- クローズする
//
//	NOTES
//	クローズする
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
ServerConnection::close()
{
	if (m_pSerialIO)
	{
		m_pSerialIO->close();
		delete m_pSerialIO; m_pSerialIO = 0;
		m_fOpen = false;
	}
}

//
//	FUNCTION public
//	Communication::ServerConnection::sync -- 相手との同期を取る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ServerConnection::sync()
{
	// 入力が来るまで、ポーリングする 5秒 x 12回 = 1分

	int n = 12;
	while (true)
	{
		--n;
		if (m_pSerialIO->wait(5000) == true)
			break;
		
		if (n == 0 || isCanceled())
		{
			// リクエストが来なかったので、終了する
			_SYDNEY_THROW0(Exception::ConnectionRanOut);
		}
	}
}

//
//	Copyright (c) 1999, 2001, 2004, 2006, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
