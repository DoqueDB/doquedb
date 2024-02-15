// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortBitmapIteraor.h -- 
// 
// Copyright (c) 2007, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_SHORTBITMAPITERATOR_H
#define __SYDNEY_BITMAP_SHORTBITMAPITERATOR_H

#include "Bitmap/Module.h"
#include "Bitmap/CompressedBitmapIterator.h"
#include "Bitmap/IDBlock.h"

#include "Trans/Transaction.h"

#include "PhysicalFile/File.h"
#include "PhysicalFile/DirectArea.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class MiddleBitmapIterator;

//
//	CLASS
//	Bitmap::ShortBitmapIterator -- 圧縮されたビット列を管理する短いリスト
//
class ShortBitmapIterator : public CompressedBitmapIterator
{
	friend class MiddleBitmapIterator;
	
	//
	//	CLASS
	//	Bitmap::ShortBitmapIterator::Area
	//
	class Area
	{
	public:
		struct Header
		{
			ModUInt32	m_uiType;		// 先頭の4バイトはタイプ
			ModUInt32	m_uiLastRowID;
		};
		
		Area() {}
		~Area() {}

		// サイズを得る
		ModSize getSize() const;
		
		// 最終ROWIDを得る
		ModUInt32 getLastRowID() const;
		// 最終ROWIDを設定する
		void setLastRowID(ModUInt32 uiRowID_);

		// 先頭のIDBlockを得る
		IDBlock* begin();
		const IDBlock* begin() const;
		// 終端のIDBlockを得る
		IDBlock* end();
		const IDBlock* end() const;

		void dirty() { m_cArea.dirty(); }

		// エリア
		PhysicalFile::DirectArea m_cArea;
	};
	
public:
	// コンストラクタ
	ShortBitmapIterator(CompressedBitmapFile& cFile_);
	// デストラクタ
	virtual ~ShortBitmapIterator();

	// 現在位置のビットマップを得て、次の位置に進む
	ModUInt32 getNext();
	// 現在位置のビットマップを得る
	ModUInt32 get()
	{
		if (m_pIDBlock == 0) next();
		return m_uiBitmap;
	}

	// 移動する(ModUInt32単位)
	void seek(ModSize offset_);

	// 終端か
	bool isEnd() { return (m_pIDBlock && m_uiBitmap == 0 && m_pIDBlock == m_pEndIDBlock); }

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

	// ミドルリストへ変換する
	CompressedBitmapIterator* convert(ModUInt32 uiRowID_);
	// 分割する
	CompressedBitmapIterator* split();

	// 整合性検査
	void verify();

	// 先頭ROWIDを得る
	ModUInt32 getFirstRowID();
	// 最終ROWIDを得る
	ModUInt32 getLastRowID() { return m_cArea.getLastRowID(); }

private:
	// 次の位置に進む
	void next();
	// 最後のIDBlockにappendする
	Result::Value append(ModUInt32 uiRowiD_);
	// ブロックを分割する
	IDBlock* split(IDBlock* pIDBlock_, ModUInt32 uiRowID_);
	// エリアを拡張する
	bool expand();
	// エリアを縮める
	void reduce(IDBlock* pPosition_);

	Area m_cArea;
	
	// 現在のビットマップ
	ModUInt32 m_uiBitmap;
	// 現在のROWIDの上限
	ModUInt32 m_uiCurrentMax;

	// 現在位置のIDBlock
	const IDBlock* m_pIDBlock;
	// IDBlockの終端
	const IDBlock* m_pEndIDBlock;
	// 現在のROWID
	ModUInt32 m_uiLastRowID;
	// 現在のオフセット
	ModSize m_uiOffset;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif // __SYDNEY_BITMAP_SHORTBITMAPITERATOR_H

//
//	Copyright (c) 2007, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
