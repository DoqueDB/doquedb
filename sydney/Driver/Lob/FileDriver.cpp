// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriver.cpp --
// 
// Copyright (c) 2003, 2005, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Lob";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Lob/FileDriver.h"
#include "Lob/LogicalInterface.h"

#include "LogicalFile/FileDriverTable.h"
#include "LogicalFile/FileID.h"

#include "Exception/NotSupported.h"

_SYDNEY_USING
_SYDNEY_LOB_USING

//
//	FUNCTION public
//	Lob::FileDriver::FileDriver -- コンストラクタ
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
{
}

//
//	FUNCTION public
//	Lob::FileDriver::~FileDriver -- デストラクタ
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
}

//
//	FUNCTION public
//	Lob::FileDriver::initialize -- ファイルドライバの初期化を行なう
//
//	NOTES
//
//	ARGUMETNS
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
//	Lob::FileDriver::terminate -- ファイルドライバの後処理を行なう
//
//	NOTES
//
//	ARGUMETNS
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
//	Lob::FileDriver::attachFile -- 新しいファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID&	cFileID_
//		FileID
//
//	RETURN
//	LogicalFile::File*
//		ファイルへのポインタ
//
//	EXCEPTIONS
//	YET!
//		YET! まだあるかもしれない。
//
LogicalFile::File*
FileDriver::attachFile(const LogicalFile::FileID& cFileID_) const
{
	LogicalInterface* p = new LogicalInterface(cFileID_);
	return LogicalInterface::attach(p);
}

//
//	FUNCTION public
//	Lob::FileDriver::attachFile -- ファイルの参照カウンタを増やす
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::File* pFile_
//		ファイルへのポインタ
//
//	RETURN
//	LogicalFile::File*
//		ファイルへのポインタ
//
//	EXCEPTIONS
//
LogicalFile::File*
FileDriver::attachFile(const LogicalFile::File*	pFile_) const
{
	return LogicalInterface::attach(
		_SYDNEY_DYNAMIC_CAST(const LogicalInterface*, pFile_));
}

//
//	FUNCTION public
//	Lob::FileDriver::detachFile -- ファイルをデタッチする
//
//	NOTES
//
//	ARGUMENTS
//	LogicalFile::File* pFile_
//		ファイルへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FileDriver::detachFile(LogicalFile::File* pFile_) const
{
	LogicalInterface::detach(_SYDNEY_DYNAMIC_CAST(LogicalInterface*, pFile_));
}

//
//	FUNCTION public
//	Record::FileDriver::getDriverID -- ドライバIDを返す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		論理ファイルドライバのドライバID
//
//	EXCEPTIONS
//	なし
//
int
FileDriver::getDriverID() const
{
	return LogicalFile::FileDriverID::Lob;
}

//
//	FUNCTION public
//	Record::FileDriver::getDriverName -- ドライバ名を返す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModString
//		論理ファイルドライバのドライバ名
//
//	EXCEPTIONS
//	なし
//
ModString
FileDriver::getDriverName() const
{
	return LogicalFile::FileDriverName::Lob;
}

//
//	Copyright (c) 2003, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
