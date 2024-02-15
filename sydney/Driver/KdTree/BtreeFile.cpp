// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreeFile.cpp --
// 
// Copyright (c) 2013, 2014, 2017, 2023 Ricoh Company, Ltd.
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
#include "KdTree/BtreeFile.h"

#include "KdTree/Page.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "PhysicalFile/Types.h"
#include "Os/AutoCriticalSection.h"

#include "Exception/Cancel.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
	// パス
	Os::Path _cSubPath("Btree");
}

//
//	FUNCTION public
//	KdTree::BtreeFile::BtreeFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::FileID& cFileID_
//		ファイルID
//	const Os::Path& cPath_
//		パス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
BtreeFile::BtreeFile(FileID& cFileID_, const Os::Path& cPath_)
	: SubFile(cFileID_, PhysicalFile::PageManageType,
			  cFileID_.getPageSize(),
			  Os::Path(cPath_).addPart(_cSubPath)),
	  m_pHeaderPage(0), m_pFree(0)
{
}

//
//	FUNCTION public
//	KdTree::BtreeFile::~BtreeFile -- デストラクタ
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
BtreeFile::~BtreeFile()
{
}

//
//	FUNCTION public
//	KdTree::BtreeFile::getCount -- エントリ数を得る
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
ModUInt32
BtreeFile::getCount()
{
	ModUInt32 count = 0;
	if (isMounted())
		count = static_cast<ModUInt32>(getHeaderPage()->getCount());
	return count;
}

//
//	FUNCTION public
//	KdTree::BtreeFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	const PhysicalFile::DirectArea::ID& id_
//		エリアID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeFile::insert(ModUInt32 uiRowID_,
				  const PhysicalFile::DirectArea::ID& id_)
{
	// 挿入するためのデータを作成する
	BtreePage::Entry cEntry(uiRowID_, id_.m_uiPageID, id_.m_uiAreaID);
	
	// 挿入するリーフページを得る
	BtreePage::PagePointer pLeafPage = getLeafPage(&cEntry);

	if (pLeafPage == 0)
	{
		// ルートページがないので新たに確保する
		pLeafPage = allocatePage(PhysicalFile::ConstValue::UndefinedPageID,
								 PhysicalFile::ConstValue::UndefinedPageID,
								 BtreePage::PagePointer());
		pLeafPage->setLeaf();
		
		// ヘッダーページに設定する
		HeaderPage* pHeader = getHeaderPage();
		pHeader->setRootPageID(pLeafPage->getID());
		pHeader->setLeftPageID(pLeafPage->getID());
		pHeader->setRightPageID(pLeafPage->getID());
	}

	// 挿入する
	pLeafPage->insertEntry(&cEntry);
	
	// エントリ数を変更する
	getHeaderPage()->addCount();
}

//
//	FUNCTION public
//	KdTree::BtreeFile::expunge -- 削除する
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
BtreeFile::expunge(ModUInt32 uiRowID_)
{
	// 削除するデータを作成する
	BtreePage::Entry cEntry(uiRowID_, 0);
	// 削除するリーフページを得る
	BtreePage::PagePointer pLeafPage = getLeafPage(&cEntry);
	// 削除する
	pLeafPage->expungeEntry(&cEntry);
	// エントリ数を変更する
	getHeaderPage()->delCount();
}

//
//	FUNCTION public
//	KdTree::BtreeFile::get -- 取得する
//
//	NOTES
//
//	ARGUEMTNS
//	ModUInt32 uiRowID_
//		ROWID
//	PhysicalFile::DirectArea::ID& id_
//		エリアID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
bool
BtreeFile::get(ModUInt32 uiRowID_, PhysicalFile::DirectArea::ID& id_)
{
	bool r = false;
	
	// 比較用のエントリ
	BtreePage::Entry cEntry(uiRowID_, 0);

	// リーフページを検索する
	BtreePage::PagePointer pLeaf = getLeafPage(&cEntry);
	if (pLeaf == 0)
	{
		// ヘッダーしか存在しない -> データがない状態
		return r;
	}

	// リールページ内を検索する
	BtreePage::Iterator i = pLeaf->find(&cEntry);
	if (i != pLeaf->end())
	{
		// ヒットした
		
		id_.m_uiPageID = (*i).m_uiPageID;
		id_.m_uiAreaID = (*i).m_uiAreaID;
		r = true;
	}

	return r;
}

//
//	FUNCTION public
//	KdTree::BtreeFile::getAll -- 全件取得する
//
//	NOTES
//
//	ARGUMENTS
//	Common::BitSet& cBitSet_
//		ビットセット
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeFile::getAll(Common::BitSet& cBitSet_)
{
	cBitSet_.clear();

	// 左端のリーフを得る
	BtreePage::PagePointer p;
	PhysicalFile::PageID uiPageID = getHeaderPage()->getLeftPageID();

	while (uiPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		BtreePage::PagePointer pLeaf
			= attachPage(uiPageID, Buffer::Page::FixMode::ReadOnly, p);

		BtreePage::Iterator i = pLeaf->begin();
		for (; i != pLeaf->end(); ++i)
		{
			cBitSet_.set((*i).m_uiKey);
		}

		uiPageID = pLeaf->getNextPageID();
	}
}

//
//	FUNCTION public
//	KdTree::BtreeFile::getPageData -- 指定ページ内のキーとバリューを得る
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID pageID_
//		指定ページ
//	Common::LargeVector<ModPair<ModUInt32, PhysicalFile::DirectArea::ID> > &
//		vecData_
//		得られるデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeFile::getPageData(PhysicalFile::PageID pageID_,
					   Common::LargeVector<ModPair<
						   ModUInt32, PhysicalFile::DirectArea::ID> >& vecData_)
{
	BtreePage::PagePointer p;
	BtreePage::PagePointer pLeaf = attachPage(pageID_,
											  Buffer::Page::FixMode::ReadOnly,
											  p);
	BtreePage::Iterator i = pLeaf->begin();
	for (; i != pLeaf->end(); ++i)
	{
		// エリアIDを得る
		PhysicalFile::DirectArea::ID id;
		id.m_uiPageID = (*i).m_uiPageID;
		id.m_uiAreaID = (*i).m_uiAreaID;

		// 配列に追加する
		vecData_.pushBack(
			ModPair<ModUInt32, PhysicalFile::DirectArea::ID>((*i).m_uiKey, id));
	}
}

//
//	FUNCTION public
//	KdTree::BtreeFile::clear -- 中身を空にする
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
BtreeFile::clear()
{
	// 変更を破棄する
	recoverAllPages();
	// 中身を空にする
	m_pPhysicalFile->clear(*m_pTransaction, false);
	
	// 初期化する
	
	PhysicalFile::Page* page = 0;
	
	try
	{
		// ヘッダーのページを確保する
		page = m_pPhysicalFile->allocatePage2(
			*m_pTransaction,
			Buffer::Page::FixMode::Write | Buffer::Page::FixMode::Discardable);

		// ヘッダーページを初期化する
		m_pHeaderPage = new HeaderPage(*this, page);
		m_pHeaderPage->initialize();

		// 変更を確定する
		flushAllPages();
	}
	catch (...)
	{
		if (m_pHeaderPage)
			delete m_pHeaderPage, m_pHeaderPage = 0;
		
		if (page)
			m_pPhysicalFile->freePage2(*m_pTransaction, page);

		// 変更を確定する
		flushAllPages();
		
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	KdTree::BtreeFile::getNextLeafPageID
//		-- 次のリーフページのページIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiCurrentPageID_
//		現在のページID
//
//	RETURN
//	PhysicalFile::PageID
//		次のリーフページのID
//			存在しない場合は PhysicalFile::ConstValue::UndefinedPageID
//
//	EXCEPTIONS
//
PhysicalFile::PageID
BtreeFile::getNextLeafPageID(PhysicalFile::PageID uiCurrentPageID_)
{
	//【注意】	PageID の 0 と PhysicalFile::ConstValue::UndefinedPageID
	//			特別な意味になるので、データ格納領域には利用できない
	
	if (uiCurrentPageID_ == 0)
		// 左端のページIDを返す
		return getHeaderPage()->getLeftPageID();

	if (uiCurrentPageID_ == PhysicalFile::ConstValue::UndefinedPageID)
		// 終了
		return uiCurrentPageID_;
	
	// リーフページをattachして、次ページを参照する
	
	BtreePage::PagePointer p;
	BtreePage::PagePointer pLeaf = attachPage(uiCurrentPageID_,
											  Buffer::Page::FixMode::ReadOnly,
											  p);

	return pLeaf->getNextPageID();
}

//
//	FUNCTION public
//	KdTree::BtreeFile::create -- ファイルを作成する
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
BtreeFile::create()
{
	// ファイルを作成する
	SubFile::create();

	PhysicalFile::Page* page = 0;
	
	try
	{
		// ヘッダーのページを確保する
		page = m_pPhysicalFile->allocatePage2(*m_pTransaction,
											  m_eFixMode);

		// ヘッダーページを初期化する
		m_pHeaderPage = new HeaderPage(*this, page);
		m_pHeaderPage->initialize();

		// 変更を確定する
		flushAllPages();
	}
	catch (...)
	{
		if (m_pHeaderPage)
			delete m_pHeaderPage, m_pHeaderPage = 0;
		
		if (page)
			m_pPhysicalFile->freePage2(*m_pTransaction, page);

		// 変更を確定する
		flushAllPages();
		
		// 削除する
		SubFile::destroy(*m_pTransaction);
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	KdTree::VectorFile::close -- ファイルをクローズする
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
BtreeFile::close()
{
	flushAllPages();
	SubFile::close();
}

//
//	FUNCTION public
//	KdTree::VectorFile::move -- ファイルを移動する
//
//	NOTES
//	移動元と移動先のパスが異なっていることが前提。
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cPath_
//		移動先のパス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeFile::move(const Trans::Transaction& cTransaction_,
				const Os::Path& cPath_)
{
	SubFile::move(cTransaction_, Os::Path(cPath_).addPart(_cSubPath));
}

//
//	FUNCTION public
//	KdTree::BtreeFile::allocatePage -- ページを確保する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPrevPageID_
//		前方ページ
//	PhysicalFile::PageID uiNextPageID_
//		後方ページ
//	BtreePage::PagePointer pParentPage_
//		親ページ
//
//	RETURN
//	BtreePage::PagePointer
//		確保したページ
//
//	EXCEPTIONS
//
BtreePage::PagePointer
BtreeFile::allocatePage(PhysicalFile::PageID uiPrevPageID_,
						PhysicalFile::PageID uiNextPageID_,
						const BtreePage::PagePointer& pParentPage_)
{
	BtreePage::PagePointer p;
	Os::AutoCriticalSection cAuto(getLatch());
	
	// 物理ページを確保する
	PhysicalFile::Page* page
		= m_pPhysicalFile->allocatePage2(*m_pTransaction,
										 m_eFixMode);
	try
	{
		p = new BtreePage(*this, page,
						  uiPrevPageID_, uiNextPageID_);
		p->setParentPage(pParentPage_);

		// マップに格納する
		m_cPageMap.insert(page->getID(), p.get());
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		m_pPhysicalFile->freePage2(*m_pTransaction, page);
		_TRMEISTER_RETHROW;
	}

	return p;
}

//
//	FUNCTION public
//	KdTree::BtreeFile::freePage -- ページを解放する
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::BtreePage* pPage_
//		解放するページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeFile::freePage(BtreePage* pPage_)
{
	Os::AutoCriticalSection cAuto(getLatch());

	PageMap::Iterator i = m_cPageMap.find(pPage_->getID());
	if (i != m_cPageMap.end())
		m_cPageMap.erase(i);
	
	pPage_->m_pNext = m_pFree;
	m_pFree = pPage_;
}

//
//	FUNCTION public
//	KdTree::BtreeFile::attachPage -- ページをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		アタッチするページID
//	Buffer::Page::FixMode::Value eFixMode_
//		FIXモード
//	KdTree::BtreePage::PagePointer& pParentPage_
//		親ページ
//
//	RETURN
//	KdTree::BtreePage::PagePointer
//		アタッチしたページ
//
//	EXCEPTIONS
//
BtreePage::PagePointer
BtreeFile::attachPage(PhysicalFile::PageID uiPageID_,
					  Buffer::Page::FixMode::Value eFixMode_,
					  const BtreePage::PagePointer& pParentPage_)
{
	BtreePage::PagePointer p;
	Os::AutoCriticalSection cAuto(getLatch());
	
	PageMap::Iterator i = m_cPageMap.find(uiPageID_);
	if (i != m_cPageMap.end())
	{
		// 見つかった
		p = (*i).second;
		
		if (eFixMode_ & Buffer::Page::FixMode::Write &&
			p->isReadOnly())
		{
			// 更新モードに変更する
			p->updateMode();
		}
	}
	else
	{
		// 見つからなかった
		PhysicalFile::Page* page = attachPhysicalPage(uiPageID_, eFixMode_);
		p = new BtreePage(*this, page);
		
		// マップに格納する
		m_cPageMap.insert(page->getID(), p.get());
	}

	// 親を設定する
	p->setParentPage(pParentPage_);

	return p;
}

//
// 	FUNCTION public
//	KdTree::BtreeFile::detachPage -- ページをdetachする
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::Page* pPage_
//		detach するページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeFile::detachPage(Page* pPage_)
{
	if (pPage_->isDirty() == false)
	{
		Os::AutoCriticalSection cAuto(getLatch());
		
		// dirty じゃないページはそのまま解放
		PageMap::Iterator i = m_cPageMap.find(pPage_->getID());
		if (i != m_cPageMap.end())
			m_cPageMap.erase(i);

		PhysicalFile::Page::UnfixMode::Value mode
			= pPage_->isDirty() ? PhysicalFile::Page::UnfixMode::Dirty
			: PhysicalFile::Page::UnfixMode::NotDirty;
		
		m_pPhysicalFile->detachPage(pPage_->m_pPhysicalPage, mode);
		delete pPage_;
	}
}

//
//	FUNCTION public
//	KdTree::BtreeFile::changeFixMode -- 物理ページのFixModeを変更する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPage_
//		変更前の物理ページ
//
//	RETURN
//	PhysicalFile::Page*
//		保存されているFIXモードでattachしたページ
//
//	EXCEPTIONS
//
PhysicalFile::Page*
BtreeFile::changeFixMode(PhysicalFile::Page* pPage_)
{
	Os::AutoCriticalSection cAuto(getLatch());
	
	// まずは、変更前のページをdetachする
	PhysicalFile::PageID uiPageID = pPage_->getID();
	m_pPhysicalFile->detachPage(pPage_);

	// 新しくattachする
	return attachPhysicalPage(uiPageID, m_eFixMode);
}

//
//	FUNCTION public
//	KdTree::BtreeFile::flushAllPages
//		-- すべてのページの内容を確定する
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
BtreeFile::flushAllPages()
{
	// ヘッダーページ
	if (m_pHeaderPage)
	{
		PhysicalFile::Page::UnfixMode::Value mode
			= m_pHeaderPage->isDirty() ? PhysicalFile::Page::UnfixMode::Dirty
			: PhysicalFile::Page::UnfixMode::NotDirty;
		m_pPhysicalFile->detachPage(m_pHeaderPage->m_pPhysicalPage, mode);
		delete m_pHeaderPage, m_pHeaderPage = 0;
	}
	
	// フリーリストのページを開放する
	BtreePage* pPage = m_pFree;
	while (pPage)
	{
		// ページを開放する
		m_pPhysicalFile->freePage2(*m_pTransaction, pPage->m_pPhysicalPage);

		BtreePage* pSave = pPage;
		pPage = pPage->m_pNext;

		delete pSave;
	}
	m_pFree = 0;

	// マップに格納されているすべてのページ内容を確定する
	PageMap::Iterator i = m_cPageMap.begin();
	for (; i != m_cPageMap.end(); ++i)
	{
		BtreePage* pPage = (*i).second;

		; _SYDNEY_ASSERT(pPage->m_iReference == 0);

		PhysicalFile::Page::UnfixMode::Value mode
			= pPage->isDirty() ? PhysicalFile::Page::UnfixMode::Dirty
			: PhysicalFile::Page::UnfixMode::NotDirty;

		// デタッチする
		m_pPhysicalFile->detachPage(pPage->m_pPhysicalPage, mode);
		delete pPage;
	}
	m_cPageMap.erase(m_cPageMap.begin(), m_cPageMap.end());

	m_pPhysicalFile->detachPageAll();
}

//
//	FUNCTION public
//	KdTree::BtreeFile::recoverAllPages
//		-- すべてのページの内容を破棄する
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
BtreeFile::recoverAllPages()
{
	// ヘッダーページ
	if (m_pHeaderPage)
	{
		m_pPhysicalFile->recoverPage(m_pHeaderPage->m_pPhysicalPage);
		delete m_pHeaderPage, m_pHeaderPage = 0;
	}
	
	// フリーリストのページの内容を破棄する
	BtreePage* pPage = m_pFree;
	while (pPage)
	{
		// 内容を破棄する
		m_pPhysicalFile->recoverPage(pPage->m_pPhysicalPage);

		BtreePage* pSave = pPage;
		pPage = pPage->m_pNext;

		delete pSave;
	}
	m_pFree = 0;

	// マップに格納されているすべてのページ内容を破棄する
	PageMap::Iterator i = m_cPageMap.begin();
	for (; i != m_cPageMap.end(); ++i)
	{
		BtreePage* pPage = (*i).second;

		// 内容を元に戻す
		m_pPhysicalFile->recoverPage(pPage->m_pPhysicalPage);
		delete pPage;
	}
	m_cPageMap.erase(m_cPageMap.begin(), m_cPageMap.end());

	m_pPhysicalFile->recoverPageAll();
}

//
//	FUNCTION public
//	KdTree::BtreeFile::getCountPerPage -- １ページに格納できるエントリ数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		1ページに格納できる最大エントリ数
//
//	EXCEPTIONS
//
ModSize
BtreeFile::getCountPerPage()
{
	return BtreePage::getMaxCount(m_cFileID.getPageSize());
}

//
//	FUNCTION public
//	KdTree::BtreeFile::findPage -- 親ページを検索する
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::BtreePage::Entry* pValue_
//		キーデータ
//	PhysicalFile::PageID uiChildPageID_
//		親を探している子ページ
//
//	RETURN
//	KdTree::BtreePage::PagePointer
//		親ページ
//
//	EXCEPTIONS
//
BtreePage::PagePointer
BtreeFile::findPage(const BtreePage::Entry* pEntry_,
					PhysicalFile::PageID uiChildPageID_)
{
	BtreePage::PagePointer pPage;
	PhysicalFile::PageID uiPageID = getHeaderPage()->getRootPageID();
	
	do
	{
		if (uiPageID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		pPage = attachPage(uiPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   pPage);
		
		if (pPage->isLeaf() == true)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}
		
		BtreePage::Iterator i = pPage->upperBound(pEntry_);
		if (i != pPage->begin())
			--i;
		uiPageID = (*i).m_uiPageID;
	}
	while (uiPageID != uiChildPageID_);

	return pPage;
}

//
//	FUNCTION private
//	KdTree::BtreeFile::getLeafPage -- リーフページを得る
//
//	NOTES
//
//	ARGUMENTS
//	int iDimension_
//		次元
//	const BtreePage::Entry* pEntry_
//		検索するためのデータ
//
//	RETURN
//	KdTree::BtreePage::PagePointer
//		リーフページ
//
//	EXCEPTIONS
//
BtreePage::PagePointer
BtreeFile::getLeafPage(const BtreePage::Entry* pEntry_)
{
	BtreePage::PagePointer pLeafPage = getRootPage();
	if (pLeafPage == 0)
		return pLeafPage;

	while (pLeafPage->isLeaf() == false)
	{
		PhysicalFile::PageID uiPageID;
		BtreePage::Iterator i;
		
		// upper_boundで検索して１つ前
		i = pLeafPage->upperBound(pEntry_);
		if (i != pLeafPage->begin())
			--i;

		// ページIDを得る
		uiPageID = (*i).m_uiPageID;
		
		// ページを得る
		if (m_eFixMode & Buffer::Page::FixMode::ReadOnly)
		{
			// 検索なので、親はいらない
			
			pLeafPage = attachPage(uiPageID,
								   Buffer::Page::FixMode::ReadOnly,
								   BtreePage::PagePointer());
		}
		else
		{
			// 	更新時もつねにノードページが更新されるとは限らないので、
			//	とりあえずReadOnlyでattachする。
			//	リーフとノードも区別できないので、多少無駄があるが両方とも
			
			pLeafPage = attachPage(uiPageID,
								   Buffer::Page::FixMode::ReadOnly,
								   pLeafPage);
		}
	}
	
	return pLeafPage;
}

//
//	FUNCTION private
//	KdTree::BtreeFile::getRootPage -- ルートページを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	KdTree::BtreePage::PagePointer
//		ルートページ
//
//	EXCEPTIONS
//
BtreePage::PagePointer
BtreeFile::getRootPage()
{
	BtreePage::PagePointer pPage;
	
	PhysicalFile::PageID uiPageID = getHeaderPage()->getRootPageID();
	if (uiPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// ルートページを得る
		// 常に ReadOnly で attach する
		
		pPage = attachPage(uiPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   BtreePage::PagePointer());
	}

	return pPage;
}

//
//	FUNCTION private
//	KdTree::BtreeFile::getHeaderPage -- ヘッダーページを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	KdTree::HeaderPage*
//		ヘッダーページ
//
//	EXCEPTIONS
//
HeaderPage*
BtreeFile::getHeaderPage()
{
	if (m_pHeaderPage == 0)
	{
		PhysicalFile::PageID id = 0;
		
		PhysicalFile::Page* p = m_pPhysicalFile->attachPage(*m_pTransaction,
															id,
															m_eFixMode);
		m_pHeaderPage = new HeaderPage(*this, p);
	}
	return m_pHeaderPage;
}

//
//	Copyright (c) 2013, 2014, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
