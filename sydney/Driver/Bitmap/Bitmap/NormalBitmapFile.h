// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalBitmapFile.h --
// 
// Copyright (c) 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_NORMALBITMAPFILE_H
#define __SYDNEY_BITMAP_NORMALBITMAPFILE_H

#include "Bitmap/Module.h"
#include "Bitmap/BitmapFile.h"
#include "Bitmap/FileID.h"

#include "Common/DataArrayData.h"

#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class HeaderPage;
class BitmapIterator;

//
//	CLASS
//	Bitmap::NormalBitmapFile --
//
//	NOTES
//
//
class NormalBitmapFile : public BitmapFile
{
public:
	// コンストラクタ
	NormalBitmapFile(const FileID& cFileID_);
	// デストラクタ
	virtual ~NormalBitmapFile();

	// コピーする
	BitmapFile* copy();

	// 整合性チェック
	void verify();

	// ビットマップイテレータを得る
	BitmapIterator* getIterator();
	BitmapIterator* getIterator(Common::Data& cKey_);
	
	// (GroupBy用) ビットマップイテレータを得る
	BitmapIterator* getIteratorForGroupBy(const Common::Data& cValue_);
	
protected:
	// ビットをONする
	void on(const Common::Data& cKey_, ModUInt32 uiRowID_,
			bool isNull_ = false);
	// ビットをOFFする
	void off(const Common::Data& ckey_, ModUInt32 uiRowID_,
			 bool isNull_ = false);
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_NORMALBITMAPFILE_H

//
//	Copyright (c) 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
