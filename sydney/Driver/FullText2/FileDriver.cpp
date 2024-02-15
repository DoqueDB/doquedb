// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.cpp --
// 
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/FileDriver.h"
#include "FullText2/LogicalInterface.h"
#include "FullText2/MergeDaemon.h"

#include "LogicalFile/File.h"
#include "LogicalFile/FileDriverTable.h"

#include "Os/Library.h"
#include "Os/Unicode.h"

#ifndef SYD_DLL
#include "FullText/FileDriver.h"
#endif

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$::_OldLibName
	//
	ModUnicodeString _OldLibName("SyDrvFts");

	//
	//	VARIABLE local
	//	_$$::_DBGetName
	//
	ModUnicodeString _DBGetName("DBGetFileDriver");
}

//
//	FUNCTION public
//	FullText2::FileDriver::FileDriver -- コンストラクタ
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
//	なし
//
FileDriver::FileDriver()
	: m_pMergeDaemon(0), m_pOldFileDriver(0), m_iCounter(0)
{
}

//
//	FUNCTION public
//	FullText2::FileDriver::~FileDriver -- デストラクタ
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
//	なし
//
FileDriver::~FileDriver()
{
	delete m_pOldFileDriver;
}

//
//	FUNCTION public
//	FullText2::FileDriver::initialize -- 論理ファイルドライバの初期化を行う
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
FileDriver::initialize()
{
	Os::AutoCriticalSection cAuto(m_cLock);

	m_pMergeDaemon = new MergeDaemon;
	m_pMergeDaemon->create();
	if (m_pOldFileDriver) m_pOldFileDriver->initialize();
}

//
//	FUNCTION public
//	FullText2::FileDriver::terminate -- 論理ファイルドライバの後処理を行う
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
FileDriver::terminate()
{
	Os::AutoCriticalSection cAuto(m_cLock);

	delete m_pMergeDaemon, m_pMergeDaemon = 0;
	if (m_pOldFileDriver) m_pOldFileDriver->terminate();
}


//
//	FUNCTION public
//	FullText2::FileDriver::stop -- 論理ファイルドライバの処理を止める
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
FileDriver::stop()
{
	Os::AutoCriticalSection cAuto(m_cLock);

	if (m_pMergeDaemon)
	{
		m_pMergeDaemon->abort();
		m_pMergeDaemon->join();
		delete m_pMergeDaemon;
		m_pMergeDaemon = 0;
		if (m_pOldFileDriver) m_pOldFileDriver->stop();

	}
	
	++m_iCounter;
}

//
//	FUNCTION public
//	FullText2::FileDriver::start -- 論理ファイルドライバの処理を再開する
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
FileDriver::start()
{
	Os::AutoCriticalSection cAuto(m_cLock);

	if (m_iCounter == 1)
	{
		m_pMergeDaemon = new MergeDaemon;
		m_pMergeDaemon->create();
		if (m_pOldFileDriver) m_pOldFileDriver->start();
	}

	--m_iCounter;
}

//
//	FUNCTION public
//	FullText2::FileDriver::prepareTerminate
//		-- 論理ファイルドライバの後処理の前準備を行う
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
FileDriver::prepareTerminate()
{
	Os::AutoCriticalSection cAuto(m_cLock);

	m_pMergeDaemon->abort();
	m_pMergeDaemon->join();
	if (m_pOldFileDriver) m_pOldFileDriver->prepareTerminate();
}

//
//	FUNCTION public
//	FullText2::FileDriver::attachFile -- 論理ファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID クラス
//
//	RETURN
//	LogicalFile::File*
//		ファイルドライバクラスポインタ
//
//	EXCEPTIONS
//
LogicalFile::File* 
FileDriver::attachFile(const LogicalFile::FileID& cFileID_) const
{
	if (FileID::checkVersion(cFileID_) == false)
		return getOld()->attachFile(cFileID_);
	return new LogicalInterface(cFileID_);
}

//
//	FUNCTION public
//	FullText2::FileDriver::attachFile -- 論理ファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::File* pFile_
//		ファイル記述子 クラス
//
//	RETURN
//	LogicalFile::File*
//		ファイルドライバクラスポインタ
//
//	EXCEPTIONS
//
LogicalFile::File* 
FileDriver::attachFile(const LogicalFile::File* pFile_) const
{
	if (FileID::checkVersion(pFile_->getFileID()) == false)
		return getOld()->attachFile(pFile_->getFileID());
	return new LogicalInterface(pFile_->getFileID());
}

//
//	FUNCTION public
//	FullText2::FileDriver::detachFile -- 論理ファイルをデタッチする
//
//	NOTES
//
//	ARGUMENTS
//	LogicalFile::File* pFile_
//		ファイルポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
FileDriver::detachFile(LogicalFile::File* pFile_) const
{
	delete pFile_;
}

//
//	FUNCTION public
//	FullText2::FileDriver::getDriverID -- 論理ファイルドライバのドライバIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		論理ファイルドライバID
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
int
FileDriver::getDriverID() const
{
	return LogicalFile::FileDriverID::FullText;
}

//
//	FUNCTION public
//	FullText2::FileDriver::getDriverName
//		-- 論理ファイルドライバのドライバ名を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModString
//		ドライバ名
//
//	EXCEPTIONS
//
ModString
FileDriver::getDriverName() const
{
	return LogicalFile::FileDriverName::FullText;
}

//
//	FUNCTION private
//	FullText2::FileDriver::getOld -- 旧全文ファイルドライバを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::FileDriver*
//	   旧全文ファイルドライバ
//
//	EXCEPTIONS
//
LogicalFile::FileDriver*
FileDriver::getOld() const
{
	Os::AutoCriticalSection cAuto(m_cLock);
	if (m_pOldFileDriver == 0)
	{
#ifdef SYD_DLL
		// ライブラリをロードする
		Os::Library::load(_OldLibName);

		// DBGetFileDriver関数を得る
		LogicalFile::FileDriver* (*f)(void);
		f = (LogicalFile::FileDriver*(*)(void))
			Os::Library::getFunction(_OldLibName, _DBGetName);

		// 全文ファイルドライバを得る
		m_pOldFileDriver = (*f)();
#else
		// 全文ファイルドライバを得る
		m_pOldFileDriver = new FullText::FileDriver;
#endif
		// 初期化する
		m_pOldFileDriver->initialize();
	}
	return m_pOldFileDriver;
}

//
//	Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
