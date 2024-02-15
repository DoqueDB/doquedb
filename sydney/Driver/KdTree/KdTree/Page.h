// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Page.h -- 
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

#ifndef __SYDNEY_KDTREE_PAGE_H
#define __SYDNEY_KDTREE_PAGE_H

#include "KdTree/Module.h"
#include "Common/Object.h"
#include "PhysicalFile/Page.h"
#include "Buffer/Page.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

class BtreeFile;

//
//	CLASS
//	KdTree::Page --
//
//	NOTES
//
class Page
{
	friend class BtreeFile;
	
public:
	// コンストラクタ
	Page(BtreeFile& cFile_, PhysicalFile::Page* pPage_);
	// デストラクタ
	virtual ~Page();

	// ReadOnlyかどうか
	bool isReadOnly() const;
	// Dirtyかどうか
	bool isDirty() const;
		
	// freeされているかどうか
	bool isFree() const { return m_bFree; }

	// 利用可能な最大ページサイズを得る
	int getPageDataSize() const;

	// 物理ページIDを得る
	PhysicalFile::PageID getID() const { return m_pPhysicalPage->getID(); }

	// ページをDirtyに設定する
	void dirty();

	// ページの内容を初期化する
	virtual void clear(unsigned char c_);

	// 参照を増やす
	void incrementReference();
	// 参照を減らし、参照数が 0 なら detach する
	void decrementReference();

	// デタッチする
	virtual void detach();

	// 更新モードに変更する
	void updateMode();

	// 利用可能なサイズを得る
	static int getPageDataSize(int iPageSize_);

protected:
	// データ領域へのポインタを得る
	virtual char* getBuffer()
	{
		return const_cast<char*>(m_pPhysicalPage->operator const char*());
	}

	// ロードする
	virtual void load() {}

	// ファイル
	BtreeFile& m_cFile;

private:
	// 物理ファイルのページ
	PhysicalFile::Page* m_pPhysicalPage;

	// 参照カウンタ(参照する変数によって増減する)
	int m_iReference;

	// フリーされたかどうか
	bool m_bFree;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_PAGE_H

//
//	Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
