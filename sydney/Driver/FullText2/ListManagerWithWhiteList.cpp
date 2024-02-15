// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ListManagerWithWhiteList.cpp --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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
#include "FullText2/ListManagerWithWhiteList.h"
#include "FullText2/ListIteratorWithWhiteList.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//  FUNCTION public
//  FullText2::ListManagerWithWhiteList::ListManagerWithWhiteList
//		-- コンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::FullTextFile& cFile_
//		全文索引ファイル
//	FullText2::ListManager* pListManager_
//		リストマネージャー (本クラスデストラクト時に破棄される)
//	const Common::BitSet& cWhiteList_
//		集合
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
ListManagerWithWhiteList::
ListManagerWithWhiteList(FullTextFile& cFile_,
						 ListManager* pListManager_,
						 const Common::BitSet& cWhiteList_)
	: ListManager(cFile_), m_pListManager(pListManager_),
	  m_cWhiteList(cWhiteList_)
{
}

//
//  FUNCTION public
//  FullText2::ListManagerWithWhiteList::~ListManagerWithWhiteList
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
ListManagerWithWhiteList::~ListManagerWithWhiteList()
{
	delete m_pListManager;
}

//
//  FUNCTION public
//  FullText2::ListManagerWithWhiteList::ListManagerWithWhiteList
//		-- コピーコンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  const FullText2::ListManagerWithWhiteList& cSrc_
//		コピー元
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
ListManagerWithWhiteList::
ListManagerWithWhiteList(const ListManagerWithWhiteList& cSrc_)
	: ListManager(cSrc_), m_cWhiteList(cSrc_.m_cWhiteList)

{
	m_pListManager = cSrc_.m_pListManager->copy();
}

//
//	FUNCTION public
//	FullText2::ListManagerWithWhiteList::copy -- コピーを得る
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
ListManagerWithWhiteList::copy() const
{
	return new ListManagerWithWhiteList(*this);
}

//
//  FUNCTION public
//  FullText2::ListManagerWithWhiteList::getIterator
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
ListManagerWithWhiteList::getIterator()
{
	return new ListIteratorWithWhiteList(m_pListManager->getIterator(),
										 m_cWhiteList);
}

//
//  Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
