// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortNolocationListIterator.h --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_SHORTNOLOCATIONLISTITERATOR_H
#define __SYDNEY_FULLTEXT2_SHORTNOLOCATIONLISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/ShortBaseListIterator.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::ShortNolocationListIterator --
//
//	NOTES
//
//
class ShortNolocationListIterator : public ShortBaseListIterator
{
public:
	//コンストラクタ
	ShortNolocationListIterator(InvertedList& cInvertedList_);
	//デストラクタ
	virtual ~ShortNolocationListIterator();

	// 先頭に戻る
	void reset();

	// 位置情報内の文書頻度を得る
	ModSize getTermFrequency();
	
	// 現在位置のデータを削除する
	void expunge();

	// 現在位置にデータを挿入する
	void undoExpunge(ModUInt32 uiDocumentID_,
					 const SmartLocationList& cLocationList_);

	// 位置情報の先頭アドレスを得る
	ModUInt32* getHeadAddress() { return m_pHeadAddress; }
	// 現在の位置情報のオフセットを得る
	ModSize getLocationOffset();
	// 現在の位置情報のビット長を得る
	ModSize getLocationBitLength();
	// 現在の位置情報データのオフセットを得る
	ModSize getLocationDataOffset();
	// 現在の位置情報データのビット長を得る
	ModSize getLocationDataBitLength();

	// コピーする
	ListIterator* copy() const;

private:
	// 位置情報の位置を文書IDの位置まで移動する
	void synchronize(bool bUndo_ = false);
	// 位置情報変数を設定する
	void setLocationInformation();

	// 位置情報を削除する
	void expungeLocation();

	// 現在の位置情報の位置
	int m_iCurrentLocPosition;
	// 現在の位置情報
	ModSize m_uiCurrentLocOffset;
	ModSize m_uiCurrentLocLength;
	// 現在の文書頻度
	ModSize m_uiCurrentFrequency;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_SHORTNOLOCATIONLISTITERATOR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
