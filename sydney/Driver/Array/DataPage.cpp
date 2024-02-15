// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataPage.cpp --
// 
// Copyright (c) 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Array";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Array/Module.h"
#include "Array/Algorithm.h"
#include "Array/ArrayFile.h"
#include "Array/Compare.h"
#include "Array/Condition.h"
#include "Array/DataPage.h"
#include "Array/Message_IllegalLeftLeafPageID.h"
#include "Array/Message_IllegalRightLeafPageID.h"
#include "Array/Message_IllegalKeyIndex.h"
#include "Array/Message_DiscordDelegateKey.h"
#include "Array/Tree.h"

#include "Common/Assert.h"
#include "Exception/Cancel.h"
#include "Exception/UniquenessViolation.h"
#include "Exception/VerifyAborted.h"

_SYDNEY_USING
_SYDNEY_ARRAY_USING

namespace
{
	//
	//	VARIABLE local
	//
	ModUInt32 _LEAF_MASK =	0x80000000;
}

//
//	FUNCTION public
//	Array::DataPage::DataPage -- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	Array::ArrayFile& cFile_
//		ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
DataPage::DataPage(ArrayFile& cFile_)
	: Page(cFile_), m_cFile(cFile_), m_pTree(0)
{
}

//
//	FUNCTION public
//	Array::DataPage::setPhysicalPage -- 物理ページを設定する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPhysicalPage_
//		物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataPage::setPhysicalPage(PhysicalFile::Page* pPhysicalPage_)
{
	Page::setPhysicalPage(pPhysicalPage_);
	m_pHeader = syd_reinterpret_cast<Header*>(Page::getBuffer());

	initialize();
}

//
//	FUNCTION public
//	Array::DataPage::setPhysicalPage -- 物理ページを設定する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPhysicalPage_
//		物理ページ
//	PhysicalFile::PageID uiPrevPageID_
//		前のページ
//	PhysicalFile::PageID uiNextPageID_
//		後のページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataPage::setPhysicalPage(PhysicalFile::Page* pPhysicalPage_,
						  PhysicalFile::PageID uiPrevPageID_,
						  PhysicalFile::PageID uiNextPageID_)
{
	Page::setPhysicalPage(pPhysicalPage_);
	m_pHeader = syd_reinterpret_cast<Header*>(Page::getBuffer());
	dirty();
	m_pHeader->m_uiCount = 0;
	m_pHeader->m_uiPrevPageID = uiPrevPageID_;
	m_pHeader->m_uiNextPageID = uiNextPageID_;

	initialize();
}

//
//	FUNCTION public
//	Array::DataPage::~DataPage -- デストラクタ
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
DataPage::~DataPage()
{
}

//
//	FUNCTION public
//	Array::DataPage::getPageSize -- ページサイズを得る(UNIT単位)
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		ヘッダーを除いたページサイズ
//
//	EXCEPTIONS
//
ModSize
DataPage::getPageSize()
{
	return Page::getPageSize() - sizeof(Header)/sizeof(ModUInt32);
}

//
//	FUNCTION public
//	Array::DataPage::isRoot -- ルートページか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		ルートページの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
DataPage::isRoot() const
{
	return (m_pHeader->m_uiPrevPageID
			== PhysicalFile::ConstValue::UndefinedPageID
			&& m_pHeader->m_uiNextPageID
			== PhysicalFile::ConstValue::UndefinedPageID);
}

//
//	FUNCTION public
//	Array::DataPage::isLeaf -- リーフページか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		リーフページの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
DataPage::isLeaf() const
{
	return (m_pHeader->m_uiCount & _LEAF_MASK);
}

//
//	FUNCTION public
//	Array::DataPage::setLeaf -- リーフページに設定する
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
DataPage::setLeaf()
{
	m_pHeader->m_uiCount |= _LEAF_MASK;
}

//
//	FUNCTION public
//	Array::DataPage::preFlush -- 確定前処理を行う
//
//	NOTES
//	ページがdirtyの時しか呼び出されない
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataPage::preFlush()
{
	m_cFile.pushPageID(getID());
}

//
//	FUNCTION public
//	Array::DataPage::getCount -- ページのエントリ数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		ページ内のエントリ数
//
//	EXCEPTIONS
//
ModUInt32
DataPage::getCount()
{
	return m_pHeader->m_uiCount & ~_LEAF_MASK;
}

//
//	FUNCTION public
//	Array::DataPage::getUsedSize -- 使用サイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		使用サイズ(UNIT単位)
//
//	EXCEPTIONS
//
ModSize
DataPage::getUsedSize()
{
	return static_cast<ModSize>(*end() - *begin());
}

//
//	FUNCTION public
//	Array::DataPage::getIteratorTowardLeafLowerBound --
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pValue_
//	const Compare& cCompare_
//
//	RETURN
//	Array::DataPage::Iterator
//
//	EXCEPTIONS
//
DataPage::Iterator
DataPage::getIteratorTowardLeafLowerBound(const ModUInt32* pValue_,
										  const Compare& cCompare_)
{
	// min(x | x >= k)
	Iterator i = Algorithm::lowerBound(begin(), end(), pValue_, cCompare_);

	if (i != begin() && isLeaf() == false)
	{
		// Get the previous iterator.
		//
		//  p : 1,2,7
		//	c1: 1,2,2
		//	c2: 2,5,6
		//	c3: 7,8,9
		// When the searched value is min(x | x >= 2),
		// Algorithm::lowerBound returns 2 in 'p'.
		// But c1 have also 2.
		// So get the previous iterator.
		//
		// And this function is NOT useful for an unique search.
		// See upperBound.
		//
		--i;
	}

	// Node does NOT return the end of iterator.
	// See upperBound.
	; _SYDNEY_ASSERT(isLeaf() == true || i != end());
	return i;
}

//
//	FUNCTION public
//	Array::DataPage::getIteratorTowardLeafUpperBound --
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pValue_
//	const Comapre& cCompare_
//
//	RETURN
//	Array::DataPage::Iterator
//
//	EXCEPTIONS
//
DataPage::Iterator
DataPage::getIteratorTowardLeafUpperBound(const ModUInt32* pValue_,
										  const Compare& cCompare_)
{
	// min(x | x > k)
	Iterator i = Algorithm::upperBound(begin(), end(), pValue_, cCompare_);

	if (i != begin() && isLeaf() == false)
	{
		// Get the previous iterator.
		//
		//  p : 1,4,7,e
		//	c1: 1,2,3,e
		//	c2: 4,5,6,e
		//	c3: 7,8,9,e
		// When the searched value is min(x | x > 2),
		// Algorithm::upperBound returns 4 in 'p'.
		// But c1 have nearer value, 3.
		// So get the previous iterator.
		//
		// And it's useful for an unique search.
		// If the value which match exactly exist,
		// it ALWAYS exists in the previous node.
		//
		// For example, when the searched value is 4,
		// Algorithm::upperBound returns 7 in 'p'.
		// And c2 which is in ahead of c3 have 4.
		// NOT need to check whether the value is equals to the condition.
		//
		// If getIteratorTowardLeafLowerBound is used,
		// Algorithm::lowerBound retuns 4 in 'p',
		// and it's not possible to get previous iterator
		// without checking whether the values is equals to the condition.
		//
		--i;
	}

	// Node does NOT return the end of iterator.
	// When the searched value is min(x | x > 7),
	// Algorithm::upperBound retuns 'e' in 'p'.
	// In such case, it's NOT possible to reach the leaf page, c3.
	; _SYDNEY_ASSERT(isLeaf() == true || i != end());
	return i;
}

//
//	FUNCTION public
//	Array::DataPage::find -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pValue_
//		比較対象データ
//	const Compare& cCompare_
//
//	RETURN
//	Array::DataPage::Iterator
//		検索結果位置へのイテレータ、見つからなかった場合はend()を返す
//
//	EXCEPTIONS
//
DataPage::Iterator
DataPage::find(const ModUInt32* pValue_, const Compare& cCompare_)
{
	// This function is always used for an unique search.
	; _SYDNEY_ASSERT(m_pTree->isForUniqueSearch(cCompare_));
	
	// Get min(x | x >=k).
	Iterator e = end();
	Iterator i = Algorithm::lowerBound(begin(), e, pValue_, cCompare_);

	// Check if x == k.
	if (i == e || cCompare_(*i, pValue_) != 0)
	{
		// Not found.
		return e;
	}
	return i;
}

//
//	FUNCTION public
//	Array::DataPage::insertEntry -- エントリを挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pBuffer_
//		挿入するエントリ
//	ModSize uiSize_
//		挿入するエントリのサイズ (default 0)
//
//	RETURN
//	bool
//		ページ分割、再配布が発生した場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
DataPage::insertEntry(const ModUInt32* pBuffer_, ModSize uiSize_)
{
	; _SYDNEY_ASSERT(uiSize_ != 0);
	
	bool result = false;

	const Data& dataClass = getDataClass();
	
	if (dataClass.isFixed() == true && uiSize_ != dataClass.getFixedSize())
	{
		// スキーマ情報と入力値の大きさがあっていない(通常ありえない)
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	PagePointer pPage = this;
	
	if (getFreeSize() < uiSize_)
	{
		// 空き領域がない -> ページ分割か再配布が必要

		pPage = expand(pBuffer_);

		result = true;
	}

	// Get the iterator which is more than pBuffer_ and minimum.
	const Compare& cCompare = Condition::getCompare(m_pTree);
	Iterator i = Algorithm::lowerBound(
		pPage->begin(), pPage->end(), pBuffer_, cCompare);
	// Check unique violation.
	if (i != pPage->end() && cCompare(*i, pBuffer_) == 0)
	{
		_SYDNEY_THROW0(Exception::UniquenessViolation);
	}
	// Insert the entry.
	pPage->insert(i, pBuffer_, uiSize_);

	return result;
}

//
//	FUNCTION public
//	Array::DataPage::expungeEntry -- エントリを削除する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pBuffer_
//		挿入するエントリ
//	ModSize uiSize_
//		挿入するエントリのサイズ (default 0)
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
DataPage::expungeEntry(const ModUInt32* pBuffer_, ModSize uiSize_,
					   bool isReduce_)
{
	bool result = false;
	
	// 削除するエントリを探して消す
	const Compare& cCompare = Condition::getCompare(m_pTree);
	Iterator i = find(pBuffer_, cCompare);
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
			// ヘッダーページを初期化する
			m_pTree->initializeHeader();
			m_pTree->incrementCount();	// decrementされるので1にしとく

				// 自分を解放する
			m_cFile.freePage(this);

			result = true;
		}
		else if (getCount() == 1 && isLeaf() == false && isReduce_ == true)
		{
			// リーフじゃないので、自分を消して子をルートにする

			// 子のPageIDを得る
			PhysicalFile::PageID id = m_pTree->getPageID(*begin());
			// ルートページを設定する
			m_pTree->setRootPageID(id);
			m_pTree->decrementStepCount();
			// 自分を解放する
			m_cFile.freePage(this);

			result = true;
		}
	}
	else if (isReduce_ == true && getFreeSize() > getPageSize() / 2)
	{
		// このページの使用率が50%を割っているので、ページ連結や再配布が必要
		reduce();

		result = true;
	}

	return result;
}

//
//	FUNCTION public
//	Array::DataPage::verify -- 整合性を検査する
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
DataPage::verify()
{
	if (isLeaf() == true)
	{
		if (getPrevPageID()	== PhysicalFile::ConstValue::UndefinedPageID)
		{
			// 一番左のリーフページIDをチェック
			if (m_pTree->getLeftLeafPageID() != getID())
			{
				_SYDNEY_VERIFY_INCONSISTENT(m_cFile.getProgress(),
											m_cFile.getPath(),
											Message::IllegalLeftLeafPageID());
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
		}
		if (getNextPageID() == PhysicalFile::ConstValue::UndefinedPageID)
		{
			// 一番右のリーフページIDをチェック
			if (m_pTree->getRightLeafPageID() != getID())
			{
				_SYDNEY_VERIFY_INCONSISTENT(m_cFile.getProgress(),
											m_cFile.getPath(),
											Message::IllegalRightLeafPageID());
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
		}

		// エントリ数を加える
		m_cFile.addVerifyEntryCount(getCount());
	}
	
	Iterator prev = 0;
	Iterator i = begin();
	Iterator e = end();
	const Compare& cCompare = Condition::getCompare(m_pTree);
	for (; i != e; ++i)
	{
		if (&*prev)
		{
			// 大小関係をチェック
			if (cCompare(*prev, *i) >= 0)
			{
				_SYDNEY_VERIFY_INCONSISTENT(m_cFile.getProgress(),
											m_cFile.getPath(),
											Message::IllegalKeyIndex(
												getID(), begin() - i));
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
		}

		if (isLeaf() == false)
		{
			// 下位ページをattachする
			PhysicalFile::PageID uiPageID = m_pTree->getPageID(*i);
			PagePointer pPage = m_cFile.attachPage(
				m_pTree, uiPageID, PagePointer(this));

			// 先頭要素が同じかチェックする
			if (cCompare(*i, *pPage->begin()) != 0)
			{
				_SYDNEY_VERIFY_INCONSISTENT(m_cFile.getProgress(),
											m_cFile.getPath(),
											Message::DiscordDelegateKey(
												getID(), uiPageID));
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}

			// 下位ページの整合性をチェックする
			pPage->verify();

			if (m_cFile.isCancel() == true)
			{
				// 中断
				_SYDNEY_THROW0(Exception::Cancel);
			}
		}

		prev = i;
	}
}

//
//	FUNCTION public
//	Array::DataPage::detach -- デタッチする
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
DataPage::detach()
{
	m_pParentPage = 0;
	Page::detach();
}

//
//	FUNCTION private
//	Array::DataPage::initialize -- 初期化
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
DataPage::initialize()
{
	ModUInt32 count = getCount();
	
	if (m_vecpEntry.getCapacity() < count + 2)
		m_vecpEntry.reserve(count + 2);	// 1件挿入されてもreallocateしないように

	// [CAUTION]
	// m_vecpEntry has one entry when the count is 0.
	m_vecpEntry.assign(count + 1, 0);

	load(count);
}

//
//	FUNCTION private
//	Array::DataPage::load -- ベクターに値を設定する
//
//	NOTES
//	ベクターの要素を書き換えるので、サイズ分のデータが格納されている必要がある
//
//	ARGUMENTS
//	ModUInt32 count_
//		要素数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataPage::load(ModUInt32 count_)
{
	;_SYDNEY_ASSERT(m_vecpEntry.getSize() == (count_ + 1));
	
	ModUInt32* p = getBuffer();
	ModVector<ModUInt32*>::Iterator j = m_vecpEntry.begin();
	const Data& dataClass = getDataClass();
	if (dataClass.isFixed() == true)
	{
		ModSize fixedSize = dataClass.getFixedSize();
		for (ModUInt32 i = 0; i < count_; ++i)
		{
			*j++ = p;
			p += fixedSize;
		}
	}
	else
	{
		for (ModUInt32 i = 0; i < count_; ++i)
		{
			*j++ = p;
			p += dataClass.getSize(p);
		}
	}
	*j = p;
}

//
//	FUNCTION protected
//	Array::DataPage::reload -- 物理ページが変更されたので読み直す
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
void
DataPage::reload()
{
	ModUInt32 count = getCount();
	load(count);
}

//
//	FUNCTION public
//	Array::DataPage::insert -- 指定位置に挿入する
//
//	NOTES
//
//	AGUMENTS
//	Array::DataPage::Iterator i
//		挿入位置
//	const ModUInt32* pBuffer_
//		挿入するエントリ
//	ModSize uiSize_
//		挿入するエントリのサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataPage::insert(Iterator i_, const ModUInt32* pBuffer_, ModSize uiSize_)
{
	updateMode();
	dirty();

	bool nodeUpdate = false;
	if (i_ == begin() && m_pParentPage)
	{
		nodeUpdate = true;
		if (getCount() != 0)
		{
			ModSize uiSize;
			AutoPointer<ModUInt32> p = makeNodeEntry(*begin(), uiSize);
			m_pParentPage->expungeEntry(p, uiSize, false);
		}
	}

	ModUInt32* first = *i_;
	ModUInt32* last = *end();
	ModSize length = static_cast<ModSize>(last - first);
	if (length != 0)
	{
		// 大きい部分を後ろに移動
		Os::Memory::move(first + uiSize_, first, length * sizeof(ModUInt32));
	}

	// ベクターのアドレスを更新する
	Iterator i = i_;
	Iterator e = end();
	for (; i <= e; ++i)
	{
		*i += uiSize_;
	}

	// 指定位置にコピーする
	Os::Memory::copy(first, pBuffer_, uiSize_ * sizeof(ModUInt32));
	// ベクターにも追加
	m_vecpEntry.insert(i_, first);
	// エントリを増やす
	addCount(1);

	if (nodeUpdate == true)
	{
		ModSize uiSize;
		AutoPointer<ModUInt32> p = makeNodeEntry(*begin(), uiSize);
		m_pParentPage->insertEntry(p, uiSize);
	}
}

//
//	FUNCTION public
//	Array::DataPage::insert -- 指定位置に挿入する
//
//	NOTES
//
//	ARGUMENTS
//	Array::DataPage::Iterator i_
//		挿入位置
//	Array::DataPage::ConstIterator start_
//		挿入するエントリの先頭
//	Array::DataPage::ConstIterator end_
//		挿入するエントリの終端
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataPage::insert(Iterator i_, ConstIterator start_, ConstIterator end_)
{
	updateMode();
	dirty();

	bool nodeUpdate = false;
	if (i_ == begin() && m_pParentPage)
	{
		nodeUpdate = true;
		if (getCount() != 0)
		{
			ModSize uiSize;
			AutoPointer<ModUInt32> p = makeNodeEntry(*begin(), uiSize);
			m_pParentPage->expungeEntry(p, uiSize, false);
		}
	}

	ModUInt32* first = *i_;
	ModUInt32* last = *end();
	ModSize length = static_cast<ModSize>(last - first);
	ModSize size = static_cast<ModSize>(*end_ - *start_);
	if (length != 0)
	{
		// 大きい部分を後ろに移動
		Os::Memory::move(first + size, first, length * sizeof(ModUInt32));
	}

	// 指定位置にコピーする
	Os::Memory::copy(first, *start_, size * sizeof(ModUInt32));
	// エントリを増やす
	addCount(end_ - start_);
	// ベクターを読み直す
	initialize();

	if (nodeUpdate == true)
	{
		ModSize uiSize;
		AutoPointer<ModUInt32> p = makeNodeEntry(*begin(), uiSize);
		m_pParentPage->insertEntry(p, uiSize);
	}
}

//
//	FUNCTION public
//	Array::DataPage::expunge -- 指定位置のものを削除する
//
//	NOTES
//
//	ARGUMENTS
//	Array::DataPage::Iterator i_
//		削除するエントリ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataPage::expunge(Iterator i_)
{
	updateMode();
	dirty();

	bool nodeUpdate = false;
	if (i_ == begin() && m_pParentPage)
	{
		nodeUpdate = true;
		ModSize uiSize;
		AutoPointer<ModUInt32> p = makeNodeEntry(*begin(), uiSize);
		if (getCount() == 1)
			m_pParentPage->expungeEntry(p, uiSize, true);
		else
			m_pParentPage->expungeEntry(p, uiSize, false);
	}

	ModUInt32* first = *(i_ + 1);
	ModUInt32* last = *end();
	ModSize length = static_cast<ModSize>(last - first);
	ModSize size = static_cast<ModSize>(first - *i_);
	if (length != 0)
	{
		// 大きい部分を前に移動
		Os::Memory::move(*i_, first, length * sizeof(ModUInt32));
	}

	// ベクターのアドレスを更新する
	Iterator i = i_ + 1;
	Iterator e = end();
	for (; i <= e; ++i)
	{
		*i -= size;
	}

	// ベクターから削除
	m_vecpEntry.erase(i_);
	// エントリを減らす
	subtractCount(1);

	if (nodeUpdate == true && getCount() != 0)
	{
		ModSize uiSize;
		AutoPointer<ModUInt32> p = makeNodeEntry(*begin(), uiSize);
		m_pParentPage->insertEntry(p, uiSize);
	}
}

//
//	FUNCTION public
//	Array::DataPage::expunge -- 指定位置のものを削除する
//
//	NOTES
//
//	ARGUMENTS
//	Array::DataPage::Iterator start_
//		削除するエントリの先頭
//	Array::DataPage::Iterator end_
//		削除するエントリの終端
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataPage::expunge(Iterator start_, Iterator end_)
{
	if (start_ != end_)
	{
		updateMode();
		dirty();

		bool nodeUpdate = false;
		if (start_ == begin() && m_pParentPage)
		{
			nodeUpdate = true;
			ModSize uiSize;
			AutoPointer<ModUInt32> p = makeNodeEntry(*begin(), uiSize);
			if (end_ == end())
				m_pParentPage->expungeEntry(p, uiSize, true);
			else
				m_pParentPage->expungeEntry(p, uiSize, false);
		}

		// エントリを減らす
		subtractCount(end_ - start_);
		
		if (end_ == end())
		{
			// 最後なので、ベクターを消すだけ
			m_vecpEntry.erase(start_ + 1, end_ + 1);
		}
		else
		{
			ModUInt32* first = *start_;
			ModUInt32* last = *end_;
			ModSize length = static_cast<ModSize>(*end() - last);
			
			// 大きい部分を前に移動
			Os::Memory::move(first, last, length * sizeof(ModUInt32));
			
			// ベクターを読み直す
			initialize();
		}

		if (nodeUpdate == true && getCount() != 0)
		{
			ModSize uiSize;
			AutoPointer<ModUInt32> p = makeNodeEntry(*begin(), uiSize);
			m_pParentPage->insertEntry(p, uiSize);
		}
	}
}

//
//	FUNCTION protected
//	Array::DataPage::getBuffer -- データ領域へのポインタを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32*
//		データ領域へのポインタ
//
//	EXCEPTIONS
//
ModUInt32*
DataPage::getBuffer()
{
	return Page::getBuffer() + sizeof(Header)/sizeof(ModUInt32);
}

//
//	FUNCTION protected
//	Array::DataPage::addCount -- ページのエントリ数を増やす
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiCount_
//		 増やす数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataPage::addCount(ModUInt32 uiCount_)
{
	dirty();
	m_pHeader->m_uiCount += uiCount_;
}

//
//	FUNCTION protected
//	Array::DataPage::subtractCount -- ページのエントリ数を減らす
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiCount_
//		 減らす数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataPage::subtractCount(ModUInt32 uiCount_)
{
	dirty();
	m_pHeader->m_uiCount -= uiCount_;
}

//
//	FUNCTION protected
//	Array::DataPage::getDataClass -- データクラスを得る
//
// 	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Array::Data&
//		 データクラス
//
//	EXCEPTIONS
//
const Data&
DataPage::getDataClass() const
{
	return (isLeaf() == true) ? m_pTree->getLeafData() : m_pTree->getNodeData();
}

//
//	FUNCTION protected
//	Array::Page::updateMode -- 更新モードに変更する
//
//	NOTES
//	ReadOnlyでattachしている物理ページをWriteでattachしなおす。
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
DataPage::updateMode()
{
	if (!isReadOnly()) return;
	
	DataPage::setPhysicalPage(m_cFile.changeFixMode(getPhysicalPage()));
	
	reload();
}

//
//	FUNCTION private
//	Array::DataPage::expand -- 拡張処理を行う
//
//	NOTES
//	ページ分割か再配分を行う
//
//	ARGUMENTS
//	const ModUInt32* pBuffer_
//		挿入するエントリ
//
//	RETURN
//	Array::DataPage::PagePointer
//		挿入するべきページ
//
//	EXCEPTIONS
//
DataPage::PagePointer
DataPage::expand(const ModUInt32* pBuffer_)
{
	// 通常のsplit or redistribute

	PagePointer pPrev;
	PagePointer pNext;
	ModSize freeSize = 0;
	
	if (getPrevPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 前ページを得る
		pPrev = m_cFile.attachPage(m_pTree, getPrevPageID(), PagePointer());
		freeSize = pPrev->getFreeSize();
		pNext = this;
	}
	else if (getNextPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 次ページを得る
		//	前がないので、親はこのページと同じはず
		pPrev = this;
		pNext = m_cFile.attachPage(m_pTree, getNextPageID(), m_pParentPage);
		freeSize = pNext->getFreeSize();
	}
	else
	{
		// 前も後ろもない
		// 新しくルートノードを作成する
		; _SYDNEY_ASSERT(isRoot() == true);
		createNewRoot();
		
		// 新しいページを得る
		pNext = getNewRightMostPage();
		pPrev = this;
		freeSize = pNext->getFreeSize();
	}

	PagePointer pResult;
	if (freeSize < (getPageSize() / 10))
	{
		// 前後どちらかの空き容量が10%未満なので、ページ分割
		pResult = pNext->split(pPrev, pBuffer_);
	}
	else
	{
		// それ以外なので、再配分
		pResult = pNext->redistribute(pPrev, pBuffer_);
	}
	return pResult;
}

//
//	FUNCTION private
//	Array::DataPage::createNewRoot --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
DataPage::createNewRoot()
{
	// This page is root.
	; _SYDNEY_ASSERT(isRoot() == true);
	
	PagePointer pNewRoot
		= m_cFile.allocatePage(
			m_pTree,
			PhysicalFile::ConstValue::UndefinedPageID,
			PhysicalFile::ConstValue::UndefinedPageID,
			PagePointer());

	// ルートノードを設定する
	m_pTree->setRootPageID(pNewRoot->getID());
	m_pTree->incrementStepCount();

	// Set the new root to the parent of the former root.
	m_pParentPage = pNewRoot;

	// ルートノードに先頭の要素を加える
	ModSize size;
	AutoPointer<ModUInt32> entry = makeNodeEntry(*begin(), size);
	m_pParentPage->insertEntry(entry, size);
}

//
//	FUNCTION private
//	Array::DataPage::getNewRightMostPage --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	Array::DataPage::PagePointer
//		New right most page
//
//	EXCEPTIONS
//
DataPage::PagePointer
DataPage::getNewRightMostPage()
{
	// This page is right most page.
	; _SYDNEY_ASSERT(getNextPageID() ==
					 PhysicalFile::ConstValue::UndefinedPageID);
	
	PagePointer pNewPage = m_cFile.allocatePage(
		m_pTree,
		getID(),
		PhysicalFile::ConstValue::UndefinedPageID,
		m_pParentPage);

	// このページの次ページを新しいページにする
	setNextPageID(pNewPage->getID());
	if (isLeaf() == true)
	{
		pNewPage->setLeaf();
		m_pTree->setRightLeafPageID(pNewPage->getID());
	}
	return pNewPage;
}

//
//	FUNCTION private
//	Array::DataPage::reduce -- 縮小処理を行う
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
DataPage::reduce()
{
	PagePointer pPrev;
	PagePointer pNext;
	ModSize freeSize = 0;

	if (getPrevPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 前ページを得る
		pPrev = m_cFile.attachPage(m_pTree, getPrevPageID(), PagePointer());
		freeSize = pPrev->getFreeSize();
		pNext = this;
	}
	else if (getNextPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 次ページを得る
		//	前がないので、親はこのページと同じはず
		pPrev = this;
		pNext = m_cFile.attachPage(m_pTree, getNextPageID(), m_pParentPage);
		freeSize = pNext->getFreeSize();
	}

	if (pPrev)
	{
		if (freeSize > getPageSize() / 2)
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
//	Array::DataPage::split -- ページ分割する
//
//	NOTES
//	自分とpPrevPage_の間に新しいページを作成する
//
//	ARGUMENTS
//	Array::DataPage::PagePointer pPrevPage_
//		前のページ
//	const ModUInt32* pBuffer_
//		挿入するエントリ
//
//	RETURN
//	Array::DataPage::PagePointer
//		挿入するべきページ
//
//	EXCEPTIONS
//
DataPage::PagePointer
DataPage::split(PagePointer pPrevPage_, const ModUInt32* pBuffer_)
{
	; _SYDNEY_ASSERT(isRoot() == false);
	
	// 新しいページを確保
	PagePointer pNewPage = m_cFile.allocatePage(m_pTree,
												pPrevPage_->getID(),
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

	// ページサイズ
	ModSize totalUnit = getPageSize();

	//
	//	まずは前ページの終わり1/3を新しいページに移動する
	//
	
	// 前ページに残すエントリを計算する
	Iterator i = pPrevPage_->begin();
	Iterator e = pPrevPage_->end();
	ModUInt32* b = *i;
	for (; i < e; ++i)
	{
		if ((*i - b) >= static_cast<int>(totalUnit * 2 / 3))
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
	i = begin();
	e = end();
	b = *i;
	for (; i < e; ++i)
	{
		if ((*i - b) >= static_cast<int>(totalUnit * 1 / 3))
			// 1/3を超えたので終わり
			break;
	}
	// 新しいページに移動する
	pNewPage->insert(pNewPage->end(), begin(), i);
	// このページから削除する(親が変わっている可能性がある)
	setParentPage(m_cFile.findParentPage(m_pTree, *begin(), getID()));
	expunge(begin(), i);

	//
	//	どのページに入れるべきか
	//

	PagePointer pResult;
	
	const Compare& cCompare = Condition::getCompare(m_pTree);
	if (cCompare(pBuffer_, *(pNewPage->begin())) < 0)
	{
		// 前ページ
		pResult = pPrevPage_;
	}
	else if (cCompare(pBuffer_, *begin()) < 0)
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
//	Array::DataPage::concatenate -- ページ連結
//
//	NOTES
//
//	ARGUMENTS
//	Array::DataPage::PagePointer pPrevPage_
//		前のページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataPage::concatenate(PagePointer pPrevPage_)
{
	if (getNextPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 次のページの前ページを更新する
		PagePointer p = m_cFile.attachPage(m_pTree,
										   getNextPageID(),
										   PagePointer());
		p->setPrevPageID(pPrevPage_->getID());
	}
	else if (isLeaf() == true)
	{
		// 一番右のリーフページを更新する
		m_pTree->setRightLeafPageID(pPrevPage_->getID());
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
//	Array::DataPage::redistribute -- 再配分
//
//	NOTES
//
//	ARGUMENTS
//	Array::DataPage::PagePointer pPrevPage_
//		前のページ
//	const ModUInt32* pBuffer_
//		挿入するエントリ (default 0)
//
//	RETURN
//	PagePointer
//		エントリを挿入するべきページ
//
//	EXCEPTIONS
//
DataPage::PagePointer
DataPage::redistribute(PagePointer pPrevPage_, const ModUInt32* pBuffer_)
{
	// 半分のユニット数を求める
	ModSize half = (pPrevPage_->getUsedSize() + getUsedSize()) / 2;
	
	if (pPrevPage_->getUsedSize() < getUsedSize())
	{
		// このページから持っていく
		ModSize m = getUsedSize() - half;

		// このページから持ってくるエントリを求める
		Iterator i = begin();
		Iterator e = end();
		ModUInt32* b = *i;
		for (; i < e; ++i)
		{
			if ((*i - b) >= static_cast<int>(m))
				break;
		}

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
		Iterator e = pPrevPage_->end();
		ModUInt32* b = *i;
		for (; i < e; ++i)
		{
			if ((*i - b) >= static_cast<int>(half))
				break;
		}

		// このページに移動する
		insert(begin(), i, pPrevPage_->end());
		// 前ページから移動した分を削除する
		pPrevPage_->expunge(i, pPrevPage_->end());
	}

	// 挿入するべきページを求める
	PagePointer pResult;
	if (pBuffer_)
	{
		const Compare& cCompare = Condition::getCompare(m_pTree);
		if (cCompare(pBuffer_, *begin()) < 0)
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
//	Array::DataPage::makeNodeEntry
//		-- ノードへ挿入するためのエントリを作成する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* pEntry_
//		このページの先頭のエントリ
//	ModSize& uiSize_
//		ノードへ挿入するためのエントリのサイズ
//
//	RETURN
//	ModUInt32*
//		ノードへ挿入するためのエントリ
//
//	EXCEPTIONS
//
AutoPointer<ModUInt32>
DataPage::makeNodeEntry(ModUInt32* pEntry_, ModSize& uiSize_)
{
	AutoPointer<ModUInt32> p = m_pTree->makeNodeEntry(pEntry_, uiSize_);
	m_pTree->setPageID(p, uiSize_, getID());
	return p;
}

//
//	Copyright (c) 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
