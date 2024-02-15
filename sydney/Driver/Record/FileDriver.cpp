// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.cpp -- 可変長レコードファイルの論理ファイルドライバクラス
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Record";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Record/FileDriver.h"

#include "LogicalFile/FileDriverTable.h"
#include "LogicalFile/FileID.h"
#include "Exception/NotSupported.h"
#include "Record/File.h"

_SYDNEY_USING

using namespace Record;

//
//	FUNCTION public
//	Record::FileDriver::FileDriver -- コンストラクタ
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
FileDriver::FileDriver()
	: LogicalFile::FileDriver()
{
}

//
//	FUNCTION public
//	Record::FileDriver::~FileDriver -- デストラクタ
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
FileDriver::~FileDriver()
{
}

//
//	FUNCTION public void
//	Record::FileDriver::initialize -- レコードファイルドライバの初期化を行なう
//	NOTES
//		レコードファイルドライバの初期化を行なう。
//	ARGUMETNS
//		なし
//	RETURN
//		なし
//	EXCEPTIONS
//
void
FileDriver::initialize()
{
	Record::File::initialize();
}

//
//	FUNCTION public void
//	Record::FileDriver::terminate -- レコードファイルドライバの後処理を行なう
//	NOTES
//		レコードファイルドライバの後処理を行なう。
//	ARGUMETNS
//		なし
//	RETURN
//		なし
//	EXCEPTIONS
//
void
FileDriver::terminate()
{
	Record::File::terminate();
}

//
//	FUNCTION public
//	Record::FileDriver::attachFile -- 可変長レコードファイルをアタッチする
//
//	NOTES
//	可変長レコードファイルをアタッチする。
//
//	ARGUMENTS
//	const LogicalFile::FileID&	cFileOption_
//		可変長レコードファイルオプションオブジェクトへの参照
//
//	RETURN
//	LogicalFile::File*
//		可変長レコードファイルオブジェクトへのポインタ
//
//	EXCEPTIONS
//	YET!
//		YET! まだあるかもしれない。
//
LogicalFile::File*
FileDriver::attachFile(const LogicalFile::FileID&	cFileOption_) const
{
	return new Record::File(cFileOption_);
}

//
//	FUNCTION public
//	Record::FileDriver::attachFile -- 可変長レコードファイルをアタッチする
//
//	NOTES
//	可変長レコードファイルをアタッチする。
//
//	ARGUMENTS
//	const LogicalFile::File*	pFile_
//		可変長レコードファイルオブジェクトへのポインタ
//
//	RETURN
//	LogicalFile::File*
//		可変長レコードファイルオブジェクトへのポインタ
//
//	EXCEPTIONS
//	YET!
//		YET! まだあるかもしれない。
//
LogicalFile::File*
FileDriver::attachFile(const LogicalFile::File*	pFile_) const
{
	return new Record::File(pFile_->getFileID());
}

//
//	FUNCTION public
//	Record::FileDriver::detachFile -- 可変長レコードファイルをデタッチする
//
//	NOTES
//	可変長レコードファイルをデタッチする。
//
//	ARGUMENTS
//	LogicalFile::File*	pFile_
//		可変長レコードファイルオブジェクトへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileDriver::detachFile(LogicalFile::File*	pFile_) const
{
	delete pFile_;
}

//
//	FUNCTION public
//	Record::FileDriver::getDriverID --
//		可変長レコードファイルの論理ファイルドライバのドライバ ID を返す
//
//	NOTES
//	可変長レコードファイルの論理ファイルドライバのドライバ ID を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		可変長レコードファイルの論理ファイルドライバのドライバ ID
//
//	EXCEPTIONS
//	なし
//
int
FileDriver::getDriverID() const
{
	return LogicalFile::FileDriverID::Record;
}

//
//	FUNCTION public
//	Record::FileDriver::getDriverName --
//		可変長レコードファイルの論理ファイルドライバのドライバ名を返す
//
//	NOTES
//	可変長レコードファイルの論理ファイルドライバのドライバ名を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModString
//		可変長レコードファイルの論理ファイルドライバのドライバ名
//
//	EXCEPTIONS
//	なし
//
ModString
FileDriver::getDriverName() const
{
	return LogicalFile::FileDriverName::Record;
}

//
//	Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
