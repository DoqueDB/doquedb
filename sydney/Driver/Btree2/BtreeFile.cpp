// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreeFile.cpp --
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2014, 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Btree2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Btree2/BtreeFile.h"
#include "Btree2/Condition.h"
#include "Btree2/LockManager.h"
#include "Btree2/Parameter.h"

#include "Checkpoint/Database.h"

#include "Exception/Unexpected.h"

_SYDNEY_USING
_SYDNEY_BTREE2_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$::_iCachePageSize -- ページをキャッシュする数
	//
	ParameterInteger _iCachePageSize("Btree2_CachePageSize", 5);

}

//
//	FUNCTION public
//	Btree2::BtreeFile::BtreeFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Btree2::FileID& cFileID_
//		ファイルID
//	const Os::Path& cSubPath_
//		サブディレクトリ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
BtreeFile::BtreeFile(const FileID& cFileID_, const Os::Path& cSubPath_)
	: File(_iCachePageSize.get()), m_cFileID(cFileID_), m_pHeaderPageInstance(0)
{
	// 物理ファイルをattachする
	attach(cFileID_, cFileID_.getPageSize(), cSubPath_);
	// データクラスを作成する
	createData();
	// 比較クラスを作成する
	createCompare();
}

//
//	FUNCTION public
//	Btree2::BtreeFile::~BtreeFile -- デストラクタ
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
BtreeFile::~BtreeFile()
{
	if (m_pHeaderPageInstance) delete m_pHeaderPageInstance;
	// 物理ファイルをdetachする
	detach();
}

//
//	FUNCTION public
//	Btree2::BtreeFile::create -- ファイルを作成する
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
BtreeFile::create()
{
	// まず下位を呼ぶ
	File::create();
	try
	{
		// ヘッダーページを初期化する
		initializeHeaderPage();
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		recoverAllPages();
		File::destroy();
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Btree2::BtreeFile::open -- オープンする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	LogicalFile::OpenOption::OpenMode::Value eOpenMode_
///		オープンモード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeFile::open(const Trans::Transaction& cTransaction_,
				LogicalFile::OpenOption::OpenMode::Value eOpenMode_)
{
	File::open(cTransaction_, eOpenMode_);
}

//
//	FUNCTION public
//	Btree2::BtreeFile::close -- クローズする
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
BtreeFile::close()
{
	m_pHeaderPage = 0;
	File::close();
}

//
//	FUNCTION public
//	Btree2::BtreeFile::getEstimateCount -- 検索結果件数を見積もる
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::Condition* pCondition_
//		検索条件
//
//	RETURN
//	ModUInt32
//		概算の検索件数
//
//	EXCEPTIONS
//
ModUInt32
BtreeFile::getEstimateCount(Condition* pCondition_)
{
	// 全登録数
	ModUInt32 count = getHeaderPage()->getCount();
	if (count == 0)
		return count;
	
	if (pCondition_->getFetchField() != 0)
	{
		// フェッチでの見積もり
		count = getEstimateCountForFetch(pCondition_, count);
	}
	else
	{
		// 検索での見積もり
		count = getEstimateCountForSearch(pCondition_, count);
	}
	
	return count;
}

//
//	FUNCTION public
//	Btree2::BtreeFile::flushAllPages -- 全ページをフラッシュする
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
BtreeFile::flushAllPages()
{
	// ヘッダーをデタッチする
	m_pHeaderPage = 0;
	// 全ページをフラッシュする
	File::flushAllPages();
	// ロック情報を削除する
	if (m_vecPageID.getSize())
		LockManager::erase(m_cFileID.getLockName(), m_vecPageID);
	// 更新ページIDを初期化する
	m_vecPageID.clear();
}

//
//	FUNCTION public
//	Btree2::BtreeFile::recoverAllPages -- 全ページを元に戻す
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
BtreeFile::recoverAllPages()
{
	// ヘッダーをデタッチする
	m_pHeaderPage = 0;
	// 全ページをフラッシュする
	File::recoverAllPages();
	// 更新ページIDを初期化する
	m_vecPageID.clear();
}

//
//	FUNCTION public
//	Btree2::BtreeFile::dirtyHeaderPage -- ヘッダーページをdirtyにする
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
BtreeFile::dirtyHeaderPage()
{
	getHeaderPage()->dirty();
}

//
//	FUNCTION public
//	Btree2::BtreeFile::getHeaderPage -- ヘッダーページを得る
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPhysicalPage_ (default 0)
//		ヘッダーページの物理ページ
//
//	RETURN
//	Btree2::HeaderPage::PagePointer
//		ヘッダーページ
//
//	EXCEPTIONS
//
HeaderPage::PagePointer
BtreeFile::getHeaderPage(PhysicalFile::Page* pPhysicalPage_)
{
	if (m_pHeaderPageInstance == 0)
	{
		m_pHeaderPageInstance = new HeaderPage(*this);
	}
	if (m_pHeaderPage == 0)
	{
		// 先頭のページを得る
		if (pPhysicalPage_ == 0) pPhysicalPage_ = attachPhysicalPage(0);
		m_pHeaderPage = m_pHeaderPageInstance;
		m_pHeaderPage->setPhysicalPage(pPhysicalPage_);
	}
	return m_pHeaderPage;
}

//
//	FUNCTION protected
//	Btree2::BtreeFile::insertLock -- ロック情報を登録する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		登録するページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeFile::insertLock(PhysicalFile::PageID uiPageID_)
{
	if (getTransaction().isNoVersion() == true && m_bSubFile == false)
	{
		LockManager::insert(m_cFileID.getLockName(),
							uiPageID_,
							this);
	}
}

//
//	FUNCTION protected
//	Btree2::BtreeFile::eraseLock -- ロック情報を削除する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		登録するページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeFile::eraseLock(PhysicalFile::PageID uiPageID_)
{
	if (getTransaction().isNoVersion() == true && m_bSubFile == false)
	{
		LockManager::erase(m_cFileID.getLockName(),
						   uiPageID_,
						   this);
	}
}

//
//	FUNCTION protected
//	Btree2::BtreeFile::checkLock -- ロック情報が存在するか
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		登録するページID
//
//	RETURN
//	bool
//		エントリが存在している場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
BtreeFile::checkLock(PhysicalFile::PageID uiPageID_)
{
	bool result = true;
	if (getTransaction().isNoVersion() == true && m_bSubFile == false)
	{
		result = LockManager::check(m_cFileID.getLockName(),
									uiPageID_,
									this);
	}
	return result;
}

//
//	FUNCTION protected
//	Btree2::BtreeFile::detachPage -- ページをデタッチする
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::Page* pPage_
//		デタッチするページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeFile::detachPage(Page* pPage_)
{
	if (pPage_->getID() == 0)
	{
		// ヘッダーページ
		
		// 確定前処理
		if (pPage_->isDirty() == true)
			pPage_->preFlush();

		PhysicalFile::Page::UnfixMode::Value mode
			= pPage_->isDirty() ? PhysicalFile::Page::UnfixMode::Dirty
			: PhysicalFile::Page::UnfixMode::NotDirty;

		m_pPhysicalFile->detachPage(pPage_->m_pPhysicalPage, mode);
	}
	else
	{
		File::detachPage(pPage_);
	}
}

//
//	FUNCTION protected
//	Btree2::BtreeFile::expungeConstraintLockEntry
//		-- 可能なら、制約ロックのためのエントリを削除する
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& trans_
//		トランザクション
//	const Lock::FileName& name_
//		ファイルのロック名
//	bool& modified_
//		ファイルを更新したかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeFile::expungeConstraintLockEntry(Trans::Transaction& trans_,
									  const Lock::FileName& name_,
									  bool& modified_)
{
	// すべての行のロックを試みて、ロック出来た場合のみ
	// ファイルをクリアする
	//
	//【注意】	このメソッドはサブファイルでのみ実行される

	if (tryLockAllEntry(trans_, name_))
	{
		// すべての行がロックできたので、ファイルをクリアする

		clear(trans_, name_);
		modified_ = true;
	}
}

//
//	FUNCTION protected
//	Btree2::BtreeFile::tryLockAllEntry
//		-- すべてのエントリの行ロックを試みる
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTransaction_
//		トランザクション
//	const Lock::FileName& name_
//		ロック名
//
//	RETURN
//	bool
//		すべてロック出来た場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
BtreeFile::tryLockAllEntry(Trans::Transaction& cTrans_,
						   const Lock::FileName& name_)
{
	// すべてロックできたかどうか
	bool lockAll = false;

	// まずは、Readでオープンする
	open(cTrans_, LogicalFile::OpenOption::OpenMode::Read);
	
	// 検索する
	Condition condition(m_cFileID);	// 全件検索なので条件なし
	search(&condition, false);
	Common::DataArrayData dummy;
	unsigned char bitset = 0;
	unsigned int rowid = 0;

	// すべての結果を取得してロックを試みる

	while (get(bitset, dummy, rowid))
	{
		// ロックする
		
		Lock::TupleName n(name_.getDatabasePart(),
						  name_.getTablePart(),
						  rowid);
		if (cTrans_.lock(n, Lock::Mode::X,
						 Lock::Duration::Pulse, 0) == false)
		{
			// ロックできなかった
			
			lockAll = false;
			break;
		}

		// ロックできた
		lockAll = true;
	}
	
	// ファイルキャッシュをクリアする
	flushAllPages();
	
	// クローズする
	close();

	return lockAll;
}

//
//	FUNCTION protected
//	Btree2::BtreeFile::clear -- ファイルの中身をクリアする
//
//	NOTES
//
//	ARGUMENTS
// 	Trans::Transaction& cTransaction_
//		トランザクション
//	const Lock::FileName& name_
//		ロック名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeFile::clear(Trans::Transaction& cTransaction_,
				 const Lock::FileName& name_)
{
	//
	//【注意】	このメソッドはサブファイルでのみ実行される
	//
	
	// 更新モードでオープンする
	open(cTransaction_, LogicalFile::OpenOption::OpenMode::Update);
	
	try
	{
		// まず下位を呼ぶ
		File::clear();
		// ヘッダーページを初期化する
		initializeHeaderPage();
		dirtyHeaderPage();
	}
	catch (...)
	{
		try
		{
			// ファイルの変更を破棄する
			recoverAllPages();
			// クローズする
			close();
		}
		catch (...)
		{
			// 利用不可
			Checkpoint::Database::setAvailability(name_, false);
		}

		_SYDNEY_RETHROW;
	}

	try
	{
		// ファイルの変更を確定する
		flushAllPages();
		// クローズする
		close();
	}
	catch (...)
	{
		// 利用不可
		Checkpoint::Database::setAvailability(name_, false);
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION private
//	Btree2::BtreeFile::initializeHeaderPage -- ヘッダーページを初期化する
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
BtreeFile::initializeHeaderPage()
{
	// ページID=0のページを得る
	PhysicalFile::Page* p = File::allocatePage();
	if (p->getID() != 0)
		_SYDNEY_THROW0(Exception::Unexpected);

	HeaderPage::PagePointer pPage = getHeaderPage(p);
	pPage->initialize();
}

//
//	FUNCTION private
//	Btree2::BtreeFile::createData -- データクラスを作成する
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
BtreeFile::createData()
{
	// キー
	m_cKeyData.setType(m_cFileID.getKeyType(), getKeySize(),
					   m_cFileID.isUseHeader());

	// ノードページ
	ModVector<Data::Type::Value> type = m_cFileID.getKeyType();
	type.pushBack(Data::Type::UnsignedInteger);	// PageID分
	m_cNodeData.setType(type, getNodeSize(), m_cFileID.isUseHeader());

	// リーフページ
	type = m_cFileID.getKeyType();
	const ModVector<Data::Type::Value>& value = m_cFileID.getValueType();
	ModVector<Data::Type::Value>::ConstIterator i = value.begin();
	for (; i != value.end(); ++i)
		type.pushBack(*i);
	m_cLeafData.setType(type, getLeafSize(), m_cFileID.isUseHeader());
}

//
//	FUNCTION private
//	Btree2::BtreeFile::createCompare -- 比較クラスを作成する
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
BtreeFile::createCompare()
{
	ModVector<Data::Type::Value> key = m_cFileID.getKeyType();
	if (m_bSubFile)
	{
		while (key.getSize() > m_cFileID.getRealKeyFieldCount())
		{
			// 本当のキーだけにするため、末尾のデータを削除する
			
			key.popBack();
		}
	}
	
	m_cCompare.setType(key, true, m_cFileID.isUseHeader());
	m_cCompare.setUsingIntegrityCheckField(m_cFileID.getRealKeyFieldCount());

	if (m_cFileID.isUnique())
	{
		// ユニーク制約のB木の場合、キーだけの比較クラスを作る
		
		ModVector<Data::Type::Value> key0 = m_cFileID.getKeyType();
		while (key0.getSize() > m_cFileID.getRealKeyFieldCount())
		{
			key0.popBack();
		}

		m_cKeyCompare.setType(key0, true, m_cFileID.isUseHeader());
		m_cKeyCompare.setUsingIntegrityCheckField(
			m_cFileID.getRealKeyFieldCount());
	}
	else
	{
		// それ以外のB木の場合は、同じもの

		m_cKeyCompare.setType(key, true, m_cFileID.isUseHeader());
		m_cKeyCompare.setUsingIntegrityCheckField(
			m_cFileID.getRealKeyFieldCount());
	}
}

//
//	FUNCTION private
//	Btree2::BtreeFile::getKeySize -- キーのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		すべてがFixedの場合、そのサイズを得る。それ以外の場合は0
//
//	EXCEPTIONS
//
ModSize
BtreeFile::getKeySize()
{
	ModSize size = 0;
	if (m_cFileID.isFixed() == true)
	{
		ModSize n = m_cFileID.getKeyType().getSize();
		const ModVector<ModSize>& s = m_cFileID.getFieldSize();
		ModVector<ModSize>::ConstIterator j = s.begin();
		for (ModSize i = 0; i < n; ++i)
		{
			size += *j++;
		}
	}
	return size / sizeof(ModUInt32);
}

//
//	FUNCTION private
//	Btree2::BtreeFile::getNodeSize -- ノードページのエントリサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		すべてがFixedの場合、そのサイズを得る。それ以外の場合は0
//
//	EXCEPTIONS
//
ModSize
BtreeFile::getNodeSize()
{
	ModSize size = 0;
	if (m_cFileID.isFixed() == true)
	{
		ModSize n = m_cFileID.getKeyType().getSize();
		const ModVector<ModSize>& s = m_cFileID.getFieldSize();
		ModVector<ModSize>::ConstIterator j = s.begin();
		for (ModSize i = 0; i < n; ++i)
		{
			size += *j++;
		}
		ModUInt32* p = 0;
		size += (Data::UnsignedInteger::getSize(p) * sizeof(ModUInt32));
	}
	return size / sizeof(ModUInt32);
}

//
//	FUNCTION private
//	Btree2::BtreeFile::getLeafSize -- リーフページのエントリサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		すべてがFixedの場合、そのサイズを得る。それ以外の場合は0
//
//	EXCEPTIONS
//
ModSize
BtreeFile::getLeafSize()
{
	ModSize size = 0;
	if (m_cFileID.isFixed() == true)
	{
		const ModVector<ModSize>& s = m_cFileID.getFieldSize();
		ModVector<ModSize>::ConstIterator j = s.begin();
		for (; j != s.end(); ++j)
		{
			size += *j;
		}
	}
	return size / sizeof(ModUInt32);
}

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2007, 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
