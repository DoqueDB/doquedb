// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchGroupBy.h --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_SEARCHGROUPBY_H
#define __SYDNEY_BITMAP_SEARCHGROUPBY_H

#include "Bitmap/Module.h"
#include "Utility/OpenMP.h"

#include "Bitmap/LogicalInterface.h"
#include "Bitmap/FileID.h"
#include "Bitmap/BitmapFile.h"

#include "Common/BitSet.h"

#include "Os/CriticalSection.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

//
//	CLASS
//	Bitmap::SearchGroupBy
//
//	NOTES
//	OpenMPで並列処理を行い group by を処理する
//
class SearchGroupBy : public Utility::OpenMP
{
public:
	// コンストラクタ
	SearchGroupBy(FileID& cFileID_,
				  BitmapFile* pBitmapFile_,
				  ModUInt32 last_,
				  const Common::BitSet* pNarrowingBitSet,
				  LogicalInterface::BitSetMap& cResult_);
	// デストラクタ
	virtual ~SearchGroupBy();

	// 準備
	void prepare();
	// マルチスレッドで実行するメソッド
	void parallel();
	// 後処理
	void dispose();

private:
	// FileID
	FileID& m_cFileID;
	// ビットマップファイル
	BitmapFile* m_pBitmapFile;
	// 最大のROWIDのUnitポジション
	ModUInt32 m_last;

	// スレッド数分のバリュー
	ModVector<Common::Data::Pointer> m_vecpValue;

	// 絞り込みのためのビットマップ
	const Common::BitSet* m_pNarrowingBitSet;

	// group by の結果
	LogicalInterface::BitSetMap& m_cResult;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_SEARCHGROUPBY_H

//
//  Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
