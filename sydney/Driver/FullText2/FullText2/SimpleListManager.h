// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SimpleListManager.h --
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

#ifndef __SYDNEY_FULLTEXT2_SIMPLELISTMANAGER_H
#define __SYDNEY_FULLTEXT2_SIMPLELISTMANAGER_H

#include "FullText2/Module.h"
#include "FullText2/UpdateListManager.h"
#include "FullText2/LeafPage.h"
#include "FullText2/InvertedList.h"
#include "FullText2/InvertedUnit.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::SimpleListManager --
//
//	NOTES
//
class SimpleListManager : public UpdateListManager
{
public:
	// コンストラクタ
	SimpleListManager(FullTextFile& cFile_,
					  InvertedUnit* pInvertedUnit_);
	// デストラクタ
	virtual ~SimpleListManager();
	// コピーコンストラクタ
	SimpleListManager(const SimpleListManager& cSrc_);

	// キー文字列の取得
	const ModUnicodeString& getKey() const;
	
	// 先頭に位置付けられた反復子の取得
	// (得たポインタは delete すること)
	ListIterator* getIterator();

	// リストを割り当てる
	bool reset(const ModUnicodeString& cstrKey_,
			   AccessMode::Value eAccessMode_);
	// 次のリストに移動
	bool next();

	// コピーを得る
	ListManager* copy() const;

	//
	//	以下は検索以外のためのインターフェース
	//
	
	// 転置リストを得る
	InvertedList* getInvertedList();
	
	// 挿入する
	void insert(ModUInt32 uiDocumentID_,
				const SmartLocationList& cLocationList_);
	void insert(InvertedList& cInvertedList_);

	// 削除する
	void expunge(ModUInt32 uiDocumentID_);
	int expunge(InvertedList& cInvertedList_);

	// 削除を取り消す
	void undoExpunge(ModUInt32 uiDocumentID_,
					 const SmartLocationList& cLocationList_);

	// 整合性検査を行う
	void verify(Admin::Verification::Treatment::Value uiTreatment_,
				Admin::Verification::Progress& cProgress_,
				const Os::Path& cRootPath_);

	// 同じものが登録されているかチェックする
	bool check(ModUInt32 uiDocumentID_,
			   const SmartLocationList& cLocationList_);

	// IDブロックを削除する
	void expungeIdBlock(const ModVector<ModUInt32>& vecDocumentID_);

	// バキュームが必要かどうか
	bool isNeedVacuum(int iNewExpungeCount_);
	// バキュームする
	void vacuum();

private:
	// 転置リストを作成する
	InvertedList* makeInvertedList(LeafPage::PagePointer pPage_,
								   LeafPage::Iterator ite_);
	// 転置リストを作成する
	InvertedList* makeShortList(const ModUnicodeString& cstrKey_);
	// 転置リストを作成する
	InvertedList* makeShortList(LeafPage::PagePointer pPage_,
								LeafPage::Iterator ite_);
	// 転置リストを作成する
	InvertedList* makeMiddleList(LeafPage::PagePointer pPage_,
								 LeafPage::Iterator ite_);

	// 転置ファイル
	InvertedUnit* m_pInvertedUnit;

	// 転置リストクラス
	InvertedList* m_pInvertedList;

	// リーフページ
	LeafPage::PagePointer m_pLeafPage;
	// リーフページイテレータ
	LeafPage::Iterator m_ite;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_SIMPLELISTMANAGER_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
