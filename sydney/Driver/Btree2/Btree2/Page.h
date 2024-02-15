// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Page.h -- PhysicalFile::Pageのラッパークラス
// 
// Copyright (c) 2003, 2004, 2005, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE2_PAGE_H
#define __SYDNEY_BTREE2_PAGE_H

#include "Btree2/Module.h"
#include "Common/Object.h"
#include "PhysicalFile/Page.h"
#include "Os/CriticalSection.h"

_SYDNEY_BEGIN
_SYDNEY_BTREE2_BEGIN

class File;
class BtreeFile;

//
//	CLASS
//	Btree2::Page -- PhysicalFile::Pageのラッパークラス
//
//	NOTES
//
class Page : public Common::Object
{
	friend class File;
	friend class BtreeFile;

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
	// 空きサイズを得る(UNIT単位)
	virtual ModSize getFreeSize();
	// 使用サイズを得る(UNIT単位)
	virtual ModSize getUsedSize() = 0;

	// 物理ページIDを得る
	PhysicalFile::PageID getID() const { return m_pPhysicalPage->getID(); }

	// ページをDirtyに設定する
	void dirty();
	// flush前処理
	virtual void preFlush() {};

	// ページの内容を初期化する
	virtual void clear(unsigned char c_);

	// 物理ページを得る
	PhysicalFile::Page* getPhysicalPage() { return m_pPhysicalPage; }

	// 参照を増やす
	void incrementReference() { m_iReference++; }
	// 参照を減らす
	int decrementReference() { return --m_iReference; }
	// 参照を得る
	int getReferece() { return m_iReference; }

	// デタッチする
	virtual void detach();

	// 排他制御用のクリティカルセクションを得る
	Os::CriticalSection& getLatch() const;

protected:
	// データ領域へのポインタを得る
	virtual ModUInt32* getBuffer()
	{
		return syd_reinterpret_cast<ModUInt32*>(
					const_cast<char*>(m_pPhysicalPage->operator const char*()));
	}

private:
	// ファイル
	File& m_cFile;

	// 物理ファイルのページ
	PhysicalFile::Page* m_pPhysicalPage;

	// フリーリスト用
	Page* m_pNext;

	// フリーされたページかどうか
	bool m_bFree;

	// アタッチ数(アタッチされるたびに増えるカウンタ)
	int m_iAttachCounter;

	// 参照カウンタ(参照する変数によって増減する)
	int m_iReference;
};

_SYDNEY_BTREE2_END
_SYDNEY_END

#endif //__SYDNEY_BTREE2_PAGE_H

//
//	Copyright (c) 2003, 2004, 2005, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
