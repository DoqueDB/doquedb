// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RemoteClientConnection.cpp -- クライアントソケットコネクション
// 
// Copyright (c) 1999, 2002, 2004, 2005, 2006, 2011, 2013, 2015, 2017, 2023 Ricoh Company, Ltd.
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
#include "Communication/Local.h"
#include "Communication/RemoteClientConnection.h"
#include "Communication/Socket.h"

#include "Common/UnicodeString.h"
#include "Common/Thread.h"

#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"

#include "Exception/CannotConnect.h"

#include "ModHashMap.h"
#include "ModPair.h"
#include "ModUnicodeString.h"

_TRMEISTER_USING

using namespace Communication;

namespace {

	typedef ModPair<ModUnicodeString, int> IPKey;
	
	class IPKeyHasher {
	public:
		// ハッシュ関数
		ModSize operator () (const IPKey& key) const {
			ModUnicodeStringHasher hash;
			return (hash.operator()(key.first) << 4 | key.second);
		}
	};
	
	typedef ModHashMap<IPKey, bool, IPKeyHasher> IPMap;
	
	// ホスト名 + ポート番号をキーに、IPv6なのかどうかを管理するマップ
	IPMap _ipMap;

	// 上記のマップを保護するためのラッチ
	Os::CriticalSection _latch;
}

//
//	FUNCTION public
//	Communication::RemoteClientConnection::RemoteClientConnection
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	const ModUnicodeString& cstrHostName_
//		接続先のホスト名
//	int iPortNumber_
//		接続するポート番号
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
RemoteClientConnection::RemoteClientConnection(
	const ModUnicodeString& cstrHostName_, int iPortNumber_,
	int iMasterID_, int iSlaveID_, Socket::Family::Value eFamily_)
	: ClientConnection(iMasterID_, iSlaveID_),
	  m_cstrHostName(cstrHostName_),
	  m_iPortNumber(iPortNumber_),
	  m_eDefaultFamily(eFamily_)
{
}

//
//	FUNCTION public
//	Communication::RemoteClientConnection::~RemoteClientConnection
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
RemoteClientConnection::~RemoteClientConnection()
{
	close();
}

//
//	FUNCTION public
//	Communication::RemoteClientConnection::open -- オープン
//
//	NOTES
//	コネクトする
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
RemoteClientConnection::open()
{
	if (m_pSerialIO == 0)
	{
		int iMasterID;
		int iSlaveID;
		Socket* pSerialIO = 0;
		ModPair<ModUnicodeString, int> key(m_cstrHostName, m_iPortNumber);

		Socket::Family::Value family = m_eDefaultFamily;
		if (family == Socket::Family::Unspec)
		{
			Os::AutoCriticalSection cAuto(_latch);
			IPMap::Iterator i = _ipMap.find(key);
			if (i != _ipMap.end())
			{
				family = ((*i).second == true) ?
					Socket::Family::Only_v6 : Socket::Family::Only_v4;
			}
		}

		try
		{
			int retry = 0;
			for (;;)
			{
				try
				{
					pSerialIO
						= new Socket(m_cstrHostName.getString(
										 Common::LiteralCode),
									 m_iPortNumber, family);
					pSerialIO->open();
				}
#ifdef NO_CATCH_ALL
				catch (Exception::Object&)
#else
				catch (...)
#endif
				{
					// connect で失敗した場合は1度だけリトライする
					if (pSerialIO) delete pSerialIO;
					pSerialIO = 0;
					if (retry++ < 1)
					{
						Common::Thread::resetErrorCondition();
						if (m_eDefaultFamily == Socket::Family::Unspec &&
							family != Socket::Family::Unspec)
						{
							family = m_eDefaultFamily;
							Os::AutoCriticalSection cAuto(_latch);
							_ipMap.erase(key);
						}
						ModOsDriver::Thread::sleep(500);
						continue;
					}
					throw;
				}
				break;
			}
					
			if (family == Socket::Family::Unspec &&
				m_eDefaultFamily == Socket::Family::Unspec)
			{
				// マップに登録する
				Os::AutoCriticalSection cAuto(_latch);
				_ipMap.insert(key, pSerialIO->isIPv6());
			}
			
			// getMasterID()は暗号Bitが落ちてくるので完全IDを取得して渡す
			pSerialIO->writeInteger(getFullMasterID());// 暗号化対応
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
			if (pSerialIO)
			{
				pSerialIO->close();
				delete pSerialIO;
				pSerialIO = 0;
			}
			Common::Thread::resetErrorCondition();
			_TRMEISTER_THROW2(Exception::CannotConnect,
						   m_cstrHostName, m_iPortNumber);
		}

		setFullMasterID(iMasterID);	// 暗号化対応
		setSlaveID(iSlaveID);
		setSerialIO(pSerialIO);
		m_fOpen = true;
	}
}

//
//	FUNCTION public
//	Communication::RemoteClientConnection::close -- クローズ
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
RemoteClientConnection::close()
{
	if (m_pSerialIO)
	{
		m_pSerialIO->close();
		delete m_pSerialIO; m_pSerialIO = 0;
		m_fOpen = false;
	}
}

//
//	Copyright (c) 1999, 2002, 2004, 2005, 2006, 2011, 2013, 2015, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
