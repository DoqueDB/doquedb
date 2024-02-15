// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DummyListManager.cpp --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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
#include "FullText2/DummyListManager.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace {
	ModUnicodeString _cNullKey;
}

//
//  FUNCTION public
//  FullText2::DummyListManager::DummyListManager -- コンストラクタ
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
DummyListManager::DummyListManager(FullTextFile& cFile_)
	: ListManager(cFile_)
{
}

//
//  FUNCTION public
//  FullText2::DummyListManager::~DummyListManager -- デストラクタ
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
DummyListManager::~DummyListManager()
{
}

//
//  FUNCTION public
//  FullText2::DummyListManager::DummyListManager -- コピーコンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  const FullText2::DummyListManager& cSrc_
//		コピー元
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
DummyListManager::DummyListManager(const DummyListManager& cSrc_)
	: ListManager(cSrc_)
{
}

//
//	FUNCTION public
//	FullText2::DummyListManager::copy -- コピーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::ListManager*
//		コピー
//
//	EXCEPTIONS
//
ListManager*
DummyListManager::copy() const
{
	return new DummyListManager(*this);
}

//
//  FUNCTION public
//  FullText2::DummyListManager::getKey -- 索引単位を得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  const ModUnicodeString& cstrKey_
//	  索引単位
//
//  EXCEPTIONS
//
const ModUnicodeString&
DummyListManager::getKey() const
{
	return _cNullKey;
}

//
//  Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
