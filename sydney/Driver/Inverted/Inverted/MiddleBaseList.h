// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleBaseList.h --
// 
// Copyright (c) 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_MIDDLEBASELIST_H
#define __SYDNEY_INVERTED_MIDDLEBASELIST_H

#include "Inverted/InvertedList.h"
#include "Inverted/LeafPage.h"
#include "Inverted/Module.h"
#include "Inverted/OverflowPage.h"

#include "Admin/Verification.h"
#include "PhysicalFile/Types.h"

#include "ModVector.h"

class ModInvertedSmartLocationList;

_SYDNEY_BEGIN

namespace Os
{
	class Path;
}

_SYDNEY_INVERTED_BEGIN

class InvertedIterator;
class InvertedUnit;

//
//	CLASS
//	Inverted::MiddleBaseList --
//
//	NOTES
//
//
class MiddleBaseList : public InvertedList
{
public:
	// コンストラクタ
	MiddleBaseList(InvertedUnit& cInvertedUnit_,		// 転置ファイル
				   LeafPage::PagePointer pLeafPage_,	// リーフページ
				   LeafPage::Iterator ite_);			// リーフページのエリア
	// デストラクタ
	virtual ~MiddleBaseList();

	// 転置リストの挿入 - 1文書挿入用
	virtual bool insert(ModUInt32 uiDocumentID_,
						const ModInvertedSmartLocationList& cLocationList_);
	// 転置リストの挿入 - マージ挿入用
	bool insert(InvertedList& cInvertedList_);
	virtual bool insert(InvertedList& cInvertedList_, ModUInt32 uiLastDocumentID_);

	// 整合性検査を行う
	void verify(Admin::Verification::Treatment::Value uiTreatment_,
				Admin::Verification::Progress& cProgress_,
				const Os::Path& cRootPath_);

	// ロングリストに変換する
	InvertedList* convert();

	// IDブロックを削除する
	void expungeIdBlock(const ModVector<ModUInt32>& vecLastDocumentID_);

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
	
	// これから挿入する転置リストが挿入済みか確認する
	bool isAlreadyInserted(const LeafPage::Area* pInsertedArea_,
						   ModUInt32 uiLastDocumentID_);

	// LOC関係の準備をする
	virtual void prepareForLocation(OverflowPage::PagePointer& pLocPage_,
									ModSize uiUnitSize_,
									ModSize uiNewUnitSize_);

private:
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
		const ModInvertedSmartLocationList& cLocationList_,
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

	// オーバフローファイル
	virtual OverflowFile* getOverflowFile() const = 0;
	virtual void setOverflowFile(OverflowFile* pOverflowFile_) = 0;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_MIDDLEBASELIST_H

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
