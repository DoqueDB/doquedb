// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UniquePage.cpp --
// 
// Copyright (c) 2014, 2015, 2018, 2023 Ricoh Company, Ltd.
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
#include "Btree2/UniquePage.h"
#include "Btree2/UniqueFile.h"
#include "Btree2/Algorithm.h"
#include "Btree2/HeaderPage.h"
#include "Btree2/AutoPointer.h"
#include "Btree2/MessageAll_Class.h"

#include "Common/Assert.h"

#include "Lock/Name.h"

#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"
#include "Os/Memory.h"

#include "Trans/Transaction.h"

#include "Exception/UniquenessViolation.h"
#include "Exception/BadArgument.h"
#include "Exception/VerifyAborted.h"
#include "Exception/Cancel.h"

_SYDNEY_USING
_SYDNEY_BTREE2_USING

namespace {
	//
	//	VARIABLE local
	//	_$$::_cLockLatch -- 行ロックを排他するラッチ

	Os::CriticalSection _cLockLatch;
}

//
//	FUNCTION public
//	Btree2::UniquePage::UniquePage -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::UniqueFile& cFile_
//		ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
UniquePage::UniquePage(UniqueFile& cFile_)
	: DataPage(cFile_), m_cFile(cFile_)
{
}

//
//	FUNCTION public
//	Btree2::UniquePage::~UniquePage -- デストラクタ
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
UniquePage::~UniquePage()
{
}

//
//	FUNCTION public
//	Btree2::UniquePage::setPhysicalPage -- 物理ページを設定する
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
UniquePage::setPhysicalPage(PhysicalFile::Page* pPhysicalPage_)
{
	DataPage::setPhysicalPage(pPhysicalPage_);
	initialize();
}

//
//	FUNCTION public
//	Btree2::UniquePage::setPhysicalPage -- 物理ページを設定する
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
UniquePage::setPhysicalPage(PhysicalFile::Page* pPhysicalPage_,
							PhysicalFile::PageID uiPrevPageID_,
							PhysicalFile::PageID uiNextPageID_)
{
	DataPage::setPhysicalPage(pPhysicalPage_, uiPrevPageID_, uiNextPageID_);
	initialize();
}

//
//	FUNCTION public
//	Btree2::UniquePage::getUsedSize -- 使用サイズを得る
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
UniquePage::getUsedSize()
{
	return static_cast<ModSize>(*end() - *begin());
}

//
//	FUNCTION public
//	Btree2::UniquePage::lowerBound -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pValue_
//		比較対象データ
//	const Btree2::Compare& cCompare_
//		比較クラス
//
//	RETURN
//	Btree2::UniquePage::Iterator
//		検索結果位置へのイテレータ
//
//	EXCEPTIONS
//
UniquePage::Iterator
UniquePage::lowerBound(const ModUInt32* pValue_, const Compare& cCompare_)
{
	return Algorithm::lowerBound(begin(), end(), pValue_, cCompare_);
}

//
//	FUNCTION public
//	Btree2::UniquePage::upperBound -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pValue_
//		比較対象データ
//	const Btree2::Compare& cCompare_
//		比較クラス
//
//	RETURN
//	Btree2::UniquePage::Iterator
//		検索結果位置へのイテレータ
//
//	EXCEPTIONS
//
UniquePage::Iterator
UniquePage::upperBound(const ModUInt32* pValue_, const Compare& cCompare_)
{
	return Algorithm::upperBound(begin(), end(), pValue_, cCompare_);
}

//
//	FUNCTION public
//	Btree2::UniquePage::find -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pValue_
//		比較対象データ
//	const Btree2::Compare& cCompare_
//		比較クラス
//
//	RETURN
//	Btree2::UniquePage::Iterator
//		検索結果位置へのイテレータ、見つからなかった場合はend()を返す
//
//	EXCEPTIONS
//
UniquePage::Iterator
UniquePage::find(const ModUInt32* pValue_, const Compare& cCompare_)
{
	Iterator e = end();
	Iterator i = Algorithm::lowerBound(begin(), e, pValue_, cCompare_);
	if (i == e || cCompare_(*i, pValue_) != 0)
		return e;
	return i;
}

//
//	FUNCTION public
//	Btree2::UniquePage::insertEntry -- エントリを挿入する
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
UniquePage::insertEntry(const ModUInt32* pBuffer_, ModSize uiSize_)
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
		// 空き領域がない -> 削除済みエントリで不要なものを削除する

		vacuum();
	}
	
	if (getFreeSize() < uiSize_)
	{
		// 空き領域がない -> ページ分割か再配布が必要

		pPage = expand(pBuffer_, uiSize_);

		result = true;
	}

	const Data::Header* h0
		= syd_reinterpret_cast<const Data::Header*>(pBuffer_);
	const Compare& compare
		= getCompareClass(h0->getNullBitmap() ? false : true);
	Iterator i = pPage->upperBound(pBuffer_, compare);
	if (i != pPage->begin())
	{
		if (compare.integrityCheck(*(i-1), pBuffer_) == false)
		{
			// 同じキー値のエントリが入っている

			Data::Header* h = syd_reinterpret_cast<Data::Header*>(*(i-1));
			if (h->isExpunge())
			{
				updateMode();
				dirty();

				// 削除されているエントリなので、上書きする

				--i;
				ModUInt32* p = *i;
				if (dataClass.getSize(p) != uiSize_)
					// 同じキー値なので、同じサイズのはず
					_SYDNEY_THROW0(Exception::BadArgument);

				// 上書きする前に親ページから削除する

				if (i == begin() && m_pParentPage)
				{
					ModSize s;
					AutoPointer<ModUInt32> pe = makeNodeEntry(p, s);
					m_pParentPage->expungeEntry(pe, s, false);
				}

				// 上書きする

				Os::Memory::copy(p, pBuffer_, uiSize_ * sizeof(ModUInt32));

				// 親ページに挿入する

				if (i == begin() && m_pParentPage)
				{
					ModSize s;
					AutoPointer<ModUInt32> pe = makeNodeEntry(p, s);
					m_pParentPage->insertEntry(pe, s);
				}

				return result;
			}

			_SYDNEY_THROW0(Exception::UniquenessViolation);
		}
	}

	// 挿入する
	pPage->insert(i, pBuffer_, uiSize_);

	return result;
}

//
//	FUNCTION public
//	Btree2::UniquePage::expungeEntry -- エントリを削除する
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
UniquePage::expungeEntry(const ModUInt32* pBuffer_, ModSize uiSize_,
						 bool isReduce_)
{
	bool result = false;
	bool eflag = false;
	
	// 削除するエントリを探して消す
	const Compare& compare = getCompareClass();
	Iterator i = find(pBuffer_, compare);
	if (i == end())
	{
		// 見つからなかった
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	if (isLeaf())
	{
		Data::Header* h = syd_reinterpret_cast<Data::Header*>(*i);
		
		if (h->getNullBitmap() == 0 && h->isExpunge() == false)
		{
			// 制約違反チェックのためのロックに利用するため、
			// nullが含まれていないものは、削除フラグを立てるだけにする
			//
			// また、すでに削除フラグが立っているエントリの場合、
			// もう一度削除に来たら、本当に削除する

			updateMode();
			dirty();

			// 更新モードにした時に、
			// バッファが置き換わっている可能性があるので、
			// もう一度イテレータから読み込む
			
			h = syd_reinterpret_cast<Data::Header*>(*i);
			h->setExpungeFlag(true);

			// 削除フラグを付けたのでヘッダーに記録する
			m_cFile.getHeaderPage()->incrementExpungeFlagCount();

			return result;
		}

		if (h->isExpunge())
			// 削除フラグが立っているものの削除である
			eflag = true;

		// リーフであっても、エントリに null が含まれれる場合は、
		// 同じデータが挿入されても制約違反にならないため、削除できる
	}

	// 実際に削除する
		
	expunge(i);

	if (isRoot() == true)
	{
		// ルートノード
		
		if (getCount() == 0)
		{
			// ヘッダーページを初期化する
			m_cFile.getHeaderPage()->initialize();
			if (eflag == false)
				// 削除フラグのエントリの削除じゃなかったら
				// UniqueFileでdecrementされるので1にしとく
				m_cFile.getHeaderPage()->incrementCount();

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
//	Btree2::UniquePage::verify -- 整合性を検査する
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
UniquePage::verify()
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
	}

	ModSize count = 0;
	Iterator prev = 0;
	Iterator i = begin();
	Iterator e = end();
	for (; i != e; ++i)
	{
		const Compare& compare = getCompareClass();
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
		else
		{
			// リーフなので、削除フラグの立っていないエントリ数を数える

			const Data::Header* h
				= syd_reinterpret_cast<const Data::Header*>(*i);
			
			if (!h->isExpunge())
				count++;
		}

		prev = i;
	}

	// エントリ数を追加する
	m_cFile.addVerifyEntryCount(count);
}

//
//	FUNCTION public
//	Btree2::UniquePage::detach -- デタッチする
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
UniquePage::detach()
{
	m_pParentPage = 0;
	Page::detach();
}

//
//	FUNCTION public
//	Btree2::UniquePage::addVacuumEntry
//		-- ページ内の削除可能な削除フラグが付いたエントリを引数に追加する
//
//	NOTES
//
//	ARGUMENTS
//	ModVector<ModPair<Btree2::AutoPointer<ModUInt32>, ModSize> >& vecEntry_
//		削除可能な削除フラグが付いたエントリ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
UniquePage::
addVacuumEntry(ModVector<ModPair<AutoPointer<ModUInt32>, ModSize> >& vecEntry_)
{
	if (isLeaf() == false)
		// バキュームするのはリーフのみ
		return;

	// このファイルのロック名
	const Lock::FileName& fileName = getLockName();

	// すべてのエントリをチェックし、削除フラグがあるエントリをロックし、
	// ロックできたら、引数に内容をコピーする

	Iterator i = begin();
	Iterator e = end();
	for (; i != e; ++i)
	{
		ModUInt32* p = *i;
		if (syd_reinterpret_cast<Data::Header*>(p)->isExpunge() == false)
			// 次へ
			continue;

		// ROWIDを得る
		ModUInt32 uiRowID = getDataClass().getRowID(p);

		// ロック名を作る
		Lock::TupleName n(fileName.getDatabasePart(),
						  fileName.getTablePart(),
						  uiRowID);

		// トランザクションを得る
		Trans::Transaction& trans
			= const_cast<Trans::Transaction&>(getTransaction());

		// ロックできたか否か
		bool bLocked = false;
		
		{
			//【注意】
			//	Lockモジュールは同じトランザクションで同じリクエストが
			//	複数のスレッドから同時に来ることは想定していない
			//	そのため、呼び出し側で直列化する
			
			Os::AutoCriticalSection cAuto(_cLockLatch);

			// 一瞬だけロックする
			bLocked = trans.lock(n, Lock::Mode::X, Lock::Duration::Pulse, 0);
		}

		if (bLocked)
		{
			// ロックできたので、削除対象

			// サイズを得る
			ModSize size = getDataClass().getSize(p);

			// メモリを確保し、コピーする
			AutoPointer<ModUInt32> buf
				= syd_reinterpret_cast<ModUInt32*>(
					Os::Memory::allocate(size * sizeof(ModUInt32)));
			Os::Memory::copy(buf, p, size * sizeof(ModUInt32));

			// 引数に追加する
			vecEntry_.pushBack(
				ModPair<AutoPointer<ModUInt32>, ModSize>(buf, size));
		}
	}
}

//
//	FUNCTION private
//	Btree2::UniquePage::initialize -- 初期化
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
UniquePage::initialize()
{
	ModUInt32 count = getCount();
	
	if (m_vecpEntry.getCapacity() < count + 2)
		m_vecpEntry.reserve(count + 2);	// 1件挿入されてもreallocateしないように

	m_vecpEntry.assign(count + 1, 0);

	load(count);
}

//
//	FUNCTION private
//	Btree2::UniquePage::load -- ベクターに値を設定する
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
UniquePage::load(ModUInt32 count_)
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
			*j = p;
			++j;
			p += size;
		}
	}
	else
	{
		for (ModUInt32 i = 0; i < count_; ++i)
		{
			*j = p;
			++j;
			p += dataClass.getSize(p);
		}
	}
	*j = p;
}

//
//	FUNCTION protected
//	Btree2::UniquePage::reload -- 物理ページが変更されたので読み直す
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
UniquePage::reload()
{
	ModUInt32 count = getCount();
	load(count);
}

//
//	FUNCTION public
//	Btree2::UniquePage::insert -- 指定位置に挿入する
//
//	NOTES
//
//	AGUMENTS
//	Btree2::UniquePage::Iterator i
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
UniquePage::insert(Iterator i_, const ModUInt32* pBuffer_, ModSize uiSize_)
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
//	Btree2::UniquePage::insert -- 指定位置に挿入する
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::UniquePage::Iterator i_
//		挿入位置
//	Btree2::UniquePage::ConstIterator start_
//		挿入するエントリの先頭
//	Btree2::UniquePage::ConstIterator end_
//		挿入するエントリの終端
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
UniquePage::insert(Iterator i_, ConstIterator start_, ConstIterator end_)
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
//	Btree2::UniquePage::expunge -- 指定位置のものを削除する
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::UniquePage::Iterator i_
//		削除するエントリ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
UniquePage::expunge(Iterator i_)
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
//	Btree2::UniquePage::expunge -- 指定位置のものを削除する
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::UniquePage::Iterator start_
//		削除するエントリの先頭
//	Btree2::UniquePage::Iterator end_
//		削除するエントリの終端
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
UniquePage::expunge(Iterator start_, Iterator end_)
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
//	FUNCTION private
//	Btree2::UniquePage::expand -- 拡張処理を行う
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
//	Btree2::UniquePage::PagePointer
//		挿入するべきページ
//
//	EXCEPTIONS
//
UniquePage::PagePointer
UniquePage::expand(const ModUInt32* pBuffer_, ModSize uiSize_)
{
	PagePointer pResult;
	const Compare& compare = getCompareClass();
	
	if (getNextPageID() == PhysicalFile::ConstValue::UndefinedPageID
		&& compare(*(end() - 1), pBuffer_) < 0
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
			AutoPointer<ModUInt32> entry = makeNodeEntry(*begin(), size);
			pRoot->insertEntry(entry, size);
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

		ModSize pageSize = getPageSize();
		Iterator i = begin();
		if (ratio < 100)
		{
			// 移動する
			ModUInt32* b = *i;
			Iterator e = end();
			for (; i != e; ++i)
			{
				if ((*i - b) >= static_cast<int>((pageSize * ratio / 100)))
					break;
			}
		}
		else
		{
			i = end();
			// 1つは新しいページに移動しないといけない
			--i;
		}

		pNewPage->insert(pNewPage->begin(), i, end());
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
			m_cFile.getHeaderPage()->setRootPageID(pRoot->getID());
			m_cFile.getHeaderPage()->incrementStepCount();
			// 自分の親を設定する
			m_pParentPage = pRoot;

			// ルートノードに先頭の要素を加える
			ModSize size;
			AutoPointer<ModUInt32> entry = makeNodeEntry(*begin(), size);
			pRoot->insertEntry(entry, size);

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
			pResult = pNext->split(pPrev, pBuffer_);
		}
		else
		{
			// それ以外なので、再配分
			pResult = pNext->redistribute(pPrev, pBuffer_);
		}
	}

	return pResult;
}

//
//	FUNCTION private
//	Btree2::UniquePage::reduce -- 縮小処理を行う
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
UniquePage::reduce()
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
//	Btree2::UniquePage::split -- ページ分割する
//
//	NOTES
//	自分とpPrevPage_の間に新しいページを作成する
//
//	ARGUMENTS
//	Btree2::UniquePage::PagePointer pPrevPage_
//		前のページ
//	const ModUInt32* pBuffer_
//		挿入するエントリ
//
//	RETURN
//	Btree2::UniquePage::PagePointer
//		挿入するべきページ
//
//	EXCEPTIONS
//
UniquePage::PagePointer
UniquePage::split(PagePointer pPrevPage_, const ModUInt32* pBuffer_)
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
	setParentPage(m_cFile.findPage(*begin(), getID()));
	expunge(begin(), i);

	//
	//	どのページに入れるべきか
	//

	PagePointer pResult;
	
	const Compare& compare = getCompareClass();
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
//	Btree2::UniquePage::concatenate -- ページ連結
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::UniquePage::PagePointer pPrevPage_
//		前のページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
UniquePage::concatenate(PagePointer pPrevPage_)
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
	pPrevPage_->insert(pPrevPage_->end(), begin(), end());
	// 移動した分を削除する
	expunge(begin(), end());

	// このページを削除
	m_cFile.freePage(this);
}

//
//	FUNCTION private
//	Btree2::UniquePage::redistribute -- 再配分
//
//	NOTES
//
//	ARGUMENTS
//	Btree2::UniquePage::PagePointer pPrevPage_
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
UniquePage::PagePointer
UniquePage::redistribute(PagePointer pPrevPage_, const ModUInt32* pBuffer_)
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
		const Compare& compare = getCompareClass();
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
//	Btree2::UniquePage::vacuum -- 削除済みエントリを削除する
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
UniquePage::vacuum()
{
	if (isLeaf() == false)
		// バキュームするのはリーフのみ
		return;
	
	Iterator i = begin();
	Iterator e = end();

	// 削除フラグがあるエントリがあるかチェックする
	for (; i != e; ++i)
	{
		ModUInt32* p = *i;
		if (syd_reinterpret_cast<Data::Header*>(p)->isExpunge() == true)
			// 削除されたものがあった
			break;
	}

	if (i == e)
		// 削除フラグが立っているものはない
		return;

	updateMode();
	dirty();

	// このファイルのロック名
	const Lock::FileName& fileName = getLockName();

	bool nodeUpdate = false;
	i = begin();
	while (i != end())
	{
		ModUInt32* p = *i;
		if (syd_reinterpret_cast<Data::Header*>(p)->isExpunge() == true)
		{
			// このエントリは削除されている
			// 消すことができるかチェックする

			// ROWIDを得る
			ModUInt32 uiRowID = getDataClass().getRowID(p);

			// ロック名を作る
			Lock::TupleName n(fileName.getDatabasePart(),
							  fileName.getTablePart(),
							  uiRowID);

			// トランザクションを得る
			Trans::Transaction& trans
				= const_cast<Trans::Transaction&>(getTransaction());
			
			// ロックできたか否か
			bool bLocked = false;
		
			{
				//【注意】
				//	Lockモジュールは同じトランザクションで同じリクエストが
				//	複数のスレッドから同時に来ることは想定していない
				//	そのため、呼び出し側で直列化する
			
				Os::AutoCriticalSection cAuto(_cLockLatch);

				// 一瞬だけロックする
				bLocked = trans.lock(n, Lock::Mode::X,
									 Lock::Duration::Pulse, 0);
			}
			
			if (bLocked)
			{
				// ロックできたので、消す

				if (i == begin() && m_pParentPage && !nodeUpdate)
				{
					// 先頭のエントリなので、親ノードからも消す
					//
					//【注意】	ここで一旦消すがあとで追加される
					
					nodeUpdate = true;
					ModSize uiSize;
					AutoPointer<ModUInt32> p = makeNodeEntry(*i, uiSize);
					m_pParentPage->expungeEntry(p, uiSize, false);
				}

				// まずは、ページのメモリを移動する
				ModUInt32* first = *(i + 1);
				ModUInt32* last = *end();
				ModSize length = static_cast<ModSize>(last - first);
				ModSize size = static_cast<ModSize>(first - *i);
				if (length != 0)
				{
					// 大きい部分を前に移動
					Os::Memory::move(*i, first, length * sizeof(ModUInt32));
				}

				// ベクターのアドレスを更新する
				Iterator j = i + 1;
				Iterator e = end();
				for (; j <= e; ++j)
				{
					*j -= size;
				}

				// ベクターから削除する
				i = m_vecpEntry.erase(i);
				// エントリ数を減らす
				subtractCount(1);

				// 削除フラグを付けたエントリを消したので、ヘッダーに記録する
				m_cFile.getHeaderPage()->decrementExpungeFlagCount();

				// 次へ
				continue;
			}
		}

		// 次へ
		++i;
	}

	if (nodeUpdate && getCount() != 0)
	{
		// 親ノードに挿入する

		ModSize uiSize;
		AutoPointer<ModUInt32> p = makeNodeEntry(*begin(), uiSize);
		m_pParentPage->insertEntry(p, uiSize);
	}
}

//
//	FUNCTION private
//	Btree2::UniquePage::makeNodeEntry
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
UniquePage::makeNodeEntry(ModUInt32* pEntry_, ModSize& uiSize_)
{
	// キー部分の大きさを得る
	ModSize size = getKeyDataClass().getSize(pEntry_);
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

	// 削除フラグを削除する
	syd_reinterpret_cast<Data::Header*>(p.get())->setExpungeFlag(false);
	
	return p;
}

//
//	Copyright (c) 2014, 2015, 2018, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
