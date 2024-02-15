// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MergeReserve.h --
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_MERGERESERVE_H
#define __SYDNEY_KDTREE_MERGERESERVE_H

#include "KdTree/Module.h"

#include "Lock/Name.h"

#include "ModTime.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

//
//	CLASS
//	KdTree::MergeReserve -- マージ処理へのエントリを管理する
//
//	NOTES
//
class MergeReserve
{
public:
	// タイプ
	struct Type
	{
		enum Value
		{
			Unknown,
			
			Merge,			// マージ
			Discard			// 版消去
		};
	};

	//
	//	STRUCT
	//	MergeReserve::Entry -- 格納するデータの型
	//
	struct Entry
	{
		Entry()
			: m_iType(MergeReserve::Type::Unknown),
			  m_pBucketPrev(0), m_pBucketNext(0),
			  m_pListPrev(0), m_pListNext(0) {}
		Entry(const Lock::FileName& cFileName_, int iType_)
			: m_cFileName(cFileName_), m_iType(iType_),
			  m_cTime(ModTime::getCurrentTime()),
			  m_pBucketPrev(0), m_pBucketNext(0),
			  m_pListPrev(0), m_pListNext(0) {}
		
		Lock::FileName	m_cFileName;
		int				m_iType;
		ModTime			m_cTime;

		Entry*			m_pBucketPrev;
		Entry*			m_pBucketNext;
		
		Entry*			m_pListPrev;
		Entry*			m_pListNext;
	};

	// 末尾にエントリを追加する
	static bool pushBack(const Lock::FileName& cFileName_, int iType_);
	// 先頭のエントリを得る
	static Entry* getFront();

	// エントリを削除する
	static void erase(Entry* pEntry_);
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_MERGERESERVE_H

//
//	Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
