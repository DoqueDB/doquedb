// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TreeFile.cpp -- ツリーファイルクラスの実現ファイル
// 
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Btree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Btree/TreeFile.h"

#include "Common/UnicodeString.h"

_SYDNEY_USING

using namespace Btree;

//////////////////////////////////////////////////
//
//	PUBLIC CONST
//
//////////////////////////////////////////////////

//
//	CONST public
//	Btree::TreeFile::DirectoryName --
//		ツリーファイル格納先ディレクトリ名
//
//	NOTES
//	ツリーファイル格納先ディレクトリ名。パスではない。
//
// static
const ModUnicodeString
TreeFile::DirectoryName = _TRMEISTER_U_STRING("Tree");

//
//	CONST public
//	Btree::TreeFile::HeaderPageID -- ヘッダページの物理ページ識別子
//
//	NOTES
//	ヘッダページの物理ページ識別子。
//
// static
const PhysicalFile::PageID
TreeFile::HeaderPageID = 0;

//////////////////////////////////////////////////
//
//	PUBLIC METHOD
//
//////////////////////////////////////////////////

#ifdef OBSOLETE
//
//	FUNCTION public
//	Btree::TreeFile::TreeFile -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const Btree::FileParameter*	FileParam_
//		ファイルパラメータへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
TreeFile::TreeFile(const FileParameter*	FileParam_)
	: Common::Object()
{
}
#endif //OBSOLETE

#ifdef OBSOLETE
//
//	FUNCTION public
//	Btree::TreeFile::~TreeFile -- デストラクタ
//
//	NOTES
//	デストラクタ。
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
TreeFile::~TreeFile()
{
}
#endif //OBSOLETE

//
//	Copyright (c) 2001, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
