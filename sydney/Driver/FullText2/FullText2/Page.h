// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Page.h -- PhysicalFile::Pageのラッパークラス
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_PAGE_H
#define __SYDNEY_FULLTEXT2_PAGE_H

#include "FullText2/Module.h"
#include "Common/Object.h"
#include "PhysicalFile/Page.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class IndexFile;

//
//	CLASS
//	FullText2::Page -- PhysicalFile::Pageのラッパークラス
//
//	NOTES
//
class Page : public Common::Object
{
	friend class IndexFile;

public:
	// コンストラクタ
	Page(IndexFile& cFile_, PhysicalFile::Page* pPhysicalPage_);
	// デストラクタ
	virtual ~Page();

	// リセットする
	void reset(PhysicalFile::Page* pPhysicalPage_);

	// ReadOnlyかどうか
	bool isReadOnly() const;

	// Dirtyかどうか
	bool isDirty() const;

	// ページユニットサイズを得る
	virtual ModSize getPageUnitSize() const;
	static ModSize getPageUnitSize(PhysicalFile::Page* pPhysicalPage_);
	// ページの空きユニットサイズを得る
	virtual ModSize getFreeUnitSize() const;
	// 使用ユニット数を得る
	virtual ModSize getUsedUnitSize() const { return 0; }

	// 物理ページIDを得る
	PhysicalFile::PageID getID() const { return m_pPhysicalPage->getID(); }

	// ページをDirtyに設定する
	void dirty() { m_pPhysicalPage->dirty(); }

	// データ領域へのポインタを得る
	virtual ModUInt32* getBuffer() const
	{
		return getBuffer(m_pPhysicalPage);
	}
	static ModUInt32* getBuffer(PhysicalFile::Page* pPhysicalPage_)
	{
		return const_cast<ModUInt32*>(syd_reinterpret_cast<const ModUInt32*>(pPhysicalPage_->operator const void*()));
	}

	// ページの内容を初期化する
	virtual void clear(unsigned char c_);
	static void clear(PhysicalFile::Page* pPage_, unsigned char c_);

	// 物理ページを得る
	PhysicalFile::Page* getPhysicalPage() { return m_pPhysicalPage; }

	// 参照を増やす
	void incrementReference();
	// 参照を減らす
	void decrementReference();

private:
	// デタッチする
	void detach();

	// ファイル
	IndexFile& m_cFile;

	// 物理ファイルのページ
	PhysicalFile::Page* m_pPhysicalPage;

	// フリーリスト用
	Page* m_pNext;

	// フリーされたページかどうか
	bool m_bFree;

	// 参照カウンタ(参照する変数によって増減する)
	int m_iReference;

	// LRU
	Page* _next;
	Page* _prev;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_PAGE_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
