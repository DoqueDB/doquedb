// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortNolocationList.h --
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

#ifndef __SYDNEY_INVERTED_SHORTNOLOCATIONLIST_H
#define __SYDNEY_INVERTED_SHORTNOLOCATIONLIST_H

#include "Inverted/LeafPage.h"
#include "Inverted/ShortBaseList.h"
#include "Inverted/Module.h"

class ModInvertedSmartLocationList;

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class InvertedIterator;
class InvertedUnit;

//
//	CLASS
//	Inverted::ShortNolocationList --
//
//	NOTES
//
//
class ShortNolocationList : public ShortBaseList
{
public:
	// コンストラクタ(1) - 該当する索引単位のエリアがある場合
	ShortNolocationList(InvertedUnit& cInvertedUnit_,		// 転置ファイル
						LeafPage::PagePointer pLeafPage_,	// リーフページ
						LeafPage::Iterator ite_);			// リーフページのエリア
	// コンストラクタ(2) - 該当する索引単位のエリアがない場合
	ShortNolocationList(InvertedUnit& cInvertedUnit_,		// 転置ファイル
						const ModUnicodeChar* pKey_,		// 索引単位
						LeafPage::PagePointer pLeafPage_,	// リーフページ
						LeafPage::Iterator ite_);			// リーフページのエリア
															//(lower_bound検索結果)
	// デストラクタ
	virtual ~ShortNolocationList();

	// 位置情報を格納していないか
	bool isNolocation() const { return true; }
	// TFを格納していないか (TFを格納しない時は位置情報も格納しない)
	bool isNoTF() const { return false; }
	
#ifdef DEBUG
	bool insert(ModUInt32 uiDocumentID_,
				const ModInvertedSmartLocationList& cLocationList_);
#endif
	
	// イテレータを得る -- 得られたインスタンスは呼び出し元で開放する
	InvertedIterator* begin() const;

	// 位置情報(実際はTFのみ)の圧縮ビット長を得る
	ModSize getCompressedBitLengthLocationList(
		const ModInvertedSmartLocationList& cLocationList_,
		ModSize& uiBitLength_);
	// 位置情報(実際はTFのみ)を圧縮して書き出す
	void writeLocationList(
		const ModInvertedSmartLocationList& cLocationList_,
		ModSize uiBitLength_,
		ModUInt32* pHeadAddress_,
		ModSize& uiBitOffset_);

private:
	// convert時にMiddleListを作成する
	InvertedList* makeMiddleList();
	// convert時にBatchListを作成する
	InvertedList* makeBatchList(LeafPage::Area* pArea_);

	// リーフページにエリアが存在するかどうか
	bool isExist() const { return m_bExist; }
	void setExist(bool bExist_) { m_bExist = bExist_; }

	// 存在するかどうか
	bool m_bExist;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_SHORTNOLOCATIONLIST_H

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
