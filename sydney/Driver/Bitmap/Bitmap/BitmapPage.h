// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BitmapPage -- ビットマップ部分のページ
// 
// Copyright (c) 2005, 2007, 2017, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_BITMAPPAGE_H
#define __SYDNEY_BITMAP_BITMAPPAGE_H

#include "Bitmap/Module.h"
#include "Bitmap/Page.h"
#include "Bitmap/PagePointer.h"

#include "PhysicalFile/File.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class BitmapFile;

//
//	CLASS
//	Bitmap::BitmapPage -- ビットマップ部分のページ
//
//	NOTES
//
class BitmapPage : public Page
{
public:
	//
	//	STRUCT
	//	Bitmap::BitmapPage::Header
	//
	struct Header
	{
		ModUInt32				m_uiCount;		// ページのエントリ数
	};

	// コンストラクタ
	BitmapPage(BitmapFile& cFile_);
	// デストラクタ
	virtual ~BitmapPage();

	// ページサイズを得る
	ModSize getPageSize()
		{ return Page::getPageSize() - sizeof(Header)/sizeof(ModUInt32); }

	// 物理ページを設定する
	void setPhysicalPage(PhysicalFile::Page* pPhysicalPage_);

	// 初期化する
	void initialize();

	// ビットをONする
	void on(ModSize position_);
	// ビットをOFFする
	void off(ModSize position_);

	// ビットを設定する
	void set(ModSize offset_, ModUInt32 b_);

	// ビット列を取り出す
	ModUInt32 get(ModSize offset_)
	{
		return *(m_pValue + offset_);
	}

	// ビット列の先頭を得る
	ModUInt32* begin() { return m_pValue; }
	const ModUInt32* begin() const { return m_pValue; }

	// 物理ページをdetachする
	void detach();

	// ページカウントを得る
	ModSize getCount() { return m_pHeader->m_uiCount; }
	// ページカウントを追加する
	void addCount(ModSize count_)
	{
		dirty();
		m_pHeader->m_uiCount += count_;
	}

	// 整合性検査
	void verify();

	// 1ページに格納できるビット数を得る
	static ModSize getBitCount(Os::Memory::Size uiPageSize_);

	// ビット数を得る
	static ModSize getCount(ModUInt32 bitmap_);

private:
	// ファイル
	BitmapFile& m_cFile;

	// ヘッダー
	Header* m_pHeader;
	// データ
	ModUInt32* m_pValue;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_BITMAPPAGE_H

//
//	Copyright (c) 2005, 2007, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
