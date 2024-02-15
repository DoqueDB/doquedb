// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ListManagerWithExpungeList.cpp --
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
#include "SyDynamicCast.h"
#include "FullText2/ListManagerWithExpungeList.h"
#include "FullText2/ListIteratorWithExpungeList.h"
#include "FullText2/InvertedSection.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//  FUNCTION public
//  FullText2::ListManagerWithExpungeList::ListManagerWithExpungeList
//		-- コンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::FullTextFile& cFile_
//		全文索引ファイル
//  FullText2::InvertedSection& cSection_
//		転置ファイルセクション
//	FullText2::ListManager* pListManager_
//		リストマネージャー (本クラスデストラクト時に破棄される)
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
ListManagerWithExpungeList::
ListManagerWithExpungeList(FullTextFile& cFile_,
						   InvertedSection& cSection_,
						   ListManager* pListManager_)
	: ListManager(cFile_),
	  m_cSection(cSection_), m_pListManager(pListManager_)
{
}

//
//  FUNCTION public
//  FullText2::ListManagerWithExpungeList::~ListManagerWithExpungeList
//		-- デストラクタ
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
ListManagerWithExpungeList::~ListManagerWithExpungeList()
{
	delete m_pListManager;
}

//
//  FUNCTION public
//  FullText2::ListManagerWithExpungeList::ListManagerWithExpungeList
//		-- コピーコンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  const FullText2::ListManagerWithExpungeList& cSrc_
//		コピー元
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
ListManagerWithExpungeList::
ListManagerWithExpungeList(const ListManagerWithExpungeList& cSrc_)
	: ListManager(cSrc_), m_cSection(cSrc_.m_cSection)

{
	m_pListManager = cSrc_.m_pListManager->copy();
}

//
//	FUNCTION public
//	FullText2::ListManagerWithExpungeList::copy -- コピーを得る
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
ListManagerWithExpungeList::copy() const
{
	return new ListManagerWithExpungeList(*this);
}

//
//  FUNCTION public
//  FullText2::ListManagerWithExpungeList::getIterator
//		-- 転置リストイテレータを得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ListIterator*
//	  転置リストイテレータ
//
//  EXCEPTIONS
//
ListIterator*
ListManagerWithExpungeList::getIterator()
{
	return new ListIteratorWithExpungeList(m_pListManager->getIterator(),
										   m_cSection.getExpungeList());
}

//
//  Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
