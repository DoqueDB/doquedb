// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LeafPage.cpp --
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

#include "FullText2/LeafPage.h"
#include "FullText2/LeafFile.h"
#include "FullText2/InvertedList.h"
#include "FullText2/InvertedUnit.h"

#include "Common/Assert.h"
#include "Os/Memory.h"

#include "ModAlgorithm.h"
#include "ModOsDriver.h"
#include "ModUnicodeCharTrait.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::LeafPage::Area::getHeadAddress -- データ部の先頭のアドレスを得る
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
ModUInt32*
LeafPage::Area::getHeadAddress()
{
	ModUInt32* p = syd_reinterpret_cast<ModUInt32*>(this);
	return p + calcAreaUnitSize(m_usKeyLength, 0);
}

//
//	FUNCTION public
//	FullText2::LeafPage::Area::getTailAddress -- データ部を終端のアドレスを得る
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
ModUInt32*
LeafPage::Area::getTailAddress()
{
	ModUInt32* p = syd_reinterpret_cast<ModUInt32*>(this);
	return p + getUnitSize();
}

//
//	FUNCTION public
//	FullText2::LeafPage::Area::getIDBlock -- IDブロックを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::OverflowPage::IDBlock
//		IDブロック
//
//	EXCEPTIONS
//
OverflowPage::IDBlock
LeafPage::Area::getIDBlock()
{
	OverflowPage::IDBlock cIDBlock(
		syd_reinterpret_cast<OverflowPage::IDBlock::Header*>(getHeadAddress()),
		InvertedList::getIDBlockUnitSize(m_pszKey));
	return cIDBlock;
}

//
//	FUNCTION public
//	FullText2::LeafPage::Area::getDirBlock
//		-- DIRブロックへの先頭のアドレスを得る
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	FullText2::LeafPage::DirBlock*
//		DIRブロックへの先頭アドレス。ミドルリストじゃない場合は0
//
//	EXCEPTIONS
//
LeafPage::DirBlock*
LeafPage::Area::getDirBlock()
{
	if (getListType() != ListType::Middle)
		return 0;
	return syd_reinterpret_cast<DirBlock*>(
		getHeadAddress() + InvertedList::getIDBlockUnitSize(m_pszKey));
}

//
//	FUNCTION public
//	FullText2::LeafPage::Area::getDirBlockCount -- DIRブロックの個数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		DIRブロックの個数
//
//	EXCEPTIONS
//
int
LeafPage::Area::getDirBlockCount()
{
	return static_cast<int>(
		getTailAddress() - syd_reinterpret_cast<ModUInt32*>(getDirBlock())) /
		(sizeof(DirBlock) / sizeof(ModUInt32));
}

//
//	FUNCTION public
//	FullText2::LeafPage::Area::copy -- 内容をコピーする
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::LeafPage::Area* pSrc_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LeafPage::Area::copy(const Area* pSrc_)
{
	ModUInt32 uiSize = getUnitSize();
	ModOsDriver::Memory::copy(this, pSrc_,
							  pSrc_->getUnitSize() * sizeof(ModUInt32));
	setUnitSize(uiSize);
}

//
//	FUNCTION public
//	FullText2::LeafPage::Area::clear -- 内容をクリアする
//
//	NOTES
//	サイズ等が書かれている先頭ユニットと索引単位以外の内容をクリアする
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
LeafPage::Area::clear()
{
	setDocumentCount(0);
	setLastDocumentID(0);
	setDocumentOffset(0);
	setLocationOffset(0);
	setFirstDocumentID(0);
	ModOsDriver::Memory::reset(getHeadAddress(),
							   getDataUnitSize() * sizeof(ModUInt32));
}

//
//	FUNCTION public static
//	FullText2::LeafPage::Area::allocateArea -- 一時的なエリアを確保する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar* pszKey_
//		索引単位
//	ModSize uiDataUnitSize_
//		データ部のユニット長 (default 0)
//
//	RETURN
//	FullText2::LeafPage::Area*
//		確保した一時的なエリア
//
//	EXCEPTIONS
//
LeafPage::Area*
LeafPage::Area::allocateArea(const ModUnicodeChar* pszKey_,
							 ModSize uiDataUnitSize_)
{
	ModSize uiKeyLength = ModUnicodeCharTrait::length(pszKey_);
	ModSize size = calcAreaUnitSize(uiKeyLength, uiDataUnitSize_);
	Area* pArea = static_cast<Area*>(
		Os::Memory::allocate(size * sizeof(ModUInt32)));

	ModOsDriver::Memory::reset(pArea, size * sizeof(ModUInt32));

	pArea->m_uiHeader = ListType::Short | size;
	pArea->m_usKeyLength = static_cast<unsigned short>(uiKeyLength);
	ModOsDriver::Memory::copy(pArea->m_pszKey, pszKey_,
							  uiKeyLength * sizeof(ModUnicodeChar));

	return pArea;
}

//
//	FUNCTION public
//	FullText2::LeafPage::LeafPage -- コンストラクタ(1)
//
//	NOTES
//	attachしたページのためのコンストラクタ
//
//	ARGUMENTS
//	FullText2::LeafFile& cFile_
//		リーフファイル
//	PhysicalFile::Page* pPhysicalPage_
//		物理ファイルのページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
LeafPage::LeafPage(LeafFile& cFile_, PhysicalFile::Page* pPhysicalPage_)
	: Page(cFile_, pPhysicalPage_), m_cFile(cFile_), m_pHeader(0)
{
	// 内容を読み込む
	load();
}

//
//	FUNCTION public
//	FullText2::LeafPage::LeafPage -- コンストラクタ(2)
//
//	NOTES
//	新たにallocateしたページのためのコンストラクタ
//
//	ARGUMENTS
//	FullText2::LeafFile& cFile_
//		リーフファイル
//	PhysicalFile::Page* pPhysicalPage_
//		物理ファイルのページ
//	PhysicalFile::PageID uiPrevPageID_
//		前ページのページID
//	PhysicalFile::PageID uiNextPageID_
//		次ページのページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
LeafPage::LeafPage(LeafFile& cFile_, PhysicalFile::Page* pPhysicalPage_,
				   PhysicalFile::PageID uiPrevPageID_,
				   PhysicalFile::PageID uiNextPageID_)
	: Page(cFile_, pPhysicalPage_), m_cFile(cFile_), m_pHeader(0)
{
	// 物理ページをDirtyにする
	dirty();

	Header* pHeader = syd_reinterpret_cast<Header*>(getBuffer());

	// 前後のページIDを書き込む
	pHeader->m_uiPrevPageID = uiPrevPageID_;
	pHeader->m_uiNextPageID = uiNextPageID_;
	// エリア個数に0を設定する
	pHeader->m_uiCount = 0;

	// 内容を読み込む
	load();
}

//
//	FUNCTION public
//	FullText2::LeafPage::~LeafPage -- デストラクタ
//
//	NOTES
//	デストラクタ。
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
LeafPage::~LeafPage()
{
}

//
//	FUNCTION public
//	FullText2::LeafPage::reset -- 内容をリセットする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPrevPageID_
//		前方のページID
//	PhysicalFile::PageID uiNextPageID_
//		後方のページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LeafPage::reset(PhysicalFile::PageID uiPrevPageID_,
				PhysicalFile::PageID uiNextPageID_)
{
	Header* pHeader = syd_reinterpret_cast<Header*>(getBuffer());

	// 前後のページIDを書き込む
	pHeader->m_uiPrevPageID = uiPrevPageID_;
	pHeader->m_uiNextPageID = uiNextPageID_;
	// エリア個数に0を設定する
	pHeader->m_uiCount = 0;

	// 物理ページをDirtyにする
	dirty();

	// 内容を読み込む
	load();
}

//
//	FUNCTION public
//	FullText2::LeafPage::reset -- リセットする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPhysicalPage_
//		物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LeafPage::reset(PhysicalFile::Page* pPhysicalPage_)
{
	Page::reset(pPhysicalPage_);
	m_pHeader = 0;
	load();
}

//
//	FUNCTION public
//	Inveted::LeafPage::reset -- リセットする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPhysicalPage_
//		物理ページ
//	PhysicalFile::PageID uiPrevPageID_
//		前ページのページID
//	PhysicalFile::PageID uiNextPageID_
//		次ページのぺーじID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LeafPage::reset(PhysicalFile::Page* pPhysicalPage_,
				PhysicalFile::PageID uiPrevPageID_,
				PhysicalFile::PageID uiNextPageID_)
{
	Page::reset(pPhysicalPage_);
	m_pHeader = 0;

	// 物理ページをDirtyにする
	dirty();

	Header* pHeader = syd_reinterpret_cast<Header*>(getBuffer());

	// 前後のページIDを書き込む
	pHeader->m_uiPrevPageID = uiPrevPageID_;
	pHeader->m_uiNextPageID = uiNextPageID_;
	// エリア個数に0を設定する
	pHeader->m_uiCount = 0;

	// 内容を読み込む
	load();
}

//
//	FUNCTION public
//	FullText2::LeafPage::search -- エリアを検索する
//
//	NOTES
//	該当する索引単位のエリアを検索する。存在しない場合はend()を返す
//
//	ARGUMENTS
//	const ModUnicodeChar* pszKey_
//		検索する索引単位
//
//	RETURN
//	FullText2::LeafPage::Iterator
//		検索にヒットした場合は該当するエリアへのイテレータ
//		それ以外の場合はend()
//
//	EXCEPTIONS
//
LeafPage::Iterator
LeafPage::search(const ModUnicodeChar* pszKey_)
{
	Iterator result = end();

	// 一時的なエリアを得る
	AutoArea pArea = Area::allocateArea(pszKey_);
	// まずlower_boundで検索してから同じかどうかチェックする
	Iterator i = lowerBound(pArea);
	if (i != end() && Area::compare(*i, pArea) == 0)
		result = i;

	return result;
}

//
//	FUNCTION public
//	LeafPage::lowerBound -- エリアをlower_boundで検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar* pszKey_
//		検索する索引単位
//
//	RETURN
//	FullText2::LeafPage::Iterator
//		lower_boundで検索したエリアへのイテレータ
//
//	EXCEPTIONS
//
LeafPage::Iterator
LeafPage::lowerBound(const ModUnicodeChar* pszKey_)
{
	// 一時的なエリアを得る
	AutoArea pArea = Area::allocateArea(pszKey_);
	return lowerBound(pArea);
}

//
//	FUNCTION public
//	LeafPage::lowerBound -- エリアをlower_boundで検索する
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::LeafPage::Area* pArea_
//		検索するエリアへのポインタ。索引単位のみの比較に用いる
//
//	RETURN
//	FullText2::LeafPage::Iterator
//		lower_boundで検索したエリアへのイテレータ
//
//	EXCEPTIONS
//
LeafPage::Iterator
LeafPage::lowerBound(const Area* pArea_)
{
	if (begin() == end())
		return begin();
	return ModLowerBound(begin(), end(), pArea_, Area::Less());
}

//
//	FUNCTION public
//	LeafPage::upperBound -- エリアをupper_boundで検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar* pszKey_
//		検索する索引単位
//
//	RETURN
//	FullText2::LeafPage::Iterator
//		upper_boundで検索したエリアへのイテレータ
//
//	EXCEPTIONS
//
LeafPage::Iterator
LeafPage::upperBound(const ModUnicodeChar* pszKey_)
{
	// 一時的なエリアを得る
	AutoArea pArea = Area::allocateArea(pszKey_);
	return upperBound(pArea);
}

//
//	FUNCTION public
//	LeafPage::upperBound -- エリアをupper_boundで検索する
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::LeafPage::Area* pArea_
//		検索するエリアへのポインタ。索引単位のみの比較に用いる
//
//	RETURN
//	FullText2::LeafPage::Iterator
//		upper_boundで検索したエリアへのイテレータ
//
//	EXCEPTIONS
//
LeafPage::Iterator
LeafPage::upperBound(const Area* pArea_)
{
	return ModUpperBound(begin(), end(), pArea_, Area::Less());
}

//
//	FUNCTION public
//	FullText2::LeafPage::insert -- エリアを挿入する
//
//	NOTES
//	指定された索引単位の指定されたデータ領域のエリアをページに挿入し、
//	確保されたエリアへのイテレータを得る
//
//	ARGUMENTS
//	FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	const ModUnicodeChar* pszKey_
//		索引単位
//	ModSize uiDataUnitSize_
//		データ領域のユニット数
//
//	RETURN
//	FullText2::LeafPage::Iterator
//		挿入されたエリアへのイテレータ。挿入できなかった場合はend()を返す
//
//	EXCEPTIONS
//
LeafPage::Iterator
LeafPage::insert(InvertedUnit& cInvertedUnit_,
				 const ModUnicodeChar* pszKey_,
				 ModSize uiDataUnitSize_)
{
	// 一時的なエリアを得る(結局検索しなくちゃいけないので...)
	AutoArea pArea = Area::allocateArea(pszKey_, uiDataUnitSize_);
	return insert(cInvertedUnit_, pArea);
}

//
//	FUCNTION public
//	FullText2::LeafPage::insert -- エリアを挿入する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	const FullText2::LeafPage::Area* pArea_
//		挿入するエリア
//
//	RETURN
//	FullText2::LeafPage::Iterator
//		挿入したエリアへのイテレータ。挿入できなかった場合はend()を返す
//
//	EXCEPTIONS
//
LeafPage::Iterator
LeafPage::insert(InvertedUnit& cInvertedUnit_, const Area* pArea_)
{
	Iterator result = end();

	if (getFreeUnitSize() >= pArea_->getUnitSize())
	{
		// 挿入できるので、挿入位置を得る
		Iterator i = lowerBound(pArea_);

		if (i == begin())
		{
			// 先頭に挿入される -> B木を更新する必要がある
			if (i == end())
			{
				// はじめての挿入なので、B木に挿入
				cInvertedUnit_.insertBtree(
					ModUnicodeString(pArea_->getKey(), pArea_->getKeyLength()),
					getID());
			}
			else
			{
				// 先頭が置き換わるので更新
				cInvertedUnit_.updateBtree(
					ModUnicodeString((*i)->getKey(), (*i)->getKeyLength()),
					getID(),
					ModUnicodeString(pArea_->getKey(), pArea_->getKeyLength()),
					getID());
			}
		}

		// サイズ分移動する
		ModUInt32* first = syd_reinterpret_cast<ModUInt32*>(*i);
		ModUInt32* last = syd_reinterpret_cast<ModUInt32*>(*end());
		// 移動する部分のサイズ
		ModUInt32 length = static_cast<ModUInt32>(last - first);
		if (length != 0)
		{
			// 挿入するところより後ろを移動する
			ModOsDriver::Memory::move(first + pArea_->getUnitSize(),
									  first, length * sizeof(ModUInt32));
		}

		// ベクターのアドレスを更新する
		Iterator s = i;
		for (; s <= end(); ++s)
		{
			(*s) = syd_reinterpret_cast<Area*>(
				syd_reinterpret_cast<ModUInt32*>(*s) + pArea_->getUnitSize());
		}

		// 見つかった位置に挿入する
		ModOsDriver::Memory::copy(first, pArea_,
								  pArea_->getUnitSize() * sizeof(ModUInt32));
		// ベクターにも追加する
		i = m_vecpArea.insert(i, syd_reinterpret_cast<Area*>(first));
		// ヘッダーを更新する
		m_pHeader->m_uiCount++;

		dirty();

		result = i;
	}

	return result;
}

//
//	FUNCTION public
//	FullText2::LeafPage::insert -- エリアを挿入する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	FullText2::LeafPage::ConstIterator first_
//		挿入するエリアの先頭
//	FullText2::LeafPage::ConstIterator last_
//		挿入するエリアの終端
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LeafPage::insert(InvertedUnit& cInvertedUnit_,
				 ConstIterator first_, ConstIterator last_)
{
	if (first_ != last_)
	{
		// 挿入する位置を求める
		Iterator i = lowerBound(*first_);

		if (i == begin())
		{
			// 先頭に挿入される -> B木を更新する必要がある
			if (i == end())
			{
				// はじめての挿入なので、B木に挿入
				cInvertedUnit_.insertBtree(
					ModUnicodeString((*first_)->getKey(),
									 (*first_)->getKeyLength()),
					getID());
			}
			else
			{
				// 先頭が置き換わるので更新
				cInvertedUnit_.updateBtree(
					ModUnicodeString((*i)->getKey(),
									 (*i)->getKeyLength()),
					getID(),
					ModUnicodeString((*first_)->getKey(),
									 (*first_)->getKeyLength()),
					getID());
			}
		}

		ModUInt32* first = syd_reinterpret_cast<ModUInt32*>(*i);
		ModUInt32* last = syd_reinterpret_cast<ModUInt32*>(*end());
		// 移動する部分の長さ
		ModUInt32 length = static_cast<ModUInt32>(last - first);
		ModUInt32* f = syd_reinterpret_cast<ModUInt32*>(*first_);
		ModUInt32* l = syd_reinterpret_cast<ModUInt32*>(*last_);
		ModUInt32 unit = static_cast<ModUInt32>(l - f);
		if (length != 0)
		{
			// 大きい部分を後ろに移動する
			ModOsDriver::Memory::move(first + unit,
									  first, length * sizeof(ModUInt32));
		}

		// コピーする
		ModOsDriver::Memory::copy(first, f, unit * sizeof(ModUInt32));

		// ヘッダーを更新する
		m_pHeader->m_uiCount += last_ - first_;

		// ベクターを設定しなおす
		loadEntry();

		dirty();
	}
}

//
//	FUNCTION public
//	FullText2::LeafPage::expunge -- エリアを削除する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	FullText2::LeafPage::Iterator i
//		削除するエリアへのイテレータ
//
//	RETURN
//	FullText2::LeafPage::Iterator
//		削除した要素の次のイテレータ
//
//	EXCEPTIONS
//
LeafPage::Iterator
LeafPage::expunge(InvertedUnit& cInvertedUnit_, Iterator i)
{
	int ops = i - begin();
	expunge(cInvertedUnit_, i, i+1);
	return begin() + ops;
}

//
//	FUNCTION public
//	FullText2::LeafPage::expunge -- エリアを削除する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	FullText2::LeafPage::Iterator first_
//		削除するエリアの先頭
//	FullText2::LeafPage::Iterator last_
//		削除するエリアの終端
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LeafPage::expunge(InvertedUnit& cInvertedUnit_, Iterator first_, Iterator last_)
{
	if (first_ != last_)
	{
		if (first_ == begin())
		{
			// 先頭が削除される -> B木を更新する必要がある
			if (last_ == end())
			{
				// 全部削除なので、B木から削除
				cInvertedUnit_.expungeBtree(
					ModUnicodeString((*first_)->getKey(),
									 (*first_)->getKeyLength()),
					getID());
			}
			else
			{
				// 先頭が置き換わるので更新
				cInvertedUnit_.updateBtree(
					ModUnicodeString((*first_)->getKey(),
									 (*first_)->getKeyLength()),
					getID(),
					ModUnicodeString((*last_)->getKey(),
									 (*last_)->getKeyLength()),
					getID());
			}
		}

		// ヘッダーを更新する
		m_pHeader->m_uiCount -= last_ - first_;

		if (last_ == end())
		{
			// 最後なのでベクターを消すだけ
			m_vecpArea.erase(first_ + 1, last_ + 1);
		}
		else
		{
			ModUInt32* f = syd_reinterpret_cast<ModUInt32*>(*first_);
			ModUInt32* l = syd_reinterpret_cast<ModUInt32*>(*last_);
			ModUInt32 length = static_cast<ModSize>(
				syd_reinterpret_cast<ModUInt32*>(*end()) - l);

			// 移動する
			ModOsDriver::Memory::move(f, l, length * sizeof(ModUInt32));

			// ベクターを設定しなおす
			loadEntry();
		}

		dirty();
	}
}

//
//	FUNCTION public
//	FullText2::LeafPage::changeAreaSize -- エリアサイズを変更する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::LeafPage::Iterator i
//		変更するエリアのイテレータ
//	int iDataUnitSize_
//		変更するのデータユニット数。増減値を指定する
//
//	RETURN
//	bool
//		変更できた場合はtrue、ページがいっぱい等で変更できなかった場合はfalse
//
//	EXCEPTIONS
//
bool
LeafPage::changeAreaSize(Iterator i, int iDataUnitSize_)
{
	if (static_cast<int>(getFreeUnitSize()) < iDataUnitSize_)
		return false;

	// サイズ分移動する
	Iterator next = i + 1;
	ModUInt32* first = syd_reinterpret_cast<ModUInt32*>(*next);
	ModUInt32* last = syd_reinterpret_cast<ModUInt32*>(*end());
	// 移動する部分のサイズ
	ModUInt32 length = static_cast<ModUInt32>(last - first);
	if (length != 0)
	{
		// 変更するところより後ろを移動する
		ModOsDriver::Memory::move(first + iDataUnitSize_,
								  first, length * sizeof(ModUInt32));
	}

	// ベクターのアドレスを更新する
	Iterator s = next;
	for (; s <= end(); ++s)
	{
		(*s) = syd_reinterpret_cast<Area*>(
			syd_reinterpret_cast<ModUInt32*>(*s) + iDataUnitSize_);
	}

	if (iDataUnitSize_ > 0)
	{
		ModUInt32* pTail = (*i)->getTailAddress();
		ModOsDriver::Memory::reset(pTail, iDataUnitSize_ * sizeof(ModUInt32));
	}
	// ユニット数を変更する
	(*i)->setUnitSize((*i)->getUnitSize() + iDataUnitSize_);

	dirty();

	return true;
}

//
//	FUNCTION public
//	FullText2::LeafPage::setNextPageID -- 次ページのIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiNextPageID_
//		次ページのID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	???
//
void
LeafPage::setNextPageID(PhysicalFile::PageID uiNextPageID_)
{
	// 次ページIDを書き込む
	m_pHeader->m_uiNextPageID = uiNextPageID_;
	dirty();
}

//
//	FUNCTION public
//	FullText2::LeafPage::setPrevPageID -- 前ページのIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPrevPageID_
//		前ページのID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	???
//
void
LeafPage::setPrevPageID(PhysicalFile::PageID uiPrevPageID_)
{
	// 次ページIDを書き込む
	m_pHeader->m_uiPrevPageID = uiPrevPageID_;
	dirty();
}

//
//	FUNCTION public
//	FullText2::LeafPage::getUsedUnitSize -- 使用ユニット数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		使用ユニット数
//
//	EXCEPTIONS
//	なし
//
ModSize
LeafPage::getUsedUnitSize() const
{
	const ModUInt32* first = getBuffer();
	const ModUInt32* last = syd_reinterpret_cast<const ModUInt32*>(*end());
	return static_cast<ModSize>(last - first);
}

//
//	FUNCTION public
//	FullText2::LeafPage::getMaxAreaUnitSize -- 最大エリアユニットサイズを得る
//
//	NOTES
//	1ページに1エリアしかない場合の、最大ユニットサイズを得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		最大データユニットサイズ
//
//	EXCEPTIONS
//
ModSize
LeafPage::getMaxAreaUnitSize() const
{
	return getPageUnitSize() - sizeof(Header) / sizeof(ModUInt32);
}

//
//	FUNCTION public
//	FullText2::LeafPage::split -- ページ分割する
//
//	NOTES
//	該当するエリアが存在していない場合
//
//	ARGUMENTS
//	FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	const ModUnicodeChar* pszKey_
//		新たに挿入するキー値
//	ModSize uiDataUnitSize_
//		新たに挿入する転置リストのデータユニット数
//	FullText2::LeafPage::Iterator i_
//		入力はlower_boundしたイテレータ
//
//	RETURN
//	FullText2::LeafPage::PagePointer
//		挿入したエリアが存在するページ
//
//	EXCEPTIONS
//
LeafPage::PagePointer
LeafPage::split(InvertedUnit& cInvertedUnit_, const ModUnicodeChar* pszKey_,
				ModSize uiDataUnitSize_, Iterator i_)
{
	// 該当するエリアが存在しないので、
	// イテレータが指すエリアのキーと新たに挿入するキーとは一致しない。
	; _TRMEISTER_ASSERT((*i_)->getKey() != pszKey_);

	// 最大エリアユニット数
	ModSize maxUnitSize = getMaxAreaUnitSize();

	// まず2分割できるかどうかチェックする
	ModSize prev = 0;
	ModSize next = 0;
	Iterator i = begin();
	for (; i < i_; ++i)
		prev += (*i)->getUnitSize();
	for (; i < end(); ++i)
		next += (*i)->getUnitSize();

	// 挿入するエリアのユニット数
	ModSize uiAreaUnitSize = calcAreaUnitSize(pszKey_, uiDataUnitSize_);

	// 挿入するページ
	PagePointer pInsertPage = 0;

	if ((prev + uiAreaUnitSize) <= maxUnitSize ||
		(next + uiAreaUnitSize) <= maxUnitSize)
	{
		// 2分割でOK!
		pInsertPage = splitTwoPages(cInvertedUnit_, prev, next,
									0, uiAreaUnitSize);
	}
	else
	{
		// 3分割
		pInsertPage = splitThreePages(cInvertedUnit_, i_, i_);
	}

	return pInsertPage;
}

//
//	FUNCTION public
//	FullText2::LeafPage::split -- ページ分割する
//
//	NOTES
//	広げるべきエリアが存在している場合
//
//	ARGUMENTS
//	FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	ModSize uiDataUnitSize_
//		拡張するユニット数
//	FullText2::LeafPage::Iterator& i_
//		広げるエリアへのイテレータ
//		ページが変わっている場合にはイテレータも変わる
//
//	RETURN
//	FullText2::LeafPage::PagePointer
//		広げたエリアが存在するページ
//
//	EXCEPTIONS
//
LeafPage::PagePointer
LeafPage::split(InvertedUnit& cInvertedUnit_,
				ModSize uiDataUnitSize_, Iterator& i_)
{
	// 最大エリアユニット数
	ModSize maxUnitSize = getMaxAreaUnitSize();

	// キー値
	ModUnicodeString cstrKey((*i_)->getKey(), (*i_)->getKeyLength());

	// まず2分割できるかどうかチェックする
	ModSize prev = 0;
	ModSize next = 0;
	Iterator i = begin();
	for (; i < i_; ++i)
		prev += (*i)->getUnitSize();
	++i;
	for (; i < end(); ++i)
		next += (*i)->getUnitSize();

	// 拡張するエリアのユニット数
	ModSize uiAreaUnitSize = (*i_)->getUnitSize() + uiDataUnitSize_;

	// 拡張するエリアが存在するページ
	PagePointer pExpandPage = 0;

	if ((prev + uiAreaUnitSize) <= maxUnitSize ||
		(next + uiAreaUnitSize) <= maxUnitSize)
	{
		// 2分割でOK!
		pExpandPage = splitTwoPages(cInvertedUnit_, prev, next,
									(*i_)->getUnitSize(), uiAreaUnitSize);
	}
	else
	{
		// 3分割
		pExpandPage = splitThreePages(cInvertedUnit_, i_, i_+1);
	}

	// 拡張するエリアのイテレータの設定
	if (pExpandPage != this)
		// 拡張するエリアが含まれるページが変更された場合
		i_ = pExpandPage->search(cstrKey);

	return pExpandPage;
}

//
//	FUNCTION public
//	FullText2::LeafPage::reduce -- 可能なら隣りのページとマージする
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	FullText2::LeafPage::Iterator& i_
//		現在のイテレータ
//
//	RETURN
//	FullText2::LeafPage::PagePointer
//		マージされたページ
//
//	EXCEPTIONS
//
LeafPage::PagePointer
LeafPage::reduce(InvertedUnit& cInvertedUnit_, Iterator& i_)
{
	if (getNextPageID() == PhysicalFile::ConstValue::UndefinedPageID)
		return this;

	// 次のページを得る
	PagePointer pNextPage = m_cFile.attachPage(getNextPageID());
	if (pNextPage->getFreeUnitSize() >= getUsedUnitSize())
	{
		// マージできる
		ModUnicodeString cstrKey((*i_)->getKey(), (*i_)->getKeyLength());

		insert(cInvertedUnit_, pNextPage->begin(), pNextPage->end());

		PhysicalFile::PageID uiNextNextPageID = pNextPage->getNextPageID();
		if (uiNextNextPageID != PhysicalFile::ConstValue::UndefinedPageID)
		{
			// 次の次のページを得る
			PagePointer pPage = m_cFile.attachPage(uiNextNextPageID);
			// 次の次のページの前のページをこのページにする
			pPage->setPrevPageID(getID());
		}
		// このページの次のページを変更する
		setNextPageID(uiNextNextPageID);

		// B木から次のページのエントリを削除する
		Iterator i = pNextPage->begin();
		ModUnicodeString cstrKey1((*i)->getKey(), (*i)->getKeyLength());
		cInvertedUnit_.expungeBtree(cstrKey1, pNextPage->getID());
		m_cFile.freePage(pNextPage);

		i_ = search(cstrKey);
	}

	return this;
}

//
//	FUNCTION private
//	FullText2::LeafPage::load -- ページ内容を読み込む
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
//	???
//
void
LeafPage::load()
{
	// ヘッダーを設定する
	m_pHeader = syd_reinterpret_cast<Header*>(getBuffer());

	// エントリを読み込む
	loadEntry();
}

//
//	FUNCTION private
//	FullText2::LeafPage::loadEntry -- エントリを読み込む
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
LeafPage::loadEntry()
{
	int count = getCount();

	ModUInt32* p = getBuffer();

	// データ領域へ移動する
	p += sizeof(Header) / sizeof(ModUInt32);

	// 配列を１つ多く確保する
	m_vecpArea.assign(count+1);

	int i;
	for (i = 0; i < count; ++i)
	{
		m_vecpArea[i] = syd_reinterpret_cast<Area*>(p);
		p += m_vecpArea[i]->getUnitSize();
	}
	m_vecpArea[i] = syd_reinterpret_cast<Area*>(p);
}

//
//	FUNCTION private
//	FullText2::LeafPage::splitTwoPages -- ページを2分割する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	ModSize prev_
//		挿入する前の容量
//	ModSize next_
//		挿入する後の容量
//	ModSize currentUnitSize_
//		現在のユニット数
//	ModSize expandUnitSize_
//		拡張後のユニット数
//
//	RETURN
//	FullText2::LeafPage::PagePointer
//		挿入するページ
//
//	EXCEPTIONS
//
LeafPage::PagePointer
LeafPage::splitTwoPages(InvertedUnit& cInvertedUnit_,
						ModSize prev_, ModSize next_,
						ModSize currentUnitSize_, ModSize expandUnitSize_)
{
	// 少なくとも、挿入or拡張エリアの前後のいずれかで二分割できる
	; _TRMEISTER_ASSERT(prev_ + expandUnitSize_ <= getMaxAreaUnitSize()
						|| next_ + expandUnitSize_ <= getMaxAreaUnitSize());

	PagePointer pInsertPage = 0;

	// 最大エリアユニット数
	ModSize maxUnitSize = getMaxAreaUnitSize();

	// このページにエリアを挿入したり
	// このページのエリアを拡張したりした場合の、増加サイズ
	ModSize size = 0;
	// このページにエリアを挿入したり
	// このページのエリアを拡張したりした後の、このページのサイズ
	// または、最大ページサイズの半分
	ModSize halfSize = maxUnitSize / 2;

	if ((prev_ + expandUnitSize_) > maxUnitSize)
	{
		// (prev_), (area + next_) と分割する場合 … (1)
		halfSize = prev_;

		// [NOTE] エリアの挿入の場合、
		//  分割後にエリアの挿入位置を、B木から再探索すると、
		//  まだareaは挿入されていないのでこのページが取得され、
		//	次ページを挿入ページとして返していることと矛盾する。
		//  しかし、呼び出し側ではB木を再探索していないので問題ない。

		// [YET] (prev1), (prev2 + area + next_) と分割し、
		//  両方のページサイズが均等になるようにした方が良くないか？
	}
	else if ((next_ + expandUnitSize_) > maxUnitSize)
	{
		// (prev_ + area), (next_) と分割する場合
		halfSize = prev_ + expandUnitSize_;
		pInsertPage = this;
		size = expandUnitSize_ - currentUnitSize_;
		
		// [YET] (prev1 + area + next1), (next2) と分割し、
		//  両方のページサイズが均等になるようにした方が良くないか？
	}
	else
	{
		// どちらのページにエリアを挿入しても、
		// または、拡張されるエリアをどちらのページに含めても、
		// 最大ページサイズを超えることがない場合

		if (prev_ < halfSize)
		{
			// このページに半分以上の空きがある場合
			
			// [NOTE] prev_ < next_ とは限らない。
			//  ただし prev_ + expandUnitSize <= maxUnitSize より、
			//  このページに収まることは保証されている。

			// このページに挿入する
			size = expandUnitSize_ - currentUnitSize_;
			pInsertPage = this;

			if ((prev_ + expandUnitSize_) > halfSize)
				// (prev_ + area), (next_) と分割する場合
				halfSize = prev_ + expandUnitSize_;
			// else
			//	(prev_ + area), (next_) または
			//	(prev_ + area + next1), (next2) と分割する場合 … (2)
		}
		// else
		//	(prev_), (area + next_) … (3) または
		//	(prev1), (prev2 + area + next_) と分割する場合
		
		// [NOTE] 上記(3)でエリアの挿入の場合、(1) の時と同様の問題がある。
		
		// [YET] 1ページ目のサイズが半分を超えるかどうかではなく、
		//  両方のページサイズが均等になるようにした方が良くないか？
	}

	// 分割する位置を取得
	Iterator i = begin();
	for (; i < end(); ++i)
	{
		size += (*i)->getUnitSize();
		if (size >= halfSize)
		{
			// 挿入or拡張後のこのページサイズの目安以上になった場合

			if (size > maxUnitSize)
			{
				// なんと、ページサイズを越えてしまった -> １つ前
				// [NOTE] 上記(2)で、next_またはnext2を調べた際にありうる。
				--i;
			}
			break;
		}
	}

	// 新しいページを確保する
	PagePointer pNewPage = m_cFile.allocatePage(getID(), getNextPageID());
	if (getNextPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 次ページをアタッチする
		PagePointer pNextPage = m_cFile.attachPage(getNextPageID());
		pNextPage->setPrevPageID(pNewPage->getID());
	}
	setNextPageID(pNewPage->getID());

	++i;
	// 新しいページに半分移動する
	pNewPage->insert(cInvertedUnit_, i, end());
	// 移動した分を削除する
	expunge(cInvertedUnit_, i, end());

	if (pInsertPage == 0)
	{
		// 新ページに挿入する
		pInsertPage = pNewPage;
	}

	return pInsertPage;
}

//
//	FUNCTION private
//	FullText2::LeafPage::splitThreePages -- 3分割する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	FullText2::LeafPage::Iterator prevEnd_
//		前半部分の終了位置
//	FullText2::LeafPage::Iterator nextStart_
//		後半部分の開始位置
//
//	RETURN
//	FullText2::LeafPage::PagePointer
//		挿入するページ
//
//	EXCEPTIONS
//
LeafPage::PagePointer
LeafPage::splitThreePages(InvertedUnit& cInvertedUnit_,
						  Iterator prevEnd_, Iterator nextStart_)
{
	// 新しいページを確保する
	PagePointer pNewPage1 = m_cFile.allocatePage(getID(), getNextPageID());
	setNextPageID(pNewPage1->getID());
	PagePointer pNewPage2 = m_cFile.allocatePage(
		pNewPage1->getID(), pNewPage1->getNextPageID());
	pNewPage1->setNextPageID(pNewPage2->getID());
	if (pNewPage2->getNextPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 次ページをアタッチする
		PagePointer pNextPage = m_cFile.attachPage(pNewPage2->getNextPageID());
		pNextPage->setPrevPageID(pNewPage2->getID());
	}

	// 新しいページ移動する
	if (prevEnd_ != nextStart_)
		pNewPage1->insert(cInvertedUnit_, prevEnd_, nextStart_);
	pNewPage2->insert(cInvertedUnit_, nextStart_, end());
	// 移動した分を削除する
	expunge(cInvertedUnit_, prevEnd_, end());

	return pNewPage1;
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
