// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreePage.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_BTREEPAGE_H
#define __SYDNEY_FULLTEXT2_BTREEPAGE_H

#include "FullText2/Module.h"
#include "FullText2/Page.h"
#include "FullText2/AutoPointer.h"
#include "FullText2/PagePointer.h"
#include "PhysicalFile/Page.h"

#include "ModUnicodeCharTrait.h"
#include "ModOsDriver.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class BtreeFile;

//
//	CLASS
//	FullText2::BtreePage --
//
//	NOTES
//
class BtreePage : public Page
{
public:
	//
	//	STRUCT
	//	FullText2::BtreePage::Header
	//
	struct Header
	{
		PhysicalFile::PageID	m_uiPrevPageID;		// 前方のページID
		PhysicalFile::PageID	m_uiNextPageID;		// 後方のページID
		ModUInt32				m_uiCount;			// ページ内のエントリ数
										// 最上位ビットはNodeかLeafかのフラグ
	};

	//
	//	STRUCT
	//	FullText2::BtreePage::Entry
	//
	struct Entry
	{
		// コンストラクタ(1)
		Entry() {}
		// コンストラクタ(2)
		Entry(const ModUnicodeChar* pszKey_, ModUInt32 uiValue_)
			: m_uiValue(uiValue_)
		{
			m_usLength = static_cast<unsigned short>(
				ModUnicodeCharTrait::length(pszKey_));
			ModOsDriver::Memory::copy(m_pszKey, pszKey_, m_usLength);
		}

		ModUInt32			m_uiValue;		// 下位ページのページID or バリュー
		unsigned short		m_usLength;		// キー値の文字数
		ModUnicodeChar		m_pszKey[1];	// キー文字列

		// 文字列の比較を行う
		static int compare(const Entry& e1, const Entry& e2)
		{
			// まずは小さい方にあわせて比較する
			ModSize length = (e1.m_usLength < e2.m_usLength) ?
				e1.m_usLength : e2.m_usLength;
			int comp = 0;
			if (length)
				comp = ModUnicodeCharTrait::compare(e1.m_pszKey,
													e2.m_pszKey, length);
			// 同じだったら長さをチェック
			// [NOTE] 比較方法は、NO PADで固定。
			//  LeafPage::Area::Less も参照。
			if (comp == 0 && e1.m_usLength != e2.m_usLength)
				comp = (e1.m_usLength < e2.m_usLength) ? -1 : 1;
			return comp;
		}
	};

	//
	//	CLASS
	//	FullText2::BtreePage::Less
	//
	class Less
	{
	public:
		// 比較関数
		ModBoolean operator()(const Entry* pEntry1, const Entry* pEntry2) const
		{
			return (Entry::compare(*pEntry1, *pEntry2) < 0) ?
				ModTrue : ModFalse;
		}
	};

	//
	//	TYPEDEF
	//	FullText2::BtreePage::Iterator
	//
	typedef ModVector<Entry*>::Iterator Iterator;

	//
	//	TYPDEF
	//	FullText2::BtreePage::ConstIterator
	//
	typedef ModVector<Entry*>::ConstIterator ConstIterator;

	//
	//	TYPEDEF
	//	FullText2::BtreePage::AutoEntry
	//
	typedef AutoPointer<Entry> AutoEntry;

	//
	//	TYPEDEF
	//	FullText2::BtreePage::PagePointer
	//
	typedef PageObjectPointer<BtreePage> PagePointer;

	// コンストラクタ(1)
	BtreePage(BtreeFile& cFile_, PhysicalFile::Page* pPage_);
	// コンストラクタ(2)
	BtreePage(BtreeFile& cFile_, PhysicalFile::Page* pPage_,
			  PhysicalFile::PageID uiPrevPageID_,
			  PhysicalFile::PageID uiNextPageID_);
	// デストラクタ
	virtual ~BtreePage();

	// リセット(1)
	void reset(PhysicalFile::Page* pPage_);
	// リセット(2)
	void reset(PhysicalFile::Page* pPage_,
			   PhysicalFile::PageID uiPrevPageID_,
			   PhysicalFile::PageID uiNextPageID_);

	// キー以下で最大のものを検索する
	bool search(const Entry& cKey_, ModUInt32& uiValue_);
	// 同じキーのエントリを検索する
	bool find(const Entry& cKey_, ModUInt32& uiValue_);

	// 挿入する
	bool insert(const Entry& cKey_);
	void insert(ConstIterator first_, ConstIterator last_);
	// 削除する
	bool expunge(const Entry& cKey_);
	void expunge(Iterator first_, Iterator last_);
	// 更新する
	bool update(const Entry& cKey1_, const Entry& cKey2_);

	// 先頭のエントリを得る
	Iterator begin();
	ConstIterator begin() const;
	// 終端のエントリを得る
	Iterator end();
	ConstIterator end() const;

	// リーフか
	bool isLeaf() const;
	// リーフにする
	void setLeaf();

	// ルートか
	bool isRoot() const
	{
		return getPrevPageID() == PhysicalFile::ConstValue::UndefinedPageID
			&& getNextPageID() == PhysicalFile::ConstValue::UndefinedPageID;
	}

	// 内容をリセットする
	void reset(PhysicalFile::PageID uiPrevPageID_,
			   PhysicalFile::PageID uiNextPageID_);

	// 前方のページIDを得る
	PhysicalFile::PageID getPrevPageID() const
		{ return m_pHeader->m_uiPrevPageID; }
	// 後方のページIDを得る
	PhysicalFile::PageID getNextPageID() const
		{ return m_pHeader->m_uiNextPageID; }

	// 前方のページIDを設定する
	void setPrevPageID(PhysicalFile::PageID uiPrevPageID_);
	// 後方のページIDを設定する
	void setNextPageID(PhysicalFile::PageID uiNextPageID_);

	// エントリ数を得る
	int getCount() const;

	// エントリーのユニット長を計算する
	static ModSize calcUnitLength(const Entry& cEntry_)
		{ return calcUnitLength(cEntry_.m_usLength); }
	static ModSize calcUnitLength(unsigned short usLength_)
	{
		// ページIDと文字列長分を足して、4バイトバウンダリにする
		return (static_cast<ModSize>(usLength_) + 2) / 2 + 1;
	}

	// 使用ユニット数を得る
	ModSize getUsedUnitSize() const;

	// 整合性検査を行う
	void verify();

	// 何段目のページかを設定する
	void setStepCount(ModSize uiStepCount_) { m_uiStepCount = uiStepCount_; }
	// 何段目のページか
	ModSize getStepCount() const { return m_uiStepCount; }

	// エントリを作成する
	static Entry* allocateEntry(const ModUnicodeChar* pszKey_,
								ModUInt32 uiValue_ = -1);
	// エントリーをコピーする
	static Entry* allocateEntry(const Entry& cEntry_);

private:
	// ページを読み込む
	void load();
	// エントリを読み込む
	void loadEntry();

	// 拡張のための操作を行う
	void expand(ModSize unit,
				const Entry* pEntry1_ = 0, const Entry* pEntry2_ = 0);
	// 縮小のための操作を行う
	void reduce();

	// ページ分割を行う
	void split(PagePointer pNextPage_,
			   const Entry* pEntry1_, const Entry* pEntry2_);
	// 再配分を行う
	void redistribute(PagePointer pNextPage_,
					  const Entry* pEntry1_ = 0, const Entry* pEntry2_ = 0);
	// ページ連結を行う
	void concatenate(PagePointer pNextPage_);

	// 親ノードを更新する
	void updateParent(const Entry* pKey1_, const Entry* pKey2_);

	// ファイル
	BtreeFile& m_cFile;

	// ヘッダー
	Header* m_pHeader;

	// エントリ
	ModVector<Entry*> m_vecpEntry;

	// 自分が何段目か
	ModSize m_uiStepCount;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_BTREEPAGE_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
