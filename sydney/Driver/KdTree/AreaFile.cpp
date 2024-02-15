// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaFile.cpp -- 多次元ベクトルを格納するファイル
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "KdTree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "KdTree/AreaFile.h"

#include "PhysicalFile/File.h"
#include "PhysicalFile/DirectArea.h"

#include "Os/Memory.h"

#include "Exception/BadArgument.h"
#include "Exception/VerifyAborted.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
	// パス
	Os::Path _cSubPath("Data");
}

//
//	FUNCTION public
//	KdTree::AreaFile::AreaFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::FileID& cFileID_
//		ファイルID
//	const Os::Path& cPath_
//		パス
//	bool bBatch_
//		バッチモードか否か
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
AreaFile::AreaFile(KdTree::FileID& cFileID_, const Os::Path& cPath_)
	: SubFile(cFileID_, PhysicalFile::DirectAreaType,
			  cFileID_.getPageSize(), Os::Path(cPath_).addPart(_cSubPath))
{
}

//
//	FUNCTION public
//	KdTree::AreaFile::~AreaFile -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
AreaFile::~AreaFile()
{
}

//
//	FUNCTION public
//	KdTree::AreaFile::recoverAllPages -- ページの更新を破棄する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
AreaFile::recoverAllPages()
{
	m_pPhysicalFile->recoverAllAreas();
}

//
//	FUNCTION public
//	KdTree::AreaFile::flushAllPages -- ページの更新を反映する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
AreaFile::flushAllPages()
{
	m_pPhysicalFile->detachAllAreas();
}

//
//	FUNCTION public
//	KdTree::AreaFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	const Common::AreaArrayArea& cValue_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
PhysicalFile::DirectArea::ID
AreaFile::insert(const Entry& cEntry_)
{
	ModSize size = cEntry_.m_iDimensionSize * sizeof(float);
	PhysicalFile::DirectArea area = allocateArea(size);

	// 多次元データ部分のみをコピーする

	Os::Memory::copy(area.operator void*(), cEntry_.m_pValue, size);
	area.dirty();
	
	return area.getID();
}

//
//	FUNCTION public
//	KdTree::AreaFile::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::DirectArea::ID& id_
//		削除するエリアのエリアID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
AreaFile::expunge(const PhysicalFile::DirectArea::ID& id_)
{
	// エリアを削除する
	m_pPhysicalFile->freeArea(*m_pTransaction, id_);
}

//
//	FUNCTION public
//	KdTree::AreaFile::get -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::DirectArea::ID id_
//		OID
//	KdTree::Entry& cEntry_
//		取得した値を格納するデータ型
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
AreaFile::get(const PhysicalFile::DirectArea::ID& id_,
			  Entry& cEntry_)
{
	// エリアをを得る
	PhysicalFile::DirectArea area = getArea(id_);
	
	ModSize size = area.getSize();
	
	// エリアの内容をコピーする
	Os::Memory::copy(cEntry_.m_pValue, area.operator const void*(), size);
}

//
//	FUNCTION public
//	KdTree::AreaFile::get -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::DirectArea::ID id_
//		OID
//	PhysicalFile::DirectArea& cArea_
//		値が格納されているエリア
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
PhysicalFile::DirectArea
AreaFile::getArea(const PhysicalFile::DirectArea::ID& id_)
{
	// エリアをattachし、サイズを得る
	PhysicalFile::DirectArea area = attachArea(id_);
	if (area.isValid() == false)
		_SYDNEY_THROW0(Exception::BadArgument);

	return area;
}

//
//	FUNCTION public
//	KdTree::AreaFile::clear -- 中身を空にする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
AreaFile::clear()
{
	// 変更を破棄する
	recoverAllPages();
	// 中身を空にする
	m_pPhysicalFile->clear(*m_pTransaction, false);
}

//
//	FUNCTION public
//	KdTree::AreaFile::move -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cNewPath_
//		移動先のパス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
AreaFile::move(const Trans::Transaction& cTransaction_,
			   const Os::Path& cNewPath_)
{
	// サブパスをappendして、親クラスを呼び出す

	SubFile::move(cTransaction_, Os::Path(cNewPath_).addPart(_cSubPath));
}

//
//	FUNCTION private
//	KdTree::AreaFile::attachArea -- エリアをattachする
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::DirectArea::ID& id_
//		attachするエリアのエリアID
//
//	RETURN
//	PhysicalFile:DirectArea
//		attachしたエリア
//
//	EXCEPTIONS
//
PhysicalFile::DirectArea
AreaFile::attachArea(const PhysicalFile::DirectArea::ID& id_)
{
	if (m_bVerification)
	{
		Admin::Verification::Progress cProgress(
			m_pProgress->getConnection());
		PhysicalFile::DirectArea area =
			m_pPhysicalFile->verifyArea(*m_pTransaction,
										id_,
										m_eFixMode,
										cProgress);
		*m_pProgress += cProgress;
		if (cProgress.isGood() == false)
		{
			area.detach();
			_SYDNEY_THROW0(Exception::VerifyAborted);
		}

		return area;
	}
	else
	{
		return m_pPhysicalFile->attachArea(*m_pTransaction,
										   id_,
										   m_eFixMode);
	}
}

//
//	FUNCTION private
//	KdTree::AreaFile::allocateArea -- エリアをallocateする
//
//	NOTES
//
//	ARGUMENTS
//	ModSize size_
//		allocateするエリアのサイズ
//
//	RETURN
//	PhysicalFile::DirectArea
//		allocateしたエリア
//
//	EXCEPTIONS
//
PhysicalFile::DirectArea
AreaFile::allocateArea(ModSize size_)
{
	return m_pPhysicalFile->allocateArea(*m_pTransaction,
										 size_);
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
