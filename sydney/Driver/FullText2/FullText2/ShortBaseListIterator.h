// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortListIterator.h --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_SHORTBASELISTITERATOR_H
#define __SYDNEY_FULLTEXT2_SHORTBASELISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/InvertedIterator.h"
#include "FullText2/InvertedList.h"
#include "FullText2/LeafPage.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class LocationListIterator;

//
//	CLASS
//	FullText2::ShortBaseListIterator --
//
//	NOTES
//
//
class ShortBaseListIterator : public InvertedIterator
{
public:
	//コンストラクタ
	ShortBaseListIterator(InvertedList& cInvertedList_);
	//デストラクタ
	virtual ~ShortBaseListIterator();

	// おおよその文書数を得る
	ModSize getEstimateCount()
		{
			// ここは正確な数
			return m_pArea->getDocumentCount();
		}
	// 索引単位の長さを得る
	int getLength();

	// 先頭に戻る
	void reset()
		{
			m_uiCurrentID = 0;
			m_iCurrentPosition = -1;
		}
	// 次へ進める
	DocumentID next() { return nextImpl(); }
	
	// 文書IDを検索する
	bool find(DocumentID uiDocumentID_, bool bUndo_);
	// 文書IDをlower_bound検索する
	DocumentID lowerBound(DocumentID uiDocumentID_, bool bUndo_);

protected:
	// 次の値を得る
	DocumentID nextImpl();
	// 終端か
	bool isEndImpl() { return (m_uiCurrentID == UndefinedDocumentID); }

	// 文書IDを削除する
	void expungeDocumentID();
	// 文書IDの削除を取り消す
	void undoExpungeDocumentID(ModUInt32 uiDocumentID_);

	//【注意】
	//	メンバー変数を protected にして、サブクラスで触れるようにするのは、
	//	本来ならお行儀がわるいことだが、速度を優先し、アクセッサは用意しない

	// 転置リスト
	InvertedListCore m_cInvertedList;
	// LeafPage
	LeafPage::PagePointer m_pLeafPage;
	// LeafPageのエリア
	LeafPage::Area* m_pArea;
	// エリアの先頭アドレス
	ModUInt32* m_pHeadAddress;
	// エリアの終端アドレス
	ModUInt32* m_pTailAddress;

	// 現在の位置
	int m_iCurrentPosition;
	// 直前の文書ID
	DocumentID m_uiPrevID;
	// 現在の文書ID
	DocumentID m_uiCurrentID;
	// 現在の文書IDのオフセット
	ModSize m_uiCurrentOffset;
	// 次の文書IDのオフセット
	ModSize m_uiNextOffset;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_SHORTLISTITERATOR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
