// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Thread.h -- スレッドを表すクラス
// 
// Copyright (c) 2000, 2002, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SYDTEST_THREAD_H
#define __SYDNEY_SYDTEST_THREAD_H

#include "Common/Common.h"
#include "Common/Internal.h"
#include "Common/Thread.h"
#include "SydTest/Executor.h"
#include "SydTest/Map.h"
#include "SydTest/Monitor.h"
#include "SydTest/CascadeConf.h"
#include "ModUnicodeString.h"
#include "ModCriticalSection.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace SydTest
{
//
//	CLASS
//	SydTest::Thread -- スレッドを表すクラス
//
//	NOTES
//  Sydneyに対する複数のスレッドを管理する
//  Common::Threadを継承し、ファイルに書かれた内容を実行する
//
class Thread : public Common::Thread
{
public:
	//コンストラクタ
	Thread(const char* pszFileName_, const char* pszLabel_,
		   int iOption_, Monitor& rMonitor_,
		   const ModUnicodeString& cstrHostName_, int iPortNumber_,
		   const ModUnicodeString& cstrRegPath_,
		   const ModVector<CascadeInfo>& cascadeInfoVector_,
		   const Communication::CryptMode::Value cryptMode_,
		   const ModUnicodeString& cstrUserName_,
		   const ModUnicodeString& cstrPassword_);
	//デストラクタ
	virtual ~Thread();
	static void createStatic(const char* pszFileName_, const char* pszLabel_, 
			   int iOption_, Monitor& rMonitor_,
			   const ModUnicodeString& cstrHostName_, int iPortNumber_,
			   const ModUnicodeString& cstrRegPath_,
			   const ModVector<CascadeInfo>& cascadeInfoVector_,
			   const Communication::CryptMode::Value cryptMode_,
			   const ModUnicodeString& cstrUserName_,
			   const ModUnicodeString& cstrPassword_);
	static void joinStatic(const char* pszLabel_);
	static bool exists(const char* pszLabel_);
	static Thread* getThread(const char* pszLabel_);

protected:
	// スレッドとして実行される内容
	void runnable();

private:
	Monitor& m_rMonitor;
	ModUnicodeString m_strFile;
	ModUnicodeString m_strLabel;
	int	   m_iOption;
	// スレッドの集合を管理するマップ
	static Map<ModUnicodeString, Thread*> m_mapThread;
	static ModCriticalSection m_cCS;

	//リモートサーバホスト名
	ModUnicodeString m_cstrHostName;
	int m_iPortNumber;

	//レジストリの親パス
	ModUnicodeString m_cstrRegPath;
		
	//子サーバ情報のベクター
	ModVector<CascadeInfo> m_cascadeInfoVector;

	// crypt mode
	Communication::CryptMode::Value m_cryptMode;

	// user name and password (passed by command option)
	ModUnicodeString m_cstrUserName;
	ModUnicodeString m_cstrPassword;
};

}

_SYDNEY_END

#endif //__SYDNEY_SYDTEST_THREAD_H

//
//	Copyright (c) 2000, 2002, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
