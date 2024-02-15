// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedList.h --
// 
// Copyright (c) 2000, 2001, 2004, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_MODINVERTEDLIST_H
#define __SYDNEY_INVERTED_MODINVERTEDLIST_H

#include "Common/Common.h"
#include "ModInvertedTypes.h"

class ModUnicodeString;
class ModInvertedIterator;

//
//	CLASS
//	Inverted::ModInvertedList --
//
//	NOTES
//	本クラスはFTSの検索処理が必要とするインターフェース
//	を規定しているクラスである。
//
class ModInvertedList
{
public:
	//コンストラクタ/デストラクタ
	ModInvertedList() {}
	virtual ~ModInvertedList() {}

	// 文書頻度の取得
	virtual ModSize	getDocumentFrequency() const = 0;

	// キー文字列の取得
	virtual const ModUnicodeString& getKey() const = 0;

	// 先頭に位置付けられた反復子の取得
	// (得たポインタは delete すること)
	virtual ModInvertedIterator* begin() const = 0;

	// 自分の複製を作る
	virtual ModInvertedList* clone() const = 0;

	//
	// マニピュレータ
	//

	virtual ModBoolean reset(const ModUnicodeString&			cstrKey_,
							 const ModInvertedListAccessMode	eAccessMode_) = 0;

	// 次のリストに移動
	virtual ModBoolean next() = 0;

	// 最終文書IDを得る
	virtual ModInvertedDocumentID getLastDocumentID()  = 0;

	// 位置情報を格納しているか？
	virtual ModBoolean isNolocation() = 0;
};

#endif //__SYDNEY_INVERTED_MODINVERTEDLIST_H

//
//	Copyright (c) 2000, 2001, 2004, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
