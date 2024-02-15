// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.cpp --
// 
// Copyright (c) 2005, 2006, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Vector2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Vector2/FileDriver.h"
#include "Vector2/LogicalInterface.h"

#include "LogicalFile/FileDriverTable.h"
#include "Os/Library.h"
#include "Os/AutoCriticalSection.h"

#include "Common/Assert.h"

#include "ModUnicodeString.h"

#ifndef SYD_CPU_SPARC
#ifndef SYD_DLL
#include "Vector/FileDriver.h"
#endif
#endif

_SYDNEY_USING
_SYDNEY_VECTOR2_USING

#ifndef SYD_CPU_SPARC
namespace
{
	//
	//	VARIABLE local
	//	_$$::_OldLibName
	//
	ModUnicodeString _OldLibName("SyDrvVct");

	//
	//	VARIABLE local
	//	_$$::_DBGetName
	//
	ModUnicodeString _DBGetName("DBGetFileDriver");
}
#endif

//
//	FUNCTION public
//	Vector2::FileDriver::FileDriver -- コンストラクタ
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
//
FileDriver::FileDriver()
#ifndef SYD_CPU_SPARC
	: m_pOldFileDriver(0)
#endif
{
}

//
//	FUNCTION public
//	Vector2::FileDriver::~FileDriver -- デストラクタ
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
//
FileDriver::~FileDriver()
{
#ifndef SYD_CPU_SPARC
	delete m_pOldFileDriver;
#endif
}

//
//	FUNCTION public
//	Vector2::FileDriver::initialize -- ファイルドライバを初期化する
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
}

//
//	FUNCTION public
//	Vector2::FileDriver::terminate -- ファイルドライバの後処理をする
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
#ifndef SYD_CPU_SPARC
	if (m_pOldFileDriver) m_pOldFileDriver->terminate();
#endif
}

//
//	FUNCTION public
//	Vector2::FileDriver::attachFile -- ファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルId
//
//	RETURN
//	LogicalFile::File*
//		ファイル
//
//	EXCEPTIONS
//
LogicalFile::File*
FileDriver::attachFile(const LogicalFile::FileID& cFileID_) const
{
#ifdef SYD_CPU_SPARC
	; _SYDNEY_ASSERT(FileID::checkVersion(cFileID_));
	return new LogicalInterface(cFileID_);
#else
	if (FileID::checkVersion(cFileID_) == true)
		return new LogicalInterface(cFileID_);
	return getOld()->attachFile(cFileID_);
#endif
}

//
//	FUNCTION public
//	Vector2::FileDriver::attachFile -- ファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::File* pSrcFile_
//		ファイル
//
//	RETURN
//	LogicalFile::File*
//		ファイル
//
//	EXCEPTIONS
//
LogicalFile::File*
FileDriver::attachFile(const LogicalFile::File*	pSrcFile_) const
{
#ifdef SYD_CPU_SPARC
	; _SYDNEY_ASSERT(FileID::checkVersion(pSrcFile_->getFileID()));
	return new LogicalInterface(pSrcFile_->getFileID());
#else
	if (FileID::checkVersion(pSrcFile_->getFileID()) == true)
		return new LogicalInterface(pSrcFile_->getFileID());
	return getOld()->attachFile(pSrcFile_);
#endif
}

//
//	FUNCTION public
//	Vector2::FileDriver::detachFile -- ファイルをデタッチする
//
//	NOTES
//
//	ARGUMENTS
//	LogicalFile::File* pFile_
//		ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileDriver::detachFile(LogicalFile::File* pFile_) const
{
	delete pFile_;
}

//
//	FUNCTION public
//	Vector2::FileDriver::getDriverID --
//		VectorファイルドライバのドライバIDを返す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Vectorファイルドライバのドライバ ID
//
//	EXCEPTIONS
//	なし
//
int
FileDriver::getDriverID() const
{
	return LogicalFile::FileDriverID::Vector;
}

//
//	FUNCTION public
//	Vector2::FileDriver::getDriverName --
//		Vectorファイルドライバのドライバ名を返す
//
//	NOTES
//  Vectorファイルドライバのドライバ名を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSting
//		Vectorファイルドライバのドライバ名
//
//	EXCEPTIONS
//	なし
//
ModString
FileDriver::getDriverName() const
{
	return LogicalFile::FileDriverName::Vector;
}

#ifndef SYD_CPU_SPARC
//
//	FUNCTION private
//	Vector2::FileDriver::getOld -- 旧ベクターファイルドライバを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::FileDriver*
//	   旧ベクターのファイルドライバ
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

		// 旧ベクターのファイルドライバを得る
		m_pOldFileDriver = (*f)();
#else
		// 旧ベクターのファイルドライバを得る
		m_pOldFileDriver = new Vector::FileDriver;
#endif
		// 初期化する
		m_pOldFileDriver->initialize();
	}
	return m_pOldFileDriver;
}

#endif

#ifdef SYD_DLL

//
//	FUNCTION global
//	DBGetFileDriver -- ベクターファイルのファイルドライバのインスタンスを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Logical::FileDriver*
//		ベクターファイルのファイルドライバ
//
LogicalFile::FileDriver*
DBGetFileDriver()
{
	return new Vector2::FileDriver;
}

#endif

//
//	Copyright (c) 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
