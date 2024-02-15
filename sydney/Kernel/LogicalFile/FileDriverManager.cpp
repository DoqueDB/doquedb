// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriverManager.cpp -- 論理ファイルドライバマネージャ
// 
// Copyright (c) 1999, 2001, 2002, 2005, 2007, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "LogicalFile";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "LogicalFile/FileDriver.h"
#include "LogicalFile/FileDriverManager.h"
#include "LogicalFile/FileDriverTable.h"
#include "LogicalFile/FileID.h"
#include "LogicalFile/OpenOption.h"
#include "LogicalFile/Externalizable.h"

#include "Common/UnicodeString.h"
#include "Os/AutoCriticalSection.h"
#include "Os/Library.h"
#include "Os/Unicode.h"

#ifndef SYD_DLL
#include "Array/FileDriver.h"
#include "Bitmap/FileDriver.h"
#ifndef SYD_CPU_SPARC
#include "Btree/FileDriver.h"
#endif
#include "Btree2/FileDriver.h"
#include "FullText/FileDriver.h"
#include "FullText2/FileDriver.h"
#include "KdTree/FileDriver.h"
#include "Lob/FileDriver.h"
#include "Record/FileDriver.h"
#ifndef SYD_CPU_SPARC
#include "Vector/FileDriver.h"
#endif
#include "Vector2/FileDriver.h"
#endif

_SYDNEY_USING
_SYDNEY_LOGICALFILE_USING

namespace {
	
	//	VARIABLE
	//	pszFunctionName -- ファイルドライバを得る関数名
	//
	//	NOTES
	//	ファイルドライバを得るライブラリの関数名

	const ModUnicodeString& pszFunctionName
		= _TRMEISTER_U_STRING("DBGetFileDriver");

}

//
//	VARIABLE private
//	LogicalFile::FileDriverManager::m_cCriticalSection -- クリティカルセクション
//
//	NOTES
//	排他制御用のクリティカルセクション。
//
Os::CriticalSection
FileDriverManager::m_cCriticalSection;

//
//	VARIABLE private
//	LogicalFile::FileDriverManager::m_mapFileDriverByID
//									-- ドライバIDと論理ファイルドライバのマップ
//
//	NOTES
//	ドライバIDと論理ファイルドライバのマップ
//
ModMap<int, FileDriver*, ModLess<int> >
FileDriverManager::m_mapFileDriverByID;

//
//	VARIABLE private
//	LogicalFile::FileDriverManager::m_mapFileDriverByName
//									-- ドライバ名と論理ファイルドライバのマップ
//
//	NOTES
//	ドライバ名の論理ファイルドライバのマップ
//
ModMap<ModString, FileDriver*, ModLess<ModString> >
FileDriverManager::m_mapFileDriverByName;

//
//	VARIABLE private
//	LogicalFile::FileDriverManager::m_iInitialized -- 初期化カウンタ
//
//	NOTES
//	初期化カウンタ
//
int
FileDriverManager::m_iInitialized = 0;

//
//	FUNCTION public
//	LogicalFile::FileDriverManager::initialize -- 初期化を行う
//
//	NOTES
//	初期化を行う
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
FileDriverManager::initialize()
{
	Os::AutoCriticalSection cAuto(m_cCriticalSection);
	if (m_iInitialized++ == 0)
	{
		// LogicalFileに属するExternalizableオブジェクトを得るための関数を
		// Commonに登録する
		Common::Externalizable::insertFunction(
			Common::Externalizable::LogicalFileClasses,
			Externalizable::getClassInstance);
		FileID::initialize();
		OpenOption::initialize();
	}
}

//
//	FUNCTION public
//	LogicalFile::FileDriverManager::terminate -- 後処理を行う
//
//	NOTES
//	後処理を行う。
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
FileDriverManager::terminate()
{
	Os::AutoCriticalSection cAuto(m_cCriticalSection);
	if (--m_iInitialized == 0)
	{
		//後処理を行う
		ModMap<int, FileDriver*, ModLess<int> >::Iterator i;
		for (i = m_mapFileDriverByID.begin();
			 i != m_mapFileDriverByID.end(); ++i)
		{
			(*i).second->terminate();
			delete (*i).second;
		}
		m_mapFileDriverByID.erase(m_mapFileDriverByID.begin(),
								  m_mapFileDriverByID.end());
		m_mapFileDriverByName.erase(m_mapFileDriverByName.begin(),
									m_mapFileDriverByName.end());
	}
}

//
//	FUNCTION public
//	LogicalFile::FileDriverManager::prepareTerminate -- 後処理の準備を行う
//
//	NOTES
//	後処理を行う。
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
FileDriverManager::prepareTerminate()
{
	if (m_iInitialized - 1 == 0)//最後のterminate呼び出しの直前
	{
		//後処理を行う
		ModMap<int, FileDriver*, ModLess<int> >::Iterator i;
		for (i = m_mapFileDriverByID.begin();
			 i != m_mapFileDriverByID.end(); ++i)
		{
			(*i).second->prepareTerminate();
		}
	}
}

//	FUNCTION public
//	LogicalFile::FileDriverManager::getDriver -- 論理ファイルドライバを得る(1)
//
//	NOTES
//	ドライバIDから論理ファイルドライバを得る。
//
//	ARGUMENTS
//	int iDriverID_
//		ドライバID
//
//	RETURN
//	LogicalFile::FileDriver*
//		論理ファイルドライバへのポインタ
//
//	EXCEPTIONS

FileDriver*
FileDriverManager::getDriver(int iDriverID_)
{
	Os::AutoCriticalSection cAuto(m_cCriticalSection);
	FileDriver* pFileDriver;
	//まずマップを検索する
	ModMap<int, FileDriver*, ModLess<int> >::Iterator i;
	i = m_mapFileDriverByID.find(iDriverID_);
	if (i == m_mapFileDriverByID.end())
	{
		//
		//	見つからないのでライブラリをロードする
		//

#ifdef SYD_DLL
		// ライブラリ名を得る

		const Os::UnicodeString libName(
			FileDriverTable::getLibraryName(iDriverID_));

		// ライブラリをロードする

		Os::Library::load(libName);

		//エントリ関数のポインタを得る
		//	void* -> 関数へのキャストは
		//	static_cast も syd_reinterpret_cast もエラーになるので、
		//	旧スタイルのキャストを用いる。

		FileDriver* (*f)(void);
		f = (FileDriver*(*)(void))
			Os::Library::getFunction(libName, pszFunctionName);

		//論理ファイルドライバを得る
		pFileDriver = (*f)();
#else
		switch (iDriverID_)
		{
		case FileDriverID::Record:
			pFileDriver = new Record::FileDriver;
			break;
		case FileDriverID::Btree:
#ifdef SYD_CPU_SPARC
			pFileDriver = new Btree2::FileDriver;
#else
			if (FileDriverTable::getLibraryName(iDriverID_)
				== "SyDrvBtr")
				pFileDriver = new Btree::FileDriver;
			else
				pFileDriver = new Btree2::FileDriver;
#endif
			break;
		case FileDriverID::FullText:
			if (FileDriverTable::getLibraryName(iDriverID_)
				== "SyDrvFts")
				pFileDriver = new FullText::FileDriver;
			else
				pFileDriver = new FullText2::FileDriver;
			break;
		case FileDriverID::Vector:
#ifdef SYD_CPU_SPARC
			pFileDriver = new Vector2::FileDriver;
#else
			if (FileDriverTable::getLibraryName(iDriverID_)
				== "SyDrvVct")
				pFileDriver = new Vector::FileDriver;
			else
				pFileDriver = new Vector2::FileDriver;
#endif
			break;
		case FileDriverID::Lob:
			pFileDriver = new Lob::FileDriver;
			break;
		case FileDriverID::Bitmap:
			pFileDriver = new Bitmap::FileDriver;
			break;
		case FileDriverID::Array:
			pFileDriver = new Array::FileDriver;
			break;
		case FileDriverID::KdTree:
			pFileDriver = new KdTree::FileDriver;
			break;
		}
#endif
		pFileDriver->initialize();
		//マップに格納する
		m_mapFileDriverByID.insert(pFileDriver->getDriverID(),
								   pFileDriver);
		m_mapFileDriverByName.insert(pFileDriver->getDriverName(),
									 pFileDriver);
	}
	else
	{
		//見つかったのでそれを返す
		pFileDriver = (*i).second;
	}

	return pFileDriver;
}

//	FUNCTION public
//	LogicalFile::FileDriverManager::getDriver -- 論理ファイルドライバを得る(2)
//
//	NOTES
//	ドライバ名から論理ファイルドライバを得る。
//
//	ARGUMENTS
//	const ModString& cstrDriverName_
//		ドライバ名
//
//	RETURN
//	LogicalFile::FileDriver*
//		論理ファイルドライバへのポインタ
//
//	EXCEPTIONS

#ifndef SYD_COVERAGE
FileDriver*
FileDriverManager::getDriver(const ModString& cstrDriverName_)
{
	Os::AutoCriticalSection cAuto(m_cCriticalSection);

	FileDriver* pFileDriver;
	//まずマップを検索する
	ModMap<ModString, FileDriver*, ModLess<ModString> >::
		Iterator i;
	i = m_mapFileDriverByName.find(cstrDriverName_);
	if (i == m_mapFileDriverByName.end())
	{
		//
		//	見つからないのでライブラリをロードする
		//

#ifdef SYD_DLL
		// ライブラリ名を得る

		const Os::UnicodeString libName(
			FileDriverTable::getLibraryName(cstrDriverName_));

		// ライブラリをロードする

		Os::Library::load(libName);

		//エントリ関数のポインタを得る
		//	void* -> 関数へのキャストは
		//	static_cast も syd_reinterpret_cast もエラーになるので、
		//	旧スタイルのキャストを用いる。

		FileDriver* (*f)(void);
		f = (FileDriver*(*)(void))
			Os::Library::getFunction(libName, pszFunctionName);

		//論理ファイルドライバを得る
		pFileDriver = (*f)();
#else
		if (cstrDriverName_ == FileDriverName::Record)
		{
			pFileDriver = new Record::FileDriver;
		}
		else if (cstrDriverName_ == FileDriverName::Btree)
		{
#ifdef SYD_CPU_SPARC
			pFileDriver = new Btree2::FileDriver;
#else
			if (FileDriverTable::getLibraryName(cstrDriverName_) == "SyDrvBtr")
				pFileDriver = new Btree::FileDriver;
			else
				pFileDriver = new Btree2::FileDriver;
#endif
		}
		else if (cstrDriverName_ == FileDriverName::FullText)
		{
			pFileDriver = new FullText::FileDriver;
		}
		else if (cstrDriverName_ == FileDriverName::Vector)
		{
#ifdef SYD_CPU_SPARC
			pFileDriver = new Vector::FileDriver;
#else
			if (FileDriverTable::getLibraryName(cstrDriverName_) == "SyDrvVct")
				pFileDriver = new Vector::FileDriver;
			else
				pFileDriver = new Vector2::FileDriver;
#endif
		}
		else if (cstrDriverName_ == FileDriverName::Lob)
		{
			pFileDriver = new Lob::FileDriver;
		}
		else if (cstrDriverName_ == FileDriverName::Bitmap)
		{
			pFileDriver = new Bitmap::FileDriver;
		}
		else if (cstrDriverName_ == FileDriverName::Array)
		{
			pFileDriver = new Array::FileDriver;
		}
		else if (cstrDriverName_ == FileDriverName::KdTree)
		{
			pFileDriver = new KdTree::FileDriver;
		}
#endif
		pFileDriver->initialize();
		//マップに格納する
		m_mapFileDriverByID.insert(pFileDriver->getDriverID(),
								   pFileDriver);
		m_mapFileDriverByName.insert(pFileDriver->getDriverName(),
									 pFileDriver);
	}
	else
	{
		//見つかったのでそれを返す
		pFileDriver = (*i).second;
	}

	return pFileDriver;
}
#endif // end of #ifndef SYD_COVERAGE

//
//	FUNCTION public static
//	LogicalFile::FileDriverManager::undo -- UNDO する
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTransaction_
//		トランザクション
//	const LogicalFile::LogData& cLogData_
//		論理ログ
//	Schema::Database& cDatabase_
//		データベース
//
// 	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileDriverManager::undo(Trans::Transaction& cTransaction_,
						const LogData& cLogData_,
						Schema::Database& cDatabase_)
{
	// 何もしない
}

//
//	FUNCTION public static
//	LogicalFile::FileDriverManager::redo -- REDO する
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTransaction_
//		トランザクション
//	const LogicalFile::LogData& cLogData_
//		論理ログ
//	Schema::Database& cDatabase_
//		データベース
//
// 	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileDriverManager::redo(Trans::Transaction& cTransaction_,
						const LogData& cLogData_,
						Schema::Database& cDatabase_)
{
	// 何もしない
}

//
//	Copyright (c) 1999, 2001, 2002, 2005, 2007, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
