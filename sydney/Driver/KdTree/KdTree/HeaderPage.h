// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HeaderPage.h --
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

#ifndef __SYDNEY_KDTREE_HEADERPAGE_H
#define __SYDNEY_KDTREE_HEADERPAGE_H

#include "KdTree/Module.h"
#include "KdTree/Page.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

class BtreeFile;

//
//	CLASS
//	KdTree::HeaderPage --
//
//	NOTES
//	
//
class HeaderPage : public Page
{
public:
	//
	//	STRUCT
	//	KdTree::HeaderPage::Header
	//
	struct Header
	{
		ModInt32				m_iCount;			// エントリ数
		
		PhysicalFile::PageID	m_uiRootPageID;		// ルートページ
		PhysicalFile::PageID	m_uiLeftPageID;		// 左端のリーフページ
		PhysicalFile::PageID	m_uiRightPageID;	// 右端のリーフページ
	};

	// コンストラクタ
	HeaderPage(BtreeFile& cFile_, PhysicalFile::Page* pPage_);
	// デストラクタ
	virtual ~HeaderPage();

	// 初期化する
	void initialize();

	// エントリ数を得る
	int getCount() const { return m_pHeader->m_iCount; }
	// エントリ数を増やす
	void addCount();
	// エントリ数を減らす
	void delCount();

	// ルートページを得る
	PhysicalFile::PageID getRootPageID()
		{ return m_pHeader->m_uiRootPageID; }
	// 左端のリーフページを得る
	PhysicalFile::PageID getLeftPageID()
		{ return m_pHeader->m_uiLeftPageID; }
	// 右端のリーフページを得る
	PhysicalFile::PageID getRightPageID()
		{ return m_pHeader->m_uiRightPageID; }

	// ルートページを設定する
	void setRootPageID(PhysicalFile::PageID id_);
	// 左端のリーフページを設定する
	void setLeftPageID(PhysicalFile::PageID id_);
	// 右端のリーフページを設定する
	void setRightPageID(PhysicalFile::PageID id_);

private:
	// ヘッダー
	Header* m_pHeader;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_HEADERPAGE_H

//
//	Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
