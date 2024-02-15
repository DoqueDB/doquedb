// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SubFile.cpp --
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
#include "KdTree/SubFile.h"

#include "PhysicalFile/File.h"
#include "PhysicalFile/Manager.h"

#include "Os/Path.h"

#include "Exception/VerifyAborted.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
}

//
//	FUNCTION public
//	KdTree::SubFile::SubFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::FileID& cFileID_
//		ファイルID
//	PhysicalFile::Type eType_
//		物理ファイルタイプ
//	int iPageSize_
//		ページサイズ
//	const Os::Path& cPath_
//		ファイルパス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SubFile::SubFile(FileID& cFileID_, PhysicalFile::Type eType_,
				 int iPageSize_, const Os::Path& cPath_)
	: File(cFileID_, cPath_), m_pPhysicalFile(0), m_eType(eType_)
{
	// 物理ファイルを attach する
	attach(iPageSize_);
}

//
//	FUNCTION public
//	KdTree::SubFile::~SubFile -- デストラクタ
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
SubFile::~SubFile()
{
	// 物理ファイルを detach する
	detach();
}

//
//	FUNCTION public
//	KdTree::SubFile::create -- ファイルを作成する
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
SubFile::create()
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

		rmdir(getPath());
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	KdTree::SubFile::destroy -- ファイルを削除する
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
SubFile::destroy(const Trans::Transaction& cTransaction_)
{
	// ファイルを削除する
	m_pPhysicalFile->destroy(cTransaction_);

	// フォルダーを削除する
	rmdir(getPath());
}

//
//	FUNCTION public
//	KdTree::SubFile::recover -- ファイルを障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const TimesStamp& cPoint_
//		回復するポイント
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SubFile::recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_)
{
	if (isMounted(cTransaction_))
	{
		m_pPhysicalFile->recover(cTransaction_, cPoint_);

		if (!isAccessible())
		{
			// リカバリの結果、実体である OS ファイルが存在しなくなたので、
			// ディレクトリを削除する

			rmdir(getPath());
		}
	}
}

//
//	FUNCTION public
//	KdTree::SubFile::startVerification -- 整合性検査を開始する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	Admin::Verification::Treatment::Value uiTreatment_
//		整合性検査で矛盾を見つけたときの処置方法
//	Admin::Verification::Progress& cProgress_
//		整合性検査の経過を表すクラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SubFile::startVerification(const Trans::Transaction& cTransaction_,
						   Admin::Verification::Treatment::Value uiTreatment_,
						   Admin::Verification::Progress& cProgress_)
{
	File::startVerification(cTransaction_,
							uiTreatment_,
							cProgress_);
	
	m_pPhysicalFile->startVerification(cTransaction_, uiTreatment_, cProgress_);
}

//
//	FUNCTION public
//	KdTree::SubFile::endVerification -- 整合性検査を終了する
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
SubFile::endVerification()
{
	m_pPhysicalFile->endVerification(*m_pTransaction, *m_pProgress);

	File::endVerification();
}

//
//	FUNCTION public
//	KdTree::SubFile::move -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cNewPath_
//		移動先のパス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SubFile::move(const Trans::Transaction& cTransaction_,
			  const Os::Path& cNewPath_)
{
	if (Os::Path::compare(getPath(), cNewPath_)
		== Os::Path::CompareResult::Unrelated)
	{

		// 実体である OS ファイルが存在するか調べる

		const bool accessible = isAccessible();

		// ファイルが一時ファイルか調べる

		const bool temporary =
			(m_pPhysicalFile->getBufferingStrategy().
			 m_VersionFileInfo._category == Buffer::Pool::Category::Temporary);

		Version::File::StorageStrategy::Path cPath;
		cPath._masterData = cNewPath_;
		if (!temporary) {
			cPath._versionLog = cNewPath_;
			cPath._syncLog = cNewPath_;
		}

		int step = 0;
		try
		{
			m_pPhysicalFile->move(cTransaction_, cPath);
			step++;

			if (accessible)

				// 古いディレクトリを削除する

				rmdir(getPath());
			step++;
		}
		catch (...)
		{
			switch (step)
			{
			case 1:
				{
					Version::File::StorageStrategy::Path cPath;
					cPath._masterData = getPath();
					if (!temporary) {
						cPath._versionLog = getPath();
						cPath._syncLog = getPath();
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

					rmdir(cNewPath_);
			}
			_SYDNEY_RETHROW;
		}

		// 新しいパスを設定する
		setPath(cNewPath_);
	}
}

//
//	FUNCTION protected
//	KdTree::SubFile::attachPhysicalPage -- 物理ページを attach する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		attach するページID
//
//	RETURN
//	PhysicalFile::Page*
//		attach した物理ページ
//
//	EXCEPTIONS
//
PhysicalFile::Page*
SubFile::attachPhysicalPage(PhysicalFile::PageID uiPageID_,
							Buffer::Page::FixMode::Value eFixMode_)
{
	if (eFixMode_ == Buffer::Page::FixMode::Unknown)
		eFixMode_ = m_eFixMode;
	
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

	return m_pPhysicalFile->attachPage(*m_pTransaction,
									   uiPageID_,
									   eFixMode_);
}

//
//	FUNCTION private
//	KdTree::SubFile::attach -- 物理ファイルを attach する
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
SubFile::attach(int iPageSize_)
{
	PhysicalFile::File::StorageStrategy cStorageStrategy;
	PhysicalFile::File::BufferingStrategy cBufferingStrategy;

	//
	//	物理ファイル格納戦略を設定する
	//

	// 物理ファイルの空き領域管理機能
	cStorageStrategy.m_PhysicalFileType = m_eType;
	// マウントされているか
	cStorageStrategy.m_VersionFileInfo._mounted = m_cFileID.isMounted();
	// 読み取り専用か
	cStorageStrategy.m_VersionFileInfo._readOnly = m_cFileID.isReadOnly();
	// ページサイズ
	cStorageStrategy.m_VersionFileInfo._pageSize = iPageSize_;

	// マスタデータファイルの親ディレクトリの絶対パス名
	cStorageStrategy.m_VersionFileInfo._path._masterData = getPath();
	if (m_cFileID.isTemporary() == false)
	{
		// バージョンログファイルの親ディレクトリの絶対パス名
		cStorageStrategy.m_VersionFileInfo._path._versionLog = getPath();
		// 同期ログファイルの親ディレクトリの絶対パス名
		cStorageStrategy.m_VersionFileInfo._path._syncLog = getPath();
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
	if (m_cFileID.isTemporary())
	{
		// 一時なら
		cBufferingStrategy.m_VersionFileInfo._category
			= Buffer::Pool::Category::Temporary;
	}
	else if (m_cFileID.isReadOnly())
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
	m_pPhysicalFile
		= PhysicalFile::Manager::attachFile(cStorageStrategy,
											cBufferingStrategy,
											m_cFileID.getLockName());
}

//
//	FUNCTION private
//	KdTree::SubFile::detach -- 物理ファイルを detach する
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
SubFile::detach()
{
	if (m_pPhysicalFile)
	{
		PhysicalFile::Manager::detachFile(m_pPhysicalFile);
		m_pPhysicalFile = 0;
	}
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
