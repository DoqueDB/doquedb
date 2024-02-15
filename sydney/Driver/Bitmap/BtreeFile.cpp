// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreeFile.cpp --
// 
// Copyright (c) 2005, 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "Bitmap/BtreeFile.h"
#include "Bitmap/BtreePage.h"
#include "Bitmap/Data.h"
#include "Bitmap/Condition.h"
#include "Bitmap/Compare.h"
#include "Bitmap/FileID.h"
#include "Bitmap/MessageAll_Class.h"

#include "Os/AutoCriticalSection.h"

#include "Exception/BadArgument.h"
#include "Exception/NullabilityViolation.h"
#include "Exception/VerifyAborted.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

//
//	FUNCTION public
//	Bitmap::BtreeFile::BtreeFile -- コンストラクタ
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
BtreeFile::BtreeFile(const FileID& cFileID_)
	: File(cFileID_),
	  m_pCondition(0),
	  m_cList(&BtreePage::m_pNext, &BtreePage::m_pPrev)
{
	// データクラスを作成する
	createData();
	// 比較クラスを作成する
	createCompare();
}

//
//	FUNCTION public
//	Bitmap::BtreeFile::~BtreeFile -- デストラクタ
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
}

//
//	FUNCTION public
//	Bitmap::BtreeFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cKey_
//		キー
//	const Common::Data& cValue_
//		バリュー
//	bool isNull_
//		配列の場合かつデータがnullの場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeFile::insert(const Common::Data& cKey_,
				  const Common::Data& cValue_,
				  bool isNull_)
{
	if (cKey_.isNull())
	{
		// null
		if (isNull_)
			getHeaderPage().setAllNullID(cValue_);
		else
			getHeaderPage().setNullID(cValue_);
		return;
	}
	
	//
	//	入力チェック
	//

	ModSize uiMaxSize = m_cFileID.getTupleSize() / sizeof(ModUInt32);

	//	【注意】
	//	ここでのサイズチェックは、つねにバリューがUInt32として行っている
	//	FileID内でも同様のチェックになっており、それと整合を取るために行っている
	
	// サイズチェック
	const Data& cData = getLeafData();
	ModSize size = cData.getSize(cKey_) + Data::UnsignedInteger::getSize(0);
	if (size > uiMaxSize)
		// 通常ありえない
		_SYDNEY_THROW0(Exception::BadArgument);
	if (size == Data::UnsignedInteger::getSize(0))	{ // nullはだめ
		ModUnicodeOstrStream stream;
		stream << "field(0)";
		_SYDNEY_THROW1(Exception::NullabilityViolation, stream.getString());
	}

	// 挿入データ作成
	size = 0;
	ModUInt32 p[FileID::MAX_SIZE];
	makeData(cKey_, cValue_, cData, p, size);

	// リーフページを得る
	BtreePage::PagePointer pLeafPage = getLeafPage(p, getCompare());
	if (pLeafPage == 0)
	{
		// ルートページがないので新たに確保する
		pLeafPage = allocatePage(PhysicalFile::ConstValue::UndefinedPageID,
								 PhysicalFile::ConstValue::UndefinedPageID,
								 BtreePage::PagePointer());
		pLeafPage->setLeaf();
		
		// ヘッダーページに設定する
		HeaderPage& cHeaderPage = getHeaderPage();
		cHeaderPage.setRootPageID(pLeafPage->getID());
		cHeaderPage.setLeftLeafPageID(pLeafPage->getID());
		cHeaderPage.setRightLeafPageID(pLeafPage->getID());
		cHeaderPage.incrementStepCount();
	}
	
	// 挿入する
	pLeafPage->insertEntry(p, size);
}

//
//	FUNCTION public
//	Bitmap::BtreeFile::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cKey_
//		キー
//	const Common::Data& cValue_
//		バリュー
//	bool isNull_
//		配列の場合かつデータがnullの場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeFile::expunge(const Common::Data& cKey_,
				   const Common::Data& cValue_,
				   bool isNull_)
{
	if (cKey_.isNull())
	{
		// null
		if (isNull_)
			getHeaderPage().clearAllNullID();
		else
			getHeaderPage().clearNullID();
		return;
	}
	
	// 削除のためのキーデータ作成
	ModSize size = 0;
	ModUInt32 p[FileID::MAX_SIZE];
	makeData(cKey_, cValue_, getLeafData(), p, size);

	// リーフページを得る
	BtreePage::PagePointer pLeafPage = getLeafPage(p, getCompare());
	if (pLeafPage == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	// 削除する
	pLeafPage->expungeEntry(p, size);
}

//
//	FUNCTION public
//	Bitmap::BtreeFile::update -- 更新する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cKey_
//		キー
//	const Common::Data& cValue_
//		バリュー
//	bool isNull_
//		配列の場合かつデータがnullの場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeFile::update(const Common::Data& cKey_,
				  const Common::Data& cValue_,
				  bool isNull_)
{
	if (cKey_.isNull())
	{
		// null
		if (isNull_)
			getHeaderPage().setAllNullID(cValue_);
		else
			getHeaderPage().setNullID(cValue_);
		return;
	}
	
	// 更新のためのキーデータ作成
	ModSize size = 0;
	ModUInt32 p[FileID::MAX_SIZE];
	makeData(cKey_, cValue_, getLeafData(), p, size);

	// リーフページを得る
	BtreePage::PagePointer pLeafPage = getLeafPage(p, getCompare());
	if (pLeafPage == 0)
		_SYDNEY_THROW0(Exception::BadArgument);

	// 更新する
	pLeafPage->updateEntry(p, size);
}

//
//	FUNCTION public
//	Bitmap::BtreeFile::search -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::Condition* pCondition_
//		検索条件
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeFile::search(Condition* pCondition_)
{
	m_pCondition = pCondition_;
	// 検索前準備を行う
	preSearch();
}

//
//	FUNCTION public
//	Bitmap::BtreeFile::get -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cValue_
//		バリュー
//
//	RETURN
//	bool
//		結果が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
BtreeFile::get(Common::Data& cValue_)
{
	return next(cValue_, 0);
}

//
//	FUNCTION public
//	Bitmap::BtreeFile::get -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cKey_
//		キー
//	Common::Data& cValue_
//		バリュー
//
//	RETURN
//	bool
//		結果が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
BtreeFile::get(Common::Data& cKey_, Common::Data& cValue_)
{
	return next(cValue_, &cKey_);
}

//
//	FUNCTION public
//	Bitmap::BtreeFile::getWithLatch -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cKey_
//		キー
//	Common::Data& cValue_
//		バリュー
//
//	RETURN
//	bool
//		結果が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
BtreeFile::getWithLatch(Common::Data& cKey_, Common::Data& cValue_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	return next(cValue_, &cKey_);
}

//
//	FUNCTION public
//	Bitmap::BtreeFile::get -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cKey_
//		ページIDを取得するキー
//	bool isNull_
//		配列かつデータがnullの場合はtrue、それ以外の場合はfalse
//	Common::Data& cValue_
//		取得するバリュー
//
//	RETURN
//	bool
//		取得できた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
BtreeFile::get(const Common::Data& cKey_, bool isNull_,
			   Common::Data& cValue_)
{
	if (cKey_.isNull())
	{
		// null データである
		if (isNull_)
			return getHeaderPage().getAllNullID(cValue_);
		else
			return getHeaderPage().getNullID(cValue_);
	}
	
	// 検索データ作成
	ModSize size = 0;
	ModUInt32 p[FileID::MAX_SIZE];
	makeData(cKey_, cValue_, getLeafData(),
			 p, size);

	// リーフページを得る
	BtreePage::PagePointer pLeafPage = getLeafPage(p, getCompare());
	if (pLeafPage == 0)
		return false;

	// ページ内を検索する
	BtreePage::Iterator i = pLeafPage->find(p, getCompare());
	if (i == pLeafPage->end())
		return false;

	// ページIDを得る
	getLeafData().getValue(*i, cValue_);

	return true;
}

//
//	FUNCTION public
//	Bitmap::BtreeFile::flushAllPages -- 全ページをフラッシュする
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
	detachAll();
	File::flushAllPages();
}

//
//	FUNCTION public
//	Bitmap::BtreeFile::recoverAllPages -- 全ページの変更を破棄する
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
	detachAll();
	File::recoverAllPages();
}

//
//	FUNCTION public
//	Bitmap::BtreeFile::freePage -- ページを開放する
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::BtreePage* pPage_
//		フリーするページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreeFile::freePage(BtreePage* pPage_)
{
	m_cList.erase(*pPage_);
	pPage_->setFree();
}

//
//	FUNCTION public
//	Bitmap::BtreeFile::verify -- 整合性チェック
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
BtreeFile::verify()
{
	BtreePage::PagePointer pRootPage = getRootPage();
	if (pRootPage != 0)
	{
		if (pRootPage->getNextPageID()
			!= PhysicalFile::ConstValue::UndefinedPageID
			|| pRootPage->getPrevPageID()
			!= PhysicalFile::ConstValue::UndefinedPageID)
		{
			_SYDNEY_VERIFY_INCONSISTENT(getProgress(),
										getPath(),
										Message::IllegalRootPageID());
			_SYDNEY_THROW0(Exception::VerifyAborted);
		}
		
		// ページをverify
		pRootPage->verify();
	}

	// 左端リーフページのチェック
	if (getHeaderPage().getLeftLeafPageID()
		!= PhysicalFile::ConstValue::UndefinedPageID)
	{
		BtreePage::PagePointer pPage
			= attachPage(getHeaderPage().getLeftLeafPageID(),
						 BtreePage::PagePointer());
		if (pPage->isLeaf() != true
			|| pPage->getPrevPageID()
			!= PhysicalFile::ConstValue::UndefinedPageID)
		{
			_SYDNEY_VERIFY_INCONSISTENT(getProgress(),
										getPath(),
										Message::IllegalLeftLeafPageID());
			_SYDNEY_THROW0(Exception::VerifyAborted);
		}
	}

	// 右端リーフページのチェック
	if (getHeaderPage().getRightLeafPageID()
		!= PhysicalFile::ConstValue::UndefinedPageID)
	{
		BtreePage::PagePointer pPage
			= attachPage(getHeaderPage().getRightLeafPageID(),
						 BtreePage::PagePointer());
		if (pPage->isLeaf() != true
			|| pPage->getNextPageID()
			!= PhysicalFile::ConstValue::UndefinedPageID)
		{
			_SYDNEY_VERIFY_INCONSISTENT(getProgress(),
										getPath(),
										Message::IllegalLeftLeafPageID());
			_SYDNEY_THROW0(Exception::VerifyAborted);
		}
	}
}

//
//	FUNCTION private
//	Bitmap::BtreeFile::attachPage -- ページを得る
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPageID_
//		ページID
//	const Bitmap::BtreePage::PagePointer& pParent_,
//		親ページ
//	Buffer::Page::FixMode::Value eFixMode_ (default Unknown)
//		FIXモード
//
//	RETURN
//	Bitmap::BtreePage::PagePointer
//		ページ
//
//	EXCEPTIONS
//
BtreePage::PagePointer
BtreeFile::attachPage(PhysicalFile::PageID uiPageID_,
					  const BtreePage::PagePointer& pParent_,
					  Buffer::Page::FixMode::Value eFixMode_)
{
	if (uiPageID_ == PhysicalFile::ConstValue::UndefinedPageID)
		return BtreePage::PagePointer();

	BtreePage::PagePointer pPage;

	ModSize readOnly = 0;
	PageList::Iterator i = m_cList.begin();
	PageList::Iterator j = m_cList.end();
	for (; i != m_cList.end(); ++i)
	{
		if ((*i).getID() == uiPageID_)
		{
			// 見つかった
			j = i;
			break;
		}
		if ((*i).isDirty() == false && (*i).getReference() == 0)
		{
			// dirtyじゃないページは5ページまでそれ以上ある場合、
			// 最後の要素をdetachして再利用する

			if (++readOnly > 5)
				j = i;
		}
	}

	if (j != m_cList.end())
	{
		// 検索でヒットした
		if ((*j).getID() != uiPageID_)
		{
			// ページが違うのでdetachする
			(*j).detachPhysicalPage();
			// 正しいページをattachする
			(*j).setPhysicalPage(attachPhysicalPage(uiPageID_, eFixMode_));
		}

		// 先頭にする
		m_cList.splice(m_cList.begin(), m_cList, j);

		pPage = &(*j);
	}
	else
	{
		// 見つからない
		PhysicalFile::Page* pPhysicalPage = attachPhysicalPage(uiPageID_,
															   eFixMode_);
		// ないので新たに確保する
		pPage = new BtreePage(*this);
		pPage->setPhysicalPage(pPhysicalPage);

		// 先頭に追加する
		m_cList.pushFront(*pPage);
	}

	// 親を設定する
	pPage->setParentPage(pParent_);

	return pPage;
}

//
//	FUNCTION public
//	Bitmap::BtreeFile::allocatePage -- 新しいページを確保する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPrevPageID_
//		前方のページID
//	PhysicalFile::PageID uiNextPageID_
//		後方のページID
//	const Bitmap::BtreePage::PagePointer& pParent_
//		親ページ
//
//	RETURN
//	Bitmap::BtreePage::PagePointer
//		新しく確保したページ
//
//	EXCEPTIONS
//
BtreePage::PagePointer
BtreeFile::allocatePage(PhysicalFile::PageID uiPrevPageID_,
						PhysicalFile::PageID uiNextPageID_,
						const BtreePage::PagePointer& pParent_)
{
	PhysicalFile::Page* p = allocatePhysicalPage();
	BtreePage::PagePointer pPage = new BtreePage(*this);
	pPage->setPhysicalPage(p, uiPrevPageID_, uiNextPageID_);

	// 先頭に追加する
	m_cList.pushFront(*pPage);

	// 親を設定する
	pPage->setParentPage(pParent_);

	return pPage;
}

//
//	FUNCTION public
//	Bitmap::BtreeFile::findPage -- 親ページを検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pValue_
//		キーデータ
//	PhysicalFile::PageID uiChildPageID_
//		親を探している子ページ
//
//	RETURN
//	Bitmap::BtreePage::PagePointer
//		親ページ
//
//	EXCEPTIONS
//
BtreePage::PagePointer
BtreeFile::findPage(const ModUInt32* pValue_,
					PhysicalFile::PageID uiChildPageID_)
{
	BtreePage::PagePointer pPage;
	PhysicalFile::PageID uiPageID = getHeaderPage().getRootPageID();
	
	do
	{
		if (uiPageID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}

		pPage = attachPage(uiPageID, pPage);
		
		if (pPage->isLeaf() == true)
		{
			_SYDNEY_THROW0(Exception::BadArgument);
		}
		
		BtreePage::Iterator i = pPage->upperBound(pValue_, getCompare());
		if (i != pPage->begin())
			--i;
		uiPageID = getNodeData().getPageID(*i);
	}
	while (uiPageID != uiChildPageID_);

	return pPage;
}

//
//	FUNCTION private
//	Bitmap::BtreeFile::makeData -- データを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cKey_
//	   	キー
//	const Common::DataArrayData& cValue_
//	   	バリュー
//	const Bitmap::Data& cData_
//		データクラス
//	ModUInt32* pBuffer_
//		データ領域
//	ModSize& uiSize_
//		サイズ
//
//	RETURN
// 	なし
//
//	EXCEPTIONS
//
void
BtreeFile::makeData(const Common::Data& cKey_,
					const Common::Data& cValue_,
					const Data& cData_,
					ModUInt32* pBuffer_,
					ModSize& uiSize_)
{
	uiSize_ = cData_.dump(pBuffer_, cKey_, cValue_);
}

//
//	FUNCTION private
//	Bitmap::BtreeFile::getLeafPage -- リーフページを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pBuffer_
//		検索するためのデータ
//	const Bitmap::Compare& cCompare_
//		比較クラス
//	bool isLower_
//		下限を検索するかどうか。上限の場合はfalseを指定 (default true)
//
//	RETURN
//	Bitmap::BtreePage::PagePointer
//		リーフページ
//
//	EXCEPTIONS
//
BtreePage::PagePointer
BtreeFile::getLeafPage(const ModUInt32* pBuffer_, const Compare& cCompare_,
					   bool isLower_)
{
	BtreePage::PagePointer pLeafPage = getRootPage();
	if (pLeafPage == 0)
		return pLeafPage;

	ModSize stepCount = getHeaderPage().getStepCount();
	ModSize c = 1;
	
	while (pLeafPage->isLeaf() == false)
	{
		PhysicalFile::PageID uiPageID;
		BtreePage::Iterator i;
		c++;
		
		if (cCompare_.isUnique() == true || isLower_ == false)
		{
			// upper_boundで検索して１つ前
			i = pLeafPage->upperBound(pBuffer_, cCompare_);
			if (i != pLeafPage->begin())
				--i;
		}
		else
		{
			// lower_boundで検索して1つ前
			i = pLeafPage->lowerBound(pBuffer_, cCompare_);
			if (i != pLeafPage->begin())
				--i;
		}

		// ページIDを得る
		uiPageID = getNodeData().getPageID(*i);
		// ページを得る
		if (getFixMode() != Buffer::Page::FixMode::ReadOnly)
		{
			// 【注意】
			//		更新時もつねにページが更新されるとは限らないので、
			//		とりあえずReadOnlyでattachする
				
			pLeafPage = attachPage(uiPageID, pLeafPage,
								   Buffer::Page::FixMode::ReadOnly);
		}
		else
		{
			pLeafPage = attachPage(uiPageID, BtreePage::PagePointer());
		}
	}
	
	return pLeafPage;
}

//
//	FUNCTION private
//	Bitmap::BtreeFile::getRootPage -- ルートページを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	BtreePage::PagePointer
//		ヘッダーページ
//
//	EXCEPTIONS
//
BtreePage::PagePointer
BtreeFile::getRootPage()
{
	BtreePage::PagePointer pPage;
	
	HeaderPage& cHeaderPage = getHeaderPage();
	PhysicalFile::PageID uiPageID = cHeaderPage.getRootPageID();
	ModSize stepCount = cHeaderPage.getStepCount();
	
	if (uiPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		Buffer::Page::FixMode::Value eFixMode
			= Buffer::Page::FixMode::Unknown;
		if (stepCount != 1)
			eFixMode = Buffer::Page::FixMode::ReadOnly;

		// ルートページを得る
		pPage = attachPage(uiPageID, BtreePage::PagePointer(), eFixMode);
	}

	return pPage;
}

//
//	FUNCTION private
//	Bitmap::BtreeFile::preSearch -- 検索前準備を行う
//
//	NOTES
//	ここではデータがあるそうなリーフページの特定まで行う
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
BtreeFile::preSearch()
{
	m_pSearchPage = 0;
	m_iSearchEntryPosition = -1;
	m_bIsNull = false;
	m_bIsNull_All = false;

	// A DirPage of NormalBitmapFile is NOT freed
	// even if the count of entry is 0.
	// So BtreeFile has to return the LeafPage which stored the ID of DirPage
	// during a verification.
	if ((getHeaderPage().getCount() || doVerify() == true) &&
		m_pCondition->isValid())
	{
		Condition::LimitCond& cLower = m_pCondition->getLowerCondition();

		if (cLower.m_eType == Data::MatchType::EqualsToNull)
		{
			if (!m_pCondition->isOtherCondition())
				// is null
				m_bIsNull = true;
		}
		else if (cLower.m_eType == Data::MatchType::EqualsToNull_All)
		{
			if (!m_pCondition->isOtherCondition())
				// is null
				m_bIsNull_All = true;
		}
		else if (cLower.m_pBuffer == 0)
		{
			// 左端のページを得る
			m_pSearchPage
				= attachPage(getHeaderPage().getLeftLeafPageID(),
							 BtreePage::PagePointer());

			// 上限条件もnullなら全件検索
			Condition::LimitCond& cUpper = m_pCondition->getUpperCondition();
			if (cUpper.m_pBuffer == 0 && !m_pCondition->isOtherCondition())
			{
				// 全件検索なので、is null も処理する
				m_bIsNull = true;
			}
		}
		else
		{
			bool isLower = true;
			if (cLower.m_eType == Data::MatchType::GreaterThan)
				isLower = false;
			m_pSearchPage = getLeafPage(cLower.m_pBuffer,
										cLower.m_cCompare,
										isLower);
		}
	}
}

//
//	FUNCTION private
//	Bitmap::BtreeFile::next -- 次の検索結果を設定する
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data& cValue_
//		バリュー
//
//	RETURN
//	bool
//		結果が得られた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
BtreeFile::next(Common::Data& cValue_, Common::Data* pKey_)
{
	bool result = false;

	if (m_bIsNull)
	{
		// is null
		result = getHeaderPage().getNullID(cValue_);
		if (pKey_ && result == true)
			pKey_->setNull();
		
		m_bIsNull = false;	// 2回目以降はだめ

		if (result == true || !m_pSearchPage)
			// null のエントリがなくても、
			// 全検索の場合はここで return してはだめ
			return result;
	}
	
	if (m_bIsNull_All)
	{
		// is null
		result = getHeaderPage().getAllNullID(cValue_);
		
		m_bIsNull_All = false;	// 2回目以降はだめ
		return result;
	}
	
	if (!m_pSearchPage)
		return result;
	
	Condition::LimitCond& cLower = m_pCondition->getLowerCondition();
	if (m_iSearchEntryPosition != -1 && cLower.m_cCompare.isUnique())
	{
		// ユニーク検索の2回目
		m_pSearchPage = 0;
		return result;
	}
	
	BtreePage::Iterator i;
		
	if (m_iSearchEntryPosition == -1)
	{
		// はじめてのnext()
		if (cLower.m_pBuffer)
		{
			if (cLower.m_eType == Data::MatchType::GreaterThan)
			{
				i = m_pSearchPage->upperBound(cLower.m_pBuffer,
											  cLower.m_cCompare);
			}
			else
			{
				i = m_pSearchPage->lowerBound(cLower.m_pBuffer,
											  cLower.m_cCompare);
			}
		}
		else
		{
			i = m_pSearchPage->begin();
		}
	}
	else
	{
		// 直前の位置
		i = m_pSearchPage->begin() + m_iSearchEntryPosition;
		// の次
		++i;
	}
		
	while (true)
	{
		if (i == m_pSearchPage->end())
		{
			// このページは終わりなので次へ
				
			PhysicalFile::PageID nextID = m_pSearchPage->getNextPageID();
			if (nextID == PhysicalFile::ConstValue::UndefinedPageID)
			{
				// もうページがない
				m_pSearchPage = 0;
				break;
			}

			m_pSearchPage = attachPage(nextID,
									   BtreePage::PagePointer());
			i = m_pSearchPage->begin();
		}

		Condition::LimitCond& cUpper = m_pCondition->getUpperCondition();
		if (cUpper.m_pBuffer && cUpper.m_cCompare(*i, cUpper.m_pBuffer) > 0)
		{
			// もう終わり
			m_pSearchPage = 0;
			break;
		}

		if (m_pCondition->isOtherConditionMatch(*i) == true)
		{
			// エントリをコピーする
			if (pKey_)
				getLeafData().getKeyAndValue(*i, *pKey_, cValue_);
			else
				getLeafData().getValue(*i, cValue_);
			result = true;
			// 位置
			m_iSearchEntryPosition
				= static_cast<int>(i - m_pSearchPage->begin());
			break;
		}

		// 条件にマッチしないので次へ
		++i;
	}

	return result;
}

//
//	FUNCTION private
//	Bitmap::BtreeFile::createData -- データクラスを作成する
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
	{
		ModVector<Data::Type::Value> type;
		type.pushBack(m_cFileID.getKeyType());
		
		ModSize size = m_cFileID.getKeySize() / sizeof(ModUInt32);
	
		m_cKeyData.setType(type, (m_cFileID.isFixed() ? size  : 0));
	}
	{
		ModVector<Data::Type::Value> type;
		type.reserve(2);
		type.pushBack(m_cFileID.getKeyType());
		type.pushBack(Data::Type::UnsignedInteger);	// PageID分
		
		ModSize size = m_cFileID.getKeySize() / sizeof(ModUInt32);
		size += Data::UnsignedInteger::getSize(0);
	
		m_cNodeData.setType(type, (m_cFileID.isFixed() ? size  : 0));
	}
	if (m_cFileID.isCompressed())
	{
		ModVector<Data::Type::Value> type;
		type.reserve(2);
		type.pushBack(m_cFileID.getKeyType());
		type.pushBack(Data::Type::ObjectID);	// AreaID分
		
		ModSize size = m_cFileID.getKeySize() / sizeof(ModUInt32);
		size += Data::ObjectID::getSize(0);
	
		m_cLeafData.setType(type, (m_cFileID.isFixed() ? size  : 0));
	}
	else
	{
		ModVector<Data::Type::Value> type;
		type.reserve(2);
		type.pushBack(m_cFileID.getKeyType());
		type.pushBack(Data::Type::UnsignedInteger);	// PageID分
		
		ModSize size = m_cFileID.getKeySize() / sizeof(ModUInt32);
		size += Data::UnsignedInteger::getSize(0);
	
		m_cLeafData.setType(type, (m_cFileID.isFixed() ? size  : 0));
	}
}

//
//	FUNCTION private
//	Bitmap::BtreeFile::createCompare -- 比較クラスを作成する
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
	m_cCompare.setType(m_cFileID.getKeyType(), true);
}

//
//	FUNCTION public
//	Bitmap::BtreeFile::detachAll -- すべてのページをdetachする
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
BtreeFile::detachAll()
{
	m_pSearchPage = 0;
	
	PageList::Iterator i = m_cList.begin();
	while (i != m_cList.end())
	{
		// deleteしなくてはいけないので、ポインタをとっておく
		BtreePage* p = &(*i);
		++i;

		// 物理ページをdetachする
		p->detachPhysicalPage();

		// eraseしてからdeleteする
		m_cList.erase(*p);
		delete p;
	}
}

//
//	Copyright (c) 2005, 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

