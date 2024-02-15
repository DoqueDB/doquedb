// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LobFile.cpp --
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2011, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Lob";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Lob/LobFile.h"
#include "Lob/TopPage.h"
#include "Lob/NodePage.h"
#include "Lob/DirPage.h"
#include "Lob/DataPage.h"
#include "Lob/CompressedDataPage.h"
#include "Lob/Parameter.h"
#include "Lob/LobData.h"
#include "Lob/CompressedLobData.h"
#include "Lob/MessageAll_Class.h"
#include "Lob/ObjectID.h"

#include "LogicalFile/Estimate.h"

#include "Common/Assert.h"

#include "Exception/EntryNotFound.h"
#include "Exception/VerifyAborted.h"
#include "Exception/Cancel.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_LOB_USING

namespace
{
	//
	//	VARIABLE
	//
	ParameterInteger _cCachePageSize("Lob_CachePageSize", 10);

	//
	//	VARIABLE
	//
	PhysicalFile::PageID _uiTopPageID = 0;
}

//
//	FUNCTION public
//	Lob::LobFile::LobFile -- コンストラクタ
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
//	なし
//
LobFile::LobFile(const FileID& cFileID_)
	: File(_cCachePageSize.get()), m_cFileID(cFileID_), m_pTopPage(0)
{
	// 物理ファイルをアタッチする
	attach(cFileID_, cFileID_.getPageSize());
}

//
//	FUNCTION public
//	Lob::LobFile::~LobFile -- デストラクタ
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
//	なし
//
LobFile::~LobFile()
{
	// 物理ファイルをデタッチする
	detach();
}

//
//	FUNCTION public
//	Lob::LobFile::getCount -- オブジェクトの総数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInt64
//		オブジェクトの総数
//
//	EXCEPTIONS
//
ModInt64
LobFile::getCount() const
{
	return const_cast<LobFile*>(this)->getTopPage()->getTotalEntryCount();
}

//
//	FUNCTION public
//	Lob::LobFile::check -- 存在をチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const Lob::ObjectID& cObjectID_
//		ObjectID
//
//	RETURN
//	bool
//		存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LobFile::check(const ObjectID& cObjectID_)
{
	bool result = false;
	
	// ブロックを得る
	BlockPage::PagePointer p = attachBlockPage(cObjectID_.m_uiPageID);
	BlockPage::Block* pBlock = p->getBlock(cObjectID_.m_uiPosition);
	if (pBlock)
	{
		result = pBlock->isExpunge() ? false : true;
	}

	return result;
}

//
//	FUNCTION public
//	Lob::LobFile::get -- 得る
//
//	NOTES
//
//	ARGUMENTS
//	const Lob::ObjectID& cObjectID_
//		ObjectID
//	ModSize uiPosition_
//		開始位置
//	ModSize& uiLength_
//		データ長(データ長が短かった場合には実際に得られた長さ)
//	bool& isNull_
//		nullの場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
AutoPointer<void>
LobFile::get(const ObjectID& cObjectID_,
			 ModSize uiPosition_, ModSize& uiLength_, bool& isNull_)
{
	// ブロックを得る
	BlockPage::PagePointer p = attachBlockPage(cObjectID_.m_uiPageID);
	BlockPage::Block* pBlock = p->getBlock(cObjectID_.m_uiPosition);
	if (pBlock == 0 || pBlock->isExpunge() == true)
	{
		isNull_ = true;
		return 0;
	}

	isNull_ = false;
	// LobDataを得る
	ModAutoPointer<LobData> pLobData = allocateLobData(p, pBlock);
	// 取得する
	return pLobData->get(uiPosition_, uiLength_);
}

//
//	FUNCTION public
//	Lob::LobFile::getDataSize -- データ長を得る
//
//	NOTES
//
//	ARGUMENTS
//	const ObjectID& cObjectID_
//		ObjectID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ModSize
LobFile::getDataSize(const ObjectID& cObjectID_)
{
	// ブロックを得る
	BlockPage::PagePointer p = attachBlockPage(cObjectID_.m_uiPageID);
	BlockPage::Block* pBlock = p->getBlock(cObjectID_.m_uiPosition);
	return pBlock->m_uiLength;
}

//
//	FUNCTION public
//	Lob::LobFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const void* pBuffer_
//		挿入するデータ
//	ModSize uiLength_
//		挿入するデータ長
//
//	RETURN
//	Lob::ObjectID
//		挿入したデータのObjectID
//
//	EXCEPTIONS
//
ObjectID
LobFile::insert(const void* pBuffer_, ModSize uiLength_)
{
	BlockPage::PagePointer pBlockPage;
	ObjectID cObjectID;

	// Blockを得る
	BlockPage::Block* pBlock = allocateBlock(pBlockPage, cObjectID);
	
	// LobDataを得る
	ModAutoPointer<LobData> pLobData = allocateLobData(pBlockPage, pBlock);
	// 挿入する
	pLobData->insert(pBuffer_, uiLength_);

	// エントリー数を増やす
	getTopPage()->incrementEntryCount();
	
	return cObjectID;
}

//
//	FUNCTION public
//	Lob::LobFile::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	const ObjectID& cObjectID_
//		削除するブロックのObjectID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobFile::expunge(const ObjectID& cObjectID_)
{
	// ブロックを得る
	BlockPage::PagePointer p = attachBlockPage(cObjectID_.m_uiPageID);
	BlockPage::Block* pBlock = p->getBlock(cObjectID_.m_uiPosition);

	p->dirty();

	// 削除したトランザクションを設定
	pBlock->m_uiTransactionID = getTransaction().getID();
	// 現在の削除リストの先頭を設定
	pBlock->m_cNextBlock = getTopPage()->getExpungeBlock();
	// 削除フラグを設定する
	pBlock->setExpungeFlag();
	
	getTopPage()->setExpungeBlock(cObjectID_);

	// エントリ数を減らす
	getTopPage()->decrementEntryCount();
}

//
//	FUNCTION public
//	Lob::LobFile::undoExpunge -- 削除を取り消す
//
//	NOTES
//
//	ARGUMENTS
//	const ObjectID& cObjectID_
//		削除を取り消すブロックのObjectID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobFile::undoExpunge(const ObjectID& cObjectID_)
{
	// 削除リストから取り除く
	undoExpungeList(cObjectID_);
	// エントリー数を増やす
	getTopPage()->incrementEntryCount();
}

//
//	FUNCTION public
//	Lob::LobFile::update -- 更新する
//
//	NOTES
//
//	ARGUMENTS
//	const ObjectID& cObjectID_
//		ObjectID
//	const void* pBuffer_
//		更新するデータ
//	ModSiz uiLength_
//		更新するデータ長
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobFile::update(const ObjectID& cObjectID_,
				const void* pBuffer_, ModSize uiLength_)
{
	BlockPage::PagePointer pNewBlockPage;
	ObjectID cNewObjectID;

	// 今のBlockを得る
	BlockPage::PagePointer pBlockPage = attachBlockPage(cObjectID_.m_uiPageID);
	BlockPage::Block* pBlock = pBlockPage->getBlock(cObjectID_.m_uiPosition);
	
	// 移動先のBlockを得る
	BlockPage::Block* pNewBlock = allocateBlock(pNewBlockPage, cNewObjectID);
	pNewBlockPage->dirty();
	// 内容を移動先にコピーする
	Os::Memory::copy(pNewBlock, pBlock, sizeof(BlockPage::Block));

	// 新しいデータを今のBlockに挿入する
	ModAutoPointer<LobData> pLobData = allocateLobData(pBlockPage, pBlock);
	pLobData->insert(pBuffer_, uiLength_);

	// 移動先のObjectIDを記録する
	pBlock->m_cPrevBlock = cNewObjectID;

	// 移動先のBlockを削除する
	pNewBlock->m_uiTransactionID = getTransaction().getID();
	pNewBlock->m_cNextBlock = getTopPage()->getExpungeBlock();
	pNewBlock->setExpungeFlag();
	getTopPage()->setExpungeBlock(cNewObjectID);
}

//
//	FUNCTION public
//	Lob::LobFile::undoUpdate -- 更新を取り消す
//
//	NOTES
//
//	ARGUMENTS
//	const ObjectID& cObjectID_
//		更新を取り消すブロックのObjectID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobFile::undoUpdate(const ObjectID& cObjectID_)
{
	// 今のBlockを得る
	BlockPage::PagePointer pBlockPage = attachBlockPage(cObjectID_.m_uiPageID);
	BlockPage::Block* pBlock = pBlockPage->getBlock(cObjectID_.m_uiPosition);
	pBlockPage->dirty();

	ObjectID cPrevID = pBlock->m_cPrevBlock;
	; _SYDNEY_ASSERT(cPrevID.isInvalid() == false);

	// 直前のイメージを削除リストから取り出す
	undoExpungeList(cPrevID);

	// 直前のイメージのBlockを得る
	BlockPage::PagePointer pPrevBlockPage = attachBlockPage(cPrevID.m_uiPageID);
	BlockPage::Block* pPrevBlock
		= pPrevBlockPage->getBlock(cPrevID.m_uiPosition);
	pPrevBlockPage->dirty();

	// 今のBlockを本当に削除する
	ModAutoPointer<LobData> pLobData = allocateLobData(pBlockPage, pBlock);
	pLobData->expunge();

	// 直前のイメージの内容をコピーする
	Os::Memory::copy(pBlock, pPrevBlock, sizeof(BlockPage::Block));

	// 直前のBlockをフリーリストへ
	pPrevBlock->clear();
	pPrevBlock->m_cNextBlock = getTopPage()->getFreeBlock();
	pPrevBlock->setExpungeFlag();
	getTopPage()->setFreeBlock(cPrevID);
}

//
//	FUNCTION public
//	Lob::LobFile::append -- 追加する
//
//	NOTES
//
//	ARGUMENTS
//	const ObjectID& cObjectID_
//		ObjectID
//	const void* pBuffer_
//		追加するデータ
//	ModSize uiLength_
//		追加するデータ長
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobFile::append(const ObjectID& cObjectID_,
				const void* pBuffer_, ModSize uiLength_)
{
	// ブロックを得る
	BlockPage::PagePointer p = attachBlockPage(cObjectID_.m_uiPageID);
	BlockPage::Block* pBlock = p->getBlock(cObjectID_.m_uiPosition);

	// LobDataを得る
	ModAutoPointer<LobData> pLobData = allocateLobData(p, pBlock);
	// 追加する
	pLobData->append(pBuffer_, uiLength_);
}

//
//	FUNCTION public
//	Lob::LobFile::truncate -- 縮める
//
//	NOTES
//
//	ARGUMENTS
//	const ObjectID& cObjectID_
//		ObjectID
//	ModSize uiLength_
//		縮めるデータ長
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobFile::truncate(const ObjectID& cObjectID_, ModSize uiLength_)
{
	// ブロックを得る
	BlockPage::PagePointer p = attachBlockPage(cObjectID_.m_uiPageID);
	BlockPage::Block* pBlock = p->getBlock(cObjectID_.m_uiPosition);

	// LobDataを得る
	ModAutoPointer<LobData> pLobData = allocateLobData(p, pBlock);
	// 縮める
	pLobData->truncate(uiLength_);
}

//
//	FUNCTION public
//	Lob::LobFile::replace -- 変更する
//
//	NOTES
//
//	ARGUMENTS
//	const ObjectID& cObjectID_
//		ObjectID
//	ModSize uiPosition_
//		変更開始位置
//	const void* pBuffer_
//		変更するデータ
//	ModSize uiLength_
//		変更するデータ長
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobFile::replace(const ObjectID& cObjectID_, ModSize uiPosition_,
				 const void* pBuffer_, ModSize uiLength_)
{
	// ブロックを得る
	BlockPage::PagePointer p = attachBlockPage(cObjectID_.m_uiPageID);
	BlockPage::Block* pBlock = p->getBlock(cObjectID_.m_uiPosition);

	// LobDataを得る
	ModAutoPointer<LobData> pLobData = allocateLobData(p, pBlock);
	// 変更する
	pLobData->replace(uiPosition_, pBuffer_, uiLength_);
}

//
//	FUNCTION public
//	Lob::LobFile::isExistExpungeData -- 削除データがあるかないか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		削除するデータがある場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LobFile::isExistExpungeData()
{
	TopPage* pTopPage = getTopPage();
	if(pTopPage->getExpungeBlock().isInvalid() == true)
		return false;
	return true;
}

//
//	FUNCTION public
//	Lob::LobFile::compact -- 削除データを１つ開放する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		削除した場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
LobFile::compact()
{
	TopPage* pTopPage = getTopPage();
	if(pTopPage->getExpungeBlock().isInvalid() == true)
		return false;

	if (Trans::Transaction::ID(pTopPage->getTransactionID())
		!= getTransaction().getID())
	{
		// はじめてなので、初期化する
		pTopPage->setTransactionID(getTransaction().getID());
		ObjectID o;
		o.initialize();
		pTopPage->setPrevFreeBlock(o);
	}

	ObjectID cPrev = pTopPage->getPrevFreeBlock();

	if (cPrev.isInvalid() == true)
	{
		// 先頭をチェック
		ObjectID cID = pTopPage->getExpungeBlock();
				
		BlockPage::PagePointer p = attachBlockPage(cID.m_uiPageID);
		BlockPage::Block* pBlock = p->getBlock(cID.m_uiPosition);

		// 削除できるかチェックする
		if (Trans::Transaction::isInProgress(
				m_cFileID.getLockName().getDatabasePart(),
				Trans::Transaction::ID(pBlock->m_uiTransactionID)) == false)
		{
			// 削除
			{
				ModAutoPointer<LobData> pLobData = allocateLobData(p, pBlock);
				pLobData->expunge();
			}
			// 次を設定
			ObjectID cNext = pBlock->m_cNextBlock;
			pTopPage->setExpungeBlock(cNext);
			
			// フリーリストへ
			pBlock->m_cNextBlock = pTopPage->getFreeBlock();
			pTopPage->setFreeBlock(cID);

			return true;
		}
		else
		{
			// 削除できないので次
			cPrev = cID;
			pTopPage->setPrevFreeBlock(cPrev);
		}
	}

	BlockPage::PagePointer pPrev = attachBlockPage(cPrev.m_uiPageID);
	BlockPage::Block* pPrevBlock = pPrev->getBlock(cPrev.m_uiPosition);
	ObjectID cID = pPrevBlock->m_cNextBlock;

	bool result = false;
	
	while (cID.isInvalid() == false)
	{
		BlockPage::PagePointer p = attachBlockPage(cID.m_uiPageID);
		BlockPage::Block* pBlock = p->getBlock(cID.m_uiPosition);

		// 削除できるかチェックする
		if (Trans::Transaction::isInProgress(
				m_cFileID.getLockName().getDatabasePart(),
				Trans::Transaction::ID(pBlock->m_uiTransactionID)) == true)
		{
			// 次へ
			cPrev = cID;
			pPrev = p;
			pPrevBlock = pBlock;
			cID = pBlock->m_cNextBlock;
			pTopPage->setPrevFreeBlock(cPrev);
			continue;
		}

		// 削除
		{
			ModAutoPointer<LobData> pLobData = allocateLobData(p, pBlock);
			pLobData->expunge();
		}

		// その次
		ObjectID cNextID = pBlock->m_cNextBlock;

		// つなぎ換え
		pPrev->dirty();
		pPrevBlock->m_cNextBlock = cNextID;
		
		// フリーリストへ
		pBlock->m_cNextBlock = pTopPage->getFreeBlock();
		pTopPage->setFreeBlock(cID);
		cID.clear();

		result = true;
	}

	return result;
}

//
//	FUNCTION public
//	Lob::LobFile::create -- ファイルを作成する
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
LobFile::create()
{
	// まず下位を呼ぶ
	File::create();
	try
	{
		// 初期化する
		initializeTopPage();
	}
	catch (...)
	{
		recoverAllPages();
		File::destroy();
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Lob::LobFile::verify -- 整合性検査を行う
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
LobFile::verify()
{
	getTopPage();	// TopPageをattachする

	// データの整合性をチェックする
	BlockPage::PagePointer p = m_pTopPagePointer;
	while (p.get())
	{
		ModSize uiPosition = 0;
		while (BlockPage::Block* pBlock = p->getNextBlock(uiPosition))
		{
			ObjectID cObjectID;
			cObjectID.m_uiPageID = p->getID();
			cObjectID.m_uiPosition = uiPosition;
			ModAutoPointer<LobData> pLobData = allocateLobData(p, pBlock);
			pLobData->verify(cObjectID);
			if (isCancel() == true)
			{
				_SYDNEY_THROW0(Exception::Cancel);
			}
		}
		PhysicalFile::PageID cID = p->getNextBlockPage();
		if (cID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			p = 0;
		}
		else
		{
			p = attachBlockPage(cID);
		}
	}
}

//
//	FUNCTION public
//	Lob::LobFile::sync -- 同期を取る
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	bool& bIncomplete_
//		同時処理の結果、物理ファイルを処理し残したかを設定する
//	bool& bModified
//		同期処理の結果、物理ファイルが更新されたかを設定する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobFile::sync(const Trans::Transaction& cTransaction_,
			  bool& bIncomplete_, bool& bModified_)
{
	File::sync(cTransaction_, bIncomplete_, bModified_);
}

//
//	FUNCTION public
//	Lob::LobFile::flushAllPages -- 内容をフラッシュする
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
LobFile::flushAllPages()
{
	// トップページをdetachする
	m_pTopPagePointer = 0;

	// 変更を確定する
	File::flushAllPages();
}

//
//	FUNCTION public
//	Lob::LobFile::recoverAllPages -- 変更内容を元に戻す
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
LobFile::recoverAllPages()
{
	// トップページをdetachする
	m_pTopPagePointer = 0;

	// 変更を元に戻す
	File::recoverAllPages();
}

//
//	FUNCTION public
//	Lob::LobFile::getOverhead -- オーバーヘッドコストを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	double
//		秒数
//
//	EXCEPTIONS
//
double
LobFile::getOverhead() const
{
	return File::getOverhead();
}

//
//	FUNCTION public
//	Lob::LobFile::getEntryCount -- エントリ数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		エントリ数
//
//	EXCEPTIONS
//
ModSize
LobFile::getEntryCount() const
{
	return const_cast<LobFile*>(this)->getTopPage()
		->getHeader()->m_uiTotalEntryCount;
}

//
//	FUNCTION public
//	Lob::LobFile::attachBlockPage -- BLOCKページをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		アタッチするページのID
//
//	RETURN
//	Lob::BlockPage::PagePointer
//		アタッチしたページ
//
//	EXCEPTIONS
//
BlockPage::PagePointer
LobFile::attachBlockPage(PhysicalFile::PageID uiPageID_)
{
	// まずこれまでattachされているかどうかマップを検索する
	BlockPage::PagePointer pPage
				   = _SYDNEY_DYNAMIC_CAST(BlockPage*, File::findMap(uiPageID_));
	if (pPage == 0)
	{
		if (uiPageID_ == _uiTopPageID)
		{
			// TOPページを返す
			(void) getTopPage();
			pPage = m_pTopPagePointer;
		}
		else
		{
			// NODEページを新たに確保する
			PhysicalFile::Page* pPhysicalPage = attachPhysicalPage(uiPageID_);
			pPage = new NodePage(*this, pPhysicalPage);
			File::attachPage(pPage);
		}
	}

	return pPage;
}

//
//	FUNCTION public
//	Lob::LobFile::allocateBlockPage -- 新しいBLOCKページを確保する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Lob::BlockPage::PagePointer
//		新しく確保したページ
//
//	EXCEPTIONS
//
BlockPage::PagePointer
LobFile::allocateBlockPage()
{
	PhysicalFile::Page* p = File::getFreePage();
	if (p == 0)
	{
		// 新たに確保する
		p = File::allocatePage();
	}

	BlockPage::PagePointer pPage = new NodePage(*this, p);
	pPage->initialize();	// 初期化する
	File::attachPage(pPage);

	return pPage;
}

//
//	FUNCTION public
//	Lob::LobFile::attachDirPage -- DIRページをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		アタッチするページのID
//
//	RETURN
//	Lob::DirPage::PagePointer
//		アタッチしたページ
//
//	EXCEPTIONS
//
DirPage::PagePointer
LobFile::attachDirPage(PhysicalFile::PageID uiPageID_)
{
	// まずこれまでattachされているかどうかマップを検索する
	DirPage::PagePointer pPage
		= _SYDNEY_DYNAMIC_CAST(DirPage*, File::findMap(uiPageID_));
	if (pPage == 0)
	{
		// 無かったので、新たに確保する
		PhysicalFile::Page* pPhysicalPage = attachPhysicalPage(uiPageID_);
		pPage = new DirPage(*this, pPhysicalPage);
		File::attachPage(pPage);
	}

	return pPage;
}

//
//	FUNCTION public
//	Lob::LobFile::allocateDirPage -- 新しいDIRページを確保する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiStep_
//		段数
//
//	RETURN
//	Lob::DirPage::PagePointer
//		新しく確保したページ
//
//	EXCEPTIONS
//
DirPage::PagePointer
LobFile::allocateDirPage(ModSize uiStep_)
{
	PhysicalFile::Page* p = File::getFreePage();
	if (p == 0)
	{
		// 新たに確保する
		p = File::allocatePage();
	}

	DirPage::PagePointer pPage = new DirPage(*this, p, uiStep_);
	File::attachPage(pPage);

	return pPage;
}

//
//	FUNCTION public
//	Lob::LobFile::attachDataPage -- DATAページをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		アタッチするページのID
//
//	RETURN
//	Lob::DataPage::PagePointer
//		アタッチしたページ
//
//	EXCEPTIONS
//
DataPage::PagePointer
LobFile::attachDataPage(PhysicalFile::PageID uiPageID_)
{
	// まずこれまでattachされているかどうかマップを検索する
	DataPage::PagePointer pPage
		= _SYDNEY_DYNAMIC_CAST(DataPage*, File::findMap(uiPageID_));
	if (pPage == 0)
	{
		// 無かったので、新たに確保する
		PhysicalFile::Page* pPhysicalPage = attachPhysicalPage(uiPageID_);
		if (m_cFileID.isCompressed() == true)
			pPage = new CompressedDataPage(*this, pPhysicalPage);
		else
			pPage = new DataPage(*this, pPhysicalPage);
		File::attachPage(pPage);
	}

	return pPage;
}

//
//	FUNCTION public
//	Lob::LobFile::allocateDataPage -- 新しいDATAページを確保する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPrevPageID_
//		前方のページID
//	PhysicalFile::PageID uiNextPageID_
//		後方のページID
//
//	RETURN
//	Lob::DataPage::PagePointer
//		新しく確保したページ
//
//	EXCEPTIONS
//
DataPage::PagePointer
LobFile::allocateDataPage(PhysicalFile::PageID uiPrevPageID_,
						  PhysicalFile::PageID uiNextPageID_)
{
	PhysicalFile::Page* p = File::getFreePage();
	if (p == 0)
	{
		// 新たに確保する
		p = File::allocatePage();
	}

	DataPage::PagePointer pPage;
	if (m_cFileID.isCompressed() == true)
		pPage = new CompressedDataPage(*this, p, uiPrevPageID_, uiNextPageID_);
	else
		pPage = new DataPage(*this, p, uiPrevPageID_, uiNextPageID_);
	File::attachPage(pPage);

	return pPage;
}

//
//	FUNCTION private
//	Lob::LobFile::initializeTopPage -- 初期化する
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
LobFile::initializeTopPage()
{
	// ヘッダーページを確保する
	PhysicalFile::Page* pPage = File::allocatePage();
	; _SYDNEY_ASSERT(pPage->getID() == _uiTopPageID);

	// トップページを初期化する
	getTopPage(pPage)->initialize();
}

//
//	FUNCTION private
//	Lob::LobFile::getTopPage -- トップページを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Lob::TopPage*
//	   	トップページ
//
//	EXCEPTIONS
//
TopPage*
LobFile::getTopPage(PhysicalFile::Page* pPhysicalPage_)
{
	if (m_pTopPagePointer == 0)
	{
		if (pPhysicalPage_ == 0)
			pPhysicalPage_ = attachPhysicalPage(_uiTopPageID);
		m_pTopPage = new TopPage(*this, pPhysicalPage_);
		m_pTopPagePointer = m_pTopPage;
		File::attachPage(m_pTopPage);
	}

	return m_pTopPage;
}

//
//	FUNCTION private
//	Lob::LobFile::allocateBlock -- 新しいBlockを得る
//
//	NOTES
//
//	ARGUMENTS
//	BlockPage::PagePointer& pBlockPage_
//		ブロックがあるぺーじ
//	ObjectID& cObjectID_
//		得られたBlockのObjectID
//
//	RETURN
//	BlockPage::Block*
//		新しいブロック
//
//	EXCEPTIONS
//
BlockPage::Block*
LobFile::allocateBlock(BlockPage::PagePointer& pBlockPage_,
					   ObjectID& cObjectID_)
{
	BlockPage::Block* pBlock = 0;
	
	// 開放されたBlockがあるか
	cObjectID_ = getTopPage()->getFreeBlock();
	if (cObjectID_.isInvalid() == true)
	{
		// ないので新たに得る
		PhysicalFile::PageID uiLastID = getTopPage()->getLastBlockPageID();
		pBlockPage_ = attachBlockPage(uiLastID);
		pBlock = pBlockPage_->allocateBlock(cObjectID_.m_uiPosition);
		if (pBlock == 0)
		{
			// これ以上作れないので新しいページ
			BlockPage::PagePointer p = allocateBlockPage();
			pBlockPage_->setNextBlockPage(p->getID());
			pBlockPage_ = p;
			getTopPage()->setLastBlockPageID(pBlockPage_->getID());
			pBlock = pBlockPage_->allocateBlock(cObjectID_.m_uiPosition);
		}
		getTopPage()->incrementBlockCount();
		cObjectID_.m_uiPageID = pBlockPage_->getID();
	}
	else
	{
		// ブロックを得る
		pBlockPage_ = attachBlockPage(cObjectID_.m_uiPageID);
		pBlock = pBlockPage_->getBlock(cObjectID_.m_uiPosition);

		// フリーブロックのつなぎ替え
		getTopPage()->setFreeBlock(pBlock->m_cNextBlock);
	}
	
	return pBlock;
}

//
//	FUNCTION private
//	Lob::LobFile::undoExpungeList -- 削除リストから取り除く
//
//	NOTES
//
//	ARGUMENTS
//	const ObjectID& cObjectID_
//		取り除くブロックのObjectID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
LobFile::undoExpungeList(const ObjectID& cObjectID_)
{
	// ブロックを得る
	BlockPage::PagePointer p = attachBlockPage(cObjectID_.m_uiPageID);
	BlockPage::Block* pBlock = p->getBlock(cObjectID_.m_uiPosition);
	p->dirty();

	// 削除リストの１つ前のBlockを得る
	ObjectID cPrevObjectID = getTopPage()->getExpungeBlock();
	; _SYDNEY_ASSERT(cPrevObjectID.isInvalid() == false);
	
	if (cPrevObjectID == cObjectID_)
	{
		// 自分が最後のブロック
		getTopPage()->setExpungeBlock(pBlock->m_cNextBlock);
	}

	while (cPrevObjectID != cObjectID_)
	{
		BlockPage::PagePointer pPrev
			= attachBlockPage(cPrevObjectID.m_uiPageID);
		BlockPage::Block* pPrevBlock
			= pPrev->getBlock(cPrevObjectID.m_uiPosition);

		if (pPrevBlock->m_cNextBlock == cObjectID_)
		{
			// １つ前のが見つかった
			pPrev->dirty();
			pPrevBlock->m_cNextBlock = pBlock->m_cNextBlock;

			// 本当の削除中かもしれない
			if (getTopPage()->getPrevFreeBlock() == cObjectID_)
			{
				// 直前がUndoされたので、書き換える
				getTopPage()->setPrevFreeBlock(cPrevObjectID);
			}
			break;
		}

		cPrevObjectID = pPrevBlock->m_cNextBlock;
		; _SYDNEY_ASSERT(cPrevObjectID.isInvalid() == false);
	}

	// 値をクリアする
	pBlock->m_cNextBlock.clear();
	pBlock->unsetExpungeFlag();
}

//
//	FUNCTION private
//	Lob::LobFile::allocateLobData -- LobDataを確保する
//
//	NOTES
//
//	ARGUMENTS
//	Lob::BlockPage::PagePointer pBlockPage_
//		ブロックページ
//	Lob::BlockPage::Block* pBlock_
//		ブロック
//
//	RETURN
//	Lob::LobData*
//		LobDataへのポインタ
//
//	EXCEPTIONS
//
LobData*
LobFile::allocateLobData(BlockPage::PagePointer pBlockPage_,
						 BlockPage::Block* pBlock_)
{
	LobData* pLobData = 0;
	if (m_cFileID.isCompressed() == true)
	{
		pLobData = new CompressedLobData(*this, pBlockPage_, pBlock_);
	}
	else
	{
		pLobData = new LobData(*this, pBlockPage_, pBlock_);
	}
	return pLobData;
}

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
