// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RowIDVectorFile.cpp --
// 
// Copyright (c) 2002, 2005, 2008, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Inverted/RowIDVectorFile.h"
#include "Inverted/Parameter.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

namespace
{
	//
	//	VARIABLE
	//
	const Os::Ucs2 _pszPath[] = {'R','o','w','I','D',0};
}

//
//	FUNCTION public
//	Inverted::RowIDVectorFile::RowIDVectorFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Inverted::FileID& cFileID_
//		転置ファイルパラメータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
RowIDVectorFile::RowIDVectorFile(const FileID& cFileID_, bool batch_)
	: VectorFile<ModUInt32>(Type::RowIDVector)
{
	// 物理ファイルをアタッチする
	attach(cFileID_,
		   cFileID_.getPageSize(),
		   cFileID_.getPath(),
		   Os::Path(_pszPath),
		   batch_);
}

//
//	FUNCTION public
//	Inverted::RowIDVectorFile::~RowIDVectorFile -- デストラクタ
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
RowIDVectorFile::~RowIDVectorFile()
{
	// 物理ファイルをデタッチする
	detach();
}

//
//	FUNCTION public
//	Inverted::RowIDVectorFile::move -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cFilePath_
//		パス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
RowIDVectorFile::move(const Trans::Transaction& cTransaction_,
					  const Os::Path& cFilePath_)
{
	File::move(cTransaction_, cFilePath_, Os::Path(_pszPath));
}

//
//	Copyright (c) 2002, 2005, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
