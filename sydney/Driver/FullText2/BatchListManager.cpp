// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BatchListManager.cpp --
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

#include "FullText2/BatchListManager.h"
#include "FullText2/DelayListIterator.h"
#include "FullText2/InvertedBatch.h"
#include "FullText2/InvertedIterator.h"

#include "Exception/BadArgument.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::BatchListManager::BatchListManager -- コンストラクタ
//
//	NOTES
//	FullText2::FullTextFile& cFile_
//		全文索引ファイル
//	FullText2::InvertedBatch* pInvertedBatch_
//		バッチ用ファイル
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
BatchListManager::BatchListManager(FullTextFile& cFile_,
								   InvertedBatch* pInvertedBatch_)
	: UpdateListManager(cFile_), m_pInvertedBatch(pInvertedBatch_)
{
}

//
//	FUNCTION public
//	FullText2::BatchListManager::~BatchListManager -- デストラクタ
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
BatchListManager::~BatchListManager()
{
}

//
//	FUNCTION public
//	FullText2::BatchListManager::BatchListManager -- コピーコンストラクタ
//
//	NOTES
//	const FullText2::BatchListManager& cSrc_
//		コピー元
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
BatchListManager::BatchListManager(const BatchListManager& cSrc_)
	: UpdateListManager(cSrc_), m_pInvertedBatch(cSrc_.m_pInvertedBatch)
{
}

//
//	FUNCTION public
//	FullText2::BatchListManager::copy -- コピーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::ListManager*
//		コピー
//
//	EXCEPTIONS
//
ListManager*
BatchListManager::copy() const
{
	return new BatchListManager(*this);
}

//
//	FUNCTION public
//	FullText2::BatchListManager::getKey -- 索引単位を得る
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
BatchListManager::getKey() const
{
	return (*m_iterator).first;
}

//
//	FUNCTION public
//	FullText2::BatchListManager::getIterator -- 転置リストイテレータを得る
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
BatchListManager::getIterator()
{
	ListIterator* i = 0;
	ModList<BatchBaseList*>::Iterator j = (*m_iterator).second.begin();
	
	if ((*m_iterator).second.getSize() > 1)
	{
		DelayListIterator* tmp = new DelayListIterator();
		for (; j != (*m_iterator).second.end(); ++j)
		{
			tmp->pushBack((*j)->getIterator(),
						  (*j)->getArea()->getLastDocumentID());
		}
		i = tmp;
	}
	else
	{
		i = (*j)->getIterator();
	}

	m_bSearchMode = true;
	
	return i;
}

//
//	FUNCTION public
//	FullText2::BatchListManager::reset -- 転置リストを割り当てる
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
BatchListManager::reset(const ModUnicodeString& cstrKey_,
						AccessMode::Value eAccessMode_)
{
	bool result = true;

	// まずは、m_pInvertedBatch のマップを下限検索する
	m_iterator = m_pInvertedBatch->lowerBound(cstrKey_);

	if (eAccessMode_ == AccessMode::Create ||
		eAccessMode_ == AccessMode::Search)
	{
		// 一致しているか確認する

		if (m_iterator == m_pInvertedBatch->end() ||
			(*m_iterator).first != cstrKey_)

			result = false;
		
		if (result == false &&
			eAccessMode_ == AccessMode::Create)
		{
			// 存在していないので、新しいものを作成する

			m_iterator = m_pInvertedBatch->addList(cstrKey_);

			result = true;
		}
	}
	else
	{
		// AccessMode::LowerBound
		if (m_iterator == m_pInvertedBatch->end())

			result = false;
	}

	if (result)
		m_listIterator = (*m_iterator).second.begin();
	
	return result;
}

//
//	FUNCTION public
//	FullText2::BatchListManager::next -- 次を得る
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
BatchListManager::next()
{
	if (m_iterator == m_pInvertedBatch->end())
		// 終了
		return false;

	// 次の転置リストへ
	++m_listIterator;
	
	if (m_bSearchMode ||
		m_listIterator == (*m_iterator).second.end())
	{
		// 次の索引単位へ

		++m_iterator;
		if (m_iterator == m_pInvertedBatch->end())
			// 終了
			return false;
		
		m_listIterator = (*m_iterator).second.begin();
	}
	
	return true;
}

//
//	FUNCTION public
//	FullText2::BatchListManager::getInvertedList -- 転置リストを取得する
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
BatchListManager::getInvertedList()
{
	m_bSearchMode = false;

	return *m_listIterator;
}

//
//	FUNCTION public
//	FullText2::BatchListManager::insert -- 挿入する
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
BatchListManager::insert(ModUInt32 uiDocumentID_,
						 const SmartLocationList& cLocationList_)
{
	for (;;)
	{
		m_listIterator = (*m_iterator).second.end();
		--m_listIterator;
	
		if ((*m_listIterator)->insert(uiDocumentID_, cLocationList_) == true)
			// 挿入できた
			break;

		// 挿入できないので、新しいリストを追加する
		(*m_iterator).second.pushBack(
			m_pInvertedBatch->makeList((*m_iterator).first));
	}
}

//
//	FUNCTION public
//	FullText2::BatchListManager::insert -- 挿入する
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
BatchListManager::insert(InvertedList& cInvertedList_)
{
	_TRMEISTER_THROW0(Exception::BadArgument);
}

//
//	FUNCTION public
//	FullText2::BatchListManager::expunge -- 削除する
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
BatchListManager::expunge(ModUInt32 uiDocumentID_)
{
	m_listIterator = (*m_iterator).second.begin();
	for (; m_listIterator != (*m_iterator).second.end(); ++m_listIterator)
	{
		if ((*m_listIterator)->getMaxDocumentID() >= uiDocumentID_)
		{
			// この転置リスト内にある
			(*m_listIterator)->expunge(uiDocumentID_);

			break;
		}
	}
}

//
//	FUNCTION public
//	FullText2::BatchListManager::expunge -- 削除する
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
BatchListManager::expunge(InvertedList& cInvertedList_)
{
	_TRMEISTER_THROW0(Exception::BadArgument);
}

//
//	FUNCTION public
//	FullText2::BatchListManager::undoExpunge -- 削除の取り消しを行う
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
BatchListManager::undoExpunge(ModUInt32 uiDocumentID_,
							  const SmartLocationList& cLocationList_)
{
	m_listIterator = (*m_iterator).second.begin();
	for (; m_listIterator != (*m_iterator).second.end(); ++m_listIterator)
	{
		if ((*m_listIterator)->getMaxDocumentID() >= uiDocumentID_)
		{
			// この転置リスト内にあった
			(*m_listIterator)->undoExpunge(uiDocumentID_, cLocationList_);

			break;
		}
	}
}

//
//	FUNCTION public
//	FullText2::BatchListManager::verify -- 整合性検査を行う
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
BatchListManager::verify(Admin::Verification::Treatment::Value uiTreatment_,
						  Admin::Verification::Progress& cProgress_,
						  const Os::Path& cRootPath_)
{
}

//
//	FUNCTION public
//	FullText2::BatchListManager::check
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
BatchListManager::check(ModUInt32 uiDocumentID_,
						const SmartLocationList& cLocationList_)
{
	_TRMEISTER_THROW0(Exception::BadArgument);
}

//
//	FUNCTION public
//	FullText2::BatchListManager::expungeIdBlock -- IDブロックを削除する
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
BatchListManager::expungeIdBlock(const ModVector<ModUInt32>& vecDocumentID_)
{
	_TRMEISTER_THROW0(Exception::BadArgument);
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
