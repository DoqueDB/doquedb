// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BitmapFile.h --
// 
// Copyright (c) 2005, 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_BITMAPFILE_H
#define __SYDNEY_BITMAP_BITMAPFILE_H

#include "Bitmap/Module.h"
#include "Bitmap/BtreeFile.h"
#include "Bitmap/FileID.h"

#include "Common/DataArrayData.h"

#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class HeaderPage;
class BitmapIterator;

//
//	CLASS
//	Bitmap::BitmapFile --
//
//	NOTES
//
//
class BitmapFile : public BtreeFile
{
public:
	// コンストラクタ
	BitmapFile(const FileID& cFileID_);
	// デストラクタ
	virtual ~BitmapFile();

	// コピーを得る
	virtual BitmapFile* copy() = 0;

	// ファイル作成
	void create();

	// 挿入
	virtual void insert(const Common::DataArrayData& cTuple_);
	// 削除
	virtual void expunge(const Common::DataArrayData& cTuple_);

	// 整合性チェック
	virtual void verify() = 0;

	// 全ページをフラッシュする
	void flushAllPages();
	// 全ページを元に戻す
	void recoverAllPages();

	// ヘッダーページを得る
	HeaderPage& getHeaderPage();

	// ページサイズ
	Os::Memory::Size getPageSize() const
	{
		return static_cast<Os::Memory::Size>(m_cFileID.getPageSize());
	}

	// ビットマップイテレータを得る
	virtual BitmapIterator* getIterator() = 0;
	virtual BitmapIterator* getIterator(Common::Data& cKey_) = 0;

	// (GroupBy用) ビットマップイテレータを得る
	virtual BitmapIterator*
	getIteratorForGroupBy(const Common::Data& cValue_) = 0;
	
protected:
	// ヘッダーページを初期化する
	virtual void initializeHeaderPage();

	// 引数のタプルをチェックする
	bool checkArgument(const Common::DataArrayData& cTuple_,
					   ModUInt32& uiRowID_);
	// ビットをONする
	virtual void on(const Common::Data& cKey_, ModUInt32 uiRowID_,
					bool isNull_ = false) = 0;
	// ビットをOFFする
	virtual void off(const Common::Data& ckey_, ModUInt32 uiRowID_,
					 bool isNull_ = false) = 0;
	
	// FileID
	const FileID& m_cFileID;

private:
	// ヘッダーページ
	HeaderPage* m_pHeaderPage;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_FILE_H

//
//	Copyright (c) 2005, 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
