// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ConnectionKeeper.cpp -- 接続したシリアルIOを管理する。
// 
// Copyright (c) 1999, 2001, 2004, 2005, 2006, 2007, 2010, 2011, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
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

#include "Communication/Connection.h"
#include "Communication/ConnectionKeeper.h"
#include "Communication/SerialIO.h"
#include "Communication/ConnectionSlaveID.h"
#include "Communication/CryptMode.h"
#include "Communication/AuthorizeMode.h"

#include "Exception/GoingShutdown.h"
#include "Exception/ConnectionClosed.h"
#include "Exception/ConnectionRanOut.h"
#include "Exception/UserLevel.h"

#include "Common/Message.h"

#include "Os/AutoCriticalSection.h"

_TRMEISTER_USING
using namespace Communication;

//
//	FUNCTION public
//	Communication::ConnectionKeeper::ConnectionKeeper
//													-- コンストラクタ
//
//	NOTES
//	コンストラクタ
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
ConnectionKeeper::ConnectionKeeper()
: m_fShutdown(false)
{
}

//
//	FUNCTION public
//	Communication::ConnectionKeeper::~ConnectionKeeper
//													-- デストラクタ
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
ConnectionKeeper::~ConnectionKeeper()
{
}

//
//	FUNCTION public
//	Communication::ConnectionKeeper::initialize -- 初期化
//
//	NOTES
//	初期化を行う。今のところ何も行わない。
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
ConnectionKeeper::initialize()
{
}

//
//	FUNCTION public
//	Communication::ConnectioKeeper::terminate -- 後処理
//
//	NOTES
//	後処理を行う。今のところ何も行わない。
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
ConnectionKeeper::terminate()
{
	for (Map::Iterator i = m_mapSerialIO.begin(); i != m_mapSerialIO.end(); ++i)
	{
		ModVector<ModPair<int, SerialIO*> >::Iterator ii = (*i).second.begin();
		for (; ii != (*i).second.end(); ++ii)
		{
			(*ii).second->close();
			delete (*ii).second;
		}
	}
	m_mapSerialIO.erase(m_mapSerialIO.begin(), m_mapSerialIO.end());
	
	for (ServerMap::Iterator j = m_mapWaitServer.begin();
		 j != m_mapWaitServer.end(); ++j)
	{
		delete (*j).second;
	}
	m_mapWaitServer.erase(m_mapWaitServer.begin(), m_mapWaitServer.end());
}

//
//	FUNCTION public
//	Communication::ConnectionKeeper::consumeConnection
//										-- 対応するシリアルIOを取り出す
//
//	NOTES
//	マップから対応するシリアルIOを取り出す
//
//	ARGUMENTS
//	Communication::Connection* pConnection_
//		対応するコネクション
//
//	RETURN
//	Communication::SerialIO*
//		シリアルIOオブジェクト。
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
SerialIO*
ConnectionKeeper::consumeConnection(Connection* pConnection_)
{
	SerialIO* pSerialIO = 0;
	Os::AutoTryCriticalSection cLock(m_cCriticalSection, false);
	cLock.lock();
	if (m_fShutdown == true)
	{
		//シャットダウン中なので例外を投げる
		_TRMEISTER_THROW0(Exception::GoingShutdown);
	}
	
	while (1)
	{
		Map::Iterator i = m_mapSerialIO.find(pConnection_->getSlaveID());
		if (i != m_mapSerialIO.end())
		{
			//見つかった
			int iClientID = (*i).second[0].first;
			pSerialIO = (*i).second[0].second;
			
			//
			// プロトコルバージョンを合わせる
			//	マップに格納されているマスターIDはクライアントから来た
			//	プロトコルバージョンである。サーバの方が大きい場合には
			//	サーバ側のマスターIDを書き換える
			//

			int iMasterID = 0;
			// getMasterID()は暗号Bitが落ちてくるので完全IDを取得して渡す
			int iServerID = pConnection_->getFullMasterID();
			
			if (iClientID != iServerID)
			{
				// プロトコルバージョン
				if ((iClientID & CryptMode::MaskMasterID)
					< (iServerID & CryptMode::MaskMasterID))
				{
					iMasterID = iClientID & CryptMode::MaskMasterID;
				}
				else
				{
					iMasterID = iServerID & CryptMode::MaskMasterID;
				}
				
				// 暗号アルゴリズム
				if ((iClientID & CryptMode::MaskAlgorithm)
					< (iServerID & CryptMode::MaskAlgorithm))
				{
					iMasterID = iMasterID
						| (iClientID & CryptMode::MaskAlgorithm);
				}
				else
				{
					iMasterID = iMasterID
						| (iServerID & CryptMode::MaskAlgorithm);
				}
				
				// 認証方式
				if ((iClientID & AuthorizeMode::MaskMode)
					< (iServerID & AuthorizeMode::MaskMode))
				{
					iMasterID = iMasterID
						| (iClientID & AuthorizeMode::MaskMode);
				}
				else
				{
					iMasterID = iMasterID
						| (iServerID & AuthorizeMode::MaskMode);
				}
				
				pConnection_->setFullMasterID(iMasterID);
			}
			
			// 削除する
			(*i).second.popFront();
			if ((*i).second.getSize() == 0)
				m_mapSerialIO.erase(i);
			
			// 自分が待っていたサーバかどうか検索してみる
			ServerMap::Iterator ii =
				m_mapWaitServer.find(pConnection_->getSlaveID());
			if (ii != m_mapWaitServer.end())
			{
				// 自分が待っているサーバだったので、エントリを削除する
				delete (*ii).second;
				m_mapWaitServer.erase(ii);
			}
			
			break;
		}

		//ないので、待つ
		ModConditionVariable* cond
			= new ModConditionVariable(ModFalse, ModFalse);
		ModPair<ServerMap::Iterator, ModBoolean> result
			= m_mapWaitServer.insert(pConnection_->getSlaveID(), cond);
		cLock.unlock();
		if (pConnection_->getSlaveID() != ConnectionSlaveID::Any)
		{
			if (cond->wait(ModTimeSpan(60,0)) == false)
			{
				// 1分待ってもクライアントが接続に来なかった
				
				cLock.lock();
				ServerMap::Iterator ii
					= m_mapWaitServer.find(pConnection_->getSlaveID());
				if (ii != m_mapWaitServer.end())
				{
					// 自分が待っているサーバだったので、エントリを削除する
					delete (*ii).second;
					m_mapWaitServer.erase(ii);
				}
				//タイムアウトに挿入する
				m_mapTimeout.insert(pConnection_->getSlaveID(), 0);
				// 例外をなげる
				_TRMEISTER_THROW0(Exception::ConnectionRanOut);
			}
		}
		else
		{
			//永遠に待つ
			cond->wait();
		}
		if (m_fShutdown == true)
		{
			//シャットダウン中なので例外を投げる
			_TRMEISTER_THROW0(Exception::GoingShutdown);
		}
		cLock.lock();
	}

	return pSerialIO;
}

//
//	FUNCTION public
//	Communication::ConnectionKeeper::execute -- シリアルIOをマップにしまう
//
//	NOTES
//	シリアルIOをマップにしまう
//
//	ARGUMENTS
//	Communication::SerialIO* pSerialIO_
//		シリアルIO
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ConnectionKeeper::execute(SerialIO* pSerialIO_, bool isCrypt_)
{
	if (pSerialIO_)
	{
		pSerialIO_->open();
		int iMasterID;
		int iSlaveID;
		try
		{
			if (pSerialIO_->wait(5000) == false)
			{
				// 5秒まったが、クライアントがデータを送ってこないので、
				// あきらめる

				_SYDNEY_THROW0(Exception::ConnectionRanOut);
			}
			
			iMasterID = pSerialIO_->readInteger();	//MasterID
			iSlaveID = pSerialIO_->readInteger();	//SlaveID
			
			if (isCrypt_ && !(iMasterID & CryptMode::MaskAlgorithm))
			{
				// 暗号化必須なのに、暗号化で接続していない
				_SYDNEY_THROW0(Exception::ConnectionClosed);
			}
		}
		catch (Exception::UserLevel&)
		{
			// メッセージは出力しない
			pSerialIO_->close();
			delete pSerialIO_;
			throw;
		}
		catch (Exception::Object&)
		{
			SydInfoMessage << "recv failed. client connection is aborted."
						   << ModEndl;
			pSerialIO_->close();
			delete pSerialIO_;
			throw;
		}
#ifndef NO_CATCH_ALL
		catch (...)
		{
			SydInfoMessage << "recv failed. client connection is aborted."
						   << ModEndl;
			pSerialIO_->close();
			delete pSerialIO_;
			throw;
		}
#endif
		
		{
			Os::AutoCriticalSection cLock(m_cCriticalSection);
			TimeoutMap::Iterator j = m_mapTimeout.find(iSlaveID);
			if (j != m_mapTimeout.end())
			{
				m_mapTimeout.erase(j);
				pSerialIO_->close();
				delete pSerialIO_;
				return;
			}
			m_mapSerialIO[iSlaveID].pushBack(
				ModPair<int, SerialIO*>(iMasterID, pSerialIO_));
			// 待っているサーバがあるかどうかチェックする
			ServerMap::Iterator i = m_mapWaitServer.find(iSlaveID);
			if (i != m_mapWaitServer.end())
			{
				// 待っているのがいるので、シグナルを送る
				(*i).second->signal();
			}
		}
	}
	else
	{
		Os::AutoCriticalSection cLock(m_cCriticalSection);
		//終了要求が来たらしい
		m_fShutdown = true;
		// 待っているすべてのサーバにシグナルを送る
		for (ServerMap::Iterator i = m_mapWaitServer.begin();
			 i != m_mapWaitServer.end(); ++i)
		{
			(*i).second->signal();
		}
	}
}

//
//	Copyright (c) 1999, 2001, 2004, 2005, 2006, 2007, 2010, 2011, 2013, 2014, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
