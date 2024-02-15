// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OverflowPage.h --
// 
// Copyright (c) 2002, 2003, 2004, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_OVERFLOWPAGE_H
#define __SYDNEY_INVERTED_OVERFLOWPAGE_H

#include "Inverted/Module.h"
#include "Inverted/Page.h"
#include "Inverted/PagePointer.h"
#include "Inverted/Types.h"
#include "PhysicalFile/Page.h"

#include "ModOsDriver.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class OverflowFile;
class InvertedList;

//
//	CLASS
//	Inverted::OverflowPage -- オーバフローファイル共通のページクラス
//
//	NOTES
//
//
class OverflowPage : public Page
{
public:
	//
	//	TYPEDEF
	//	Inverted::OverflowPage::PagePointer
	//
	typedef PageObjectPointer<OverflowPage> PagePointer;

	//
	//	STRUCT
	//	Inverted::OverflowPage::Type
	//
	struct Type
	{
		//
		//	ENUM
		//	ページ種別
		//
		enum Value
		{
			ID		= 1,	// IDページ
			LOC		= 2,	// LOCページ
			IDLOC	= 3		// ID-LOC共通ページ
		};
	};

	//
	//	STRUCT
	//	OverflowPage::LocHeader
	//
	struct LocHeader
	{
		PhysicalFile::PageID m_uiPrevPageID;	// 前のページID
		PhysicalFile::PageID m_uiNextPageID;	// 次のページID
		unsigned short m_usBlockCount;			// ロックブロック数
		unsigned short m_usOffset;				// 最後のロックブロックへのオフセット
	};

	//
	//	CLASS
	//	OverflowPage::LocBlock -- ロックブロックをあらわすクラス
	//
	//	NOTES
	//
	class LocBlock
	{
	public:
		// コンストラクタ(1)
		LocBlock()
			: m_pPage(0), m_pBuffer(0) {}
		// コンストラクタ(2)
		LocBlock(OverflowPage* pPage_, unsigned short usOffset_, ModUInt32* pBuffer_)
			: m_pPage(pPage_), m_usOffset(usOffset_), m_pBuffer(0)
		{
			m_pBuffer = pBuffer_ + m_usOffset;
		}

		// ユニット長
		ModSize getUnitSize() const { return getDataUnitSize() + 1; };

		// 次のブロックに続くかどうか
		bool isContinue() const;
		// データ部のユニット数(アロケート長)
		ModSize getDataUnitSize() const;
		// データ部のビット長(実際の使用ビット数)
		ModSize getDataBitLength() const;
		// データ部のビット長を設定する(実際の使用ビット数)
		void setDataBitLength(ModSize uiBitLength);

		// ユニット数を増やす
		bool expandUnitSize(ModSize iUnitSize_);
		// ビット数を増やす
		void expandBitLength(ModSize iBitLength_);

		// 次のブロックに続くブロックにする
		void setContinueFlag();
		// 次に続くブロックのフラグを削除する
		void unsetContinueFlag();

		// データ部のアドレスを得る
		ModUInt32* getBuffer() { return (m_pBuffer ? m_pBuffer + 1 : m_pBuffer); }

		// オフセット値を得る(ユニット単位)
		unsigned short getOffset() const { return m_usOffset; }

		// 不正なブロックかどうか
		bool isInvalid() const { return m_pBuffer ? false : true; }

	private:
		// 親のページへの参照
		OverflowPage* m_pPage;

		// オフセット
		unsigned short m_usOffset;			// ページ内のオフセット

		// バッファへのポインタ
		ModUInt32* m_pBuffer;
	};

	//
	//	STRUCT
	//	Inverted::OverflowPage::IDHeader
	//
	struct IDHeader
	{
		ModSize m_uiBlockCount;
		ModSize m_uiBlockSize;
	};

	//
	//	CLASS
	//	Inverted::OverflowPage::IDBlock
	//
	//	NOTES
	//	IDブロックをあらわすクラス
	//
	class IDBlock
	{
	public:
		//
		//	STRUCT
		//	Inverted::OverflowPage::IDBlock::Header -- IDブロックのヘッダ情報
		//
		struct Header
		{
			// 先頭文書IDを得る
			ModUInt32 getFirstDocumentID() const;
			// 対応するロックブロックのページID
			PhysicalFile::PageID getLocBlockPageID() const { return m_uiLocBlockPageID; }
			// 対応するロックブロックのページ内のオフセット
			unsigned short getLocBlockOffset() const { return static_cast<unsigned short>(m_uiLocBlockOffset >> 16); }
			void setLocBlockOffset(unsigned short usLocBlockOffset_);
			
			// 削除されたか
			bool isExpunge() const;
			// 削除フラグを設定する
			void setExpunge();

			// 先頭文書ID
			ModUInt32 m_uiFirstDocumentID;
			// 対応するロックブロックのページID
			PhysicalFile::PageID m_uiLocBlockPageID;
			// 対応するロックブロックのページ内のオフセット
			ModUInt32 m_uiLocBlockOffset;
		};

		//
		//	CLASS
		//	Inverted::OverflowPage::IDBlock::Less -- IDブロックの比較を行う
		//
		//	NOTES
		//	ModAlgorithmで使用するので、ModBooleanを返す
		//
		class Less
		{
		public:
			// 比較関数
			ModBoolean operator ()(const Header* b1, const Header* b2) const
			{
				return (b1->getFirstDocumentID() < b2->getFirstDocumentID()) ? ModTrue : ModFalse;
			}
		};

		// コンストラクタ(1)
		IDBlock()
			: m_pHeader(0), m_uiBlockSize(0) {}
		// コンストラクタ(2)
		IDBlock(Header* pHeader_, ModSize uiBlockSize_)
			: m_pHeader(pHeader_), m_uiBlockSize(uiBlockSize_) {}

		// 先頭文書IDを得る
		ModUInt32 getFirstDocumentID() const { return m_pHeader->getFirstDocumentID(); }
		// 最終文書IDを設定する
		void setFirstDocumentID(ModUInt32 uiFirstDocumentID_) { m_pHeader->m_uiFirstDocumentID = uiFirstDocumentID_; }

		// 対応するロックブロックのページIDを得る
		PhysicalFile::PageID getLocBlockPageID() const { return m_pHeader->getLocBlockPageID(); }
		// 対応するロックブロックのページIDを設定する
		void setLocBlockPageID(PhysicalFile::PageID uiLocBlockPageID_) { m_pHeader->m_uiLocBlockPageID = uiLocBlockPageID_; }

		// 対応するロックブロックのページ内オフセットを得る
		unsigned short getLocBlockOffset() const { return m_pHeader->getLocBlockOffset(); }
		// 対応するロックブロックのページ内オフセットを設定する
		void setLocBlockOffset(unsigned short usLocBlockOffset_) { m_pHeader->setLocBlockOffset(usLocBlockOffset_); }

		// 文書IDデータ部のアドレスを得る(後ろからかかれるので、最後のアドレス)
		const ModUInt32* getBuffer() const { return syd_reinterpret_cast<const ModUInt32*>(m_pHeader) + m_uiBlockSize; }
		ModUInt32* getBuffer() { return syd_reinterpret_cast<ModUInt32*>(m_pHeader) + m_uiBlockSize; }

		// ブロック全体のサイズを得る(ユニット単位)
		ModSize getBlockUnitSize() const { return m_uiBlockSize; }
		// 文書IDデータ部のサイズを得る(ビット単位)
		ModSize getDataSize() const
		{
			return (m_uiBlockSize*sizeof(ModUInt32) - 10)*8;
		}

		// 不正なブロックかどうか
		bool isInvalid() const { return m_pHeader ? false : true; }

		// 内容をコピーする
		void copy(const IDBlock& cIDBlock)
		{
			m_uiBlockSize = cIDBlock.m_uiBlockSize;
			ModOsDriver::Memory::copy(m_pHeader, cIDBlock.m_pHeader, m_uiBlockSize*sizeof(ModUInt32));
		}

		// データをクリアする
		void clear()
		{
			ModOsDriver::Memory::reset(m_pHeader, m_uiBlockSize*sizeof(ModUInt32));
			setLocBlockPageID(PhysicalFile::ConstValue::UndefinedPageID);
		}

		// 削除フラグを設定する
		void setExpunge() { m_pHeader->setExpunge(); }
		// 削除フラグが立っているか
		bool isExpunge() { return m_pHeader->isExpunge(); }

		// 比較
		bool operator == (const IDBlock& cIdBlock_) const { return m_pHeader == cIdBlock_.m_pHeader; }
		bool operator != (const IDBlock& cIdBlock_) const { return m_pHeader != cIdBlock_.m_pHeader; }
		bool operator == (const ModUInt32* pHeader_) const { return syd_reinterpret_cast<const ModUInt32*>(m_pHeader) == pHeader_; }
		bool operator != (const ModUInt32* pHeader_) const { return syd_reinterpret_cast<const ModUInt32*>(m_pHeader) != pHeader_; }

	private:
		// ヘッダー
		Header* m_pHeader;
		// ブロックのユニット数
		ModSize m_uiBlockSize;
	};

	// コンストラクタ(1)
	OverflowPage(OverflowFile& cFile_, PhysicalFile::Page* pPage_);
	// コンストラクタ(2)
	OverflowPage(OverflowFile& cFile_, PhysicalFile::Page* pPage_,
				 ModSize uiBlockSize_);
	// コンストラクタ(3)
	OverflowPage(OverflowFile& cFile_, PhysicalFile::Page* pPage_,
				 PhysicalFile::PageID uiPrevPageID_, PhysicalFile::PageID uiNextPageID_);
	// コンストラクタ(4)
	OverflowPage(OverflowFile& cFile_, PhysicalFile::Page* pPage_,
				 ModSize uiBlockSize_,
				 PhysicalFile::PageID uiPrevPageID_, PhysicalFile::PageID uiNextPageID_);
	// デストラクタ
	virtual ~OverflowPage();

	// 初期化する
	void reset(ModSize uiBlockSize_);
	void reset(PhysicalFile::PageID uiPrevPageID_,
			   PhysicalFile::PageID uiNextPageID_);
	void reset(ModSize uiBlockSize_,
			   PhysicalFile::PageID uiPrevPageID_,
			   PhysicalFile::PageID uiNextPageID_);

	// リセットする
	void reset2(PhysicalFile::Page* pPage_);
	void reset2(PhysicalFile::Page* pPage_,
				ModSize uiBlockSize_);
	void reset2(PhysicalFile::Page* pPage_,
				PhysicalFile::PageID uiPrevPageID_,
				PhysicalFile::PageID uiNextPageID_);
	void reset2(PhysicalFile::Page* pPage_,
				ModSize uiBlockSize_,
				PhysicalFile::PageID uiPrevPageID_,
				PhysicalFile::PageID uiNextPageID_);

	// ページ種別を得る
	Type::Value getType() const { return m_eType; }
	// ページ種別を設定する
	void setType(Type::Value eType_);

	// IDブロック数を得る
	ModSize getIDBlockCount() const { return m_pIDHeader->m_uiBlockCount; }
	// IDブロックサイズを得る
	ModSize getIDBlockSize() const { return m_pIDHeader->m_uiBlockSize; }

	// あたらしいIDブロックを確保する
	IDBlock allocateIDBlock();
	// IDブロックを削除する
	void freeIDBlock(ModSize uiPosition_);

	// IDブロックを得る
	IDBlock getIDBlock(ModSize uiPosition_);
	// IDブロックを検索する
	IDBlock lowerBoundIDBlock(ModUInt32 uiDocumentID_, ModSize& uiPosition_, bool bUndo_ = false);

	// IDブロックを移動する
	void moveIDBlock(PagePointer pPage_);

	// 直前の文書IDを得る
	ModUInt32 getPrevDocumentID(ModSize uiPosition_);
	// 直後の文書IDを得る
	ModUInt32 getNextDocumentID(ModSize uiPosition_);

	// 前方のLOCページのページIDを得る
	PhysicalFile::PageID getPrevPageID() const { return m_pLocHeader->m_uiPrevPageID; }
	// 後方のLOCページのページIDを得る
	PhysicalFile::PageID getNextPageID() const { return m_pLocHeader->m_uiNextPageID; }

	// 前方のLOCページのページIDを設定する
	void setPrevPageID(PhysicalFile::PageID uiPrevPageID_);
	// 後方のLOCページのページIDを設定する
	void setNextPageID(PhysicalFile::PageID uiNextPageID_);

	// ブロック数を得る
	ModSize getLocBlockCount() const { return m_pLocHeader->m_usBlockCount; }

	// 新しいLOCブロックを作成する
	LocBlock allocateLocBlock();
	// LOCブロックを削除する
	void freeLocBlock();

	// 指定したオフセット位置のLOCブロックを得る
	LocBlock getLocBlock(unsigned short usOffset_);
	// 先頭のLOCブロックを得る(ページ跨ぎ専用)
	LocBlock getLocBlock();

	// 使用ユニット数を増やす
	bool addUsedUnitSize(ModSize uiUnitSize_);

	// 使用ユニット数を得る
	ModSize getUsedUnitSize() const;

	// 十分な空き領域があるか？
	bool isLargeEnough(ModSize uiExpandUnitSize_) const
		{ return isLargeEnough(0, uiExpandUnitSize_); }
	bool isLargeEnough(ModSize uiUnitSize_, ModSize uiNewUnitSize_) const
		{ return (uiUnitSize_ + getFreeUnitSize() >= uiNewUnitSize_); }

	// このページ中の削除ずみIDブロックを登録する
	bool enterDeleteIdBlock(InvertedList& cInvertedList_);

private:
	// ファイル
	OverflowFile& m_cFile;

	// ページ種別を読む
	void readType();
	// ID情報を読み込む
	void loadIDHeader();
	void loadIDData();
	// Loc情報を読み込む
	void loadLocHeader();

	// ページ種別
	Type::Value m_eType;
	// 使用ユニット数
	mutable ModSize m_uiUsedUnitSize;

	// IDページのヘッダー情報
	IDHeader* m_pIDHeader;
	// Locページのヘッダー情報
	LocHeader* m_pLocHeader;

	// IDBlock配列
	ModVector<IDBlock::Header*> m_vecIDBlockHeader;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_OVERFLOWPAGE_H

//
//	Copyright (c) 2002, 2003, 2004, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
