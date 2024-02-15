// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiListManager.cpp --
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
#include "FullText2/MultiListManager.h"
#include "FullText2/MultiListIteratorImpl.h"
#include "FullText2/InvertedSection.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//  FUNCTION public
//  FullText2::MultiListManager::MultiListManager -- コンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::FullTextFile& cFile_
//		全文索引ファイル
//  FullText2::InvertedSection& cSection_
//		転置ファイルセクション
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
MultiListManager::MultiListManager(FullTextFile& cFile_,
								   InvertedSection& cSection_)
	: ListManager(cFile_), m_cSection(cSection_)
{
}

//
//  FUNCTION public
//  FullText2::MultiListManager::~MultiListManager -- デストラクタ
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
MultiListManager::~MultiListManager()
{
	destruct();
}

//
//  FUNCTION public
//  FullText2::MultiListManager::MultiListManager -- コピーコンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  const FullText2::MultiListManager& cSrc_
//		コピー元
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
MultiListManager::MultiListManager(const MultiListManager& cSrc_)
	: ListManager(cSrc_), m_cSection(cSrc_.m_cSection)
{
	m_vecpListManager.reserve(cSrc_.m_vecpListManager.getSize());
	ModVector<ModPair<ListManager*, bool> >::ConstIterator i
		= cSrc_.m_vecpListManager.begin();
	for (; i != cSrc_.m_vecpListManager.end(); ++i)
	{
		m_vecpListManager.pushBack(
			ModPair<ListManager*, bool>((*i).first->copy(), false));
	}
}

//
//	FUNCTION public
//	FullText2::MultiListManager::copy -- コピーを得る
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
MultiListManager::copy() const
{
	return new MultiListManager(*this);
}

//
//	FUNCTION public
//	FullText2::MultiListManager::reserve -- 要素を格納する領域を確保する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize size_
//		要素数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiListManager::reserve(ModSize size_)
{
	m_vecpListManager.reserve(size_);
}

//
//	FUNCTION public
//	FullText2::MultiListManager::pushBack -- 要素を追加する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::ListManager* pListManager_
//		追加する要素
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiListManager::pushBack(ListManager* pListManager_)
{
	m_vecpListManager.pushBack(
		ModPair<ListManager*, bool>(pListManager_, false));
}

//
//  FUNCTION public
//  FullText2::MultiListManager::getKey -- 索引単位を得る
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
MultiListManager::getKey() const
{
	return m_cstrKey;
}

//
//  FUNCTION public
//  FullText2::MultiListManager::getIterator -- 転置リストイテレータを得る
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
MultiListManager::getIterator()
{
	MultiListIterator* iterator = new MultiListIteratorImpl();
	iterator->reserve(m_vecpListManager.getSize());
	ModVector<ModPair<ListManager*, bool> >::ConstIterator
		i = m_vecpListManager.begin();
	for (; i != m_vecpListManager.end(); ++i)
	{
		// ヒットしていないユニットは 0 を pushBack する
		
		ListIterator* ite = 0;
		if ((*i).second == true && (*i).first->getKey() == m_cstrKey)
		{
			ite = (*i).first->getIterator();
		}
		iterator->pushBack(ite);
	}

	return iterator;
}

//
//  FUNCTION public
//  FullText2::MultiListManager::reset -- 転置リストを割り当てる
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeString& cstrKey_
//	  索引単位
//  FullText2::ListManager::AccessMode::Value eAccessMode_
//	  アクセスモード
//
//  RETURN
//  bool
//	  該当する転置リストが存在した場合はtrue、それ以外の場合はfalse
//
bool
MultiListManager::reset(const ModUnicodeString& cstrKey_,
						AccessMode::Value eAccessMode_)
{
	bool bResult = false;
	m_cstrKey.clear();

	ModVector<ModPair<ListManager*, bool> >::Iterator
		i = m_vecpListManager.begin();
	for (; i != m_vecpListManager.end(); ++i)
	{
		(*i).second = (*i).first->reset(cstrKey_, eAccessMode_);
		if ((*i).second == true)
		{
			if (bResult == false || m_cstrKey > (*i).first->getKey())
			{
				m_cstrKey = (*i).first->getKey();
			}
			bResult = true;
		}
	}

	return bResult;
}

//
//  FUNCTION public
//  FullText2::MultiListManager::next -- 次を得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  bool
//	  次の転置リストが存在する場合はtrue、それ以外の場合はfalse
//
//  EXCEPTIONS
//
bool
MultiListManager::next()
{
	bool bResult = false;

	ModVector<ModPair<ListManager*, bool> >::Iterator
		i = m_vecpListManager.begin();
	for (; i != m_vecpListManager.end(); ++i)
	{
		if ((*i).second == false)
			// この要素はすでに最後まで見ている
			continue;

		if ((*i).first->getKey() == m_cstrKey)
			// 最小のキーと同じキーのもののみ next する
			(*i).second = (*i).first->next();
	}

	i = m_vecpListManager.begin();
	for (; i != m_vecpListManager.end(); ++i)
	{
		if ((*i).second == true)
		{
			if (bResult == false || m_cstrKey > (*i).first->getKey())
			{
				m_cstrKey = (*i).first->getKey();
			}
			bResult = true;
		}
	}

	return bResult;
}

//
//  FUNCTION private
//  MultiListManager::destruct -- ListManagerを破棄する
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
//
void
MultiListManager::destruct()
{
	if (m_vecpListManager.getSize())
	{
		ModVector<ModPair<ListManager*, bool> >::Iterator i
			= m_vecpListManager.begin();
		for (; i != m_vecpListManager.end(); ++i)
		{
			if ((*i).first)
			{
				delete (*i).first;
				(*i).first = 0;
			}
		}
	}
}

//
//  Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
