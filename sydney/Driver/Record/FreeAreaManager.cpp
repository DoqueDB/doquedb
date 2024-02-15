// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FreeAreaManager.cpp -- 
// 
// Copyright (c) 2000, 2007, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Record";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Record/FreeAreaManager.h"
#include "Record/Module.h"
#include "Record/Parameter.h"
#include "Record/PhysicalPosition.h"
#include "Record/UseInfo.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"
#include "PhysicalFile/File.h"
#include "PhysicalFile/Page.h"

#include "Admin/Verification.h"

_SYDNEY_USING
_SYDNEY_RECORD_USING

namespace 
{
	// max cached page size for Batch mode
	ParameterInteger _cParameterBatchCacheSize("Record_BatchMaxPageCache", 10);

	// DirectFileのヘッダーページの置き換え優先度
	const Buffer::ReplacementPriority::Value
		_ePriority = Buffer::ReplacementPriority::Low;

	// FixModeからUnFixModeを得る
	PhysicalFile::Page::UnfixMode::Value
	_getUnfixMode(Buffer::Page::FixMode::Value eFixMode_)
	{
		if (eFixMode_ & Buffer::Page::FixMode::Write) {
			return PhysicalFile::Page::UnfixMode::Dirty;
		} else {
			return PhysicalFile::Page::UnfixMode::NotDirty;
		}
	}

	// OperationからFixModeを得るための表
	Buffer::Page::FixMode::Value _FixModeTable[FreeAreaManager::Operation::ValueNum] =
	{
		// Read
		Buffer::Page::FixMode::ReadOnly,
		// Write
		Buffer::Page::FixMode::Write | Buffer::Page::FixMode::Discardable,
		// Expunge
		Buffer::Page::FixMode::Write | Buffer::Page::FixMode::Discardable,
		// Batch
		Buffer::Page::FixMode::Write
	};
	// OperationからUnFixModeを得るための表
	PhysicalFile::Page::UnfixMode::Value _UnfixModeTable[][FreeAreaManager::Operation::ValueNum] =
	{
		// NotDirty
		{
			// Read
			PhysicalFile::Page::UnfixMode::NotDirty,	
			// Write
			PhysicalFile::Page::UnfixMode::NotDirty,
			// Expunge
			PhysicalFile::Page::UnfixMode::NotDirty,
			// Batch -- unfix as dirty even in error handling
			PhysicalFile::Page::UnfixMode::Dirty
		},
		// Dirty
		{
			// Read
			PhysicalFile::Page::UnfixMode::NotDirty,	
			// Write
			PhysicalFile::Page::UnfixMode::Dirty,
			// Expunge
			PhysicalFile::Page::UnfixMode::Dirty,
			// Batch
			PhysicalFile::Page::UnfixMode::Dirty
		}
	};
}

//
//	FUNCTION public
//	Record::FreeAreaManager::FreeAreaManager -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	const Trans::Transaction&	pTransaction_
//		トランザクション記述子
//	const PhysicalFile::File&	pFile_
//		領域の取得/解放を行なうための物理ファイル
//	const PhysicalFile::PageID	ulStartPageID_
//		空き領域管理するページのなかでも先頭ページのID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
FreeAreaManager::FreeAreaManager(const Trans::Transaction&	Transaction_,
								 PhysicalFile::File&		File_,
								 const PhysicalFile::PageID	StartPageID_,
								 const bool					DoCachePage_)
	: m_Transaction(Transaction_),
	  m_File(File_),
	  m_StartPageID(StartPageID_),
	  m_CachedPage(0),
	  m_mapAttachedPage(),
	  m_vecFreePageID(),
	  m_eCacheOperation(Operation::Read)
{
	// ページIDが Undefined ではない事を確認
	; _SYDNEY_ASSERT(
		StartPageID_ != PhysicalFile::ConstValue::UndefinedPageID);

	m_PageDataSize = File_.getPageDataSize();

	// “PhysicalFile::File::searchFreePage()で空いているページを
	// 　高速検索可能なサイズ”を、
	// 物理ファイルマネージャに問い合わせる。
	m_iSearchPageThreshold = File_.getPageSearchableThreshold();
}

//
//	FUNCTION public
//	Record::FreeAreaManager::~FreeAreaManager -- デストラクタ
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
FreeAreaManager::~FreeAreaManager()
{
	clearCachePage(true /* force */);
}

//
// アクセッサ
//

//
//	FUNCTION public
//	Record::FreeAreaManager::freeUsedArea --  使用していたエリアを解放する
//
//	NOTES
//	エリアを解放して再利用可能にする。
//
//	ARGUMENTS
//	const PhysicalFile::AreaID	uiAreaID_
//		解放するエリアのID
//	PhysicalFile::Page*	pPage_
//		解放するエリアが属するページ記述子
//	Tools::ObjectID& iFreeID_
//		開放するエリアに書いておく次の空き領域ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
FreeAreaManager::
freeUsedArea(const PhysicalFile::AreaID		AreaID_,
			 PhysicalFile::Page*			Page_,
			 Tools::ObjectID&				iFreeID_) const
{
	// 渡されたページ記述子の親と、本クラスが所有しているファイル記述子は
	// 同じであるはず。
	; _SYDNEY_ASSERT(Page_->getFile() == &m_File);

	if (Page_->getAreaSize(AreaID_) >= getSearchPageThreshold()) {
		// エリアのサイズがSearchPageでは探せないサイズであるので
		// 空き領域リストで管理する（∴ PhysicalPage::freeArea() しない）
		// 次の空き領域IDを書き込み、空き領域IDに自分のオブジェクトIDを入れる
		// つまり、自身の領域の先頭をObjectIDへのポインタに見立て、
		// いままでiFreeID_が指していたObjectIDをコピーし、
		// iFreeID_の指し示す先を自分のObjectIDに変更する。
		// iFreeID_の初期値はUndefinedなので、リストの終端もUndefinedになる。
		char* pPointer = static_cast<char*>(Tools::getAreaTop(Page_, AreaID_));
		pPointer += Tools::writeObjectID(pPointer, iFreeID_);
		iFreeID_ = PhysicalPosition::getObjectID(Page_->getID(), AreaID_);
		// ※このリンクは永続化されるので、記録位置を変更した場合、
		// 　既存のデータベースが使用不可になってしまうことに注意
	}
	else
	{

		//
		// エリアのサイズが、PhysicalFile::File::searchFreePage()によって
		// 探せないサイズである場合には、物理エリアは解放しない。
		// これらの物理エリアは、
		// ファイル管理情報の“空きオブジェクトIDリストの先頭”からの
		// リンクで辿れるので、解放しなくても、その後のinsert処理で
		// 再利用されるはずである。
		//

		Page_->freeArea(m_Transaction, AreaID_);

		// Discardableによるエラー処理を有効にするには
		// 処理の最後までdetachPageを行ってはいけない
		// ここでfreePageするのではなく
		// freePageが必要なページIDを覚えておくしかないだろう

		if (Page_->getTopAreaID(m_Transaction) == PhysicalFile::ConstValue::UndefinedAreaID) {
			m_vecFreePageID.pushBack(Page_->getID());
		}

/*
		// ページに含まれる全てのAreaが、SearchPageで探せるサイズであった場合は、
		// freeArea() 後に、ページ内に Areaが全く存在しないことが起こる。
		// その場合、この物理ページはどれからも参照されないことになる。
		// この時点では、Page_ がアタッチしている状態なので、単純にfreePage()はできない。
		// ページのデタッチは、Fileのメソッドの最後の detachPageAll() で行うが、
		// その時点まで、このページIDを保持するのは変更量が多いので、
		// 呼び出し系列を確認したら、以後ページを操作することがない様なので
		// ここでデタッチする。
		if (Page_->getTopAreaID(m_Transaction) == PhysicalFile::ConstValue::UndefinedAreaID) {
			const PhysicalFile::PageID iPageID = Page_->getID();
			m_File.detachPage(Page_, PhysicalFile::Page::UnfixMode::Dirty, false);
			_SYDNEY_ASSERT(Page_ == 0);
			// ページの内容が、PhysicalFile::ConstValue::UndefinedAreaID なのだから、
			// そのページが他の場所から参照されていてはいけない。
			// 参照カウンタが０の場合は、detachPage() 後の Page_ は 0 になっている。
			m_File.freePage(m_Transaction, iPageID);
			// 注意：この領域は永続化されているので、
			// 　　　アクセスは物理フォーマットに注意すること
		}
*/
	}

	// エリアを解放する
	// ★注意★
	// freeAreaではエリアの中身は変更されない

	// 上のコメントに書いてあるとおり、ここでのfreeAreaはコメントアウト。
	//Page_->freeArea(m_Transaction, AreaID_);
}

//	FUNCTION public
//	Record::FreeAreaManager::setUseInfo -- 
//
//	NOTES
//		使用中の物理ページIDとエリアIDを集める。（FreeArea 用）
//		仕様は LinkedObject::setUseInfo() と同様。
//		但し Area のリンクを構成するフォーマットが異なるので、
//		FreeAreaには LinkedObject::setUseInfo() は使用できない。
//
//	ARGUMENTS
//		Tools::ObjectID iObjectID_
//			登録を開始するオブジェクトID
//		Record::UseInfo& cUseInfo_
//			登録情報
//
//	REUTRN
//		なし
//
//	EXCEPTIONS

void
FreeAreaManager::
setUseInfo(Tools::ObjectID iObjectID_, UseInfo& cUseInfo_ ,Admin::Verification::Progress& cProgress_)
{
	PhysicalFile::File& cFile = getFile();
	Tools::ObjectID		iObjectID = iObjectID_;
	PhysicalFile::Page* pPage = 0;

	try {
		do {
			// 読み込みを行なうエリアをアタッチ
			PhysicalPosition pos(iObjectID);
			pPage = verifyPage(pos.m_PageID, Operation::Read ,cProgress_);
			if (!cProgress_.isGood()) {
				if (pPage) {
					detachPage(pPage, Operation::Read, false);
				}
				return;
			}
			; _SYDNEY_ASSERT(pPage != 0);

			const char*	areaTop =
				static_cast<const char*>(
					Tools::getConstAreaTop(pPage, pos.m_AreaID));
			; _SYDNEY_ASSERT(areaTop != 0);

			const char* areaPointer = areaTop;
			// 注意：この領域は永続化されているので、
			// 　　　アクセスは物理フォーマットに注意すること

			// 次のオブジェクトのObjectIDを得る
			areaPointer += Tools::readObjectID(areaPointer, iObjectID);

			// 読み終ったオブジェクトIDを登録する
			cUseInfo_.append(pos.m_PageID, pos.m_AreaID);

			detachPage(pPage, Operation::Read, false);
			; _SYDNEY_ASSERT(!pPage);

		} while (iObjectID != Tools::m_UndefinedObjectID);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		if (pPage) {
			detachPage(pPage, Operation::Read, false);
		}
		_SYDNEY_RETHROW;	// 例外処理が終ったので再送
	}
}


//	FUNCTION public
//	Record::FreeAreaManager::attachPage -- ページをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	アタッチしたページ
//
//	EXCEPTIONS

PhysicalFile::Page*
FreeAreaManager::
attachPage(PhysicalFile::PageID iPageID_,
		   Operation::Value eOperation_) const
{
	PhysicalFile::Page* page = 0;
	
	if (eOperation_ == Operation::Read)
	{
		// ReadOnlyの時はページをマップに格納する必要はない
		// 1ページキャッシュしていれば十分である
		
		if (m_CachedPage && m_CachedPage->getID() == iPageID_)
			return m_CachedPage;
		
		if (m_CachedPage)
		{
			detachPage(m_CachedPage, eOperation_, true);
		}
		m_CachedPage = m_File.attachPage(m_Transaction,
										 iPageID_, _FixModeTable[eOperation_], _ePriority);

		page = m_CachedPage;
	}
	else
	{
		// 以前attachPageした中にあるか調べる
		if (m_mapAttachedPage.getSize()) {
			AttachedPageMap::Iterator iterator
				= m_mapAttachedPage.find(iPageID_);
			if (iterator != m_mapAttachedPage.end())
				page = (*iterator).second;
		}
		if (!page) {
			// 本当にアタッチする
			page = m_File.attachPage(m_Transaction,
									 iPageID_, _FixModeTable[eOperation_], _ePriority);
			m_mapAttachedPage.insert(iPageID_, page);
		}
		m_eCacheOperation = eOperation_;
	}

	return page;
}

//	FUNCTION public
//	Record::FreeAreaManager::verifyPage -- ページをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	アタッチしたページ
//
//	EXCEPTIONS

PhysicalFile::Page*
FreeAreaManager::
verifyPage(PhysicalFile::PageID iPageID_,
		   Operation::Value eOperation_,
		   Admin::Verification::Progress& cProgress_) const
{
	return m_File.verifyPage(m_Transaction, iPageID_, _FixModeTable[eOperation_], cProgress_);
}

// FUNCTION public
//	Record::FreeAreaManager::detachPage -- ページをデタッチする
//
// NOTES
//
// ARGUMENTS
//	PhysicalFile::Page*& pPage_
//	Operation::Value eOperation_
//	bool bDirty
//	bool bForce = false
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FreeAreaManager::
detachPage(PhysicalFile::Page*& pPage_,
		   Operation::Value eOperation_,
		   bool bDirty,
		   bool bForce) const
{

	if (pPage_ == 0) return;

	PhysicalFile::Page::UnfixMode::Value eUnfixMode = _UnfixModeTable[bDirty][eOperation_];
	if (eUnfixMode == PhysicalFile::Page::UnfixMode::Dirty) {
		pPage_->dirty();
	}

	if (m_CachedPage == pPage_)
	{
		m_File.detachPage(pPage_, eUnfixMode);
		m_CachedPage = 0;
		return;
	}

	if (bForce) {
		// If force flag is true, detach anyway.
		// [NOTES]
		// If the page instance was come from m_mapAttechedPage,
		// the entry should be erased by caller
		m_File.detachPage(pPage_, eUnfixMode);

	} else {

		// マップを調べる
		AttachedPageMap::Iterator i = m_mapAttachedPage.find(pPage_->getID());
		if (i == m_mapAttachedPage.end())
		{
			// マップにないのでdetachする
			m_File.detachPage(pPage_, eUnfixMode);
		}
	}
	
	pPage_ = 0;
}

//
//	FUNCTION public
//	Record::FreeAreaManager::getPageDataSize -- 確保可能な領域のサイズ(最大値)を返す
//
//	NOTES
//	確保可能な領域のサイズ(最大値)を返す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	確保可能な領域のサイズ(最大値)
//
//	EXCEPTIONS
//
Os::Memory::Size
FreeAreaManager::getPageDataSize() const
{
	; _SYDNEY_ASSERT(m_PageDataSize > 0);

	return m_PageDataSize;
}

//	FUNCTION public
//	Record::FreeAreaManager::getSearchPageThreshold --
//		SearchFreePageで探すことができるサイズの閾値を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	SearchFreePageで探すことができるサイズの閾値
//
//	EXCEPTIONS

Os::Memory::Size
FreeAreaManager::
getSearchPageThreshold() const
{
	return m_iSearchPageThreshold;
}

#ifdef OBSOLETE
//
//	FUNCTION public
//	Record::FreeAreaManager::getTransaction -- トランザクション記述子を返す
//
//	NOTES
//	トランザクション記述子を返す
//
//	ARGUMENTS
//	なし
//
//	RETURN
// 	トランザクション記述子
//
//	EXCEPTIONS
//
const Trans::Transaction&
FreeAreaManager::getTransaction() const
{
	return m_Transaction;
}
#endif

//
//	FUNCTION public
//	Record::FreeAreaManager::getFile -- 物理ファイル記述子を返す
//
//	NOTES
//	領域の取得/解放の対象になっている物理ファイル記述子を返す
//
//	ARGUMENTS
//	なし
//
//	RETURN
// 	領域の取得/解放の対象になっている物理ファイル記述子
//
//	EXCEPTIONS
//
PhysicalFile::File&
FreeAreaManager::getFile() const
{
	return m_File;
}

//
// マニピュレータ
//

//
//	FUNCTION public
//	Record::FreeAreaManager::getFreeArea --  空き領域を返す
//
//	NOTES
//	ファイルから空き領域を探して返す。空き領域がみつからない場合は
//	新規にアロケートした物理ページから空き領域を返す。
//	空き領域を探す順番は次の通り。
//
//	１、キャッシュされているページのうち、いちばん最後に(最近に)使用
//		されたページから探す
//	２、他にもキャッシュされたページが存在するならば、そのページからも探す。
//	３、新規にアロケートしたページから探す。
//
//	新規ページから取得できる空き領域より大きなサイズが要求された場合は
//	例外を返す。
//
//	(!! 注意 !!)
//	本関数で取得したエリア記述子は、本クラスの detachArea で解放すること。
//
//	ARGUMENTS
//	const ModSize			ulSize_
//		探している空き領域のサイズ
//	PhysicalFile::PageID&	uiFoundPageID_		// OUT
//		見つかった空き領域が存在するページのページID
//	PhysicalFile::AreaID&	uiFoundAreaID_		// OUT
//		見つかった空き領域のエリアID
//	FreeAreaManager::Operation::Value eOperation_
//		操作内容を表すENUM値
//
//	RETURN
//	なし
//		空き領域のページＩＤ、エリアＩＤを返す	
//
//	EXCEPTIONS
//	
//
void
FreeAreaManager::getFreeArea(const Os::Memory::Size	Size_,
							 PhysicalFile::PageID&	FoundPageID_,
							 PhysicalFile::AreaID&	FoundAreaID_,
							 Operation::Value eOperation_)
{
	if (Size_ > m_File.getPageDataSize(1))
	{
		// 新規ページから取得できる空き領域より大きなサイズが要求された

		_SYDNEY_THROW0(Exception::BadArgument);
	}

	PhysicalFile::Page*	parentPage = 0;

	// キャッシュされているページから空き領域を探してみる
	if (getFreeAreaFromCachedPage(false,
								  Size_,
								  FoundPageID_,
								  FoundAreaID_))
	{
		return;
	}

	// まだ空き領域が見つかっていないならば新規ページから確保する
/*
	// キャッシュ中のページをデタッチする
	clearCachePage();
*/

	// 空き領域管理のメカニズムを利用してみる
	//	1. 未使用領域（空き領域＋開放領域）で探す
	//	2. allocateArea してみる
	//	3. allocateArea できない場合は compaction する
	PhysicalFile::PageID	pageID =
		m_File.searchFreePage(m_Transaction,
							  Size_,
							  m_StartPageID,
							  true,
							  1);

	PhysicalFile::Page* pPage = 0;

	if (pageID == PhysicalFile::ConstValue::UndefinedPageID)
	{
		// まだ空き領域が見つからないならば、最後の手段である
		// "新規ページのアロケート" を行なう
		pPage = m_File.allocatePage2(m_Transaction, _FixModeTable[eOperation_]);
	}
	else {
		// freePageの候補に入っていたら除く
		ModVector<PhysicalFile::PageID>::Iterator iterator = m_vecFreePageID.find(pageID);
		if (iterator != m_vecFreePageID.end())
			m_vecFreePageID.erase(iterator);
	}

	// 新しいページをキャッシュページにしてから空き領域を探し直す
	// (物理ファイルの searchFreePage が見つけたものはコンパクション
	//  が必要かも知れない。
	if (pPage == 0)
	{
		pPage =	attachPage(pageID, eOperation_);
	}

	bool found = false;

	try
	{
		found = getFreeAreaFromPage(pPage,
									true,
									Size_,
									FoundPageID_,
									FoundAreaID_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		detachPage(pPage, eOperation_, false);
		_SYDNEY_RETHROW;
	}

	// 以下のメソッドではDoCachePageの場合にデタッチされない
	detachPage(pPage, eOperation_, true);

	// 新しいキャッシュページは絶対に空き領域を返すことができる
	// (最悪の場合でもコンパクションによって確保できているはず)
	; _SYDNEY_ASSERT(found);

	return;
}

//	FUNCTION public
//	Record::FreeAreaManager::getFreeArea --  空き領域を返す
//
//	NOTES
//	ファイルから空き領域を探して返す
//	空き領域リストがあればその先頭を返す。
//	空き領域を探す順番は次の通り。
//
//	１、空き領域リストの先頭IDがあればそれを返す
//	２、空き領域リストがUndefinedであれば新規にアロケートしたページから探す
//
//	新規ページから取得できる空き領域より大きなサイズが要求された場合は
//	例外を返す。
//
//	ARGUMENTS
//	const ModSize			ulSize_
//		探している空き領域のサイズ
//	PhysicalFile::PageID&	uiFoundPageID_		// OUT
//		見つかった空き領域が存在するページのページID
//	PhysicalFile::AreaID&	uiFoundAreaID_		// OUT
//		見つかった空き領域のエリアID
//	Tools::ObjectID&		iFreeID_			// IN/OUT
//		空き領域リストの先頭
//	Os::Memory::Size&		iAllocated_			// OUT
//		実際に割り当てられたサイズ
//	FreeAreaManager::Operation::Value eOperation_
//		操作内容を表すENUM値
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
FreeAreaManager::
getFreeArea(const Os::Memory::Size	Size_,
			PhysicalFile::PageID&	FoundPageID_,
			PhysicalFile::AreaID&	FoundAreaID_,
			Tools::ObjectID&		iFreeID_,
			Os::Memory::Size&		iAllocated_,
			Operation::Value		eOperation_)
{
	if (Size_ > m_File.getPageDataSize(1)) {
		// 新規ページから取得できる空き領域より大きなサイズが要求された
		_SYDNEY_THROW0(Exception::BadArgument);

	} else if (Size_ < getSearchPageThreshold()) {
		// SearchPageで探すことのできるサイズなので通常のgetFreeAreaを呼ぶ
		getFreeArea(Size_, FoundPageID_, FoundAreaID_, eOperation_);
		return;
	}

	PhysicalFile::PageID	pageID;
	PhysicalFile::AreaID	areaID;
	if (iFreeID_ == Tools::m_UndefinedObjectID) {

		// 空き領域リストがないので新たにページをアロケートする

		PhysicalFile::Page* pPage =
			m_File.allocatePage2(m_Transaction, _FixModeTable[eOperation_]);
		bool found = false;

		found = getFreeAreaFromPage(pPage,
									true,
									Size_,
									FoundPageID_,
									FoundAreaID_);

		iAllocated_ = Size_;
		
		detachPage(pPage, eOperation_, true);

	} else {
		; _SYDNEY_ASSERT(iFreeID_ < Tools::m_UndefinedObjectID);

		// 空き領域リストの先頭を得る
		PhysicalPosition pos(iFreeID_);
		pageID = pos.m_PageID;
		areaID = pos.m_AreaID;

		PhysicalFile::Page* pPage =
			attachPage(pageID, eOperation_);

		// ファイル管理情報の“空きオブジェクトIDリストの先頭”から
		// 辿れる物理エリアは解放しないようになった。
		// なので、ここではPhysicalFile::Page::reuseArea()を
		// する必要がなくなった。
		//
		//pPage->reuseArea(m_Transaction, areaID);

		; _SYDNEY_ASSERT(pPage->getAreaSize(areaID) >= getSearchPageThreshold());

		// compactionされていないはずなのでエリアの先頭に次の空き領域が書かれている
		const char* pPointer = static_cast<const char*>(Tools::getConstAreaTop(pPage, areaID));

		pPointer += Tools::readObjectID(pPointer, iFreeID_);
		//; _SYDNEY_ASSERT(iFreeID_ < Tools::m_UndefinedObjectID);

		iAllocated_ = pPage->getAreaSize(areaID);

		FoundPageID_ = pageID;
		FoundAreaID_ = areaID;

		detachPage(pPage, eOperation_, true);
	}
}

//
//	FUNCTION public
//	Record::FreeAreaManager::clearCachePage --  キャッシュページをクリアする
//
//	NOTES
//
//	ARGUMENTS
//		bool bForce_ = false
//			trueのとき条件によらずすべてのキャッシュページをデタッチする
//			falseのとき条件によって一部のページをデタッチせずに残す
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FreeAreaManager::
clearCachePage(bool bForce_/* = false */)
{
	ModSize iCacheLimit =
		(bForce_ || (m_eCacheOperation != Operation::Batch))
		? 0
		: _cParameterBatchCacheSize.get();

	if (m_mapAttachedPage.getSize() > iCacheLimit) {
		if (!bForce_ && m_eCacheOperation == Operation::Batch) {
			// detach pages here
			AttachedPageMap::Iterator iterator = m_mapAttachedPage.begin();
			const AttachedPageMap::Iterator last = m_mapAttachedPage.end();
			for (; iterator != last; ++iterator) {
				detachPage((*iterator).second, m_eCacheOperation, true, true);
			}
		}
		m_mapAttachedPage.erase(m_mapAttachedPage.begin(),
								m_mapAttachedPage.end());
	}
	if (m_CachedPage)
	{
		detachPage(m_CachedPage, m_eCacheOperation, true);
		m_CachedPage = 0;
	}
}

/*
//
//	FUNCTION public
//	Record::FreeAreaManager::setDoCache -- キャッシュするかどうかのフラグを切り替える
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FreeAreaManager::
setDoCache(bool bDoCache_)
{
	m_DoCachePage = bDoCache_;
	if (!bDoCache_) {
		clearCachePage();
	}
}
*/

//
//	FUNCTION public
//	Record::FreeAreaManager::freePageAll -- 正常終了時にfreePageすべきページを解放する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FreeAreaManager::
freePageAll()
{
	if (m_vecFreePageID.getSize() == 0) return;
	
	try
	{
		ModVector<PhysicalFile::PageID>::Iterator i = m_vecFreePageID.begin();
		for (; i != m_vecFreePageID.end(); ++i)
		{
			m_File.freePage(m_Transaction, *i);
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// ページの開放に失敗した
		// もう元に戻せないので、データベースは利用不可する
		// (上位関数がやっている)
		
		m_vecFreePageID.clear();
		m_File.unfixVersionPage(true);
		_SYDNEY_RETHROW;
	}
	m_vecFreePageID.clear();
	m_File.unfixVersionPage(true);
}

// FUNCTION public
//	Record::FreeAreaManager::discardFreePage -- 異常終了時にfreePageすべきページの記録を破棄する
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
FreeAreaManager::
discardFreePage()
{
	m_vecFreePageID.clear();
}

//
// PRIVATE
//


//
//	FUNCTION private
//	Record::FreeAreaManager::getFreeAreaFromCachedPage --  空き領域を返す
//
//	NOTES
//	ページ記述子から空き領域を探す。
//	見つからない場合は 0 を返す。
//
//	ARGUMENTS
//	const bool			bCompaction_
//		true ならばコンパクションを行う
//	const ModSize		ulSize_
//		探している空き領域のサイズ
//	PhysicalFile::PageID&	uiFoundPageID_		// OUT
//		見つかった空き領域が存在するページのページID
//	PhysicalFile::AreaID&	uiFoundAreaID_		// OUT
//		見つかった空き領域のエリアID
//
//	RETURN
//	bool
//		見つかったかどうか
//
//	EXCEPTIONS
//	
//
bool
FreeAreaManager::getFreeAreaFromCachedPage(
	const bool				Compaction_,
	const Os::Memory::Size	Size_,
	PhysicalFile::PageID&	FoundPageID_,
	PhysicalFile::AreaID&	FoundAreaID_)
{
	if (m_mapAttachedPage.getSize()) {
		// search from tail
		// max is half of all the caches
		int n = _cParameterBatchCacheSize.get() / 2;
		AttachedPageMap::Iterator iterator = m_mapAttachedPage.end();
		const AttachedPageMap::Iterator& begin = m_mapAttachedPage.begin();
		do {
			--iterator;
			if (getFreeAreaFromPage((*iterator).second, Compaction_, Size_, FoundPageID_, FoundAreaID_))
				return true;
		} while (iterator != begin && --n > 0);
	}
	return false;
}

//
//	FUNCTION private
//	Record::FreeAreaManager::getFreeAreaFromPage --  空き領域を返す
//
//	NOTES
//	ページ記述子から空き領域を探す。
//	見つからない場合は 0 を返す。
//
//	ARGUMENTS
//	PhysicalFile::Page* pPage_
//		空き領域を探すページ
//	const bool			bCompaction_
//		true ならばコンパクションを行う
//	const ModSize		ulSize_
//		探している空き領域のサイズ
//	PhysicalFile::PageID&	uiFoundPageID_		// OUT
//		見つかった空き領域が存在するページのページID
//	PhysicalFile::AreaID&	uiFoundAreaID_		// OUT
//		見つかった空き領域のエリアID
//
//	RETURN
//	bool
//		見つかったかどうか
//
//	EXCEPTIONS
//	
//
bool
FreeAreaManager::getFreeAreaFromPage(
	PhysicalFile::Page*		pPage_,
	const bool				Compaction_,
	const Os::Memory::Size	Size_,
	PhysicalFile::PageID&	FoundPageID_,
	PhysicalFile::AreaID&	FoundAreaID_)
{
	// 記述子がちゃんと存在する事を期待してる
	; _SYDNEY_ASSERT(pPage_ != 0);

	FoundAreaID_ = pPage_->allocateArea(m_Transaction, Size_, Compaction_);
	if (FoundAreaID_ == PhysicalFile::ConstValue::UndefinedAreaID)
	{
		return false;
	}

	// 確保できた
	FoundPageID_ = pPage_->getID();

	return true;
}

//
//	Copyright (c) 2000, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
