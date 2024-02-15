// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HeaderPage.cpp --
// 
// Copyright (c) 2003, 2004, 2023 Ricoh Company, Ltd.
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
#include "Btree2/HeaderPage.h"
#include "Btree2/BtreeFile.h"

#include "Common/DateTimeData.h"
#include "Os/Memory.h"

#include "ModTime.h"

_SYDNEY_USING
_SYDNEY_BTREE2_USING

//
//	FUNCTION public
//	Btree2::HeaderPage::HeaderPage -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
// 	Btree2::BtreeFile& cFile_
//		ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
HeaderPage::HeaderPage(BtreeFile& cFile_)
	: Page(cFile_)
{
}

//
//	FUNCTION public
//	Btree2::HeaderPage::~HeaderPage -- デストラクタ
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
HeaderPage::~HeaderPage()
{
}

//
//	FUNCTION public
//	Btree2::HeaderPage::setPhysicalPage -- 物理ページを設定する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPage_
//		物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
HeaderPage::setPhysicalPage(PhysicalFile::Page* pPage_)
{
	Page::setPhysicalPage(pPage_);
	Os::Memory::copy(&m_cHeader, getBuffer(), sizeof(Header));
}

//
//	FUNCTION public
//	Btree2::HeaderPage::initialize -- 初期化する
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
HeaderPage::initialize()
{
	Os::Memory::reset(&m_cHeader, sizeof(Header));
	setRootPageID(PhysicalFile::ConstValue::UndefinedPageID);
	setLeftLeafPageID(PhysicalFile::ConstValue::UndefinedPageID);
	setRightLeafPageID(PhysicalFile::ConstValue::UndefinedPageID);
	setLastModificationTime();
}

//
//	FUNCTION public
//	Btree2::HeaderPage::incrementCount -- エントリ数を増やす
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
HeaderPage::incrementCount()
{
	m_cHeader.m_uiCount++;
	m_cHeader.m_uiInsertCount++;
	setLastModificationTime();
}

//
//	FUNCTION public
//	Btree2::HeaderPage::decrementCount -- エントリ数を減らす
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
HeaderPage::decrementCount()
{
	m_cHeader.m_uiCount--;
	setLastModificationTime();
}

//
//	FUNCTION public
//	Btree2::HeaderPage::getSplitRatio
//		-- ページ分割時の前ページに残すエントリの割合(%)を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		割合(%)
//
//	EXCEPTIONS
//
int
HeaderPage::getSplitRatio() const
{
	double maxCount = m_cHeader.m_uiMaxValueCount;
	double total = m_cHeader.m_uiInsertCount;

	int ratio = 100;
	
	if (m_cHeader.m_uiInsertCount == 0
		|| (maxCount / total) < 0.4)
		ratio =  50;
	else if ((maxCount / total) < 0.8)
		ratio = 90;

	return ratio;
}

//
//	FUNCTION public
//	Btree2::HeaderPage::preFlush -- 確定前処理を行う
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
HeaderPage::preFlush()
{
	Os::Memory::copy(getBuffer(), &m_cHeader, sizeof(Header));
}

//
//	FUNCTION private
//	Btree2::HeaderPage::setLastModificationTime -- 最終更新日時を設定する
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
HeaderPage::setLastModificationTime()
{
#ifdef OBSOLETE
	ModTime cutTime = ModTime::getCurrentTime();
	Common::DateTimeData cTimeData(curTime.getYear(),
								   cutTime.getMonth(),
								   curTime.getDay(),
								   curTime.getHour(),
								   curTime.getMinute(),
								   curTime.getSecond(),
								   curTime.getMilliSecond());
	cTimeData.dumpValue(m_cHeader.m_pTimeStamp);
#endif
}

//
//	Copyright (c) 2003, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
