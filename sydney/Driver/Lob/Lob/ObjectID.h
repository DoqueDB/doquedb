// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectID.h --
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

#ifndef __SYDNEY_LOB_OBJECTID_H
#define __SYDNEY_LOB_OBJECTID_H

#include "Lob/Module.h"
#include "PhysicalFile/Page.h"

_SYDNEY_BEGIN

//
//	STRUCT
//	Lob::ObjectID --
//
struct ObjectID
{
	PhysicalFile::PageID	m_uiPageID;
	ModUInt32				m_uiPosition;

	// クリアする
	void clear() { initialize(); }

	// 初期化する
	void initialize()
	{
		m_uiPageID = PhysicalFile::ConstValue::UndefinedPageID;
		m_uiPosition = 0;
	}

	// 不正な値か？
	bool isInvalid()
	{
		return m_uiPageID == PhysicalFile::ConstValue::UndefinedPageID
			? true : false;
	}

	// 同じか？
	bool operator == (const ObjectID& cOther_)
	{
		return m_uiPageID == cOther_.m_uiPageID
			&& m_uiPosition == cOther_.m_uiPosition;
	}
	// 違うか？
	bool operator != (const ObjectID& cOther_)
	{
		return m_uiPageID != cOther_.m_uiPageID
			|| m_uiPosition != cOther_.m_uiPosition;
	}
};

_SYDNEY_END

#endif // __SYDNEY_LOB_OBJECTID_H

//
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
