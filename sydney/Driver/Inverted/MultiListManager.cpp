// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiListManager.cpp --
// 
// Copyright (c) 2005, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "Inverted/MultiListManager.h"
#include "Inverted/MultiListIterator.h"
#include "Inverted/ListManager.h"
#include "Inverted/DocumentIDVectorFile.h"
#include "Inverted/InvertedUnit.h"

#include "Common/Assert.h"
#include "Os/Memory.h"

#include "Exception/Object.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//  FUNCTION public
//  Inverted::MultiListManager::MultiListManager -- コンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  Inverted::DocumentIDVectorFile* pDocumentIDVectorFile_
//	  文書IDベクターファイル
//  Inverted::RowIDVectorFile2* pRowIDVectorFile2_
//	  ROWIDベクターファイル
//  Inveted::InvertedUnit* pInvertedUnit_
//	  転置ファイルユニット
//  int iUnitCount_
//	  ユニット数
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
MultiListManager::MultiListManager(DocumentIDVectorFile* pDocumentIDVectorFile_,
								   RowIDVectorFile2* pRowIDVectorFile2_,
								   InvertedUnit* pInvertedUnit_,
								   int iUnitCount_)
	: m_pDocumentIDVectorFile(pDocumentIDVectorFile_),
	  m_pRowIDVectorFile2(pRowIDVectorFile2_),
	  m_uiLastDocumentID(ModInvertedUndefinedDocumentID)
{
	try
	{
		; _SYDNEY_ASSERT(iUnitCount_ != 0);
		
		m_vecpListManager.reserve(iUnitCount_);
		for (int i = 0; i < iUnitCount_; ++i)
		{
			ListManager* p = 0;
			if (pInvertedUnit_->isAttached())
			{
				p = new ListManager(pInvertedUnit_);
			}
			m_vecpListManager.pushBack(p);
			if (i == 0)
			{
				m_bNolocation =
					pInvertedUnit_->isNolocation() ? ModTrue : ModFalse;
			}
			else
			{
				; _SYDNEY_ASSERT(
					pInvertedUnit_->isNolocation() == static_cast<bool>(m_bNolocation));
			}
			pInvertedUnit_++;
		}
	}
	catch (...)
	{
		destruct();
		_SYDNEY_RETHROW;
	}
}

//
//  FUNCTION public
//  Inverted::MultiListManager::~MultiListManager -- デストラクタ
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
//  Inverted::MultiListManager::MultiListManager -- コピーコンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
MultiListManager::MultiListManager(const MultiListManager& other_)
	: m_pDocumentIDVectorFile(other_.m_pDocumentIDVectorFile),
	  m_pRowIDVectorFile2(other_.m_pRowIDVectorFile2),
	  m_bNolocation(other_.m_bNolocation)
{
	try
	{
		ModSize iUnitCount = other_.m_vecpListManager.getSize();
		m_vecpListManager.reserve(iUnitCount);
		for (ModSize i = 0; i < iUnitCount; ++i)
		{
			ListManager* p = 0;
			if (other_.m_vecpListManager[i])
				p = _SYDNEY_DYNAMIC_CAST(ListManager*,
										 other_.m_vecpListManager[i]->clone());
			m_vecpListManager.pushBack(p);
		}
		m_cstrKey = other_.m_cstrKey;
		m_uiLastDocumentID = other_.m_uiLastDocumentID;
	}
	catch (...)
	{
		destruct();
		_SYDNEY_RETHROW;
	}
}

//
//  FUNCTION public
//  Inverted::MultiListManager::getDocumentFrequency -- 文書頻度を得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModSize
//	  現在のリストの文書頻度
//
//  EXCEPTIONS
//
ModSize
MultiListManager::getDocumentFrequency() const
{
	ModSize s = 0;
	ModVector<ListManager*>::ConstIterator i = m_vecpListManager.begin();
	for (; i != m_vecpListManager.end(); ++i)
		if (*i && (*i)->isSetList() && (*i)->getKey() == m_cstrKey)
			s += (*i)->getDocumentFrequency();
	return s;
}

//
//  FUNCTION public
//  Inverted::MultiListManager::getKey -- 索引単位を得る
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
//  Inverted::MultiListManager::begin -- 転置リストイテレータを得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModInvertedIterator*
//	  転置リストイテレータ
//
//  EXCEPTIONS
//
ModInvertedIterator*
MultiListManager::begin() const
{
	MultiListIterator* iterator = new MultiListIterator(m_pDocumentIDVectorFile,
														m_pRowIDVectorFile2);
	iterator->reserve(m_vecpListManager.getSize());
	ModVector<ListManager*>::ConstIterator i = m_vecpListManager.begin();
	for (; i != m_vecpListManager.end(); ++i)
	{
		ModInvertedIterator* ite = 0;
		if (*i && (*i)->isSetList() && (*i)->getKey() == m_cstrKey)
		{
			ite = (*i)->begin();
		}
		iterator->pushBack(ite);
	}
	iterator->set();
	return iterator;
}

//
//  FUNCTION public
//  Inverted::MultiListManager::clone -- 自分の複製を得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModInvertedList*
//	  自分の複製
//
//  EXCEPTIONS
//
ModInvertedList*
MultiListManager::clone() const
{
	return new MultiListManager(*this);
}

//
//  FUNCTION public
//  Inverted::MultiListManager::reset -- 転置リストを割り当てる
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeString& cstrKey_
//	  索引単位
//  const ModInvertedListAccessMode eAccessMode_
//	  アクセスモード
//
//  RETURN
//  ModBoolean
//	  該当する転置リストが存在した場合はModTrue、それ以外の場合はModFalse
//
ModBoolean
MultiListManager::reset(const ModUnicodeString& cstrKey_,
						const ModInvertedListAccessMode eAccessMode_)
{
	ModBoolean bResult = ModFalse;
	m_cstrKey.clear();

	m_uiLastDocumentID = ModInvertedUndefinedDocumentID;

	ModVector<ListManager*>::Iterator i = m_vecpListManager.begin();
	for (; i != m_vecpListManager.end(); ++i)
	{
		if (*i)
		{
			ModBoolean r = (*i)->reset(cstrKey_, eAccessMode_);
			if (r == ModTrue)
			{
				if (bResult == ModFalse || m_cstrKey > (*i)->getKey())
				{
					m_cstrKey = (*i)->getKey();
				}
				bResult = ModTrue;
			}
		}
	}

	return bResult;
}

//
//  FUNCTION public
//  Inverted::MultiListManager::next -- 次を得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModBoolean
//	  次の転置リストが存在する場合はModTrue、それ以外の場合はModFalse
//
//  EXCEPTIONS
//
ModBoolean
MultiListManager::next()
{
	ModBoolean bResult = ModFalse;

	m_uiLastDocumentID = ModInvertedUndefinedDocumentID;

	ModVector<ListManager*>::Iterator i = m_vecpListManager.begin();
	for (; i != m_vecpListManager.end(); ++i)
	{
		if (*i)
		{
			if ((*i)->isSetList() == false)
				// すでに最後まで見ている
				continue;

			if ((*i)->getKey() == m_cstrKey)
				// 最小のキーと同じキーのもののみ next する
				(*i)->next();
		}
	}

	i = m_vecpListManager.begin();
	for (; i != m_vecpListManager.end(); ++i)
	{
		if (*i && (*i)->isSetList() == true)
		{
			if (bResult == ModFalse || m_cstrKey > (*i)->getKey())
			{
				m_cstrKey = (*i)->getKey();
			}
			bResult = ModTrue;
		}
	}

	return bResult;
}

//
//  FUNCTION public
//  Inverted::MultiListManager::getLastDocumentID -- 最終文書IDを得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModInvertedDocumentID
//	  最終文書ID
//
//  EXCEPTIONS
//
ModInvertedDocumentID
MultiListManager::getLastDocumentID()
{
	if (m_uiLastDocumentID == ModInvertedUndefinedDocumentID)
	{
		m_uiLastDocumentID = 0;
		ModVector<ListManager*>::Iterator i = m_vecpListManager.begin();
		for (; i != m_vecpListManager.end(); ++i)
		{
			if (*i && (*i)->isSetList() && (*i)->getKey() == m_cstrKey &&
				m_uiLastDocumentID < (*i)->getLastDocumentID())
			{
				m_uiLastDocumentID = (*i)->getLastDocumentID();
			}
		}
	}

	return m_uiLastDocumentID;
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
		ModVector<ListManager*>::Iterator i = m_vecpListManager.begin();
		for (; i != m_vecpListManager.end(); ++i)
			if (*i) delete (*i);
	}
}

//
//  Copyright (c) 2005, 2006, 2008, 2009, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
