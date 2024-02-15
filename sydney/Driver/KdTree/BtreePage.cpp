// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreePage.cpp --
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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
#include "KdTree/BtreePage.h"

#include "KdTree/Algorithm.h"
#include "KdTree/BtreeFile.h"
#include "KdTree/PagePointer.h"
#include "KdTree/MessageAll_Class.h"

#include "Common/Assert.h"

#include "Os/Memory.h"

#include "Exception/UniquenessViolation.h"
#include "Exception/BadArgument.h"
#include "Exception/VerifyAborted.h"
#include "Exception/Cancel.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
	//
	//	VARIABLE local
	//
	ModUInt32 _LEAF_MASK =	0x80000000;
}

//
//	FUNCTION public
//	KdTree::BtreePage::BtreePage -- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::BtreeFile& cFile_
//		ファイル
//	PhysicalFile::Page* pPage_
//		物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
BtreePage::BtreePage(BtreeFile& cFile_, PhysicalFile::Page* pPage_)
	: Page(cFile_, pPage_),
	  m_pHeader(0), m_pEntry(0), m_pNext(0)
{
	load();
}

//
//	FUNCTION public
//	KdTree::BtreePage::BtreePage -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::BtreeFile& cFile_
//		ファイル
//	PhysicalFile::Page* pPage_
//		物理ページ
//	PhysicalFile::PageID uiPrevPageID_
//		前方ページのページID
//	PhysicalFile::PageID uiNextPageID_
//		後方ページのページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
BtreePage::BtreePage(BtreeFile& cFile_, PhysicalFile::Page* pPage_,
					 PhysicalFile::PageID uiPrevPageID_,
					 PhysicalFile::PageID uiNextPageID_)
	: Page(cFile_, pPage_),
	  m_pHeader(0), m_pEntry(0), m_pNext(0)
{
	load();
	setPrevPageID(uiPrevPageID_);
	setNextPageID(uiNextPageID_);
}

//
//	FUNCTION public
//	KdTree::BtreePage::~BtreePage -- デストラクタ
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
BtreePage::~BtreePage()
{
}

//
//	FUNCTION public
//	KdTree::BtreePage::lowerBound -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::BtreePage::Entry* pEntry_
//		比較対象データ
//
//	RETURN
//	KdTree::BtreePage::Iterator
//		検索結果位置へのイテレータ
//
//	EXCEPTIONS
//
BtreePage::Iterator
BtreePage::lowerBound(const Entry* pEntry_)
{
	return Algorithm::lowerBound(begin(), end(), *pEntry_, Compare());
}

//
//	FUNCTION public
//	KdTree::BtreePage::upperBound -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::BtreePage::Entry* pEntry_
//		比較対象データ
//
//	RETURN
//	KdTree::BtreePage::Iterator
//		検索結果位置へのイテレータ
//
//	EXCEPTIONS
//
BtreePage::Iterator
BtreePage::upperBound(const Entry* pEntry_)
{
	return Algorithm::upperBound(begin(), end(), *pEntry_, Compare());
}

//
//	FUNCTION public
//	KdTree::BtreePage::find -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::BtreePage::Entry* pEntry_
//		比較対象データ
//
//	RETURN
//	KdTree::BtreePage::Iterator
//		検索結果位置へのイテレータ、見つからなかった場合はend()を返す
//
//	EXCEPTIONS
//
BtreePage::Iterator
BtreePage::find(const Entry* pEntry_)
{
	Compare compare;
	Iterator e = end();
	Iterator i = Algorithm::lowerBound(begin(), e, *pEntry_, compare);
	if (i == e || compare(*i, *pEntry_) != 0)
		return e;
	return i;
}

//
//	FUNCTION public
//	KdTree::BtreePage::insertEntry -- エントリを挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::BtreePage::Entry* pEntry_
//		挿入するエントリ
//
//	RETURN
//	bool
//		ページ分割、再配布が発生した場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
BtreePage::insertEntry(const Entry* pEntry_)
{
	bool result = false;

	PagePointer pPage = this;
	
	if (getFreeCount() < 1)
	{
		// 空き領域がない -> ページ分割か再配布が必要

		pPage = expand(pEntry_);

		result = true;
	}

	Iterator i = pPage->upperBound(pEntry_);
	if (i != pPage->begin())
	{
		Compare compare;
		if (compare(*(i-1), *pEntry_) == 0)
		{
			_SYDNEY_THROW0(Exception::UniquenessViolation);
		}
	}
	pPage->insert(i, pEntry_);

	return result;
}

//
//	FUNCTION public
//	KdTree::BtreePage::expungeEntry -- エントリを削除する
//
//	NOTES
//
//	ARGUMENTS
//	const KdTree::BtreePage::Entry* pEntry__
//		削除するエントリ
//	bool isReduce_
//		段数変更を行うかどうか (default true)
//
//	RETURN
//	bool
//		ページ連結、再配布が発生した場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
BtreePage::expungeEntry(const Entry* pEntry_, bool isReduce_)
{
	bool result = false;
	
	// 削除するエントリを探して消す
	Iterator i = find(pEntry_);
	if (i == end())
	{
		// 見つからなかった
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	expunge(i);

	if (isRoot() == true)
	{
		// ルートノード
		
		if (getCount() == 0)
		{
			// ルートページをクリアする
			// -> 同時に両端のリーフページもクリアされる
			m_cFile.getHeaderPage()
				->setRootPageID(PhysicalFile::ConstValue::UndefinedPageID);
			// 自分を解放する
			m_cFile.freePage(this);

			result = true;
		}
		else if (getCount() == 1 && isLeaf() == false && isReduce_ == true)
		{
			// リーフじゃないので、自分を消して子をルートにする

			// 子のPageIDを得る
			PhysicalFile::PageID id = (*begin()).m_uiPageID;
			// ルートページを設定する
			m_cFile.getHeaderPage()->setRootPageID(id);
			// 自分を解放する
			m_cFile.freePage(this);

			result = true;
		}
	}
	else if (isReduce_ == true && getFreeCount() > (getMaxCount() / 2))
	{
		// このページの使用率が50%を割っているので、ページ連結や再配布が必要
		reduce();

		result = true;
	}

	return result;
}

//
//	FUNCTION public
//	KdTree::BtreePage::insert -- 指定位置に挿入する
//
//	NOTES
//
//	AGUMENTS
//	KdTree::BtreePage::Iterator i
//		挿入位置
//	const KdTree::BtreePage::Entry* pEntry_
//		挿入するエントリ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::insert(Iterator i_, const Entry* pEntry_)
{
	int n = static_cast<int>(i_ - begin());
	updateMode();
	dirty();
	i_ = begin() + n;

	bool nodeUpdate = false;
	if (i_ == begin() && m_pParentPage)
	{
		nodeUpdate = true;
		if (getCount() != 0)
		{
			Entry e((*begin()).m_uiKey, getID());
			m_pParentPage->expungeEntry(&e, false);
		}
	}

	ModSize length = static_cast<ModSize>(end() - i_);
	if (length != 0)
	{
		// 大きい部分を後ろに移動
		Os::Memory::move(&(*i_) + 1, &(*i_),
						 length * sizeof(Entry));
	}

	// 指定位置にコピーする
	Os::Memory::copy(&(*i_), pEntry_, sizeof(Entry));
	// エントリを増やす
	changeCount(1);

	if (nodeUpdate == true)
	{
		Entry e((*begin()).m_uiKey, getID());
		m_pParentPage->insertEntry(&e);
	}
}

//
//	FUNCTION public
//	KdTree::BtreePage::insert -- 指定位置に挿入する
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::BtreePage::Iterator i_
//		挿入位置
//	KdTree::BtreePage::ConstIterator start_
//		挿入するエントリの先頭
//	KdTree::BtreePage::ConstIterator end_
//		挿入するエントリの終端
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::insert(Iterator i_, ConstIterator start_, ConstIterator end_)
{
	int n = static_cast<int>(i_ - begin());
	updateMode();
	dirty();
	i_ = begin() + n;

	bool nodeUpdate = false;
	if (i_ == begin() && m_pParentPage)
	{
		nodeUpdate = true;
		if (getCount() != 0)
		{
			Entry e((*begin()).m_uiKey, getID());
			m_pParentPage->expungeEntry(&e, false);
		}
	}

	ModSize length = static_cast<ModSize>(end() - i_);
	ModSize size = static_cast<ModSize>(end_ - start_);
	if (length != 0)
	{
		// 大きい部分を後ろに移動
		Os::Memory::move(&(*i_) + size, &(*i_),
						 length * sizeof(Entry));
	}

	// 指定位置にコピーする
	Os::Memory::copy(&(*i_), &(*start_), size * sizeof(Entry));
	// エントリを増やす
	changeCount(static_cast<int>(end_ - start_));

	if (nodeUpdate == true)
	{
		Entry e((*begin()).m_uiKey, getID());
		m_pParentPage->insertEntry(&e);
	}
}

//
//	FUNCTION public
//	KdTree::BtreePage::expunge -- 指定位置のものを削除する
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::BtreePage::Iterator i_
//		削除するエントリ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::expunge(Iterator i_)
{
	int n = static_cast<int>(i_ - begin());
	updateMode();
	dirty();
	i_ = begin() + n;

	bool nodeUpdate = false;
	if (i_ == begin() && m_pParentPage)
	{
		nodeUpdate = true;
		Entry e((*begin()).m_uiKey, getID());
		if (getCount() == 1)
			m_pParentPage->expungeEntry(&e, true);
		else
			m_pParentPage->expungeEntry(&e, false);
	}

	ModSize length = static_cast<ModSize>(end() - (i_ + 1));
	if (length != 0)
	{
		// 大きい部分を前に移動
		Os::Memory::move(&(*i_), &(*(i_ + 1)), length * sizeof(Entry));
	}

	// エントリを減らす
	changeCount(-1);

	if (nodeUpdate == true && getCount() != 0)
	{
		Entry e((*begin()).m_uiKey, getID());
		m_pParentPage->insertEntry(&e);
	}
}

//
//	FUNCTION public
//	KdTree::BtreePage::expunge -- 指定位置のものを削除する
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::BtreePage::Iterator start_
//		削除するエントリの先頭
//	KdTree::BtreePage::Iterator end_
//		削除するエントリの終端
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::expunge(Iterator start_, Iterator end_)
{
	if (start_ != end_)
	{
		int ns = static_cast<int>(start_ - begin());
		int ne = static_cast<int>(end_ - begin());
		
		updateMode();
		dirty();
		
		start_ = begin() + ns;
		end_ = begin() + ne;

		bool nodeUpdate = false;
		if (start_ == begin() && m_pParentPage)
		{
			nodeUpdate = true;
			Entry e((*begin()).m_uiKey, getID());
			if (end_ == end())
				m_pParentPage->expungeEntry(&e, true);
			else
				m_pParentPage->expungeEntry(&e, false);
		}

		if (end_ != end())
		{
			ModSize length = static_cast<ModSize>(&(*end()) - &(*end_));
			
			// 大きい部分を前に移動
			Os::Memory::move(&(*start_), &(*end_), length * sizeof(Entry));
		}

		// エントリを減らす
		changeCount(static_cast<int>(end_ - start_) * -1);
		
		if (nodeUpdate == true && getCount() != 0)
		{
			Entry e((*begin()).m_uiKey, getID());
			m_pParentPage->insertEntry(&e);
		}
	}
}

//
//	FUNCTION public
//	KdTree::BtreePage::verify -- 整合性を検査する
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
BtreePage::verify()
{
}

//
//	FUNCTION public
//	KdTree::BtreePage::setLeaf -- リーフにする
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
BtreePage::setLeaf()
{
	updateMode();
	dirty();
	m_pHeader->m_uiCount |= 0x80000000;
}

//
//	FUNCTION public
//	KdTree::BtreePage::setPrevPageID -- 前方のページIDを設定する
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
BtreePage::setPrevPageID(PhysicalFile::PageID uiPrevPageID_)
{
	updateMode();
	dirty();
	m_pHeader->m_uiPrevPageID = uiPrevPageID_;
}

//
//	FUNCTION public
//	KdTree::BtreePage::setNextPageID -- 後方のページIDを設定する
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
BtreePage::setNextPageID(PhysicalFile::PageID uiNextPageID_)
{
	updateMode();
	dirty();
	m_pHeader->m_uiNextPageID = uiNextPageID_;
}

//
//	FUNCTION public
//	KdTree::BtreePage::detach -- ページをdetachする
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
BtreePage::detach()
{
	//	主に、BtreePage::PagePointer から呼ばれる
	
	m_pParentPage = 0;
	Page::detach();
}

//
//	FUNCTION private
//	KdTree::BtreePage::load -- ページ内容をクラス変数に設定する
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
BtreePage::load()
{
	char* p = getBuffer();
	m_pHeader = syd_reinterpret_cast<Header*>(p);
	m_pEntry = syd_reinterpret_cast<Entry*>(p + sizeof(Header));
}

//
//	FUNCTION private
//	KdTree::BtreePage::expand -- 拡張処理を行う
//
//	NOTES
//	ページ分割か再配分を行う
//
//	ARGUMENTS
//	const KdTree::BtreePage::Entry* pEntry_
//		挿入するエントリ
//
//	RETURN
//	KdTree::BtreePage::PagePointer
//		挿入するべきページ
//
//	EXCEPTIONS
//
BtreePage::PagePointer
BtreePage::expand(const Entry* pEntry_)
{
	PagePointer pResult;
	
	PagePointer pPrev;
	PagePointer pNext;
	int freeCount = 0;
		
	if (getPrevPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 前ページを得る
		pPrev = m_cFile.attachPage(getPrevPageID(),
								   m_cFile.getFixMode(),
								   PagePointer());
		freeCount = pPrev->getFreeCount();
		pNext = this;
	}
	else if (getNextPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 次ページを得る
		//	前がないので、親はこのページと同じはず
		pPrev = this;
		pNext = m_cFile.attachPage(getNextPageID(),
								   m_cFile.getFixMode(),
								   m_pParentPage);
		freeCount = pNext->getFreeCount();
	}
	else
	{
		// 前も後ろもない
		// 新しくルートノードを作成する
		PagePointer pRoot
			= m_cFile.allocatePage(
				PhysicalFile::ConstValue::UndefinedPageID,
				PhysicalFile::ConstValue::UndefinedPageID,
				PagePointer());

		// ルートノードを設定する
		m_cFile.getHeaderPage()->setRootPageID(pRoot->getID());
		// 自分の親を設定する
		m_pParentPage = pRoot;

		// ルートノードに先頭の要素を加える
		Entry e((*begin()).m_uiKey, getID());
		pRoot->insertEntry(&e);

		// 新しいページを得る
		pNext = m_cFile.allocatePage(
			getID(),
			PhysicalFile::ConstValue::UndefinedPageID,
			m_pParentPage);

		// 自分のヘッダーを更新する
		setNextPageID(pNext->getID());

		if (isLeaf() == true)
		{
			pNext->setLeaf();
			m_cFile.getHeaderPage()
				->setRightPageID(pNext->getID());
		}

		pPrev = this;
		freeCount = pNext->getFreeCount();
	}

	if (freeCount < (getMaxCount() / 10))
	{
		// 前後どちらかの空き容量が10%未満なので、ページ分割
		pResult = pNext->split(pPrev, pEntry_);
	}
	else
	{
		// それ以外なので、再配分
		pResult = pNext->redistribute(pPrev, pEntry_);
	}

	return pResult;
}

//
//	FUNCTION private
//	KdTree::BtreePage::reduce -- 縮小処理を行う
//
//	NOTES
//	ページ連結か再配分を行う
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
BtreePage::reduce()
{
	PagePointer pPrev;
	PagePointer pNext;
	int freeCount = 0;

	if (getPrevPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 前ページを得る
		pPrev = m_cFile.attachPage(getPrevPageID(),
								   m_cFile.getFixMode(),
								   PagePointer());
		freeCount = pPrev->getFreeCount();
		pNext = this;
	}
	else if (getNextPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 次ページを得る
		//	前がないので、親はこのページと同じはず
		pPrev = this;
		pNext = m_cFile.attachPage(getNextPageID(),
								   m_cFile.getFixMode(),
								   m_pParentPage);
		freeCount = pNext->getFreeCount();
	}

	if (pPrev)
	{
		if (freeCount > getMaxCount() / 2)
		{
			// ページ連結
			pNext->concatenate(pPrev);
		}
		else
		{
			// 再配分
			pNext->redistribute(pPrev);
		}
	}
}

//
//	FUNCTION private
//	KdTree::BtreePage::split -- ページ分割する
//
//	NOTES
//	自分とpPrevPage_の間に新しいページを作成する
//
//	ARGUMENTS
//	KdTree::BtreePage::PagePointer pPrevPage_
//		前のページ
//	const KdTree::BtreePage::Entry* pEntry_
//		挿入するエントリ
//
//	RETURN
//	KdTree::BtreePage::PagePointer
//		挿入するべきページ
//
//	EXCEPTIONS
//
BtreePage::PagePointer
BtreePage::split(PagePointer pPrevPage_, const Entry* pEntry_)
{
	// 新しいページを確保
	PagePointer pNewPage = m_cFile.allocatePage(pPrevPage_->getID(),
												getID(),
												m_pParentPage);
	if (isLeaf() == true)
	{
		pNewPage->setLeaf();
	}
	// 前ページの次ページを更新する
	pPrevPage_->setNextPageID(pNewPage->getID());
	// 自分の前ページを更新する
	setPrevPageID(pNewPage->getID());

	// 最大登録数
	int maxCount = getMaxCount();

	//
	//	まずは前ページの終わり1/3を新しいページに移動する
	//
	
	// 前ページに残すエントリを計算する
	Iterator b = pPrevPage_->begin();
	Iterator i = b;
	Iterator e = pPrevPage_->end();
	for (; i < e; ++i)
	{
		if ((i - b) >= (maxCount * 2 / 3))
			// 2/3を超えたので終わり
			break;
	}
	// 新しいページに移動する
	pNewPage->insert(pNewPage->begin(), i, pPrevPage_->end());
	// 前ページから削除する
	pPrevPage_->expunge(i, pPrevPage_->end());

	//
	//	次にこのページの先頭1/3を新しいページに移動する
	//

	// このページに残すエントリを計算する
	b = begin();
	e = end();
	i = b;
	for (; i < e; ++i)
	{
		if ((i - b) >= (maxCount * 1 / 3))
			// 1/3を超えたので終わり
			break;
	}
	// 新しいページに移動する
	pNewPage->insert(pNewPage->end(), begin(), i);
	// このページから削除する(親が変わっている可能性がある)
	setParentPage(m_cFile.findPage(&(*begin()), getID()));
	expunge(begin(), i);

	//
	//	どのページに入れるべきか
	//

	PagePointer pResult;
	
	Compare compare;
	if (compare(*pEntry_, *pNewPage->begin()) < 0)
	{
		// 前ページ
		pResult = pPrevPage_;
	}
	else if (compare(*pEntry_, *begin()) < 0)
	{
		// 新しいページ
		pResult = pNewPage;
	}
	else
	{
		// このページ
		pResult = this;
	}

	return pResult;
}

//
//	FUNCTION private
//	KdTree::BtreePage::concatenate -- ページ連結
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::BtreePage::PagePointer pPrevPage_
//		前のページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::concatenate(PagePointer pPrevPage_)
{
	if (getNextPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 次のページの前ページを更新する
		PagePointer p = m_cFile.attachPage(getNextPageID(),
										   m_cFile.getFixMode(),
										   PagePointer());
		p->setPrevPageID(pPrevPage_->getID());
	}
	else if (isLeaf() == true)
	{
		// 一番右のリーフページを更新する
		m_cFile.getHeaderPage()
			->setRightPageID(pPrevPage_->getID());
	}
	// 前ページの次ページを更新する
	pPrevPage_->setNextPageID(getNextPageID());

	// このページの内容を移動する
	pPrevPage_->insert(pPrevPage_->end(), begin(), end());
	// 移動した分を削除する
	expunge(begin(), end());

	// このページを削除
	m_cFile.freePage(this);
}

//
//	FUNCTION private
//	KdTree::BtreePage::redistribute -- 再配分
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::BtreePage::PagePointer pPrevPage_
//		前のページ
//	const Entry* pEntry_
//		挿入するエントリ (default 0)
//
//	RETURN
//	PagePointer
//		エントリを挿入するべきページ
//
//	EXCEPTIONS
//
BtreePage::PagePointer
BtreePage::redistribute(PagePointer pPrevPage_, const Entry* pEntry_)
{
	// 半分のエントリ数を求める
	int half = (pPrevPage_->getCount() + getCount()) / 2;
	
	if (pPrevPage_->getCount() < getCount())
	{
		// このページから持っていく
		int m = getCount() - half;

		// このページから持ってくるエントリを求める
		Iterator i = begin();
		i += m;

		// 前ページに移動する
		pPrevPage_->insert(pPrevPage_->end(), begin(), i);
		// このページから移動した分を削除する
		expunge(begin(), i);
	}
	else
	{
		// 前ページから持ってくる

		// 前ページに残すエントリを求める
		Iterator i = pPrevPage_->begin();
		i += half;

		// このページに移動する
		insert(begin(), i, pPrevPage_->end());
		// 前ページから移動した分を削除する
		pPrevPage_->expunge(i, pPrevPage_->end());
	}

	// 挿入するべきページを求める
	PagePointer pResult;
	if (pEntry_)
	{
		Compare compare;
		if (compare(*pEntry_, *begin()) < 0)
		{
			// 前ページ
			pResult = pPrevPage_;
		}
		else
		{
			// このページ
			pResult = this;
		}
	}

	return pResult;
}

//
//	FUNCTION private
//	KdTree::BtreePage::changeCount -- 件数を変更する
//
//	NOTES
//
//	ARGUMENTS
//	int changeCount_
//		増減する数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::changeCount(int changeCount_)
{
	dirty();
	m_pHeader->m_uiCount += changeCount_;
}

//
//	Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
