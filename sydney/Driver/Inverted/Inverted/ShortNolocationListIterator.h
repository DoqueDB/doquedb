// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortNolocationListIterator.h --
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

#ifndef __SYDNEY_INVERTED_SHORTNOLOCATIONLISTITERATOR_H
#define __SYDNEY_INVERTED_SHORTNOLOCATIONLISTITERATOR_H

#include "Inverted/LeafPage.h"
#include "Inverted/Module.h"
#include "Inverted/ShortBaseListIterator.h"

#ifdef DEBUG
class ModInvertedSmartLocationList;
#endif

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class InvertedList;

//
//	CLASS
//	Inverted::ShortNolocationListIterator --
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

#ifdef DEBUG
	void expunge();
	void undoExpunge(ModUInt32 uiDocumentID_, const ModInvertedSmartLocationList& cLocationList_);
#endif

	// 位置情報の先頭アドレスを得る
	ModUInt32* getHeadAddress() { return m_pHeadAddress; }
	
private:
	// 位置情報変数を設定する
	void setLocationInformation();
	// 位置情報を読み飛ばす
	void skipLocationInformation();

	// 位置情報
	int getLocPosition() const { return m_iCurrentLocPosition; }
	void setLocPosition(int uiPosition_)
		{ m_iCurrentLocPosition = uiPosition_; }
	void incrementLocPosition() { ++m_iCurrentLocPosition; }
	ModSize getLocOffset() const { return m_uiCurrentLocOffset; }
	void setLocOffset(ModSize uiOffset_)
		{ m_uiCurrentLocOffset = uiOffset_; }
	ModSize getLocLength() const { return m_uiCurrentLocLength; }
	void setLocLength(ModSize uiLength_)
		{ m_uiCurrentLocLength = uiLength_; }
	ModSize getTermFrequency() const { return m_uiCurrentFrequency; }

	
	// エリアの先頭アドレス
	ModUInt32* m_pHeadAddress;

	// 現在の位置情報の位置
	int m_iCurrentLocPosition;
	// 現在の位置情報
	ModSize m_uiCurrentLocOffset;
	ModSize m_uiCurrentLocLength;
	// 現在の文書頻度
	ModSize m_uiCurrentFrequency;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_SHORTNOLOCATIONLISTITERATOR_H

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
