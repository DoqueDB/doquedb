// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BatchNolocationNoTFList.cpp --
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
#include "FullText2/BatchNolocationNoTFList.h"
#include "FullText2/BatchListMap.h"
#include "FullText2/ShortNolocationNoTFListIterator.h"

#include "ModOsDriver.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::BatchNolocationNoTFList::BatchNolocationNoTFList
//		-- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUpdateFile& cInvertedFile_
//		転置ファイル
//	FullText2::BatchListMap& cBatchListMap_
//		バッチリストマップ
//	const ModUnicodeChar* pKey_
//		索引単位
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
BatchNolocationNoTFList::
BatchNolocationNoTFList(InvertedUpdateFile& cInvertedFile_,
						BatchListMap& cBatchListMap_,
						const ModUnicodeChar* pszKey_)
	: BatchBaseList(cInvertedFile_, cBatchListMap_, pszKey_)
{
}

//
//	FUNCTION public
//	FullText2::BatchNolocationNoTFList::BatchNolocationNoTFList
//		-- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUpdateFile& cInvertedFile_
//		転置ファイル
//	LeafPage::Area* pArea_
//		リーフページのエリア
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
BatchNolocationNoTFList::
BatchNolocationNoTFList(InvertedUpdateFile& cInvertedFile_,
						LeafPage::Area* pArea_)
	: BatchBaseList(cInvertedFile_, pArea_)
{
}

//
//	FUNCTION public
//	FullText2::BatchNolocationNoTFList::~BatchNolocationNoTFList -- デストラクタ
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
BatchNolocationNoTFList::~BatchNolocationNoTFList()
{
}

//
//	FUNCTION public
//	FullText2::BatchNolocationNoTFList::insert -- 転置リストの挿入(1文書挿入用)
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//	const SmartLocationList& cLocationList_
//		位置情報リスト
//
//	RETURN
//	bool
//		挿入できた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
BatchNolocationNoTFList::insert(ModUInt32 uiDocumentID_,
								const SmartLocationList& cLocationList_)
{
	//
	//	必要なユニット数を求める
	//
	ModSize idLength = 0;
	if (m_pArea->getDocumentCount())
	{
		idLength = getCompressedBitLengthDocumentID(
			m_pArea->getLastDocumentID(), uiDocumentID_);
	}

	ModSize totalLength = m_pArea->getDocumentOffset() + idLength;
	ModSize unitSize = calcUnitSize(totalLength);

	//
	//	大きさをチェックする
	//
	if (m_pArea->getDataUnitSize() < unitSize)
	{
		// 広げる
		ModSize newSize = m_pArea->getDataUnitSize();
		
		if (unitSize >= static_cast<ModSize>(getRegularUnitSize()))
		{
			while (newSize < unitSize)
				newSize += getRegularUnitSize();
		}
		else
		{
			while (newSize < unitSize)
				newSize *= 2;
		}

		if (newSize > static_cast<ModSize>(getMaxUnitSize()) &&
			m_pArea->getDocumentCount() != 0)
		{
			// エリアの最大値を超えた
			return false;
		}
		
		ModSize expand = newSize - m_pArea->getDataUnitSize();
		LeafPage::Area* pArea = 0;

		try
		{
			pArea = LeafPage::Area::allocateArea(getKey(),
										   m_pArea->getDataUnitSize() + expand);
		}
		catch (Exception::Object&)
		{
			// メモリーの確保に失敗した
			return false;
		}

		// コピーする
		pArea->copy(m_pArea);
		// 文書IDを広げた分移動する
		ModSize unit = calcUnitSize(m_pArea->getDocumentOffset());
		ModOsDriver::Memory::move(
			pArea->getTailAddress() - unit,
			pArea->getTailAddress() - unit - expand, unit*sizeof(ModUInt32));
		setOff(pArea->getHeadAddress(),
			   pArea->getLocationOffset(), 
			   pArea->getDataUnitSize() * 32 - pArea->getLocationOffset()
			   - pArea->getDocumentOffset());

		m_pArea = pArea;

		// バッチリストマップに広げた分のバイト数を登録する
		m_pMap->addListSize(expand * sizeof(ModUInt32));
	}

	//
	//	文書IDを書く
	//
	if (m_pArea->getDocumentCount() == 0)
	{
		m_pArea->setFirstDocumentID(uiDocumentID_);
	}
	else
	{
		// 文書IDビットオフセット
		ModSize offset = m_pArea->getDocumentOffset();
		// 圧縮して格納する
		writeDocumentID(m_pArea->getLastDocumentID(), uiDocumentID_,
						m_pArea->getTailAddress(), offset);
		// 文書IDビットオフセットを設定する
		m_pArea->setDocumentOffset(offset);
	}

	// 最終文書IDを設定する
	m_pArea->setLastDocumentID(uiDocumentID_);
	// 頻度情報を設定する
	m_pArea->incrementDocumentCount();
	// 最大文書IDを設定する
	m_uiMaxDocumentID = uiDocumentID_;

	return true;
}

//
//	FUNCTION public
//	FullText2::BatchNolocationNoTFList::getIterator -- イテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::InvertedItertor*
//		イテレータ
//
//	EXCEPTIONS
//
InvertedIterator*
BatchNolocationNoTFList::getIterator()
{
	return new ShortNolocationNoTFListIterator(*this);
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
