// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IDBlock.h --
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

#ifndef __SYDNEY_BITMAP_IDBLOCK_H
#define __SYDNEY_BITMAP_IDBLOCK_H

#include "Bitmap/Module.h"

#include "ModTypes.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

//
//	CLASS
//	Bitmap::IDBlock --
//
//	NOTES
//	圧縮されたROWIDの１ブロックをあらわすクラス。今の実装は全部で24バイト。
//
//	【注意】
//	このクラスはページのバッファから直接キャストされるので、
//	virtual メソッドは定義してはならない。
//
class IDBlock
{
public:

	// 圧縮されたROWIDを格納するバッファのサイズ
	enum { BLOCK_SIZE = 4 };

	//
	//	CLASS
	//	Bitmap::IDBlock::Less --
	//
	//	NOTES
	//
	class Less
	{
	public:
		ModBoolean operator () (const IDBlock& c1, const IDBlock& c2)
			{
				return (c1.getFirstID() < c2.getFirstID()) ? ModTrue : ModFalse;
			}
	};
	
	// コンストラクタ
	IDBlock() {}
	IDBlock(ModUInt32 uiFirstRowID_);
	// デストラクタ
	~IDBlock() {}

	// 初期化する
	void initialize(ModUInt32 uiFirstRowID_);

	// 先頭のROWIDを得る
	ModUInt32 getFirstID() const
		{ return m_uiFirstRowID; }
	// 最終のROWIDを得る
	ModUInt32 getLastID() const;

	// 登録数を得る
	ModSize getCount() const
		{ return static_cast<ModSize>(m_usCount); }

	// ビット数を得る
	ModSize getBitLength() const
		{ return static_cast<ModSize>(m_usBitLength); }

	// 指定されたオフセットのROWIDを取得する
	// 引数 uiOffset_ には次のエントリのオフセットが設定される
	ModUInt32 getID(ModSize uiLastID_, ModSize& uiOffset_) const;

	// ROWIDを挿入する。領域が不足して格納できない場合はfalseを返す
	bool insert(ModUInt32 uiRowID_);
	// ROWIDを末尾に追加する。領域が不足して格納できない場合はfalseを返す
	bool append(ModUInt32 uiLastID_, ModUInt32 uiRowID_);
	// ROWIDを削除する。登録されていないROWIDの場合はfalseを返す
	bool erase(ModUInt32 uiRowID_, ModUInt32& uiPrevID_);

	// 自身のエントリの後ろ半分を分け与える
	void split(IDBlock* pDst_);
	// 引数ブロックの内容をすべて自身のエントリとして追加する
	// 領域が不足して追加できない場合はfalseを返す
	bool merge(const IDBlock* pSrc_);

	// 最大ビット長を得る
	static ModSize getMaxBitLength();

private:
	// ビット単位のmove
	static void move(ModUInt32* pDst_, ModSize uiDstBegin_,
					 const ModUInt32* pSrc_, ModSize uiSrcBegin_,
					 ModSize uiSize_);
	// ビット単位のreset
	static void reset(ModUInt32* pBuffer_, ModSize uiBegin_, ModSize uiSize_);
	// ビット長を得る
	static ModSize getBitLength(ModUInt32 uiLastRowID_, ModUInt32 uiRowID_);
	// ビットを書き出す
	static void write(ModUInt32 uiLastRowID_, ModUInt32 uiRowID_,
					  ModUInt32* pBuffer_, ModSize& uiOffset_);
	// ビットを読み出す
	static ModUInt32 read(ModUInt32 uiLastRowID_,
						  const ModUInt32* pBuffer_,
						  ModSize uiBitLength_, ModSize& uiOffset_);
	
	// 先頭ROWID
	ModUInt32 m_uiFirstRowID;
	// 登録数
	unsigned short m_usCount;
	// 利用ビット数
	unsigned short m_usBitLength;
	
	// 圧縮されたROWIDを格納する領域
	ModUInt32 m_pBuffer[BLOCK_SIZE];
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif // __SYDNEY_BITMAP_IDBLOCK_H

//
//	Copyright (c) 2007, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
