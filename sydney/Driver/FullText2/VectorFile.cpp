// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VectorFile.cpp -- ベクターファイル
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "FullText2/VectorFile.h"

#include "FullText2/FakeError.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "FileCommon/OpenOption.h"

#include "LogicalFile/OpenOption.h"

#include "Version/File.h"

#include "Os/Memory.h"
#include "Os/File.h"

#include "Schema/File.h"

#include "Exception/VerifyAborted.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::VectorFile::Page::~Page -- デストラクタ
//
//	NOTES
//
///	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
VectorFile::Page::~Page()
{
	unfix(m_bDirty);
}

//
//	FUNCTION public
//	FullText2::VectorFile::Page::getConstBuffer -- バッファを得る
//	FullText2::VectorFile::Page::getBuffer -- バッファを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const char*
//	char*
//		バッファ
//
//	EXCEPTIONS
//
const char*
VectorFile::Page::getConstBuffer() const
{
	if (m_bReadOnly)
	{
		if (m_pConstBuffer == 0)
			m_pConstBuffer = m_cMemory.operator const char*();
		return m_pConstBuffer;
	}
	else
	{
		return m_cMemory.operator const char*();
	}
}
char*
VectorFile::Page::getBuffer()
{
	return m_cMemory.operator char*();
}

//
//	FUNCTION public
//	FullText2::VectorFile::Page::unfix -- バッファリング内容を確定 or 破棄する
//
//	NOTES
//
//	ARGUMENTS
//	bool commit_
//		内容を確定するかどうかのフラグ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorFile::Page::unfix(bool commit_)
{
	if (m_cMemory.isOwner())
	{
		m_cMemory.unfix(commit_);
	}
	m_bDirty = false;
}

//
//	FUNCTION public
//	FullText2::VectorFile::VectorFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::FileID& cFileID_
//		ファイルID
//	const Os::Path& cPath_
//		このファイルを格納するパス
//	bool bBatch_
//		バッチモードか否か
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
VectorFile::VectorFile(const FullText2::FileID& cFileID_,
					   const Os::Path& cPath_,
					   bool bBatch_)
	: m_cFileID(cFileID_),
	  m_pVersionFile(0), m_pTransaction(0),
	  m_eFixMode(Buffer::Page::FixMode::Unknown), m_bVerify(false),
	  m_pProgress(0), m_bDirtyHeaderPage(false), m_pCurrentPage(0),
	  m_cPath(cPath_), m_bMounted(false)
{
	// ファイルをアタッチする
	attach(bBatch_);
}

//
//	FUNCTION public
//	FullText2::VectorFile::~VectorFile -- デストラクタ
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
VectorFile::~VectorFile()
{
	detach();
}

//
//	FUNCTION public
//	FullText2::VectorFile::getCount -- 登録されているカウントを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		登録カウント
//
//	EXCEPTIONS
//
ModUInt32
VectorFile::getCount()
{
	readHeader();
	return m_cHeader.m_uiCount;
}

//
//	FUNCTION public
//	FullText2::VectorFile::create -- ファイルを作成する
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
VectorFile::create()
{
	try
	{
		// バージョンファイルに作成を依頼する
		m_pVersionFile->create(*m_pTransaction);
	}
	catch (...)
	{
		// ディレクトリを破棄する
		rmdir(m_cPath);
		_SYDNEY_RETHROW;
	}

	try
	{
		// ヘッダーページを確保する
		m_cHeaderPage = Version::Page::fix(*m_pTransaction,
										   *m_pVersionFile,
										   0,
										   Buffer::Page::FixMode::Allocate,
										   Buffer::ReplacementPriority::Low);
		// 初期化する
		initializeHeader();
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
//	FullText2::VectorFile::destroy -- ファイルを破棄する
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
VectorFile::destroy(const Trans::Transaction& cTransaction_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく削除する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	m_bMounted = false;
	m_pVersionFile->destroy(cTransaction_);
	rmdir(m_cPath);
}

//
//	FUNCTION public
//	FullText2::VectorFile::recover -- リカバリする
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
VectorFile::recover(const Trans::Transaction& cTransaction_,
					const Trans::TimeStamp& cPoint_)
{
	if (isMounted(cTransaction_))
	{
		m_pVersionFile->recover(cTransaction_, cPoint_);

		if (!isAccessible())
		{
			// リカバリの結果
			// 実体である OS ファイルが存在しなくなったので、
			// サブディレクトを削除する

			rmdir(m_cPath);
		}
	}
}

//
//	FUNCTION public
//	FullText2::VectorFile::startVerification -- 整合性検査を開始する
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
VectorFile::startVerification(
	const Trans::Transaction& cTransaction_,
	Admin::Verification::Treatment::Value uiTreatment_,
	Admin::Verification::Progress& cProgress_)
{
	// 必要なメンバーの初期化
	m_pTransaction = &cTransaction_;
	if (uiTreatment_ & Admin::Verification::Treatment::Correct)
	{
		m_eFixMode = Buffer::Page::FixMode::Write |
			Buffer::Page::FixMode::Discardable;
	}
	else
	{
		m_eFixMode = Buffer::Page::FixMode::ReadOnly;
	}
	m_bVerify = true;
	m_pProgress = &cProgress_;

	// 物理ファイルへの開始通知
	m_pVersionFile->startVerification(cTransaction_,
									  uiTreatment_,
									  cProgress_,
									  false);

	try
	{
		// ヘッダーを読み込む
		readHeader();
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
				m_pVersionFile->endVerification(cTransaction_, cProgress_);
				m_bVerify = false;
				m_pProgress = 0;
			}
			catch(...)
			{
				Schema::File::setAvailability(m_cFileID.getLockName(), false);
			}
			_SYDNEY_RETHROW;
		}
}

//
//	FUNCTION public
//	FullText2::VectorFile::endVerification -- 整合性検査を終了する
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
VectorFile::endVerification()
{
	if (m_pTransaction)
	{
		try
		{
			// ページをデタッチする
			flushAllPages();
			// 物理ファイルへの終了通知
			m_pVersionFile->endVerification(*m_pTransaction, *m_pProgress);
		}
		catch (...)
		{
			m_pTransaction = 0;
			m_eFixMode = Buffer::Page::FixMode::Unknown;
			m_bVerify = false;
			m_pProgress = 0;
			
			_SYDNEY_RETHROW;
		}
	}
	
	m_pTransaction = 0;
	m_eFixMode = Buffer::Page::FixMode::Unknown;
	m_bVerify = false;
	m_pProgress = 0;
}

//
//	FUNCTION public
//	FullText2::VectorFile::open -- ファイルをオープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	Buffer::Page::FixMode::Value eFixMode_
//		FIXモード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorFile::open(const Trans::Transaction& cTransaction_,
				 Buffer::Page::FixMode::Value eFixMode_)
{
	// トランザクションを保存する
	m_pTransaction = &cTransaction_;
	// FIXモードを保存する
	m_eFixMode = eFixMode_;
}

//
//	FUNCTION public
//	FullText2::VectorFile::close -- ファイルをクローズする
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
VectorFile::close()
{
	flushAllPages();
	m_eFixMode = Buffer::Page::FixMode::Unknown;
	m_pTransaction = 0;
}

//
//	FUNCTION public
//	FullText2::VectorFile::move -- ファイルを移動する
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
VectorFile::move(const Trans::Transaction& cTransaction_,
				 const Os::Path& cPath_)
{
	// ファイルがあるか
	bool accessible = (isAccessible() &&
					   Os::Path::compare(cPath_, m_cPath)
					   == Os::Path::CompareResult::Unrelated);

	// 一時ファイルか
	bool temporary =
		(m_pVersionFile->getBufferingStrategy()._category
		 == Buffer::Pool::Category::Temporary);
	
	int step = 0;
	try
	{
		; _FULLTEXT2_FAKE_ERROR(VectorFile::move);
			
		Version::File::StorageStrategy::Path cVersionPath;
		cVersionPath._masterData = cPath_;
		if (!temporary)
		{
			cVersionPath._versionLog = cPath_;
			cVersionPath._syncLog = cPath_;
		}
		
		m_pVersionFile->move(cTransaction_, cVersionPath);
		step++;
		if (accessible)
			// 古いディレクトリを削除する
			rmdir(m_cPath);
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
				m_pVersionFile->move(cTransaction_, cVersionPath);
			}
		case 0:
			if (accessible)
				rmdir(cPath_);
		}
		_SYDNEY_RETHROW;
	}

	m_cPath = cPath_;
}

//
//	FUNCTION public
//	FullText2::VectorFile::recoverAllPages -- ページの更新を破棄する
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
VectorFile::recoverAllPages()
{
	if (m_cHeaderPage.isOwner())
	{
		if (m_bDirtyHeaderPage)
		{
			// allocateしたページは元に戻さないので、
			// 最大ページID以外のものを元に戻す

			// 最大ページIDを保存する
			Version::Page::ID maxPageID = m_cHeader.m_uiMaxPageID;
			// すべての内容をクリアする
			m_cHeaderPage.unfix(false);

			// もう一度ヘッダーをfixする
			readHeader();
			// 保存しておいた最大ページIDを元に戻す
			m_cHeader.m_uiMaxPageID = maxPageID;
			
			// 内容を確定する
			char* buffer = m_cHeaderPage.operator char*();
			Os::Memory::copy(buffer, &m_cHeader, sizeof(m_cHeader));
			m_cHeaderPage.unfix(true);
		}
		else
		{
			m_cHeaderPage.unfix(false);
		}
		m_bDirtyHeaderPage = false;
	}
	
	Version::Page::ID id = Version::Block::IllegalID;
	if (m_pCurrentPage)
	{
		// 二重にdeleteしないため
		id = m_pCurrentPage->m_cMemory.getPageID();
		
		m_pCurrentPage->unfix(false);
		delete m_pCurrentPage;
		m_pCurrentPage = 0;
	}
	
	PageMap::Iterator i = m_mapDirtyPage.begin();
	for (; i != m_mapDirtyPage.end(); ++i)
	{
		if ((*i).first == id)
			continue;
		
		(*i).second->unfix(false);
		delete (*i).second;
	}
	
	m_mapDirtyPage.erase(m_mapDirtyPage.begin(), m_mapDirtyPage.end());
}

//
//	FUNCTION public
//	FullText2::VectorFile::flushAllPages -- ページの更新を反映する
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
VectorFile::flushAllPages()
{
	if (m_cHeaderPage.isOwner())
	{
		if (m_bDirtyHeaderPage)
		{
			char* buffer = m_cHeaderPage.operator char*();
			Os::Memory::copy(buffer, &m_cHeader, sizeof(m_cHeader));
			m_cHeaderPage.unfix(true);
		}
		else
		{
			m_cHeaderPage.unfix(false);
		}
		m_bDirtyHeaderPage = false;
	}
	
	Version::Page::ID id = Version::Block::IllegalID;
	if (m_pCurrentPage)
	{
		// 二重にdeleteしないため
		id = m_pCurrentPage->m_cMemory.getPageID();
		
		delete m_pCurrentPage;	// デストラクタで unfix が実行される
		m_pCurrentPage = 0;
	}
	
	PageMap::Iterator i = m_mapDirtyPage.begin();
	for (; i != m_mapDirtyPage.end(); ++i)
	{
		if ((*i).first == id)
			continue;
		
		delete (*i).second;	// デストラクタで unfix が実行される
	}
	
	m_mapDirtyPage.erase(m_mapDirtyPage.begin(), m_mapDirtyPage.end());
}

//
//	FUNCTION public
//	FullText2::VectorFile::getMaxKey -- 最大のキー値を得る
//
//	NOTES
//
//	ARUGMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		登録されている最大のキー
//
//	EXCEPTIONS
//
ModUInt32
VectorFile::getMaxKey()
{
	readHeader();
	return m_cHeader.m_uiMaxKey;
}

//
//	FUNCTION public
//	FullText2::VectorFile::getMaxPageID -- 最大ページIDを得る
//
//	NOTES
//
//	ARUGMENTS
//	なし
//
//	RETURN
//	Version::Page::ID
//		現在利用している最大のページID
//
//	EXCEPTIONS
//
Version::Page::ID
VectorFile::getMaxPageID()
{
	readHeader();
	return m_cHeader.m_uiMaxPageID;
}

//
//	FUNCTION public
//	FullText2::VectorFile::clear -- ファイルの内容をクリアする
//
//	NOTES
//
//	ARUGMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorFile::clear()
{
	if (isMounted(*m_pTransaction))
	{
		// ページ内の領域サイズ
		ModSize contentSize
			= Version::Page::getContentSize(m_pVersionFile->getPageSize());
		
		try
		{
			// 現在attachしているページをそのままunfixする
			flushAllPages();
			
			// ヘッダーから最大ページIDを取得し、保存する
			Version::Page::ID maxPageID = getMaxPageID();

			// ヘッダーを初期化する
			initializeHeader();

			// 最大ページIDを書き戻す
			m_cHeader.m_uiMaxPageID = maxPageID;
			dirtyHeaderPage();

			// データ領域を初期化する
			for (Version::Page::ID id = 1; id <= maxPageID; ++id)
			{
				// ページを確保
				Version::Page::Memory memory
					= Version::Page::fix(*m_pTransaction,
										 *m_pVersionFile,
										 id,
										 Buffer::Page::FixMode::Write,
										 Buffer::ReplacementPriority::Low);
				// 0xffで初期化する
				char* p = memory.operator char*();
				unsigned char c = 0xff;
				Os::Memory::set(p, c, contentSize);
				memory.unfix(true);
			}
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			recoverAllPages();
			_SYDNEY_RETHROW;
		}
		flushAllPages();
	}
}

//
//	FUNCTION protected
//	FullText2::VectorFile::getSubClassHeader
//		-- サブクラス用のヘッダー領域を得る
//
//	NOTES
//	Version::Page::Memory は FixMode が ReadOnly の場合は const char* しか
//	受け付けないので、FixMode を参照して const char* か char* を呼び分け、
//	ReadOnly の場合には、const_cast することとする
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	char*
//		サブクラス用のヘッダー領域の先頭
//
//	EXCEPTIONS
//
char*
VectorFile::getSubClassHeader()
{
	// ヘッダーを読む
	readHeader();

	// 先頭のポインタを得る
	char* h = (m_eFixMode == Buffer::Page::FixMode::ReadOnly) ?
		const_cast<char*>(
			static_cast<const Version::Page::Memory&>(m_cHeaderPage)
			.operator const char*()) :
		m_cHeaderPage.operator char*();

	return h + _SUBCLASS_HEADER_OFFSET;
}

//
//	FUNCTION protected
//	FullText2::VectorFile::getSubClassHeaderSize
//		-- サブクラス用のヘッダー領域のサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		サブクラス用のヘッダー領域のサイズ
//
//	EXCEPTIONS
//
ModSize
VectorFile::getSubClassHeaderSize()
{
	return Version::Page::getContentSize(m_pVersionFile->getPageSize()) -
		_SUBCLASS_HEADER_OFFSET;
}

//
//	FUNCTION protected
//	FullText2::VectorFile::attach -- 物理ファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	bool bBatch_
//		バッチモードかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorFile::attach(bool bBatch_)
{
	Version::File::StorageStrategy cStorageStrategy;
	Version::File::BufferingStrategy cBufferingStrategy;

	//
	//	格納戦略を設定する
	//

	// マウントされているか
	cStorageStrategy._mounted = m_cFileID.isMounted();
	// 読み取り専用か
	cStorageStrategy._readOnly = m_cFileID.isReadOnly();
	// ページサイズ
	cStorageStrategy._pageSize = m_cFileID.getPageSize();

	// マスタデータファイルの親ディレクトリの絶対パス名
	cStorageStrategy._path._masterData = m_cPath;
	if (m_cFileID.isTemporary() == false)
	{
		// バージョンログファイルの親ディレクトリの絶対パス名
		cStorageStrategy._path._versionLog = m_cPath;
		// 同期ログファイルの親ディレクトリの絶対パス名
		cStorageStrategy._path._syncLog = m_cPath;
	}

	// マスタデータファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy._sizeMax._masterData = 0;
	// バージョンログファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy._sizeMax._versionLog = 0;
	// 同期ログファイルの最大ファイルサイズ(B 単位)
	cStorageStrategy._sizeMax._syncLog = 0;

	// マスタデータファイルのエクステンションサイズ(B 単位)
	cStorageStrategy._extensionSize._masterData = 0;
	// バージョンログファイルのエクステンションサイズ(B 単位)
	cStorageStrategy._extensionSize._versionLog = 0;
	// 同期ログファイルのエクステンションサイズ(B 単位)
	cStorageStrategy._extensionSize._syncLog = 0;

	
	//
	//	バッファリング戦略を設定する
	//
	if (m_cFileID.isTemporary())
	{
		// 一時なら
		cBufferingStrategy._category = Buffer::Pool::Category::Temporary;
	}
	else if (m_cFileID.isReadOnly())
	{
		// 読み取り専用なら
		cBufferingStrategy._category = Buffer::Pool::Category::ReadOnly;
	}
	else
	{
		// その他
		cBufferingStrategy._category = Buffer::Pool::Category::Normal;
	}


	// バージョンファイルをアタッチする
	//
	//【注意】
	// 全文ではバッチモードかどうかを下位に渡さない
	//
	m_pVersionFile = Version::File::attach(cStorageStrategy,
										   cBufferingStrategy,
										   m_cFileID.getLockName());
}

//
//	FUNCTION protected
//	FullText2::VectorFile::detach -- 物理ファイルをデタッチする
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
VectorFile::detach()
{
	if (m_pVersionFile)
	{
		Version::File::detach(m_pVersionFile, true);
		m_pVersionFile = 0;
	}
}

//
//	FUNCTION protected
//	FullText2::VectorFile::readHeader -- ヘッダーページの内容を読み込む
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
VectorFile::readHeader()
{
	if (m_cHeaderPage.isOwner() == false)
	{
		fixHeaderPage();

		const char* buffer = static_cast<const Version::Page::Memory&>(
			m_cHeaderPage).operator const char*();
		Os::Memory::copy(&m_cHeader, buffer, sizeof(m_cHeader));
	}
}

//
//	FUNCTION protected
//	FullText2::VectorFile::initializeHeader -- ヘッダーを初期化する
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
VectorFile::initializeHeader()
{
	readHeader();
	m_cHeader.m_uiVersion = 0;
	m_cHeader.m_uiCount = 0;
	m_cHeader.m_uiMaxKey = 0;
	m_cHeader.m_uiMaxPageID = 0;
	dirtyHeaderPage();
}

//
//	FUNCTION protected
//	Fulltext::VectorFile::fixHeaderPage -- ヘッダーページをfixする
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
VectorFile::fixHeaderPage()
{
	if (m_cHeaderPage.isOwner())
	{
		// すでに fix されているので、何もしない
		return;
	}
		
	if (m_bVerify == true)
	{
		Admin::Verification::Progress cProgress(
			m_pProgress->getConnection());
		m_cHeaderPage
			= Version::Page::verify(*m_pTransaction,
									*m_pVersionFile,
									0,
									m_eFixMode,
									cProgress);
		*m_pProgress += cProgress;
		if (cProgress.isGood() == false)
		{
			m_cHeaderPage.unfix(false);
			_SYDNEY_THROW0(Exception::VerifyAborted);
		}
	}
	else
	{
		m_cHeaderPage
			= Version::Page::fix(*m_pTransaction,
								 *m_pVersionFile,
								 0,
								 m_eFixMode,
								 Buffer::ReplacementPriority::Low);
	}
}

//
//	FUNCTION protected
//	Fulltext::VectorFile::fixPage -- ページをfixする
//
//	NOTES
//
//	ARGUMENTS
//	Version::Page::ID uiPageID_
//		fixするページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorFile::fixPage(Version::Page::ID uiPageID_)
{
	if (m_pCurrentPage != 0)
	{
		if (m_pCurrentPage->m_cMemory.getPageID() == uiPageID_)
			// 同じページなら何もしない
			return;
		
		if (m_pCurrentPage->isDirty())
		{
			// 更新されているので、マップに格納する
			// 同じページが格納されているかも知れないが、
			// 検索してから挿入するより、そのまま挿入した方が早い
			
			m_mapDirtyPage.insert(m_pCurrentPage->m_cMemory.getPageID(),
								  m_pCurrentPage);
		}
		else
		{
			// 更新されていないので、そのまま unifx する
			
			delete m_pCurrentPage, m_pCurrentPage = 0;
		}
	}

	// マップを検索する
	PageMap::Iterator i = m_mapDirtyPage.find(uiPageID_);
	if (i != m_mapDirtyPage.end())
	{
		// 見つかったので、それを返す

		m_pCurrentPage = (*i).second;
		return;
	}

	m_pCurrentPage = new Page(m_eFixMode & Buffer::Page::FixMode::ReadOnly);
		
	if (m_bVerify == true)
	{
		Admin::Verification::Progress cProgress(
			m_pProgress->getConnection());
		m_pCurrentPage->m_cMemory
			= Version::Page::verify(*m_pTransaction,
									*m_pVersionFile,
									uiPageID_,
									m_eFixMode,
									cProgress);
		*m_pProgress += cProgress;
	}
	else
	{
		m_pCurrentPage->m_cMemory
			= Version::Page::fix(*m_pTransaction,
								 *m_pVersionFile,
								 uiPageID_,
								 m_eFixMode,
								 Buffer::ReplacementPriority::Low);
	}
}

//
//	FUNCTION protected
//	Fulltext::VectorFile::allocatePage -- ページをallocateする
//
//	NOTES
//	新しく確保したページの格納領域はすべて 0xff で初期化する
//
//	ARGUMENTS
//	Version::Page::ID uiPageID_
//		allocateするページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorFile::allocatePage(Version::Page::ID uiPageID_)
{
	// 今回 allocate するページID
	Version::Page::ID saveMax = getMaxPageID() + 1;

	// ページ内の領域サイズ
	ModSize contentSize
		= Version::Page::getContentSize(m_pVersionFile->getPageSize());

	try
	{
		if (m_pCurrentPage != 0)
		{
			if (m_pCurrentPage->isDirty())
			
				// 更新されているので、マップに格納する
				m_mapDirtyPage.insert(m_pCurrentPage->m_cMemory.getPageID(),
									  m_pCurrentPage);

			else

				// 更新されていないので、そのまま unifx する
				delete m_pCurrentPage;
		}

		m_pCurrentPage = 0;

		Version::Page::ID id = saveMax;
		for (; id < uiPageID_; ++id)
		{
			// 途中のページを確保する
			Version::Page::Memory memory
				= Version::Page::fix(*m_pTransaction,
									 *m_pVersionFile,
									 id,
									 Buffer::Page::FixMode::Allocate,
									 Buffer::ReplacementPriority::Low);
			// 0xffで初期化する
			char* p = memory.operator char*();
			unsigned char c = 0xff;
			Os::Memory::set(p, c, contentSize);
			memory.unfix(true);
		}

		m_pCurrentPage = new Page(false);
		
		// 目的のページは Discardable で確保する
		m_pCurrentPage->m_cMemory =
			Version::Page::fix(*m_pTransaction,
							   *m_pVersionFile,
							   uiPageID_,
							   Buffer::Page::FixMode::Allocate |
							   Buffer::Page::FixMode::Discardable,
							   Buffer::ReplacementPriority::Low);
		
		// 0xffで初期化する
		unsigned char c = 0xff;
		Os::Memory::set(m_pCurrentPage->getBuffer(), c, contentSize);
		m_pCurrentPage->m_cMemory.touch(true);
		m_pCurrentPage->dirty();
			
		// ヘッダーを更新する
		if (m_cHeader.m_uiMaxPageID < uiPageID_)
		{
			m_cHeader.m_uiMaxPageID = uiPageID_;
			char* buffer = m_cHeaderPage.operator char*();
			Os::Memory::copy(buffer, &m_cHeader, sizeof(m_cHeader));
			m_cHeaderPage.touch(true);
			dirtyHeaderPage();
		}
	}
	catch (...)
	{
		try
		{
			if (saveMax < uiPageID_)
				// allocateに失敗したので、truncateする
				m_pVersionFile->truncate(*m_pTransaction, saveMax);
		}
		catch (...)
		{
			// truncateできなかったので、利用可能性をOFFにする
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(m_cFileID.getLockName(), false);
		}
		_SYDNEY_RETHROW;
	}
}

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
