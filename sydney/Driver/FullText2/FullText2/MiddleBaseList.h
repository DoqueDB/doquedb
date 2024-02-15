// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleBaseList.h --
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

#ifndef __SYDNEY_FULLTEXT2_MIDDLEBASELIST_H
#define __SYDNEY_FULLTEXT2_MIDDLEBASELIST_H

#include "FullText2/Module.h"
#include "FullText2/InvertedList.h"

#include "FullText2/LeafPage.h"
#include "FullText2/LocationList.h"
#include "FullText2/OverflowFile.h"
#include "FullText2/OverflowPage.h"

#include "Admin/Verification.h"
#include "PhysicalFile/Types.h"

#include "ModMap.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace Os
{
	class Path;
}

_SYDNEY_FULLTEXT2_BEGIN

class InvertedIterator;
class InvertedUnit;

//
//	CLASS
//	FullText2::MiddleBaseList --
//
//	NOTES
//
//
class MiddleBaseList : public InvertedList
{
public:
	// コンストラクタ(1)
	MiddleBaseList(InvertedUnit& cInvertedUnit_,		// 転置ファイル
				   LeafPage::PagePointer pLeafPage_,	// リーフページ
				   LeafPage::Iterator ite_);			// リーフページのエリア
	// コンストラクタ(2)
	MiddleBaseList(InvertedUnit& cInvertedUnit_,		// 転置ファイル
				   LeafPage::Area* pTmpArea_);			// エリア
	// デストラクタ
	virtual ~MiddleBaseList();

	// 転置リストの挿入 - 1文書挿入用
	bool insert(ModUInt32 uiDocumentID_,
				const SmartLocationList& cLocationList_);
	// 転置リストの挿入 - マージ挿入用
	bool insert(InvertedList& cInvertedList_);

	// 整合性検査を行う
	void verify(Admin::Verification::Treatment::Value uiTreatment_,
				Admin::Verification::Progress& cProgress_,
				const Os::Path& cRootPath_);

	// エリアを得る
	LeafPage::Area* getArea();
	const LeafPage::Area* getArea() const;

	// ロングリストに変換する
	InvertedList* convert();

	// バキューム
	void vacuum();

	// IDブロックを削除する
	void expungeIdBlock(const ModVector<ModUInt32>& vecLastDocumentID_);

	// 転置ファイルユニットを得る
	InvertedUnit& getInvertedUnit() { return m_cInvertedUnit; }

protected:
	// 文書IDを1つ書き出す
	bool insertDocumentID(ModUInt32 uiDocumentID_,
						  OverflowPage::IDBlock& cIdBlock_,
						  OverflowPage::PagePointer& pLocPage_,
						  OverflowPage::LocBlock& cLocBlock_);

	// IDページを得る
	OverflowPage::PagePointer allocateIdPage(
		OverflowPage::PagePointer pPrevPage_);
	
	// LOCブロックを確保する
	virtual OverflowPage::LocBlock allocateLocBlock(
		OverflowPage::PagePointer& pLocPage_);

	// LOCブロックのデータ領域を拡張する
	virtual void expandLocBlock(OverflowPage::LocBlock& cLocBlock_,
								ModSize uiInsertedLocBitLength_,
								ModSize uiDataBitLength_,
								ModSize uiDataUnitSize_);
	virtual void expandLocBlock(OverflowPage::PagePointer& pLocPage_,
								OverflowPage::LocBlock& cLocBlock_,
								ModSize uiInsertedLocBitLength_,
								ModSize& uiDataBitLength_,
								ModSize& uiDataUnitSize_);
	
	// 最終DIRブロックを取得する
	LeafPage::DirBlock* getLastDirBlock();
	// 最終DIRブロックのデータを設定する
	void setToLastDirBlock(PhysicalFile::PageID uiIDPageID_,
						   ModUInt32 uiDocumentID_);
	
	// DIRブロックを追加する
	bool addDirBlock(OverflowPage::IDBlock& cLastBlock_);
	
	// 自身を初期化する
	virtual void initialize();
	
	// LOC関係の準備をする
	virtual void prepareForLocation(OverflowPage::PagePointer& pLocPage_,
									ModSize uiUnitSize_,
									ModSize uiNewUnitSize_);

	// LOCページを得る
	OverflowPage::PagePointer allocateLocPage(
		OverflowPage::PagePointer pPrevPage_);
	// ID-LOCページを得る
	OverflowPage::PagePointer allocateIdLocPage(
		OverflowPage::PagePointer pPrevPage_);
	
	// 最終LOCページを開放する
	void freeLastLocPage();

	// ID-LOCページをLOCページにコンバートする
	OverflowPage::PagePointer convertToLocPage(
		OverflowPage::PagePointer pIdLocPage_);

	// IDブロックを確保する
	bool allocateIDBlock(OverflowPage::IDBlock& cIdBlock_,
						 OverflowPage::PagePointer& pLocPage_,
						 OverflowPage::LocBlock& cLocBlock_);
	// 最終IDブロックをIDページにコピーする
	virtual bool copyIDBlock(OverflowPage::PagePointer& pIdPage_,
							 OverflowPage::IDBlock& cLastBlock_);
	
	// 最終IDブロック関係を初期化する
	void initializeLastBlock(OverflowPage::IDBlock& cLastBlock_,
							 ModUInt32 uiDocumentID_,
							 OverflowPage::PagePointer& pLocPage_,
							 OverflowPage::LocBlock& cLocBlock_);

	// 最終DIRブロックをデータを設定する
	void setToLastDirBlock(PhysicalFile::PageID uiIDPageID_);
	
	// 位置情報を1つ書き出す
	virtual void insertLocation(
		const SmartLocationList& cLocationList_,
		OverflowPage::PagePointer& pLocPage_,
		OverflowPage::LocBlock& cLocBlock_);
	virtual void insertLocation(
		InvertedIterator& cIterator_,
		OverflowPage::PagePointer& pLocPage_,
		OverflowPage::LocBlock& cLocBlock_);

	// LOCブロックを拡張する
	bool expandUnitSize(OverflowPage::PagePointer& pLocPage_,
						OverflowPage::LocBlock& cLocBlock_,
						ModSize uiExpandUnitSize_);
	
	// LOC関係の準備をする
	void prepareForLocation(OverflowPage::PagePointer& pLocPage_,
							ModSize uiExpandUnitSize_)
		{ prepareForLocation(pLocPage_, 0, uiExpandUnitSize_); }

	// LOCブロックの整合性検査を行う
	virtual void verifyLocBlock(
		Admin::Verification::Treatment::Value uiTreatment_,
		Admin::Verification::Progress& cProgress_,
		const Os::Path& cRootPath_,
		const OverflowPage::IDBlock& cIdBlock_,
		OverflowPage::PagePointer& pLocPage_);

	// 最終LOCブロックの次のLOCページの整合性検査を行う
	virtual void verifyNextLocPage(
		Admin::Verification::Treatment::Value uiTreatment_,
		Admin::Verification::Progress& cProgress_,
		const Os::Path& cRootPath_,
		OverflowPage::PagePointer& pLocPage_);

	// ミドルリストを得る
	virtual MiddleBaseList* makeTmpMiddleList(InvertedUnit& cInvertedUnit_,
											  LeafPage::Area* pTmpArea_) = 0;
	// すべてのOverflowFileのページを解放する
	void freeAllPages();
	// IDブロック以下のページをマップに追加する
	void addIdBlockPages(const OverflowPage::IDBlock& cIdBlock_);
	// LOCページをマップに追加する
	void addLocPages(PhysicalFile::PageID uiPageID_);

	// 転置ファイルユニット
	InvertedUnit& m_cInvertedUnit;
	
	// オーバフローファイル
	OverflowFile* m_pOverflowFile;

	// 一時的なエリア
	LeafPage::Area* m_pTmpArea;

	typedef	ModMap<PhysicalFile::PageID, OverflowFile::PagePointer,
				   ModLess<PhysicalFile::PageID> >	FreePageMap;

	// フリーページマップ
	FreePageMap m_mapFreePage;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_MIDDLEBASELIST_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
