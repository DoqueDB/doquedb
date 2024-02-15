// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UpdateListManager.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "FullText2/UpdateListManager.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//  FUNCTION public
//  FullText2::UpdateListManager::UpdateListManager -- コンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::FullTextFile& cFile_
//		全文索引ファイル
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
UpdateListManager::UpdateListManager(FullTextFile& cFile_)
	: ListManager(cFile_)
{
}

//
//  FUNCTION public
//  FullText2::UpdateListManager::~UpdateListManager -- デストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
UpdateListManager::~UpdateListManager()
{
}

//
//  FUNCTION public
//  FullText2::UpdateListManager::UpdateListManager
//		-- コピーコンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//	const FullText2::UpdateListManager& src_
//		コピー元
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
UpdateListManager::UpdateListManager(const UpdateListManager& src_)
	: ListManager(src_)
{
}

//
//	FUNCTION public
//	FullText2::UpdateListManager::isNeedVacuum
//		-- バキュームが必要かどうか
//
//	NOTES
//
//	ARGUMENTS
//	int iNewExpungeCount_
//		今回の削除数
//
//	RETURN
//	bool
//		バキュームが必要な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
UpdateListManager::isNeedVacuum(int iNewExpungeCount_)
{
	return false;
}

//
//	FUNCTION public
//	FullText2::UpdateListManager::vacuum
//		-- バキュームする
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
UpdateListManager::vacuum()
{
}

//
//  Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
