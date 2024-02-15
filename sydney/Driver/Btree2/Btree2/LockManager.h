// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LockManager.h -- 
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

#ifndef __SYDNEY_BTREE2_LOCKMANAGER_H
#define __SYDNEY_BTREE2_LOCKMANAGER_H

#include "Btree2/Module.h"
#include "Lock/Name.h"
#include "PhysicalFile/Page.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_BTREE2_BEGIN

//
//	CLASS
//	Btree2::LockManager -- ページロックを管理する
//
//	NOTES
//
class LockManager
{
public:
	// ロック情報を挿入する
	static void insert(const Lock::FileName& cFileName_,
					   PhysicalFile::PageID uiPageID_,
					   void* pObject_);

	// ロック情報を削除する
	static void erase(const Lock::FileName& cFileName_,
					  PhysicalFile::PageID uiPageID_,
					  void* pObject_);

	// ロック情報を削除する
	static void erase(const Lock::FileName& cFileName_,
					  const ModVector<PhysicalFile::PageID>& vecPageID_);

	// ロック情報があるかチェックする
	static bool check(const Lock::FileName& cFileName_,
					  PhysicalFile::PageID uiPageID_,
					  void* pObject_);
};

_SYDNEY_BTREE2_END
_SYDNEY_END

#endif //__SYDNEY_BTREE2_LOCKMANAGER_H

//
//	Copyright (c) 2003, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
