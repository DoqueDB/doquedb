// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LocalClientConnection.cpp -- クライアントメモリコネクション
// 
// Copyright (c) 2000, 2001, 2004, 2005, 2006, 2007, 2013, 2023 Ricoh Company, Ltd.
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

#include "Communication/ConnectionSlaveID.h"
#include "Communication/CryptMode.h"
#include "Communication/LocalClientConnection.h"
#include "Communication/LocalMemory.h"
#include "Communication/MemoryDaemon.h"

#include "Common/UnicodeString.h"
#include "Exception/CannotConnect.h"

#include "Os/AutoCriticalSection.h"

_TRMEISTER_USING
using namespace Communication;

//
//	VARIABLE private
//	Communication::LocalClientConnection::m_cCriticalSection
//
//	NOTES
//	サーバからの一時コネクションのやり取りを排他制御するための
//	クリティカルセクション
//
Os::CriticalSection
Communication::LocalClientConnection::m_cCriticalSection;

//
//	FUNCTION public
//	Communication::LocalClientConnection::LocalClientConnection
//												-- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	int iMasterID_
//		マスタID
//	int iSlaveID_
//		スレーブID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
LocalClientConnection::LocalClientConnection(int iMasterID_, int iSlaveID_)
	: ClientConnection(iMasterID_, iSlaveID_)
{
}

//
//	FUNCTION public
//	Communication::LocalClientConnection::~LocalClientConnection
//													-- デストラクタ
//
//	NOTES
//	デストラクタ。
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
LocalClientConnection::~LocalClientConnection()
{
	close();
}

//
//	FUNCTION public
//	Communication::LocalClientConnection::open -- オープン
//
//	NOTES
//	コネクトする。
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
LocalClientConnection::open()
{
	if (m_pSerialIO == 0)
	{
		int iMasterID;
		int iSlaveID;
		void* p;
		{
			Os::AutoCriticalSection cAuto(m_cCriticalSection);
			LocalMemory* pMemory = MemoryDaemon::getClientLocalMemory();
			pMemory->writeInteger(0);	//ダミーを送る
			pMemory->flush();
			p = pMemory->readPointer();
		}
		SerialIO* pSerialIO = syd_reinterpret_cast<SerialIO*>(p);
		try
		{
			pSerialIO->open();
			pSerialIO->writeInteger(getFullMasterID() & ~CryptMode::MaskAlgorithm);
			pSerialIO->writeInteger(getSlaveID());
			pSerialIO->flush();
			iMasterID = pSerialIO->readInteger();
			iSlaveID = pSerialIO->readInteger();
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			//エラー
			pSerialIO->close();
			delete pSerialIO;
			_TRMEISTER_THROW2(Exception::CannotConnect, _TRMEISTER_U_STRING("Local"), 0);
		}
		setFullMasterID(iMasterID);	// 暗号化対応
		setSlaveID(iSlaveID);
		setSerialIO(pSerialIO);
		m_fOpen = true;
	}
}

//
//	FUNCTION public
//	Communication::LocalClientConnection::close -- クローズ
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
LocalClientConnection::close()
{
	if (m_pSerialIO)
	{
		m_pSerialIO->close();
		delete m_pSerialIO; m_pSerialIO = 0;
		m_fOpen = false;
	}
}

//
//	Copyright (c) 2000, 2001, 2004, 2005, 2006, 2007, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
