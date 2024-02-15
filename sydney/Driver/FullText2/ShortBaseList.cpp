// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortBaseList.cpp --
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

#include "FullText2/ShortBaseList.h"
#include "FullText2/BatchBaseList.h"
#include "FullText2/InvertedUnit.h"
#include "FullText2/LeafPage.h"
#include "FullText2/MiddleBaseList.h"
#include "FullText2/BatchList.h"
#include "FullText2/FakeError.h"

#include "Common/Assert.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::ShortBaseList::ShortBaseList -- コンストラクタ(1)
//
//	NOTES
//	該当する索引単位のエリアがある場合
//
//	ARGUMENTS
//	FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	FullText2::LeafPage::PagePointer pLeafPage_
//		リーフページ
//	FullText2::LeafPage::Iterator ite_
//		該当する索引単位のエリアへのイテレータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ShortBaseList::ShortBaseList(InvertedUnit& cInvertedUnit_,
							 LeafPage::PagePointer pLeafPage_,
							 LeafPage::Iterator ite_)
	: InvertedList(cInvertedUnit_,
				   (*ite_)->getKey(),
				   (*ite_)->getKeyLength(),
				   ListType::Short,
				   pLeafPage_,
				   ite_),
	  m_cInvertedUnit(cInvertedUnit_),
	  m_bExist(true)
{
}

//
//	FUNCTION public
//	FullText2::ShortBaseList::ShortBaseList -- コンストラクタ(2)
//
//	NOTES
//	該当する索引単位のエリアが存在しない場合
//
//	ARGUMENTS
//	FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	const ModUnicodeChar* pszKey_
//		索引単位
//	FullText2::LeafPage::PagePointer pLeafPage_
//		リーフページ
//	FullText2::LeafPage::Iterator ite_
//		lower_boundで検索したエリアへのイテレータ(挿入位置)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ShortBaseList::ShortBaseList(InvertedUnit& cInvertedUnit_,
							 const ModUnicodeChar* pszKey_,
							 LeafPage::PagePointer pLeafPage_,
							 LeafPage::Iterator ite_)
	: InvertedList(cInvertedUnit_,
				   pszKey_,
				   ListType::Short,
				   pLeafPage_,
				   ite_),
	  m_cInvertedUnit(cInvertedUnit_),
	  m_bExist(false)
{
}

//
//	FUNCTION public
//	FullText2::ShortBaseList::~ShortBaseList -- デストラクタ
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
ShortBaseList::~ShortBaseList()
{
}

//
//	FUNCTION public
//	FullText2::ShortBaseList::insert -- 転置リストの挿入(転置リスト単位)
//
//	NOTES
//	マージ時に転置リスト単位の挿入を行うためのメソッド
//
//	ARGUMENTS
//	const FullText2::InvertedList& cInvertedList_
//		挿入する転置リスト
//
//	RETURN
//	bool
//		ショートリストの範囲内ならtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
ShortBaseList::insert(InvertedList& cInvertedList_)
{
	if (cInvertedList_.getListType() == ListType::Middle)
		// 挿入する転置リストがミドルリストなので、だめ
		return false;

	LeafPage::Area* pArea = cInvertedList_.getArea();

	if (pArea->getDocumentCount() == 0)
		// 挿入する転置リストが0件なので、終わり
		return true;

	if (isExist() == true)
	{
		// 最終文書ID
		ModUInt32 uiLastDocumentID = (*m_ite)->getLastDocumentID();

		// すでにマージ済みかどうか確認する
		if (uiLastDocumentID >= pArea->getFirstDocumentID())
		{
			// 挿入するものは必ず大きな文書IDになっている
			// そうなっていないっていうことは、すでにマージ済みってこと
			
			return true;
		}

		// 拡張するべきサイズを求める

		// 文書IDの圧縮ビット長を求める
		ModSize idLength = 0;
		if (uiLastDocumentID != 0)
			idLength = getCompressedBitLengthDocumentID(
				uiLastDocumentID,
				pArea->getFirstDocumentID());

		idLength += pArea->getDocumentOffset();
		ModSize locLength = pArea->getLocationOffset();

		// 結局何ビット？
		ModSize totalLength = (*m_ite)->getDocumentOffset() + idLength
								+ (*m_ite)->getLocationOffset() + locLength;

		if (calcUnitSize(totalLength) > (*m_ite)->getDataUnitSize())
		{
			// で、何ユニット必要？
			ModSize expandUnit
				= calcUnitSize(totalLength) - (*m_ite)->getDataUnitSize();

			// エリアの領域を広げる
			if (expandArea(expandUnit) == false)
				return false;
		}

		m_pLeafPage->dirty();

		// 先頭文書IDを追加する
		if (uiLastDocumentID == 0)
		{
			// 今のショートリストには文書IDがない
			(*m_ite)->setFirstDocumentID(pArea->getFirstDocumentID());
		}
		else
		{
			ModSize offset = (*m_ite)->getDocumentOffset();
			// 圧縮して格納する
			writeDocumentID((*m_ite)->getLastDocumentID(),
							pArea->getFirstDocumentID(),
							(*m_ite)->getTailAddress(), offset);
			(*m_ite)->setDocumentOffset(offset);
		}

		// 残りの文書ID
		{
			ModSize offset = (*m_ite)->getDocumentOffset();
			// コピーする
			InvertedList::moveBack((*m_ite)->getTailAddress(), offset,
								   pArea->getTailAddress(), 0,
								   pArea->getDocumentOffset());
			(*m_ite)->setDocumentOffset(offset + pArea->getDocumentOffset());
		}

		// 位置情報
		{
			ModSize offset = (*m_ite)->getLocationOffset();
			// コピーする
			InvertedList::move((*m_ite)->getHeadAddress(), offset,
								pArea->getHeadAddress(), 0,
							   pArea->getLocationOffset());
			(*m_ite)->setLocationOffset(offset + pArea->getLocationOffset());
		}

		// 最終文書IDを設定する
		(*m_ite)->setLastDocumentID(pArea->getLastDocumentID());
		// 頻度情報を設定する
		(*m_ite)->setDocumentCount((*m_ite)->getDocumentCount() +
								   pArea->getDocumentCount());
	}
	else
	{
		// 存在していないので、ショートリストの範囲ならそのままコピー

		// 正確なデータユニット数を求める
		ModSize totalLength
			= pArea->getDocumentOffset() + pArea->getLocationOffset();
		ModSize dataUnitSize = calcUnitSize(totalLength);

		if (pArea->getUnitSize() - pArea->getDataUnitSize() + dataUnitSize >
			m_pLeafPage->getMaxAreaUnitSize())
			// ショートリストの範囲外
			return false;

		m_pLeafPage->dirty();

		LeafPage::AutoArea pNewArea;
		if (pArea->getDataUnitSize() != dataUnitSize)
		{
			// 正確なデータユニット数のエリアを確保する
			pNewArea = LeafPage::Area::allocateArea(getKey(), dataUnitSize);
			pNewArea->setListType(ListType::Short);
			pNewArea->setDocumentCount(pArea->getDocumentCount());
			pNewArea->setLastDocumentID(pArea->getLastDocumentID());
			pNewArea->setDocumentOffset(pArea->getDocumentOffset());
			pNewArea->setLocationOffset(pArea->getLocationOffset());
			pNewArea->setFirstDocumentID(pArea->getFirstDocumentID());
			// 文書IDデータのコピー
			InvertedList::moveBack(pNewArea->getTailAddress(), 0,
								   pArea->getTailAddress(), 0,
								   pArea->getDocumentOffset());
			// 位置情報データのコピー
			InvertedList::move(pNewArea->getHeadAddress(), 0,
							   pArea->getHeadAddress(), 0,
							   pArea->getLocationOffset());

			pArea = pNewArea;
		}
			
		// 挿入できるか？
		if (m_pLeafPage->getFreeUnitSize() < pArea->getUnitSize())
		{
			// もう領域がないので、split
			m_pLeafPage = m_pLeafPage->split(m_cInvertedUnit, getKey(),
											 pArea->getDataUnitSize(), m_ite);
		}

		// 挿入する
		m_ite = m_pLeafPage->insert(m_cInvertedUnit, pArea);

		m_bExist = true;
	}

	return true;
}

//
//	FUNCTION public
//	FullText2::ShortBaseList::verify -- 整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//	Admin::Verification::Treatment::Value uiTreatment_
//		不整合発見時の動作
//	Admin::Verification::Progress& cProgress_
//		不整合を通知するストリーム
//	const Os::Path& cRootPath_
//		転置ファイルのルートパス
//
//	RETURN
//	なし
//
//	EXCEPTINS
//
void
ShortBaseList::verify(Admin::Verification::Treatment::Value uiTreatment_,
					  Admin::Verification::Progress& cProgress_,
					  const Os::Path& cRootPath_)
{
	// 特に確認することはない
}

//
//	FUNCTION public
//	FullText2::ShortBaseList::convert -- ミドルリストへ変換する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::InvertedList*
//		ミドルリスト
//
//	EXCEPTIONS
//
InvertedList*
ShortBaseList::convert()
{
	MiddleBaseList* pMiddleList = 0;

	if (isExist() == true)
	{
		// まず既存のエリアをコピーする
		LeafPage::Area* pArea
			= LeafPage::Area::allocateArea(getKey(),
										   (*m_ite)->getDataUnitSize());
		pArea->copy(*m_ite);

		// エリアサイズを変更する
		ModSize uiAreaSize
			= LeafPage::calcAreaUnitSize(
				(*m_ite)->getKeyLength(),
				InvertedList::getIDBlockUnitSize(getKey()));
		int change = static_cast<int>(uiAreaSize) - (*m_ite)->getUnitSize();
		if (change > 0)
		{
			// 拡大する
			expandArea(change);
		}
		else if (change < 0)
		{
			// 縮小する
			shotenArea(-change);
		}

		// エリアを初期化する
		(*m_ite)->clear();
		(*m_ite)->setListType(ListType::Middle);
		(*m_ite)->setLastLocationPageID(
			PhysicalFile::ConstValue::UndefinedPageID);

		// ミドルリストを確保する
		pMiddleList = makeMiddleList(m_cInvertedUnit, m_pLeafPage, m_ite);

		// コピーしたエリアを挿入する
		ModAutoPointer<BatchBaseList> pBatchList
			= makeBatchList(m_cInvertedUnit, pArea);
		pMiddleList->insert(*pBatchList);
	}
	else
	{
		// 存在していないので、新しいエリアを挿入する

		// エリアサイズを得る
		ModSize uiDataUnit = InvertedList::getIDBlockUnitSize(getKey());
		ModSize uiAreaSize = LeafPage::calcAreaUnitSize(getKey(), uiDataUnit);

		// 挿入できるか？
		if (m_pLeafPage->getFreeUnitSize() < uiAreaSize)
		{
			// もう領域がないので、split
			m_pLeafPage = m_pLeafPage->split(m_cInvertedUnit, getKey(),
											 uiDataUnit, m_ite);
		}

		// 挿入する
		m_ite = m_pLeafPage->insert(m_cInvertedUnit, getKey(), uiDataUnit);
		// フォーマット識別フラグを設定する
		(*m_ite)->setListType(ListType::Middle);
		(*m_ite)->setLastLocationPageID(
			PhysicalFile::ConstValue::UndefinedPageID);

		// ミドルリストを確保する
		pMiddleList = makeMiddleList(m_cInvertedUnit, m_pLeafPage, m_ite);

		m_bExist = true;
	}

	return pMiddleList;
}

//
//	FUNCTION protected
//	FullText2::ShortBaseList::insertOrExpandArea
//		-- LeafPageのエリアを挿入または拡大する
//
//	NOTES
//	LeafPageのエリアを拡大する。必要ならページ分割を行い、
//	メンバー変数のページとエリアを該当するエリアのものに更新する
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//	ModSize uiLocBitLength_
//		位置情報全体のビット長
//
//	RETURN
//	bool
//		ショートリストの範囲内ならtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
ShortBaseList::insertOrExpandArea(ModUInt32 uiDocumentID_,
								  ModSize uiLocBitLength_)
{
	if (isExist() == true)
	{
		// 現在のエリアを拡大する

		// 最終文書ID
		ModUInt32 uiLastDocumentID = (*m_ite)->getLastDocumentID();

		// 文書IDの圧縮ビット長を求める
		ModSize idLength = 0;
		if (uiLastDocumentID != 0)
			idLength = getCompressedBitLengthDocumentID(uiLastDocumentID,
														uiDocumentID_);

		// 結局何ビット？
		ModSize totalLength
			= (*m_ite)->getDocumentOffset() + idLength +
			(*m_ite)->getLocationOffset() + uiLocBitLength_;

		if (calcUnitSize(totalLength) > (*m_ite)->getDataUnitSize())
		{
			// で、何ユニット必要？
			ModSize expandUnit
				= calcUnitSize(totalLength) - (*m_ite)->getDataUnitSize();

			// 拡張する
			return expandArea(expandUnit);
		}
	}
	else
	{
		// 新しいエリアを作成する

		// データ部は何ユニット？
		ModSize dataUnit = calcUnitSize(uiLocBitLength_);
		// エリアの大きさは？
		ModSize areaUnit = LeafPage::calcAreaUnitSize(getKey(), dataUnit);

		// ショートリストの範囲かチェックする
		if (areaUnit > m_pLeafPage->getMaxAreaUnitSize())
			return false;

		// 挿入できるか？
		if (m_pLeafPage->getFreeUnitSize() < areaUnit)
		{
			// もう領域がないので、split
			m_pLeafPage = m_pLeafPage->split(m_cInvertedUnit,
											 getKey(), dataUnit, m_ite);
		}

		// 挿入する
		m_ite = m_pLeafPage->insert(m_cInvertedUnit, getKey(), dataUnit);
		// フォーマット識別フラグを設定する
		(*m_ite)->setListType(ListType::Short);

		m_bExist = true;
	}

	return true;
}

//
//	FUNCTION protected
//	FullText2::ShortBaseList::expandArea -- エリアを拡張する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiExpandUnit_
//		拡張するユニット数
//
//	RETURN
//	bool
//		拡張した結果ショートリストの範囲だった場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
ShortBaseList::expandArea(ModSize uiExpandUnit_)
{
	// ショートリストの範囲か？
	if ((*m_ite)->getUnitSize() + uiExpandUnit_ >
		m_pLeafPage->getMaxAreaUnitSize())
		return false;

	// 広げられるか？
	if (m_pLeafPage->getFreeUnitSize() < uiExpandUnit_)
	{
		// もう領域がないので、split
		m_pLeafPage = m_pLeafPage->split(m_cInvertedUnit,
										 uiExpandUnit_, m_ite);
	}

	// 広げる
	bool result = m_pLeafPage->changeAreaSize(m_ite, uiExpandUnit_);
	; _SYDNEY_ASSERT(result);

	// 文書IDを広げた分移動する
	ModSize unit = calcUnitSize((*m_ite)->getDocumentOffset());
	if (unit)
	{
		ModOsDriver::Memory::move(
			(*m_ite)->getTailAddress() - unit,
			(*m_ite)->getTailAddress() - unit - uiExpandUnit_,
			unit*sizeof(ModUInt32));
		setOff((*m_ite)->getHeadAddress(), (*m_ite)->getLocationOffset(), 
			   (*m_ite)->getDataUnitSize() * 32 - (*m_ite)->getLocationOffset()
			   - (*m_ite)->getDocumentOffset());
	}

	return true;
}

//
//	FUNCTION protected
//	FullText2::ShortBaseList::shotenArea -- エリアを縮小する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiShotenUnit_
//		縮小するユニット数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortBaseList::shotenArea(ModSize uiShotenUnit_)
{
	// 縮める
	m_pLeafPage->changeAreaSize(m_ite, -static_cast<int>(uiShotenUnit_));

	if (m_pLeafPage->getFreeUnitSize()
		- sizeof(LeafPage::DirBlock)/sizeof(ModUInt32) >
		m_pLeafPage->getPageUnitSize() / 2)
	{
		// 空き領域が半分を超える場合
		// [NOTE] DIRブロック一個分はマージンを取っておく。
		
		// ページ結合や再配置をする
		m_pLeafPage = m_pLeafPage->reduce(m_cInvertedUnit, m_ite);
	}
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
