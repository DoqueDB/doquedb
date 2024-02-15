// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaFile.h -- 多次元ベクトルを格納するファイル
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

#ifndef __SYDNEY_KDTREE_AREAFILE_H
#define __SYDNEY_KDTREE_AREAFILE_H

#include "KdTree/Module.h"
#include "KdTree/SubFile.h"

#include "KdTree/Entry.h"
#include "KdTree/FileID.h"

#include "PhysicalFile/File.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

//	CLASS
//	KdTree::AreaFile	-- 多次元ベクトルを格納するファイル
//
//	NOTES
//
class AreaFile : public SubFile
{
public:
	//コンストラクタ
	AreaFile(FileID& cFileID_, const Os::Path& cPath_);
	//デストラクタ
	virtual ~AreaFile();

	// すべてのページの更新を確定する
	void flushAllPages();
	// すべてのページの更新を破棄する
	void recoverAllPages();

	// 挿入する
	PhysicalFile::DirectArea::ID insert(const Entry& cEntry_);
	// 削除する
	void expunge(const PhysicalFile::DirectArea::ID& id_);
	// 取得する
	void get(const PhysicalFile::DirectArea::ID& id_,
			 Entry& cEntry_);
	
	// エリアを得る
	PhysicalFile::DirectArea getArea(const PhysicalFile::DirectArea::ID& id_);

	// 中身を空にする
	void clear();

	// 最大エリアサイズを得る
	PhysicalFile::AreaSize getMaxStorableAreaSize() const
	{
		return m_pPhysicalFile->getMaxStorableAreaSize();
	}

	// ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cNewPath_);

private:
	// エリアを attach する
	PhysicalFile::DirectArea attachArea(
		const PhysicalFile::DirectArea::ID& id_);
	// エリアを allocate する
	PhysicalFile::DirectArea allocateArea(ModSize size_);

	// 物理ファイルをアタッチする
	void attach();
	// 物理ファイルをデタッチする
	void detach();
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_AREAFILE_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
