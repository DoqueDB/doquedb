// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaVectorFile.h -- ベクターファイル
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_AREAVECTORFILE_H
#define __SYDNEY_KDTREE_AREAVECTORFILE_H

#include "KdTree/Module.h"
#include "KdTree/VectorFile.h"

#include "Common/BitSet.h"
#include "Common/LargeVector.h"

#include "KdTree/FileID.h"

#include "Version/Page.h"

#include "Os/Path.h"

#include "PhysicalFile/DirectArea.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

//	CLASS
//	KdTree::AreaVectorFile -- ベクターファイル
//
//	NOTES
//
class AreaVectorFile : public VectorFile
{
public:
	//コンストラクタ
	AreaVectorFile(FileID& cFileID_, const Os::Path& cPath_);
	//デストラクタ
	virtual ~AreaVectorFile();

	// 挿入
	void insert(ModUInt32 uiRowID_, PhysicalFile::DirectArea::ID& id_);
	// 削除
	void expunge(ModUInt32 uiRowID_);

	// 取得
	bool get(ModUInt32 uiRowID_, PhysicalFile::DirectArea::ID& id_);
	// 全件取得
	void getAll(Common::BitSet& cBitSet_);

	// 指定ページ内のキーとバリューを得る
	void getPageData(Version::Page::ID pageID_,
					 Common::LargeVector<
					 	ModPair<ModUInt32, PhysicalFile::DirectArea::ID> >&
					 vecData_);

	//ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cPath_);

	// 1ページに格納できるエントリ数を得る
	ModSize getCountPerPage() { return m_uiCountPerPage; }

protected:
	// 格納領域のメモリを得る
	char* getBuffer(ModUInt32 uiRowID_);
	const char* getConstBuffer(ModUInt32 uiRowID_);

	// キー値からページIDとオフセットを得る
	Version::Page::ID convertToPageID(ModUInt32 uiKey_, int& offset_) const
	{
		Version::Page::ID id = uiKey_ / m_uiCountPerPage + 1;
		offset_ = static_cast<int>(
			(uiKey_ % m_uiCountPerPage) * m_uiElementSize);
		return id;
	}

	// 1ページに格納できるエントリ数
	ModSize m_uiCountPerPage;
	// エントリのサイズ
	ModSize m_uiElementSize;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_AREAVECTORFILE_H

//
//	Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
