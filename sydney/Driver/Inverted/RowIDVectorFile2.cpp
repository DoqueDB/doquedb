// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RowIDVectorFile2.cpp --
// 
// Copyright (c) 2005, 2008, 2023 Ricoh Company, Ltd.
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
#include "Inverted/RowIDVectorFile2.h"
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
//	Inverted::RowIDVectorFile2::RowIDVectorFile2 -- コンストラクタ
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
RowIDVectorFile2::RowIDVectorFile2(const FileID& cFileID_, bool batch_)
	: VectorFile<ModPair<ModUInt32, ModInt32> >(Type::RowIDVector)
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
//	Inverted::RowIDVectorFile2::~RowIDVectorFile2 -- デストラクタ
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
RowIDVectorFile2::~RowIDVectorFile2()
{
	// 物理ファイルをデタッチする
	detach();
}

//
//	FUNCTION public
//	Inverted::RowIDVectorFile2::move -- ファイルを移動する
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
RowIDVectorFile2::move(const Trans::Transaction& cTransaction_,
					  const Os::Path& cFilePath_)
{
	File::move(cTransaction_, cFilePath_, Os::Path(_pszPath));
}

//
//	FUNCTION public
//	Inverted::DocumentIDVectorFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	ModUInt32 uiDocumentID_
//		文書ID
//	ModInt32 iElement_
//		要素番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
RowIDVectorFile2::insert(ModUInt32 uiRowID_,
						 ModUInt32 uiDocumentID_,
						 ModInt32 iElement_)
{
	// ベクターファイルに挿入する
	VectorFile<ModPair<ModUInt32, ModInt32> >::insert(uiRowID_,
		ModPair<ModUInt32, ModInt32>(uiDocumentID_, iElement_));
}

//
//	FUNCTION public
//	Inverted::RowIDVectorFile2::find -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//	   	ROWID
//	ModUInt32& uiDocumentID_
//		文書ID
//	ModInt32& iElement_
//		要素番号
//
//	RETURN
//	bool
//		検索にヒットした場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
RowIDVectorFile2::find(ModUInt32 uiRowID_,
					   ModUInt32& uiDocumentID_,
					   ModInt32& iElement_)
{
	// ベクターファイルを検索する
	ModPair<ModUInt32, ModInt32> cData;
	if (VectorFile<ModPair<ModUInt32, ModInt32> >::find(uiRowID_, cData)
		== false)
		// ヒットしなかった
		return false;

	uiDocumentID_ = cData.first;
	iElement_ = cData.second;

	return true;
}

//
//	Copyright (c) 2005, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
