// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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
#include "FullText2/File.h"

#include "LogicalFile/Estimate.h"
#include "Os/File.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::File::File -- コンストラクタ
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
File::File()
{
}

//
//	FUNCTION public
//	FullText2::File::~File -- デストラクタ
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
File::~File()
{
}

//
//	FUNCTION public
//	FullText2::File::getUsedSize -- 利用サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
// 	ModUInt64
//		利用サイズ
//
//	EXCEPTIONS
//
ModUInt64
File::getUsedSize(const Trans::Transaction& cTransaction_)
{
	return 0;
}

//
//  FUNCTION public
//  Inverted::File::getOverhead -- 1ページを得るコストを得る
//
//  NOTES
//
//  ARGUMENTS
//  ModSize uiPageSize_
//	  1ページのサイズ (byte)
//
//  RETURN
//  double
//	  1ページを得る秒数
//
//  EXCEPTIONS
//
double
File::getOverhead(ModSize uiPageSize_)
{
	double cost = static_cast<double>(uiPageSize_);
	return cost
		/ LogicalFile::Estimate::getTransferSpeed(LogicalFile::Estimate::File);
}

//
//	FUNCTION protected
//	FullText2::File::rmdir -- ディレクトリを削除する
//
//	NOTES
//
//	ARGUMENTS
//	const Os::Path& cPath_
//		削除するディレクトリ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::rmdir(const Os::Path& cPath_)
{
	if (cPath_.getLength() == 0)
		// 空なので、何もしない
		return;
	
	// 存在を確認し、あれば削除する

	if (Os::Directory::access(cPath_, Os::Directory::AccessMode::File) == true)
	{
		// 存在するので、削除する
		
		Os::Directory::remove(cPath_);
	}
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
