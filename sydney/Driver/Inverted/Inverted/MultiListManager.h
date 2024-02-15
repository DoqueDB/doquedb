// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiListManager.h --
// 
// Copyright (c) 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_MULTILISTMANAGER_H
#define __SYDNEY_INVERTED_MULTILISTMANAGER_H

#include "Inverted/Module.h"
#include "Inverted/LeafPage.h"
#include "Inverted/InvertedList.h"

#include "ModVector.h"

#include "ModInvertedTypes.h"
#include "ModInvertedList.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class InvertedUnit;
class DocumentIDVectorFile;
class RowIDVectorFile2;
class ListManager;

//
//	CLASS
//	Inverted::MultiListManager --
//
//	NOTES
//	ModInvertedListを実装するクラス(検索用)
//	ModInvertedListというクラスはショートリストとかミドルリストとかの基底クラスではなく、
//	転置リストを検索して取り出すクラスである
//
class MultiListManager : public ModInvertedList
{
public:
	// コンストラクタ
	MultiListManager(DocumentIDVectorFile* pDocumentIDVectorFile_,
					 RowIDVectorFile2* pRowIDVectorFile2_,
					 InvertedUnit* pInvertedUnit_,
					 int iUnitCount_);

	// デストラクタ
	virtual ~MultiListManager();

	// コピーコンストラクタ
	MultiListManager(const MultiListManager& other_);

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
	ModBoolean isNolocation() { return m_bNolocation; }

private:
	// ListManagerを破棄する
	void destruct();
	
	// 文書IDベクター
	DocumentIDVectorFile* m_pDocumentIDVectorFile;
	// ROWIDベクター
	RowIDVectorFile2* m_pRowIDVectorFile2;

	// 各ユニットごとのListManager
	ModVector<ListManager*> m_vecpListManager;

	// 一番小さい索引単位
	ModUnicodeString m_cstrKey;

	// 最終文書ID
	ModInvertedDocumentID m_uiLastDocumentID;

	// 位置情報を格納しているか？
	ModBoolean m_bNolocation;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_MULTILISTMANAGER_H

//
//	Copyright (c) 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
