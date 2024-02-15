// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2009, 2010, 2023 Ricoh Company, Ltd.
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
#include "Inverted/File.h"
#include "Inverted/FileID.h"

#include "Common/Assert.h"
#include "PhysicalFile/Manager.h"
#include "Version/File.h"
#include "Os/File.h"
#include "LogicalFile/Estimate.h"
#include "Admin/Verification.h"
#include "Exception/VerifyAborted.h"
#include "Exception/Cancel.h"

#include "ModOsDriver.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//  FUNCTION public
//  Inverted::File::File -- コンストラクタ
//
//  NOTES
//  コンストラクタ。
//
//  ARGUMENTS
//  Inverted::File::Type::Value eType_
//	  転置ファイル種別
//  int iCacheCount_
//	  キャッシュしておく数
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
File::File(Type::Value eType_, int iCacheCount_)
	: m_cPageList(&Page::_next, &Page::_prev),
		m_pPhysicalFile(0), m_eType(eType_), m_pTransaction(0),
		m_eFixMode(Buffer::Page::FixMode::Unknown),
		m_bVerification(false), m_pProgress(0),
		m_pFreeList(0), m_pInstanceList(0),
		m_iCacheCount(iCacheCount_), m_iCurrentCacheCount(0),
		m_iFreeInstanceCount(0),
		m_iAttachCount(0)
{
	m_cPageMap.reserve(m_iCacheCount * 2);
}

//
//  FUNCTION public
//  Inverted::File::~File -- デストラクタ
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
File::~File()
{
	; _SYDNEY_ASSERT(m_pFreeList == 0);
	; _SYDNEY_ASSERT(m_cPageList.getSize() == 0);

	// インスタンスリストを開放する
	while (m_pInstanceList)
	{
		Page* p = m_pInstanceList;
		m_pInstanceList = p->m_pNext;
		delete p;
	}

	// 物理ファイルをデタッチする
	detach();
}

//
//  FUNCTION public
//  Inverted::File::attach -- 物理ファイルをアタッチする
//
//  NOTES
//
//  ARGUMENTS
//  const Inverted::FileID& cFileID_
//	  転置ファイルパラメータ
//  int iPageSize_
//	  ページサイズ
//  const Os::Path& cFilePath_
//	  格納ディレクトリパート
//  const Os::Path& cPart_
//	  格納ディレクトリ
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
File::attach(const FileID& cFileID_,
			 int iPageSize_,
			 const Os::Path& cFilePath_,
			 const Os::Path& cPart_,
			 bool batch_)
{
	PhysicalFile::File::StorageStrategy cStorageStrategy;
	PhysicalFile::File::BufferingStrategy cBufferingStrategy;

	//
	//  物理ファイル格納戦略を設定する
	//

	// 物理ファイルの空き領域管理機能
	cStorageStrategy.m_PhysicalFileType = PhysicalFile::PageManageType;
	// マウントされているか
	cStorageStrategy.m_VersionFileInfo._mounted = cFileID_.isMounted();
	// 読み取り専用か
	cStorageStrategy.m_VersionFileInfo._readOnly = cFileID_.isReadOnly();
	// ページサイズ
	cStorageStrategy.m_VersionFileInfo._pageSize = iPageSize_;

	m_cRootPath = cFileID_.getPath();

	m_cPath = cFilePath_;
	m_cPath.addPart(cPart_);

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
	//  物理ファイルバッファリング戦略を設定する
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
//  FUNCTION public
//  Inverted::File::detach -- 物理ファイルをデタッチする
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
File::detach()
{
	if (m_pPhysicalFile)
	{
		PhysicalFile::Manager::detachFile(m_pPhysicalFile);
		m_pPhysicalFile = 0;
	}
}

//
//  FUNCTION public
//  Inverted::File::create -- ファイルを作成する
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
File::create()
{
	try
	{
		// 物理ファイルに作成を依頼する
		m_pPhysicalFile->create(*m_pTransaction);
	}
	catch (...)
	{
		// ディレクトリを破棄する
		//
		//【注意】  ディレクトリは
		//		  実体である物理ファイルの生成時に
		//		  必要に応じて生成されるが、
		//		  エラー時には削除されないので、
		//		  この関数で削除する必要がある

		rmdir();
		_SYDNEY_RETHROW;
	}
}

//
//  FUNCTION public
//  Inverted::File::destroy -- ファイルを削除する
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
File::destroy(const Trans::Transaction& cTransaction_)
{
	// まずファイルを削除する
	m_pPhysicalFile->destroy(cTransaction_);
	// ディレクトリを削除する
	rmdir();
}

//
//  FUNCTION public
//  Inverted::File::open -- オープンする
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//  Buffer::Page::FixMode::Value eFixMode_
//	  FIXモード
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
File::open(const Trans::Transaction& cTransaction_,
			 Buffer::Page::FixMode::Value eFixMode_)
{
	// FixModeを保存する
	m_eFixMode = eFixMode_;
	// トランザクションを保存する
	m_pTransaction = &cTransaction_;
}

//
//  FUNCTION public
//  Inverted::File::close -- クローズする
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
File::close()
{
	m_eFixMode = Buffer::Page::FixMode::Unknown;
	m_pTransaction = 0;
}

//
//  FUNCTION public
//  Inverted::File::clear -- クリアする
//
//  NOTES
//  すべてのページ内容をクリアする。
//  継承ファイルでヘッダーページが存在する場合には、
//  継承側で初期化する必要がある。
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//  bool bForce_
//	  強制モードかどうか
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
File::clear(const Trans::Transaction& cTransaction_, bool bForce_)
{
	// 物理ファイルをクリアする
	m_pPhysicalFile->clear(cTransaction_, bForce_);

	// トランザクション
	m_pTransaction = &cTransaction_;
	// FixMode
	m_eFixMode = Buffer::Page::FixMode::Write
		| Buffer::Page::FixMode::Discardable;
}

//
//  FUNCTION public
//  Inverted::File::freePage -- ページを開放する
//
//  NOTES
//  ページを開放する。
//  ただし実際には、ページは flush が呼ばれるまで開放されない。
//
//  ARGUMENTS
//  Inverted::Page* pPage_
//	  開放するページ
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
File::freePage(Page* pPage_)
{
	pPage_->m_bFree = true;

	m_cPageMap.erase(pPage_->getID());
	m_cPageList.erase(*pPage_);

	Page* p = m_pFreeList;
	pPage_->m_pNext = p;
	m_pFreeList = pPage_;
}

//
//  FUNCTION public
//  Inverted::File::flushAllPages
//	  -- デタッチされているすべてのページの内容を確定する
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
File::flushAllPages()
{
	// フリーリストのページを開放する
	Page* pPage = m_pFreeList;
	while (pPage)
	{
		PhysicalFile::PageID id = pPage->getID();
		// ページを開放する
		m_pPhysicalFile->freePage2(*m_pTransaction, pPage->m_pPhysicalPage);

		Page* pSave = pPage;
		pPage = pPage->m_pNext;

		pushInstanceList(pSave, id);
	}
	m_pFreeList = 0;

	// LRUに格納されているすべてのページ内容を確定する
	PageList::Iterator i = m_cPageList.begin();
	while (i != m_cPageList.end())
	{
		Page* pPage = &(*i);
		++i;

		; _SYDNEY_ASSERT(pPage->m_iReference == 0);

		PhysicalFile::Page::UnfixMode::Value mode
			= pPage->isDirty() ? PhysicalFile::Page::UnfixMode::Dirty
			: PhysicalFile::Page::UnfixMode::NotDirty;

		PhysicalFile::PageID id = pPage->getID();
		// デタッチする
		m_pPhysicalFile->detachPage(pPage->m_pPhysicalPage, mode);

		m_cPageList.erase(*pPage);
		pushInstanceList(pPage, id);
	}

	m_cPageMap.erase(m_cPageMap.begin(), m_cPageMap.end());
	m_iCurrentCacheCount = 0;

	m_pPhysicalFile->detachPageAll();
	m_iAttachCount = 0;
}

//
//  FUNCTION public
//  Inverted::File::recoverAllPages
//	  -- デタッチされているすべてのページの内容を破棄する
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
File::recoverAllPages()
{
	// フリーリストのページの内容を破棄する
	Page* pPage = m_pFreeList;
	while (pPage)
	{
		PhysicalFile::PageID id = pPage->getID();
		// 内容を破棄する
		if (pPage->m_pPhysicalPage->getFixMode()
			& Buffer::Page::FixMode::Discardable)
		{
			// discardableなら修正を破棄できる
			m_pPhysicalFile->recoverPage(pPage->m_pPhysicalPage);
		}
		else
		{
			// discardableでないのでコミットしてしまう
			m_pPhysicalFile->detachPage(pPage->m_pPhysicalPage,
										PhysicalFile::Page::UnfixMode::Dirty);
		}

		Page* pSave = pPage;
		pPage = pPage->m_pNext;

		pushInstanceList(pSave, id);
	}
	m_pFreeList = 0;

	// LRUに格納されているすべてのページ内容を破棄する
	PageList::Iterator i = m_cPageList.begin();
	while (i != m_cPageList.end())
	{
		Page* pPage = &(*i);
		++i;

		PhysicalFile::PageID id = pPage->getID();
		// 内容を元に戻す
		if (pPage->m_pPhysicalPage->getFixMode()
			& Buffer::Page::FixMode::Discardable)
		{
			// discardableなら修正を破棄できる
			m_pPhysicalFile->recoverPage(pPage->m_pPhysicalPage);
		}
		else
		{
			// discardableでないのでコミットしてしまう
			m_pPhysicalFile->detachPage(pPage->m_pPhysicalPage,
										PhysicalFile::Page::UnfixMode::Dirty);
		}

		m_cPageList.erase(*pPage);
		pushInstanceList(pPage, id);
	}

	m_cPageMap.erase(m_cPageMap.begin(), m_cPageMap.end());
	m_iCurrentCacheCount = 0;

	m_pPhysicalFile->recoverPageAll();
	m_iAttachCount = 0;
}

//
//  FUNCTION public
//  Inverted::File::saveAllPages
//	  -- デタッチされているすべてのページの内容を保存する
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
File::saveAllPages()
{
	// フリーリストのページを開放する
	Page* pPage = m_pFreeList;
	while (pPage)
	{
		PhysicalFile::PageID id = pPage->getID();
		// ページを開放する
		m_pPhysicalFile->freePage2(*m_pTransaction, pPage->m_pPhysicalPage);

		Page* pSave = pPage;
		pPage = pPage->m_pNext;

		pushInstanceList(pSave, id);
	}
	m_pFreeList = 0;

	// マップに格納されているすべてのページ内容を保存する
	PageList::Iterator i = m_cPageList.begin();
	while (i != m_cPageList.end())
	{
		Page* pPage = &(*i);
		++i;

		if (pPage->isDirty())
		{
			// 保存する
			m_pPhysicalFile->savePage(pPage->m_pPhysicalPage, true);
			if (pPage->m_iReference == 0 && pPage->isDirty() == false)
				m_iCurrentCacheCount++;
		}
	}

	if (m_iCurrentCacheCount > m_iCacheCount)
	{
		// 上限を超えているので、dirtyじゃないページを開放する
		detachNoDirtyPage();
	}

	// 物理ファイルのキャッシュを削除する
	m_pPhysicalFile->unfixVersionPage(true);
}

//
//  FUNCTION public
//  Inverted::File::getOverhead -- 1ページを得るコストを得る
//
//  NOTES
//
//  ARGUMENTS
//  ModSize uiPageSize_
//	  1ページのサイズ (byte)
//
//  RETURN
//  double
//	  1ページを得る秒数
//
//  EXCEPTIONS
//
double
File::getOverhead(ModSize uiPageSize_)
{
	double cost = static_cast<double>(uiPageSize_);
	return cost
		/ LogicalFile::Estimate::getTransferSpeed(LogicalFile::Estimate::File);
}

//  FUNCTION public
//  Inverted::File::rmdir -- ディレクトリのみ削除する
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

void
File::rmdir()
{
	ModOsDriver::File::rmAll(m_cPath, ModTrue);
}

//
//  FUNCTION public
//  Inverted::File::startVerification -- 整合性検査を開始する
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//  Admin::Verification::Treatment::Value uiTreatment_
//	  整合性検査で矛盾を見つけたときの処置を表す値
//  Admin::Verification::Progress& cProgress_
//	  整合性検査の経過を表すクラス
//
//  RETURN
//  なし
//
//  EXCEPTIONS
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
//  FUNCTION public
//  Inverted::File::endVerification -- 整合性検査を終了する
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
File::endVerification()
{
	m_pPhysicalFile->endVerification(*m_pTransaction, *m_pProgress);
	m_bVerification = false;
	m_pProgress = 0;
	m_pTransaction = 0;
	m_eFixMode = Buffer::Page::FixMode::Unknown;
}

//
//  FUNCTION protected
//  Inverted::File::attachPhysicalPage -- 物理ページをアタッチする
//
//  NOTES
//
//  ARGUMENTS
//  PhysicalFile::PageID uiPageID_
//	  アタッチするページID
//  Buffer::ReplacementPriority::Value eReplacementPriority_
//  (default Buffer::ReplacementPriority::Low)
//	  プリイオリティー
//
//  RETURN
//  PhysicalFile::Page*
//	  アタッチした物理ページ
//
//  EXCEPTIONS
//
PhysicalFile::Page*
File::attachPhysicalPage(
	PhysicalFile::PageID uiPageID_,
	Buffer::ReplacementPriority::Value eReplacementPriority_)
{
	if ((++m_iAttachCount % 100) == 0)
	{
		// 100回物理ページをattachしたら１回中断要求をチェックする
		if (m_pTransaction->isCanceledStatement() == true)
		{
			// 中断要求が来ているので例外を投げる
			_SYDNEY_THROW0(Exception::Cancel);
		}
	}

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
												m_eFixMode,
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
			m_pPhysicalFile->notifyUsePage(
				*m_pTransaction, cProgress, uiPageID_);
			*m_pProgress += cProgress;
			if (cProgress.isGood() != true)
			{
				m_pPhysicalFile->detachPage(pPage);
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
		}
		
		return pPage;
	}

	return m_pPhysicalFile->attachPage(*m_pTransaction,
										uiPageID_,
										m_eFixMode,
										eReplacementPriority_);
}

//
//  FUNCTION protected
//  Inverted::File::findMap -- マップに格納されているページを取り出す
//
//  NOTES
//
//  ARGUMENTS
//  PhysicalFile::PageID uiPageID_
//	  取り出すページのページID
//
//  RETURN
//  Inverted::Page*
//	  格納されていたページ。存在しなかった場合は0を返す
//
//  EXCEPTIONS
//
Page*
File::findMap(PhysicalFile::PageID uiPageID_)
{
	Page* pPage = 0;
	PageMap::Iterator i = m_cPageMap.find(uiPageID_);
	if (i != m_cPageMap.end())
	{
		// 見つかった
		pPage = (*i).second;
		if (pPage->m_iReference == 0 && pPage->isDirty() == false)
			--m_iCurrentCacheCount;

		// 最後尾に移動する
		PageList::Iterator position(m_cPageList.end());
		PageList::Iterator ite(m_cPageList, pPage);
		m_cPageList.splice(position, m_cPageList, ite);
	}

	return pPage;
}

//  FUNCTION protected
//  Inverted::File::move -- ファイルを移動する
//
//  NOTES
//
//  ARGUMENTS
//  const Trans::Transaction& cTransaction_
//	  トランザクション
//  const Os::Path& cPath_
//	  移動先のディレクトリ
//  const Os::Path& cDirectory_
//	  その下のディレクトリ
//
//  RETURN
//  なし
//
//  EXCEPTIONS

void
File::move(const Trans::Transaction& cTransaction_,
			 const Os::Path& cPath_, const Os::Path& cDirectory_)
{
	Os::Path cTmp = cPath_;
	cTmp.addPart(cDirectory_);

	if (Os::Path::compare(m_cPath, cTmp)
		== Os::Path::CompareResult::Unrelated)
	{

		// 実体である OS ファイルが存在するか調べる

		const bool accessible = isAccessible();

		// ファイルが一時ファイルか調べる

		const bool temporary =
			(m_pPhysicalFile->getBufferingStrategy().
			 m_VersionFileInfo._category == Buffer::Pool::Category::Temporary);

		Version::File::StorageStrategy::Path cPath;
		cPath._masterData = cTmp;
		; _SYDNEY_ASSERT(m_pPhysicalFile);
		if (!temporary) {
			cPath._versionLog = cTmp;
			cPath._syncLog = cTmp;
		}

		int step = 0;
		try
		{
			m_pPhysicalFile->move(cTransaction_, cPath);
			step++;

			if (accessible)

				// 古いディレクトリを削除する

				rmdir();
			step++;
		}
		catch (...)
		{
			switch (step)
			{
			case 1:
				{
					Version::File::StorageStrategy::Path cPath;
					cPath._masterData = m_cPath;
					if (!temporary) {
						cPath._versionLog = m_cPath;
						cPath._syncLog = m_cPath;
					}
					m_pPhysicalFile->move(cTransaction_, cPath);
				}
			case 0:
				if (accessible)

					// ディレクトリを破棄する
					//
					//【注意】  ディレクトリは
					//		  実体である物理ファイルの移動時に
					//		  必要に応じて生成されるが、
					//		  エラー時には削除されないので、
					//		  この関数で削除する必要がある

					Os::Directory::remove(cTmp);
			}
			_SYDNEY_RETHROW;
		}

		m_cPath = cTmp;
	}
}

//
//  FUNCTION protected
//  Inverted::File::attachPage -- attachしたページを登録する
//
//  NOTES
//
//  ARGUMENTS
//  Inverted::Page* pPage_
//	  attachしたページ
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
File::attachPage(Page* pPage_)
{
	// LRUリストには存在していないもの

	m_cPageList.pushBack(*pPage_);
	m_cPageMap.insert(pPage_->getID(), pPage_);
}

//
//  FUNCTION protected
//  Inverted::File::detachPage -- ページがdirtyじゃなければdetachする
//
//  NOTES
//
//  ARGUMENTS
//  Inverted::Page* pPage_
//	  デタッチするページ
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
File::detachPage(Page* pPage_)
{
	// まずは最後尾に移動する
	PageList::Iterator position(m_cPageList.end());
	PageList::Iterator ite(m_cPageList, pPage_);
	m_cPageList.splice(position, m_cPageList, ite);

	if (pPage_->isDirty() == false)
	{
		++m_iCurrentCacheCount;
		if (m_iCurrentCacheCount > m_iCacheCount)
		{
			detachNoDirtyPage();
		}
	}
}

//
//  FUNCTION protected
//  Inverted::File::savePage -- ページの内容を保存する
//
//  NOTES
//
//  ARGUMENTS
//  Inverted::Page* pPage_
//	  保存するページ
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
File::savePage(Page* pPage_)
{
	m_pPhysicalFile->savePage(pPage_->m_pPhysicalPage, pPage_->isDirty());
}

//
//  FUNCTION protected
//  Inverted::File::pushInstanceList -- インスタンスをpushする
//
//  NOTES
//
void
File::pushInstanceList(Page* pPage_,
						 PhysicalFile::PageID id_)
{
	if (m_iFreeInstanceCount < 10 &&
		(m_eType != Type::Btree || id_ != 0))
	{
		// 最大10個までのページインスタンスをためておく
		// ただしB木のヘッダーページは型が違うので開放してしまう

		pPage_->m_pNext = m_pInstanceList;
		m_pInstanceList = pPage_;
		++m_iFreeInstanceCount;
	}
	else
	{
		delete pPage_;
	}
}

//
//  FUNCTION protected
//  Inverted::File::popInstanceList -- インスタンスをpopする
//
//  NOTES
//
Page*
File::popInstanceList()
{
	Page* p = 0;
	if (m_iFreeInstanceCount)
	{
		p = m_pInstanceList;
		m_pInstanceList = p->m_pNext;
		p->m_pNext = 0;
		--m_iFreeInstanceCount;
	}
	return p;
}

//
//  FUNCTION private
//  Inverted::File::detachNoDirtyPage -- 不要なものを削除する
//
void
File::detachNoDirtyPage()
{
	PageList::Iterator i = m_cPageList.begin();
	while (i != m_cPageList.end())
	{
		Page* p = &(*i);
		++i;

		if (p->isDirty() == false && p->m_iReference == 0)
		{
			// dirtyでもないし、参照もされていない
			//  -> 削除する

			PhysicalFile::PageID id = p->getID();

			// デタッチする
			m_pPhysicalFile->detachPage(p->m_pPhysicalPage);

			m_cPageList.erase(*p);
			m_cPageMap.erase(id);
			pushInstanceList(p, id);

			--m_iCurrentCacheCount;

			if (m_iCurrentCacheCount <= m_iCacheCount)
				break;
		}
	}
}

//
//  Copyright (c) 2002, 2003, 2004, 2005, 2006, 2009, 2010, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
