// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Page.h -- PhysicalFile::Pageのラッパークラス
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

#ifndef __SYDNEY_LOB_PAGE_H
#define __SYDNEY_LOB_PAGE_H

#include "Lob/Module.h"
#include "Common/Object.h"
#include "PhysicalFile/Page.h"

_SYDNEY_BEGIN
_SYDNEY_LOB_BEGIN

class File;

//
//	CLASS
//	Lob::Page -- PhysicalFile::Pageのラッパークラス
//
//	NOTES
//
class Page : public Common::Object
{
	friend class File;

public:
	//
	//	ENUM
	//	Lob::Page::Type -- ページタイプ
	//
	struct Type
	{
		enum Value
		{
			Top = 1,
			Node,
			Dir,
			Data
		};
	};
	
	// コンストラクタ
	Page(File& cFile_, PhysicalFile::Page* pPhysicalPage_);
	// デストラクタ
	virtual ~Page();

	// ReadOnlyかどうか
	bool isReadOnly() const;
	// Dirtyかどうか
	bool isDirty() const;

	// ページサイズを得る
	ModSize getPageSize();
	// 空きサイズを得る
	ModSize getFreeSize();
	// 使用サイズを得る
	virtual ModSize getUsedSize() = 0;

	// 物理ページIDを得る
	PhysicalFile::PageID getID() const { return m_pPhysicalPage->getID(); }

	// ページをDirtyに設定する
	void dirty() { m_pPhysicalPage->dirty(); }

	// データ領域へのポインタを得る
	virtual char* getBuffer() const
	{
		// タイプの次のアドレスを返す
		return 	const_cast<char*>(
						m_pPhysicalPage->operator const char*()) + sizeof(int);
	}

	// ページの内容を初期化する
	virtual void clear(unsigned char c_);

	// 物理ページを得る
	PhysicalFile::Page* getPhysicalPage() { return m_pPhysicalPage; }

	// 参照を増やす
	void incrementReference() { m_iReference++; }
	// 参照を減らす
	int decrementReference() { return --m_iReference; }

	// デタッチする
	void detach();

	// タイプを得る
	Type::Value getType() const
	{
		return getType(m_pPhysicalPage);
	}
	static Type::Value getType(const PhysicalFile::Page* pPage_)
	{
		return static_cast<Type::Value>(
						*syd_reinterpret_cast<const int*>(
								pPage_->operator const char*()));
	}

	// タイプを設定する
	void setType(Type::Value eType_)
	{
		setType(m_pPhysicalPage, eType_);
	}
	static void setType(PhysicalFile::Page* pPage_, Type::Value eType_)
	{
		(*syd_reinterpret_cast<int*>(pPage_->operator char*())) = eType_;
	}

	// ページサイズを得る
	static ModSize getPageSize(Os::Memory::Size uiPageSize_);

protected:
	// ファイル
	File& m_cFile;

private:
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

_SYDNEY_LOB_END
_SYDNEY_END

#endif //__SYDNEY_LOB_PAGE_H

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
