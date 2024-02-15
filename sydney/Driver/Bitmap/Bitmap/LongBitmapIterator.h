// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LongBitmapIteraor.h -- 
// 
// Copyright (c) 2007, 2012, 2017, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_LONGBITMAPITERATOR_H
#define __SYDNEY_BITMAP_LONGBITMAPITERATOR_H

#include "Bitmap/Module.h"
#include "Bitmap/CompressedBitmapIterator.h"
#include "Bitmap/IDBlock.h"
#include "Bitmap/NormalBitmapIterator.h"

#include "Trans/Transaction.h"

#include "PhysicalFile/File.h"
#include "PhysicalFile/DirectArea.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class MiddleBitmapIterator;
class ShortBitmapIterator;

//
//	CLASS
//	Bitmap::LongBitmapIterator -- 圧縮されたビット列を管理する短いリスト
//
class LongBitmapIterator : public CompressedBitmapIterator
{
	friend class MiddleBitmapIterator;
	friend class ShortBitmapIterator;
	
	//
	//	CLASS
	//	Bitmap::LongBitmapIterator::Area
	//
	class Area
	{
	public:
		struct Header
		{
			ModUInt32				m_uiType;		// 先頭の4バイトはタイプ
		};
		
		Area() {}
		~Area() {}

		// サイズを得る
		ModSize getSize() const;
		
		// 先頭のPageIDを得る
		PhysicalFile::PageID* begin();
		const PhysicalFile::PageID* begin() const;
		// 終端のPageIDを得る
		PhysicalFile::PageID* end();
		const PhysicalFile::PageID* end() const;

		// dirty
		void dirty() { m_cArea.dirty(); }

		// エリア
		PhysicalFile::DirectArea m_cArea;
	};
	
public:
	// コンストラクタ
	LongBitmapIterator(CompressedBitmapFile& cFile_);
	// デストラクタ
	virtual ~LongBitmapIterator();

	// 現在位置のビットマップを得て、次の位置に進む
	ModUInt32 getNext();
	// 現在位置のビットマップを得る
	ModUInt32 get();

	// 移動する(ModUInt32単位)
	void seek(ModSize offset_);

	// 終端か
	bool isEnd() { return m_bEndOfData; }

	// 初期化する
	void initialize(ModUInt32 uiRowID_);

	// エリアを設定する
	void setArea(const PhysicalFile::DirectArea& cArea_);
	// エリアを得る
	const PhysicalFile::DirectArea& getArea() const
		{ return m_cArea.m_cArea; }

	// ビットをONする
	Result::Value on(ModUInt32 uiRowID_);
	// ビットをOFFする
	Result::Value off(ModUInt32 uiRowID_);

	// この形式が最長なので例外が発生する
	CompressedBitmapIterator* convert(ModUInt32 uiRowID_);

	// 整合性検査
	void verify();

private:
	// 移動する
	Result::Value seek(ModSize offset_, bool bUpdate_);
	// 初期化する
	void initialize(MiddleBitmapIterator* i);

	// エリアを初期化する
	void initializeArea(ModUInt32 uiMaxRowID_);
	// ショートリストの内容をコピーする
	void insert(ShortBitmapIterator* i);
	// ビットマップ単位(ModUInt32単位)でビットを設定する
	void set(ModSize offset_, ModUInt32 b_);
	
	// NormalBitmapIterator
	NormalBitmapIterator m_cIterator;

	// エリア
	Area m_cArea;
	// 現在のページID
	const PhysicalFile::PageID* m_pPageID;

	// 終端か
	bool m_bEndOfData;
	// 最初の取得か
	bool m_bFirstGet;

	// ModUInt32単位の位置
	ModSize m_uiOffset;

	// ビットマップページ内のビットマップ領域の大きさ
	const ModSize m_uiBitmapSize;
	// DIRページ内に格納できるPageIDの数
	const ModSize m_uiDirCount;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif // __SYDNEY_BITMAP_LONGBITMAPITERATOR_H

//
//	Copyright (c) 2007, 2012, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
