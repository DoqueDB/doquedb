// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreePage.cpp --
// 
// Copyright (c) 2005, 2007, 2009, 2023 Ricoh Company, Ltd.
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
#include "Bitmap/BtreePage.h"
#include "Bitmap/BtreeFile.h"
#include "Bitmap/FileID.h"
#include "Bitmap/Algorithm.h"
#include "Bitmap/HeaderPage.h"
#include "Bitmap/MessageAll_Class.h"

#include "Common/Assert.h"

#include "Os/Memory.h"

#include "Exception/UniquenessViolation.h"
#include "Exception/BadArgument.h"
#include "Exception/VerifyAborted.h"
#include "Exception/Cancel.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

namespace
{
	//
	//	VARIABLE local
	//
	ModUInt32 _LEAF_MASK =	0x80000000;
}

//
//	FUNCTION public
//	Bitmap::BtreePage::BtreePage -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::BtreeFile& cFile_
//		ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
BtreePage::BtreePage(BtreeFile& cFile_)
	: NodePage(cFile_), m_cFile(cFile_), m_pHeader(0), m_iReference(0)
{
}

//
//	FUNCTION public
//	Bitmap::BtreePage::~BtreePage -- デストラクタ
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
//	Bitmap::BtreePage::setPhysicalPage -- 物理ページを設定する
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
BtreePage::setPhysicalPage(PhysicalFile::Page* pPhysicalPage_)
{
	Page::setPhysicalPage(pPhysicalPage_);
	m_pHeader = syd_reinterpret_cast<Header*>(Page::getBuffer());
	initialize();
}

//
//	FUNCTION public
//	Bitmap::BtreePage::setPhysicalPage -- 物理ページを設定する
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
BtreePage::setPhysicalPage(PhysicalFile::Page* pPhysicalPage_,
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
//	Btree2::BtreePage::getPageSize -- ページサイズを得る(UNIT単位)
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
BtreePage::getPageSize()
{
	return Page::getPageSize() - sizeof(Header)/sizeof(ModUInt32);
}

//
//	FUNCTION public
//	Bitmap::BtreePage::getUsedSize -- 使用サイズを得る
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
BtreePage::getUsedSize()
{
	return static_cast<ModSize>(*end() - *begin());
}

//
//	FUNCTION public
//	BtreePage::getFreeSize -- 空きサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		空きユニット数
//
//	EXCEPTIONS
//	なし
//
ModSize
BtreePage::getFreeSize()
{
	return getPageSize() - getUsedSize();
}

//
//	FUNCTION public
//	Bitmap::BtreePage::lowerBound -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pValue_
//		比較対象データ
//	const Bitmap::Compare& cCompare_
//		比較クラス
//
//	RETURN
//	Bitmap::BtreePage::Iterator
//		検索結果位置へのイテレータ
//
//	EXCEPTIONS
//
BtreePage::Iterator
BtreePage::lowerBound(const ModUInt32* pValue_, const Compare& cCompare_)
{
	return Algorithm::lowerBound(begin(), end(), pValue_, cCompare_);
}

//
//	FUNCTION public
//	Bitmap::BtreePage::upperBound -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pValue_
//		比較対象データ
//	const Bitmap::Compare& cCompare_
//		比較クラス
//
//	RETURN
//	Bitmap::BtreePage::Iterator
//		検索結果位置へのイテレータ
//
//	EXCEPTIONS
//
BtreePage::Iterator
BtreePage::upperBound(const ModUInt32* pValue_, const Compare& cCompare_)
{
	return Algorithm::upperBound(begin(), end(), pValue_, cCompare_);
}

//
//	FUNCTION public
//	Bitmap::BtreePage::lowerBound -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pValue_
//		比較対象データ
//	const Bitmap::Compare& cCompare_
//		比較クラス
//
//	RETURN
//	Bitmap::BtreePage::Iterator
//		検索結果位置へのイテレータ、見つからなかった場合はend()を返す
//
//	EXCEPTIONS
//
BtreePage::Iterator
BtreePage::find(const ModUInt32* pValue_, const Compare& cCompare_)
{
	Iterator i = Algorithm::lowerBound(begin(), end(), pValue_, cCompare_);
	if (i == end() || cCompare_(*i, pValue_) != 0)
		return end();
	return i;
}

//
//	FUNCTION public
//	Bitmap::BtreePage::insertEntry -- エントリを挿入する
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
BtreePage::insertEntry(const ModUInt32* pBuffer_, ModSize uiSize_)
{
	bool result = false;

	const Data& dataClass = getDataClass();
	
	if (uiSize_ == 0)
		uiSize_ = dataClass.getSize(pBuffer_);

	if (dataClass.getFixedSize() != 0 && uiSize_ != dataClass.getFixedSize())
	{
		// スキーマ情報と入力値の大きさがあっていない(通常ありえない)
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	PagePointer pPage = this;
	
	if (getFreeSize() < uiSize_)
	{
		// 空き領域がない -> ページ分割か再配布が必要

		pPage = expand(pBuffer_, uiSize_);

		result = true;
	}

	const Compare& compare = m_cFile.getCompare();
	Iterator i = pPage->upperBound(pBuffer_, compare);
	if (i != pPage->begin())
	{
		if (compare(*(i-1), pBuffer_) == 0)
		{
			_SYDNEY_THROW0(Exception::UniquenessViolation);
		}
	}
	pPage->insert(i, pBuffer_, uiSize_);

	return result;
}

//
//	FUNCTION public
//	Bitmap::BtreePage::expungeEntry -- エントリを削除する
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
BtreePage::expungeEntry(const ModUInt32* pBuffer_, ModSize uiSize_,
						 bool isReduce_)
{
	bool result = false;
	
	// 削除するエントリを探して消す
	const Compare& compare = m_cFile.getCompare();
	Iterator i = find(pBuffer_, compare);
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
			
			// 以前は全体を初期化していたが、B木部分だけを初期化するようにした。
			// getCountは非nullの数を返すが、
			// HeaderPageは、nullを格納するエリアのIDも保持しているため。
			m_cFile.getHeaderPage().setRootPageID(
				PhysicalFile::ConstValue::UndefinedPageID);
			m_cFile.getHeaderPage().setLeftLeafPageID(
				PhysicalFile::ConstValue::UndefinedPageID);
			m_cFile.getHeaderPage().setRightLeafPageID(
				PhysicalFile::ConstValue::UndefinedPageID);

			// 自分を解放する
			m_cFile.freePage(this);

			result = true;
		}
		else if (getCount() == 1 && isLeaf() == false && isReduce_ == true)
		{
			// リーフじゃないので、自分を消して子をルートにする

			// 子のPageIDを得る
			PhysicalFile::PageID id = getDataClass().getPageID(*begin());
			// ルートページを設定する
			m_cFile.getHeaderPage().setRootPageID(id);
			m_cFile.getHeaderPage().decrementStepCount();
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
//	Bitmap::BtreePage::updateEntry -- エントリを更新する
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
//		つねにfalse
//
//	EXCEPTIONS
//
bool
BtreePage::updateEntry(const ModUInt32* pBuffer_, ModSize uiSize_)
{
	//	【注意】
	//	キーに変更はなく、バリューのみ変更されることを想定している
	//	それ以外のパターンでは、B木のページが壊れる
	
	bool result = false;
	
	const Data& dataClass = getDataClass();
	
	if (uiSize_ == 0)
		uiSize_ = dataClass.getSize(pBuffer_);

	// 削除するエントリを探して消す
	const Compare& compare = m_cFile.getCompare();
	Iterator i = find(pBuffer_, compare);
	if (i == end())
	{
		// 見つからなかった
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// コピーする
	dirty();
	Os::Memory::copy(*i, pBuffer_, uiSize_ * sizeof(ModUInt32));

	return result;
}

//
//	FUNCTION public
//	Bitmap::BtreePage::verify -- 整合性を検査する
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
	if (isLeaf() == true)
	{
		if (getPrevPageID() == PhysicalFile::ConstValue::UndefinedPageID)
		{
			// 一番左のリーフページIDをチェック
			if (m_cFile.getHeaderPage().getLeftLeafPageID() != getID())
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
			if (m_cFile.getHeaderPage().getRightLeafPageID() != getID())
			{
				_SYDNEY_VERIFY_INCONSISTENT(m_cFile.getProgress(),
											m_cFile.getPath(),
											Message::IllegalRightLeafPageID());
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
		}
	}
	
	Iterator prev = 0;
	Iterator i = begin();
	for (; i != end(); ++i)
	{
		const Compare& compare = m_cFile.getCompare();
		const Data& data = getDataClass();
		if (&*prev)
		{
			// 大小関係をチェック
			if (compare(*prev, *i) >= 0)
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
			PhysicalFile::PageID uiPageID = data.getPageID(*i);
			PagePointer pPage = m_cFile.attachPage(uiPageID, PagePointer(this));

			// 先頭要素が同じかチェックする
			if (compare(*i, *pPage->begin()) != 0)
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
//	Bitmap::BtreePage::isRoot -- ルートページか
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
BtreePage::isRoot() const
{
	return (m_pHeader->m_uiPrevPageID
			== PhysicalFile::ConstValue::UndefinedPageID
			&& m_pHeader->m_uiNextPageID
			== PhysicalFile::ConstValue::UndefinedPageID);
}

//
//	FUNCTION public
//	Bitmap::BtreePage::isLeaf -- リーフページか
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
BtreePage::isLeaf() const
{
	return (m_pHeader->m_uiCount & _LEAF_MASK);
}

//
//	FUNCTION public
//	Bitmap::BtreePage::setLeaf -- リーフページに設定する
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
	m_pHeader->m_uiCount |= _LEAF_MASK;
}

//
//	FUNCTION public
//	Bitmap::BtreePage::detach -- デタッチする
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
	m_pParentPage = 0;
	if (m_bFree)
	{
		detachPhysicalPage();
		delete this;
	}
}

//
//	FUNCTION public
//	Bitmap::BtreePage::detachPhysicalPage -- 物理ファイルをdetachする
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
BtreePage::detachPhysicalPage()
{
	Page::detach();
}

//
//	FUNCTION public
//	Bitmap::BtreePage::getCount -- ページのエントリ数を得る
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
BtreePage::getCount()
{
	return m_pHeader->m_uiCount & ~_LEAF_MASK;
}

//
//	FUNCTION protected
// 	Bitmap::BtreePage::getBuffer -- データ領域へのポインタを得る
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
BtreePage::getBuffer()
{
	return Page::getBuffer() + sizeof(Header)/sizeof(ModUInt32);
}

//
//	FUNCTION protected
//	Bitmap::BtreePage::addCount -- ページのエントリ数を増やす
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
BtreePage::addCount(ModUInt32 uiCount_)
{
	dirty();
	m_pHeader->m_uiCount += uiCount_;
}

//
//	FUNCTION protected
// 	Bitmap::BtreePage::subtractCount -- ページのエントリ数を減らす
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
BtreePage::subtractCount(ModUInt32 uiCount_)
{
	dirty();
	m_pHeader->m_uiCount -= uiCount_;
}

//
//	FUNCTION private
//	Bitmap::BtreePage::initialize -- 初期化
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
BtreePage::initialize()
{
	ModUInt32 count = getCount();
	
	if (m_vecpEntry.getCapacity() < count + 2)
		m_vecpEntry.reserve(count + 2);	// 1件挿入されてもreallocateしないように

	m_vecpEntry.assign(count + 1, 0);

	load(count);
}

//
//	FUNCTION private
//	Bitmap::BtreePage::load -- ベクターに値を設定する
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
BtreePage::load(ModUInt32 count_)
{
	;_SYDNEY_ASSERT(m_vecpEntry.getSize() == (count_ + 1));
	
	ModUInt32* p = getBuffer();
	const Data& dataClass = getDataClass();
	ModSize size = dataClass.getFixedSize();
	ModVector<ModUInt32*>::Iterator j = m_vecpEntry.begin();
	if (size)
	{
		for (ModUInt32 i = 0; i < count_; ++i)
		{
			*j++ = p;
			p += size;
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
//	Bitmap::BtreePage::reload -- 物理ページが変更されたので読み直す
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
BtreePage::reload()
{
	m_pHeader = syd_reinterpret_cast<Header*>(Page::getBuffer());
	ModUInt32 count = getCount();
	load(count);
}

//
//	FUNCTION public
//	Bitmap::BtreePage::insert -- 指定位置に挿入する
//
//	NOTES
//
//	AGUMENTS
//	Bitmap::BtreePage::Iterator i
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
BtreePage::insert(Iterator i_, const ModUInt32* pBuffer_, ModSize uiSize_)
{
	dirty();

	bool nodeUpdate = false;
	if (i_ == begin() && m_pParentPage)
	{
		nodeUpdate = true;
		if (getCount() != 0)
		{
			ModSize uiSize;
			ModUInt32 p[FileID::MAX_SIZE];
			makeNodeEntry(*begin(), p, uiSize);
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
	for (; i <= end(); ++i)
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
		ModUInt32 p[FileID::MAX_SIZE];
		makeNodeEntry(*begin(), p, uiSize);
		m_pParentPage->insertEntry(p, uiSize);
	}
}

//
//	FUNCTION public
//	Bitmap::BtreePage::insert -- 指定位置に挿入する
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::BtreePage::Iterator i_
//		挿入位置
//	Bitmap::BtreePage::ConstIterator start_
//		挿入するエントリの先頭
//	Bitmap::BtreePage::ConstIterator end_
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
	dirty();

	bool nodeUpdate = false;
	if (i_ == begin() && m_pParentPage)
	{
		nodeUpdate = true;
		if (getCount() != 0)
		{
			ModSize uiSize;
			ModUInt32 p[FileID::MAX_SIZE];
			makeNodeEntry(*begin(), p, uiSize);
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
		ModUInt32 p[FileID::MAX_SIZE];
		makeNodeEntry(*begin(), p, uiSize);
		m_pParentPage->insertEntry(p, uiSize);
	}
}

//
//	FUNCTION public
//	Bitmap::BtreePage::expunge -- 指定位置のものを削除する
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::BtreePage::Iterator i_
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
	dirty();

	bool nodeUpdate = false;
	if (i_ == begin() && m_pParentPage)
	{
		nodeUpdate = true;
		ModSize uiSize;
		ModUInt32 p[FileID::MAX_SIZE];
		makeNodeEntry(*begin(), p, uiSize);
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
	for (; i <= end(); ++i)
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
		ModUInt32 p[FileID::MAX_SIZE];
		makeNodeEntry(*begin(), p, uiSize);
		m_pParentPage->insertEntry(p, uiSize);
	}
}

//
//	FUNCTION public
//	Bitmap::BtreePage::expunge -- 指定位置のものを削除する
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::BtreePage::Iterator start_
//		削除するエントリの先頭
//	Bitmap::BtreePage::Iterator end_
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
		dirty();

		bool nodeUpdate = false;
		if (start_ == begin() && m_pParentPage)
		{
			nodeUpdate = true;
			ModSize uiSize;
			ModUInt32 p[FileID::MAX_SIZE];
			makeNodeEntry(*begin(), p, uiSize);
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
			ModUInt32 p[FileID::MAX_SIZE];
			makeNodeEntry(*begin(), p, uiSize);
			m_pParentPage->insertEntry(p, uiSize);
		}
	}
}

//
//	FUNCTION private
//	Bitmap::BtreePage::expand -- 拡張処理を行う
//
//	NOTES
//	ページ分割か再配分を行う
//
//	ARGUMENTS
//	const ModUInt32* pBuffer_
//		挿入するエントリ
//	ModSize uiSize_
//		挿入するエントリのサイズ
//
//	RETURN
//	Bitmap::BtreePage::PagePointer
//		挿入するべきページ
//
//	EXCEPTIONS
//
BtreePage::PagePointer
BtreePage::expand(const ModUInt32* pBuffer_, ModSize uiSize_)
{
	PagePointer pResult;
	const Compare& compare = m_cFile.getCompare();
	
	PagePointer pPrev;
	PagePointer pNext;
	ModSize freeSize = 0;
		
	if (getPrevPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 前ページを得る
		pPrev = m_cFile.attachPage(getPrevPageID(), PagePointer());
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
				PagePointer());

		// ルートノードを設定する
		m_cFile.getHeaderPage().setRootPageID(pRoot->getID());
		m_cFile.getHeaderPage().incrementStepCount();
		// 自分の親を設定する
		m_pParentPage = pRoot;

		// ルートノードに先頭の要素を加える
		ModSize size;
		ModUInt32 p[FileID::MAX_SIZE];
		makeNodeEntry(*begin(), p, size);
		pRoot->insertEntry(p, size);

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
			m_cFile.getHeaderPage().setRightLeafPageID(pNext->getID());
		}

		pPrev = this;
		freeSize = pNext->getFreeSize();
	}

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
//	Bitmap::BtreePage::reduce -- 縮小処理を行う
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
	ModSize freeSize = 0;

	if (getPrevPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 前ページを得る
		pPrev = m_cFile.attachPage(getPrevPageID(), PagePointer());
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
//	Bitmap::BtreePage::split -- ページ分割する
//
//	NOTES
//	自分とpPrevPage_の間に新しいページを作成する
//
//	ARGUMENTS
//	Bitmap::BtreePage::PagePointer pPrevPage_
//		前のページ
//	const ModUInt32* pBuffer_
//		挿入するエントリ
//
//	RETURN
//	Bitmap::BtreePage::PagePointer
//		挿入するべきページ
//
//	EXCEPTIONS
//
BtreePage::PagePointer
BtreePage::split(PagePointer pPrevPage_, const ModUInt32* pBuffer_)
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

	//
	//	まずは前ページの終わり1/3を新しいページに移動する
	//
	
	// 前ページに残すエントリを計算する
	Iterator i = pPrevPage_->begin();
	ModUInt32* b = *i;
	for (; i < pPrevPage_->end(); ++i)
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
	b = *i;
	for (; i < end(); ++i)
	{
		if ((*i - b) >= static_cast<int>(totalUnit * 1 / 3))
			// 1/3を超えたので終わり
			break;
	}
	// 新しいページに移動する
	pNewPage->insert(pNewPage->end(), begin(), i);
	// このページから削除する(親が変わっている可能性がある)
	setParentPage(m_cFile.findPage(*begin(), getID()));
	expunge(begin(), i);

	//
	//	どのページに入れるべきか
	//

	PagePointer pResult;
	
	const Compare& compare = m_cFile.getCompare();
	if (compare(pBuffer_, *(pNewPage->begin())) < 0)
	{
		// 前ページ
		pResult = pPrevPage_;
	}
	else if (compare(pBuffer_, *begin()) < 0)
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
//	Bitmap::BtreePage::concatenate -- ページ連結
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::BtreePage::PagePointer pPrevPage_
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
										   PagePointer());
		p->setPrevPageID(pPrevPage_->getID());
	}
	else if (isLeaf() == true)
	{
		// 一番右のリーフページを更新する
		m_cFile.getHeaderPage().setRightLeafPageID(pPrevPage_->getID());
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
//	Bitmap::BtreePage::redistribute -- 再配分
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::BtreePage::PagePointer pPrevPage_
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
BtreePage::PagePointer
BtreePage::redistribute(PagePointer pPrevPage_, const ModUInt32* pBuffer_)
{
	// 半分のユニット数を求める
	ModSize half = (pPrevPage_->getUsedSize() + getUsedSize()) / 2;
	
	if (pPrevPage_->getUsedSize() < getUsedSize())
	{
		// このページから持っていく
		ModSize m = getUsedSize() - half;

		// このページから持ってくるエントリを求める
		Iterator i = begin();
		ModUInt32* b = *i;
		for (; i < end(); ++i)
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
		ModUInt32* b = *i;
		for (; i < pPrevPage_->end(); ++i)
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
		const Compare& compare = m_cFile.getCompare();
		if (compare(pBuffer_, *begin()) < 0)
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
//	Bitmap::BtreePage::makeNodeEntry
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
void
BtreePage::makeNodeEntry(ModUInt32* pEntry_,
						 ModUInt32* pBuffer_, ModSize& uiSize_)
{
	// 大きさを得る
	uiSize_ = m_cFile.getNodeData().getSize(pEntry_);
	// キー部分のみの大きさ
	ModSize size = uiSize_ - Data::UnsignedInteger::getSize(0);
	
	// キー部分をコピーする
	Os::Memory::copy(pBuffer_, pEntry_, size * sizeof(ModUInt32));
	
	// ページIDを設定する
	ModUInt32* pageid = pBuffer_ + size;
	syd_reinterpret_cast<Data::UnsignedInteger*>(pageid)->m_value = getID();
}

//
//	FUNCTION protected
//	Bitmap::DataPage::getDataClass -- データクラスを得る
//
// 	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Bitmap::Data&
//		 データクラス
//
//	EXCEPTIONS
//
const Data&
BtreePage::getDataClass()
{
	return (isLeaf() == true) ? m_cFile.getLeafData() : m_cFile.getNodeData();
}

//
//	Copyright (c) 2005, 2007, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
