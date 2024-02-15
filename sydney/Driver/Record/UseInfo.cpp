// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UseInfo.cpp --
//		レコードファイル内で使用している物理ページおよび物理エリアの
//		各識別子を登録するための情報クラスの実現ファイル
// 
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Record";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Record/UseInfo.h"

_SYDNEY_USING

using namespace Record;

/////////////////////////////////////////////////////////////////////////////
//
//	PUBLIC METHOD
//
/////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION public
//	Record::UseInfo::UseInfo -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
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
UseInfo::UseInfo()
	: m_Table(),
	  m_LastPageID(PhysicalFile::ConstValue::UndefinedPageID),
	  m_AreaIDsMaxCount(0)
{
}

//
//	FUNCTION public
//	Record::UseInfo::~UseInfo -- デストラクタ
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
UseInfo::~UseInfo()
{
}

//
//	FUNCTION public
//	Record::UseInfo::append --
//		レコードファイルで使用している物理ページと物理エリアの
//		識別子を追加登録する
//
//	NOTES
//	レコードファイルで使用している物理ページと物理エリアの
//	識別子を追加登録する
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		追加登録する物理ページ識別子
//	const PhysicalFile::AreaID	AreaID_
//		投下登録する物理エリア識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
UseInfo::append(const PhysicalFile::PageID	PageID_,
				const PhysicalFile::AreaID	AreaID_)
{
	Table::Iterator	tableIterator = this->m_Table.find(PageID_);

	Table::Iterator	tableEnd = this->m_Table.end();

	if (tableIterator != tableEnd)
	{
		// 既に物理ページ識別子は登録されている…

		//
		// 物理エリア識別子ベクターの先頭にUndefinedが
		// 登録されている場合、その物理エリア識別子は抹消し、
		// 指定された物理エリア識別子を登録し直す。
		// 『物理ページ内で、1つも物理エリアが使用されていない場合、
		// 　唯一の物理エリア識別子としてUndefinedを登録しておく』
		// という仕様のため。
		//

		AreaIDs	areaIDs = (*tableIterator).second;
		AreaIDs::Iterator	topAreaID = areaIDs.begin();

		if (*topAreaID == PhysicalFile::ConstValue::UndefinedAreaID)
		{
			(*tableIterator).second.popFront();
		}

		(*tableIterator).second.pushBack(AreaID_);

		ModSize	areaIDsCount = (*tableIterator).second.getSize();

		if (areaIDsCount > this->m_AreaIDsMaxCount)
		{
			this->m_AreaIDsMaxCount =
				static_cast<PhysicalFile::AreaNum>(areaIDsCount);
		}
	}
	else
	{
		// 初回登録の物理ページ識別子…

		if (this->m_LastPageID == PhysicalFile::ConstValue::UndefinedPageID ||
			PageID_ > this->m_LastPageID)
		{
			this->m_LastPageID = PageID_;
		}

		AreaIDs	areaIDs;

		areaIDs.pushBack(AreaID_);

		this->m_Table.insert(PageID_, areaIDs);

		if (this->m_AreaIDsMaxCount == 0)
		{
			this->m_AreaIDsMaxCount++;
		}
	}
}

//
//	FUNCTION public
//	Record::UseInfo::getAreaIDs --
//		物理エリア識別子ベクターへの参照を返す
//
//	NOTES
//	引数が示す物理ページ内で使用中の物理エリアの識別子が
//	登録されている物理エリア識別ベクターへの参照を返す。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//
//	RETURN
//	const UseInfo::AreaIDs&
//		物理エリア識別子ベクターへの参照
//
//	EXCEPTIONS
//	なし
//
const UseInfo::AreaIDs&
UseInfo::getAreaIDs(const PhysicalFile::PageID	PageID_) const
{
	Table::ConstIterator	tableIterator = this->m_Table.find(PageID_);

	return (*tableIterator).second;
}

#ifdef OBSOLETE
//
//	FUNCTION public
//	Record::UseInfo::getLastAreaID --
//		物理ページ内での最終物理エリアの識別子を返す
//
//	NOTES
//	引数が示す物理ページ内での最終物理エリアの識別子を返す。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//
//	RETURN
//	PhysicalFile::AreaID
//		物理ページ内での最終物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
PhysicalFile::AreaID
UseInfo::getLastAreaID(const PhysicalFile::PageID	PageID_) const
{
	Table::ConstIterator	tableIterator = this->m_Table.find(PageID_);

	PhysicalFile::AreaID	lastAreaID =
		PhysicalFile::ConstValue::UndefinedAreaID;

	if ((*tableIterator).second.isEmpty() == ModFalse)
	{
		lastAreaID = 0;

		AreaIDs::ConstIterator	areaID = (*tableIterator).second.begin();
		AreaIDs::ConstIterator	areaIDsEnd = (*tableIterator).second.end();

		while (areaID != areaIDsEnd)
		{
			if (lastAreaID < *areaID)
			{
				lastAreaID = *areaID;
			}

			areaID++;
		}
	}

	return lastAreaID;
}
#endif //OBSOLETE

/////////////////////////////////////////////////////////////////////////////
//
//	PRIVATE METHOD
//
/////////////////////////////////////////////////////////////////////////////

//
//	Copyright (c) 2001, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
