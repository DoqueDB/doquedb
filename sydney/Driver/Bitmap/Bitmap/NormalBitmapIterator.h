// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalBitmapIterator.h -- バリュー部分を得るイテレータ
// 
// Copyright (c) 2007, 2017, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_NORMALBITMAPITERATOR_H
#define __SYDNEY_BITMAP_NORMALBITMAPITERATOR_H

#include "Bitmap/Module.h"
#include "Bitmap/BitmapIterator.h"
#include "Bitmap/DirPage.h"
#include "Bitmap/BitmapPage.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class BitmapFile;

//
//	CLASS
//	Bitmap::NormalBitmapIterator -- バリュー部分を得るイテレータ
//
//	NOTES
//
class NormalBitmapIterator : public BitmapIterator
{
public:
	// コンストラクタ
	NormalBitmapIterator(BitmapFile& cFile_);
	// デストラクタ
	virtual ~NormalBitmapIterator();

	// 現在位置のビットマップを得て、次の位置に進む
	ModUInt32 getNext();
	// 現在位置のビットマップを得る
	ModUInt32 get();

	// 移動する(ModUInt32単位)
	void seek(ModSize offset_);

	// 終端か
	bool isEnd() { return m_bEndOfData; }

	// ビットをONする
	void on(ModUInt32 uiRowID_);
	// ビットをOFFする
	void off(ModUInt32 uiRowID__);

	// ビットマップ単位でビットを設定する
	void set(ModSize offset_, ModUInt32 b_);

	// 整合性検査
	void verify();

	// DirPageを設定する
	void setDirPage(PhysicalFile::PageID uiDirPageID_);
	// DirPageを開放する
	void unsetDirPage();

	// DIRページのページIDを得る
	PhysicalFile::PageID getDirPageID() { return m_cDirPage.getID(); }

private:
	// DIRページをattachする
	void attachDirPage(PhysicalFile::PageID uiPageID_);
	// ビットマップページをattachする
	void attachBitmapPage();
	
	// ファイル
	BitmapFile& m_cFile;
	
	// DIRページ
	DirPage	m_cDirPage;
	// Bitmapページ
	BitmapPage m_cBitmapPage;

	// 終端か
	bool m_bEndOfData;
	// 最初の取得か
	bool m_bFirstGet;

	// 今参照しているBitmapページのビット列
	ModUInt32* m_pBitset;
	// Bitmapページ内の位置
	ModSize m_uiOffset;

	// ビットマップページ内のビットマップ領域の大きさ
	const ModSize m_uiBitmapSize;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif // __SYDNEY_BITMAP_NORMALBITMAPITERATOR_H

//
//	Copyright (c) 2007, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
