// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Page.h -- PhysicalFile::Pageのラッパークラス
// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_PAGE_H
#define __SYDNEY_BITMAP_PAGE_H

#include "Bitmap/Module.h"
#include "Common/Object.h"
#include "PhysicalFile/Page.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class File;
class BitmapFile;

//
//	CLASS
//	Bitmap::Page -- PhysicalFile::Pageのラッパークラス
//
//	NOTES
//
class Page : public Common::Object
{
	friend class File;
	friend class BitmapFile;

public:
	// コンストラクタ
	Page(File& cFile_);
	// デストラクタ
	virtual ~Page();

	// 物理ページを設定する
	virtual void setPhysicalPage(PhysicalFile::Page* pPage_);
	// 物理ページがあるかどうか
	bool isAttached() { return (m_pPhysicalPage != 0); }

	// ReadOnlyかどうか
	bool isReadOnly() const;

	// Dirtyかどうか
	bool isDirty() const;

	// ページサイズを得る(UNIT単位)
	virtual ModSize getPageSize();

	// 物理ページIDを得る
	PhysicalFile::PageID getID() const { return m_pPhysicalPage->getID(); }

	// ページをDirtyに設定する
	virtual void dirty();

	// ページの内容を初期化する
	virtual void clear(unsigned char c_);

	// 物理ページを得る
	PhysicalFile::Page* getPhysicalPage() { return m_pPhysicalPage; }

	// フリーフラグを立てる
	void setFree() { m_bFree = true; }

	// デタッチする
	virtual void detach();

protected:
	// データ領域へのポインタを得る
	virtual ModUInt32* getBuffer()
	{
		return syd_reinterpret_cast<ModUInt32*>(
					const_cast<char*>(m_pPhysicalPage->operator const char*()));
	}

	// 変数のクリア
	virtual void clear() { m_bFree = false; }

	// 物理ファイルのページ
	PhysicalFile::Page* m_pPhysicalPage;

	// ファイル
	File& m_cFile;

	// フリーされるものかどうか
	bool m_bFree;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_PAGE_H

//
//	Copyright (c) 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
