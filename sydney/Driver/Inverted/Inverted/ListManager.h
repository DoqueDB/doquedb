// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ListManager.h --
// 
// Copyright (c) 2002, 2004, 2005, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_LISTMANAGER_H
#define __SYDNEY_INVERTED_LISTMANAGER_H

#include "Inverted/Module.h"
#include "Inverted/LeafPage.h"
#include "Inverted/InvertedList.h"
#include "Inverted/InvertedUnit.h"

#include "ModInvertedTypes.h"
#include "ModInvertedList.h"

class ModInvertedSmartLocationList;

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

//
//	CLASS
//	Inverted::ListManager --
//
//	NOTES
//	ModInvertedListを実装するクラス
//	ModInvertedListというクラスはショートリストとかミドルリストとかの
//	基底クラスではなく、転置リストを検索して取り出すクラスである
//
class ListManager : public ModInvertedList
{
public:
	// コンストラクタ
	ListManager(InvertedUnit* pInvertedUnit_);
	// デストラクタ
	virtual ~ListManager();

	// 文書頻度の取得
	ModSize getDocumentFrequency() const;

	// キー文字列の取得
	const ModUnicodeString& getKey() const;

	// 先頭に位置付けられた反復子の取得
	// (得たポインタは delete すること)
	ModInvertedIterator* begin() const;

	// 自分の複製を作る
	ModInvertedList* clone() const;

	// リストを割り当てる
	ModBoolean reset(const ModUnicodeString& cstrKey_,
					 const ModInvertedListAccessMode eAccessMode_);

	// 次のリストに移動
	ModBoolean next();

	// 最終文書IDを得る
	ModInvertedDocumentID getLastDocumentID();

	// 位置情報を格納しているか？
	ModBoolean isNolocation()
	{
		return m_pInvertedUnit->isNolocation() ? ModTrue : ModFalse;
	}

	// 今転置リストが割り当てられているか
	bool isSetList() const { return m_pInvertedList != 0; }

	// 転置リストを得る
	InvertedList* getInvertedList();
	
	// 挿入する
	void insert(ModUInt32 uiDocumentID_,
				const ModInvertedSmartLocationList& cLocationList_);
	void insert(InvertedList& cInvertedList_);

	// 削除する
	void expunge(ModUInt32 uiDocumentID_);
	void expunge(InvertedList& cInvertedList_);

	// 削除を取り消す
	void undoExpunge(ModUInt32 uiDocumentID_,
					 const ModInvertedSmartLocationList& cLocationList_);

	// 整合性検査を行う
	void verify(Admin::Verification::Treatment::Value uiTreatment_,
				Admin::Verification::Progress& cProgress_,
				const Os::Path& cRootPath_);

	// 同じものが登録されているかチェックする
	bool check(ModUInt32 uiDocumentID_,
			   const ModInvertedSmartLocationList& cLocationList_);

	// IDブロックを削除する
	void expungeIdBlock(const ModVector<ModUInt32>& vecDocumentID_);

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

	// 転置ファイルクラス
	InvertedUnit* m_pInvertedUnit;

	// 転置リストクラス
	InvertedList* m_pInvertedList;

	// リーフページ
	LeafPage::PagePointer m_pLeafPage;
	// リーフページイテレータ
	LeafPage::Iterator m_ite;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_LISTMANAGER_H

//
//	Copyright (c) 2002, 2004, 2005, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
