// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BatchNolocationNoTFList.h --
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

#ifndef __SYDNEY_INVERTED_BATCHNOLOCATIONNOTFLIST_H
#define __SYDNEY_INVERTED_BATCHNOLOCATIONNOTFLIST_H

#include "Inverted/AutoPointer.h"
#include "Inverted/BatchBaseList.h"
#include "Inverted/LeafPage.h"
#include "Inverted/Module.h"

class ModInvertedSmartLocationList;

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class BatchListMap;
class InvertedUnit;

//
//	CLASS
//	Inverted::BatchNolocationNoTFList --
//
//	NOTES
//
//
class BatchNolocationNoTFList : public BatchBaseList
{
public:
	// コンストラクタ(1)
	BatchNolocationNoTFList(InvertedUnit& cInvertedUnit_,
			  BatchListMap& cBatchListMap_,
			  const ModUnicodeChar* pszKey_);
	// コンストラクタ(2)
	BatchNolocationNoTFList(InvertedUnit& cInvertedUnit_, LeafPage::Area* pArea_);
	// デストラクタ
	virtual ~BatchNolocationNoTFList();

	// 位置情報リストを格納していないか
	bool isNolocation() const { return true; }
	// TFを格納していないか (TFを格納しない時は位置情報リストも格納しない)
	bool isNoTF() const { return true; }
	
	// イテレータを得る -- 得られたインスタンスは呼び出し元で開放する
	InvertedIterator* begin() const;

	// エリアを得る
	LeafPage::Area* getArea() { return m_pArea; }
	const LeafPage::Area* getArea() const { return m_pArea; }

	// 位置情報リストの圧縮ビット長を得る(実際は何もしない)
	ModSize getCompressedBitLengthLocationList(
		const ModInvertedSmartLocationList& cLocationList_,
		ModSize& uiBitLength_);
	// 位置情報リストを圧縮して書き出す(実際は何もしない)
	void writeLocationList(
		const ModInvertedSmartLocationList& cLocationList_,
		ModSize uiBitLength_,
		ModUInt32* pHeadAddress_,
		ModSize& uiBitOffset_);

private:
	// バッチリストマップを取得する
	BatchListMap* getMap() { return m_pMap; }
	
	// エリアを設定する
	void setArea(LeafPage::Area* pArea_) { m_pArea = pArea_; }
	
	// バッチリストマップ
	BatchListMap* m_pMap;

	// エリア
	AutoPointer<LeafPage::Area> m_pArea;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_BATCHNOLOCATIONNOTFLIST_H

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
