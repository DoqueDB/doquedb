// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HeaderPage.cpp --
// 
// Copyright (c) 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Bitmap";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Bitmap/HeaderPage.h"
#include "Bitmap/BitmapFile.h"

#include "Common/DateTimeData.h"
#include "Common/ObjectIDData.h"
#include "Common/UnsignedIntegerData.h"
#include "Os/Memory.h"

#include "Exception/BadArgument.h"

#include "ModTime.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

//
//	FUNCTION public
//	Bitmap::HeaderPage::HeaderPage -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
// 	Bitmap::BitmapFile& cFile_
//		ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
HeaderPage::HeaderPage(BitmapFile& cFile_)
	: Page(cFile_)
{
}

//
//	FUNCTION public
//	Bitmap::HeaderPage::~HeaderPage -- デストラクタ
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
//	Bitmap::HeaderPage::setPhysicalPage -- 物理ページを設定する
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
	m_pHeader = syd_reinterpret_cast<Header*>(getBuffer());
}

//
//	FUNCTION public
//	Bitmap::HeaderPage::initialize -- 初期化する
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
	dirty();
	Os::Memory::reset(m_pHeader, sizeof(Header));
	setRootPageID(PhysicalFile::ConstValue::UndefinedPageID);
	setLeftLeafPageID(PhysicalFile::ConstValue::UndefinedPageID);
	setRightLeafPageID(PhysicalFile::ConstValue::UndefinedPageID);
	clearNullID();
	clearAllNullID();
	setLastModificationTime();
}

//
// 	FUNCTION public
//	Bitmap::HeaderPage::getNullID -- null用のIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cID_
//		取得したID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
bool
HeaderPage::getNullID(Common::Data& cID_) const
{
	if (cID_.getType() == Common::DataType::UnsignedInteger)
	{
		Common::UnsignedIntegerData& c
			= _SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData&, cID_);
		c.setValue(m_pHeader->m_uiNullPageID);
	}
	else if (cID_.getType() == Common::DataType::ObjectID)
	{
		Common::ObjectIDData& c
			= _SYDNEY_DYNAMIC_CAST(Common::ObjectIDData&, cID_);
		c.setValue(m_pHeader->m_uiNullPageID,
				   m_pHeader->m_usNullAreaID);
	}
	else
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	
	return (m_pHeader->m_uiNullPageID !=
			PhysicalFile::ConstValue::UndefinedPageID) ? true : false;
}

//
//	FUNCTION public
//	Bitmap::HeaderPage::setNullID -- null用のIDを設定する
//
//	NOTES
//
//	ARUGMENTS
//	const Common::Data& cID_
//		設定するID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
HeaderPage::setNullID(const Common::Data& cID_)
{
	if (cID_.getType() == Common::DataType::UnsignedInteger)
	{
		dirty();
		const Common::UnsignedIntegerData& c
			= _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&, cID_);
		m_pHeader->m_uiNullPageID = c.getValue();
	}
	else if (cID_.getType() == Common::DataType::ObjectID)
	{
		dirty();
		const Common::ObjectIDData& c
			= _SYDNEY_DYNAMIC_CAST(const Common::ObjectIDData&, cID_);
		m_pHeader->m_uiNullPageID = c.getFormerValue();
		m_pHeader->m_usNullAreaID = c.getLatterValue();
	}
	else
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}
}

//
//	FUNCTION public
//	Bitmap::HeaderPage::clearNullID -- null用のIDをクリアする
//
//	NOTES
//
//	ARUGMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
HeaderPage::clearNullID()
{
	dirty();
	m_pHeader->m_uiNullPageID = PhysicalFile::ConstValue::UndefinedPageID;
	m_pHeader->m_usNullAreaID = 0;
}

//
// 	FUNCTION public
//	Bitmap::HeaderPage::getAllNullID -- null用のIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cID_
//		取得したID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
bool
HeaderPage::getAllNullID(Common::Data& cID_) const
{
	bool result = false;
	
	if (cID_.getType() == Common::DataType::UnsignedInteger)
	{
		Common::UnsignedIntegerData& c
			= _SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData&, cID_);
		// 過去との互換性のため
		c.setValue((m_pHeader->m_uiAllNullPageID == 0) ?
				   PhysicalFile::ConstValue::UndefinedPageID :
				   m_pHeader->m_uiAllNullPageID);
		if (c.getValue() != PhysicalFile::ConstValue::UndefinedPageID)
			result = true;
	}
	else if (cID_.getType() == Common::DataType::ObjectID)
	{
		Common::ObjectIDData& c
			= _SYDNEY_DYNAMIC_CAST(Common::ObjectIDData&, cID_);
		c.setValue(m_pHeader->m_uiAllNullPageID,
				   m_pHeader->m_usAllNullAreaID);
		if (m_pHeader->m_uiAllNullPageID !=
			PhysicalFile::ConstValue::UndefinedPageID)
			result = true;
	}
	else
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	return result;
}

//
//	FUNCTION public
//	Bitmap::HeaderPage::setAllNullID -- null用のIDを設定する
//
//	NOTES
//
//	ARUGMENTS
//	const Common::Data& cID_
//		設定するID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
HeaderPage::setAllNullID(const Common::Data& cID_)
{
	if (cID_.getType() == Common::DataType::UnsignedInteger)
	{
		dirty();
		const Common::UnsignedIntegerData& c
			= _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&, cID_);
		m_pHeader->m_uiAllNullPageID = c.getValue();
	}
	else if (cID_.getType() == Common::DataType::ObjectID)
	{
		dirty();
		const Common::ObjectIDData& c
			= _SYDNEY_DYNAMIC_CAST(const Common::ObjectIDData&, cID_);
		m_pHeader->m_uiAllNullPageID = c.getFormerValue();
		m_pHeader->m_usAllNullAreaID = c.getLatterValue();
	}
	else
	{
		_SYDNEY_THROW0(Exception::BadArgument);
	}
}

//
//	FUNCTION public
//	Bitmap::HeaderPage::clearAllNullID -- null用のIDをクリアする
//
//	NOTES
//
//	ARUGMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
HeaderPage::clearAllNullID()
{
	dirty();
	m_pHeader->m_uiAllNullPageID = PhysicalFile::ConstValue::UndefinedPageID;
	m_pHeader->m_usAllNullAreaID = 0;
}

//
//	FUNCTION public
//	Bitmap::HeaderPage::incrementCount -- エントリ数を増やす
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
	dirty();
	m_pHeader->m_uiCount++;
	setLastModificationTime();
}

//
//	FUNCTION public
//	Bitmap::HeaderPage::decrementCount -- エントリ数を減らす
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
	dirty();
	m_pHeader->m_uiCount--;
	setLastModificationTime();
}

//
//	FUNCTION public
//	Bitmap::HeaderPage::getSplitRatio
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
	return 50;
}

//
//	FUNCTION private
//	Bitmap::HeaderPage::setLastModificationTime -- 最終更新日時を設定する
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
//	Copyright (c) 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
