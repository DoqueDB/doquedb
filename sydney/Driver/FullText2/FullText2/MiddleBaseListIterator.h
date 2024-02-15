// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleBaseListIterator.h --
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

#ifndef __SYDNEY_FULLTEXT2_MIDDLEBASELISTITERATOR_H
#define __SYDNEY_FULLTEXT2_MIDDLEBASELISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/InvertedIterator.h"
#include "FullText2/InvertedListCore.h"
#include "FullText2/LeafPage.h"
#include "FullText2/OverflowPage.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class InvertedUnit;
class MiddleBaseList;
class OverflowFile;

//
//	CLASS
//	FullText2::MiddleBaseListIterator --
//
//	NOTES
//
//
class MiddleBaseListIterator : public InvertedIterator
{
public:
	// コンストラクタ
	MiddleBaseListIterator(MiddleBaseList& cMiddleBaseList_);
	// デストラクタ
	virtual ~MiddleBaseListIterator();

	// おおよその文書数を得る
	ModSize getEstimateCount()
		{
			// ここは正確な値
			return m_pArea->getDocumentCount();
		}
	// 索引単位の長さを得る
	int getLength();

	// 次の値を得る
	DocumentID next() { return nextImpl(false); }
	// 先頭に戻る
	void reset() { resetImpl(); }
	
	// 文書IDを検索する
	bool find(DocumentID uiDocumentID_, bool bUndo_);
	// 文書IDをlower_bound検索する
	DocumentID lowerBound(DocumentID uiDocumentID_, bool bUndo_);

	// IDブロックを削除する
	virtual bool expungeIdBlock() = 0;

	// 先頭文書ID削除のログを得る
	ModUInt32 getExpungeFirstDocumentID(ModUInt32 uiOldDocumentID_);
	// 不用なIDブロックを登録する
	void enterDeleteIdBlock(ModUInt32 uiFirstDocumentID_);
	// 先頭文書IDの削除ログを登録する
	void enterExpungeFirstDocumentID(ModUInt32 uiOldDocumentID_,
									 ModUInt32 uiNewDocumentID_);
	
protected:
	// 初期化する
	void resetImpl();
	// 終端か
	bool isEndImpl() { return (m_uiCurrentID == UndefinedDocumentID); }
	// 次に進める
	DocumentID nextImpl(bool bUndo_);

	// 先頭文書IDを設定する
	void setFirstDocumentID();
	// 先頭文書IDを設定する
	void setFirstDocumentID(OverflowPage::IDBlock& cIdBlock_);
	// 次のIDブロックへ移動する
	bool nextIdBlock();

	// 先頭文書IDを削除する
	void expungeFirstDocumentID();
	// 文書IDを削除する
	void expungeDocumentID();
	// 文書IDの削除を取り消す
	void undoExpungeDocumentID(ModUInt32 uiDocumentID_);

	//【注意】
	//	メンバー変数を protected にして、サブクラスで触れるようにするのは、
	//	本来ならお行儀がわるいことだが、速度を優先し、アクセッサは用意しない

	// オーバーフローファイル
	OverflowFile* m_pOverflowFile;
	// IDブロック
	OverflowPage::IDBlock m_cIdBlock;
	// LOCブロック
	OverflowPage::LocBlock m_cLocBlock;
	// IDページ
	OverflowPage::PagePointer m_pIdPage;

	// DIRブロック
	LeafPage::DirBlock* m_pDirBlock;
	// 現在のDIRブロック位置
	int m_iDirBlockPosition;
	// 現在のIDブロック位置(IDページ中の)
	ModSize m_uiIDBlockPosition;

	// 転置ファイル
	InvertedUnit& m_cInvertedUnit;

	// 転置リスト
	InvertedListCore m_cInvertedList;
	// LeafPage
	LeafPage::PagePointer m_pLeafPage;
	// LeafPageのエリア
	LeafPage::Area* m_pArea;

	// 現在の位置
	int m_iCurrentPosition;
	// 直前の文書ID
	ModUInt32 m_uiPrevID;
	// 現在の文書ID
	ModUInt32 m_uiCurrentID;
	// 現在の文書IDのオフセット
	ModSize m_uiCurrentOffset;
	// 次の文書IDのオフセット
	ModSize m_uiNextOffset;

	// 現在のIDブロックの上限文書ID
	ModUInt32 m_uiLastDocumentID;

	// 位置情報と同期しているか
	bool m_bSynchronized;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_MIDDLEBASELISTITERATOR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
