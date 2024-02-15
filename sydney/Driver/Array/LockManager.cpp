// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LockManager.cpp --
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Array";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Array/LockManager.h"

#include "Os/CriticalSection.h"
#include "Os/AutoCriticalSection.h"

#include "ModHashMap.h"
#include <set>

using namespace std;

_SYDNEY_USING
_SYDNEY_ARRAY_USING

namespace
{
	//
	//	TYPEDEF
	//	_$$::_key -- std::set用
	//
	typedef pair<PhysicalFile::PageID, void*> _key;
	
	//
	//	CLASS
	//	_$$::_Hasher -- ModHashMap用
	//
	class _Hasher
	{
	public:
		ModSize operator () (const Lock::FileName& key) const
		{
			return *key;
		}
	};

	//
	//	CLASS
	//	_$$::_Less -- std::set用
	//
	class _Less
	{
	public:
		bool operator () (const _key& c1, const _key& c2) const
		{
			return (c1.first < c2.first) ? true	:
				(c1.first == c2.first && c1.second < c2.second) ? true : false;
		}
	};

	//
	//	TYPEDEF
	//	_$$::_idSet
	//
	typedef set<_key, _Less>	_idSet;
	
	//
	//	TYPEDEF
	//	_$$::_hashMap
	//
	typedef ModHashMap<Lock::FileName, _idSet, _Hasher>	_hashMap;

	//
	//	VARIABLE local
	//	_$$::_lockMap -- すべてのロック情報を格納しているマップ
	//
	_hashMap _lockMap;

	//
	//	VARIABLE local
	//	_$$::_latch -- ロック情報を排他するためのクリティカルセクション
	//
	Os::CriticalSection _latch;
}

//
//	FUNCTION public static
//	Array::LockManager::insert -- ロック情報を挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const Lock::FileName& cFileName_
//		ファイル名
//	PhysicalFile::PageID uiPageID_
//		ロックするページID
//	void* pObject_
//		ロックを発行するオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LockManager::insert(const Lock::FileName& cFileName_,
					PhysicalFile::PageID uiPageID_,
					void* pObject_)
{
	Os::AutoCriticalSection cAuto(_latch);
	ModPair<_hashMap::Iterator, ModBoolean> r
		= _lockMap.insert(cFileName_, _idSet());
	_idSet::value_type v(uiPageID_, pObject_);
	(*r.first).second.insert(v);
}

//
//	FUNCTION public static
//	Array::LockManager::erase -- ロック情報を削除する
//
//	NOTES
//
//	ARGUMENTS
//	const Lock::FileName& cFileName_
//		ファイル名
//	PhysicalFile::PageID uiPageID_
//		ロックするページID
//	void* pObject_
//		ロックを発行するオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LockManager::erase(const Lock::FileName& cFileName_,
				   PhysicalFile::PageID uiPageID_,
				   void* pObject_)
{
	Os::AutoCriticalSection cAuto(_latch);
	_hashMap::Iterator i = _lockMap.find(cFileName_);
	if (i == _lockMap.end())
		return;
	_idSet::value_type v(uiPageID_, pObject_);
	_idSet::iterator j = (*i).second.find(v);
	if (j != (*i).second.end())
	{
		(*i).second.erase(j);
	}
}

//
//	FUNCTION public static
//	Array::LockManager::erase -- ロック情報を削除する
//
//	NOTES
//
//	ARGUMENTS
//	const Lock::FileName& cFileName_
//		ファイル名
//	const ModVector<PhysicalFile::PageID>& vecPageID_
//		ロックを解除するページIDの配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LockManager::erase(const Lock::FileName& cFileName_,
				   const ModVector<PhysicalFile::PageID>& vecPageID_)
{
	Os::AutoCriticalSection cAuto(_latch);
	_hashMap::Iterator i = _lockMap.find(cFileName_);
	if (i == _lockMap.end())
		return;
	ModVector<PhysicalFile::PageID>::ConstIterator k = vecPageID_.begin();
	for (; k != vecPageID_.end(); ++k)
	{
		_idSet::value_type v(*k, syd_reinterpret_cast<void*>(0));
		_idSet::iterator j = (*i).second.lower_bound(v);
		_idSet::iterator e = j;
		while (e != (*i).second.end())
		{
			if ((*e).first != *k)
				break;
			++e;
		}
		if (j != e)
		{
			(*i).second.erase(j, e);
		}
	}
}

//
//	FUNCTION public static
//	Array::LockManager::check -- ロック情報があるかチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const Lock::FileName& cFileName_
//		ファイル名
//	PhysicalFile::PageID uiPageID_
//		ロックするページID
//	void* pObject_
//		ロックを発行するオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
bool
LockManager::check(const Lock::FileName& cFileName_,
				   PhysicalFile::PageID uiPageID_,
				   void* pObject_)
{
	Os::AutoCriticalSection cAuto(_latch);
	_hashMap::Iterator i  = _lockMap.find(cFileName_);
	if (i == _lockMap.end())
		return false;
	_idSet::value_type v(uiPageID_, pObject_);
	_idSet::iterator j = (*i).second.find(v);
	return (j != (*i).second.end()) ? true : false;
}

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
