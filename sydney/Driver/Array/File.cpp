// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp --
// 
// Copyright (c) 2006, 2007, 2010, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Array";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Array/File.h"
#include "Array/FileID.h"

#include "PhysicalFile/Manager.h"
#include "Version/File.h"
#include "Os/File.h"
#include "LogicalFile/Estimate.h"
#include "Admin/Verification.h"
#include "Common/Assert.h"

#include "Exception/VerifyAborted.h"

_SYDNEY_USING
_SYDNEY_ARRAY_USING

//
//	FUNCTION public
//	Array::File::File -- コンストラクタ
//
//	NOTES
//	コンストラクタ
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
	  m_pFreeList(0), m_pInstanceFreeList(0),
	  m_bVerification(false), m_pProgress(0),
	  m_iCacheCount(iCacheCount_), m_iCurrentCacheCount(0)
{
}

//
//	FUNCTION public
//	Array::File::~File -- デストラクタ
//
//	NOTES
//	デストラクタ
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

	// インスタンスを開放する
	while (m_pInstanceFreeList)
	{
		Page* p = m_pInstanceFreeList;
		m_pInstanceFreeList = p->m_pNext;
		delete p;
	}

	// 物理ファイルをデタッチする
	detach();
}

//
//	FUNCTION public
//	Array::File::attach -- 物理ファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	const Array::FileID& cFileID_
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
File::attach(const FileID& cFileID_,
			 int iPageSize_)
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
//	Array::File::detach -- 物理ファイルをデタッチする
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
//	Array::File::create -- ファイルを作成する
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
	try
	{
		// 物理ファイルに作成を依頼する
		m_pPhysicalFile->create(*m_pTransaction);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// ディレクトリを破棄する
		//
		//【注意】	ディレクトリは
		//			実体である物理ファイルの生成時に
		//			必要に応じて生成されるが、
		//			エラー時には削除されないので、
		//			この関数で削除する必要がある

		rmdir();
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Array::File::destroy -- ファイルを削除する
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
	// まずファイルを削除する
	m_pPhysicalFile->destroy(cTransaction_);
}

//
//	FUNCTION public
//	Array::File::open -- オープンする
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
		m_eFixMode = Buffer::Page::FixMode::Write
						| Buffer::Page::FixMode::Discardable;
	else if (eOpenMode_ == LogicalFile::OpenOption::OpenMode::Batch)
		m_eFixMode = Buffer::Page::FixMode::Write;
	else
		m_eFixMode = Buffer::Page::FixMode::ReadOnly;

	// トランザクションを保存する
	m_pTransaction = &cTransaction_;
}

//
//	FUNCTION public
//	Array::File::close -- クローズする
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
}

//
//	FUNCTION public
//	Array::File::freePage -- ページを開放する
//
//	NOTES
//	ページを開放する。ただし実際には、ページはflushが呼ばれるまで開放されない。
//
//	ARGUMENTS
//	Array::Page* pPage_
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
//	Array::File::flushAllPages
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
	// フリーリストのページを開放する
	Page* pPage = m_pFreeList;
	while (pPage)
	{
		pPage->preFlush();
		// ページを開放する
		m_pPhysicalFile->freePage2(*m_pTransaction, pPage->m_pPhysicalPage);

		Page* pSave = pPage;
		pPage = pPage->m_pNext;

		pushPage(pSave);
	}
	m_pFreeList = 0;

	// マップに格納されているすべてのページ内容を確定する
	Map::Iterator i = m_mapPage.begin();
	for (; i != m_mapPage.end(); ++i)
	{
		Page* pPage = (*i).second;

		; _SYDNEY_ASSERT(pPage->m_iReference == 0);

		// 確定前処理
		if (pPage->isDirty() == true)
			pPage->preFlush();

		PhysicalFile::Page::UnfixMode::Value mode
			= pPage->isDirty() ? PhysicalFile::Page::UnfixMode::Dirty
			: PhysicalFile::Page::UnfixMode::NotDirty;

		// デタッチする
		m_pPhysicalFile->detachPage(pPage->m_pPhysicalPage, mode);
		pushPage(pPage);
	}
	m_mapPage.erase(m_mapPage.begin(), m_mapPage.end());

	m_iCurrentCacheCount = 0;

	m_pPhysicalFile->detachPageAll();
}

//
//	FUNCTION public
//	Array::File::recoverAllPages
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
	// フリーリストのページの内容を破棄する
	Page* pPage = m_pFreeList;
	while (pPage)
	{
		// 内容を破棄する
		m_pPhysicalFile->recoverPage(pPage->m_pPhysicalPage);

		Page* pSave = pPage;
		pPage = pPage->m_pNext;

		pushPage(pSave);
	}
	m_pFreeList = 0;

	// マップに格納されているすべてのページ内容を破棄する
	Map::Iterator i = m_mapPage.begin();
	for (; i != m_mapPage.end(); ++i)
	{
		Page* pPage = (*i).second;

		// 内容を元に戻す
		m_pPhysicalFile->recoverPage(pPage->m_pPhysicalPage);
		pushPage(pPage);
	}
	m_mapPage.erase(m_mapPage.begin(), m_mapPage.end());

	m_iCurrentCacheCount = 0;
	
	m_pPhysicalFile->recoverPageAll();
}

//
//	FUNCTION public
//	Array::File::getCost -- 1ページを得るコストを得る
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
File::getCost() const
{
	double cost = static_cast<double>(getPageDataSize());
	return cost
		/ LogicalFile::Estimate::getTransferSpeed(LogicalFile::Estimate::File);
}

//
//	FUNCTION public
//	Array::File::rmdir -- ディレクトリを削除する
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
File::rmdir()
{
	Os::Directory::remove(m_cPath);
}

//
//	FUNCTION public
//	Array::File::startVerification -- 整合性検査を開始する
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
//	Array::File::endVerification -- 整合性検査を終了する
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
//	FUNCTION public
//	Array::File::move -- ファイルを移動する
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
//	FUNCTION protected
//	Array::File::attachPhysicalPage -- 物理ページをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		アタッチするページID
//	Buffer::Page::FixMode::Value eFixMode_
//		FIXモード(default Buffer::Page::FixMode::Unknown)
//
//	RETURN
//	PhysicalFile::Page*
//		アタッチした物理ページ
//
//	EXCEPTIONS
//
PhysicalFile::Page*
File::attachPhysicalPage(PhysicalFile::PageID uiPageID_,
						 Buffer::Page::FixMode::Value eFixMode_)
{
	// Get fix mode.
	if (eFixMode_ == Buffer::Page::FixMode::Unknown) eFixMode_ = m_eFixMode;
	if (uiPageID_ == 0)
	{
		// ヘッダーページはDiscardableじゃない
		eFixMode_ &= ~Buffer::Page::FixMode::Discardable;
	}

	// For verify
	if (m_bVerification == true)
	{
		PhysicalFile::Page* pPage = 0;
		
		// PhysicalFile::notifyUsePage 内でバージョンページを fix しないように
		// attach してから notifyUsePage を呼び出す
		
		{
			Admin::Verification::Progress
				cProgress(m_pProgress->getConnection());
			pPage = m_pPhysicalFile->verifyPage(*m_pTransaction,
												uiPageID_,
												eFixMode_,
												cProgress);
			*m_pProgress += cProgress;
			if (cProgress.isGood() != true)
			{
				if (pPage) m_pPhysicalFile->detachPage(pPage);
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
		}
		{
			Admin::Verification::Progress
				cProgress(m_pProgress->getConnection());
			m_pPhysicalFile->notifyUsePage(*m_pTransaction,
										   cProgress,
										   uiPageID_);
			*m_pProgress += cProgress;
			if (cProgress.isGood() != true)
			{
				m_pPhysicalFile->detachPage(pPage);
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
		}
		
		return pPage;
	}

	// Attach PhysicalFile page.
	return m_pPhysicalFile->attachPage(*m_pTransaction,
									   uiPageID_,
									   eFixMode_);
}

//
//	FUNCTION protected
//	Array::File::changeFixMode -- 物理ページのFixModeを変更する
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
File::changeFixMode(PhysicalFile::Page* pPage_)
{
	// まずは、変更前のページをdetachする
	PhysicalFile::PageID uiPageID = pPage_->getID();
	m_pPhysicalFile->detachPage(pPage_);

	// 新しくattachする
	return attachPhysicalPage(uiPageID, getFixMode());
}

//
//	FUNCTION protected
//	Array::File::findMap -- マップに格納されているページを取り出す
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		取り出すページのページID
//
//	RETURN
//	Array::Page*
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
//	FUNCTION protected
//	Array::File::attachPage -- attachしたページを登録する
//
//	NOTES
//
//	ARGUMENTS
//	Array::Page* pPage_
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
	if (m_eFixMode != Buffer::Page::FixMode::ReadOnly)
	{
		pPage_->m_iAttachCounter++;
		m_mapPage.insert(pPage_->getID(), pPage_);
	}
}

//
//	FUNCTION protected
//	Array::File::detachPage -- 
//
//	NOTES
//
//	ARGUMENTS
//	Array::Page* pPage_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::detachPage(Page* pPage_)
{
	if (m_eFixMode != Buffer::Page::FixMode::ReadOnly)
	{
		// Detaching Non-ReadOnly page is delaied as long as possible.
		
		if (pPage_->isDirty() == false)
		{
			// For NOT dirty page
			
			m_iCurrentCacheCount++;
			while (m_iCurrentCacheCount > m_iCacheCount)
			{
				// 上限を超えているので、dirtyじゃないページを開放する
				detachNoDirtyPage();
			}
		}
	}
	else
	{
		// ReadOnly page is detached now.
		
		m_pPhysicalFile->detachPage(pPage_->m_pPhysicalPage);
		pushPage(pPage_);
	}
}

//
//	FUNCTION public
//	Array::File::popPage -- インスタンスを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Array::Page*
//		インスタンス
//
//	EXCETPIONS
//
Page*
File::popPage()
{
	Page* p = m_pInstanceFreeList;
	if (p)
	{
		m_pInstanceFreeList = p->m_pNext;
		p->m_pNext = 0;
	}
	return p;
}

//
//	FUNCTION protected
//	Array::File::pushPage -- インスタンスをつなげる
//
//	NOTES
//
//	ARGUMENTS
//	Array::Page* pPage_
//		インスタンス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::pushPage(Page* pPage_)
{
	pPage_->m_pNext = m_pInstanceFreeList;
	m_pInstanceFreeList = pPage_;
}

//
//	FUNCTION private
//	Array::File::detachNoDiryPage -- dirtyじゃないページをすべてデタッチする
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
			pushPage(pPage);

			// エントリを削除する
			m_mapPage.erase(s);

			m_iCurrentCacheCount--;
		}
	}
}

//
//	Copyright (c) 2006, 2007, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
