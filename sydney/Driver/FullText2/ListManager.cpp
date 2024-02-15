// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ListManager.cpp --
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
#include "FullText2/ListManager.h"
#include "FullText2/FullTextFile.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::ListManager::ListManager -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::FullTextFile& cFile_
//		全文索引ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ListManager::ListManager(FullTextFile& cFile_)
	: m_cFile(cFile_)
{
	// 高速化のためのキャッシュ
	m_bNoLocation = cFile_.isNoLocation();
}

//
//	FUNCTION public
//	FullText2::ListManager::~ListManager -- デストラクタ
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
ListManager::~ListManager()
{
}

//
//	FUNCTION public
//	FullText2::ListManager::ListManager -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::ListManager& cSrc_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ListManager::ListManager(const ListManager& cSrc_)
	: m_cFile(cSrc_.m_cFile), m_bNoLocation(cSrc_.m_bNoLocation)
{
}

//
//	FUNCTION public
//	FullText2::ListManager::getTokenizer -- トークナイザーを得る
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	FullText2::Tokenizer::AutoPointer
//		トークナイザー
//
//	EXCEPTIONS
//
Tokenizer::AutoPointer
ListManager::getTokenizer()
{
	return m_cFile.getTokenizer();
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
