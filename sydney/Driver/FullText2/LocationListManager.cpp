// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LocationListManager.cpp --
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
#include "FullText2/LocationListManager.h"
#include "FullText2/LocationListIterator.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::LocationListManager::LocationListManager -- コンストラクタ
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
LocationListManager::LocationListManager()
	: m_pFree(0)
{
}

//
//	FUNCTION public
//	FullText2::LocationListManager::~LocationListManager -- デストラクタ
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
LocationListManager::~LocationListManager()
{
	clearFree();
}

//
//	FUNCTION public
//	FullText2::LocationListManager::addFree
//		-- 解放された位置情報走査クラスをリストに追加する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::LocationListIterator* pList_
//		追加するリスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LocationListManager::addFree(LocationListIterator* pList_)
{
	pList_->m_pNext = m_pFree;
	m_pFree = pList_;
}

//
//	FUNCTION public
//	FullText2::LocationListManager::getFree
//		-- 解放された位置情報走査クラスを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::LocationListIterator*
//		位置情報走査クラス
//
//	EXCEPTIONS
//
LocationListIterator*
LocationListManager::getFree()
{
	LocationListIterator* pList = m_pFree;
	if (pList)
		m_pFree = pList->m_pNext;
	return pList;
}

//
//	FUNCTION protected
//	FullText2::LocationListManager::clearFree
//		-- 解放された位置情報走査クラスをフリーする
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
LocationListManager::clearFree()
{
	while (m_pFree)
	{
		LocationListIterator* pList = m_pFree->m_pNext;
		delete m_pFree;
		m_pFree = pList;
	}
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
