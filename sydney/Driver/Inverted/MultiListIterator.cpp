// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiListIterator.cpp --
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
#include "SyDynamicCast.h"
#include "Inverted/MultiListIterator.h"
#include "Inverted/DocumentIDVectorFile.h"
#include "Inverted/RowIDVectorFile2.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::MultiListIterator::MultiListIterator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::DocumentIDVectorFile* pDocumentIDVectorFile
//		文書IDベクターファイル
//	Inverted::RowIDVectorFile2* pRowIDVectorFile2
//		ROWIDベクターファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
MultiListIterator::MultiListIterator(
	DocumentIDVectorFile* pDocumentIDVectorFile_,
	RowIDVectorFile2* pRowIDVectorFile2_)
	: m_pDocumentIDVectorFile(pDocumentIDVectorFile_),
	  m_pRowIDVectorFile2(pRowIDVectorFile2_),
	  m_uiCurrentID(UndefinedDocumentID),
	  m_iCurrentElement(-1), m_bFind(false)
{
	set();
}

//
//	FUNCTION public
//	Inverted::MultiListIterator::~MultiListIterator -- デストラクタ
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
MultiListIterator::~MultiListIterator()
{
	ModVector<ModInvertedIterator*>::Iterator i = m_vecpIterator.begin();
	for (; i != m_vecpIterator.end(); ++i)
		if (*i) delete *i;
	m_vecpIterator.clear();
}

//
//	FUNCTION public
//	Inverted::MultiListIterator::set -- 文書IDを設定する
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
MultiListIterator::set()
{
	m_bFind = false;
	m_uiCurrentID = UndefinedDocumentID;	// ModUInt32Max
	m_uiOtherMinimumID = UndefinedDocumentID;
	int element = 0;
	ModVector<ModInvertedIterator*>::Iterator i = m_vecpIterator.begin();
	for (; i != m_vecpIterator.end(); ++i, ++element)
	{
		if (*i && (*i)->isEnd() == ModFalse)
		{
			if (m_uiCurrentID > (*i)->getDocumentId())
			{
				m_uiOtherMinimumID = m_uiCurrentID;	// その他の最小値
				m_uiCurrentID = (*i)->getDocumentId();
				m_iCurrentElement = element;
			}
			else if (m_uiOtherMinimumID > (*i)->getDocumentId())
			{
				m_uiOtherMinimumID = (*i)->getDocumentId();
			}
		}
	}
}

//
//	FUNCTION public
//	Inverted::MultiListIterator::next -- 次へ進める
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
MultiListIterator::next()
{
	if (isEnd() == ModTrue) return;

	if (m_bFind == true)
	{
		// 前回findを実行して、そのままnextが呼ばれている
		// -> 次がわからないので、現在値でlowerBoundする
		reset();
		lowerBound(m_uiCurrentID);
	}
	
	ModVector<ModInvertedIterator*>::Iterator i
		= m_vecpIterator.begin() + m_iCurrentElement;
	(*i)->next();
	if ((*i)->isEnd() == ModFalse &&
		(*i)->getDocumentId() < m_uiOtherMinimumID)
	{
		// その他のものの文書IDよりまだ小さいので、この文書IDを採用する
		m_uiCurrentID = (*i)->getDocumentId();
	}
	else
	{
		// 他のイテレータを参照する必要がある
		set();
	}
}

//
//	FUNCTION public
//	Inverted::MultiListIterator::reset -- 先頭へ戻る
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
MultiListIterator::reset()
{
	ModVector<ModInvertedIterator*>::Iterator i = m_vecpIterator.begin();
	for (; i != m_vecpIterator.end(); ++i)
		if (*i) (*i)->reset();
	set();
}

//
//	FUNCTION public
//	Inverted::MultiListIterator::find -- 文書IDを検索する
//
//	NOTES
//
//	ARGUMENTS
//	ModInvertedIterator::DocumentID uiDocumentID_
//		検索する文書ID
//
//	RETURN
//	ModBoolean
//		検索にヒットした場合はModTrue、それ以外の場合はModFalse
//
//	EXCEPTIONS
//
ModBoolean
MultiListIterator::find(const DocumentID uiDocumentID_)
{
	m_iCurrentElement = getUnit(uiDocumentID_);
	if (m_iCurrentElement != -1)
	{
		m_bFind = true;
		ModBoolean r = m_vecpIterator[m_iCurrentElement]->find(uiDocumentID_);
		if (r == ModTrue)
		{
			m_uiCurrentID = m_vecpIterator[m_iCurrentElement]->getDocumentId();
			return ModTrue;
		}
	}

	m_uiCurrentID = UndefinedDocumentID;
	return ModFalse;
}

//
//	FUNCTION public
//	Inverted::MultiListIterator::lowerBound -- 文書IDをlower_bound検索する
//
//	NOTES
//
//	ARGUMENTS
//	ModInvertedListIterator::DocumentID uiDocumentID_
//		検索する文書ID
//
//	RETURN
//	ModBoolean
//		検索できた場合はModTrue、それ以外の場合はModFalse
//
//	EXCEPTIONS
//
ModBoolean
MultiListIterator::lowerBound(const DocumentID uiDocumentID_)
{
	ModBoolean bResult = ModFalse;
	
	if (m_uiCurrentID < uiDocumentID_ &&
		m_uiOtherMinimumID > uiDocumentID_)
	{
		// この要素に存在する
		bResult = m_vecpIterator[m_iCurrentElement]->lowerBound(uiDocumentID_);
		if (bResult == ModTrue)
		{
			m_uiCurrentID = m_vecpIterator[m_iCurrentElement]->getDocumentId();
			if (m_uiCurrentID < m_uiOtherMinimumID)
				// 他の要素の最小値を超えなければOK
				return bResult;
		}
	}

	int n = 0;
	ModVector<ModInvertedIterator*>::Iterator i = m_vecpIterator.begin();
	for (; i != m_vecpIterator.end(); ++i, ++n)
	{
		if (*i && (*i)->isEnd() == ModFalse)
			// 終端に達した要素はもうlowerBoundの対象外
			// ModInvertedからのlowerBoundの引数は単調増加する
			if ((*i)->lowerBound(uiDocumentID_) == ModTrue)
				bResult = ModTrue;
	}
	set();
	return bResult;
}

//
//	FUNCTION public
//	Inverted::MultiListIterator::getInDocumentFrequency
//		-- 位置情報内の頻度情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		位置情報内の頻度情報
//
//	EXCEPTIONS
//
ModSize
MultiListIterator::getInDocumentFrequency()
{
	return m_vecpIterator[m_iCurrentElement]->getInDocumentFrequency();
}

//
//	FUNCTION public
//	Inverted::MultiListIterator::getLocationListIterator
//		-- 位置情報へのイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInvertedLocationListIterator*
//		位置情報イテレータへのポインタ。
//		このメモリーは呼び出し側で解放する必要がある
//
//	EXCEPTIONS
//
ModInvertedLocationListIterator*
MultiListIterator::getLocationListIterator()
{
	return m_vecpIterator[m_iCurrentElement]->getLocationListIterator();
}

//
//	FUNCTION private
//	Inverted::MultiListIterator::getUnit -- 対象文書IDのユニット番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	DocumentID uiDocumentID_
//		文書ID
//
//	RETURN
//	int
//		ユニット番号、存在しない場合は -1 を返す
//
//	EXCEPTIONS
//
int
MultiListIterator::getUnit(DocumentID uiDocumentID_)
{
	ModInt32 iUnit = -1;
	ModUInt32 uiRowID;
	ModUInt32 dummy;
	if (m_pDocumentIDVectorFile->find(uiDocumentID_, uiRowID, dummy) == true)
		m_pRowIDVectorFile2->find(uiRowID, dummy, iUnit);

	return iUnit;
}

//
//	Copyright (c) 2005, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
