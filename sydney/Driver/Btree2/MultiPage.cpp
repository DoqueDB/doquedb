// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiPage.cpp --
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2009, 2014, 2015, 2023 Ricoh Company, Ltd.
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
#include "Btree2/MultiPage.h"
#include "Btree2/MultiFile.h"
#include "Btree2/Algorithm.h"
#include "Btree2/HeaderPage.h"
#include "Btree2/AutoPointer.h"
#include "Btree2/MessageAll_Class.h"

#include "Common/Assert.h"

#include "Os/Memory.h"

#include "Exception/UniquenessViolation.h"
#include "Exception/BadArgument.h"
#include "Exception/VerifyAborted.h"
#include "Exception/Cancel.h"

_SYDNEY_USING
_SYDNEY_BTREE2_USING

//
//	FUNCTION public
//	Btree2::MultiPage::MultiPage -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::MultiFile& cFile_
//		ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
MultiPage::MultiPage(MultiFile& cFile_)
	: DataPage(cFile_), m_cFile(cFile_)
{
}

//
//	FUNCTION public
//	Btree2::MultiPage::~MultiPage -- デストラクタ
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
MultiPage::~MultiPage()
{
}

//
//	FUNCTION public
//	Btree2::MultiPage::setPhysicalPage -- 物理ページを設定する
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
MultiPage::setPhysicalPage(PhysicalFile::Page* pPhysicalPage_)
{
	DataPage::setPhysicalPage(pPhysicalPage_);
	initialize();
}

//
//	FUNCTION public
//	Btree2::MultiPage::setPhysicalPage -- 物理ページを設定する
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
MultiPage::setPhysicalPage(PhysicalFile::Page* pPhysicalPage_,
						   PhysicalFile::PageID uiPrevPageID_,
						   PhysicalFile::PageID uiNextPageID_)
{
	DataPage::setPhysicalPage(pPhysicalPage_, uiPrevPageID_, uiNextPageID_);
	initialize();
}

//
//	FUNCTION public
//	Btree2::MultiPage::getUsedSize -- 使用サイズを得る
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
MultiPage::getUsedSize()
{
	return static_cast<ModSize>(*end() - *begin()) + getNullBitmapSize();
}

//
//	FUNCTION public
//	Btree2::MultiPage::lowerBound -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pValue_
//		比較対象データ
//	unsigned char nullBitmap_
//		nullビットマップ
//	const Btree2::Compare& cCompare_
//		比較クラス
//
//	RETURN
//	Btree2::MultiPage::Iterator
//		検索結果位置へのイテレータ
//
//	EXCEPTIONS
//
MultiPage::Iterator
MultiPage::lowerBound(const ModUInt32* pValue_, unsigned char nullBitmap_,
					  const Compare& cCompare_)
{
	return Algorithm::lowerBound(begin(), end(),
								 beginNullBitmap(), endNullBitmap(),
								 pValue_, nullBitmap_, cCompare_);
}

//
//	FUNCTION public
//	Btree2::MultiPage::upperBound -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pValue_
//		比較対象データ
//	unsigned char nullBitmap_
//		nullビットマップ
//	const Btree2::Compare& cCompare_
//		比較クラス
//
//	RETURN
//	Btree2::MultiPage::Iterator
//		検索結果位置へのイテレータ
//
//	EXCEPTIONS
//
MultiPage::Iterator
MultiPage::upperBound(const ModUInt32* pValue_, unsigned char nullBitmap_,
					  const Compare& cCompare_)
{
	return Algorithm::upperBound(begin(), end(),
								 beginNullBitmap(), endNullBitmap(),
								 pValue_, nullBitmap_, cCompare_);
}

//
//	FUNCTION public
//	Btree2::MultiPage::lowerBound -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pValue_
//		比較対象データ
//	unsigned char nullBitmap_
//		nullビットマップ
//	const Btree2::Compare& cCompare_
//		比較クラス
//
//	RETURN
//	Btree2::MultiPage::Iterator
//		検索結果位置へのイテレータ、見つからなかった場合はend()を返す
//
//	EXCEPTIONS
//
MultiPage::Iterator
MultiPage::find(const ModUInt32* pValue_, unsigned char nullBitmap_,
				const Compare& cCompare_)
{
	Iterator e = end();
	Iterator i = Algorithm::lowerBound(begin(), e,
									   beginNullBitmap(), endNullBitmap(),
									   pValue_, nullBitmap_, cCompare_);
	if (i != e)
	{
		int position = i - begin();
		NullBitmapVector::Iterator j = beginNullBitmap();
		j += position;
		if (cCompare_(*i, *j, pValue_, nullBitmap_) != 0)
			return e;
	}
	return i;
}

//
//	FUNCTION public
//	Btree2::MultiPage::exist -- 存在を確認する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pValue_
//		比較対象データ
//	unsigned char nullBitmap_
//		nullビットマップ
//
//	RETURN
//	bool
//		存在している場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
MultiPage::exist(const ModUInt32* pValue_, unsigned char nullBitmap_)
{
	const Compare& compare = getCompareClass();
	Iterator i = find(pValue_, nullBitmap_, compare);
	if (i == end())
		return false;
	return true;
}

//
//	FUNCTION public
//	Btree2::MultiPage::insertEntry -- エントリを挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pBuffer_
//		挿入するエントリ
//	unsigned char nullBitmap_
//		nullビットマップ
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
MultiPage::insertEntry(const ModUInt32* pBuffer_, unsigned char nullBitmap_,
					   ModSize uiSize_)
{
	bool result = false;
	
	if (uiSize_ == 0)
		uiSize_ = getDataClass().getSize(pBuffer_, nullBitmap_);

	ModSize expandSize = uiSize_ + getNullBitmapExpandSize(1);

	PagePointer pPage = this;
	
	if (getFreeSize() < expandSize)
	{
		// 空き領域がない -> ページ分割か再配布が必要

		pPage = expand(pBuffer_, nullBitmap_, uiSize_);

		result = true;
	}

	const Compare& compare = getCompareClass(nullBitmap_ ? false : true);
	Iterator i = pPage->upperBound(pBuffer_, nullBitmap_, compare);
	if (i != pPage->begin())
	{
		int position = i - pPage->begin();
		NullBitmapVector::Iterator j = pPage->beginNullBitmap();
		j += position;
		if (compare.integrityCheck(*(i-1), *(j-1), pBuffer_, nullBitmap_)
			== false)
		{
			_SYDNEY_THROW0(Exception::UniquenessViolation);
		}
	}
	pPage->insert(i, pBuffer_, nullBitmap_, uiSize_);

	return result;
}

//
//	FUNCTION public
//	Btree2::MultiPage::expungeEntry -- エントリを削除する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pBuffer_
//		挿入するエントリ
//	unsigned char nullBitmap_
//		nullビットマップ
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
MultiPage::expungeEntry(const ModUInt32* pBuffer_, unsigned char nullBitmap_,
						ModSize uiSize_, bool isReduce_)
{
	bool result = false;
	
	// 削除するエントリを探して消す
	const Compare& compare = getCompareClass();
	Iterator i = find(pBuffer_, nullBitmap_, compare);
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
			m_cFile.getHeaderPage()->initialize();
			m_cFile.getHeaderPage()->incrementCount();	// MultiFileでdecrement
														// されるので1にしとく

			// 自分を解放する
			m_cFile.freePage(this);

			result = true;
		}
		else if (getCount() == 1 && isLeaf() == false && isReduce_ == true)
		{
			// リーフじゃないので、自分を消して子をルートにする

			// 子のPageIDを得る
			PhysicalFile::PageID id
				= getDataClass().getPageID(*begin(),
										   *beginNullBitmap());
			// ルートページを設定する
			m_cFile.getHeaderPage()->setRootPageID(id);
			m_cFile.getHeaderPage()->decrementStepCount();
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
//	Btree2::MultiPage::verify -- 整合性を検査する
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
MultiPage::verify()
{
	if (isLeaf() == true)
	{
		if (getPrevPageID() == PhysicalFile::ConstValue::UndefinedPageID)
		{
			// 一番左のリーフページIDをチェック
			if (m_cFile.getHeaderPage()->getLeftLeafPageID() != getID())
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
			if (m_cFile.getHeaderPage()->getRightLeafPageID() != getID())
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
	NullBitmapVector::Iterator prevNullBitmap;
	Iterator i = begin();
	Iterator e = end();
	NullBitmapVector::Iterator j = beginNullBitmap();
	for (; i != e; ++i, ++j)
	{
		const Compare& compare = getCompareClass();
		const Data& data = getDataClass();
		if (&*prev)
		{
			// 大小関係をチェック
			if (compare(*prev, *prevNullBitmap, *i, *j) >= 0)
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
			PhysicalFile::PageID uiPageID = data.getPageID(*i, *j);
			PagePointer pPage = m_cFile.attachPage(uiPageID, PagePointer(this));

			// 先頭要素が同じかチェックする
			if (compare(*i, *j, *pPage->begin(), *pPage->beginNullBitmap())
				!= 0)
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
		prevNullBitmap = j;
	}
}

//
//	FUNCTION public
//	Btree2::MultiPage::detach -- デタッチする
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
MultiPage::detach()
{
	m_pParentPage = 0;
	Page::detach();
}

//
//	FUNCTION private
//	Btree2::MultiPage::initialize -- 初期化
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
MultiPage::initialize()
{
	ModUInt32 count = getCount();
	
	//nullビットマップ
	unsigned char* n
		= syd_reinterpret_cast<unsigned char*>(getBuffer() + getPageSize());
	m_cVector.assign(n, count);
	
	//データ
	if (m_vecpEntry.getCapacity() < count + 2)
		m_vecpEntry.reserve(count + 2);	// 1件挿入されてもreallocateしないように

	// ModVector::pushBackは遅いので、assignしてからポインタを書き換える
	m_vecpEntry.assign(count + 1, 0);

	load(count);
}

//
//	FUNCTION private
//	Btree2::MultiPage::load -- ベクターに値を設定する
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
MultiPage::load(ModUInt32 count_)
{
	;_SYDNEY_ASSERT(m_vecpEntry.getSize() == (count_ + 1));
	
	ModUInt32* p = getBuffer();
	const Data& dataClass = getDataClass();
	ModVector<ModUInt32*>::Iterator j = m_vecpEntry.begin();
	for (ModUInt32 i = 0; i < count_; ++i)
	{
		*j++ = p;
		p += dataClass.getSize(p, m_cVector[i]);
	}
	*j = p;

	;_SYDNEY_ASSERT(getPageSize() >= getUsedSize());
}

//
//	FUNCTION protected
//	Btree2::MultiPage::reload -- 物理ページが変更されたので読み直す
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
MultiPage::reload()
{
	ModUInt32 count = getCount();

	//nullビットマップ
	unsigned char* n
		= syd_reinterpret_cast<unsigned char*>(getBuffer() + getPageSize());
	m_cVector.assign(n, count);
	
	load(count);
}

//
//	FUNCTION public
//	Btree2::MultiPage::insert -- 指定位置に挿入する
//
//	NOTES
//
//	AGUMENTS
//	Btree2::MultiPage::Iterator i
//		挿入位置
//	const ModUInt32* pBuffer_
//		挿入するエントリ
//	unsigned char nullBitmap_
//		nullビットマップ
//	ModSize uiSize_
//		挿入するエントリのサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiPage::insert(Iterator i_, const ModUInt32* pBuffer_,
				  unsigned char nullBitmap_, ModSize uiSize_)
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
			unsigned char nullBitmap;
			AutoPointer<ModUInt32> p = makeNodeEntry(*begin(),
													 *beginNullBitmap(),
													 uiSize, nullBitmap);
			m_pParentPage->expungeEntry(p, nullBitmap, uiSize, false);
		}
	}

	if (isLeaf() == true
		&& getNextPageID() == PhysicalFile::ConstValue::UndefinedPageID
		&& i_ == end())
	{
		// 登録されているもののなかで最大のものを挿入した
		m_cFile.getHeaderPage()->incrementMaxValueCount();
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
	// nullビットマップを更新する
	int position = i_ - begin();
	NullBitmapVector::Iterator ite = beginNullBitmap() + position;
	m_cVector.insert(ite, nullBitmap_);
	// ベクターにも追加
	m_vecpEntry.insert(i_, first);
	// エントリを増やす
	addCount(1);

	if (nodeUpdate == true)
	{
		ModSize uiSize;
		unsigned char nullBitmap;
		AutoPointer<ModUInt32> p = makeNodeEntry(*begin(),
												 *beginNullBitmap(),
												 uiSize, nullBitmap);
		m_pParentPage->insertEntry(p, nullBitmap, uiSize);
	}
}

//
//	FUNCTION public
//	Btree2::MultiPage::insert -- 指定位置に挿入する
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::MultiPage::Iterator i_
//		挿入位置
//	Btree2::MultiPage::ConstIterator start_
//		挿入するエントリの先頭
//	Btree2::MultiPage::ConstIterator end_
//		挿入するエントリの終端
//	const Btree2::MultiPage::NullBitmapVector::Iterator& startBitmap_
//		挿入するnullビットマップの先頭
//	const Btree2::MultiPage::NullBitmapVector::Iterator& endBitmap_
//		挿入するnullビットマップの終端
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiPage::insert(Iterator i_, ConstIterator start_, ConstIterator end_,
				  const NullBitmapVector::Iterator& startBitmap_,
				  const NullBitmapVector::Iterator& endBitmap_)
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
			unsigned char nullBitmap;
			AutoPointer<ModUInt32> p = makeNodeEntry(*begin(),
													 *beginNullBitmap(),
													 uiSize, nullBitmap);
			m_pParentPage->expungeEntry(p, nullBitmap, uiSize, false);
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
	// nullビットマップを挿入する
	int position = i_ - begin();
	m_cVector.insert(beginNullBitmap() + position, startBitmap_, endBitmap_);
	// エントリを増やす
	addCount(end_ - start_);
	// ベクターを読み直す
	initialize();

	if (nodeUpdate == true)
	{
		ModSize uiSize;
		unsigned char nullBitmap;
		AutoPointer<ModUInt32> p = makeNodeEntry(*begin(),
												 *beginNullBitmap(),
												 uiSize, nullBitmap);
		m_pParentPage->insertEntry(p, nullBitmap, uiSize);
	}
}

//
//	FUNCTION public
//	Btree2::MultiPage::expunge -- 指定位置のものを削除する
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::MultiPage::Iterator i_
//		削除するエントリ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiPage::expunge(Iterator i_)
{
	updateMode();
	dirty();

	int position = i_ - begin();
	NullBitmapVector::Iterator j = beginNullBitmap() + position;

	bool nodeUpdate = false;
	if (i_ == begin() && m_pParentPage)
	{
		nodeUpdate = true;
		ModSize uiSize;
		unsigned char nullBitmap;
		AutoPointer<ModUInt32> p = makeNodeEntry(*begin(),
												 *beginNullBitmap(),
												 uiSize, nullBitmap);
		if (getCount() == 1)
			m_pParentPage->expungeEntry(p, nullBitmap, uiSize, true);
		else
			m_pParentPage->expungeEntry(p, nullBitmap, uiSize, false);
	}

	if (isLeaf() == true
		&& getNextPageID() == PhysicalFile::ConstValue::UndefinedPageID
		&& i_ == (end() - 1))
	{
		// 登録されているもののなかで最大のものを削除した
		m_cFile.getHeaderPage()->decrementMaxValueCount();
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
	// nullビットマップを更新する
	m_cVector.expunge(j);
	// エントリを減らす
	subtractCount(1);

	if (nodeUpdate == true && getCount() != 0)
	{
		ModSize uiSize;
		unsigned char nullBitmap;
		AutoPointer<ModUInt32> p = makeNodeEntry(*begin(),
												 *beginNullBitmap(),
												 uiSize, nullBitmap);
		m_pParentPage->insertEntry(p, nullBitmap, uiSize);
	}
}

//
//	FUNCTION public
//	Btree2::MultiPage::expunge -- 指定位置のものを削除する
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::MultiPage::Iterator start_
//		削除するエントリの先頭
//	Btree2::MultiPage::Iterator end_
//		削除するエントリの終端
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiPage::expunge(Iterator start_, Iterator end_)
{
	if (start_ != end_)
	{
		updateMode();
		dirty();

		int position = start_ - begin();
		NullBitmapVector::Iterator startBitmap = beginNullBitmap() + position;
		position = end_ - begin();
		NullBitmapVector::Iterator endBitmap = beginNullBitmap() + position;

		bool nodeUpdate = false;
		if (start_ == begin() && m_pParentPage)
		{
			nodeUpdate = true;
			ModSize uiSize;
			unsigned char nullBitmap;
			AutoPointer<ModUInt32> p = makeNodeEntry(*begin(),
													 *beginNullBitmap(),
													 uiSize, nullBitmap);
			if (end_ == end())
				m_pParentPage->expungeEntry(p, nullBitmap, uiSize, true);
			else
				m_pParentPage->expungeEntry(p, nullBitmap, uiSize, false);
		}

		// エントリを減らす
		subtractCount(end_ - start_);
		// nullビットマップを消す
		m_cVector.expunge(startBitmap, endBitmap);
		
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
			unsigned char nullBitmap;
			AutoPointer<ModUInt32> p = makeNodeEntry(*begin(),
													 *beginNullBitmap(),
													 uiSize,
													 nullBitmap);
			m_pParentPage->insertEntry(p, nullBitmap, uiSize);
		}
	}
}

//
//	FUNCTION private
//	Btree2::MultiPage::expand -- 拡張処理を行う
//
//	NOTES
//	ページ分割か再配分を行う
//
//	ARGUMENTS
//	const ModUInt32* pBuffer_
//		挿入するエントリ
//	unsigned char nullBitmap_
//		エントリのnullビットマップ
//	ModSize uiSize_
//		挿入するエントリのサイズ
//
//	RETURN
//	Btree2::MultiPage::PagePointer
//		挿入するべきページ
//
//	EXCEPTIONS
//
MultiPage::PagePointer
MultiPage::expand(const ModUInt32* pBuffer_, unsigned char nullBitmap_,
				  ModSize uiSize_)
{
	PagePointer pResult;
	const Compare& compare = getCompareClass();
	
	if (getNextPageID() == PhysicalFile::ConstValue::UndefinedPageID
		&& compare(*(end() - 1), *(endNullBitmap() - 1),
				   pBuffer_, nullBitmap_) < 0
		&& m_cFile.getHeaderPage()->getSplitRatio() > 50)
	{
		if (isRoot() == true)
		{
			// あたらしくルートノードを作成する
			PagePointer pRoot
				= m_cFile.allocatePage(
					PhysicalFile::ConstValue::UndefinedPageID,
					PhysicalFile::ConstValue::UndefinedPageID,
					PagePointer());

			// ルートノードを設定する
			m_cFile.getHeaderPage()->setRootPageID(pRoot->getID());
			m_cFile.getHeaderPage()->incrementStepCount();
			// 自分の親を設定する
			m_pParentPage = pRoot;

			// ルートノードに先頭の要素を加える
			ModSize size;
			unsigned char nullBitmap;
			AutoPointer<ModUInt32> entry = makeNodeEntry(*begin(),
														 *beginNullBitmap(),
														 size,
														 nullBitmap);
			pRoot->insertEntry(entry, nullBitmap, size);
		}
		
		// 自分が右端のページでかつ挿入するエントリが最大だったら
		// 分割比率に基づいて処理をする

		// 新しいページを確保する
		PagePointer pNewPage = m_cFile.allocatePage(
			getID(),
			PhysicalFile::ConstValue::UndefinedPageID,
			m_pParentPage);
		// このページの次ページを新しいページにする
		setNextPageID(pNewPage->getID());
		if (isLeaf() == true)
		{
			pNewPage->setLeaf();
			m_cFile.getHeaderPage()->setRightLeafPageID(pNewPage->getID());
		}

		ModSize ratio = static_cast<ModSize>(
			m_cFile.getHeaderPage()->getSplitRatio());
		
		ModSize pageSize = getPageSize() - getNullBitmapSize();
		Iterator i = begin();
		NullBitmapVector::Iterator j = beginNullBitmap();
		if (ratio < 100)
		{
			// 移動する
			ModUInt32* b = *i;
			Iterator e = end();
			for (; i != e; ++i, ++j)
			{
				if ((*i - b) >= static_cast<int>((pageSize * ratio / 100)))
					break;
			}
		}
		else
		{
			i = end();
			j = endNullBitmap();
			// 1つは新しいページに移動しないといけない
			--i;
			--j;
		}

		pNewPage->insert(pNewPage->begin(), i, end(), j, endNullBitmap());
		expunge(i, end());
		pResult = pNewPage;
	}
	else
	{
		// 通常のsplit or redistribute

		PagePointer pPrev;
		PagePointer pNext;
		ModSize freeSize = 0;
		
		if (getPrevPageID() != PhysicalFile::ConstValue::UndefinedPageID)
		{
			// 前ページを得る
			pPrev = m_cFile.attachPage(getPrevPageID(), PagePointer(0));
			freeSize = pPrev->getFreeSize();
			pNext = this;
		}
		else if (getNextPageID() != PhysicalFile::ConstValue::UndefinedPageID)
		{
			// 次ページを得る
			//	前がないので、親はこのページと同じはず
			pPrev = this;
			pNext = m_cFile.attachPage(getNextPageID(), m_pParentPage);
			freeSize = pNext->getFreeSize();
		}
		else
		{
			// 前も後ろもない
			// 新しくルートノードを作成する
			PagePointer pRoot
				= m_cFile.allocatePage(
					PhysicalFile::ConstValue::UndefinedPageID,
					PhysicalFile::ConstValue::UndefinedPageID,
					PagePointer(0));

			// ルートノードを設定する
			m_cFile.getHeaderPage()->setRootPageID(pRoot->getID());
			m_cFile.getHeaderPage()->incrementStepCount();
			// 自分の親を設定する
			m_pParentPage = pRoot;

			// ルートノードに先頭の要素を加える
			ModSize size;
			unsigned char nullBitmap;
			AutoPointer<ModUInt32> entry = makeNodeEntry(*begin(),
														 *beginNullBitmap(),
														 size,
														 nullBitmap);
			pRoot->insertEntry(entry, nullBitmap, size);

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
				m_cFile.getHeaderPage()->setRightLeafPageID(pNext->getID());
			}

			pPrev = this;
			freeSize = pNext->getFreeSize();
		}

		if (freeSize < (getPageSize() / 10))
		{
			// 前後どちらかの空き容量が10%未満なので、ページ分割
			pResult = pNext->split(pPrev, pBuffer_, nullBitmap_);
		}
		else
		{
			// それ以外なので、再配分
			pResult = pNext->redistribute(pPrev, pBuffer_, nullBitmap_);
		}
	}

	return pResult;
}

//
//	FUNCTION private
//	Btree2::MultiPage::reduce -- 縮小処理を行う
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
MultiPage::reduce()
{
	PagePointer pPrev;
	PagePointer pNext;
	ModSize freeSize = 0;

	if (getPrevPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 前ページを得る
		pPrev = m_cFile.attachPage(getPrevPageID(), PagePointer(0));
		freeSize = pPrev->getFreeSize();
		pNext = this;
	}
	else if (getNextPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 次ページを得る
		//	前がないので、親はこのページと同じはず
		pPrev = this;
		pNext = m_cFile.attachPage(getNextPageID(), m_pParentPage);
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
//	Btree2::MultiPage::split -- ページ分割する
//
//	NOTES
//	自分とpPrevPage_の間に新しいページを作成する
//
//	ARGUMENTS
//	Btree2::MultiPage::PagePointer pPrevPage_
//		前のページ
//	const ModUInt32* pBuffer_
//		挿入するエントリ
//	unsigned char nullBitmap_
//		エントリのnullビットマップ
//
//	RETURN
//	Btree2::MultiPage::PagePointer
//		挿入するべきページ
//
//	EXCEPTIONS
//
MultiPage::PagePointer
MultiPage::split(PagePointer pPrevPage_,
				 const ModUInt32* pBuffer_, unsigned char nullBitmap_)
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

	// ページサイズ
	ModSize totalUnit = getPageSize();
	totalUnit -= (getNullBitmapSize() * 2 / 3);

	//
	//	まずは前ページの終わり1/3を新しいページに移動する
	//
	
	// 前ページに残すエントリを計算する
	Iterator i = pPrevPage_->begin();
	Iterator e = pPrevPage_->end();
	NullBitmapVector::Iterator j = pPrevPage_->beginNullBitmap();
	ModUInt32* b = *i;
	for (; i < e; ++i, ++j)
	{
		if ((*i - b) >= static_cast<int>(totalUnit * 2 / 3))
			// 2/3を超えたので終わり
			break;
	}
	// 新しいページに移動する
	pNewPage->insert(pNewPage->begin(), i, pPrevPage_->end(),
					 j, pPrevPage_->endNullBitmap());
	// 前ページから削除する
	pPrevPage_->expunge(i, pPrevPage_->end());

	//
	//	次にこのページの先頭1/3を新しいページに移動する
	//

	// このページに残すエントリを計算する
	i = begin();
	e = end();
	j = beginNullBitmap();
	b = *i;
	for (; i < e; ++i, ++j)
	{
		if ((*i - b) >= static_cast<int>(totalUnit * 1 / 3))
			// 1/3を超えたので終わり
			break;
	}
	// 新しいページに移動する
	pNewPage->insert(pNewPage->end(), begin(), i, beginNullBitmap(), j);
	// このページから削除する(親が変わっている可能性がある)
	setParentPage(m_cFile.findPage(*begin(), *beginNullBitmap(), getID()));
	expunge(begin(), i);

	//
	//	どのページに入れるべきか
	//

	PagePointer pResult;
	
	const Compare& compare = getCompareClass();
	if (compare(pBuffer_, nullBitmap_,
				*pNewPage->begin(), *pNewPage->beginNullBitmap()) < 0)
	{
		// 前ページ
		pResult = pPrevPage_;
	}
	else if (compare(pBuffer_, nullBitmap_, *begin(), *beginNullBitmap()) < 0)
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
//	Btree2::MultiPage::concatenate -- ページ連結
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::MultiPage::PagePointer pPrevPage_
//		前のページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MultiPage::concatenate(PagePointer pPrevPage_)
{
	if (getNextPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 次のページの前ページを更新する
		PagePointer p = m_cFile.attachPage(getNextPageID(),
										   PagePointer());
		p->setPrevPageID(pPrevPage_->getID());
	}
	else if (isLeaf() == true)
	{
		// 一番右のリーフページを更新する
		m_cFile.getHeaderPage()->setRightLeafPageID(pPrevPage_->getID());
	}
	// 前ページの次ページを更新する
	pPrevPage_->setNextPageID(getNextPageID());

	// このページの内容を移動する
	pPrevPage_->insert(pPrevPage_->end(), begin(), end(),
					   beginNullBitmap(), endNullBitmap());
	// 移動した分を削除する
	expunge(begin(), end());

	// このページを削除
	m_cFile.freePage(this);
}

//
//	FUNCTION private
//	Btree2::MultiPage::redistribute -- 再配分
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::MultiPage::PagePointer pPrevPage_
//		前のページ
//	const ModUInt32* pBuffer_
//		挿入するエントリ (default 0)
//	unsigned char nullBitmap_
//		エントリのビットマップ
//
//	RETURN
//	PagePointer
//		エントリを挿入するべきページ
//
//	EXCEPTIONS
//
MultiPage::PagePointer
MultiPage::redistribute(PagePointer pPrevPage_,
						const ModUInt32* pBuffer_, unsigned char nullBitmap_)
{
	// 半分のユニット数を求める
	ModSize half = (pPrevPage_->getUsedSize() + getUsedSize()) / 2;
	
	if (pPrevPage_->getUsedSize() < getUsedSize())
	{
		// このページから持っていく
		ModSize m = getUsedSize() - half;

		// このページから持っていくエントリを求める
		Iterator i = begin();
		Iterator e = end();
		NullBitmapVector::Iterator j = beginNullBitmap();
		ModUInt32* b = *i;
		for (; i < e; ++i, ++j)
		{
			if ((*i - b) + (j - beginNullBitmap()) / 4 + 1
				>= static_cast<int>(m))
				break;
		}

		// 前ページに移動する
		pPrevPage_->insert(pPrevPage_->end(), begin(), i, beginNullBitmap(), j);
		// このページから移動した分を削除する
		expunge(begin(), i);
	}
	else
	{
		// 前ページから持ってくる

		// 前ページに残すエントリを求める
		Iterator i = pPrevPage_->begin();
		Iterator e = pPrevPage_->end();
		NullBitmapVector::Iterator j = pPrevPage_->beginNullBitmap();
		ModUInt32* b = *i;
		for (; i < e; ++i, ++j)
		{
			if ((*i - b) + (j - pPrevPage_->beginNullBitmap()) / 4 + 1
				>= static_cast<int>(half))
				break;
		}

		// このページに移動する
		insert(begin(), i, pPrevPage_->end(), j, pPrevPage_->endNullBitmap());
		// 前ページから移動した分を削除する
		pPrevPage_->expunge(i, pPrevPage_->end());
	}

	// 挿入するべきページを求める
	PagePointer pResult;
	if (pBuffer_)
	{
		const Compare& compare = getCompareClass();
		if (compare(pBuffer_, nullBitmap_, *begin(), *beginNullBitmap()) < 0)
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
//	Btree2::MultiPage::makeNodeEntry
//		-- ノードへ挿入するためのエントリを作成する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* pEntry_
//		このページの先頭のエントリ
//	unsigned char nullBitmap_
//		エントリのnullビットマップ
//	ModSize& uiSize_
//		ノードへ挿入するためのエントリのサイズ
//	unsigned char nullBitmap2_
//		ノードへ挿入するためのエントリのnullビットマップ
//
//	RETURN
//	ModUInt32*
//		ノードへ挿入するためのエントリ
//
//	EXCEPTIONS
//
AutoPointer<ModUInt32>
MultiPage::makeNodeEntry(ModUInt32* pEntry_, unsigned char nullBitmap_,
						 ModSize& uiSize_, unsigned char& nullBitmap2_)
{
	// キー部分の大きさを得る
	ModSize size = getKeyDataClass().getSize(pEntry_, nullBitmap_);
	// ページID分を加える
	uiSize_ = size + Data::UnsignedInteger::getSize(0);

	AutoPointer<ModUInt32> p
		= syd_reinterpret_cast<ModUInt32*>(
			Os::Memory::allocate(uiSize_ * sizeof(ModUInt32)));
	
	// キー部分をコピーする
	Os::Memory::copy(p, pEntry_, size * sizeof(ModUInt32));
	
	// ページIDを設定する
	ModUInt32* pageid = p.get() + size;
	syd_reinterpret_cast<Data::UnsignedInteger*>(pageid)->m_value = getID();

	// nullビットマップ
	unsigned char bit = 1;
	bit <<= getKeyDataClass().getTypeCount();
	nullBitmap2_ = nullBitmap_ & ~bit;
	
	return p;
}

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2009, 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
