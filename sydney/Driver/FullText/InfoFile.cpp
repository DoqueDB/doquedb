// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InfoFile.cpp -- 全文ファイル情報ファイル
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2008, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText/InfoFile.h"
#include "FileCommon/OpenOption.h"
#include "LogicalFile/OpenOption.h"
#include "PhysicalFile/Manager.h"
#include "Version/File.h"
#include "Os/File.h"
#include "Exception/Object.h"
#include "Schema/File.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT_USING

namespace
{
	//
	//	VARIABLE LOCAL
	//	_$$::_Directory -- ディレクトリ
	//
	ModUnicodeString _Directory("Dispatch");

	//
	//	VARIABLE LOCAL
	//	_$$::_PageSize -- ページサイズ(KB)
	//
	ModSize _PageSize = 4;
}

//
//	FUNCTION public
//	FullText::InfoFile::InfoFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
InfoFile::InfoFile(const FullText::FileID& cFileID_)
	: m_cFileID(cFileID_), m_pPhysicalFile(0), m_pTransaction(0),
	  m_eFixMode(Buffer::Page::FixMode::Unknown), m_bVerify(false),
	  m_pProgress(0), m_pPage(0),
	  m_eUnfixMode(PhysicalFile::Page::UnfixMode::NotDirty)
{
	attach();
}

//
//	FUNCTION public
//	FullText::InfoFile::~InfoFile -- デストラクタ
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
InfoFile::~InfoFile()
{
	detach();
}

//
//	FUNCTION public
//	FullText::InfoFile::create -- ファイルを作成する
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
InfoFile::create(const Trans::Transaction& cTransaction_)
{
	try
	{
		// 物理ファイルに作成を依頼する
		m_pPhysicalFile->create(*m_pTransaction);
	}
	catch (...)
	{
		// ディレクトリを破棄する
		Os::Directory::remove(m_cPath);
		_SYDNEY_RETHROW;
	}

	try
	{
		// 初期化する
		initializeData();
	}
	catch (...)
	{
		recoverAllPages();
		destroy(*m_pTransaction);
		_SYDNEY_RETHROW;
	}

	flushAllPages();
}

//
//	FUNCTION public
//	FullText::InfoFile::destroy -- ファイルを破棄する
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
InfoFile::destroy(const Trans::Transaction& cTransaction_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかくアンマウントする
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	m_pPhysicalFile->destroy(cTransaction_);
	ModOsDriver::File::rmAll(m_cPath, ModTrue);
}

//
//	FUNCTION public
//	FullText::InfoFile::recover -- リカバリする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Trans::TimeStamp& cPoint_
//		チェックポイント
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InfoFile::recover(const Trans::Transaction& cTransaction_,
				  const Trans::TimeStamp& cPoint_)
{
	if (isMounted(cTransaction_))
	{
		m_pPhysicalFile->recover(cTransaction_, cPoint_);

		if (!isAccessible())
		{
			// リカバリの結果
			// 実体である OS ファイルが存在しなくなったので、
			// サブディレクトを削除する

			Os::Directory::remove(m_cPath);
		}
	}
}

//
//	FUNCTION public
//	FullText::InfoFile::verify -- 整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	Admin::Verification::Treatment::Value uiTreatment_
//		動作
//	Admin::Verification::Progress& cProgress_
//		経過
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InfoFile::verify(const Trans::Transaction& cTransaction_,
				 Admin::Verification::Treatment::Value uiTreatment_,
				 Admin::Verification::Progress& cProgress_)
{
	if (isMounted(cTransaction_))
	{
		// 物理ファイルへの開始通知
		m_pPhysicalFile->startVerification(cTransaction_,
										   uiTreatment_,
										   cProgress_);

		// 必要なメンバーの初期化
		m_pTransaction = &cTransaction_;
		if (uiTreatment_ & Admin::Verification::Treatment::Correct)
			m_eFixMode = Buffer::Page::FixMode::Write;
		else
			m_eFixMode = Buffer::Page::FixMode::ReadOnly;
		m_bVerify = true;
		m_pProgress = &cProgress_;

		try
		{
			// 読んでみる
			read();
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			try
			{
				flushAllPages();
				// 整合性検査終了
				m_pPhysicalFile->endVerification(cTransaction_, cProgress_);
				m_bVerify = false;
				m_pProgress = 0;
			}
			catch(...)
			{
				Schema::File::setAvailability(m_cFileID.getLockName(), false);
			}
			_SYDNEY_RETHROW;
		}

		// ページをデタッチする
		flushAllPages();

		// 物理ファイルへの終了通知
		m_pPhysicalFile->endVerification(cTransaction_, cProgress_);

		m_bVerify = false;
		m_pProgress = 0;
	}
}

//
//	FUNCTION public
//	FullText::InfoFile::open -- ファイルをオープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const LogicalFile::OpenOption& cOption_
//		オープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InfoFile::open(const Trans::Transaction& cTransaction_,
			   const LogicalFile::OpenOption& cOption_)
{
	// OpenModeを求める
	int value = cOption_.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key));
	open(cTransaction_,
		 static_cast<LogicalFile::OpenOption::OpenMode::Value>(value));
}

//
//	FUNCTION public
//	FullText::InfoFile::open -- ファイルをオープンする
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
InfoFile::open(const Trans::Transaction& cTransaction_,
			   LogicalFile::OpenOption::OpenMode::Value eOpenMode_)
{
	if (eOpenMode_ == LogicalFile::OpenOption::OpenMode::Update ||
		eOpenMode_ == LogicalFile::OpenOption::OpenMode::Batch)
		m_eFixMode = Buffer::Page::FixMode::Write;
	else
		m_eFixMode = Buffer::Page::FixMode::ReadOnly;

	// トランザクションを保存する
	m_pTransaction = &cTransaction_;
}

//
//	FUNCTION public
//	FullText::InfoFile::close -- ファイルをクローズする
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
InfoFile::close()
{
	flushAllPages();
	m_eFixMode = Buffer::Page::FixMode::Unknown;
	m_pTransaction = 0;
}

//
//	FUNCTION public
//	FullText::InfoFile::move -- ファイルを移動する
//
//	NOTES
//	移動元と移動先のパスが異なっていることが前提。
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Common::StringArrayData& cArea_
//		移動先のエリア
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InfoFile::move(const Trans::Transaction& cTransaction_,
			   const Common::StringArrayData& cArea_)
{
	// 新しいパス
	Os::Path cPath(cArea_.getElement(0));
	cPath.addPart(_Directory);

	// ファイルがあるか
	bool accessible = isAccessible();

	// 一時ファイルか
	bool temporary
		= (m_pPhysicalFile->getBufferingStrategy().
		   m_VersionFileInfo._category == Buffer::Pool::Category::Temporary);
	
	int step = 0;
	try
	{
		Version::File::StorageStrategy::Path cVersionPath;
		cVersionPath._masterData = cPath;
		if (!temporary)
		{
			cVersionPath._versionLog = cPath;
			cVersionPath._syncLog = cPath;
		}
		
		m_pPhysicalFile->move(cTransaction_, cVersionPath);
		step++;
		if (accessible)
			// 古いディレクトリを削除する
			Os::Directory::remove(m_cPath);
		step++;
	}
	catch (...)
	{
		switch (step)
		{
		case 1:
			{
				Version::File::StorageStrategy::Path cVersionPath;
				cVersionPath._masterData = m_cPath;
				if (!temporary)
				{
					cVersionPath._versionLog = m_cPath;
					cVersionPath._syncLog = m_cPath;
				}
				m_pPhysicalFile->move(cTransaction_, cVersionPath);
			}
		case 0:
			if (accessible)
				Os::Directory::remove(cPath);
		}
		_SYDNEY_RETHROW;
	}

	m_cPath = cPath;
}

//
//	FUNCTION public
//	FullText::InfoFile::recoverAllPages -- ページの更新を破棄する
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
InfoFile::recoverAllPages()
{
	if (m_pPage)
	{
		m_pPhysicalFile->detachPage(m_pPage,
									PhysicalFile::Page::UnfixMode::NotDirty);
		m_pPage = 0;
		m_eUnfixMode = PhysicalFile::Page::UnfixMode::NotDirty;
	}
}

//
//	FUNCTION public
//	FullText::InfoFile::flushAllPages -- ページの更新を反映する
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
InfoFile::flushAllPages()
{
	if (m_pPage)
	{
		if (m_eUnfixMode == PhysicalFile::Page::UnfixMode::Dirty)
		{
			m_pPage->write(*m_pTransaction, &m_cData, 0, sizeof(m_cData));
		}
		m_pPhysicalFile->detachPage(m_pPage, m_eUnfixMode);
		m_pPage = 0;
		m_eUnfixMode = PhysicalFile::Page::UnfixMode::NotDirty;
	}
}

//
//	FUNCTION public
//	FullText::InfoFile::flip -- 入れ替える
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
InfoFile::flip()
{
	read();
	if (m_cData.m_iProceeding == 0)
	{
		// マージ途中じゃない
		m_cData.m_iIndex = (m_cData.m_iIndex == 0) ? 1 : 0;
		m_cData.m_iProceeding = 1;
		dirty();
	}
}

//
//	FUNCTION public
//	FullText::InfoFile::isProceeding -- マージ中かどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		マージ中の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InfoFile::isProceeding()
{
	read();
	return (m_cData.m_iProceeding != 0);
}

//
//	FUNCTION public
//	FullText::InfoFile::getProceeding -- 現在の工程番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		工程番号
//
//	EXCEPTIONS
//
int
InfoFile::getProceeding()
{
	read();
	return m_cData.m_iProceeding;
}

//
//	FUNCTION public
//	FullText::InfoFile::nextProceeding -- 次の工程に進める
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
InfoFile::nextProceeding()
{
	read();
	m_cData.m_iProceeding++;
	dirty();
}

//
//	FUNCTION public
//	FullText::InfoFile::unsetProceeding -- マージ終了に設定する
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
InfoFile::unsetProceeding()
{
	read();
	m_cData.m_iProceeding = 0;
	dirty();
}

//
//	FUNCTION public
//	FullText::InfoFile::getIndex -- エグゼキュータ側の要素番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		エグゼキュータ側の小転置の要素番号
//
//	EXCEPTIONS
//
int
InfoFile::getIndex()
{
	read();
	return m_cData.m_iIndex;
}

//
//	FUNCTION private
//	FullText::InfoFile::attach -- 物理ファイルをアタッチする
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
InfoFile::attach()
{
	PhysicalFile::File::StorageStrategy cStorageStrategy;
	PhysicalFile::File::BufferingStrategy cBufferingStrategy;

	//
	//	物理ファイル格納戦略を設定する
	//

	// 物理ファイルタイプ
	cStorageStrategy.m_PhysicalFileType = PhysicalFile::NonManageType;
	// マウントされているか
	cStorageStrategy.m_VersionFileInfo._mounted = m_cFileID.isMounted();
	// 読み取り専用か
	cStorageStrategy.m_VersionFileInfo._readOnly = m_cFileID.isReadOnly();
	// ページサイズ
	cStorageStrategy.m_VersionFileInfo._pageSize
		= Version::File::verifyPageSize(_PageSize << 10);

	// パスを保存
	m_cPath = m_cFileID.getPath();
	m_cPath.addPart(_Directory);

	// マスタデータファイルの親ディレクトリの絶対パス名
	cStorageStrategy.m_VersionFileInfo._path._masterData = m_cPath;
	if (m_cFileID.isTemporary() == false)
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
//	FullText::InfoFile::detach -- 物理ファイルをデタッチする
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
InfoFile::detach()
{
	if (m_pPhysicalFile)
	{
		PhysicalFile::Manager::detachFile(m_pPhysicalFile);
		m_pPhysicalFile = 0;
	}
}

//
//	FUNCTION private
//	FullText::InfoFile::read -- ページ内容を読み込む
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
InfoFile::read()
{
	if (m_pPage == 0)
	{
		if (m_bVerify == true)
		{
			Admin::Verification::Progress cProgress(m_pProgress->getConnection());
			m_pPage
				= m_pPhysicalFile->verifyPage(*m_pTransaction,
											  m_eFixMode,
											  cProgress);
			*m_pProgress += cProgress;
		}
		else
		{
			m_pPage
				= m_pPhysicalFile->attachPage(
					*m_pTransaction,
					m_eFixMode,
					Buffer::ReplacementPriority::Middle);
		}
		m_pPage->read(*m_pTransaction, &m_cData, 0, sizeof(m_cData));
	}
}

//
//	FUNCTION private
//	FullText::InfoFile::initializeData -- ページ内容を初期化する
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
InfoFile::initializeData()
{
	read();
	m_cData.m_uiVersion = 2;
	m_cData.m_iIndex = 0;
	m_cData.m_iProceeding = 0;
	dirty();
}

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
