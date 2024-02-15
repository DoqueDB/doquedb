// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SimpleListManager.cpp --
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
#include "SyDynamicCast.h"

#include "FullText2/SimpleListManager.h"
#include "FullText2/InvertedUnit.h"
#include "FullText2/InvertedList.h"
#include "FullText2/InvertedIterator.h"
#include "FullText2/LeafPage.h"
#include "FullText2/ShortList.h"
#include "FullText2/ShortNolocationList.h"
#include "FullText2/ShortNolocationNoTFList.h"
#include "FullText2/MiddleBaseList.h"
#include "FullText2/MiddleList.h"
#include "FullText2/MiddleNolocationList.h"
#include "FullText2/MiddleNolocationNoTFList.h"
#include "FullText2/MessageAll_Class.h"

#include "FullText2/FakeError.h"

#include "Common/Assert.h"
#include "Exception/VerifyAborted.h"
#include "Exception/Cancel.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::SimpleListManager::SimpleListManager -- コンストラクタ
//
//	NOTES
//	FullText2::FullTextFile& cFile_
//		全文索引ファイル
//	FullText2::InvertedUnit* pInvertedUnit_
//		転置ユニット
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
SimpleListManager::SimpleListManager(FullTextFile& cFile_,
									 InvertedUnit* pInvertedUnit_)
	: UpdateListManager(cFile_), m_pInvertedUnit(pInvertedUnit_),
	  m_pInvertedList(0), m_pLeafPage(0), m_ite(0)
{
}

//
//	FUNCTION public
//	FullText2::SimpleListManager::~SimpleListManager -- デストラクタ
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
SimpleListManager::~SimpleListManager()
{
	if (m_pInvertedList) delete m_pInvertedList;
}

//
//	FUNCTION public
//	FullText2::SimpleListManager::SimpleListManager -- コンストラクタ
//
//	NOTES
//	FullText2::FullTextFile& cFile_
//		全文索引ファイル
//	FullText2::InvertedUnit* pInvertedUnit_
//		転置ユニット
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
SimpleListManager::SimpleListManager(const SimpleListManager& cSrc_)
	: UpdateListManager(cSrc_), m_pInvertedUnit(cSrc_.m_pInvertedUnit),
	  m_pInvertedList(0), m_pLeafPage(0), m_ite(0)
{
}

//
//	FUNCTION public
//	FullText2::SimpleListManager::getKey -- 索引単位を得る
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
SimpleListManager::getKey() const
{
	return m_pInvertedList->getKey();
}

//
//	FUNCTION public
//	FullText2::SimpleListManager::getIterator -- 転置リストイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ListIterator*
//		転置リストイテレータ
//
//	EXCEPTIONS
//
ListIterator*
SimpleListManager::getIterator()
{
	return m_pInvertedList->getIterator();
}

//
//	FUNCTION public
//	FullText2::SimpleListManager::reset -- 転置リストを割り当てる
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//	FullText2::ListManager::AccessMode::Value eAccessMode_
//		アクセスモード
//
//	RETURN
//	bool
//		該当する転置リストが存在した場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
SimpleListManager::reset(const ModUnicodeString& cstrKey_,
						 AccessMode::Value eAccessMode_)
{
	bool bResult = false;

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
			switch (eAccessMode_)
			{
			case AccessMode::LowerBound:
				if (m_ite != m_pLeafPage->begin())
				{
					// このページの先頭ではなかったので、
					// lower_boundの結果はこのページ中に存在する
					
					bResult = true;
				}
				break;
			case AccessMode::Create:
			case AccessMode::Search:
				{
					ModUnicodeString cstrKey((*m_ite)->getKey(),
											 (*m_ite)->getKeyLength());
					if (cstrKey == cstrKey_)
					{
						bResult = true;
					}
					else if (eAccessMode_ == AccessMode::Create &&
							 m_ite != m_pLeafPage->begin())
					{
						// あるならこのページにあるはずなのに無かった
						// -> とりあえずショートリストを作る
						
						m_pInvertedList = makeShortList(cstrKey_);
						bResult = true;
					}
				}
				break;
			}
		}

		if (bResult == false)
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
	if (bResult == false && m_pInvertedUnit &&
		m_pInvertedUnit->searchBtree(cstrKey_, uiPageID) == true)
	{
		// リーフページをアタッチする
		m_pLeafPage = m_pInvertedUnit->attachLeafPage(uiPageID);
		// lower_boundで検索する
		m_ite = m_pLeafPage->lowerBound(cstrKey_);
		
		// アクセスモードによって処理が違う
		switch (eAccessMode_)
		{
		case AccessMode::LowerBound:
			{
				bResult = true;
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
						bResult = false;
					}
				}
			}
			break;
		case AccessMode::Create:
			{
				// lower_boundで検索する
				ModUnicodeString cstrKey;
				if (m_ite != m_pLeafPage->end())
				{
					cstrKey.append((*m_ite)->getKey(),
								   (*m_ite)->getKeyLength());
				}

				if (cstrKey != cstrKey_)
				{
					// 無かった -> とりあえずショートリストを作る
					m_pInvertedList = makeShortList(cstrKey_);
				}

				bResult = true;
			}
			break;
		case AccessMode::Search:
			{
				if (m_ite != m_pLeafPage->end())
				{
					ModUnicodeString cstrKey((*m_ite)->getKey(),
											 (*m_ite)->getKeyLength());
					if (cstrKey == cstrKey_)
					{
						bResult = true;
					}
				}
			}
			break;
		}

		if (bResult == false)
		{
			// デタッチする
			m_pLeafPage = 0;
			m_ite = 0;
		}

	}

	if (bResult == true)
	{
		getInvertedList();
	}

	return bResult;
}

//
//	FUNCTION public
//	FullText2::SimpleListManager::next -- 次を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		次の転置リストが存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
SimpleListManager::next()
{
	LeafPage::Iterator i = 0;

	// 前の転置リスト
	if (m_pInvertedList)
	{
		delete m_pInvertedList, m_pInvertedList = 0;
	}

	if (m_pLeafPage == 0)
		return false;

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
			return false;
		}

		m_pLeafPage = m_pInvertedUnit->attachLeafPage(uiNextPageID);
		m_ite = m_pLeafPage->begin();
	}

	getInvertedList();

	return true;
}

//
//	FUNCTION public
//	FullText2::SimpleListManager::copy -- コピーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETRUN
//	FullText2::ListManager*
//		コピーしたListManager
//
//	EXCEPTIONS
//
ListManager*
SimpleListManager::copy() const
{
	return new SimpleListManager(*this);
}

//
//	FUNCTION public
//	FullText2::SimpleListManager::getInvertedList -- 転置リストを取得する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::InvertedList*
//		転置リスト
//
//	EXCEPTIONS
//
InvertedList*
SimpleListManager::getInvertedList()
{
	if (m_pInvertedList == 0)
	{
		m_pInvertedList = makeInvertedList(m_pLeafPage, m_ite);
	}
	return m_pInvertedList;
}

//
//	FUNCTION public
//	FullText2::SimpleListManager::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//	const FullText2::SmartLocationList& cLocationList_
//		位置情報配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleListManager::insert(ModUInt32 uiDocumentID_,
						  const SmartLocationList& cLocationList_)
{
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
//	FullText2::SimpleListManager::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedList& cInvertedList_
//		転置リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleListManager::insert(InvertedList& cInvertedList_)
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
//	FullText2::SimpleListManager::expunge -- 削除する
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
SimpleListManager::expunge(ModUInt32 uiDocumentID_)
{
	m_pInvertedList->expunge(uiDocumentID_);
}

//
//	FUNCTION public
//	FullText2::SimpleListManager::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedList& cInvertedList_
//		転置リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
int
SimpleListManager::expunge(InvertedList& cInvertedList_)
{
	return m_pInvertedList->expunge(cInvertedList_);
}

//
//	FUNCTION public
//	FullText2::SimpleListManager::undoExpunge -- 削除の取り消しを行う
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//	const FullText2::SmartLocationList& cLocationList_
//		位置情報配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
SimpleListManager::undoExpunge(ModUInt32 uiDocumentID_,
							   const SmartLocationList& cLocationList_)
{
	m_pInvertedList->undoExpunge(uiDocumentID_, cLocationList_);
}

//
//	FUNCTION public
//	FullText2::SimpleListManager::verify -- 整合性検査を行う
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
SimpleListManager::verify(Admin::Verification::Treatment::Value uiTreatment_,
						  Admin::Verification::Progress& cProgress_,
						  const Os::Path& cRootPath_)
{
	ModUnicodeString cstrKey;
	if (reset(cstrKey, AccessMode::Search) == false)
	{
		_SYDNEY_VERIFY_INCONSISTENT(cProgress_, cRootPath_,
									Message::NullStringNotInserted());
		_SYDNEY_THROW0(Exception::VerifyAborted);
	}

	do
	{
		// 索引単位がB木に格納されているかチェックする
		PhysicalFile::PageID uiPageID;
		if (m_pInvertedUnit->searchBtree(getKey(), uiPageID) == false
			|| uiPageID != m_pLeafPage->getID())
		{
			_SYDNEY_VERIFY_INCONSISTENT(cProgress_, cRootPath_,
										Message::IllegalIndex());
			_SYDNEY_THROW0(Exception::VerifyAborted);
		}

		// 索引単位が昇順に並んでいるかチェックする
		if (getKey().getLength() != 0 && cstrKey >= getKey())
		{
			_SYDNEY_VERIFY_INCONSISTENT(cProgress_, cRootPath_,
										Message::IllegalIndex());
			_SYDNEY_THROW0(Exception::VerifyAborted);
		}

		// 転置リスト内をチェックする
		m_pInvertedList->verify(uiTreatment_, cProgress_, cRootPath_);

		cstrKey = getKey();

		// for Debug
		; _FULLTEXT2_FAKE_ERROR(SimpleListManager::verify);
	}
	while (next() == ModTrue);
}

//
//	FUNCTION public
//	FullText2::SimpleListManager::check
//		-- 同じものが登録されているかチェックする
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//	const FullText2::SmartLocationList& cLocationList_
//		位置情報配列
//
//	RETURN
//	bool
//		同じだった場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
SimpleListManager::check(ModUInt32 uiDocumentID_,
						 const SmartLocationList& cLocationList_)
{
	bool result = false;
	ModAutoPointer<InvertedIterator> i = m_pInvertedList->getIterator();

	// 文書IDをチェックする
	if ((*i).find(uiDocumentID_, false) == true)
	{
		// 文書IDは存在した

		if (m_pInvertedList->isNoTF() == true)
		{
			// 文書IDの情報しかないので、一致したとみなす
			
			result = true;
		}
		else if (m_pInvertedList->isNolocation() == true)
		{
			// 索引に位置リストが格納されていない

			// TFだけでもチェックする

			// 転置リストのTFとcLocationList_のリスト数とを比較
			if ((*i).getTermFrequency() == cLocationList_.getCount())
			{
				// 一致した
				result = true;
			}
		}
		else
		{
			// 位置情報がすべて同じかチェックする
			LocationListIterator::AutoPointer j
				= (*i).getLocationListIterator();
			LocationListIterator::AutoPointer k
				= cLocationList_.getIterator();
			
			result = true;	// 一旦 true にする
			ModSize posJ, posK;

			do
			{
				int dummy;
				posJ = (*j).next(dummy);
				posK = (*k).next(dummy);

				if (posJ != posK)
				{
					// 位置が違っている
					result = false;
					break;
				}

			}
			while (posJ != UndefinedLocation);
		}
	}

	return result;
}

//
//	FUNCTION public
//	FullText2::SimpleListManager::expungeIdBlock -- IDブロックを削除する
//
//	NOTES
//	このメソッドは UpdateListManager から呼び出されるメソッドである
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
SimpleListManager::expungeIdBlock(const ModVector<ModUInt32>& vecDocumentID_)
{
	if (m_pInvertedList->getListType() == ListType::Middle)
	{
		MiddleBaseList* pMiddleList
			= _SYDNEY_DYNAMIC_CAST(MiddleBaseList*, m_pInvertedList);
		pMiddleList->expungeIdBlock(vecDocumentID_);
	}
}

//
//	FUNCTION private
//	FullText2::SimpleListManager::makeInvertedList -- 転置リストを作成する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::LeafPage::PagePointer pPage_
//		リーフページ
//	FullText2::LeafPage::Iterator ite_
//		該当するエリアへのイテレータ
//
//	RETURN
//	FullText2::InvertedList*
//		転置リスト
//
//	EXCEPTIONS
//
InvertedList*
SimpleListManager::makeInvertedList(LeafPage::PagePointer pPage_,
									LeafPage::Iterator ite_)
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
//	FullText2::SimpleListManager::makeShortList -- 転置リストを作成する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		索引単位
//
//	RETURN
//	FullText2::InvertedList*
//		転置リスト
//
//	EXCEPTIONS
//
InvertedList*
SimpleListManager::makeShortList(const ModUnicodeString& cstrKey_)
{
	//
	// 該当するエリアが存在しない時に使われる関数なので、引数にはキーが必要
	//
	
	InvertedList* pList = 0;

	if (m_pInvertedUnit->isNolocation() == true)
	{
		if (m_pInvertedUnit->isNoTF() == true)
		{
			pList = new ShortNolocationNoTFList(*m_pInvertedUnit,
												cstrKey_, m_pLeafPage, m_ite);
		}
		else
		{
			pList = new ShortNolocationList(*m_pInvertedUnit,
											cstrKey_, m_pLeafPage, m_ite);
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
//	FullText2::SimpleListManager::makeShortList -- 転置リストを作成する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::LeafPage::PagePointer pPage_
//		リーフページ
//	FullText2::LeafPage::Iterator ite_
//		該当するエリアへのイテレータ
//
//	RETURN
//	FullText2::InvertedList*
//		転置リスト
//
//	EXCEPTIONS
//
InvertedList*
SimpleListManager::makeShortList(LeafPage::PagePointer pPage_,
								 LeafPage::Iterator ite_)
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
//	FullText2::SimpleListManager::makeMiddleList -- 転置リストを作成する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::LeafPage::PagePointer pPage_
//		リーフページ
//	FullText2::LeafPage::Iterator ite_
//		該当するエリアへのイテレータ
//
//	RETURN
//	FullText2::InvertedList*
//		転置リスト
//
//	EXCEPTIONS
//
InvertedList*
SimpleListManager::makeMiddleList(LeafPage::PagePointer pPage_,
								  LeafPage::Iterator ite_)
{
	//
	// 該当するエリアが存在する時に使われる方のmakeShortListの、MiddleList版
	//
	
	InvertedList* pList = 0;

	if (m_pInvertedUnit->isNolocation() == true)
	{
		if (m_pInvertedUnit->isNoTF() == true)
		{
			pList = new MiddleNolocationNoTFList(*m_pInvertedUnit,
												 pPage_, ite_);
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
//	FUNCTION public
//	FullText2::SimpleListManager::isNeedVacuum
//		-- バキュームが必要かどうかを確認する
//
//	NOTES
//
//	ARGUMETNS
//	int iNewExpungeCount_
//		今回の削除数
//
//	RETURN
//	bool
//		バキュームが必要な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
SimpleListManager::isNeedVacuum(int iNewExpungeCount_)
{
	bool result = false;
	
	if (iNewExpungeCount_ != 0 &&
		m_pInvertedList->getListType() == ListType::Middle)
	{
		// 今回の削除数が0ではなく、対象の転置リストがミドルリストだったら、
		// B木を確認する

		result = m_pInvertedUnit->isNeedVacuum(getKey(), iNewExpungeCount_);
	}
	
	return result;
}

//
//	FUNCTION public
//	FullText2::SimpleListManager::vacuum
//		-- バキュームする
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
SimpleListManager::vacuum()
{
	m_pInvertedList->vacuum();
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
