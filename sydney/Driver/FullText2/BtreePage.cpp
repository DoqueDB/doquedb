// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreePage.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
#include "FullText2/BtreePage.h"
#include "FullText2/BtreeFile.h"
#include "FullText2/MessageAll_Class.h"

#include "Os/Memory.h"

#include "Exception/UniquenessViolation.h"
#include "Exception/EntryNotFound.h"
#include "Exception/VerifyAborted.h"
#include "Exception/Cancel.h"

#include "ModAlgorithm.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//
	//	CONST
	//	_$$::_LEAF_MASK
	//
	const ModUInt32 _LEAF_MASK = 0x80000000;
}

//
//	FUNCTION public
//	FullText2::BtreePage::BtreePage -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::BtreeFile& cFile_
//		B木ファイル
//	PhysicalFile::Page* pPage_
//		物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
BtreePage::BtreePage(BtreeFile& cFile_, PhysicalFile::Page* pPage_)
	: Page(cFile_, pPage_), m_cFile(cFile_), m_pHeader(0)
{
	// ページを読み込む
	load();
}

//
//	FUNCTION public
//	FullText2::BtreePage::BtreePage -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::BtreeFile& cFile_
//		B木ファイル
//	PhysicalFile::Page* pPage_
//		物理ページ
//	PhysicalFile::PageID uiPrevPageID_
//		前方のページID
//	PhysicalFile::PageID uiNextPageID_
//		後方のページID
//
//	RETURN
//
//	EXCEPTIONS
//
BtreePage::BtreePage(BtreeFile& cFile_, PhysicalFile::Page* pPage_,
					 PhysicalFile::PageID uiPrevPageID_,
					 PhysicalFile::PageID uiNextPageID_)
	: Page(cFile_, pPage_), m_cFile(cFile_), m_pHeader(0)
{
	dirty();
	
	Header* pHeader = syd_reinterpret_cast<Header*>(getBuffer());
	pHeader->m_uiPrevPageID = uiPrevPageID_;
	pHeader->m_uiNextPageID = uiNextPageID_;
	pHeader->m_uiCount = 0;

	// ページを読み込む
	load();
}

//
//	FUNCTION public
//	FullText2::BtreePage::~BtreePage -- デストラクタ
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
BtreePage::~BtreePage()
{
}

//
//	FUNCTION public
//	FullText2::BtreePage::reset -- リセットする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPage_
//		物理ページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::reset(PhysicalFile::Page* pPage_)
{
	Page::reset(pPage_);
	m_pHeader = 0;
	load();
}

//
//	FUNCTION public
//	BtreePage::reset -- リセットする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::Page* pPage_
//		物理ページ
//	PhysicalFile::PageID uiPrevPageID_
//		前方のページID
//	PhysicalFile::PageID uiNextPageID_
//		後方のページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::reset(PhysicalFile::Page* pPage_,
				 PhysicalFile::PageID uiPrevPageID_,
				 PhysicalFile::PageID uiNextPageID_)
{
	Page::reset(pPage_);
	m_pHeader = 0;
	
	dirty();
	
	Header* pHeader = syd_reinterpret_cast<Header*>(getBuffer());
	pHeader->m_uiPrevPageID = uiPrevPageID_;
	pHeader->m_uiNextPageID = uiNextPageID_;
	pHeader->m_uiCount = 0;

	// ページを読み込む
	load();
}

//
//	FUNCTION public
//	FullText2::BtreePage::search -- 引数以下で最大のエントリを検索する
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::BtreePage::Entry& cEntry_
//		検索するエントリ
//	ModUInt32& uiValue_
//		バリュー
//
//	RETURN
//	bool
//		存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
BtreePage::search(const Entry& cEntry_, ModUInt32& uiValue_)
{
	// まずupper_boundで、キーより大きなもので最小のものを検索する
	Iterator i = ModUpperBound(begin(), end(), &cEntry_, Less());
	if (i == begin()) return false;
	// 検索結果の前のノードが、キー以下の最大のものである
	// キーがユニークなのでこれでいい
	uiValue_ = (*--i)->m_uiValue;
	return true;
}

//
//	FUNCTION public
//	FullText2::BtreePage::find -- 同じキーのエントリを検索する
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::BtreePage::Entry& cEntry_
//		検索するエントリ
//	ModUInt32& uiValue_
//		バリュー
//
//	RETURN
//	bool
//		存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
BtreePage::find(const Entry& cEntry_, ModUInt32& uiValue_)
{
	// まずlower_boundで検索する
	Iterator i = ModLowerBound(begin(), end(), &cEntry_, Less());
	if (i == end()) return false;
	// キーが同じか確認する
	if (Entry::compare(**i, cEntry_) != 0) return false;
	uiValue_ = (*i)->m_uiValue;
	return true;
}

//
//	FUNCTION public
//	FullText2::BtreePage::insert -- 1つ挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::BtreePage::Entry& cEntry_
//		挿入するエントリ
//
//	RETURN
//	bool
//		普通に挿入できた場合はtrue、ページ分割や再配分が発生した場合はfalse
//
//	EXCEPTIONS
//
bool
BtreePage::insert(const Entry& cEntry_)
{
	// 挿入するエントリの使用ユニット数をもとめる
	ModSize unit = calcUnitLength(cEntry_);
	if (getFreeUnitSize() < unit)
	{
		// ページがいっぱいなので、挿入できない
		//	-> ページ分割や再配分が必要

		expand(unit, &cEntry_);

		return false;
	}
	else
	{
		AutoEntry pKey1;
		AutoEntry pKey2;

		// 挿入する位置を重複チェックのためにlower_boundで求める
		Iterator i;
		if (getCount() == 0)
		{
			i = begin();
		}
		else
		{
			i = ModLowerBound(begin(), end(), &cEntry_, Less());
			if (i != end() && Entry::compare(**i, cEntry_) == 0)
			{
				// 同じキー値のエントリが存在している
				_SYDNEY_THROW0(Exception::UniquenessViolation);
			}
		}
		if (i == begin())
		{
			// 先頭に挿入するので、親ノードに対して更新要求する必要がある

			if (i != end())
			{
				pKey1 = allocateEntry(**begin());
				pKey1->m_uiValue = getID();
			}

			pKey2 = allocateEntry(cEntry_);
			pKey2->m_uiValue = getID();
		}

		ModUInt32* first = syd_reinterpret_cast<ModUInt32*>(*i);
		ModUInt32* last = syd_reinterpret_cast<ModUInt32*>(*end());
		// 移動する部分の長さ
		ModUInt32 length = static_cast<ModUInt32>(last - first);
		if (length != 0)
		{
			// 大きい部分を後ろに移動する
			ModOsDriver::Memory::move(first + unit, first,
									  length*sizeof(ModUInt32));
		}

		// ベクターのアドレスを更新する
		Iterator s = i;
		for (; s <= end(); ++s)
		{
			(*s) = syd_reinterpret_cast<Entry*>(
				syd_reinterpret_cast<ModUInt32*>(*s) + unit);
		}
		
		// 見つかった位置に挿入する
		ModOsDriver::Memory::copy(first, &cEntry_, unit*sizeof(ModUInt32));
		// ベクターにも追加する
		m_vecpEntry.insert(i, syd_reinterpret_cast<Entry*>(first));
		// ヘッダーを更新する
		m_pHeader->m_uiCount++;
		// ページをdirtyにする
		dirty();

		if (pKey2.get())
		{
			// 親ノードを更新する
			updateParent(pKey1, pKey2);
		}

		return true;
	}
}

//
//	FUNCTION public
//	FullText2::BtreePage::insert -- 配列を挿入する
//
//	NOTES
//	このinsertは、ページ分割や再配置は発生しないことが保障されている
//	場合にのみ呼び出すことができる
//
//	ARGUMENTS
//	FullText2::BtreePage::ConstIterator first_
//		挿入する配列の先頭
//	FullText2::BtreePage::ConstIterator last_
//		挿入する配列の終端
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::insert(ConstIterator first_, ConstIterator last_)
{
	if (first_ != last_)
	{
		AutoEntry pKey1;
		AutoEntry pKey2;

		// 挿入する値より大きくて最小のものを求める
		Iterator i = ModUpperBound(begin(), end(), *(last_-1), Less());
		if (i == begin())
		{
			// 先頭に挿入するので、親ノードに対して更新要求する必要がある

			if (i != end())
			{
				pKey1 = allocateEntry(**begin());
				pKey1->m_uiValue = getID();
			}

			pKey2 = allocateEntry(**first_);
			pKey2->m_uiValue = getID();
		}

		ModUInt32* first = syd_reinterpret_cast<ModUInt32*>(*i);
		ModUInt32* last = syd_reinterpret_cast<ModUInt32*>(*end());
		// 移動する部分の長さ
		ModUInt32 length = static_cast<ModUInt32>(last - first);
		ModUInt32* f = syd_reinterpret_cast<ModUInt32*>(*first_);
		ModUInt32* l = syd_reinterpret_cast<ModUInt32*>(*last_);
		ModUInt32 unit = static_cast<ModUInt32>(l - f);
		if (length != 0)
		{
			// 大きい部分を後ろに移動する
			ModOsDriver::Memory::move(first + unit, first,
									  length*sizeof(ModUInt32));
		}

		// コピーする
		ModOsDriver::Memory::copy(first, f, unit*sizeof(ModUInt32));

		// ヘッダーを更新する
		m_pHeader->m_uiCount += last_ - first_;

		// ベクターを設定しなおす
		loadEntry();

		dirty();

		if (pKey2.get())
		{
			// 親ノードを更新する
			updateParent(pKey1, pKey2);
		}
	}
}

//
//	FUNCTION public
//	FullText2::BtreePage::expunge -- １つ削除する
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::BtreePage::Entry& cEntry_
//		削除するエントリ
//
//	RETURN
//	bool
//		普通に削除できた場合はtrue、再配分やページ転結が発生した場合はfalse
//
//	EXCEPTIONS
//
bool
BtreePage::expunge(const Entry& cEntry_)
{
	AutoEntry pKey1;
	AutoEntry pKey2;

	// 削除する位置をlower_boundで求める
	Iterator i = ModLowerBound(begin(), end(), &cEntry_, Less());
	if (i == end() || Entry::compare(**i, cEntry_) != 0)
	{
		_SYDNEY_THROW1(Exception::EntryNotFound,
					   ModUnicodeString(cEntry_.m_pszKey, cEntry_.m_usLength));
	}
	if (i == begin())
	{
		// 先頭を削除するので、親ノードに対して更新要求する必要がある

		pKey1 = allocateEntry(**begin());
		pKey1->m_uiValue = getID();

		if (getCount() > 1)
		{
			pKey2 = allocateEntry(**(begin()+1));
			pKey2->m_uiValue = getID();
		}
	}

	ModUInt32* first = syd_reinterpret_cast<ModUInt32*>(*i);
	ModUInt32* last = syd_reinterpret_cast<ModUInt32*>(*end());
	ModUInt32 unit = calcUnitLength(**i);
	ModUInt32 length = static_cast<ModUInt32>(last - first - unit);
	if (length != 0)
	{
		// 大きい部分を前に移動する
		ModOsDriver::Memory::move(first, first + unit,
								  length*sizeof(ModUInt32));
	}

	// ベクターのアドレスを更新する
	Iterator s = i + 1;
	for (; s <= end(); ++s)
	{
		(*s) = syd_reinterpret_cast<Entry*>(
			syd_reinterpret_cast<ModUInt32*>(*s) - unit);
	}

	// ベクターから削除する
	m_vecpEntry.erase(i);
	// ヘッダーを更新する
	m_pHeader->m_uiCount--;
	// ページをdirtyにする
	dirty();

	if (pKey1.get())
	{
		// 親ノードを更新する
		updateParent(pKey1, pKey2);
	}

	bool bResult = true;

	if (isRoot())
	{
		// ルートノード

		if (getCount() == 0)
		{
			// ルートページを設定する
			m_cFile.setRootPage(0);

			// 自分を削除
			m_cFile.freePage(this);

			bResult = false;
		}
		else if (getCount() == 1 && isLeaf() == false)
		{
			// Leafじゃないのにエントリが1つになった
			//	-> 自分を削除して、子をルートにする

			// 子ページをattachする
			PagePointer pPage
				= m_cFile.attachPage((*begin())->m_uiValue, m_uiStepCount + 1);

			// ルートページを設定する
			m_cFile.setRootPage(pPage);

			// 自分の段数を１つ減らす
			m_uiStepCount--;

			// 自分を削除
			m_cFile.freePage(this);
			
			bResult = false;
		}
	}
	else if (getFreeUnitSize() > getPageUnitSize() * 2 / 5)
	{
		// ページ使用率が40%未満になった
		//	-> 再配分やページ連結が必要

		reduce();

		bResult = false;
	}

	return bResult;
}

//
//	FUNCTION public
//	FullText2::BtreePage::expunge -- 配列を削除する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::BtreePage::Iterator first_
//		削除する配列の先頭
//	FullText2::BtreePage::Iterator last_
//		削除する配列の終端
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::expunge(Iterator first_, Iterator last_)
{
	if (first_ != last_)
	{
		AutoEntry pKey1;
		AutoEntry pKey2;

		if (begin() == first_)
		{
			// 先頭も消す
			pKey1 = allocateEntry(**first_);
			pKey1->m_uiValue = getID();

			if (end() != last_)
			{
				// 残るエントリがある
				pKey2 = allocateEntry(**last_);
				pKey2->m_uiValue = getID();
			}
		}

		// ヘッダーを更新する
		m_pHeader->m_uiCount -= last_ - first_;

		if (last_ == end())
		{
			// 最後なので消すだけ
			m_vecpEntry.erase(first_ + 1, last_ + 1);
		}
		else
		{
			ModUInt32* f = syd_reinterpret_cast<ModUInt32*>(*first_);
			ModUInt32* l = syd_reinterpret_cast<ModUInt32*>(*last_);
			ModUInt32 length = static_cast<ModUInt32>(
				syd_reinterpret_cast<ModUInt32*>(*end()) - l);

			// 移動する
			ModOsDriver::Memory::move(f, l, length*sizeof(ModUInt32));

			// ベクターを設定しなおす
			loadEntry();
		}

		dirty();

		if (pKey1.get())
		{
			// 親ノードを更新する
			updateParent(pKey1, pKey2);
		}
	}
}

//
//	FUNCTION public
//	FullText2::BtreePage::update -- １つ更新する
//
//	NOTES
//	１つのエントリを更新する。変更するキー値の値を直接変更するので、
//	このページ内のエントリの順番が変わらないことが想定されている
//
//	ARGUMENTS
//	const FullText2::BtreePage::Entry& cKey1_
//		更新する位置を示すエントリ
//	const FullText2::BtreePage::Entry& cKey2_
//		その位置に上書きするエントリ
//
//	RETURN
//	bool
//		普通に更新できた場合はtrue
//		ページ分割や再配分やページ連結が発生した場合はfalse
//
//	EXCEPTIONS
//
bool
BtreePage::update(const Entry& cKey1_, const Entry& cKey2_)
{
	int unit = calcUnitLength(cKey2_) - calcUnitLength(cKey1_);

	if (static_cast<int>(getFreeUnitSize()) < unit)
	{
		// ページがいっぱいなので、挿入できない
		//	-> ページ分割や再配分が必要

		expand(unit, &cKey1_, &cKey2_);

		return false;
	}
	else
	{
		AutoEntry pKey1;
		AutoEntry pKey2;

		// 更新する位置をlower_boundで求める
		Iterator i = ModLowerBound(begin(), end(), &cKey1_, Less());
		if (i == end() || Entry::compare(**i, cKey1_) != 0)
		{
			_SYDNEY_THROW1(Exception::EntryNotFound,
						  ModUnicodeString(cKey1_.m_pszKey, cKey1_.m_usLength));
		}
		if (i == begin())
		{
			// 先頭を更新するので、親ノードに対して更新要求する必要がある

			pKey1 = allocateEntry(cKey1_);
			pKey1->m_uiValue = getID();

			pKey2 = allocateEntry(cKey2_);
			pKey2->m_uiValue = getID();
		}

		ModUInt32* first = syd_reinterpret_cast<ModUInt32*>(*(i+1));
		ModUInt32* last = syd_reinterpret_cast<ModUInt32*>(*end());
		// 移動する部分の長さ
		ModUInt32 length = static_cast<ModUInt32>(last - first);
		if (length != 0)
		{
			// 更新する位置より後ろの部分を移動する
			ModOsDriver::Memory::move(first + unit, first,
									  length*sizeof(ModUInt32));
		}

		// ベクターのアドレスを更新する
		Iterator s = i + 1;
		for (; s <= end(); ++s)
		{
			(*s) = syd_reinterpret_cast<Entry*>(
				syd_reinterpret_cast<ModUInt32*>(*s) + unit);
		}

		// 更新する位置にコピーする
		ModOsDriver::Memory::copy(syd_reinterpret_cast<ModUInt32*>(*i), &cKey2_,
									calcUnitLength(cKey2_)*sizeof(ModUInt32));
		// ページをdirtyにする
		dirty();

		if (pKey1.get())
		{
			// 親ノードを更新する
			updateParent(pKey1, pKey2);
		}

		if (isRoot() == false
			&& getFreeUnitSize() > getPageUnitSize() * 2 / 5)
		{
			// ページ使用率が40%未満になった
			//	-> 再配分やページ連結が必要

			reduce();
			
			return false;
		}

		return true;
	}
}

//
//	FUNCTION public
//	Inveted::BtreePage::begin -- 先頭のエントリを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Inverrted::BtreePage::Iterator(Inverrted::BtreePage::ConstIterator)
//		先頭のエントリ
//
//	EXCEPTIONS
//
BtreePage::Iterator
BtreePage::begin()
{
	return m_vecpEntry.begin();
}
BtreePage::ConstIterator
BtreePage::begin() const
{
	return m_vecpEntry.begin();
}

//
//	FUNCTION public
//	FullText2::BtreePage::end -- 終端のエントリを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::BtreePage::Iterator(Inverrted::BtreePage::ConstIterator)
//		終端のエントリ
//
//	EXCEPTIONS
//
BtreePage::Iterator
BtreePage::end()
{
	return m_vecpEntry.end() - 1;
}
BtreePage::ConstIterator
BtreePage::end() const
{
	return m_vecpEntry.end() - 1;
}

//
//	FUNCTION public
//	FullText2::BtreePage::isLeaf -- リーフページかどうか
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
//	なし
//
bool
BtreePage::isLeaf() const
{
	return m_pHeader->m_uiCount & _LEAF_MASK;
}

//
//	FUNCTION public
//	FullText2::BtreePage::setLeaf -- リーフファイルに設定する
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
BtreePage::setLeaf()
{
	m_pHeader->m_uiCount |= _LEAF_MASK;
	dirty();
}

//
//	FUNCTION public
//	FullText2::BtreePage::reset -- 内容をリセットする
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPrevPageID_
//		前方のページID
//	PhysicalFile::PageID uiNextPageID_
//		後方のページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::reset(PhysicalFile::PageID uiPrevPageID_,
				 PhysicalFile::PageID uiNextPageID_)
{
	dirty();
	
	m_pHeader->m_uiPrevPageID = uiPrevPageID_;
	m_pHeader->m_uiNextPageID = uiNextPageID_;
	m_pHeader->m_uiCount = 0;

	load();
}

//
//	FUNCTION public
//	FullText2::BtreePage::setPrevPageID -- 前方のページIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiPrevPageID_
//		設定する前方のページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::setPrevPageID(PhysicalFile::PageID uiPrevPageID_)
{
	m_pHeader->m_uiPrevPageID = uiPrevPageID_;
	dirty();
}

//
//	FUNCTION public
//	FullText2::BtreePage::setNextPageID -- 後方のページIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiNextPageID_
//		設定する後方のページID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::setNextPageID(PhysicalFile::PageID uiNextPageID_)
{
	m_pHeader->m_uiNextPageID = uiNextPageID_;
	dirty();
}

//
//	FUNCTION public
//	FullText2::BtreePage::getCount -- エントリ数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		このページのエントリ数
//
//	EXCEPTIONS
//
int
BtreePage::getCount() const
{
	return m_pHeader->m_uiCount & ~_LEAF_MASK;
}

//
//	FUNCTION public
//	FullText2::BtreePage::getUsedUnitSize -- 使用ユニット数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		使用ユニット数
//
//	EXCEPTIONS
//
ModSize
BtreePage::getUsedUnitSize() const
{
	const ModUInt32* first = getBuffer();
	const ModUInt32* last = syd_reinterpret_cast<const ModUInt32*>(*end());
	return static_cast<ModSize>(last - first);
}

//
//	FUNCTION public
//	FullText2::BtreeFile::verify -- 整合性検査を行う
//
//	NOTES
//	B木ページの整合性検査は以下の項目をチェックする
//	1. すべてのページをたどり、使用しているページIDを物理ファイルに通知する
//	2. すべてのページのエントリが昇順に格納されているかチェックする
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
	Iterator prev = 0;
	Iterator i = begin();
	for (; i != end(); ++i)
	{
		if (&*prev)
		{
			// 大きさをチェック
			if (Entry::compare(**prev, **i) >= 0)
			{
				_SYDNEY_VERIFY_INCONSISTENT(m_cFile.getProgress(),
											m_cFile.getRootPath(),
											Message::IllegalIndex());
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}
		}

		if (isLeaf() == false)
		{
			// 下位ページをattachする
			PagePointer pPage = m_cFile.attachPage((*i)->m_uiValue,
												   m_uiStepCount + 1);

			// 先頭要素が同じかチェックする
			if (Entry::compare(**i, **pPage->begin()) != 0)
			{
				_SYDNEY_VERIFY_INCONSISTENT(m_cFile.getProgress(),
											m_cFile.getRootPath(),
											Message::IllegalIndex());
				_SYDNEY_THROW0(Exception::VerifyAborted);
			}

			// 下位ページの整合性をチェックする
			pPage->verify();

		}

		prev = i;
	}
}

//
//	FUNCTION public static
//	FullText2::BtreePage::allocateEntry -- エントリを得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar* pszKey_
//		エントリのキー文字列
//	PhysicalFile::PageID uiPageID_
//		リーフファイルのページID
//		(default PhysicalFile::ConstValue::UndefinedPageID)
//
//	RETURN
//	FullText2::BtreePage::Entry*
//		新たに確保したエントリ
//
//	EXCEPTIONS
//
BtreePage::Entry*
BtreePage::allocateEntry(const ModUnicodeChar* pszKey_,
						 PhysicalFile::PageID uiPageID_)
{
	unsigned short length
		= static_cast<unsigned short>(ModUnicodeCharTrait::length(pszKey_));
	int size = calcUnitLength(length)*sizeof(ModUInt32);
	Entry* pEntry = static_cast<Entry*>(Os::Memory::allocate(size));
	pEntry->m_uiValue = uiPageID_;
	pEntry->m_usLength = length;
	ModOsDriver::Memory::copy(pEntry->m_pszKey, pszKey_,
							  length*sizeof(ModUnicodeChar));
	return pEntry;
}

//
//	FUNCTION public static
//	FullText2::BtreePage::allocateEntry
//		-- 新たにメモリーを確保し、エントリーをコピーする
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::BtreePage::Entry& cEntry_
//		コピー元のエントリ
//
//	RETURN
//	FullText2::BtreePage::Entry*
//		新たに確保したメモリーにコピーされたエントリ
//
//	EXCEPTIONS
//
BtreePage::Entry*
BtreePage::allocateEntry(const Entry& cEntry_)
{
	int size = calcUnitLength(cEntry_)*sizeof(ModUInt32);
	Entry* pEntry = static_cast<Entry*>(Os::Memory::allocate(size));
	ModOsDriver::Memory::copy(pEntry, &cEntry_, size);
	return pEntry;
}

//
//	FUNCTION private
//	FullText2::BtreePage::load -- ページを読み込む
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
	// ヘッダーを設定する
	m_pHeader = syd_reinterpret_cast<Header*>(getBuffer());

	// エントリを読み込む
	loadEntry();
}

//
//	FUNCTION private
//	FullText2::BtreePage::loadEntry -- エントリを読み込む
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
BtreePage::loadEntry()
{
	int count = getCount();

	ModUInt32* p = const_cast<ModUInt32*>(getBuffer());

	// データ領域へ移動する
	p += sizeof(Header) / sizeof(ModUInt32);

	// 配列を１つ多く確保する
	m_vecpEntry.assign(count+1);

	// データへのポインターを割り当てる
	Iterator i = begin();
	Iterator e = end();
	for (; i < e; ++i)
	{
		(*i) = syd_reinterpret_cast<Entry*>(p);
		p += calcUnitLength((*i)->m_usLength);
	}
	
	// 終端を設定する
	(*i) = syd_reinterpret_cast<Entry*>(p);
}

//
//	FUNCTION private
//	FullText2::BtreePage::expand -- 拡張のための操作を行う
//
//	NOTES
//
//	ARGUMENTS
//	ModSize unit_
//		拡張するユニットサイズ
//	const FullText2::BtreePage::Entry* pEntry1_
//		挿入するエントリまたは、更新するキー (default 0)
//	const FullText2::BtreePage::Entry* pEntry2_
//		更新する値 (default 0)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::expand(ModSize unit_, const Entry* pEntry1_, const Entry* pEntry2_)
{
	PagePointer pPrevPage = 0;
	PagePointer pNextPage = 0;

	if (getNextPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 後方のページをattachする
		pNextPage = m_cFile.attachPage(getNextPageID(), m_uiStepCount);
	}
	else if (getPrevPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 前方のページをattachする
		pPrevPage = m_cFile.attachPage(getPrevPageID(), m_uiStepCount);
	}
	else
	{
		// 新しくルートノードを作成する
		PagePointer pRootPage
			= m_cFile.allocatePage(PhysicalFile::ConstValue::UndefinedPageID,
								   PhysicalFile::ConstValue::UndefinedPageID,
								   1);
		// ルートノードを設定する
		m_cFile.setRootPage(pRootPage);

		// 自分の段数を1つ増やす
		m_uiStepCount++;

		// ルートノードに先頭の要素を追加する
		AutoEntry pEntry(allocateEntry(**begin()));
		pEntry->m_uiValue = getID();
		pRootPage->insert(*pEntry);

		// 後方も前方もページが存在しないので、新しいページをアタッチする
		pNextPage
			= m_cFile.allocatePage(getID(),
								   PhysicalFile::ConstValue::UndefinedPageID,
								   m_uiStepCount);
		// ヘッダーを更新する
		setNextPageID(pNextPage->getID());

		if (isLeaf() == true)
		{
			// 自分がリーフなら新しく確保したページもリーフ
			pNextPage->setLeaf();
			// 新しく確保したページは一番右のページ
			m_cFile.setRightLeafPage(pNextPage);
		}
	}

	// 空き容量を得る
	ModSize freeUnit;
	if (pNextPage)
	{
		freeUnit = pNextPage->getFreeUnitSize();
		pPrevPage = this;
	}
	else
	{
		freeUnit = pPrevPage->getFreeUnitSize();
		pNextPage = this;
	}

	if (freeUnit < getPageUnitSize() / 5 || freeUnit < unit_)
	{
		// 後方の空き領域が20%未満か、空き領域に入りきらないなので、ページ分割
		pPrevPage->split(pNextPage, pEntry1_, pEntry2_);
	}
	else
	{
		// 後方は十分空いているので、再配分
		pPrevPage->redistribute(pNextPage, pEntry1_, pEntry2_);
	}
}

//
//	FUNCTION private
//	FullText2::BtreePage::reduce -- 縮小のための操作を行う
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
BtreePage::reduce()
{
	PagePointer pPrevPage = 0;
	PagePointer pNextPage = 0;

	if (getNextPageID() != PhysicalFile::ConstValue::UndefinedPageID)
	{
		pPrevPage = this;
		// 後方のページをattachする
		pNextPage = m_cFile.attachPage(getNextPageID(), m_uiStepCount);
	}
	else
	{
		// 前方のページをattachする
		pPrevPage = m_cFile.attachPage(getPrevPageID(), m_uiStepCount);
		pNextPage = this;
	}

	if (pPrevPage->getFreeUnitSize() > getPageUnitSize () / 2
		&& pNextPage->getFreeUnitSize() > getPageUnitSize () / 2)
	{
		// ページ連結
		pPrevPage->concatenate(pNextPage);
	}
	else
	{
		// 再配置
		pPrevPage->redistribute(pNextPage);
	}
}

//
//	FUNCTION private
//	FullText2::BtreePage::split -- ページ分割を行う
//
//	NOTES
//	B木のページ分割は2ページを3ページに分割する
//	引数で与えられたページは自分のひとつ後ろのページであり、
//	自分のページと次のページの間に新しいページを作成し、
//	2->3の分割を行う
//
//	ARGUMENTS
//	FullText2::PagePointer pNextPage_
//		1つ後ろのページ
//	const FullText2::BtreePage::Entry* pEntry1_
//		挿入するエントリまたは、更新するキー
//	const FullText2::BtreePage::Entry* pEntry2_
//		更新するエントリ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::split(PagePointer pNextPage_,
				 const Entry* pEntry1_, const Entry* pEntry2_)
{
	// ページのユニット数
	int totalUnit = getPageUnitSize();

	// 新しいページを確保
	PagePointer pMiddle = m_cFile.allocatePage(getID(), pNextPage_->getID(),
											   m_uiStepCount);
	setNextPageID(pMiddle->getID());
	pNextPage_->setPrevPageID(pMiddle->getID());
	// 自分がリーフならお隣さんもリーフ
	if (isLeaf() == true) pMiddle->setLeaf();

	//
	//	まずはこのページの1/3をpMiddleに移動する
	//

	// このページに残すエントリを計算する
	Iterator s = begin();
	ModUInt32* b = syd_reinterpret_cast<ModUInt32*>(*begin());
	for (; s < end(); ++s)
	{
		if ((syd_reinterpret_cast<ModUInt32*>(*s) - b) >= totalUnit*2/3)
			// 2/3を超えたので終わり
			break;
	}
	// 新しいページに移動する
	pMiddle->insert(s, end());	// 段数変化あり
	if (pMiddle->getStepCount() != getStepCount())
	{
		// 段数が変わった
		setStepCount(pMiddle->getStepCount());
		pNextPage_->setStepCount(pMiddle->getStepCount());
	}
	// このページから移動した分を削除する
	expunge(s, end());	// 段数変化なし

	//
	//	次にpNextPage_の1/3をpMiddleに移動する
	//
	s = pNextPage_->begin();
	b = syd_reinterpret_cast<ModUInt32*>(*pNextPage_->begin());
	for (; s < pNextPage_->end(); ++s)
	{
		if ((syd_reinterpret_cast<ModUInt32*>(*s) - b) >= totalUnit*1/3)
			// 1/3を超えたので終わり
			break;
	}
	// 新しいページに移動する
	pMiddle->insert(pNextPage_->begin(), s);	// 段数変化なし
	// pNextPage_から移動した分を削除する
	pNextPage_->expunge(pNextPage_->begin(), s);	// 段数変化あり
	if (pNextPage_->getStepCount() != getStepCount())
	{
		// 段数が変わった
		setStepCount(pNextPage_->getStepCount());
		pMiddle->setStepCount(pNextPage_->getStepCount());
	}

	if (pEntry1_)
	{
		// 挿入するべきエントリを挿入する

		// まずはどのページに挿入するべきかを求める
		Less cLess;
		if (cLess(pEntry1_, *pMiddle->begin()) == ModTrue)
		{
			if (pEntry2_ == 0)
				// このページに挿入する
				insert(*pEntry1_);
			else
				// このページで更新する
				update(*pEntry1_, *pEntry2_);
		}
		else if (cLess(pEntry1_, *pNextPage_->begin()) == ModTrue) 
		{
			if (pEntry2_ == 0)
				// 新しいページに挿入する
				pMiddle->insert(*pEntry1_);
			else
				// 新しいページで更新する
				pMiddle->update(*pEntry1_, *pEntry2_);
		}
		else
		{
			if (pEntry2_ == 0)
				// pNextPage_に挿入する
				pNextPage_->insert(*pEntry1_);
			else
				// pNextPage_で更新する
				pNextPage_->update(*pEntry1_, *pEntry2_);
		}
	}
}

//
//	FUNCTION private
//	FullText2::BtreePage::redistribute -- 再配分を行う
//
//	NOTES
//	次ページと同じような容量になるように、再配分を行う
//
//	ARGUMENTS
//	FullText2::PagePointer pNextPage_
//		次のページ
//	const FullText2::BtreePage::Entry* pEntry1_
//		挿入するエントリまたは、更新するキー (default 0)
//	const FullText2::BtreePage::Entry* pEntry2_
//		更新するエントリ (default 0)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::redistribute(PagePointer pNextPage_,
						const Entry* pEntry1_, const Entry* pEntry2_)
{
	// 半分のユニット数を求める
	int half = (getUsedUnitSize() + pNextPage_->getUsedUnitSize()) / 2;

	if (getUsedUnitSize() < pNextPage_->getUsedUnitSize())
	{
		// 次ページから移動してくるユニット数
		int move = pNextPage_->getUsedUnitSize() - half;

		// 次ページに残すエントリを計算する
		Iterator s = pNextPage_->begin();
		ModUInt32* b = syd_reinterpret_cast<ModUInt32*>(*pNextPage_->begin());
		for (; s < pNextPage_->end(); s++)
		{
			if ((syd_reinterpret_cast<ModUInt32*>(*s) - b) >= move)
				// moveを超えたので終わり
				break;
		}
		// このページに移動する
		insert(pNextPage_->begin(), s);		// 段数変化なし
		// 次ページから移動した分を削除する
		pNextPage_->expunge(pNextPage_->begin(), s);	// 段数変化あり
		if (pNextPage_->getStepCount() != getStepCount())
		{
			setStepCount(pNextPage_->getStepCount());
		}
	}
	else
	{
		// このページに残すエントリを計算する
		Iterator s = begin();
		ModUInt32* b = syd_reinterpret_cast<ModUInt32*>(*begin());
		for (; s < end(); s++)
		{
			if ((syd_reinterpret_cast<ModUInt32*>(*s) - b) >= half)
				// halfを超えたので終わり
				break;
		}
		// 次ページに移動する
		pNextPage_->insert(s, end());	// 段数変化あり
		if (pNextPage_->getStepCount() != getStepCount())
		{
			setStepCount(pNextPage_->getStepCount());
		}
		// このページから移動した分を削除する
		expunge(s, end());		// 段数変化なし
	}

	if (pEntry1_)
	{
		// 挿入するべきエントリを挿入する
		Less cLess;
		if (cLess(pEntry1_, *pNextPage_->begin()) == ModTrue)
		{
			if (pEntry2_ == 0)
				// このページに挿入する
				insert(*pEntry1_);
			else
				// このページで更新する
				update(*pEntry1_, *pEntry2_);
		}
		else
		{
			if (pEntry2_ == 0)
				// 次ページに挿入する
				pNextPage_->insert(*pEntry1_);
			else
				// 次ページで更新する
				pNextPage_->update(*pEntry1_, *pEntry2_);
		}
	}
}

//
//	FUNCTION private
//	FullText2::BtreePage::concatenate -- ページ連結を行う
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::PagePointer pNextPage_
//		連結するページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::concatenate(PagePointer pNextPage_)
{
	// pNextPage_のエントリをこのページに移動する
	insert(pNextPage_->begin(), pNextPage_->end());

	// pNextPage_の次ページを自分の次ページに設定する
	PhysicalFile::PageID uiNextPageID = pNextPage_->getNextPageID();
	setNextPageID(uiNextPageID);
	if (uiNextPageID != PhysicalFile::ConstValue::UndefinedPageID)
	{
		// pNextPage_の次ページの前ページを自分に設定する
		PagePointer pNextNextPage = m_cFile.attachPage(uiNextPageID,
													   m_uiStepCount);
		pNextNextPage->setPrevPageID(getID());
	}

	if (isLeaf() == true &&
	   pNextPage_->getNextPageID() == PhysicalFile::ConstValue::UndefinedPageID)
	{
		// 右端のページを削除するので、このページが右端になる
		m_cFile.setRightLeafPage(this);
	}

	// pNextPage_の内容を削除する
	pNextPage_->expunge(pNextPage_->begin(), pNextPage_->end());

	// pNextPage_を削除する
	m_cFile.freePage(pNextPage_);
}

//
//	FUNCTION private
//	FullText2::BtreePage::updateParent --- 親ノードを更新する
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::BtreePage::Entry* pKey1_
//		更新元
//	const FullText2::BtreePage::Entry* pKey2_
//		更新先
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BtreePage::updateParent(const Entry* pKey1_, const Entry* pKey2_)
{
	if (isRoot())
		// 親が設定されていないので、何もしない
		return;

	// 親ノードを得る
	PagePointer pParent = m_cFile.getNodePage(*(pKey1_ ? pKey1_ : pKey2_),
											  m_uiStepCount - 1);

	bool status = true;

	if (pKey1_ == 0)
	{
		// 挿入
		status = pParent->insert(*pKey2_);
	}
	else if (pKey2_ == 0)
	{
		// 削除
		status = pParent->expunge(*pKey1_);
	}
	else
	{
		// 更新
		status = pParent->update(*pKey1_, *pKey2_);
	}

	if (status == false)
	{
		// 段数が変わったかも
		m_uiStepCount = pParent->getStepCount() + 1;
	}
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
