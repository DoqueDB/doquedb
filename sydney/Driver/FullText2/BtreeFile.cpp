// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreeFile.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "FullText2/BtreeFile.h"
#include "FullText2/BtreePage.h"
#include "FullText2/Parameter.h"
#include "FullText2/FakeError.h"
#include "FullText2/MessageAll_Class.h"

#include "LogicalFile/Estimate.h"

#include "Common/Assert.h"

#include "Os/Path.h"

#include "Exception/EntryNotFound.h"
#include "Exception/VerifyAborted.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//
	//  VARIABLE
	//
	ParameterInteger _cBtreeCachePageSize("FullText2_BtreeCachePageSize", 15);
}

//
//  FUNCTION public
//  FullText2::BtreeFile::BtreeFile -- コンストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  const FullText2::FileID& cFileID_
//		ファイルID
//	const Os::Path& cPath_
//		パス
//	bool bBatch_
//		バッチモードかどうか
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
BtreeFile::BtreeFile(const FileID& cFileID_,
					 const Os::Path& cPath_,
					 bool bBatch_)
	: IndexFile(Type::Btree, _cBtreeCachePageSize.get())
{
	// ファイルをアタッチする
	attach(cFileID_, cFileID_.getBtreePageSize(), cPath_, bBatch_);
}

//
//  FUNCTION public
//  FullText2::BtreeFile::~BtreeFile -- デストラクタ
//
//  NOTES
//  デストラクタ。
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
BtreeFile::~BtreeFile()
{
	// 物理ファイルをデタッチする
	detach();
}

//
//  FUNCTION public
//  FullText2::BtreeFile::create -- ファイルを作成する
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::create()
{
	// まず下位を呼ぶ
	IndexFile::create();
	try
	{
	// ヘッダーを初期化する
		initializeHeaderPage();
	}
	catch (...)
	{
		recoverAllPages();
		IndexFile::destroy();
		_SYDNEY_RETHROW;
	}
}

//
//  FUNCTION public
//  FullText2::BtreeFile::verify -- 整合性検査を行う
//
//  NOTES
//  B木の整合性検査は以下の項目をチェックする
//  1. ルートノードからすべてのページをたどり、
//	   使用しているページIDを物理ファイルに通知する
//  2. すべてのページのエントリが昇順に格納されているかチェックする
//  3. リーフページを横に走査し、ヘッダーに記録されているエントリ数との
//	   整合性をチェックする
//  ※1と2の検査はBtreePage内のverifyで行う
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::verify()
{
	// ルートページを得る
	PagePointer pRootPage = getRootPage();
	if (pRootPage.get() == 0)
		return;
	
	// ページの整合性検査を行う(1と2が同時にチェックされる)
	pRootPage->verify();
	// リーフページの総数とヘッダーが同じかどうかチェックする
	verifyEntryCount();
}

//
//  FUNCTION public
//  FullText2::BtreeFile::move -- ファイルを移動する
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//		  トランザクション
//  const Os::Path& cPath_
//		  パス
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::move(const Trans::Transaction& cTransaction_,
				const Os::Path& cPath_)
{
	IndexFile::move(cTransaction_, cPath_);
}

//
//  FUNCTION public
//  FullText2::BtreeFile::clear -- ページをクリアする
//
//  NOTES
//
//  ARGUMENTS
//  bool bForce_
//		強制モードかどうか
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::clear(bool bForce_)
{
	// まず下位を呼ぶ
	IndexFile::clear(bForce_);
	// ヘッダーページを初期化する
	initializeHeaderPage();
}

//
//  FUNCTION public
//  FullText2::BtreeFile::search
//		-- 引数以下で最大のキー値をもつエントリを検索する
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeChar* pszKey_
//		検索するキー値
//
//  RETURN
//  PhysicalFile::PageID
//		リーフファイルのページID。存在しなかった場合はUndefinedPageID
//
//  EXCEPTIONS
//
bool
BtreeFile::search(const ModUnicodeChar* pszKey_, ModUInt32& uiValue_)
{
	BtreePage::AutoEntry pEntry(BtreePage::allocateEntry(pszKey_));

	// まずはリーフページを得る
	PagePointer pLeafPage = getLeafPage(*pEntry);
	if (pLeafPage == 0)
	{
		; _SYDNEY_ASSERT(0);
		// リーフページがない
		return false;
	}

	// リーフの中を検索する
	return pLeafPage->search(*pEntry, uiValue_);
}

//
//  FUNCTION public
//  FullText2::BtreeFile::find -- 同じキーを持つエントリを検索する
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeChar* pszKey_
//		検索するキー値
//	ModUInt32& uiValue_
//		検索されたバリュー
//
//  RETURN
//	bool
//		ヒットした場合はtrue、それ以外の場合はfalse
//
//  EXCEPTIONS
//
bool
BtreeFile::find(const ModUnicodeChar* pszKey_, ModUInt32& uiValue_)
{
	BtreePage::AutoEntry pEntry(BtreePage::allocateEntry(pszKey_));

	// まずはリーフページを得る
	PagePointer pLeafPage = getLeafPage(*pEntry);
	if (pLeafPage == 0)
	{
		// リーフページがない
		return false;
	}

	// リーフの中を検索する
	return pLeafPage->find(*pEntry, uiValue_);
}

//
//  FUNCTION public
//  FullText2::BtreeFile::insert -- 挿入する
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeChar* pszKey_
//		キー値
//  PhysicalFile::PageID uiValue_
//		バリュー
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::insert(const ModUnicodeChar* pszKey_, PhysicalFile::PageID uiValue_)
{
	BtreePage::AutoEntry pEntry(BtreePage::allocateEntry(pszKey_, uiValue_));

	// まずはリーフページまで検索する
	PagePointer pLeafPage = getLeafPage(*pEntry);
	if (pLeafPage == 0)
	{
		// リーフファイルがない見つからない -> 挿入されているキーより小さなキー
		if (m_pHeader->m_uiLeftLeafPageID
			!= PhysicalFile::ConstValue::UndefinedPageID)
		{
			// 一番左のリーフページをattachする
			pLeafPage = attachPage(m_pHeader->m_uiLeftLeafPageID,
								   m_pHeader->m_uiStepCount);
		}
		else
		{
			// まだこのファイルには１つもデータが挿入されていない
			pLeafPage = allocatePage(PhysicalFile::ConstValue::UndefinedPageID,
									 PhysicalFile::ConstValue::UndefinedPageID,
									 1);
			// リーフである
			pLeafPage->setLeaf();
			// ルートページにする
			setRootPage(pLeafPage);
			// 一番左のリーフページにする
			setLeftLeafPage(pLeafPage);
			// 一番右のリーフページにする
			setRightLeafPage(pLeafPage);
		}
	}
	// リーフファイルに挿入する
	pLeafPage->insert(*pEntry);

	// ヘッダーを更新する
	m_pHeader->m_uiCount++;
	// ヘッダーページをdirtyにする
	m_pHeaderPage->dirty();
}

//
//  FUNCTION public
//  FullText2::BtreeFile::expunge -- 削除する
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeChar* pszKey_
//		削除するエントリのキー値
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::expunge(const ModUnicodeChar* pszKey_)
{
	BtreePage::AutoEntry pEntry(BtreePage::allocateEntry(pszKey_));

	// まずはリーフページまで検索する
	PagePointer pLeafPage = getLeafPage(*pEntry);
	if (pLeafPage == 0)
	{
		// リーフファイルがない -> １つもエントリが挿入されていない
		return;
	}
	// リーフファイルから削除する
	pLeafPage->expunge(*pEntry);

	// ヘッダーを更新する
	m_pHeader->m_uiCount--;
	// ヘッダーページをdirtyにする
	m_pHeaderPage->dirty();
}

//
//  FUNCTION public
//  FullText2::BtreeFile::update -- 更新する
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeChar* pszKey1_
//		更新するエントリのキー値
//  PhysicalFile::PageID uiPageID1_
//		更新するエントリのバリュー値
//  const ModUnicodeChar* pszKey2_
//		更新後のエントリのキー値
//  PhysicalFile::PageID uiPageID2_
//		更新後のエントリのバリュー値
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::update(const ModUnicodeChar* pszKey1_, ModUInt32 uiValue1_,
				  const ModUnicodeChar* pszKey2_, ModUInt32 uiValue2_)
{
	BtreePage::AutoEntry
		pEntry1(BtreePage::allocateEntry(pszKey1_, uiValue1_));
	BtreePage::AutoEntry
		pEntry2(BtreePage::allocateEntry(pszKey2_, uiValue2_));

		// まずはリーフページまで検索する
	PagePointer pLeafPage = getLeafPage(*pEntry1);
	if (pLeafPage == 0)
	{
		// リーフファイルがない
		_SYDNEY_THROW1(Exception::EntryNotFound, pszKey1_);
	}
	// 更新する
	pLeafPage->update(*pEntry1, *pEntry2);
}

//
//  FUNCTION public
//  FullText2::BtreeFile::attachPage -- ページをアタッチする
//
//  NOTES
//
//  ARGUMENTS
//  PhysicalFile::PageID uiPageID_
//		アタッチするページのID
//  ModSize uiStepCount_
//		段数
//
//  RETURN
//  FullText2::BtreeFile::PagePointer
//		アタッチしたページ
//
//  EXCEPTIONS
//
BtreeFile::PagePointer
BtreeFile::attachPage(PhysicalFile::PageID uiPageID_,
												ModSize uiStepCount_)
{
	// まずこれまでattachされているかどうかマップを検索する
	PagePointer pPage
		= _SYDNEY_DYNAMIC_CAST(BtreePage*, IndexFile::findMap(uiPageID_));
	if (pPage == 0)
	{
		; _FULLTEXT2_FAKE_ERROR(BtreeFile::attachPage);
		// 無かったので、新たに確保する
		PhysicalFile::Page* pPhysicalPage = attachPhysicalPage(
				uiPageID_, Buffer::ReplacementPriority::Middle);
		BtreePage* p
			= _SYDNEY_DYNAMIC_CAST(BtreePage*, IndexFile::popInstanceList());
		if (p)
		{
			p->reset(pPhysicalPage);
			pPage = p;
		}
		else
		{
			pPage = new BtreePage(*this, pPhysicalPage);
		}
		IndexFile::attachPage(pPage);
	}

	pPage->setStepCount(uiStepCount_);

	return pPage;
}

//
//  FUNCTION public
//  FullText2::BtreeFile::allocatePage -- 新しいページを確保する
//
//  NOTES
//
//  ARGUMENTS
//  PhysicalFile::PageID uiPrevPageID_
//		前方のページID
//  PhysicalFile::PageID uiNextPageID_
//		後方のページID
//  ModSize uiStepCount_
//		段数
//
//  RETURN
//  FullText2::BtreeFile::PagePointer
//		新しく確保したページ
//
//  EXCEPTIONS
//
BtreeFile::PagePointer
BtreeFile::allocatePage(PhysicalFile::PageID uiPrevPageID_,
						PhysicalFile::PageID uiNextPageID_,
						ModSize uiStepCount_)
{
	PagePointer pPage
		= _SYDNEY_DYNAMIC_CAST(BtreePage*, IndexFile::getFreePage());
	if (pPage)
	{
		pPage->reset(uiPrevPageID_, uiNextPageID_);
	}
	else
	{
		; _FULLTEXT2_FAKE_ERROR(BtreeFile::allocatePage);
		// 新たに確保する
		PhysicalFile::Page* pPhysicalPage = IndexFile::allocatePage();
		pPage = _SYDNEY_DYNAMIC_CAST(BtreePage*, IndexFile::popInstanceList());
		if (pPage)
		{
			pPage->reset(pPhysicalPage, uiPrevPageID_, uiNextPageID_);
		}
		else
		{
			pPage = new BtreePage(*this, pPhysicalPage,
								  uiPrevPageID_, uiNextPageID_);
		}
	}

	pPage->setStepCount(uiStepCount_);

	IndexFile::attachPage(pPage);

	return pPage;
}

//
//  FUNCTION public
//  FullText2::BtreeFile::setRootPage -- ルートページを設定する
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::BtreeFile::PagePointer pRootPage_
//		設定するルートページ
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::setRootPage(PagePointer pRootPage_)
{
	// ヘッダーを更新する
	m_pHeader->m_uiRootPageID = pRootPage_->getID();

	if (pRootPage_->getStepCount() == 1)
		m_pHeader->m_uiStepCount++;
	else
		m_pHeader->m_uiStepCount--;

	m_pHeaderPage->dirty();
}

//
//  FUNCTION public
//  FullText2::BtreeFile::setRightLeafPage -- 一番右のリーフページを設定する
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::BtreeFile::PagePointer pRootPage_
//		設定するリーフページ
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::setRightLeafPage(PagePointer pRootPage_)
{
	// ヘッダーを更新する
	m_pHeader->m_uiRightLeafPageID = pRootPage_->getID();
	m_pHeaderPage->dirty();
}

//
//  FUNCTION public
//  FullText2::BtreeFile::setLeftLeafPage -- 一番左のリーフページを設定する
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::BtreeFile::PagePointer pRootPage_
//		設定するリーフページ
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::setLeftLeafPage(BtreeFile::PagePointer pRootPage_)
{
	// ヘッダーを更新する
	m_pHeader->m_uiLeftLeafPageID = pRootPage_->getID();
	m_pHeaderPage->dirty();
}

//
//  FUNCTION public
//  FullText2::BtreeFile::flushAllPages -- 内容をフラッシュする
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::flushAllPages()
{
	// ヘッダーページをdetachする
	m_pHeaderPage = 0;

	// 変更を確定する
	IndexFile::flushAllPages();
}

//
//  FUNCTION public
//  FullText2::BtreeFile::recoverAllPages -- 変更内容を元に戻す
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::recoverAllPages()
{
	// ヘッダーページをdetachする
	m_pHeaderPage = 0;

	// 変更を元に戻す
	IndexFile::recoverAllPages();
}

//
//  FUNCTION public
//  FullText2::BtreeFile::saveAllPages -- 変更内容を保存する
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::saveAllPages()
{
	// 変更を保存する
	IndexFile::saveAllPages();
}

//
//  FUNCTION public
//  FullText2::BtreeFile::getNodePage -- 該当するノードページを得る
//
//  NOTES
//
//  ARGUMENTS
//  const FullText2::BtreePage::Entry& cEntry_
//		ノードページを検索するためのキー値を含んだもの
//  ModSize uiStepCount_
//		何段目のノードページを得るか
//
//  RETURN
//  FullText2::BtreeFile::PagePointer
//		見つかったノードページ
//		エントリが１つも存在していなかった場合には0が返る
//
//  EXCEPTIONS
//
BtreeFile::PagePointer
BtreeFile::getNodePage(const BtreePage::Entry& cEntry_, ModSize uiStepCount_)
{
	PagePointer pPage = getRootPage();
	ModSize uiStepCount = 1;

	while (pPage)
	{
		if (uiStepCount == uiStepCount_)
			break;

		// 検索する
		PhysicalFile::PageID uiTmpPageID;
		pPage->search(cEntry_, uiTmpPageID);

		// 下位ノードを得る
		uiStepCount++;
		pPage = attachPage(uiTmpPageID, uiStepCount);
	}

	return pPage;
}

//
//  FUNCTION public
//  FullText2::BtreeFile::getEntryCount -- エントリ数を得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModSize
//		エントリ数
//
//  EXCEPTIONS
//
ModSize
BtreeFile::getEntryCount() const
{
	const_cast<BtreeFile*>(this)->setHeader();

	return m_pHeader->m_uiCount;
}

#ifndef SYD_COVERAGE
//
//  FUNCTION public
//  FullText2::BtreeFile::reportFile -- ファイル状態を出力する
//
//  NOTES
//
//  ARGUMETNS
//  const Trans::Transaction& cTransaction_
//		トランザクション
//  ModOstream& stream_
//		出力ストリーム
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::reportFile(const Trans::Transaction& cTransaction_,
										ModOstream& stream_)
{
	// B木では以下の情報を出力する
	//  ・エントリーカウント
	//  ・段数
	//  ・使用ファイルサイズ
	//  ・ページサイズ
	//  ・平均ページ使用率(ヘッダーページ以外)

	stream_ << "B-Tree:" << ModEndl;
	stream_ << "\tPageCount=" << getUsedPageNum(cTransaction_) << ModEndl;
	stream_ << "\tUsedFileSize=" << getUsedSize(cTransaction_) << ModEndl;
	stream_ << "\tPageDataSize=" << getPageDataSize() << ModEndl;
	stream_ << "\tEntryCount=" << getEntryCount() << ModEndl;
	stream_ << "\tStepCount=" << m_pHeader->m_uiStepCount << ModEndl;
	stream_ << "\tAverageUsedPageRatio=" << getAverageUsedPageRatio() << ModEndl;
}
#endif

//
//  FUNCTION private
//  FullText2::BtreeFile::initializeHeaderPage -- ヘッダーページを初期化する
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::initializeHeaderPage()
{
	// ヘッダーページを確保する
	PhysicalFile::Page* pPhysicalPage = IndexFile::allocatePage();
	// ヘッダーを設定する
	setHeader(pPhysicalPage);

	// 内容を設定する
	m_pHeader->m_uiRootPageID = PhysicalFile::ConstValue::UndefinedPageID;
	m_pHeader->m_uiLeftLeafPageID = PhysicalFile::ConstValue::UndefinedPageID;
	m_pHeader->m_uiRightLeafPageID = PhysicalFile::ConstValue::UndefinedPageID;
	m_pHeader->m_uiCount = 0;
	m_pHeader->m_uiStepCount = 0;

	// ヘッダーページをdirtyにする
	m_pHeaderPage->dirty();
}

//
//  FUNCTION private
//  FullText2::BtreeFile::setHeader -- ヘッダーを割り当てる
//
//  NOTES
//
//  ARGUMENTS
//  PhysicalFile::Page* pPhysicalPage_ (default 0)
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::setHeader(PhysicalFile::Page* pPhysicalPage_)
{
	if (m_pHeaderPage == 0)
	{
		if (pPhysicalPage_ == 0)
			// 先頭のページをattach
			pPhysicalPage_ = attachPhysicalPage(
								0, Buffer::ReplacementPriority::Middle);

		m_pHeaderPage = new Page(*this, pPhysicalPage_);
		IndexFile::attachPage(m_pHeaderPage);

		// ヘッダーを割り当てる
		m_pHeader = syd_reinterpret_cast<Header*>(m_pHeaderPage->getBuffer());
	}
}

//
//  FUNCTION private
//  FullText2::BtreeFile::getRootPage -- ルートページを得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  FullText2::BtreeFile::PagePointer
//		ルートページ
//
//  EXCEPTIONS
//
BtreeFile::PagePointer
BtreeFile::getRootPage()
{
	setHeader();

	if (m_pHeader->m_uiRootPageID == PhysicalFile::ConstValue::UndefinedPageID)
		return 0;

	PagePointer pRootPage = attachPage(m_pHeader->m_uiRootPageID, 1);

	return pRootPage;
}

//
//  FUNCTION private
//  FullText2::BtreeFile::getLeafPage -- 該当するリーフページを得る
//
//  NOTES
//
//  ARGUMENTS
//  const FullText2::BtreePage::Entry& cEntry_
//		リーフページを検索するためのキー値を含んだもの
//
//  RETURN
//  FullText2::BtreeFile::PagePointer
//		見つかったリーフページ
//		エントリが１つも存在していなかった場合には0が返る
//
//  EXCEPTIONS
//
BtreeFile::PagePointer
BtreeFile::getLeafPage(const BtreePage::Entry& cEntry_)
{
	PagePointer pPage = getRootPage();
	ModSize uiStepCount = 1;

	while (pPage)
	{
		// リーフだったら終了
		if (pPage->isLeaf()) break;

		// 下位ノードをたどる
		PhysicalFile::PageID uiTmpPageID;
		if (pPage->search(cEntry_, uiTmpPageID) == false)
		{
			// 見つからなかった
			pPage = 0;
			break;
		}

		uiStepCount++;
		pPage = attachPage(uiTmpPageID, uiStepCount);
	}

	return pPage;
}

//
//  FUNCTION public
//  FullText2::BtreeFile::verifyEntryCount -- エントリ数が正しいかチェックする
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::verifyEntryCount()
{
	ModSize count = 0;
	PhysicalFile::PageID uiNextPageID = m_pHeader->m_uiLeftLeafPageID;
	while (uiNextPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		PagePointer pLeafPage = attachPage(uiNextPageID,
										   m_pHeader->m_uiStepCount);
		uiNextPageID = pLeafPage->getNextPageID();

		count += pLeafPage->getCount();

		if (pLeafPage->getID() == m_pHeader->m_uiLeftLeafPageID)
		{
			// 一番左のページなので、前方のページはない
			if (pLeafPage->getPrevPageID() !=
				PhysicalFile::ConstValue::UndefinedPageID)
			{
				_SYDNEY_VERIFY_INCONSISTENT(
					getProgress(), getRootPath(),
					Message::PreviousLinkOfTopPage(pLeafPage->getID()));
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
		}

		if (pLeafPage->getID() == m_pHeader->m_uiRightLeafPageID)
		{
			// 一番右のページなので、後方のページはない
			if (pLeafPage->getNextPageID() !=
				PhysicalFile::ConstValue::UndefinedPageID)
			{
				_SYDNEY_VERIFY_INCONSISTENT(
					getProgress(), getRootPath(),
					Message::NextLinkOfLastPage(pLeafPage->getID()));
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
		}
	}

	if (count != m_pHeader->m_uiCount)
	{
		_SYDNEY_VERIFY_INCONSISTENT(
			getProgress(), getRootPath(),
			Message::IllegalEntryCount(m_pHeader->m_uiCount, count));
		_SYDNEY_THROW0(Exception::VerifyAborted);
	}
}

//
//  FUNCTION private
//  FullText2::BtreeFile::getAverageUsedPageRatio -- 平均ページ使用率を得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  float
//		平均ページ使用率
//
//  EXCEPTIONS
//
float
BtreeFile::getAverageUsedPageRatio()
{
	ModSize totalUnit = 0;
	ModSize usedUnit = 0;

	PagePointer pPage = getRootPage();
	PagePointer pLeftPage = pPage;
	ModSize uiStepCount = 1;

	while (pPage)
	{
		totalUnit += pPage->getPageUnitSize();
		usedUnit += pPage->getUsedUnitSize();
		if (pPage->getNextPageID() == PhysicalFile::ConstValue::UndefinedPageID)
		{
			if (pLeftPage->isLeaf() == false)
			{
				BtreePage::Iterator i = pLeftPage->begin();
				uiStepCount++;
				pLeftPage = attachPage((*i)->m_uiValue, uiStepCount);
				pPage = pLeftPage;
			}
			else
			{
				pPage = 0;
				pLeftPage = 0;
			}
		}
		else
		{
			pPage = attachPage(pPage->getNextPageID(), uiStepCount);
		}
	}

	return static_cast<float>(usedUnit)/static_cast<float>(totalUnit);
}

//
//  Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
