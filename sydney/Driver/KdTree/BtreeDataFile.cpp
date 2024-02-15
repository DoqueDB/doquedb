// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreeDataFile.cpp --
// 
// Copyright (c) 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
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
#include "KdTree/BtreeDataFile.h"

#include "KdTree/Allocator.h"
#include "KdTree/AreaFile.h"
#include "KdTree/BtreeFile.h"
#include "KdTree/IDVectorFile.h"

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
//	KdTree::BtreeDataFile::BtreeDataFile -- コンストラクタ
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
BtreeDataFile::BtreeDataFile(FileID& cFileID_, const Os::Path& cPath_)
	: DataFile(cFileID_, cPath_),
	  m_pAreaFile(0), m_pBtreeFile(0)
{
	attach();
}

//
//	FUNCTION public
//	KdTree::BtreeDataFile::~BtreeDataFile -- デストラクタ
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
BtreeDataFile::~BtreeDataFile()
{
	if (isOpened()) close();
	detach();
}

//
//	FUNCTION public
//	KdTree::BtreeDataFile::verify -- 整合性検査を行う
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
BtreeDataFile::verify(const Trans::Transaction& cTransaction_,
					  const Admin::Verification::Treatment::Value eTreatment_,
					  Admin::Verification::Progress& cProgress_)
{
	Admin::Verification::Progress cProgress(cProgress_.getConnection());
	
	// 開始
	startVerification(cTransaction_, eTreatment_, cProgress);

	try
	{
		// B木ファイルを参照して、そのすべてがデータファイルに
		// 格納されているか確認する

		PhysicalFile::PageID uiPageID = 0;

		while ((uiPageID = getNextPageID(uiPageID)) != DataFile::IllegalPageID)
		{
			Common::LargeVector<ModPair<
				ModUInt32, PhysicalFile::DirectArea::ID> >	vecBtreeData;

			// B木ファイルから指定ページに格納されている
			// すべてのデータを取得する
			m_pBtreeFile->getPageData(uiPageID, vecBtreeData);

			// 得られたデータを走査し、エリアファイルからエントリを取得する
			Common::LargeVector<ModPair<
				ModUInt32, PhysicalFile::DirectArea::ID> >::Iterator i
				= vecBtreeData.begin();
			Common::LargeVector<ModPair<
				ModUInt32, PhysicalFile::DirectArea::ID> >::Iterator e
				= vecBtreeData.end();
			for (; i != e; ++i)
			{
				// エリアファイルから読み込む
				m_pAreaFile->getArea((*i).second);
			}
		}

		// 削除ファイルの全件を取得する

		Common::BitSet cBitSet;
		m_pExpungeFile->getAllValue(cBitSet);
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
//	KdTree::BtreeDataFile::insert -- 挿入する
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
BtreeDataFile::insert(const Entry& cEntry_)
{
	// まず、多次元ベクトルをAreaFileに登録する
	PhysicalFile::DirectArea::ID areaID = m_pAreaFile->insert(cEntry_);
	// 次にB木に登録する
	m_pBtreeFile->insert(cEntry_.getID(), areaID);
}

//
//	FUNCTION public
//	KdTree::BtreeDataFile::expunge -- 削除する
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
BtreeDataFile::expunge(ModUInt32 uiRowID_)
{
	// B木からAreaIDを得る
	PhysicalFile::DirectArea::ID areaID;
	if (m_pBtreeFile->get(uiRowID_, areaID))
	{
		// ここに挿入されているものなので、削除する
		
		// B木から削除する
		m_pBtreeFile->expunge(uiRowID_);
		// エリアを削除する
		m_pAreaFile->expunge(areaID);
	}
	else
	{
		// ここにはないデータ -> メイン索引に存在するデータなので、
		// 削除フラグ用のベクターに挿入する

		m_pExpungeFile->pushBack(uiRowID_);
	}
}

//
//	FUNCTION public
//	KdTree::BtreeDataFile::get -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	ModBtree<float>& vecValue_
//		多次元ベクトル
//
//	RETURN
//	bool
//		存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
BtreeDataFile::get(ModUInt32 uiRowID_, Entry& cEntry_)
{
	// B木を検索する
	PhysicalFile::DirectArea::ID areaID;
	if (m_pBtreeFile->get(uiRowID_, areaID) == false)
		return false;

	// 多次元ベクトルを得る
	m_pAreaFile->get(areaID, cEntry_);

	// ROWIDを設定する
	cEntry_.m_uiID = uiRowID_;

	return true;
}

//
//	FUNCTION public
//	KdTree::BtreeDataFile::getAll -- 全数検索する
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
BtreeDataFile::getAll(Common::BitSet& cBitSet_)
{
	m_pBtreeFile->getAll(cBitSet_);
}

//
//	FUNCTION public
//	KdTree::BtreeDataFile::undoExpungedEntry -- 削除フラグから削除する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		削除フラグから削除するID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeDataFile::undoExpungedEntry(ModUInt32 uiRowID_)
{
	m_pExpungeFile->expunge(uiRowID_);
}

//
//	FUNCTION public
//	KdTree::BtreeDataFile::getExpungedEntry -- 削除済みエントリの一覧を得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::BitSet& cBitSet_
//		削除済みエントリのROWIDのビットセット
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeDataFile::getExpungedEntry(Common::BitSet& cBitSet_)
{
	//【注意】	引数の BitSet に追加される

	m_pExpungeFile->getAllValue(cBitSet_);
}

//
//	FUNCTION public
//	KdTree::BtreeDataFile::getExpungedEntryCount -- 削除済みエントリ数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		削除済みのエントリ数
//
//	EXCEPTIONS
//
ModUInt32
BtreeDataFile::getExpungedEntryCount()
{
	return m_pExpungeFile->getCount();
}

//
//	FUNCTION public
//	KdTree::BtreeDataFile::getPageData
//		-- 指定されたB木のページIDに格納されているエントリを得る
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
BtreeDataFile::getPageData(DataFile::PageID uiPageID_,
						   Allocator& allocator_,
						   Common::LargeVector<Entry*>& vecpEntry_)
{
	Common::LargeVector<ModPair<ModUInt32, PhysicalFile::DirectArea::ID> >
		vecBtreeData;

	// B木ファイルから指定ページに格納されているすべてのデータを取得する
	m_pBtreeFile->getPageData(uiPageID_, vecBtreeData);

	// 得られたデータを走査し、エリアファイルからエントリを取得する
	Common::LargeVector<ModPair<ModUInt32, PhysicalFile::DirectArea::ID> >
		::Iterator i = vecBtreeData.begin();
	Common::LargeVector<ModPair<ModUInt32, PhysicalFile::DirectArea::ID> >
		::Iterator e = vecBtreeData.end();
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
//	KdTree::BtreeDataFile::getNextPageID -- 次のページIDを得る
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
BtreeDataFile::getNextPageID(PageID uiCurrentPageID_)
{
	//【注意】	PageID の 0 と PhysicalFile::ConstValue::UndefinedPageID
	//			特別な意味になるので、データ格納領域には利用できない
	
	return m_pBtreeFile->getNextLeafPageID(uiCurrentPageID_);
}

//
//	FUNCTION public
//	KdTree::BtreeDataFile::copy -- コピーを得る
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
BtreeDataFile::copy()
{
	BtreeDataFile* f = new BtreeDataFile(m_cFileID, getPath());
	if (isOpened()) f->open(*m_pTransaction, m_eFixMode);
	return f;
}

//
//	FUNCTION public
//	KdTree::BtreeDataFile::clear -- ファイルの内容をクリアする
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
BtreeDataFile::clear()
{
	// PhysicalFile::DirectAreaFile の clear は実装されていないので、
	// ここですべてのエリアを解放する

	PhysicalFile::PageID uiCurrentPageID = 0;
	uiCurrentPageID = m_pBtreeFile->getNextLeafPageID(uiCurrentPageID);
	while (uiCurrentPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		Common::LargeVector<ModPair<ModUInt32, PhysicalFile::DirectArea::ID> >
			vecBtreeData;

		m_pBtreeFile->getPageData(uiCurrentPageID, vecBtreeData);

		Common::LargeVector<ModPair<ModUInt32, PhysicalFile::DirectArea::ID> >
			::Iterator i = vecBtreeData.begin();
		Common::LargeVector<ModPair<ModUInt32, PhysicalFile::DirectArea::ID> >
			::Iterator e = vecBtreeData.end();
		for (; i != e; ++i)
		{
			m_pAreaFile->expunge((*i).second);
		}

		uiCurrentPageID = m_pBtreeFile->getNextLeafPageID(uiCurrentPageID);
	}
	
	m_pBtreeFile->clear();
	m_pExpungeFile->clear();
}

//
//	FUNCTION private
//	KdTree::BtreeDataFile::attach -- ファイルをattachする
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
BtreeDataFile::attach()
{
	ModAutoPointer<AreaFile> pAreaFile
		= new AreaFile(m_cFileID, getPath());
	ModAutoPointer<BtreeFile> pBtreeFile
		= new BtreeFile(m_cFileID, getPath());
	ModAutoPointer<IDVectorFile> pExpungeFile
		= new IDVectorFile(m_cFileID, getPath());

	// MultiFileに登録する
	reserveSubFile(3);
	pushBackSubFile(pAreaFile.get());
	pushBackSubFile(pBtreeFile.get());
	pushBackSubFile(pExpungeFile.get());

	// メンバー変数に設定する
	m_pAreaFile = pAreaFile.release();
	m_pBtreeFile = pBtreeFile.release();
	m_pExpungeFile = pExpungeFile.release();
}

//
//	FUNCTION private
//	KdTree::BtreeDataFile::detach -- ファイルをdetachする
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
BtreeDataFile::detach()
{
	clearSubFile();

	delete m_pAreaFile, m_pAreaFile = 0;
	delete m_pBtreeFile, m_pBtreeFile = 0;
	delete m_pExpungeFile, m_pExpungeFile = 0;
}

//
//	Copyright (c) 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
