// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SmartLocationList.h -- 位置情報リストをあらわすクラス
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

#ifndef __SYDNEY_FULLTEXT2_SMARTLOCATIONLIST_H
#define __SYDNEY_FULLTEXT2_SMARTLOCATIONLIST_H

#include "FullText2/Module.h"
#include "FullText2/LocationList.h"
#include "FullText2/LocationListIterator.h"

#include "ModInvertedCoder.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::SmartLocationList --
//
//	NOTES
//	位置情報を圧縮して保持する
//
class SmartLocationList : public LocationList
{
public:
	// コンストラクタ(1) -- ModMap用
	SmartLocationList();
	// コンストラクタ(2)
	SmartLocationList(ModInvertedCoder* pLocationCoder_, ModSize length_,
					  bool bNoLocation_ = false);
	// デストラクタ
	virtual ~SmartLocationList();
	// コピーコンストラクタ -- ModMap用
	SmartLocationList(const SmartLocationList& src_);
	// 代入演算子 -- ModMap用
	SmartLocationList& operator = (const SmartLocationList& src_);

	// イテレータを得る
	LocationListIterator::AutoPointer getIterator() const;

	// 位置情報を追加する
	void pushBack(ModSize uiLocation_);
	// 中身をclearする
	void clear();

	// バッファを得る
	const ModUInt32* getBuffer() const { return m_pBuffer; }
	// ビット長を得る
	ModSize getBitLength() const { return m_uiBitOffset; }

private:
	// 圧縮器
	ModInvertedCoder* m_pLocationCoder;		// 位置情報用

	// 最後に格納した位置情報
	ModSize m_uiLastLocation;
	// 現在のビット位置
	ModSize m_uiBitOffset;
	
	// 位置情報を格納したバッファ
	ModUInt32* m_pBuffer;
	// バッファのサイズ
	ModSize m_uiBufferSize;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_SMARTLOCATIONLIST_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
