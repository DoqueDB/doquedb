// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.cpp --
// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Bitmap";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "LogicalFile/FileDriverTable.h"
#include "LogicalFile/FileID.h"
#include "LogicalFile/File.h"

#include "Bitmap/FileDriver.h"
#include "Bitmap/LogicalInterface.h"

#include "Os/Library.h"
#include "Os/Unicode.h"

#ifndef SYD_DLL
#include "Btree/FileDriver.h"
#endif

_SYDNEY_USING
_SYDNEY_BITMAP_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$::_OldLibName
	//
	ModUnicodeString _OldLibName("SyDrvBtr");

	//
	//	VARIABLE local
	//	_$$::_DBGetName
	//
	ModUnicodeString _DBGetName("DBGetFileDriver");
}

//
//	FUNCTION public
//	Bitmap::FileDriver::FileDriver -- コンストラクタ
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
{
}

//
//	FUNCTION public
//	Bitmap::FileDriver::~FileDriver -- デストラクタ
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
}

//
//	FUNCTION public
//	Bitmap::FileDriver::initialize -- ファイルドライバを初期化する
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
//	Bitmap::FileDriver::terminate -- ファイルドライバの後処理をする
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
}

//
//	FUNCTION public
//	Bitmap::FileDriver::attachFile -- ファイルをアタッチする
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
	return new LogicalInterface(cFileID_);
}

//
//	FUNCTION public
//	Bitmap::FileDriver::attachFile -- ファイルをアタッチする
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
	return new LogicalInterface(pSrcFile_->getFileID());
}

//
//	FUNCTION public
//	Bitmap::FileDriver::detachFile -- ファイルをデタッチする
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
//	Bitmap::FileDriver::getDriverID --
//		ファイルドライバのドライバIDを返す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		ファイルドライバのドライバ ID
//
//	EXCEPTIONS
//	なし
//
int
FileDriver::getDriverID() const
{
	return LogicalFile::FileDriverID::Bitmap;
}

//
//	FUNCTION public
//	Bitmap::FileDriver::getDriverName --
//		ファイルドライバのドライバ名を返す
//
//	NOTES
//  ファイルドライバのドライバ名を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSting
//		Btreeファイルドライバのドライバ名
//
//	EXCEPTIONS
//	なし
//
ModString
FileDriver::getDriverName() const
{
	return LogicalFile::FileDriverName::Bitmap;
}

#ifdef SYD_DLL

//
//	FUNCTION global
//	DBGetFileDriver -- ファイルドライバのインスタンスを得る
//
//	NOTES
//	ファイルドライバのインスタンスを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Logical::FileDriver*
//		Ｂ木ファイルのファイルドライバ
//
LogicalFile::FileDriver*
DBGetFileDriver()
{
	return new FileDriver;
}

#endif

//
//	Copyright (c) 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
