// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BatchBaseList.h --
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

#ifndef __SYDNEY_INVERTED_BATCHBASELIST_H
#define __SYDNEY_INVERTED_BATCHBASELIST_H

#include "Inverted/AutoPointer.h"
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

class BatchListMap;
class InvertedUnit;

//
//	CLASS
//	Inverted::BatchBaseList --
//
//	NOTES
//
//
class BatchBaseList : public InvertedList
{
public:
	// コンストラクタ(1)
	BatchBaseList(InvertedUnit& cInvertedUnit_,
				  BatchListMap& cBatchListMap_,
				  const ModUnicodeChar* pszKey_);
	// コンストラクタ(2)
	BatchBaseList(InvertedUnit& cInvertedUnit_, LeafPage::Area* pArea_);
	// デストラクタ
	virtual ~BatchBaseList();

	// 転置リストの挿入 - 1文書挿入用
	bool insert(ModUInt32 uiDocumentID_,
				const ModInvertedSmartLocationList& cLocationList_);
	// 転置リストの挿入 - マージ挿入用
	bool insert(InvertedList& cInvertedList_) { return false; }

	// 整合性検査を行う
	void verify(Admin::Verification::Treatment::Value uiTreatment_,
				Admin::Verification::Progress& cProgress_,
				const Os::Path& cRootPath_) {}

	// 格納されている文書数を得る
	ModUInt32 getCount() const { return getArea()->getDocumentCount(); }

	// エリアを得る
	virtual LeafPage::Area* getArea() = 0;
	virtual const LeafPage::Area* getArea() const = 0;

	// コンバート
	InvertedList* convert() { return 0; }

protected:
	// エリアの初期ユニット数を取得する
	ModSize getAllocateUnitSize() const;

private:
	// バッチリストマップを取得する
	virtual BatchListMap* getMap() = 0;
	
	// エリアを設定する
	virtual void setArea(LeafPage::Area* pArea_) = 0;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_BATCHBASELIST_H

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
