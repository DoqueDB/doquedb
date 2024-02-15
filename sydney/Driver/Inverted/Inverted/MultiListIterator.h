// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiListIterator.h --
// 
// Copyright (c) 2005, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_MULTILISTITERATOR_H
#define __SYDNEY_INVERTED_MULTILISTITERATOR_H

#include "Inverted/Module.h"
#include "Inverted/Types.h"

#include "ModInvertedIterator.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class DocumentIDVectorFile;
class RowIDVectorFile2;

//
//	CLASS
//	Inverted::MultiListIterator --
//
//	NOTES
//
//
class MultiListIterator : public ModInvertedIterator
{
public:
	//コンストラクタ
	MultiListIterator(DocumentIDVectorFile* pDocumentIDVectorFile_,
					  RowIDVectorFile2* pRowIDVectorFile2_);
	//デストラクタ
	virtual ~MultiListIterator();

	// リザーブする
	void reserve(ModSize size)
	{
		m_vecpIterator.reserve(size);
	}
	// 追加する
	void pushBack(ModInvertedIterator* iterator)
	{
		m_vecpIterator.pushBack(iterator);
	}
	// 先頭を設定する
	void set();

	// 次へ進める
	void next();
	// 先頭に戻る
	void reset();
	
	// 文書IDを検索する
	ModBoolean find(const DocumentID uiDocumentID_);
	// 文書IDをlower_bound検索する
	ModBoolean lowerBound(const DocumentID uiDocumentID_);

	// 終端か
	ModBoolean isEnd() const { return (m_uiCurrentID == UndefinedDocumentID) ? ModTrue : ModFalse; }

	// 文書IDを得る
	DocumentID getDocumentId() { return m_uiCurrentID; }

	// 位置情報内の文書頻度を得る
	ModSize getInDocumentFrequency();
	// 位置情報へのイテレータを得る
	ModInvertedLocationListIterator* getLocationListIterator();

private:
	// 対象文書IDのユニット番号を得る
	int getUnit(DocumentID uiDocumentID_);
	
	// 現在の文書ID
	DocumentID m_uiCurrentID;
	// その他の文書IDの最小値
	DocumentID m_uiOtherMinimumID;
	// 現在の文書IDを得ている配列要素番号
	int m_iCurrentElement;
	
	// 文書IDベクタ
	DocumentIDVectorFile* m_pDocumentIDVectorFile;
	// ROWIDベクタ
	RowIDVectorFile2* m_pRowIDVectorFile2;

	// ModInvertedIteratorの配列
	ModVector<ModInvertedIterator*> m_vecpIterator;

	// findした直後かどうか
	bool m_bFind;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_MULTILISTITERATOR_H

//
//	Copyright (c) 2005, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
