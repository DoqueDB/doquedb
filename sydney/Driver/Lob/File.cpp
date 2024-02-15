// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp --
// 
// Copyright (c) 2003, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
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
#include "Lob/File.h"
#include "Lob/FileID.h"
#include "PhysicalFile/Manager.h"
#include "Version/File.h"
#include "Os/File.h"
#include "LogicalFile/Estimate.h"
#include "Admin/Verification.h"
#include "Exception/VerifyAborted.h"
#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_LOB_USING

//
//	FUNCTION public
//	Inverted::File::File -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	int iCacheCount_
//		キャッシュしておく数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
File::File(int iCacheCount_)
	: m_pPhysicalFile(0), m_pTransaction(0),
	  m_eFixMode(Buffer::Page::FixMode::Unknown),
	  m_bVerification(false), m_pProgress(0), m_pFreeList(0),
	  m_iCacheCount(iCacheCount_), m_iCurrentCacheCount(0)
{
}

//
//	FUNCTION public
//	Lob::File::~File -- デストラクタ
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
File::~File()
{
	; _SYDNEY_ASSERT(m_pFreeList == 0);
	; _SYDNEY_ASSERT(m_mapPage.getSize() == 0);

	// 物理ファイルをデタッチする
	detach();
}

//
//	FUNCTION public
//	Lob::File::attach -- 物理ファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	const Lob::FileID& cFileID_
//		転置ファイルパラメータ
//	int iPageSize_
//		ページサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::attach(const FileID& cFileID_, int iPageSize_)
{
	PhysicalFile::File::StorageStrategy cStorageStrategy;
	PhysicalFile::File::BufferingStrategy cBufferingStrategy;

	//
	//	物理ファイル格納戦略を設定する
	//

	// 物理ファイルの空き領域管理機能
	cStorageStrategy.m_PhysicalFileType = PhysicalFile::PageManageType;
	// マウントされているか
	cStorageStrategy.m_VersionFileInfo._mounted = cFileID_.isMounted();
	// 読み取り専用か
	cStorageStrategy.m_VersionFileInfo._readOnly = cFileID_.isReadOnly();
	// ページサイズ
	cStorageStrategy.m_VersionFileInfo._pageSize = iPageSize_;

	// パスをコピー
	m_cPath = cFileID_.getPath();

	// マスタデータファイルの親ディレクトリの絶対パス名
	cStorageStrategy.m_VersionFileInfo._path._masterData = m_cPath;
	if (cFileID_.isTemporary() == false)
	{
		// バージョンログファイルの親ディレクトリの絶対パス名
		cStorageStrategy.m_VersionFileInfo._path._versionLog = m_cPath;
		// 同期ログファイルの親ディレクトリの絶対パス名
		cStorageStrategy.m_VersionFileInfo._path._syncLog = m_cPath;
	}

	// マスタデータファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._sizeMax._masterData
		= PhysicalFile::ConstValue::DefaultFileMaxSize;
	// バージョンログファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._sizeMax._versionLog
		= PhysicalFile::ConstValue::DefaultFileMaxSize;
	// 同期ログファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._sizeMax._syncLog
		= PhysicalFile::ConstValue::DefaultFileMaxSize;

	// マスタデータファイルのエクステンションサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._extensionSize._masterData
		= PhysicalFile::ConstValue::DefaultFileExtensionSize;
	// バージョンログファイルのエクステンションサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._extensionSize._versionLog
		= PhysicalFile::ConstValue::DefaultFileExtensionSize;
	// 同期ログファイルのエクステンションサイズ(B 単位)
	cStorageStrategy.m_VersionFileInfo._extensionSize._syncLog
		= PhysicalFile::ConstValue::DefaultFileExtensionSize;

	
	//
	//	物理ファイルバッファリング戦略を設定する
	//
	if (cFileID_.isTemporary())
	{
		// 一時なら
		cBufferingStrategy.m_VersionFileInfo._category
			= Buffer::Pool::Category::Temporary;
	}
	else if (cFileID_.isReadOnly())
	{
		// 読み取り専用なら
		cBufferingStrategy.m_VersionFileInfo._category
			= Buffer::Pool::Category::ReadOnly;
	}
	else
	{
		// その他
		cBufferingStrategy.m_VersionFileInfo._category
			= Buffer::Pool::Category::Normal;
	}


	// 物理ファイルをアタッチする
	m_pPhysicalFile = PhysicalFile::Manager::attachFile(cStorageStrategy,
														cBufferingStrategy,
														cFileID_.getLockName());
}

//
//	FUNCTION public
//	Lob::File::detach -- 物理ファイルをデタッチする
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
File::detach()
{
	if (m_pPhysicalFile)
	{
		PhysicalFile::Manager::detachFile(m_pPhysicalFile);
		m_pPhysicalFile = 0;
	}
}

//
//	FUNCTION public
//	Lob::File::create -- ファイルを作成する
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
File::create()
{
	// 物理ファイルに作成を依頼する
	m_pPhysicalFile->create(*m_pTransaction);
}

//
//	FUNCTION public
//	Lob::File::destroy -- ファイルを削除する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::destroy(const Trans::Transaction& cTransaction_)
{
	// ファイルを削除する
	m_pPhysicalFile->destroy(cTransaction_);
}

//
//	FUNCTION public
//	Lob::File::open -- オープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	LogicalFile::OpenOption::OpenMode::Value eOpenMode_
//		オープンモード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::open(const Trans::Transaction& cTransaction_,
		   LogicalFile::OpenOption::OpenMode::Value eOpenMode_)
{
	// FixModeを求める
	if (eOpenMode_ == LogicalFile::OpenOption::OpenMode::Update)
	{
		m_eFixMode = Buffer::Page::FixMode::Write
			| Buffer::Page::FixMode::Discardable;
	}
	else if (eOpenMode_ == LogicalFile::OpenOption::OpenMode::Batch)
	{
		m_eFixMode = Buffer::Page::FixMode::Write;
		m_pPhysicalFile->setBatch(true);
	}
	else
	{
		m_eFixMode = Buffer::Page::FixMode::ReadOnly;
	}

	// トランザクションを保存する
	m_pTransaction = &cTransaction_;
}

//
//	FUNCTION public
//	Lob::File::close -- クローズする
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
File::close()
{
	m_eFixMode = Buffer::Page::FixMode::Unknown;
	m_pTransaction = 0;
	m_pPhysicalFile->setBatch(false);
}

//
//	FUNCTION public
//	Lob::File::freePage -- ページを開放する
//
//	NOTES
//	ページを開放する。
//	ただし実際には、ページは flush が呼ばれるまで開放されない。
//
//	ARGUMENTS
//	Lob::Page* pPage_
//		開放するページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::freePage(Page* pPage_)
{
	pPage_->m_bFree = true;
	m_mapPage.erase(pPage_->getID());
	Page* p = m_pFreeList;
	pPage_->m_pNext = p;
	m_pFreeList = pPage_;
}

//
//	FUNCTION public
//	Lob::File::freePage -- ページを開放する
//
//	NOTES
//	ページを開放する。
//	ただし実際には、ページは flush が呼ばれるまで開放されない。
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		開放するページのページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::freePage(PhysicalFile::PageID uiPageID_)
{
	m_vecFreePageID.pushBack(uiPageID_);
}

//
//	FUNCTION public
//	Lob::File::flushAllPages
//		-- デタッチされているすべてのページの内容を確定する
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
File::flushAllPages()
{
	// フリー配列のページを開放する
	ModVector<PhysicalFile::PageID>::Iterator i = m_vecFreePageID.begin();
	for (; i != m_vecFreePageID.end(); ++i)
	{
		m_pPhysicalFile->freePage(*m_pTransaction, *i);
	}
	// フリーリストのページを開放する
	Page* pPage = m_pFreeList;
	while (pPage)
	{
		// ページを開放する
		m_pPhysicalFile->freePage2(*m_pTransaction, pPage->m_pPhysicalPage);

		Page* pSave = pPage;
		pPage = pPage->m_pNext;

		delete pSave;
	}
	m_pFreeList = 0;

	// マップに格納されているすべてのページ内容を確定する
	Map::Iterator j = m_mapPage.begin();
	for (; j != m_mapPage.end(); ++j)
	{
		Page* pPage = (*j).second;

		; _SYDNEY_ASSERT(pPage->m_iReference == 0);

		PhysicalFile::Page::UnfixMode::Value mode
			= pPage->isDirty() ? PhysicalFile::Page::UnfixMode::Dirty
			: PhysicalFile::Page::UnfixMode::NotDirty;

		// デタッチする
		m_pPhysicalFile->detachPage(pPage->m_pPhysicalPage, mode);
		delete pPage;
	}
	m_mapPage.clear();

	m_iCurrentCacheCount = 0;

	m_pPhysicalFile->detachPageAll();
}

//
//	FUNCTION public
//	Lob::File::recoverAllPages
//		-- デタッチされているすべてのページの内容を破棄する
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
File::recoverAllPages()
{
	// フリー配列をクリアする
	m_vecFreePageID.clear();
	// フリーリストのページの内容を破棄する
	Page* pPage = m_pFreeList;
	while (pPage)
	{
		// 内容を破棄する
		m_pPhysicalFile->recoverPage(pPage->m_pPhysicalPage);

		Page* pSave = pPage;
		pPage = pPage->m_pNext;

		delete pSave;
	}
	m_pFreeList = 0;

	// マップに格納されているすべてのページ内容を破棄する
	Map::Iterator i = m_mapPage.begin();
	for (; i != m_mapPage.end(); ++i)
	{
		Page* pPage = (*i).second;

		// 内容を元に戻す
		m_pPhysicalFile->recoverPage(pPage->m_pPhysicalPage);
		delete pPage;
	}
	m_mapPage.clear();

	m_iCurrentCacheCount = 0;
	
	m_pPhysicalFile->recoverPageAll();
}

//
//	FUNCTION public
//	Lob::File::getOverhead -- 1ページを得るコストを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	double
//		1ページを得る秒数
//
//	EXCEPTIONS
//
double
File::getOverhead() const
{
	double cost = static_cast<double>(m_pPhysicalFile->getPageDataSize());
	return cost
		/ LogicalFile::Estimate::getTransferSpeed(LogicalFile::Estimate::File);
}

//
//	FUNCTION public
//	Lob::File::startVerification -- 整合性検査を開始する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	Admin::Verification::Treatment::Value uiTreatment_
//		整合性検査で矛盾を見つけたときの処置を表す値
//	Admin::Verification::Progress& cProgress_
//		整合性検査の経過を表すクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::startVerification(const Trans::Transaction& cTransaction_,
						Admin::Verification::Treatment::Value uiTreatment_,
						Admin::Verification::Progress& cProgress_)
{
	m_pPhysicalFile->startVerification(cTransaction_, uiTreatment_, cProgress_);
	m_pTransaction = &cTransaction_;
	m_uiTreatment = uiTreatment_;
	m_pProgress = &cProgress_;
	m_bVerification = true;

	if (uiTreatment_ & Admin::Verification::Treatment::Correct)
		m_eFixMode = Buffer::Page::FixMode::Write
			| Buffer::Page::FixMode::Discardable;
	else
		m_eFixMode = Buffer::Page::FixMode::ReadOnly;
}

//
//	FUNCTION public
//	Lob::File::endVerification -- 整合性検査を終了する
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
File::endVerification()
{
	m_pPhysicalFile->endVerification(*m_pTransaction, *m_pProgress);
	m_bVerification = false;
	m_pProgress = 0;
	m_pTransaction = 0;
	m_eFixMode = Buffer::Page::FixMode::Unknown;
}

//
//	FUNCTION private
//	Lob::File::attachPhysicalPage -- 物理ページをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		アタッチするページID
//	Buffer::ReplacementPriority::Value eReplacementPriority_
//	   		(default Buffer::ReplacementPriority::Low)
//		プリイオリティー
//
//	RETURN
//	PhysicalFile::Page*
//		アタッチした物理ページ
//
//	EXCEPTIONS
//
PhysicalFile::Page*
File::attachPhysicalPage(PhysicalFile::PageID uiPageID_,
					 Buffer::ReplacementPriority::Value eReplacementPriority_)
{
	if (m_bVerification == true)
	{
		{
			Admin::Verification::Progress cProgress(m_pProgress->getConnection());
			m_pPhysicalFile->notifyUsePage(*m_pTransaction,
										   cProgress, uiPageID_);
			*m_pProgress += cProgress;
			if (cProgress.isGood() != true)
			{
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
		}
		{
			Admin::Verification::Progress cProgress(m_pProgress->getConnection());
			PhysicalFile::Page* pPage
				= m_pPhysicalFile->verifyPage(*m_pTransaction,
											  uiPageID_,
											  m_eFixMode,
											  cProgress);
			*m_pProgress += cProgress;
			if (cProgress.isGood() != true)
			{
				m_pPhysicalFile->detachPage(pPage);
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
			return pPage;
		}
	}

	return m_pPhysicalFile->attachPage(*m_pTransaction,
										uiPageID_,
										m_eFixMode,
										eReplacementPriority_);
}

//
//	FUNCTION private
//	Lob::File::findMap -- マップに格納されているページを取り出す
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		取り出すページのページID
//
//	RETURN
//	Lob::Page*
//		格納されていたページ。存在しなかった場合は0を返す
//
//	EXCEPTIONS
//
Page*
File::findMap(PhysicalFile::PageID uiPageID_)
{
	Page* pPage = 0;
	Map::Iterator i = m_mapPage.find(uiPageID_);
	if (i != m_mapPage.end())
	{
		pPage = (*i).second;
		if (pPage->m_iReference == 0 && pPage->isDirty() == false)
			m_iCurrentCacheCount--;
	}
	return pPage;
}

//
//	FUNCTION private
//	Lob::File::move -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cPath_
//		移動先のディレクトリ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::move(const Trans::Transaction& cTransaction_,
		   const Os::Path& cPath_)
{
	if (Os::Path::compare(m_cPath, cPath_)
		== Os::Path::CompareResult::Unrelated)
	{
		
		// ファイルが一時ファイルか調べる
		const bool temporary =
			(m_pPhysicalFile->getBufferingStrategy().
			 m_VersionFileInfo._category == Buffer::Pool::Category::Temporary);

		// 新しいパス名を設定する
		Version::File::StorageStrategy::Path cPath;
		cPath._masterData = cPath_;
		; _SYDNEY_ASSERT(m_pPhysicalFile);
		if (!temporary) {
			cPath._versionLog = cPath_;
			cPath._syncLog = cPath_;
		}

		m_pPhysicalFile->move(cTransaction_, cPath);

		m_cPath = cPath_;
	}
}

//
//	FUNCTION private
//	Lob::File::attachPage -- attachしたページを登録する
//
//	NOTES
//
//	ARGUMENTS
//	Lob::Page* pPage_
//		attachしたページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::attachPage(Page* pPage_)
{
	pPage_->m_iAttachCounter++;
	m_mapPage.insert(pPage_->getID(), pPage_);
}

//
//	FUNCTION private
//	Lob::File::detachPage -- ページがdirtyじゃなければdetachする
//
//	NOTES
//
//	ARGUMENTS
//	Lob::Page* pPage_
//		デタッチするページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::detachPage(Page* pPage_)
{
	if (pPage_->isDirty() == false)
	{
		m_iCurrentCacheCount++;
		while (m_iCurrentCacheCount > m_iCacheCount)
		{
			// 上限を超えているので、dirtyじゃないページを開放する
			detachNoDirtyPage();
		}
	}
}

//
//	FUNCTION private
//	Lob::File::detachNoDiryPage -- dirtyじゃないページをすべてデタッチする
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
File::detachNoDirtyPage()
{
	// マップに格納されているdirtyじゃないページ内容を確定する
	Map::Iterator i = m_mapPage.begin();
	while (i != m_mapPage.end())
	{
		Page* pPage = (*i).second;
		Map::Iterator s = i;
		++i;

		if (pPage->m_iAttachCounter != 0) pPage->m_iAttachCounter--;

		if (pPage->m_iReference == 0
			&& pPage->isDirty() == false && pPage->m_iAttachCounter == 0)
		{
			// デタッチする
			m_pPhysicalFile->detachPage(pPage->m_pPhysicalPage);
			delete pPage;

			// エントリを削除する
			m_mapPage.erase(s);

			m_iCurrentCacheCount--;
		}
	}
}

//
//	Copyright (c) 2003, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
