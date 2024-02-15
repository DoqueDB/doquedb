// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortBaseList.h --
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

#ifndef __SYDNEY_INVERTED_SHORTBASELIST_H
#define __SYDNEY_INVERTED_SHORTBASELIST_H

#include "Inverted/InvertedList.h"
#include "Inverted/LeafPage.h"
#include "Inverted/Module.h"

#include "Admin/Verification.h"

class ModInvertedSmartLocationList;

_SYDNEY_BEGIN
namespace Os
{
	class Path;
}

_SYDNEY_INVERTED_BEGIN

class InvertedUnit;

//
//	CLASS
//	Inverted::ShortBaseList --
//
//	NOTES
//
//
class ShortBaseList : public InvertedList
{
public:
	// コンストラクタ(1) - 該当する索引単位のエリアがある場合
	ShortBaseList(InvertedUnit& cInvertedUnit_,			// 転置ファイル
				  LeafPage::PagePointer pLeafPage_,		// リーフページ
				  LeafPage::Iterator ite_);				// リーフページのエリア
	// コンストラクタ(2) - 該当する索引単位のエリアがない場合
	ShortBaseList(InvertedUnit& cInvertedUnit_,			// 転置ファイル
				  const ModUnicodeChar* pKey_,			// 索引単位
				  LeafPage::PagePointer pLeafPage_,		// リーフページ
				  LeafPage::Iterator ite_);				// リーフページのエリア
														//(lower_bound検索結果)
	// デストラクタ
	virtual ~ShortBaseList();

	// 転置リストの挿入 - 1文書挿入用
	virtual bool insert(ModUInt32 uiDocumentID_,
						const ModInvertedSmartLocationList& cLocationList_);
	// 転置リストの挿入 - マージ挿入用
	virtual bool insert(InvertedList& cInvertedList_);
	
	// 整合性検査を行う
	virtual void verify(Admin::Verification::Treatment::Value uiTreatment_,
						Admin::Verification::Progress& cProgress_,
						const Os::Path& cRootPath_);

	// ミドルリストに変換する
	InvertedList* convert();

protected:
	// エリア領域を挿入または拡張する
	virtual bool insertOrExpandArea(
		ModUInt32 uiDocumentID_,
		const ModInvertedSmartLocationList& vecLocationList_,
		ModSize uiLocBitLength_);
	// エリア領域を拡張する
	virtual bool expandArea(ModSize uiExpandUnit_);

private:
	// エリア領域を縮める
	void shortenArea(ModSize uiShotenUnit_);

	// convert時にMiddleListを作成する
	virtual InvertedList* makeMiddleList() = 0;
	// convert時にBatchListを作成する
	virtual InvertedList* makeBatchList(LeafPage::Area* pArea_) = 0;

	// リーフページにエリアが存在するかどうか
	virtual bool isExist() const = 0;
	virtual void setExist(bool bExist_) = 0;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_SHORTBASELIST_H

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
