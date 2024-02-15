// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortNolocationNoTFListIterator.h --
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

#ifndef __SYDNEY_INVERTED_SHORTNOLOCATIONNOTFLISTITERATOR_H
#define __SYDNEY_INVERTED_SHORTNOLOCATIONNOTFLISTITERATOR_H

#include "Inverted/Module.h"
#include "Inverted/ShortBaseListIterator.h"
#include "Inverted/LeafPage.h"

#ifdef DEBUG
class ModInvertedSmartLocationList;
#endif

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class InvertedList;

//
//	CLASS
//	Inverted::ShortNolocationNoTFListIterator --
//
//	NOTES
//
//
class ShortNolocationNoTFListIterator : public ShortBaseListIterator
{
public:
	//コンストラクタ
	ShortNolocationNoTFListIterator(InvertedList& cInvertedList_);
	//デストラクタ
	virtual ~ShortNolocationNoTFListIterator();

	// 位置情報内の文書頻度を得る (TFは格納されていないので常に1を返す)
	ModSize getInDocumentFrequency() { return 1; }
	
#ifdef DEBUG
	void expunge();
	void undoExpunge(ModUInt32 uiDocumentID_, const ModInvertedSmartLocationList& cLocationList_);
#endif

	// 位置情報の先頭アドレスを得る (呼ばれないはず)
	ModUInt32* getHeadAddress();
	
private:
	// 位置情報の位置を文書IDの位置まで移動する (何もしない)
	void synchronize() {}
	
	// 位置の先頭に戻る (何もしない)
	void resetLocation() {}
	
	// 位置情報を削除する (何もしない)
	void expungeLocation() {}
	
	// 位置情報の削除を取り消す (何もしない)
	void undoExpungeLocation(const ModInvertedSmartLocationList& cLocationList_) {}
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_SHORTNOLOCATIONNOTFLISTITERATOR_H

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
