// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataPage.cpp --
// 
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Lob";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Lob/DataPage.h"
#include "Lob/LobFile.h"

#include "ModOsDriver.h"

_SYDNEY_USING
_SYDNEY_LOB_USING

//
//	FUNCTION public
//	Lob::DataPage::DataPage -- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	Lob::LobFile& cFile_
//		ファイル
//	PhysicalFile::Page* pPhysicalPage_
//		物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
DataPage::DataPage(LobFile& cFile_, PhysicalFile::Page* pPhysicalPage_)
	: Page(cFile_, pPhysicalPage_), m_cFile(cFile_)
{
	m_pHeader = syd_reinterpret_cast<Header*>(getBuffer());
}

//
//	FUNCTION public
//	Lob::DataPage::DataPage -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	Lob::LobFile& cFile_
//		ファイル
//	PhysicalFile::Page* pPhysicalPage_
//		物理ページ
//	PhysicalFile::PageID uiPrevPageID_
//		前のページ
//	PhysicalFile::PageID uiNextPageID_
//		次のページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
DataPage::DataPage(LobFile& cFile_, PhysicalFile::Page* pPhysicalPage_,
					 PhysicalFile::PageID uiPrevPageID_,
					 PhysicalFile::PageID uiNextPageID_)
	: Page(cFile_, pPhysicalPage_), m_cFile(cFile_)
{
	dirty();
	m_pHeader = syd_reinterpret_cast<Header*>(getBuffer());
	m_pHeader->m_uiPrevPageID = uiPrevPageID_;
	m_pHeader->m_uiNextPageID = uiNextPageID_;
	m_pHeader->m_uiLength = 0;
}

//
//	FUNCTION public
//	Lob::DataPage::~DataPage -- デストラクタ
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
//	Lob::DataPage::getPrevPage -- 前のページを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Lob::DataPage::PagePointer
//		前のページ。存在しない場合は0
//
//	EXCEPTIONS
//
DataPage::PagePointer
DataPage::getPrevPage()
{
	PagePointer p;
	if (getPrevPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		p = m_cFile.attachDataPage(getPrevPageID());
	}
	return p;
}

//
//	FUNCTION public
//	Lob::DataPage::getNextPage -- 後のページを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Lob::DataPage::PagePointer
//		後のページ。存在しない場合は0
//
//	EXCEPTIONS
//
DataPage::PagePointer
DataPage::getNextPage()
{
	PagePointer p;
	if (getNextPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		p = m_cFile.attachDataPage(getNextPageID());
	}
	return p;
}

//
//	FUNCTION public
//	Lob::DataPage::append -- データを追加する
//
//	NOTES
//
//	ARGUMENTS
//	const char* pBuffer_
//		追加するデータ
//	ModSize uiLength_
//		追加するデータの長さ
//
//	RETURN
//	bool
//		格納できた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
DataPage::append(const char* pBuffer_, ModSize uiLength_)
{
	if (getFreeSize() < uiLength_)
		return false;
	
	dirty();
	char* p = getData();
	ModOsDriver::Memory::copy(p + getLength(), pBuffer_, uiLength_);
	m_pHeader->m_uiLength += uiLength_;
	return true;
}

//
//	FUNCTION public
//	Lob::DataPage::truncate --  データを削除する
//
//	NOTES
//	最後から指定長さ分でデータが削除される
//
//	ARGUMENTS
//	ModSize uiLength_
//		削除する長さ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataPage::truncate(ModSize uiLength_)
{
	dirty();
	m_pHeader->m_uiLength -= uiLength_;
}

//
//	FUNCTION public
//	Lob::DataPage::replace -- データを書き換える
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiPosition_
//		書き換える先頭位置
//	const char* pBuffer_
//		書き換えるデータ
//	ModSize uiLength_
//		書き換えるデータの長さ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataPage::replace(ModSize uiPosition_, const char* pBuffer_, ModSize uiLength_)
{
	dirty();
	char* p = getData();
	ModOsDriver::Memory::copy(p + uiPosition_, pBuffer_, uiLength_);
}

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
