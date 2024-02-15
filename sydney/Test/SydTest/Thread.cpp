// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Thread.cpp -- スレッドを表すクラス
// 
// Copyright (c) 2000, 2001, 2002, 2007, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "SydTest";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "ModAutoMutex.h"
#include "SydTest/Thread.h"
#include "SydTest/SydTestException.h"
#include "SydTest/SydTestMessage.h"
#include "Common/ExceptionMessage.h"
#include "Exception/Object.h"

_SYDNEY_USING

using namespace SydTest;

Map<ModUnicodeString, Thread*> Thread::m_mapThread;
ModCriticalSection Thread::m_cCS;

//
//	FUNCTION public
//	SydTest::Thread::Thread -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
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
Thread::Thread(const char* pszFileName_,
			   const char* pszLabel_, int iOption_,
			   Monitor& rMonitor_,
			   const ModUnicodeString& cstrHostName_,
			   int iPortNumber_,
			   const ModUnicodeString& cstrRegPath_,
			   const ModVector<CascadeInfo>& cascadeInfoVector_,
			   const Communication::CryptMode::Value cryptMode_,
			   const ModUnicodeString& cstrUserName_,
			   const ModUnicodeString& cstrPassword_)
: m_strFile(pszFileName_), m_strLabel(pszLabel_),
  m_iOption(iOption_), m_rMonitor(rMonitor_),
  m_cstrHostName(cstrHostName_), m_iPortNumber(iPortNumber_),
  m_cstrRegPath(cstrRegPath_), m_cryptMode(cryptMode_),
  m_cascadeInfoVector(cascadeInfoVector_),
  m_cstrUserName(cstrUserName_), m_cstrPassword(cstrPassword_)
{
}

//
//	FUNCTION public
//	SydTest::Thread::~Thread -- デストラクタ
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
Thread::~Thread()
{
}

//
//	FUNCTION public
//	SydTest::Thread::runnable -- スレッドの実行ルーチン
//
//	NOTES
//	スレッドの実行ルーチン
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
Thread::runnable()
{
	Executor cExecutor(m_strFile.getString(), 
		m_strLabel.getString(), m_iOption, m_rMonitor, 
		m_cstrHostName, m_iPortNumber, m_cstrRegPath, m_cascadeInfoVector,
		m_cryptMode, m_cstrUserName, m_cstrPassword);
	cExecutor.executeThread(m_strLabel.getString());
}

//
//  static
//	FUNCTION public
//	SydTest::Thread::createStatic -- スレッドをstaticに生成する
//
//	NOTES
//	スレッドをstaticに生成する
//
//	ARGUMENTS
//    const char* pszFileName
//      スクリプトファイル名
//    const char* pszLabel_
//      スレッド名
//	  int iOption_
//      SydTestの実行オプション
//    Monitor& rMonitor_
//      タイムアップのためのモニタ
//	  const ModUnicodeString& cstrHostName_
//      接続先のホスト名
//    int iPortNumber_
//      ポート番号
//	  const ModUnicodeString& cstrUserName_
//	  const ModUnicodeString& cstrPassword_
//	RETURN
//	なし
//
//	EXCEPTIONS
//	SydTestException
//
void
Thread::createStatic(const char* pszFileName_, const char* pszLabel_,
					 int iOption_, Monitor& rMonitor_,
					 const ModUnicodeString& cstrHostName_, int iPortNumber_,
					 const ModUnicodeString& cstrRegPath_,
					 const ModVector<CascadeInfo>& cascadeInfoVector_,
					 const Communication::CryptMode::Value cryptMode_,
					 const ModUnicodeString& cstrUserName_,
					 const ModUnicodeString& cstrPassword_)
{
	// 同じスレッドを同時に複数回実行しない
	if (m_mapThread.exists(pszLabel_))
	{
		SydTestErrorMessage << "Thread " << pszLabel_
			<< " already runs." << ModEndl;
		throw SydTestException();
	}
	try 
	{
		Thread* pThread = new Thread(pszFileName_, pszLabel_, iOption_, rMonitor_, 
			cstrHostName_, iPortNumber_, cstrRegPath_, cascadeInfoVector_, cryptMode_, cstrUserName_, cstrPassword_);
		m_mapThread.insert(pszLabel_, pThread);
		pThread->create();
	}
	catch (Exception::Object& err)
	{
		SydTestErrorMessage << err << ModEndl;
	}
}

//
//  static
//	FUNCTION public
//	SydTest::Thread::createStatic -- スレッドをstaticに終了する
//
//	NOTES
//	スレッドをstaticに終了する
//
//	ARGUMENTS
//    const char* pszLabel_
//      スレッド名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	SydTestException
//
void
Thread::joinStatic(const char* pszLabel_)
{
	if (!m_mapThread.exists(pszLabel_))
	{
		SydTestErrorMessage << "No such thread " 
			<< pszLabel_ << "." << ModEndl;
		throw SydTestException();
	}
	try
	{
//		ModAutoMutex<ModCriticalSection> cAuto(&m_cCS);
//		cAuto.lock();
		m_mapThread[pszLabel_]->join();
		m_mapThread.erase(pszLabel_);
	}
	catch (Exception::Object& err)
	{
		SydTestErrorMessage << err << ModEndl;
	}
}

//
//  static
//	FUNCTION public
//	SydTest::Thread::exists -- スレッドの存在を調べる
//
//	NOTES
//	スレッドの存在を調べる
//
//	ARGUMENTS
//    const char* pszLabel_
//      スレッド名
//
//	RETURN
//	bool
//    true: 存在する、false: 存在しない
//
//	EXCEPTIONS
//	なし
//
bool
Thread::exists(const char* pszLabel_)
{
	return m_mapThread.exists(pszLabel_);
}

//
//  static
//	FUNCTION public
//	SydTest::Thread::getThread -- 指定したスレッドへのポインタを返す
//
//	NOTES
//	指定したスレッドへのポインタを返す
//
//	ARGUMENTS
//    const char* pszLabel_
//      スレッド名
//
//	RETURN
//	  Thread*
//      スレッドへのポインタ
//
//	EXCEPTIONS
//	なし
//
Thread*
Thread::getThread(const char* pszLabel_)
{
	return m_mapThread[pszLabel_];
}

//
//	Copyright (c) 2000, 2001, 2002, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
