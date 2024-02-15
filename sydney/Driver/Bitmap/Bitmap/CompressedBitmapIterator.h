// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CompressedBitmapIterator.h -- バリュー部分を得るイテレータ
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

#ifndef __SYDNEY_BITMAP_COMPRESSEDBITMAPITERATOR_H
#define __SYDNEY_BITMAP_COMPRESSEDBITMAPITERATOR_H

#include "Bitmap/Module.h"
#include "Bitmap/BitmapIterator.h"

#include "Common/Data.h"

#include "PhysicalFile/File.h"
#include "PhysicalFile/DirectArea.h"

#include "Trans/Transaction.h"

#include "ModTypes.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class CompressedBitmapFile;

//
//	CLASS
//	Bitmap::CompressedBitmapIterator -- バリュー部分を得るイテレータ
//
//	NOTES
//
class CompressedBitmapIterator : public BitmapIterator
{
public:
	//
	//	ENUM
	//
	struct Result
	{
		enum Value
		{
			Success,		// 成功
			Modify,			// 成功したが、エリアが変更になった
			NeedConvert,	// より大きな構造を持つもに変更が必要
			Deleted			// エリア内の件数が0になり削除した
		};
	};

	//
	//	ENUM
	//
	struct Type
	{
		enum Value
		{
			Short = 1,		// 短いリスト
			Middle = 2,		// 中間のリスト
			Long = 3		// 長いリスト
		};
	};
	
	// コンストラクタ
	CompressedBitmapIterator(CompressedBitmapFile& cFile_);
	// デストラクタ
	virtual ~CompressedBitmapIterator();

	// タイプを得る
	static Type::Value getType(const PhysicalFile::DirectArea& cArea_);

	// 初期化する
	virtual void initialize(ModUInt32 uiRowID_) = 0;

	// エリアを設定する
	virtual void setArea(const PhysicalFile::DirectArea& cArea_) = 0;
	// エリアを得る
	virtual const PhysicalFile::DirectArea& getArea() const = 0;

	// ビットをonする
	virtual Result::Value on(ModUInt32 uiRowiD_) = 0;
	// ビットをoffする
	virtual Result::Value off(ModUInt32 uiRowID_) = 0;

	// より大きな構造のものへ変換する
	virtual CompressedBitmapIterator* convert(ModUInt32 uiRowID) = 0;
	// データを分割する
	virtual CompressedBitmapIterator* split();

	// 整合性検査を行う
	virtual void verify() = 0;

	// トランザクションを得る
	const Trans::Transaction& getTransaction() const;

	// FixModeを得る
	Buffer::Page::FixMode::Value getFixMode() const;

	// 最大エリアサイズを得る
	PhysicalFile::AreaSize getMaxStorableAreaSize() const;

	// 先頭のROWIDを得る
	virtual ModUInt32 getFirstRowID();
	// 最後のROWIDを得る
	virtual ModUInt32 getLastRowID();

protected:
	// 圧縮ファイル
	CompressedBitmapFile& m_cFile;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif // __SYDNEY_BITMAP_COMPRESSEDBITMAPITERATOR_H

//
//	Copyright (c) 2007, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
