// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ListManager.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Inverted/ListManager.h"
#include "Inverted/InvertedUnit.h"
#include "Inverted/InvertedList.h"
#include "Inverted/InvertedIterator.h"
#include "Inverted/LeafPage.h"
#include "Inverted/ShortList.h"
#include "Inverted/ShortNolocationList.h"
#include "Inverted/ShortNolocationNoTFList.h"
#include "Inverted/MiddleBaseList.h"
#include "Inverted/MiddleList.h"
#include "Inverted/MiddleNolocationList.h"
#include "Inverted/MiddleNolocationNoTFList.h"
#include "Inverted/MessageAll_Class.h"
#include "ModInvertedSmartLocationList.h"

#include "Common/Assert.h"
#include "Exception/VerifyAborted.h"
#include "Exception/Cancel.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::ListManager::ListManager -- コンストラクタ
//
//	NOTES
//	Inverted::InvertedUnit* pInvertedUnit_
//		 転置ファイルクラス
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
ListManager::ListManager(InvertedUnit* pInvertedUnit_)
: m_pInvertedUnit(pInvertedUnit_), m_pInvertedList(0), m_pLeafPage(0), m_ite(0)
{
}

//
//	FUNCTION public
//	Inverted::ListManager::~ListManager -- デストラクタ
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
ListManager::~ListManager()
{
	if (m_pInvertedList) delete m_pInvertedList;
}

//
//	FUNCTION public
//	Inverted::ListManager::getDocumentFrequency -- 文書頻度を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		現在のリストの文書頻度
//
//	EXCEPTIONS
//
ModSize
ListManager::getDocumentFrequency() const
{
	return m_pInvertedList->getArea()->getDocumentCount();
}

//
//	FUNCTION public
//	Inverted::ListManager::getKey -- 索引単位を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUnicodeString& cstrKey_
//		索引単位
//
//	EXCEPTIONS
//
const ModUnicodeString&
ListManager::getKey() const
{
	return m_pInvertedList->getKey();
}

//
//	FUNCTION public
//	Inverted::ListManager::begin -- 転置リストイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInvertedIterator*
//		転置リストイテレータ
//
//	EXCEPTIONS
//
ModInvertedIterator*
ListManager::begin() const
{
	return m_pInvertedList->begin();
}

//
//	FUNCTION public
//	Inverted::ListManager::clone -- 自分の複製を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInvertedList*
//		自分の複製
//
//	EXCEPTIONS
//
ModInvertedList*
ListManager::clone() const
{
	ListManager* pListManager = new ListManager(m_pInvertedUnit);
	pListManager->m_pLeafPage = m_pLeafPage;
	pListManager->m_ite = m_ite;
	if (isSetList())
		pListManager->getInvertedList();

	return pListManager;
}

//
//	FUNCTION public
//	Inverted::ListManager::reset -- 転置リストを割り当てる
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//	const ModInvertedListAccessMode eAccessMode_
//		アクセスモード
//
//	RETURN
//	ModBoolean
//		該当する転置リストが存在した場合はModTrue、それ以外の場合はModFalse
//
ModBoolean
ListManager::reset(const ModUnicodeString& cstrKey_, const ModInvertedListAccessMode eAccessMode_)
{
	ModBoolean bResult = ModFalse;

	// 前回見つかった転置リストを削除する
	if (m_pInvertedList)
	{
		delete m_pInvertedList, m_pInvertedList = 0;
	}

	if (m_pLeafPage)
	{
		// 同じページにあるかもしれないので、ページを検索する
		m_ite = m_pLeafPage->lowerBound(cstrKey_);

		if (m_ite != m_pLeafPage->end())
		{
			// [NOTE] cstrKey_に対応する転置リストが存在するなら、
			//  このページ内に存在するべき。
			//  lowerBound()がend()以外を返すのは、
			//  ページ内にcstrKey_より大きいデータが存在することを意味する。
			//  また、cstrKey_より小さいデータの存在も以下より保障されている。
			//  この関数は、昇順のcstrKey_を順番に呼び出すので、
			//  このページがアタッチされているということは、
			//  現在のcstrKey_より小さい、直前のcstrKey_が、
			//  このページ内に存在することを意味しているため。
			switch (eAccessMode_)
			{
			case ModInvertedListLowerBoundMode:
				bResult = ModTrue;
				break;
			case ModInvertedListCreateMode:
			case ModInvertedListSearchMode:
				{
					ModUnicodeString cstrKey((*m_ite)->getKey(), (*m_ite)->getKeyLength());
					if (cstrKey == cstrKey_)
					{
						bResult = ModTrue;
					}
					else if (eAccessMode_ == ModInvertedListCreateMode)
					{
						// 無かった -> とりあえずショートリストを作る
						m_pInvertedList = makeShortList(cstrKey_);
						bResult = ModTrue;
					}
				}
				break;
			}
		}
		else
		{
			// このページにはないのでdetachする
			m_pLeafPage = 0;
			m_ite = 0;
		}
	}
	
	//
	//	B木を検索する
	//
	//	転置ファイルはcreate時に空文字列のエントリをB木とLeafFileに挿入している
	//	ので、この検索でB木がfalseを返すことはない。
	//	ただし、作成遅延でファイルがまだ作成されていない場合はfalseが返る。
	//	しかし、CreateModeではこのようなことはありえない。
	//
	PhysicalFile::PageID uiPageID;
	if (bResult == ModFalse && m_pInvertedUnit &&
		m_pInvertedUnit->searchBtree(cstrKey_, uiPageID) == true)
	{
		// リーフページをアタッチする
		m_pLeafPage = m_pInvertedUnit->attachLeafPage(uiPageID);
		// lower_boundで検索する
		m_ite = m_pLeafPage->lowerBound(cstrKey_);
		
		// アクセスモードによって処理が違う
		switch (eAccessMode_)
		{
		case ModInvertedListLowerBoundMode:
			{
				bResult = ModTrue;
				if (m_ite == m_pLeafPage->end())
				{
					uiPageID = m_pLeafPage->getNextPageID();
					if (uiPageID != PhysicalFile::ConstValue::UndefinedPageID)
					{
						m_pLeafPage = m_pInvertedUnit->attachLeafPage(uiPageID);
						m_ite = m_pLeafPage->begin();
					}
					else
					{
						bResult = ModFalse;
					}
				}
			}
			break;
		case ModInvertedListCreateMode:
			{
				// lower_boundで検索する
				ModUnicodeString cstrKey;
				if (m_ite != m_pLeafPage->end())
				{
					cstrKey.append((*m_ite)->getKey(), (*m_ite)->getKeyLength());
				}

				if (cstrKey != cstrKey_)
				{
					// 無かった -> とりあえずショートリストを作る
					m_pInvertedList = makeShortList(cstrKey_);
				}

				bResult = ModTrue;
			}
			break;
		case ModInvertedListSearchMode:
			{
				if (m_ite != m_pLeafPage->end())
				{
					ModUnicodeString cstrKey((*m_ite)->getKey(), (*m_ite)->getKeyLength());
					if (cstrKey == cstrKey_)
					{
						bResult = ModTrue;
					}
				}
			}
			break;
		}

		if (bResult == ModFalse)
		{
			// デタッチする
			m_pLeafPage = 0;
			m_ite = 0;
		}

	}

	if (bResult == ModTrue)
	{
		getInvertedList();
	}

	return bResult;
}

//
//	FUNCTION public
//	Inverted::ListManager::next -- 次を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModBoolean
//		次の転置リストが存在する場合はModTrue、それ以外の場合はModFalse
//
//	EXCEPTIONS
//
ModBoolean
ListManager::next()
{
	LeafPage::Iterator i = 0;

	// 前の転置リスト
	if (m_pInvertedList)
	{
		delete m_pInvertedList, m_pInvertedList = 0;
	}

	if (m_pLeafPage == 0)
		return ModFalse;

	// イテレータが割り当てられている
	++m_ite;

	while (m_ite == m_pLeafPage->end())
	{
		// ページの終わりなので、次のページ
		PhysicalFile::PageID uiNextPageID = m_pLeafPage->getNextPageID();

		m_ite = 0;
		m_pLeafPage = 0;

		if (uiNextPageID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			// もう次のページは無いので終了
			return ModFalse;
		}

		m_pLeafPage = m_pInvertedUnit->attachLeafPage(uiNextPageID);
		m_ite = m_pLeafPage->begin();
	}

	getInvertedList();

	return ModTrue;
}

//
//	FUNCTION public
//	Inverted::ListManager::getLastDocumentID -- 最終文書IDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInvertedDocumentID
//		最終文書ID
//
//	EXCEPTIONS
//
ModInvertedDocumentID
ListManager::getLastDocumentID()
{
	return m_pInvertedList->getArea()->getLastDocumentID();
}

//
//	FUNCTION public
//	Inverted::ListManager::getInvertedList -- 転置リストを取得する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Inverted::InvertedList*
//		転置リスト
//
//	EXCEPTIONS
//
InvertedList*
ListManager::getInvertedList()
{
	if (m_pInvertedList == 0)
	{
		m_pInvertedList = makeInvertedList(m_pLeafPage, m_ite);
	}
	return m_pInvertedList;
}

//
//	FUNCTION public
//	Inverted::ListManager::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//	const ModInvertedSmartLocationList& cLocationList_
//		位置情報配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ListManager::insert(ModUInt32 uiDocumentID_, const ModInvertedSmartLocationList& cLocationList_)
{
	if (cLocationList_.getSize() == 0)
		return;

	for (;;)
	{
		if (m_pInvertedList->insert(uiDocumentID_, cLocationList_) == true)
			// 挿入できた
			break;

		// 今の形式では挿入できない -> 大きな転置リストに変換する
		InvertedList* pNewList = m_pInvertedList->convert();
		delete m_pInvertedList;
		m_pInvertedList = pNewList;
	}

	// 挿入したページとイテレータを得る
	m_pLeafPage = m_pInvertedList->getLeafPage();
	m_ite = m_pInvertedList->getLeafPageIterator();
}

//
//	FUNCTION public
//	Inverted::ListManager::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::InvertedList& cInvertedList_
//		転置リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ListManager::insert(InvertedList& cInvertedList_)
{
	for (;;)
	{
		if (m_pInvertedList->insert(cInvertedList_) == true)
			// 挿入できた
			break;

		// 今の形式では挿入できない -> 大きな転置リストに変換する
		InvertedList* pNewList = m_pInvertedList->convert();
		delete m_pInvertedList;
		m_pInvertedList = pNewList;
	}

	// 挿入したページとイテレータを得る
	m_pLeafPage = m_pInvertedList->getLeafPage();
	m_ite = m_pInvertedList->getLeafPageIterator();
}

//
//	FUNCTION public
//	Inverted::ListManager::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ListManager::expunge(ModUInt32 uiDocumentID_)
{
	m_pInvertedList->expunge(uiDocumentID_);
}

//
//	FUNCTION public
//	Inverted::ListManager::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::InvertedList& cInvertedList_
//		転置リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ListManager::expunge(InvertedList& cInvertedList_)
{
	m_pInvertedList->expunge(cInvertedList_);
}

//
//	FUNCTION public
//	Inverted::ListManager::undoExpunge -- 削除の取り消しを行う
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//	const ModInvertedSmartLocationList& cLocationList_
//		位置情報配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ListManager::undoExpunge(ModUInt32 uiDocumentID_, const ModInvertedSmartLocationList& cLocationList_)
{
	m_pInvertedList->undoExpunge(uiDocumentID_, cLocationList_);
}

//
//	FUNCTION public
//	Inverted::ListManager::verify -- 整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//	Admin::Verification::Treatment::Value uiTreatment_
//		不整合が発見された場合の処理
//	Admin::Verification::Progress& cProgress_
//		整合性検査結果を報告するストリーム
//	const Os::Path& cRootPath_
//		転置ファイルのルートパス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ListManager::verify(Admin::Verification::Treatment::Value uiTreatment_,
					Admin::Verification::Progress& cProgress_,
					const Os::Path& cRootPath_)
{
	ModUnicodeString cstrKey;
	if (reset(cstrKey, ModInvertedListSearchMode) == ModFalse)
	{
		_SYDNEY_VERIFY_INCONSISTENT(cProgress_, cRootPath_, Message::NullStringNotInserted());
		_SYDNEY_THROW0(Exception::VerifyAborted);
	}

	do
	{
		if (m_pInvertedUnit->isCancel() == true)
		{
			_SYDNEY_THROW0(Exception::Cancel);
		}

		// 索引単位がB木に格納されているかチェックする
		PhysicalFile::PageID uiPageID;
		if (m_pInvertedUnit->searchBtree(getKey(), uiPageID) == false
			|| uiPageID != m_pLeafPage->getID())
		{
			_SYDNEY_VERIFY_INCONSISTENT(cProgress_, cRootPath_, Message::IllegalIndex());
			_SYDNEY_THROW0(Exception::VerifyAborted);
		}

		// 索引単位が昇順に並んでいるかチェックする
		if (getKey().getLength() != 0 && cstrKey >= getKey())
		{
			_SYDNEY_VERIFY_INCONSISTENT(cProgress_, cRootPath_, Message::IllegalIndex());
			_SYDNEY_THROW0(Exception::VerifyAborted);
		}

		// 転置リスト内をチェックする
		m_pInvertedList->verify(uiTreatment_, cProgress_, cRootPath_);

		cstrKey = getKey();
	}
	while (next() == ModTrue);
}

//
//	FUNCTION public
//	Inverted::ListManager::check -- 同じものが登録されているかチェックする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//	const ModInvertedSmartLocationList& cLocationList_
//		位置情報配列
//
//	RETURN
//	bool
//		同じだった場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
ListManager::check(ModUInt32 uiDocumentID_, const ModInvertedSmartLocationList& cLocationList_)
{
	bool result = false;
	ModAutoPointer<InvertedIterator> i = m_pInvertedList->begin();

	// 文書IDをチェックする
	if ((*i).find(uiDocumentID_) == ModTrue)
	{
		// 文書IDは存在した

		if (m_pInvertedList->isNolocation() == true)
		{
			// 索引に位置リストが格納されていない

			// TFだけでもチェックする

			// 転置リストのTFとcLocationList_のリスト数とを比較
			_SYDNEY_ASSERT(m_pInvertedList->isNoTF() == false);
			if ((*i).getInDocumentFrequency() == cLocationList_.getSize())
			{
				// 一致した
				result = true;
			}
		}
		else
		{
			// 位置情報がすべて同じかチェックする
			ModAutoPointer<ModInvertedLocationListIterator> j = (*i).getLocationListIterator();
			ModAutoPointer<ModInvertedLocationListIterator> k = cLocationList_.begin();
			while ((*j).isEnd() == ModFalse && (*k).isEnd() == ModFalse)
			{
				if ((*j).getLocation() != (*k).getLocation())
					// 位置が違っている
					break;
				(*j).next();
				(*k).next();
			}
			if ((*j).isEnd() == ModTrue && (*k).isEnd() == ModTrue) result = true;
		}
	}

	return result;
}

//
//	FUNCTION public
//	Inverted::ListManager::expungeIdBlock -- IDブロックを削除する
//
//	NOTES
//
//	ARGUMENTS
//	const ModVector<ModUInt32>& vecDocumentID_
//		削除するIDブロックの先頭文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ListManager::expungeIdBlock(const ModVector<ModUInt32>& vecDocumentID_)
{
	if (m_pInvertedList->getListType() == ListType::Middle)
	{
		MiddleBaseList* pMiddleList = _SYDNEY_DYNAMIC_CAST(MiddleBaseList*, m_pInvertedList);
		pMiddleList->expungeIdBlock(vecDocumentID_);
	}
}

//
//	FUNCTION private
//	Inverted::ListManager::makeInvertedList -- 転置リストを作成する
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::LeafPage::PagePointer pPage_
//		リーフページ
//	Inverted::LeafPage::Iterator ite_
//		該当するエリアへのイテレータ
//
//	RETURN
//	Inverted::InvertedList*
//		転置リスト
//
//	EXCEPTIONS
//
InvertedList*
ListManager::makeInvertedList(LeafPage::PagePointer pPage_, LeafPage::Iterator ite_)
{
	InvertedList* pList = 0;

	if ((*ite_)->getListType() == ListType::Short)
	{
		// ショートリスト
		pList = makeShortList(pPage_, ite_);
	}
	else
	{
		// ミドルリスト
		pList = makeMiddleList(pPage_, ite_);
	}

	return pList;
}

//
//	FUNCTION private
//	Inverted::ListManager::makeShortList -- 転置リストを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//
//	RETURN
//	Inverted::InvertedList*
//		転置リスト
//
//	EXCEPTIONS
//
InvertedList*
ListManager::makeShortList(const ModUnicodeString& cstrKey_)
{
	//
	// 該当するエリアが存在しない時に使われる関数なので、引数にはキーが必要
	//
	
	InvertedList* pList = 0;

	if (m_pInvertedUnit->isNolocation() == true)
	{
		if (m_pInvertedUnit->isNoTF() == true)
		{
			pList = new ShortNolocationNoTFList(*m_pInvertedUnit, cstrKey_, m_pLeafPage, m_ite);
		}
		else
		{
			pList = new ShortNolocationList(*m_pInvertedUnit, cstrKey_, m_pLeafPage, m_ite);
		}
	}
	else
	{
		pList = new ShortList(*m_pInvertedUnit, cstrKey_, m_pLeafPage, m_ite);
	}

	return pList;
}

//
//	FUNCTION private
//	Inverted::ListManager::makeShortList -- 転置リストを作成する
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::LeafPage::PagePointer pPage_
//		リーフページ
//	Inverted::LeafPage::Iterator ite_
//		該当するエリアへのイテレータ
//
//	RETURN
//	Inverted::InvertedList*
//		転置リスト
//
//	EXCEPTIONS
//
InvertedList*
ListManager::makeShortList(LeafPage::PagePointer pPage_, LeafPage::Iterator ite_)
{
	//
	// 該当するエリアが存在する時に使われる関数なので、キーは不要。
	// pPage_とite_も、実際はm_pLeafPageとm_ite。
	//
	
	InvertedList* pList = 0;

	if (m_pInvertedUnit->isNolocation() == true)
	{
		if (m_pInvertedUnit->isNoTF() == true)
		{
			pList = new ShortNolocationNoTFList(*m_pInvertedUnit, pPage_, ite_);
		}
		else
		{
			pList = new ShortNolocationList(*m_pInvertedUnit, pPage_, ite_);
		}
	}
	else
	{
		pList = new ShortList(*m_pInvertedUnit, pPage_, ite_);
	}

	return pList;
}

//
//	FUNCTION private
//	Inverted::ListManager::makeMiddleList -- 転置リストを作成する
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::LeafPage::PagePointer pPage_
//		リーフページ
//	Inverted::LeafPage::Iterator ite_
//		該当するエリアへのイテレータ
//
//	RETURN
//	Inverted::InvertedList*
//		転置リスト
//
//	EXCEPTIONS
//
InvertedList*
ListManager::makeMiddleList(LeafPage::PagePointer pPage_, LeafPage::Iterator ite_)
{
	//
	// 該当するエリアが存在する時に使われる方のmakeShortListの、MiddleList版
	//
	
	InvertedList* pList = 0;

	if (m_pInvertedUnit->isNolocation() == true)
	{
		if (m_pInvertedUnit->isNoTF() == true)
		{
			pList = new MiddleNolocationNoTFList(*m_pInvertedUnit, pPage_, ite_);
		}
		else
		{
			pList = new MiddleNolocationList(*m_pInvertedUnit, pPage_, ite_);
		}
	}
	else
	{
		pList = new MiddleList(*m_pInvertedUnit, pPage_, ite_);
	}

	return pList;
}

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
