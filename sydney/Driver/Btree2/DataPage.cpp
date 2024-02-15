// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataPage.cpp --
// 
// Copyright (c) 2003, 2004, 2005, 2014, 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Btree2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Btree2/DataPage.h"
#include "Btree2/BtreeFile.h"

_SYDNEY_USING
_SYDNEY_BTREE2_USING

namespace
{
	//
	//	VARIABLE local
	//
	ModUInt32 _LEAF_MASK =	0x80000000;
}

//
//	FUNCTION public
//	Btree2::DataPage::DataPage -- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::BtreeFile& cFile_
//		ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
DataPage::DataPage(BtreeFile& cFile_)
	: Page(cFile_), m_cFile(cFile_)
{
}

//
//	FUNCTION public
//	Btree2::DataPage::setPhysicalPage -- 物理ページを設定する
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
DataPage::setPhysicalPage(PhysicalFile::Page* pPhysicalPage_)
{
	Page::setPhysicalPage(pPhysicalPage_);
	m_pHeader = syd_reinterpret_cast<Header*>(Page::getBuffer());
}

//
//	FUNCTION public
//	Btree2::DataPage::setPhysicalPage -- 物理ページを設定する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPhysicalPage_
//		物理ページ
//	PhysicalFile::PageID uiPrevPageID_
//		前のページ
//	PhysicalFile::PageID uiNextPageID_
//		後のページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataPage::setPhysicalPage(PhysicalFile::Page* pPhysicalPage_,
						  PhysicalFile::PageID uiPrevPageID_,
						  PhysicalFile::PageID uiNextPageID_)
{
	Page::setPhysicalPage(pPhysicalPage_);
	m_pHeader = syd_reinterpret_cast<Header*>(Page::getBuffer());
	dirty();
	m_pHeader->m_uiCount = 0;
	m_pHeader->m_uiPrevPageID = uiPrevPageID_;
	m_pHeader->m_uiNextPageID = uiNextPageID_;
}

//
//	FUNCTION public
//	Btree2::DataPage::~DataPage -- デストラクタ
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
DataPage::~DataPage()
{
}

//
//	FUNCTION public
//	Btree2::DataPage::getPageSize -- ページサイズを得る(UNIT単位)
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		ヘッダーを除いたページサイズ
//
//	EXCEPTIONS
//
ModSize
DataPage::getPageSize()
{
	return Page::getPageSize() - sizeof(Header)/sizeof(ModUInt32);
}

//
//	FUNCTION public
//	Btree2::DataPage::isRoot -- ルートページか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		ルートページの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
DataPage::isRoot() const
{
	return (m_pHeader->m_uiPrevPageID
			== PhysicalFile::ConstValue::UndefinedPageID
			&& m_pHeader->m_uiNextPageID
			== PhysicalFile::ConstValue::UndefinedPageID);
}

//
//	FUNCTION public
//	Btree2::DataPage::isLeaf -- リーフページか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		リーフページの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
DataPage::isLeaf() const
{
	return (m_pHeader->m_uiCount & _LEAF_MASK);
}

//
//	FUNCTION public
//	Btree2::DataPage::setLeaf -- リーフページに設定する
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
DataPage::setLeaf()
{
	m_pHeader->m_uiCount |= _LEAF_MASK;
}

//
//	FUNCTION public
//	Btree2::DataPage::preFlush -- 確定前処理を行う
//
//	NOTES
//	ページがdirtyの時しか呼び出されない
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataPage::preFlush()
{
	m_cFile.pushPageID(getID());
}

//
//	FUNCTION public
//	Btree2::DataPage::getCount -- ページのエントリ数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		ページ内のエントリ数
//
//	EXCEPTIONS
//
ModUInt32
DataPage::getCount()
{
	return m_pHeader->m_uiCount & ~_LEAF_MASK;
}

//
//	FUNCTION protected
//	Btree2::DataPage::getBuffer -- データ領域へのポインタを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32*
//		データ領域へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
DataPage::getBuffer()
{
	return Page::getBuffer() + sizeof(Header)/sizeof(ModUInt32);
}

//
//	FUNCTION protected
//	Btree2::DataPage::addCount -- ページのエントリ数を増やす
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiCount_
//		 増やす数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataPage::addCount(ModUInt32 uiCount_)
{
	dirty();
	m_pHeader->m_uiCount += uiCount_;
}

//
//	FUNCTION protected
//	Btree2::DataPage::subtractCount -- ページのエントリ数を減らす
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiCount_
//		 減らす数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataPage::subtractCount(ModUInt32 uiCount_)
{
	dirty();
	m_pHeader->m_uiCount -= uiCount_;
}

//
//	FUNCTION protected
//	Btree2::DataPage::getDataClass -- データクラスを得る
//
// 	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Btree2::Data&
//		 データクラス
//
//	EXCEPTIONS
//
const Data&
DataPage::getDataClass()
{
	return (isLeaf() == true) ? m_cFile.getLeafData() : m_cFile.getNodeData();
}

//
//	FUNCTION protected
//	Btree2::DataPage::getKeyDataClass -- データクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Btree2::Data&
//		データクラス
//
//	EXCEPTIONS
//
const Data&
DataPage::getKeyDataClass()
{
	return m_cFile.getKeyData();
}

//
//	FUNCTION protected
//	Btree2::DataPage::getCompareClass -- 比較クラスを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Btree2::Compare&
//		比較クラス
//
//	EXCEPTIONS
//
const Compare&
DataPage::getCompareClass(bool onlyKey_)
{
	return m_cFile.getCompare(onlyKey_);
}

//
//	FUNCTION protected
//	Btree2::Page::updateMode -- 更新モードに変更する
//
//	NOTES
//	ReadOnlyでattachしている物理ページをWriteでattachしなおす。
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
DataPage::updateMode()
{
	if (!isReadOnly()) return;
	
	DataPage::setPhysicalPage(
		m_cFile.changeFixMode(
			getPhysicalPage()));
	
	reload();
}

//
//	FUNCTION protected
//	Btree2::DataPage::getLockName -- ロック名を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Lock::FileName&
//		このファイルのロック名
//
//	EXCEPTIONS
//
const Lock::FileName&
DataPage::getLockName() const
{
	return m_cFile.getLockName();
}

//
//	FUNCTION protected
//	Btree2::DataPage::getTransaction -- トランザクションを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Trans::Transaction&
//		このファイルをオープンしたトランザクション
//
//	EXCEPTIONS
//
const Trans::Transaction&
DataPage::getTransaction() const
{
	return m_cFile.getTransaction();
}

//
//	Copyright (c) 2003, 2004, 2005, 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
