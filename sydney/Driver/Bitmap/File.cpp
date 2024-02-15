// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp --
// 
// Copyright (c) 2005, 2006, 2007, 2010, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Bitmap";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Bitmap/File.h"
#include "Bitmap/FileID.h"

#include "PhysicalFile/Manager.h"
#include "Version/File.h"
#include "Os/File.h"
#include "LogicalFile/Estimate.h"
#include "Admin/Verification.h"
#include "Common/Assert.h"

#include "Exception/FileNotFound.h"
#include "Exception/VerifyAborted.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$::_cSubDir -- 圧縮する場合に付加するサブディレクトリ名
	//
	ModUnicodeString _cSubDir("Btree");
}

//
//	FUNCTION public
//	Bitmap::File::File -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	const Bitmap::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
File::File(const FileID& cFileID_)
	: m_cFileID(cFileID_), m_pPhysicalFile(0), m_pTransaction(0),
	  m_eFixMode(Buffer::Page::FixMode::Unknown),
	  m_bVerification(false), m_pProgress(0)
{
	// 物理ファイルをattachする
	attach(cFileID_);
}

//
//	FUNCTION public
//	Bitmap::File::~File -- デストラクタ
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
	; _SYDNEY_ASSERT(m_vecpFreeList.getSize() == 0);
	; _SYDNEY_ASSERT(m_mapPage.getSize() == 0);

	// 物理ファイルをデタッチする
	detach();
}

//
//	FUNCTION public
//	Bitmap::File::create -- ファイルを作成する
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
//	Bitmap::File::destroy -- ファイルを削除する
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
//	Bitmap::File::open -- オープンする
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
//	Bitmap::File::close -- クローズする
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
//	Bitmap::File::move -- ファイルを移動する
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
	try
	{
		if (Os::Path::compare(m_cFileID.getPath(), cPath_)
			== Os::Path::CompareResult::Unrelated)
		{
			; _SYDNEY_ASSERT(m_pPhysicalFile);
		
			m_cPath = cPath_;
			if (m_cFileID.isCompressed() == true)
				// 圧縮する場合にはサブディレクトを追加する
				m_cPath.addPart(_cSubDir);
		
			// ファイルが一時ファイルか調べる
			const bool temporary =
				(m_pPhysicalFile->getBufferingStrategy().
				 m_VersionFileInfo._category
				 == Buffer::Pool::Category::Temporary);

			// 新しいパス名を設定する
			Version::File::StorageStrategy::Path cPath;
			cPath._masterData = m_cPath;
			if (!temporary) {
				cPath._versionLog = m_cPath;
				cPath._syncLog = m_cPath;
			}

			m_pPhysicalFile->move(cTransaction_, cPath);
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		m_cPath = m_cFileID.getPath();
		if (m_cFileID.isCompressed() == true)
			// 圧縮する場合にはサブディレクトを追加する
			m_cPath.addPart(_cSubDir);

		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Bitmap::File::flushAllPages
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
	ModVector<PhysicalFile::Page*>::Iterator j = m_vecpFreeList.begin();
	for (; j != m_vecpFreeList.end(); ++j)
	{
		// ページを開放する
		m_pPhysicalFile->freePage2(*m_pTransaction, *j);
	}
	m_vecpFreeList.clear();

	// マップに格納されているすべてのページ内容を確定する
	Map::Iterator i = m_mapPage.begin();
	for (; i != m_mapPage.end(); ++i)
	{
		// 変更を確定する
		PhysicalFile::Page* pPage = (*i).second;
		m_pPhysicalFile->detachPage(pPage,
									PhysicalFile::Page::UnfixMode::Dirty);
	}
	m_mapPage.erase(m_mapPage.begin(), m_mapPage.end());

	m_pPhysicalFile->detachPageAll();
}

//
//	FUNCTION public
//	Bitmap::File::recoverAllPages
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
	if (!(m_eFixMode & Buffer::Page::FixMode::Discardable))
	{
		// リカバリが不可なモードなので確定させる
		flushAllPages();
	}
	else
	{
		// フリーリストのページを開放する
		ModVector<PhysicalFile::Page*>::Iterator j = m_vecpFreeList.begin();
		for (; j != m_vecpFreeList.end(); ++j)
		{
			// 変更を破棄する
			m_pPhysicalFile->recoverPage(*j);
		}
		m_vecpFreeList.clear();

		// マップに格納されているすべてのページ内容を確定する
		Map::Iterator i = m_mapPage.begin();
		for (; i != m_mapPage.end(); ++i)
		{
			// 変更を破棄する
			PhysicalFile::Page* pPage = (*i).second;
			m_pPhysicalFile->recoverPage(pPage);
		}
		m_mapPage.erase(m_mapPage.begin(), m_mapPage.end());

		m_pPhysicalFile->recoverPageAll();
	}
}

//
//	FUNCTION public
//	Bitmap::File::getCost -- 1ページを得るコストを得る
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
//	Bitmap::File::startVerification -- 整合性検査を開始する
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
	m_pPhysicalFile->startVerification(
		cTransaction_, uiTreatment_, cProgress_);
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
//	Bitmap::File::endVerification -- 整合性検査を終了する
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
//	Bitmap::File::allocatePhysicalPage -- 物理ページを新たに確保する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::Page*
//		新たに確保したページ
//
//	EXCEPTIONS
//
PhysicalFile::Page*
File::allocatePhysicalPage()
{
	PhysicalFile::Page* pPage = 0;
	if (m_vecpFreeList.getSize())
	{
		// フリーされているものがあるので、それを返す
		pPage = m_vecpFreeList[0];
		m_vecpFreeList.popFront();
	}
	else
	{
		// ないので、新たに確保する
		pPage = m_pPhysicalFile->allocatePage2(*m_pTransaction,
											   m_eFixMode);
	}
	return pPage;
}

//
//	FUNCTION public
//	Bitmap::File::freePhysicalPage -- 物理ページを開放する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPage_
//		フリーする物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::freePhysicalPage(PhysicalFile::Page* pPage_)
{
	// マップから削除する
	m_mapPage.erase(pPage_->getID());
	// フリーページリストに追加する
	m_vecpFreeList.pushBack(pPage_);
}

//
//	FUNCTION public
//	Bitmap::File::attachPhysicalPage -- 物理ページをアタッチする
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
	if (eFixMode_ == Buffer::Page::FixMode::Unknown)
		eFixMode_ = m_eFixMode;
	
	// マップを検索する
	Map::Iterator i = m_mapPage.find(uiPageID_);
	if (i != m_mapPage.end())
	{
		// 見つかった -> マップから削除する
		PhysicalFile::Page* pPage = (*i).second;
		m_mapPage.erase(i);
		return pPage;
	}

	// マップにないので新たに確保する
	return attachPhysicalPageInternal(uiPageID_, eFixMode_);
}

//
//	FUNCTION public
//	Bitmap::File::detachPhysicalPage -- ページがdirtyじゃなければdetachする
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::Page* pPage_
//		デタッチするページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::detachPhysicalPage(PhysicalFile::Page* pPage_)
{
	if (pPage_->getUnfixMode() != PhysicalFile::Page::UnfixMode::Dirty)
	{
		// dirtyじゃないのでdetach
		m_pPhysicalFile->detachPage(pPage_,
									PhysicalFile::Page::UnfixMode::NotDirty);
	}
	else
	{
		// dirtyなので、マップへ登録
		ModPair<Map::Iterator, ModBoolean>
			result = m_mapPage.insert(pPage_->getID(), pPage_);
		if (result.second == ModFalse)
		{
			// すでに同じページIDのものが登録されているので、
			// こちらはdetachする
			m_pPhysicalFile->detachPage(pPage_);
		}
	}
}

//
//	FUNCTION public
//	Bitmap::File::changeFixMode -- 物理ページのFixModeを変更する
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
	// attachされているページはマップには入っていない
	
	// まずは、変更前のページをdetachする
	PhysicalFile::PageID uiPageID = pPage_->getID();
	m_pPhysicalFile->detachPage(pPage_);
	// 新しくattachする
	return attachPhysicalPageInternal(uiPageID, getFixMode());
}

//
//	FUNCTION protected
//	Bitmap::File::rmdir -- ディレクトリを削除する
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
	try
	{
		Os::Directory::remove(m_cPath);
	}
	catch (Exception::FileNotFound&)
	{
		// 無視
	}
}

//
//	FUNCTION private
//	Bitmap::File::attach -- 物理ファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	const Bitmap::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
File::attach(const FileID& cFileID_)
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
	cStorageStrategy.m_VersionFileInfo._pageSize = cFileID_.getPageSize();

	m_cPath = cFileID_.getPath();
	if (cFileID_.isCompressed() == true)
		// 圧縮する場合にはサブディレクトを追加する
		m_cPath.addPart(_cSubDir);

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
//	FUNCTION private
//	Bitmap::File::detach -- 物理ファイルをデタッチする
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
//	FUNCTION private
//	Bitmap::File::attachPhysicalPageInternal -- 物理ページをattachする
//
//	NOTE
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		ページID
//	Buffer::Page::FixMode::Value eFixMode_
//		FIXモード
//
//	RETURN
//	PhysicalFile::Page*
//		attachした物理ページ
//
//	EXCEPTIONS
//
PhysicalFile::Page*
File::attachPhysicalPageInternal(PhysicalFile::PageID uiPageID_,
								 Buffer::Page::FixMode::Value eFixMode_)
{
	PhysicalFile::Page* pPage = 0;
	
	if (m_bVerification == true)
	{
		// 整合性検査の時
		
		// PhysicalFile::notifyUsePage 内でバージョンページを fix しないように
		// attach してから notifyUsePage を呼び出す
		
		{
			Admin::Verification::Progress
				cProgress(m_pProgress->getConnection());
			pPage
				= m_pPhysicalFile->verifyPage(*m_pTransaction,
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
	}
	else
	{
		// 普通の時
		pPage = m_pPhysicalFile->attachPage(*m_pTransaction,
											uiPageID_,
											eFixMode_);
	}

	return pPage;
}

//
//	Copyright (c) 2005, 2006, 2007, 2010, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
