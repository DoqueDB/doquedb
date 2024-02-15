// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VectorFile.cpp -- 全文ファイルのその他情報を格納するベクターファイル
// 
// Copyright (c) 2005, 2006, 2008, 2023 Ricoh Company, Ltd.
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
#include "FullText/VectorFile.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "FileCommon/OpenOption.h"

#include "LogicalFile/OpenOption.h"

#include "Version/File.h"

#include "Os/File.h"

#include "Schema/File.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$::_Directory -- ディレクトリ
	//
	ModUnicodeString _Directory("Vector");

	//
	//	FUNCTION local
	//	_$$::_IsNull -- 該当するビットがnullかどうか
	//
	bool _IsNull(const char* bitmap_, ModSize pos_)
	{
		bitmap_ += (pos_ / 8);
		return *bitmap_ & (1 << (pos_ % 8));
	}

	//
	//	FUNCTION local
	//	_$$::_BitOn -- 該当するビットをONにする
	//
	void _BitOn(char* bitmap_, ModSize pos_)
	{
		bitmap_ += (pos_ / 8);
		*bitmap_ |= (1 << (pos_ % 8));
	}

	//
	//	FUNCTION local
	//	_$$::_BitOff -- 該当するビットをOFFにする
	//
	void _BitOff(char* bitmap_, ModSize pos_)
	{
		bitmap_ += (pos_ / 8);
		*bitmap_ &= ~(1 << (pos_ % 8));
	}
}

//
//	FUNCTION public
//	FullText::VectorFile::VectorFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
VectorFile::VectorFile(FullText::FileID& cFileID_)
	: m_cFileID(cFileID_), m_pVersionFile(0), m_pTransaction(0),
	  m_eFixMode(Buffer::Page::FixMode::Unknown), m_bVerify(false),
	  m_pProgress(0), m_bDirtyHeaderPage(false),
	  m_bDirtyCurrentPage(false), m_bMounted(false)
{
	attach();
}

//
//	FUNCTION public
//	FullText::VectorFile::~VectorFile -- デストラクタ
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
//	FullText::VectorFile::getCount -- 登録されているカウントを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		登録カウント
//
//	EXCEPTIONS
//
ModInt64
VectorFile::getCount() const
{
	const_cast<VectorFile*>(this)->readHeader();
	return static_cast<ModInt64>(m_cHeader.m_uiCount);
}

//
//	FUNCTION public
//	FullText::VectorFile::create -- ファイルを作成する
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
VectorFile::create(const Trans::Transaction& cTransaction_)
{
	try
	{
		// バージョンファイルに作成を依頼する
		m_pVersionFile->create(cTransaction_);
	}
	catch (...)
	{
		// ディレクトリを破棄する
		Os::Directory::remove(m_cPath);
		_SYDNEY_RETHROW;
	}

	try
	{
		// ヘッダーページを確保する
		m_cHeaderPage = Version::Page::fix(cTransaction_,
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
//	FullText::VectorFile::destroy -- ファイルを破棄する
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
	// とにかくアンマウントする
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	m_bMounted = false;
	m_pVersionFile->destroy(cTransaction_);
	ModOsDriver::File::rmAll(m_cPath, ModTrue);
}

//
//	FUNCTION public
//	FullText::VectorFile::recover -- リカバリする
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

			Os::Directory::remove(m_cPath);
		}
	}
}

//
//	FUNCTION public
//	FullText::VectorFile::verify -- 整合性検査を行う
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
VectorFile::verify(const Trans::Transaction& cTransaction_,
				 Admin::Verification::Treatment::Value uiTreatment_,
				 Admin::Verification::Progress& cProgress_)
{
	if (isMounted(cTransaction_))
	{
		// 物理ファイルへの開始通知
		m_pVersionFile->startVerification(cTransaction_,
										  uiTreatment_,
										  cProgress_,
										  false);

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

		try
		{
			// 読んでみる
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

		// ページをデタッチする
		flushAllPages();

		// 物理ファイルへの終了通知
		m_pVersionFile->endVerification(cTransaction_, cProgress_);

		m_bVerify = false;
		m_pProgress = 0;
	}
}

//
//	FUNCTION public
//	FullText::VectorFile::open -- ファイルをオープンする
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
VectorFile::open(const Trans::Transaction& cTransaction_,
				 const LogicalFile::OpenOption& cOption_)
{
	// OpenModeを得る
	int value = cOption_.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::OpenMode::Key));
	
	// OpenModeからFixModeを得る
	if (value == LogicalFile::OpenOption::OpenMode::Update ||
		value == LogicalFile::OpenOption::OpenMode::Batch)
	{
		m_eFixMode = Buffer::Page::FixMode::Write |
			Buffer::Page::FixMode::Discardable;
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
//	FullText::VectorFile::close -- ファイルをクローズする
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
//	FullText::VectorFile::move -- ファイルを移動する
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
VectorFile::move(const Trans::Transaction& cTransaction_,
			   const Common::StringArrayData& cArea_)
{
	// 新しいパス
	Os::Path cPath(cArea_.getElement(0));
	cPath.addPart(_Directory);

	// ファイルがあるか
	bool accessible = isAccessible();

	// 一時ファイルか
	bool temporary =
		(m_pVersionFile->getBufferingStrategy()._category
		 == Buffer::Pool::Category::Temporary);
	
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
		
		m_pVersionFile->move(cTransaction_, cVersionPath);
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
				m_pVersionFile->move(cTransaction_, cVersionPath);
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
//	FullText::VectorFile::recoverAllPages -- ページの更新を破棄する
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
		m_cHeaderPage.unfix(false);
		m_bDirtyHeaderPage = false;
	}
	if (m_cCurrentPage.isOwner())
	{
		m_cCurrentPage.unfix(false);
	}
}

//
//	FUNCTION public
//	FullText::VectorFile::flushAllPages -- ページの更新を反映する
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
	if (m_cCurrentPage.isOwner())
	{
		if (m_bDirtyCurrentPage)
			m_cCurrentPage.unfix(true);
		else
			m_cCurrentPage.unfix(false);
	}
}

//
//	FUNCTION public
//	FullText::VectorFile::getMaxPageID -- 最大ページIDを得る
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
//	FullText::VectorFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	const Common::DataArrayData& cValue_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorFile::insert(ModUInt32 uiRowID_,
				   const Common::DataArrayData& cValue_)
{
	//
	//	【注意】
	//	ページのキャッシュは1ページ分しかない
	//	1件挿入したら、必ず flushAllPages か、recoverAllPages を
	//	実行すること
	//

	; _SYDNEY_ASSERT(cValue_.getCount() == (int)getElementFieldCount());
	; _SYDNEY_ASSERT(m_cCurrentPage.isOwner() == false);
		  
	char* bitmap;
	char* buf = getBuffer(uiRowID_, bitmap);
	m_bDirtyCurrentPage = true;
	int n = cValue_.getCount();
	for (int i = 0; i < n; ++i)
	{
		const Common::Data::Pointer& p = cValue_.getElement(i);
		if (p->isNull() == true)
		{
			// nullビットマップをONする
			bitOn(bitmap, uiRowID_, i);
		}
		else
		{
			// nullビットマップをOFFする
			bitOff(bitmap, uiRowID_, i);
			// データを書き出す
			p->dumpValue(buf);
		}
		buf += m_cFileID.getVectorElementSize(i);
	}

	// 登録件数を更新する
	readHeader();
	++m_cHeader.m_uiCount;
	m_bDirtyHeaderPage = true;
}

//
//	FUNCTION public
//	FullText::VectorFile::update -- 更新する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	const Common::DataArrayData& cValue_
//		データ
//	const ModVector<int>& vecUpdateField_
//		更新対象のフィールド
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorFile::update(ModUInt32 uiRowID_,
				   const Common::DataArrayData& cValue_,
				   const ModVector<int>& vecUpdateFields_)
{
	//
	//	【注意】
	//	ページのキャッシュは1ページ分しかない
	//	1件更新したら、必ず flushAllPages か、recoverAllPages を
	//	実行すること
	//

	; _SYDNEY_ASSERT(cValue_.getCount() == (int)vecUpdateFields_.getSize());

	char* bitmap;
	char* buf = getBuffer(uiRowID_, bitmap);
	m_bDirtyCurrentPage = true;
	int n = static_cast<int>(getElementFieldCount());
	ModVector<int>::ConstIterator j = vecUpdateFields_.begin();
	int k = 0;
	for (int i = 0; i < n; ++i)
	{
		if (i == *j)
		{
			// 更新フィールド番号に一致

			Common::Data::Pointer p = cValue_.getElement(k++);
			if (p->isNull())
			{
				// nullビットマップをONにする
				bitOn(bitmap, uiRowID_, i);
			}
			else
			{
				// nullビットマップをOFFする
				bitOff(bitmap, uiRowID_, i);
				p->dumpValue(buf);
			}
			
			++j;	// 次の依頼へ
		}
		buf += m_cFileID.getVectorElementSize(i);
	}
}

//
//	FUNCTION public
//	FullText::VectorFile::expunge -- 削除する
//
//	NOTES
//
//	ARUGMENTS
//	ModUInt32 uiRowID_
//		ROWID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorFile::expunge(ModUInt32 uiRowID_)
{
	if (isMounted(*m_pTransaction))
	{
		//
		//	【注意】
		//	ページのキャッシュは1ページ分しかない
		//	1件削除したら、必ず flushAllPages か、recoverAllPages を
		//	実行すること
		//

		char* bitmap;
		char* buf = getBuffer(uiRowID_, bitmap);
		m_bDirtyCurrentPage = true;
		int n = static_cast<int>(getElementFieldCount());
		for (int i = 0; i < n; ++i)
		{
			// nullビットマップをONする
			bitOn(bitmap, uiRowID_, i);
		}
		
		// 登録件数を更新する
		readHeader();
		--m_cHeader.m_uiCount;
		m_bDirtyHeaderPage = true;
	}
}

//
//	FUNCTION public
//	FullText::VectorFile::get -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	int iField_
//		取得するフィールド
//	Common::Data& cValue_
//		取得した値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorFile::get(ModUInt32 uiRowID_, int iField_, Common::Data& cValue_)
{
	; _SYDNEY_ASSERT(iField_ < (int)getElementFieldCount());

	if (isMounted(*m_pTransaction))
	{
		const char* bitmap;
		const char* buf = getConstBuffer(uiRowID_, bitmap);
		if (buf == 0)
		{
			// そのROWIDのページは存在していないので、null に設定する
			
			cValue_.setNull();

			return;
		}
		
		int n = static_cast<int>(getElementFieldCount());
		for (int i = 0; i < n; ++i)
		{
			if (i == iField_)
			{
				// 取得依頼フィールド番号に一致
				
				if (isNull(bitmap, uiRowID_, i))
				{
					// null
					cValue_.setNull();
				}
				else
				{
					// not null
					cValue_.setDumpedValue(buf);
				}
			}
			buf += m_cFileID.getVectorElementSize(i);
		}
	}
}

//
//	FUNCTION public
//	FullText::VectorFile::get -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	Common::DataArrayData& cValue_
//		取得した値
//	const ModVector<int>& vecGetFields_
//		取得するフィールド
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorFile::get(ModUInt32 uiRowID_, Common::DataArrayData& cValue_,
				const ModVector<int>& vecGetFields_)
{
	; _SYDNEY_ASSERT(cValue_.getCount() == (int)vecGetFields_.getSize());
	; _SYDNEY_ASSERT(cValue_.getCount() <= (int)getElementFieldCount());

	if (isMounted(*m_pTransaction))
	{
		const char* bitmap;
		const char* buf = getConstBuffer(uiRowID_, bitmap);
		int n = static_cast<int>(getElementFieldCount());
		ModVector<int>::ConstIterator j = vecGetFields_.begin();
		int k = 0;
		for (int i = 0;j != vecGetFields_.end() && i < n; ++i)
		{
			if (i == *j)
			{
				// 取得依頼フィールド番号に一致
				
				Common::Data::Pointer p = cValue_.getElement(k++);
				if (isNull(bitmap, uiRowID_, i))
				{
					// null
					p->setNull();
				}
				else
				{
					// not null
					p->setDumpedValue(buf);
				}
				
				++j;	// 次の依頼へ
			}
			buf += m_cFileID.getVectorElementSize(i);
		}
	}
}

//
//	FUNCTION private
//	FullText::VectorFile::attach -- 物理ファイルをアタッチする
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
VectorFile::attach()
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
	cStorageStrategy._pageSize = m_cFileID.getVectorPageSize();

	// パスを保存
	m_cPath = m_cFileID.getPath();
	m_cPath.addPart(_Directory);

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
	m_pVersionFile = Version::File::attach(cStorageStrategy,
										   cBufferingStrategy,
										   m_cFileID.getLockName());
}

//
//	FUNCTION private
//	FullText::VectorFile::detach -- 物理ファイルをデタッチする
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
//	FUNCTION private
//	FullText::VectorFile::readHeader -- ヘッダーページの内容を読み込む
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
		// これでヘッダーページがfixされる
		fixPage(0);

		//const char* buffer = m_cHeaderPage.operator const char*();
		const char* buffer = static_cast<const Version::Page::Memory&>(m_cHeaderPage).operator const char*();
		Os::Memory::copy(&m_cHeader, buffer, sizeof(m_cHeader));
	}
}

//
//	FUNCTION private
//	FullText::VectorFile::initializeHeader -- ヘッダーを初期化する
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
	m_cHeader.m_uiMaxPageID = 0;
	dirtyHeaderPage();
}

//
//	FUNCTION private
//	Fulltext::VectorFile::fixPage -- ページをfixする
//
//	NOTES
//
//	ARGUMENTS
//	Version::Page::ID uiPageID_
//		fixするページID
//
//	RETURN
//	Version::Page::Memory&
//		fixしたページ領域への参照
//
//	EXCEPTIONS
//
Version::Page::Memory&
VectorFile::fixPage(Version::Page::ID uiPageID_)
{
	Version::Page::Memory& memory =
		(uiPageID_ == 0) ? m_cHeaderPage : m_cCurrentPage;
	if (uiPageID_ == 0)
		m_eFixMode &= ~Buffer::Page::FixMode::Discardable;
	
	if (memory.getPageID() != uiPageID_)
	{
		// ページが違うのでunfixする
		memory.unfix(false);
	}
	if (memory.isOwner() == false)
	{
		// まだfixされていないのでfixする
		if (m_bVerify == true)
		{
			Admin::Verification::Progress cProgress(
				m_pProgress->getConnection());
			memory = Version::Page::verify(*m_pTransaction,
										   *m_pVersionFile,
										   uiPageID_,
										   m_eFixMode,
										   cProgress);
			*m_pProgress += cProgress;
		}
		else
		{
			memory = Version::Page::fix(*m_pTransaction,
										*m_pVersionFile,
										uiPageID_,
										m_eFixMode,
										Buffer::ReplacementPriority::Low);
		}
	}
	
	return memory;
}

//
//	FUNCTION private
//	Fulltext::VectorFile::allocatePage -- ページをallocateする
//
//	NOTES
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

	try
	{
		Version::Page::ID id = saveMax;
		for (; id < uiPageID_; ++id)
		{
			// 途中のページを確保する
			m_cCurrentPage
				= Version::Page::fix(*m_pTransaction,
									 *m_pVersionFile,
									 id,
									 Buffer::Page::FixMode::Allocate,
									 Buffer::ReplacementPriority::Low);
			m_cCurrentPage.unfix(true);
		}

		// 目的のページは Discardable で確保する
		m_cCurrentPage = Version::Page::fix(*m_pTransaction,
											*m_pVersionFile,
											uiPageID_,
											Buffer::Page::FixMode::Allocate |
											Buffer::Page::FixMode::Discardable,
											Buffer::ReplacementPriority::Low);
		
		// ヘッダーを更新する
		m_cHeader.m_uiMaxPageID = uiPageID_;
		char* buffer = m_cHeaderPage.operator char*();
		Os::Memory::copy(buffer, &m_cHeader, sizeof(m_cHeader));
		m_cHeaderPage.touch(true);
	}
	catch (...)
	{
		try
		{
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
//	FUNCTION private
//	FullText::VectorFile::getConstBuffer -- ROWIDから格納領域を得る
//	FullText::VectorFile::getBuffer
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//
//	RETURN
//	const char*
//	char*
//		格納領域へのポインタ
//
//	EXCEPTIONS
//
const char*
VectorFile::getConstBuffer(ModUInt32 uiRowID_, const char*& bitmap_)
{
	// ROWID -> ページID+オフセットへ変換
	int offset = 0;
	Version::Page::ID id = convertToPageID(uiRowID_, offset);
	if (id > getMaxPageID())
		return 0;

	// ページを得る
	Version::Page::Memory& memory = fixPage(id);

	// メモリーを得る
	const char* buf = static_cast<const Version::Page::Memory&>(memory).
		operator const char*();    
    
	// ビットマップ領域の先頭を得る
	bitmap_ = buf + getCountPerPage() * getElementSize();

	return buf + offset;
}
char*
VectorFile::getBuffer(ModUInt32 uiRowID_, char*& bitmap_)
{
	// ROWID -> ページID+オフセットへ変換
	int offset = 0;
	Version::Page::ID id = convertToPageID(uiRowID_, offset);

	if (id > getMaxPageID())
	{
		// 最大ページIDより大きい -> 必要なところまでallocateする
		allocatePage(id);
	}

	// ページを得る
	Version::Page::Memory& memory = fixPage(id);

	// メモリーを得る
	char* buf = memory.operator char*();

	// ビットマップ領域の先頭を得る
	bitmap_ = buf + getCountPerPage() * getElementSize();

	return buf + offset;
}

//
//	FUNCTION private
//	FullText::VectorFile::isNull -- ビットマップをチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const char* bitmap_
//		このページのビットマップ領域の先頭
//	ModUInt32 uiRowID_
//		ROWID
//	int n_
//		要素番号
//
//	RETURN
//	bool
//		該当するフィールドがnullの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
VectorFile::isNull(const char* bitmap_, ModUInt32 uiRowID_, int n_) const
{
	ModSize pos = uiRowID_ % getCountPerPage();
	pos = pos * getElementFieldCount() + n_;
	return _IsNull(bitmap_, pos);
}

//
//	FUNCTION private
//	FullText::VectorFile::bitOn -- ビットをONにする
//
//	NOTES
//
//	ARGUMENTS
//	char* bitmap_
//		このページのビットマップ領域の先頭
//	ModUInt32 uiRowID_
//		ROWID
//	int n_
//		要素番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorFile::bitOn(char* bitmap_, ModUInt32 uiRowID_, int n_)
{
	ModSize pos = uiRowID_ % getCountPerPage();
	pos = pos * getElementFieldCount() + n_;
	_BitOn(bitmap_, pos);
}

//
//	FUNCTION private
//	FullText::VectorFile::bitOff -- ビットをOFFする
//
//	NOTES
//
//	ARGUMENTS
//	char* bitmap_
//		このページのビットマップ領域の先頭
//	ModUInt32 uiRowID_
//		ROWID
//	int n_
//		要素番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
VectorFile::bitOff(char* bitmap_, ModUInt32 uiRowID_, int n_)
{
	ModSize pos = uiRowID_ % getCountPerPage();
	pos = pos * getElementFieldCount() + n_;
	_BitOff(bitmap_, pos);
}

//
//	Copyright (c) 2005, 2006, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
