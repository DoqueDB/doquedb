// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PhysicalPosition.cpp -- 位置クラス
// 
// Copyright (c) 2000, 2004, 2005, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Record";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Record/PhysicalPosition.h"
#include "PhysicalFile/Page.h"
#include "Common/ObjectIDData.h"

_SYDNEY_USING
using namespace Record;

namespace
{
	const int _iAreaIDBits = sizeof(Common::ObjectIDData::LatterType) * 8;
	const Tools::ObjectID _ullAreaIDMask = ~((~static_cast<Tools::ObjectID>(0)) << _iAreaIDBits);
}

#ifndef SYD_COVERAGE
//
//	FUNCTION public
//	Record::PhysicalPosition::PhysicalPosition -- デフォルトコンストラクタ
//
//	NOTES
//	デフォルトコンストラクタ
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
PhysicalPosition::PhysicalPosition()
	: m_PageID(PhysicalFile::ConstValue::UndefinedPageID),
	  m_AreaID(PhysicalFile::ConstValue::UndefinedAreaID)

{
	; // do nothing
}
#endif //SYD_COVERAGE

//
//	FUNCTION public
//	Record::PhysicalPosition::PhysicalPosition -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
PhysicalPosition::PhysicalPosition(const Tools::ObjectID	ObjectID_)
	: m_PageID(static_cast<PhysicalFile::PageID>(ObjectID_ >> _iAreaIDBits)),
	  m_AreaID(
		static_cast<PhysicalFile::AreaID>(ObjectID_ & _ullAreaIDMask))
{
	; // do nothing
}

//
//	FUNCTION public
//	Record::PhysicalPosition::PhysicalPosition -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
PhysicalPosition::PhysicalPosition(const PhysicalFile::PageID	PageID_,
								   const PhysicalFile::AreaID	AreaID_)
	: m_PageID(PageID_),
	  m_AreaID(AreaID_)
{
	; // do nothing
}

//
//	FUNCTION public
//	Record::PhysicalPosition::~PhysicalPosition -- デストラクタ
//
//	NOTES
//	デストラクタ
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
PhysicalPosition::~PhysicalPosition()
{
	; // do nothing
}

//
// アクセッサ
//

//
//	FUNCTION public
//	Record::PhysicalPosition::getPageID -- 位置情報の物理ページID部分を返す
//
//	NOTES
//	位置情報の物理ページID部分を返す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	
//
//	EXCEPTIONS
//	なし
//
PhysicalFile::PageID
PhysicalPosition::getPageID() const
{
	return this->m_PageID;
}

#ifndef SYD_COVERAGE
//
//	FUNCTION public
//	Record::PhysicalPosition::getAreaID -- 位置情報のエリアID部分を返す
//
//	NOTES
//	位置情報のエリアID部分を返す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	
//
//	EXCEPTIONS
//	なし
//
PhysicalFile::AreaID
PhysicalPosition::getAreaID() const
{
	return this->m_AreaID;
}
#endif //SYD_COVERAGE

//
//	FUNCTION public
//	Record::PhysicalPosition::getObjectID --  位置情報をオブジェクトIDに変換して返す
//
//	NOTES
//	 位置情報をオブジェクトIDに変換して返す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	
//
//	EXCEPTIONS
//	なし
//
Tools::ObjectID
PhysicalPosition::getObjectID() const
{
	return getObjectID(m_PageID, m_AreaID);
}

//
//	FUNCTION public
//	Record::PhysicalPosition::getObjectID --  位置情報をオブジェクトIDに変換して返す
//
//	NOTES
//	 位置情報をオブジェクトIDに変換して返す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	
//
//	EXCEPTIONS
//	なし
//
Tools::ObjectID
PhysicalPosition::getObjectID(PhysicalFile::PageID PageID_,
							  PhysicalFile::AreaID AreaID_)
{
	return (static_cast<Tools::ObjectID>(PageID_) << _iAreaIDBits)
		| static_cast<Tools::ObjectID>(AreaID_);
}

//
//	FUNCTION public static
//	Record::PhysicalPosition::readObjectID
//		-- メモリーからオブジェクトIDに変換して返す
//		   Common::ObjectIDDataを使うと遅いので実装した
//
Tools::ObjectID
PhysicalPosition::readObjectID(const char* p)
{
	Common::ObjectIDData::FormerType Former;
	Os::Memory::copy(&Former, p, sizeof(Former));
	Common::ObjectIDData::LatterType Latter;
	Os::Memory::copy(&Latter, p+sizeof(Former), sizeof(Latter));
	return (static_cast<Tools::ObjectID>(Former) << _iAreaIDBits)
		| static_cast<Tools::ObjectID>(Latter);
}

//
//	FUNCTION public static
//	Record::PhysicalPosition::writeObjectID
//		-- オブジェクトIDをメモリーに書き込む
//		   Common::ObjectIDDataを使うと遅いので実装した
//
void
PhysicalPosition::writeObjectID(char* p, Tools::ObjectID id)
{
	Common::ObjectIDData::FormerType Former
		= static_cast<Common::ObjectIDData::FormerType>(id >> _iAreaIDBits);
	Os::Memory::copy(p, &Former, sizeof(Former));
	Common::ObjectIDData::LatterType Latter
		= static_cast<Common::ObjectIDData::LatterType>(id & _ullAreaIDMask);
	Os::Memory::copy(p+sizeof(Former), &Latter, sizeof(Latter));
}

//
// マニピュレータ
//

#ifndef SYD_COVERAGE
// 物理ページIDを変更
void
PhysicalPosition::setPageID(const PhysicalFile::PageID	PageID_)
{
	this->m_PageID = PageID_;
}

// エリアIDを変更
void
PhysicalPosition::setAreaID(const PhysicalFile::AreaID	AreaID_)
{
	this->m_AreaID = AreaID_;
}

//
// 比較演算子
//

bool
PhysicalPosition::operator==(
	const PhysicalPosition&	PhysicalPosition_) const
{
	return
		(this->m_PageID == PhysicalPosition_.m_PageID &&
		 this->m_AreaID == PhysicalPosition_.m_AreaID);
}

bool
PhysicalPosition::operator!=(
	const PhysicalPosition&	PhysicalPosition_) const
{
	return (*this == PhysicalPosition_) == false;
}

bool
PhysicalPosition::operator<(
	const PhysicalPosition&	PhysicalPosition_) const
{
	if (this->m_PageID < PhysicalPosition_.m_PageID)
	{
		return true;
	}
	else if (this->m_PageID == PhysicalPosition_.m_PageID &&
			 this->m_AreaID < PhysicalPosition_.m_AreaID)
	{
		return true;
	}
	return false;
}

bool
PhysicalPosition::operator<=(
	const PhysicalPosition&	PhysicalPosition_) const
{
	return (*this < PhysicalPosition_ || *this == PhysicalPosition_);
}

bool
PhysicalPosition::operator>(
	const PhysicalPosition&	PhysicalPosition_) const
{
	if (this->m_PageID > PhysicalPosition_.m_PageID)
	{
		return true;
	}
	else if (this->m_PageID == PhysicalPosition_.m_PageID &&
			 this->m_AreaID > PhysicalPosition_.m_AreaID)
	{
		return true;
	}
	return false;
}

bool
PhysicalPosition::operator>=(
	const PhysicalPosition&	PhysicalPosition_) const
{
	return (*this > PhysicalPosition_ || *this == PhysicalPosition_);
}
#endif //SYD_COVERAGE

//
//	Copyright (c) 2000, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
