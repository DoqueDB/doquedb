// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VectorDataFile.cpp --
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
#include "KdTree/VectorDataFile.h"

#include "KdTree/Allocator.h"
#include "KdTree/AreaFile.h"

#include "Common/Message.h"

#include "Os/Math.h"

#include "Exception/BadArgument.h"
#include "Exception/Unexpected.h"

#include "Schema/File.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
}

//
//	FUNCTION public
//	KdTree::VectorDataFile::VectorDataFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
VectorDataFile::VectorDataFile(FileID& cFileID_, const Os::Path& cPath_)
	: DataFile(cFileID_, cPath_),
	  m_pAreaFile(0), m_pVectorFile(0)
{
	attach();
}

//
//	FUNCTION public
//	KdTree::VectorDataFile::~VectorDataFile -- デストラクタ
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
VectorDataFile::~VectorDataFile()
{
	if (isOpened()) close();
	detach();
}

//
//	FUNCTION public
//	KdTree::VectorDataFile::verify -- 整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Admin::Verification::Treatment::Value eTreatment_
//		対応指示
//	Admin::Verification::Progress& cProgress_
//		経過報告
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorDataFile::verify(const Trans::Transaction& cTransaction_,
					   const Admin::Verification::Treatment::Value eTreatment_,
					   Admin::Verification::Progress& cProgress_)
{
	Admin::Verification::Progress cProgress(cProgress_.getConnection());
	
	// 開始
	startVerification(cTransaction_, eTreatment_, cProgress);

	try
	{
		// ベクターファイルを参照して、そのすべてがデータファイルに
		// 格納されているか確認する

		PhysicalFile::PageID uiPageID = 0;

		while ((uiPageID = getNextPageID(uiPageID)) != DataFile::IllegalPageID)
		{
			Common::LargeVector<ModPair<
				ModUInt32, PhysicalFile::DirectArea::ID> > vecVectorData;

			// ベクターファイルから指定ページに格納されている
			// すべてのデータを取得する
			m_pVectorFile->getPageData(uiPageID, vecVectorData);

			// 得られたデータを走査し、エリアファイルからエントリを取得する
			Common::LargeVector<ModPair<
				ModUInt32, PhysicalFile::DirectArea::ID> >::Iterator i
				= vecVectorData.begin();
			Common::LargeVector<ModPair<
				ModUInt32, PhysicalFile::DirectArea::ID> >::Iterator e
				= vecVectorData.end();
			for (; i != e; ++i)
			{
				// エリアファイルから読み込む
				m_pAreaFile->getArea((*i).second);
			}
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// 整合性検査を終了する
		endVerification();
		_SYDNEY_RETHROW;
	}

	// 整合性検査を終了する
	endVerification();

	// 経過を追加する
	cProgress_ += cProgress;
}

//
//	FUNCTION public
//	KdTree::VectorDataFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::Entry& cEntry_
//		多次元ベクトル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorDataFile::insert(const Entry& cEntry_)
{
	// まず、多次元ベクトルをAreaFileに登録する
	PhysicalFile::DirectArea::ID areaID = m_pAreaFile->insert(cEntry_);
	// 次にベクターに登録する
	m_pVectorFile->insert(cEntry_.getID(), areaID);
}

//
//	FUNCTION public
//	KdTree::VectorDataFile::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorDataFile::expunge(ModUInt32 uiRowID_)
{
	// ベクターからAreaIDを得る
	PhysicalFile::DirectArea::ID areaID;
	if (m_pVectorFile->get(uiRowID_, areaID) == false)
	{
		// 存在しないものを削除している
		// 索引なので、メッセージを出力するだけにする
		
		SydErrorMessage << "entry not found. rowid = " << uiRowID_ << ModEndl;
		return;
	}

	// ベクターから削除する
	m_pVectorFile->expunge(uiRowID_);
	// エリアを削除する
	m_pAreaFile->expunge(areaID);
}

//
//	FUNCTION public
//	KdTree::VectorDataFile::get -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	ModVector<float>& vecValue_
//		多次元ベクトル
//
//	RETURN
//	bool
//		存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
VectorDataFile::get(ModUInt32 uiRowID_, Entry& cEntry_)
{
	// ベクターを検索する
	PhysicalFile::DirectArea::ID areaID;
	if (m_pVectorFile->get(uiRowID_, areaID) == false)
		return false;

	// 多次元ベクトルを得る
	m_pAreaFile->get(areaID, cEntry_);

	// ROWIDを設定する
	cEntry_.m_uiID = uiRowID_;

	return true;
}

//
//	FUNCTION public
//	KdTree::VectorDataFile::test -- 挿入されているか検査する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//
//	RETURN
//	bool
//		挿入されている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
VectorDataFile::test(ModUInt32 uiRowID_)
{
	// ベクターを検索する
	PhysicalFile::DirectArea::ID areaID;
	if (m_pVectorFile->get(uiRowID_, areaID) == false)
		return false;

	return true;
}
	
//
//	FUNCTION public
//	KdTree::VectorDataFile::getAll -- 全数検索する
//
//	NOTES
//
//	ARGUMENTS
//	Common::BitSet& cBitSet_
//		検索結果
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorDataFile::getAll(Common::BitSet& cBitSet_)
{
	m_pVectorFile->getAll(cBitSet_);
}

//
//	FUNCTION public
//	KdTree::VectorDataFile::getPageData
//		-- 指定されたベクターのページIDに格納されているエントリを得る
//
//	NOTES
//
//	ARGUMENTS
//	DataFile::PageID uiPageID_
//		ページID
//	KdTree::Allocator& allocator_
//		アロケータ
//	Common::LargeVector<Entry*>& vecpEntry_
//		呼び出したエントリ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorDataFile::getPageData(DataFile::PageID uiPageID_,
							Allocator& allocator_,
							Common::LargeVector<Entry*>& vecpEntry_)
{
	Common::LargeVector<ModPair<ModUInt32, PhysicalFile::DirectArea::ID> >
		vecVectorData;

	// ベクターファイルから指定ページに格納されているすべてのデータを取得する
	m_pVectorFile->getPageData(uiPageID_, vecVectorData);

	// 得られたデータを走査し、エリアファイルからエントリを取得する
	Common::LargeVector<ModPair<ModUInt32, PhysicalFile::DirectArea::ID> >
		::Iterator i = vecVectorData.begin();
	Common::LargeVector<ModPair<ModUInt32, PhysicalFile::DirectArea::ID> >
		::Iterator e = vecVectorData.end();
	for (; i != e; ++i)
	{
		// 領域を確保する
		Entry* pEntry = allocator_.allocateEntry();

		// エリアファイルから読み込む
		m_pAreaFile->get((*i).second, *pEntry);

		// ROWIDを設定する
		pEntry->m_uiID = (*i).first;

		// 引数に追加する
		vecpEntry_.pushBack(pEntry);
	}
}

//
//	FUNCTION public
//	KdTree::VectorDataFile::getNextPageID -- 次のページIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::DataFile::PageID uiCurrentPageID_
//		現在のページID。0 を渡すと先頭のページIDを返す
//
//	RETURN
//	KdTree::DataFile::PageID
//		次のページID。存在しない場合は DataFile::IllegalPageID
//
//	EXCEPTIONS
//
DataFile::PageID
VectorDataFile::getNextPageID(PageID uiCurrentPageID_)
{
	//【注意】	PageID の 0 と DataFile::IllegalPageID は
	//			特別な意味になるので、データ格納領域には利用できない
	
	++uiCurrentPageID_;
	return (uiCurrentPageID_ <= m_pVectorFile->getMaxPageID()) ?
		uiCurrentPageID_ : DataFile::IllegalPageID;
}

//
//	FUNCTION public
//	KdTree::VectorDataFile::copy -- コピーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	KdTree::DataFile*
//		コピー
//
//	EXCEPTIONS
//
DataFile*
VectorDataFile::copy()
{
	VectorDataFile* f = new VectorDataFile(m_cFileID, getPath());
	if (isOpened()) f->open(*m_pTransaction, m_eFixMode);
	return f;
}

//
//	FUNCTION public
//	KdTree::VectorDataFile::clear -- ファイルの内容をクリアする
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
VectorDataFile::clear()
{
	//【注意】	本ファイルはメイン索引でしか使われないので、
	//			clear メソッドが呼び出されることはない。そのため、未実装

	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	KdTree::VectorDataFile::attach -- ファイルをattachする
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
VectorDataFile::attach()
{
	ModAutoPointer<AreaFile> pAreaFile
		= new AreaFile(m_cFileID, getPath());
	ModAutoPointer<AreaVectorFile> pVectorFile
		= new AreaVectorFile(m_cFileID, getPath());

	// MultiFileに登録する
	reserveSubFile(2);
	pushBackSubFile(pAreaFile.get());
	pushBackSubFile(pVectorFile.get());

	// メンバー変数に設定する
	m_pAreaFile = pAreaFile.release();
	m_pVectorFile = pVectorFile.release();
}

//
//	FUNCTION private
//	KdTree::VectorDataFile::detach -- ファイルをdetachする
//
// 	NOTES
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
VectorDataFile::detach()
{
	clearSubFile();

	delete m_pAreaFile, m_pAreaFile = 0;
	delete m_pVectorFile, m_pVectorFile = 0;
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
