// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LeafPage.h -- リーフページ
// 
// Copyright (c) 2002, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_LEAFPAGE_H
#define __SYDNEY_INVERTED_LEAFPAGE_H

#include "Inverted/Module.h"
#include "Inverted/Page.h"
#include "Inverted/OverflowPage.h"
#include "Inverted/AutoPointer.h"
#include "Inverted/Types.h"
#include "Inverted/PagePointer.h"
#include "ModVector.h"
#include "ModUnicodeCharTrait.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class LeafFile;
class InvertedUnit;

//
//	CLASS
//	Inverted::LeafPage -- リーフファイルをあらわすクラス
//
//	NOTES
//
class LeafPage : public Page
{
	friend class LeafFile;

public:
	//
	//	TYPEDEF
	//	Inverted::LeafPage::PagePointer
	//
	typedef PageObjectPointer<LeafPage> PagePointer;

	//
	//	STRUCT
	//	Inverted::LeafPage::Header -- リーフページのヘッダー
	//
	//	NOTES
	//
	struct Header
	{
		PhysicalFile::PageID	m_uiPrevPageID;		// 前ページのページID
		PhysicalFile::PageID	m_uiNextPageID;		// 次ページのページID
		ModUInt32				m_uiCount;			// エリア個数
	};

	//
	//	STRUCT
	//	Inverted::LeafPage::DirBlock -- DIRブロック
	//
	//	NOTES
	//
	struct DirBlock
	{
		//
		//	CLASS
		//	Inverted::LeafPage::DirBlock::Less -- 比較
		//
		class Less
		{
		public:
			// 比較関数
			ModBoolean operator() (const DirBlock& d1, const DirBlock& d2)
			{
				return (d1.getDocumentID() < d2.getDocumentID()) ? ModTrue : ModFalse;
			}
		};

		// IDページIDを得る
		PhysicalFile::PageID getPageID() const { return m_uiIDPageID; }
		// 最小文書IDを得る
		ModUInt32 getDocumentID() const { return m_uiDocumentID & DocumentIdMask; }

		// 削除フラグを設定する
		void setExpunge() { m_uiDocumentID |= UndefinedDocumentID; }
		// 削除フラグを落とす
		void unsetExpunge() { m_uiDocumentID &= DocumentIdMask; }
		// 削除されているか
		bool isExpunge() { return m_uiDocumentID & UndefinedDocumentID; }

		PhysicalFile::PageID	m_uiIDPageID;		// IDページのページID
		ModUInt32				m_uiDocumentID;		// 最小文書ID
	};

	//
	//	CLASS
	//	Inverted::LeafPage::Area -- ページ中のエリアをあらわすクラス
	//
	//	NOTES
	//
	class Area
	{
	public:
		//
		//	CLASS
		//	Inverted::LeafPage::Area::Less -- Areaの比較を行う。a1 < a2 を求める
		//
		//	NOTES
		//	ModAlgorithmで使用するので、ModBooleanを返す
		//
		class Less
		{
		public:
			// 比較関数
			ModBoolean operator ()(const Area* a1, const Area* a2) const
			{
				return (Area::compare(a1, a2) < 0) ? ModTrue : ModFalse;
			}

		};

		// エリア比較を行う
		static int compare(const Area* a1, const Area* a2)
		{
			return compare(a1->getKey(), a1->getKeyLength(),
						   a2->getKey(), a2->getKeyLength());
		}

		// 文字列比較を行う
		static int compare(const ModUnicodeChar* p1, ModSize length1,
						   const ModUnicodeChar* p2, ModSize length2)
		{
			// まずは小さい方にあわせて比較する
			ModSize length = (length1 < length2) ? length1 : length2;
			int comp = 0;
			if (length)
				comp = ModUnicodeCharTrait::compare(p1, p2, length);
			// 同じだったら長さをチェック
			// [NOTE] 比較方法は、NO PADで固定。
			//  もしPAD SPACEを使うなら、空白文字と長い方の余った文字列とを
			//  順番に比較する必要がある。
			//  BtreePage::Entry も参照。
			if (comp == 0 && length1 != length2)
				comp = (length1 < length2) ? -1 : 1;
			return comp;
		}

		// リスト種別を得る
		ModUInt32 getListType() const { return m_uiHeader & ListType::TypeMask; }
		// リスト種別を設定する
		void setListType(ModUInt32 uiListType_)
		{
			m_uiHeader &= ListType::SizeMask;
			m_uiHeader |= uiListType_;
		}

		// 全体のユニット数を得る
		ModSize getUnitSize() const { return m_uiHeader & ListType::SizeMask; }
		// 全体のユニット数を設定する
		void setUnitSize(ModSize uiUnitSize_)
		{
			m_uiHeader &= ListType::TypeMask;
			m_uiHeader |= uiUnitSize_;
		}

		// 文書数を得る
		ModUInt32 getDocumentCount() const { return m_uiDocumentCount; }
		// 文書数を設定する
		void setDocumentCount(ModUInt32 uiDocumentCount_) { m_uiDocumentCount = uiDocumentCount_; }
		void incrementDocumentCount() { ++m_uiDocumentCount; }
		void decrementDocumentCount() { --m_uiDocumentCount; }

		// 最終文書IDを得る
		ModUInt32 getLastDocumentID() const { return m_uiLastDocumentID; }
		// 最終文書IDを設定する
		void setLastDocumentID(ModUInt32 uiLastDocumentID_) { m_uiLastDocumentID = uiLastDocumentID_; }

		// 最終の文書IDへのビットオフセットを得る
		ModUInt32 getDocumentOffset() const { return m_uiDocumentOffset; }
		// 最終の文書IDへのビットオフセットを設定する
		void setDocumentOffset(ModUInt32 uiDocumentOffset_) { m_uiDocumentOffset = uiDocumentOffset_; }

		// 最終の位置情報へのビットオフセットを得る
		ModUInt32 getLocationOffset() const { return m_uiLocationOffset; }
		// 最終の位置情報へのビットオフセットを設定する
		void setLocationOffset(ModUInt32 uiLocationOffset_) { m_uiLocationOffset = uiLocationOffset_; }

		// 先頭文書IDを得る
		ModUInt32 getFirstDocumentID() const { return m_uiFirstDocumentID; }
		// 先頭文書IDを設定する
		void setFirstDocumentID(ModUInt32 uiFirstDocumentID_) { m_uiFirstDocumentID = uiFirstDocumentID_; }

		// 最終LocページのページIDを得る
		PhysicalFile::PageID getLastLocationPageID() const { return m_uiLocationPageID; }
		// 最終LocページのページIDを設定する
		void setLastLocationPageID(PhysicalFile::PageID uiPageID_) { m_uiLocationPageID = uiPageID_; }

		// キー長を得る
		unsigned short getKeyLength() const { return m_usKeyLength; }

		// キーを得る
		const ModUnicodeChar* getKey() const { return m_pszKey; }

		// データ部の先頭のアドレスを得る
		ModUInt32* getHeadAddress();
		// データ部の終わりのアドレスを得る
		ModUInt32* getTailAddress();
		// データ部のユニット数
		ModSize getDataUnitSize() { return static_cast<ModSize>(getTailAddress() - getHeadAddress()); }

		// IDブロックを得る
		OverflowPage::IDBlock getIDBlock();

		// DIRブロックを得る
		DirBlock* getDirBlock();

		// DIRブロックの個数を得る
		int getDirBlockCount();

		// 内容をコピーする
		void copy(const Area* pSrc_);

		// 内容をクリアする
		void clear();

		// 一時的なエリアを確保する
		static Area* allocateArea(const ModUnicodeChar* pszKey_, ModSize uiDataUnitSize_ = 0);

	private:
		ModUInt32 m_uiHeader;				// ヘッダー
		ModUInt32 m_uiDocumentCount;		// 文書数
		ModUInt32 m_uiLastDocumentID;		// 最終文書ID
		ModUInt32 m_uiDocumentOffset;		// 最終の文書IDへのビットオフセット
		ModUInt32 m_uiLocationOffset;		// 最終の位置情報へのビットオフセット
		union {
		ModUInt32 m_uiFirstDocumentID;		// 先頭文書ID
		ModUInt32 m_uiLocationPageID;		// 最終位置情報ページのページID
		};
		unsigned short m_usKeyLength;		// キー長
		ModUnicodeChar m_pszKey[1];			// キー
	};

	//
	//	TYPEDEF
	//	Inverted::LeafPage::Iterator
	//
	typedef ModVector<Area*>::Iterator Iterator;

	//
	//	TYPEDEF
	//	Inverted::LeafPage::ConstIterator
	//
	typedef ModVector<Area*>::ConstIterator ConstIterator;

	//
	//	TYPEDEF
	//	Inverted::LeafPage::AutoArea
	//
	typedef AutoPointer<Area> AutoArea;

	// コンストラクタ(1)
	LeafPage(LeafFile& cFile_, PhysicalFile::Page* pPhysicalPage_);
	// コンストラクタ(2)
	LeafPage(LeafFile& cFile_, PhysicalFile::Page* pPhysicalPage_,
			 PhysicalFile::PageID uiPrevPageID_,
			 PhysicalFile::PageID uiNextPageID_);
	// デストラクタ
	virtual ~LeafPage();

	// 内容をクリアする
	void reset(PhysicalFile::PageID uiPrevPageID_,
			   PhysicalFile::PageID uiNextPageID_);
	void reset(PhysicalFile::Page* pPhysicalPage_);
	void reset(PhysicalFile::Page* pPhysicalPage_,
			   PhysicalFile::PageID uiPrevPageID_,
			   PhysicalFile::PageID uiNextPageID_);

	// エリア数を得る
	int getCount() { return static_cast<int>(m_pHeader->m_uiCount); }

	// エリアを得る
	Iterator search(const ModUnicodeChar* pszKey_);
	Iterator lowerBound(const ModUnicodeChar* pszKey_);
	Iterator lowerBound(const Area* pArea_);
	Iterator upperBound(const ModUnicodeChar* pszKey_);
	Iterator upperBound(const Area* pArea_);

	// 先頭を得る
	Iterator begin() { return m_vecpArea.begin(); }
	ConstIterator begin() const { return m_vecpArea.begin(); }

	// 終端を得る
	Iterator end() { return m_vecpArea.end() - 1; }
	ConstIterator end() const { return m_vecpArea.end() - 1; }

	// エリアを挿入する
	Iterator insert(InvertedUnit& cInvertedUnit_, const ModUnicodeChar* pszKey_,
					ModSize uiDataUnitSize_);
	Iterator insert(InvertedUnit& cInvertedUnit_, const Area* pArea_);
	void insert(InvertedUnit& cInvertedUnit_, ConstIterator first_, ConstIterator last_);

	// エリアを削除する
	Iterator expunge(InvertedUnit& cInvertedUnit_, Iterator i);
	void expunge(InvertedUnit& cInvertedUnit_, Iterator first_, Iterator last_);

	// エリアサイズを変更する
	bool changeAreaSize(Iterator i, int iDataUnitSize_);

	// 指定したエリアが挿入できるか問い合わせる
	bool isInsertArea(const ModUnicodeChar* pszKey_, ModSize uiDataUnitSize_)
	{
		if (getFreeUnitSize() - calcAreaUnitSize(pszKey_, uiDataUnitSize_) < 0)
			return false;
		return true;
	}

	// 次のリーフページのIDを得る
	PhysicalFile::PageID getNextPageID() const { return m_pHeader->m_uiNextPageID; }
	// 前のリーフページのIDを得る
	PhysicalFile::PageID getPrevPageID() const { return m_pHeader->m_uiPrevPageID; }

	// 次のリーフページのIDを設定する
	void setNextPageID(PhysicalFile::PageID uiNextPageID_);
	// 前のリーフページのIDを設定する
	void setPrevPageID(PhysicalFile::PageID uiPrevPageID_);

	// 使用ユニット数を得る
	ModSize getUsedUnitSize() const;

	// 最大データユニットサイズを得る(1ページに1エリアの場合)
	ModSize getMaxAreaUnitSize() const;

	// ページ分割 (1->2 or 1->3)
	// エリア挿入用
	PagePointer split(InvertedUnit& cInvertedUnit_,
						const ModUnicodeChar* pszKey_, ModSize uiDataUnitSize_, Iterator i_);
	// エリア拡張用
	PagePointer split(InvertedUnit& cInvertedUnit_,
						ModSize uiExpandUnitSize_, Iterator& i_);

	// ページ縮小
	PagePointer reduce(InvertedUnit& cInvertedUnit_, Iterator& i_);

	// エリアサイズを計算する
	static ModSize calcAreaUnitSize(const ModUnicodeChar* pszKey_, ModSize uiDataUnitSize_)
	{
		return calcAreaUnitSize(ModUnicodeCharTrait::length(pszKey_), uiDataUnitSize_);
	}
	static ModSize calcAreaUnitSize(ModSize uiKeyLength_, ModSize uiDataUnitSize_)
	{
		return sizeof(Area)/sizeof(ModUInt32) + uiKeyLength_ / 2 + uiDataUnitSize_;
	}
					
private:
	// ページ内容をセットする
	void load();
	void loadEntry();

	// 2分割する
	PagePointer splitTwoPages(InvertedUnit& cInvertedUnit_, ModSize prev_, ModSize next_, ModSize currentUnitSize_, ModSize expandUnitSize_);
	// 3分割する
	PagePointer splitThreePages(InvertedUnit& cInvertedUnit_, Iterator prevEnd_, Iterator nextStart_);

	// ファイル
	LeafFile& m_cFile;

	// ヘッダー
	Header* m_pHeader;

	// エリア配列
	ModVector<Area*> m_vecpArea;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_LEAFPAGE_H

//
//	Copyright (c) 2002, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
