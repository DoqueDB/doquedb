// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreeFile.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Inverted/BtreeFile.h"
#include "Inverted/BtreePage.h"
#include "Inverted/Parameter.h"
#include "Inverted/FakeError.h"
#include "Inverted/MessageAll_Class.h"

#include "LogicalFile/Estimate.h"

#include "Common/Assert.h"

#include "Exception/EntryNotFound.h"
#include "Exception/VerifyAborted.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

namespace
{
	//
	//  VARIABLE
	//
	const Os::Ucs2 _pszPath[] = {'B','t','r','e','e',0};

	//
	//  VARIABLE
	//
	ParameterInteger _cBtreeCachePageSize("Inverted_BtreeCachePageSize", 15);
}

//
//  FUNCTION public
//  Inverted::BtreeFile::BtreeFile -- コンストラクタ
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
//  なし
//
BtreeFile::BtreeFile(const FileID& cFileID_,
						const Os::Path& cFilePath_, bool batch_)
						: File(Type::Btree, _cBtreeCachePageSize.get())
{
	// 物理ファイルをアタッチする
	attach(cFileID_,
			cFileID_.getBtreePageSize(),
			cFilePath_,
			Os::Path(_pszPath),
			batch_);
}

//
//  FUNCTION public
//  Inverted::BtreeFile::~BtreeFile -- デストラクタ
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
//  Inverted::BtreeFile::create -- ファイルを作成する
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
	File::create();
	try
	{
	// ヘッダーを初期化する
		initializeHeaderPage();
	}
	catch (...)
	{
		recoverAllPages();
		File::destroy();
		_SYDNEY_RETHROW;
	}
}

//
//  FUNCTION public
//  Inverted::BtreeFile::verify -- 整合性検査を行う
//
//  NOTES
//  B木の整合性検査は以下の項目をチェックする
//  1. ルートノードからすべてのページをたどり、使用しているページIDを物理ファイルに通知する
//  2. すべてのページのエントリが昇順に格納されているかチェックする
//  3. リーフページを横に走査し、ヘッダーに記録されているエントリ数との整合性をチェックする
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
	// ページの整合性検査を行う(1と2が同時にチェックされる)
	pRootPage->verify();
	// リーフページの総数とヘッダーが同じかどうかチェックする
	verifyEntryCount();
}

//
//  FUNCTION public
//  Inverted::BtreeFile::move -- ファイルを移動する
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//		  トランザクション
//  const Os::Path& cFilePath_
//		  パス
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::move(const Trans::Transaction& cTransaction_,
					const Os::Path& cFilePath_)
{
	File::move(cTransaction_, cFilePath_, Os::Path(_pszPath));
}

//
//  FUNCTION public
//  Inverted::BtreeFile::clear -- ページをクリアする
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//		  トランザクション
//  bool bForce_
//		  強制モードかどうか
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::clear(const Trans::Transaction& cTransaction_, bool bForce_)
{
	// まず下位を呼ぶ
	File::clear(cTransaction_, bForce_);
	// ヘッダーページを初期化する
	initializeHeaderPage();
}

//
//  FUNCTION public
//  Inverted::BtreeFile::search -- 引数以下で最大のキー値をもつエントリを検索する
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeChar* pszKey_
//		  検索するキー値
//
//  RETURN
//  PhysicalFile::PageID
//		  リーフファイルのページID。存在しなかった場合はUndefinedPageID
//
//  EXCEPTIONS
//
PhysicalFile::PageID
BtreeFile::search(const ModUnicodeChar* pszKey_)
{
	BtreePage::AutoEntry pEntry(BtreePage::allocateEntry(pszKey_));

	// まずはリーフページを得る
	PagePointer pLeafPage = getLeafPage(*pEntry);
	if (pLeafPage == 0)
	{
		; _SYDNEY_ASSERT(0);
		// リーフページがない
		return PhysicalFile::ConstValue::UndefinedPageID;
	}

	// リーフの中を検索する
	return pLeafPage->search(*pEntry);
}

//
//  FUNCTION public
//  Inverted::BtreeFile::insert -- 挿入する
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeChar* pszKey_
//		  キー値
//  PhysicalFile::PageID uiPageID_
//		  バリュー
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::insert(const ModUnicodeChar* pszKey_, PhysicalFile::PageID uiPageID_)
{
	BtreePage::AutoEntry pEntry(BtreePage::allocateEntry(pszKey_, uiPageID_));

	// まずはリーフページまで検索する
	PagePointer pLeafPage = getLeafPage(*pEntry);
	if (pLeafPage == 0)
	{
		// リーフファイルがない見つからない -> 挿入されているキーより小さなキー
		if (m_pHeader->m_uiLeftLeafPageID != PhysicalFile::ConstValue::UndefinedPageID)
		{
			// 一番左のリーフページをattachする
			pLeafPage = attachPage(m_pHeader->m_uiLeftLeafPageID, m_pHeader->m_uiStepCount);
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
//  Inverted::BtreeFile::expunge -- 削除する
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeChar* pszKey_
//		  削除するエントリのキー値
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
//  Inverted::BtreeFile::update -- 更新する
//
//  NOTES
//
//  ARGUMENTS
//  const ModUnicodeChar* pszKey1_
//		  更新するエントリのキー値
//  PhysicalFile::PageID uiPageID1_
//		  更新するエントリのバリュー値
//  const ModUnicodeChar* pszKey2_
//		  更新後のエントリのキー値
//  PhysicalFile::PageID uiPageID2_
//		  更新後のエントリのバリュー値
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
BtreeFile::update(const ModUnicodeChar* pszKey1_, PhysicalFile::PageID uiPageID1_,
					const ModUnicodeChar* pszKey2_, PhysicalFile::PageID uiPageID2_)
{
	BtreePage::AutoEntry pEntry1(BtreePage::allocateEntry(pszKey1_, uiPageID1_));
	BtreePage::AutoEntry pEntry2(BtreePage::allocateEntry(pszKey2_, uiPageID2_));

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
//  Inverted::BtreeFile::attachPage -- ページをアタッチする
//
//  NOTES
//
//  ARGUMENTS
//  PhysicalFile::PageID uiPageID_
//		  アタッチするページのID
//  ModSize uiStepCount_
//		  段数
//
//  RETURN
//  Inverted::BtreeFile::PagePointer
//		  アタッチしたページ
//
//  EXCEPTIONS
//
BtreeFile::PagePointer
BtreeFile::attachPage(PhysicalFile::PageID uiPageID_,
												ModSize uiStepCount_)
{
	// まずこれまでattachされているかどうかマップを検索する
	PagePointer pPage
		= _SYDNEY_DYNAMIC_CAST(BtreePage*, File::findMap(uiPageID_));
	if (pPage == 0)
	{
		; _INVERTED_FAKE_ERROR(BtreeFile::attachPage);
		// 無かったので、新たに確保する
		PhysicalFile::Page* pPhysicalPage = attachPhysicalPage(
				uiPageID_, Buffer::ReplacementPriority::Middle);
		BtreePage* p
					= _SYDNEY_DYNAMIC_CAST(BtreePage*, File::popInstanceList());
		if (p)
		{
			p->reset(pPhysicalPage);
			pPage = p;
		}
		else
		{
			pPage = new BtreePage(*this, pPhysicalPage);
		}
		File::attachPage(pPage);
	}

	pPage->setStepCount(uiStepCount_);

	return pPage;
}

//
//  FUNCTION public
//  Inverted::BtreeFile::allocatePage -- 新しいページを確保する
//
//  NOTES
//
//  ARGUMENTS
//  PhysicalFile::PageID uiPrevPageID_
//		  前方のページID
//  PhysicalFile::PageID uiNextPageID_
//		  後方のページID
//  ModSize uiStepCount_
//		  段数
//
//  RETURN
//  Inverted::BtreeFile::PagePointer
//		  新しく確保したページ
//
//  EXCEPTIONS
//
BtreeFile::PagePointer
BtreeFile::allocatePage(PhysicalFile::PageID uiPrevPageID_,
						PhysicalFile::PageID uiNextPageID_,
						ModSize uiStepCount_)
{
	PagePointer pPage = _SYDNEY_DYNAMIC_CAST(BtreePage*, File::getFreePage());
	if (pPage)
	{
		pPage->reset(uiPrevPageID_, uiNextPageID_);
	}
	else
	{
		; _INVERTED_FAKE_ERROR(BtreeFile::allocatePage);
		// 新たに確保する
		PhysicalFile::Page* pPhysicalPage = File::allocatePage();
		pPage = _SYDNEY_DYNAMIC_CAST(BtreePage*, File::popInstanceList());
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

	File::attachPage(pPage);

	return pPage;
}

//
//  FUNCTION public
//  Inverted::BtreeFile::setRootPage -- ルートページを設定する
//
//  NOTES
//
//  ARGUMENTS
//  Inverted::BtreeFile::PagePointer pRootPage_
//		  設定するルートページ
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
//  Inverted::BtreeFile::setRightLeafPage -- 一番右のリーフページを設定する
//
//  NOTES
//
//  ARGUMENTS
//  Inverted::BtreeFile::PagePointer pRootPage_
//		  設定するリーフページ
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
//  Inverted::BtreeFile::setLeftLeafPage -- 一番左のリーフページを設定する
//
//  NOTES
//
//  ARGUMENTS
//  Inverted::BtreeFile::PagePointer pRootPage_
//		  設定するリーフページ
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
//  Inverted::BtreeFile::flushAllPages -- 内容をフラッシュする
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
	File::flushAllPages();
}

//
//  FUNCTION public
//  Inverted::BtreeFile::recoverAllPages -- 変更内容を元に戻す
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
	File::recoverAllPages();
}

//
//  FUNCTION public
//  Inverted::BtreeFile::saveAllPages -- 変更内容を保存する
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
	File::saveAllPages();
}

//
//  FUNCTION public
//  Inverted::BtreeFile::getNodePage -- 該当するノードページを得る
//
//  NOTES
//
//  ARGUMENTS
//  const Inverted::BtreePage::Entry& cEntry_
//		  ノードページを検索するためのキー値を含んだもの
//  ModSize uiStepCount_
//		  何段目のノードページを得るか
//
//  RETURN
//  Inverted::BtreeFile::PagePointer
//		  見つかったノードページ。エントリが１つも存在していなかった場合には0が返る
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
		PhysicalFile::PageID uiTmpPageID = pPage->search(cEntry_);

		// 下位ノードを得る
		uiStepCount++;
		pPage = attachPage(uiTmpPageID, uiStepCount);
	}

	return pPage;
}

//
//  FUNCTION public
//  Inverted::BtreeFile::getEntryCount -- エントリ数を得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModSize
//		  エントリ数
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
//  Inverted::BtreeFile::reportFile -- ファイル状態を出力する
//
//  NOTES
//
//  ARGUMETNS
//  const Trans::Transaction& cTransaction_
//		  トランザクション
//  ModOstream& stream_
//		  出力ストリーム
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
//  Inverted::BtreeFile::initializeHeaderPage -- ヘッダーページを初期化する
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
	PhysicalFile::Page* pPhysicalPage = File::allocatePage();
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
//  Inverted::BtreeFile::setHeader -- ヘッダーを割り当てる
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
		File::attachPage(m_pHeaderPage);

		// ヘッダーを割り当てる
		m_pHeader = syd_reinterpret_cast<Header*>(m_pHeaderPage->getBuffer());
	}
}

//
//  FUNCTION private
//  Inverted::BtreeFile::getRootPage -- ルートページを得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  Inverted::BtreeFile::PagePointer
//		  ルートページ
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
//  Inverted::BtreeFile::getLeafPage -- 該当するリーフページを得る
//
//  NOTES
//
//  ARGUMENTS
//  const Inverted::BtreePage::Entry& cEntry_
//		  リーフページを検索するためのキー値を含んだもの
//
//  RETURN
//  Inverted::BtreeFile::PagePointer
//		  見つかったリーフページ。エントリが１つも存在していなかった場合には0が返る
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
		PhysicalFile::PageID uiTmpPageID = pPage->search(cEntry_);
		if (uiTmpPageID == PhysicalFile::ConstValue::UndefinedPageID)
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
//  Inverted::BtreeFile::verifyEntryCount -- エントリ数が正しいかチェックする
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
		PagePointer pLeafPage = attachPage(uiNextPageID, m_pHeader->m_uiStepCount);
		uiNextPageID = pLeafPage->getNextPageID();

		count += pLeafPage->getCount();

		if (pLeafPage->getID() == m_pHeader->m_uiLeftLeafPageID)
		{
			// 一番左のページなので、前方のページはない
			if (pLeafPage->getPrevPageID() != PhysicalFile::ConstValue::UndefinedPageID)
			{
				_SYDNEY_VERIFY_INCONSISTENT(getProgress(), getRootPath(), Message::PreviousLinkOfTopPage(pLeafPage->getID()));
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
		}

		if (pLeafPage->getID() == m_pHeader->m_uiRightLeafPageID)
		{
			// 一番右のページなので、後方のページはない
			if (pLeafPage->getNextPageID() != PhysicalFile::ConstValue::UndefinedPageID)
			{
				_SYDNEY_VERIFY_INCONSISTENT(getProgress(), getRootPath(), Message::NextLinkOfLastPage(pLeafPage->getID()));
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
		}
	}

	if (count != m_pHeader->m_uiCount)
	{
		_SYDNEY_VERIFY_INCONSISTENT(getProgress(), getRootPath(), Message::IllegalEntryCount(m_pHeader->m_uiCount, count));
		_SYDNEY_THROW0(Exception::VerifyAborted);
	}
}

//
//  FUNCTION private
//  Inverted::BtreeFile::getAverageUsedPageRatio -- 平均ページ使用率を得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  float
//		  平均ページ使用率
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
				pLeftPage = attachPage((*i)->m_uiPageID, uiStepCount);
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
//  Copyright (c) 2002, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
