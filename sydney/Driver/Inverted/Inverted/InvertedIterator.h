// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedIterator.h --
// 
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_INVERTEDITERATOR_H
#define __SYDNEY_INVERTED_INVERTEDITERATOR_H

#include "Inverted/Module.h"
#include "ModInvertedIterator.h"
#include "ModInvertedSmartLocationList.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

//
//	CLASS
//	Inverted::InvertedIterator --
//
//	NOTES
//
//
class InvertedIterator : public ModInvertedIterator
{
public:
	//コンストラクタ
	InvertedIterator();
	//デストラクタ
	virtual ~InvertedIterator();

	// 文書IDを検索する
	ModBoolean find(const DocumentID uiDocumentID_) { return find(uiDocumentID_, false); }
	virtual ModBoolean find(const DocumentID uiDocumentID_, bool bUndo_) = 0;

	// 文書IDをlower_bound検索する
	ModBoolean lowerBound(const DocumentID uiDocumentID_) { return lowerBound(uiDocumentID_, false); }
	virtual ModBoolean lowerBound(const DocumentID uiDocumentID_, bool bUndo_) = 0;

	// 現在位置のデータを削除する
	virtual void expunge() = 0;

	// 現在位置にデータを挿入する
	virtual void undoExpunge(ModUInt32 uiDocumentID_, const ModInvertedSmartLocationList& cLocationList_) = 0;

	// 位置情報の先頭アドレスを得る
	virtual ModUInt32* getHeadAddress() = 0;
	// 現在の位置情報のオフセットを得る
	virtual ModSize getLocationOffset() = 0;
	// 現在の位置情報のビット長を得る
	virtual ModSize getLocationBitLength() = 0;
	// 現在の位置情報データのオフセットを得る
	virtual ModSize getLocationDataOffset() = 0;
	// 現在の位置情報データのビット長を得る
	virtual ModSize getLocationDataBitLength() = 0;
	// 次のLOCブロックに続くか？
	virtual bool isContinue() { return false; }
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_INVERTEDITERATOR_H

//
//	Copyright (c) 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
