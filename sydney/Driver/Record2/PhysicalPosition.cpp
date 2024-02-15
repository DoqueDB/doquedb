// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PhysicalPosition.cpp -- 位置クラス
// 
// Copyright (c) 2000, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Record2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Record2/PhysicalPosition.h"
#include "PhysicalFile/Page.h"
#include "Common/ObjectIDData.h"

_SYDNEY_USING

_SYDNEY_RECORD2_USING

namespace
{
	const int _iAreaIDBits = sizeof(Common::ObjectIDData::LatterType) * 8;
	const Utility::ObjectID _ullAreaIDMask = ~((~static_cast<Utility::ObjectID>(0)) << _iAreaIDBits);
}

#ifndef SYD_COVERAGE
//
//	FUNCTION public
//	Record2::PhysicalPosition::PhysicalPosition -- デフォルトコンストラクタ
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
//	Record2::PhysicalPosition::PhysicalPosition -- コンストラクタ
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
PhysicalPosition::PhysicalPosition(const Utility::ObjectID	iObjectID_)
	: m_PageID(static_cast<PhysicalFile::PageID>(iObjectID_ >> _iAreaIDBits)),
	  m_AreaID(
		static_cast<PhysicalFile::AreaID>(iObjectID_ & _ullAreaIDMask))
{
	; // do nothing
}

//
//	FUNCTION public
//	Record2::PhysicalPosition::PhysicalPosition -- コンストラクタ
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
PhysicalPosition::PhysicalPosition(const PhysicalFile::PageID	uiPageID_,
								   const PhysicalFile::AreaID	uiAreaID_)
	: m_PageID(uiPageID_),
	  m_AreaID(uiAreaID_)
{
	; // do nothing
}

//
//	FUNCTION public
//	Record2::PhysicalPosition::~PhysicalPosition -- デストラクタ
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
//	Record2::PhysicalPosition::getPageID -- 位置情報の物理ページID部分を返す
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
//	Record2::PhysicalPosition::getAreaID -- 位置情報のエリアID部分を返す
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
//	Record2::PhysicalPosition::getObjectID --  位置情報をオブジェクトIDに変換して返す
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
Utility::ObjectID
PhysicalPosition::getObjectID() const
{
	return getObjectID(m_PageID, m_AreaID);
}

//
//	FUNCTION public
//	Record2::PhysicalPosition::getObjectID --  位置情報をオブジェクトIDに変換して返す
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
Utility::ObjectID
PhysicalPosition::getObjectID(PhysicalFile::PageID uiPageID_,
							  PhysicalFile::AreaID uiAreaID_)
{
	return (static_cast<Utility::ObjectID>(uiPageID_) << _iAreaIDBits)
		| static_cast<Utility::ObjectID>(uiAreaID_);
}

//
//	FUNCTION public static
//	Record2::PhysicalPosition::readObjectID
//		-- メモリーからオブジェクトIDに変換して返す
//		   Common::ObjectIDDataを使うと遅いので実装した
//
Utility::ObjectID
PhysicalPosition::readObjectID(const char* pBuf_)
{
	Common::ObjectIDData::FormerType Former;
	Os::Memory::copy(&Former, pBuf_, sizeof(Former));
	Common::ObjectIDData::LatterType Latter;
	Os::Memory::copy(&Latter, pBuf_+sizeof(Former), sizeof(Latter));
	return (static_cast<Utility::ObjectID>(Former) << _iAreaIDBits)
		| static_cast<Utility::ObjectID>(Latter);
}

//
//	FUNCTION public static
//	Record2::PhysicalPosition::writeObjectID
//		-- オブジェクトIDをメモリーに書き込む
//		   Common::ObjectIDDataを使うと遅いので実装した
//
void
PhysicalPosition::writeObjectID(char* pBuf_, Utility::ObjectID iObjectID_)
{
	Common::ObjectIDData::FormerType Former
		= static_cast<Common::ObjectIDData::FormerType>(iObjectID_ >> _iAreaIDBits);
	Os::Memory::copy(pBuf_, &Former, sizeof(Former));
	Common::ObjectIDData::LatterType Latter
		= static_cast<Common::ObjectIDData::LatterType>(iObjectID_ & _ullAreaIDMask);
	Os::Memory::copy(pBuf_+sizeof(Former), &Latter, sizeof(Latter));
}

//
// マニピュレータ
//

#ifndef SYD_COVERAGE
// 物理ページIDを変更
void
PhysicalPosition::setPageID(const PhysicalFile::PageID	uiPageID_)
{
	this->m_PageID = uiPageID_;
}

// エリアIDを変更
void
PhysicalPosition::setAreaID(const PhysicalFile::AreaID	uiAreaID_)
{
	this->m_AreaID = uiAreaID_;
}

//
// 比較演算子
//

bool
PhysicalPosition::operator==(
	const PhysicalPosition&	cPhysicalPosition_) const
{
	return
		(this->m_PageID == cPhysicalPosition_.m_PageID &&
		 this->m_AreaID == cPhysicalPosition_.m_AreaID);
}

bool
PhysicalPosition::operator!=(
	const PhysicalPosition&	cPhysicalPosition_) const
{
	return (*this == cPhysicalPosition_) == false;
}

bool
PhysicalPosition::operator<(
	const PhysicalPosition&	cPhysicalPosition_) const
{
	if (this->m_PageID < cPhysicalPosition_.m_PageID)
	{
		return true;
	}
	else if (this->m_PageID == cPhysicalPosition_.m_PageID &&
			 this->m_AreaID < cPhysicalPosition_.m_AreaID)
	{
		return true;
	}
	return false;
}

bool
PhysicalPosition::operator<=(
	const PhysicalPosition&	cPhysicalPosition_) const
{
	return (*this < cPhysicalPosition_ || *this == cPhysicalPosition_);
}

bool
PhysicalPosition::operator>(
	const PhysicalPosition&	cPhysicalPosition_) const
{
	if (this->m_PageID > cPhysicalPosition_.m_PageID)
	{
		return true;
	}
	else if (this->m_PageID == cPhysicalPosition_.m_PageID &&
			 this->m_AreaID > cPhysicalPosition_.m_AreaID)
	{
		return true;
	}
	return false;
}

bool
PhysicalPosition::operator>=(
	const PhysicalPosition&	cPhysicalPosition_) const
{
	return (*this > cPhysicalPosition_ || *this == cPhysicalPosition_);
}
#endif //SYD_COVERAGE

//
//	Copyright (c) 2000, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
