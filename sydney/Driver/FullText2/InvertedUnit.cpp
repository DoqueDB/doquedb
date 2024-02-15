// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedUnit.cpp --
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
#include "FullText2/InvertedUnit.h"

#include "FullText2/BtreeFile.h"
#include "FullText2/FakeError.h"
#include "FullText2/LeafFile.h"
#include "FullText2/OverflowFile.h"
#include "FullText2/Parameter.h"
#include "FullText2/SimpleListManager.h"
#include "FullText2/MessageAll_Class.h"

#include "Schema/File.h"

#include "Common/Message.h"
#include "Common/Assert.h"

#include "Os/Path.h"

#include "Exception/Object.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//  VARIABLE
	const Os::Ucs2 _pszBtreePath[] =	{'B','t','r','e','e',0};
	const Os::Ucs2 _pszLeafPath[] =		{'L','e','a','f',0};
	const Os::Ucs2 _pszOverflowPath[] = {'O','v','e','r','f','l','o','w',0};

	// 保持しているページ数がいくつ以上になったら一旦セーブするかの閾値
#ifdef SYD_ARCH64
	ParameterInteger _cSavePageCountThreshold(
		"FullText2_SavePageCountThreshold", 1000);
#else
	ParameterInteger _cSavePageCountThreshold(
		"FullText2_SavePageCountThreshold", 100);
#endif
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::InvertedUnit -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedSection& cInvertedSection_
//		転置ファイル
//	const FullText2::FileID& cFileID_
//		ファイルID
//	const Os::Path& cPath_
//		パス
//	bool bBatch_
//		バッチモードかどうか
//	int iUnitNumber
//		ユニット番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
InvertedUnit::InvertedUnit(InvertedSection& cInvertedSection_,
						   const Os::Path& cPath_,
						   bool bBatch_,
						   int iUnitNumber_)
	: InvertedUpdateFile(cInvertedSection_, cPath_),
	  m_pBtreeFile(0), m_pLeafFile(0), m_pOverflowFile(0),
	  m_bBatch(bBatch_), m_iUnitNumber(iUnitNumber_)
{
	// ファイルをattachする
	attach();
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::~InvertedUnit -- デストラクタ
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
InvertedUnit::~InvertedUnit()
{
	detach();
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::getUsedSize -- 使用ファイルサイズを得る
//
//	NOTES
//	このメソッドはマージが必要かどうかの確認に利用されるので、
//	転置ファイルの更新に影響を与えるファイルのみのサイズの合計である必要がある
//	このクラスを継承しても以下の３つのファイルのサイズのみにする必要があるので、
//	このクラスでは親クラスの実装を上書きしている
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		使用ファイルサイズ
//
//	EXCEPTIONS
//
ModUInt64
InvertedUnit::getUsedSize(const Trans::Transaction& cTransaction_)
{
	ModUInt64 size = 0;
	
	if (isMounted(cTransaction_))
	{
		size += m_pBtreeFile->getUsedSize(cTransaction_);
		size += m_pLeafFile->getUsedSize(cTransaction_);
		size += m_pOverflowFile->getUsedSize(cTransaction_);
	}
	return size;
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::create -- ファイルを作成する
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
InvertedUnit::create()
{
	int step = 0;
	try
	{
		m_pBtreeFile->create();
		step++;
		// これより前にB木と文書IDベクタを作成する必要あり
		m_pLeafFile->create(*this);
		step++;
		m_pOverflowFile->create();
		step++;

		flushAllPages();
	}
	catch (...)
	{
		try
		{
			recoverAllPages();

			switch (step)
			{
			case 2: m_pLeafFile->destroy();
			case 1: m_pBtreeFile->destroy();
			case 0:
				break;
			}

			rmdir(m_cPath);
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(getLockName(), false);
		}

		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::clear -- ファイルをクリアする
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
InvertedUnit::clear()
{
	if (m_pBtreeFile->isMounted())
	{
		try
		{
			m_pBtreeFile->clear(false);
			m_pOverflowFile->clear(false);
			// これより前にB木と文書IDベクタをクリアする必要あり
			m_pLeafFile->clear(*this, false);

			flushAllPages();
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
	}
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::verify -- 整合性検査を行う
//
//	NOTES
//
//	ARGUMENS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Admin::Verification::Treatment::Value eTreatment_
//		処理方法
//	Admin::Verification::Progress& cProgress_
//		経過報告
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedUnit::verify(const Trans::Transaction& cTransaction_,
					 const Admin::Verification::Treatment::Value eTreatment_,
					 Admin::Verification::Progress& cProgress_)
{
	if (isMounted(cTransaction_) == false)
		return;
		
	// 整合性検査の開始を通知する
	startVerification(cTransaction_, eTreatment_, cProgress_);

	try
	{
		// B木ファイルの整合性を検査する
			
		m_pBtreeFile->verify();

		// 索引単位の整合性を検査する

		ModAutoPointer<UpdateListManager> pListManager = getUpdateListManager();
		pListManager->verify(eTreatment_, cProgress_, m_cFileID.getPath());

		if (eTreatment_ & Admin::Verification::Treatment::Correct)
		{
			// 削除できるIDブロックがあれば削除する
			InvertedUnit::Map::Iterator k
				= m_mapDeleteIdBlock.begin();
			for (; k != m_mapDeleteIdBlock.end(); ++k)
			{
				// 転置リストを割り当てる
				if (pListManager->reset((*k).first,
										ListManager::AccessMode::Search)
					== true)
				{
					// 割り当てたリストからIDブロックを削除する
					pListManager->expungeIdBlock((*k).second);

					InvertedUnit::Vector::Iterator n
						= (*k).second.begin();
					for (; n != (*k).second.end(); ++n)
					{
						_SYDNEY_VERIFY_CORRECTED(
							cProgress_,
							m_cFileID.getPath(),
							Message::CorrectDisusedIdBlock(*n));
					}
				}

				// 必要なら全ファイルの変更を確定する
				saveAllPages();
			}
		}

		// 変更を確定する
		flushAllPages();
		// 整合性検査の終了を通知する
		endVerification();
	}
	catch (...)
	{
		recoverAllPages();
		endVerification();
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::saveAllPages
//		-- 必要ならすべてのページを確定する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		確定した場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedUnit::saveAllPages()
{
	ModSize c = m_pBtreeFile->getAttachedPageCount();
	c += m_pLeafFile->getAttachedPageCount();
	c += m_pOverflowFile->getAttachedPageCount();

	bool r = false;

	if (c > static_cast<ModSize>(_cSavePageCountThreshold.get()))
	{
		m_pBtreeFile->saveAllPages();
		m_pLeafFile->saveAllPages();
		m_pOverflowFile->saveAllPages();
		r = true;
	}

	return r;
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::attachLeafPage -- リーフページをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		アタッチするページID
//
//	RETURN
//	FullText2::LeafPage::PagePointer
//		リーフページ
//
//	EXCEPTIONS
//
LeafPage::PagePointer
InvertedUnit::attachLeafPage(PhysicalFile::PageID uiPageID_)
{
	return m_pLeafFile->attachPage(uiPageID_);
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::enterDeleteIdBlock -- 削除するIDブロックを登録する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//	ModUInt32 uiFirstDocumentID_
//		削除するIDブロックの先頭文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedUnit::enterDeleteIdBlock(const ModUnicodeString& cstrKey_,
								 ModUInt32 uiFirstDocumentID_)
{
	Map::Iterator i = m_mapDeleteIdBlock.find(cstrKey_);
	if (i != m_mapDeleteIdBlock.end())
	{
		Vector::Iterator j = (*i).second.begin();
		for (; j != (*i).second.end(); ++j)
			if ((*j) == uiFirstDocumentID_)
				return;
		(*i).second.pushBack(uiFirstDocumentID_);
	}
	else
	{
		m_mapDeleteIdBlock[cstrKey_].pushBack(uiFirstDocumentID_);
	}
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::enterExpungeFirstDocumentID
//		-- 先頭文書IDを削除したIDブロックのログを登録する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//	ModUInt32 uiOldDocumentID_
//		削除する前の先頭文書ID
//	ModUInt32 uiNewDocumentID_
//		削除した後に設定した先頭文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedUnit::enterExpungeFirstDocumentID(const ModUnicodeString& cstrKey_,
										  ModUInt32 uiOldDocumentID_,
										  ModUInt32 uiNewDocumentID_)
{
	m_mapExpungeFirstDocumentID.insert(
		ModPair<ModUnicodeString, ModUInt32>(cstrKey_, uiOldDocumentID_),
		uiNewDocumentID_);
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::getExpungeFirstDocumentID
//		-- 先頭文書IDを削除したIDブロックのログを検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//	ModUInt32 uiOldDocumentID_
//		削除する前の先頭文書ID
//
//	RETURN
//	ModUInt32
//		エントリが存在した場合は削除後のID、それ以外の場合はUndefinedDocumentID
//
//	EXCEPTIONS
//
ModUInt32
InvertedUnit::getExpungeFirstDocumentID(const ModUnicodeString& cstrKey_,
										ModUInt32 uiOldDocumentID_)
{
	ModUInt32 uiNewID = UndefinedDocumentID;
	IDMap::Iterator i = m_mapExpungeFirstDocumentID.find(
		ModPair<ModUnicodeString, ModUInt32>(cstrKey_, uiOldDocumentID_));
	if (i != m_mapExpungeFirstDocumentID.end())
	{
		uiNewID = (*i).second;
	}
	return uiNewID;
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::insertBtree -- B木に挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//	PhysicalFile::PageID uiPageID_
//		リーフファイルのページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedUnit::insertBtree(const ModUnicodeString& cstrKey_,
						  PhysicalFile::PageID uiPageID_)
{
	m_pBtreeFile->insert(cstrKey_, uiPageID_);
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::expungeBtree -- B木から削除する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		 索引単位
//	PhysicalFile::PageID uiPageID_
//		リーフファイルのページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedUnit::expungeBtree(const ModUnicodeString& cstrKey_,
						   PhysicalFile::PageID uiPageID_)
{
	m_pBtreeFile->expunge(cstrKey_);
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::updateBtree -- B木を更新する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey1_
//		更新前の索引単位
//	PhysicalFile::PageID uiPageID1_
//		更新前のページID
//	const ModUnicodeString& cstrKey2_
//		更新後の索引単位
//	PhysicalFile::PageID uiPageID2_
//		更新後のページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedUnit::updateBtree(const ModUnicodeString& cstrKey1_,
						  PhysicalFile::PageID uiPageID1_,
						  const ModUnicodeString& cstrKey2_,
						  PhysicalFile::PageID uiPageID2_)
{
	m_pBtreeFile->update(cstrKey1_, uiPageID1_, cstrKey2_, uiPageID2_);
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::searchBtree -- B木を検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//	PhysicalFile::PageID uiPageID_
//		リーフファイルのページID(検索結果)
//
//	RETURN
//	bool
//		検索にヒットした場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedUnit::searchBtree(const ModUnicodeString& cstrKey_,
						  PhysicalFile::PageID& uiPageID_)
{
	if (m_pBtreeFile && m_pBtreeFile->isMounted())
	{
		if (m_pBtreeFile->search(cstrKey_, uiPageID_) == true)
			return true;
	}
	return false;
}

#ifndef SYD_COVERAGE
//
//	FUNCTION public
//	FullText2::InvertedUnit::reportFile -- ファイル状況を報告する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	ModOstream& stream_
//		出力ストリーム
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedUnit::reportFile(const Trans::Transaction& cTransaction_,
						 Buffer::Page::FixMode::Value eFixMode_,
						 ModOstream& stream_)
{
	open(cTransaction_, eFixMode_);

	if (isMounted(cTransaction_))
	{
		m_pBtreeFile->reportFile(cTransaction_, stream_);
		m_pLeafFile->reportFile(cTransaction_, stream_);
		m_pOverflowFile->reportFile(cTransaction_, stream_);
	}
}
#endif

//
//	FUNCTION public
//	FullText2::InvertedUnit::isMounted -- ファイルがマウントされているか
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	bool
//		マウントされている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedUnit::isMounted(const Trans::Transaction& cTransaction_) const
{
	return m_pBtreeFile->isMounted(cTransaction_);
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::isAccessible -- ファイルが存在するか
//
//	NOTES
//
//	ARGUMENTS
//	bool force_
//		強制モードかどうか(default false)
//
//	RETURN
//	bool
//		ファイルが存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedUnit::isAccessible(bool force_) const 
{
	return m_pBtreeFile->isAccessible(force_);
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::getUpdateListManager
//		-- 更新時に利用するListManagerを得る
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	FullText2::UpdateListManager*
//		更新用のListManager
//
//	EXCEPTIONS
//
UpdateListManager*
InvertedUnit::getUpdateListManager()
{
	return new SimpleListManager(m_cSection.getFile(), this);
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::expungeIdBlock -- 削除対象のIDブロックを削除する
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
InvertedUnit::expungeIdBlock()
{
	if (m_mapDeleteIdBlock.isEmpty() == ModTrue)
		return;
	
	ModAutoPointer<UpdateListManager> pListManager = getUpdateListManager();
	
	Map::Iterator i = m_mapDeleteIdBlock.begin();
	for (; i != m_mapDeleteIdBlock.end(); ++i)
	{
		if (pListManager->reset((*i).first, ListManager::AccessMode::Search)
			== true)
		{
			// IDブロックを削除する
			pListManager->expungeIdBlock((*i).second);

			// 広範囲にページを更新する恐れがあるので、
			// 毎回ページを save する

			saveAllPages();
		}
	}

	// 削除データをクリアする
	m_mapDeleteIdBlock.erase(m_mapDeleteIdBlock.begin(),
							 m_mapDeleteIdBlock.end());
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::isNeedVacuum -- バキュームが必要かどうか
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		キー
//	int iNewExpungeCount_
//		今回新たに削除した数
//
//	RETURN
//	bool
//		バキュームが必要な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedUnit::isNeedVacuum(const ModUnicodeString& cstrKey_,
						   int iNewExpungeCount_)
{
	return false;
}

//
//	FUNCTION public
//	FullText2::InvertedUnit::clearExpungeCount
//		バキュームが必要かどうかのカウントをクリアする
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		カウントをクリアするキー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedUnit::clearExpungeCount(const ModUnicodeString& cstrKey_)
{
}

//
//	FUNCTION protected
//	FullText2::InvertedUnit::attach -- ファイルをattachする
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
InvertedUnit::attach()
{
	if (!m_pBtreeFile)
	{
		Os::Path path = m_cPath;
		path.addPart(_pszBtreePath);
		m_pBtreeFile = new BtreeFile(m_cSection.getFileID(),
									 path, m_bBatch);

		path = m_cPath;
		path.addPart(_pszLeafPath);
		m_pLeafFile = new LeafFile(m_cSection.getFileID(),
								   path, m_bBatch);

		path = m_cPath;
		path.addPart(_pszOverflowPath);
		m_pOverflowFile = new OverflowFile(m_cSection.getFileID(),
										   path, m_bBatch);

		MultiFile::pushBackSubFile(m_pBtreeFile);
		MultiFile::pushBackSubFile(m_pLeafFile);
		MultiFile::pushBackSubFile(m_pOverflowFile);
	}
}

//
//	FUNCTION protected
//	FullText2::InvertedUnit::detach -- ファイルをdetachする
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
InvertedUnit::detach()
{
	delete m_pBtreeFile, m_pBtreeFile = 0;
	delete m_pLeafFile, m_pLeafFile = 0;
	delete m_pOverflowFile, m_pOverflowFile = 0;
}

//
//	FUNCTION protected
//	FullText2::InvertedUnit::startVerification -- 整合性検査を開始する
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
InvertedUnit::startVerification(
	const Trans::Transaction& cTransaction_,
	Admin::Verification::Treatment::Value uiTreatment_,
	Admin::Verification::Progress& cProgress_)
{
	int n = 0;
	try
	{
		m_pBtreeFile->startVerification(cTransaction_,
										uiTreatment_,
										cProgress_);
		++n;
		m_pLeafFile->startVerification(cTransaction_,
									   uiTreatment_,
									   cProgress_);
		++n;
		m_pOverflowFile->startVerification(cTransaction_,
										   uiTreatment_,
										   cProgress_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		switch (n) {
		case 2: m_pLeafFile->endVerification();
		case 1: m_pBtreeFile->endVerification();
		default:
			;
		}

		_TRMEISTER_RETHROW;
	}
}

//
//	FUNCTION protected
//	FullText2::InvertedUnit::endVerification -- 整合性検査を終了する
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
InvertedUnit::endVerification()
{
	m_mapDeleteIdBlock.erase(m_mapDeleteIdBlock.begin(),
							 m_mapDeleteIdBlock.end());

	m_pBtreeFile->endVerification();
	m_pLeafFile->endVerification();
	m_pOverflowFile->endVerification();
}

//
//	FUNCTION protected
//	FullText2::InvertedUnit::move -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cNewPath_
//		移動先のパス
//	bool bDirectory_
//		ディレクトリの操作をするかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedUnit::move(const Trans::Transaction& cTransaction_,
				   const Os::Path& cNewPath_,
				   bool bDirectory_)
{
	// マウントの有無や実体の存在の有無を確認せずに
	// とにかく移動する
	//
	//【注意】	そうしないと下位層で管理している
	//			情報がメンテナンスされない

	bool accessible = (bDirectory_ && isAccessible() &&
					   Os::Path::compare(cNewPath_, m_cPath)
					   == Os::Path::CompareResult::Unrelated);
	int step = 0;
	try
	{
		Os::Path path = cNewPath_;
		path.addPart(_pszBtreePath);
		m_pBtreeFile->move(cTransaction_, path);
		step++;

		path = cNewPath_;
		path.addPart(_pszLeafPath);
		m_pLeafFile->move(cTransaction_, path);
		step++;

		path = cNewPath_;
		path.addPart(_pszOverflowPath);
		m_pOverflowFile->move(cTransaction_, path);
		step++;
		
		if (accessible)
			rmdir(m_cPath);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
#ifdef SYD_FAKE_ERROR
		FakeErrorMessage << "InvertedUnit::move (step="
						 << step << ")" << ModEndl;
#endif
		try
		{
			switch (step)
			{
			case 3:
				{
					Os::Path path = m_cPath;
					path.addPart(_pszOverflowPath);
					m_pOverflowFile->move(cTransaction_, path);
				}
			case 2:
				{
					Os::Path path = m_cPath;
					path.addPart(_pszLeafPath);
					m_pLeafFile->move(cTransaction_, path);
				}
			case 1:
				{
					Os::Path path = m_cPath;
					path.addPart(_pszBtreePath);
					m_pBtreeFile->move(cTransaction_, path);
				}
			}
			if (accessible)
				rmdir(cNewPath_);
		}
		catch (...)
		{
			SydErrorMessage << "Recovery failed." << ModEndl;
			Schema::File::setAvailability(getLockName(), false);
		}

		_SYDNEY_RETHROW;
	}

	m_cPath = cNewPath_;
}

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
