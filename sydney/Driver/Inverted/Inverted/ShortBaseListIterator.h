// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortBaseListIterator.h --
// 
// Copyright (c) 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_SHORTBASELISTITERATOR_H
#define __SYDNEY_INVERTED_SHORTBASELISTITERATOR_H

#include "Inverted/InvertedIterator.h"
#include "Inverted/LeafPage.h"
#include "Inverted/Module.h"

class ModInvertedLocationListIterator;
class ModInvertedSmartLocationList;

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class InvertedList;

//
//	CLASS
//	Inverted::ShortBaseListIterator --
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

	// 次へ進める
	void next();
	// 先頭に戻る
	void reset();
	
	// 文書IDを検索する
	ModBoolean find(const DocumentID uiDocumentID_, bool bUndo_);
	// 文書IDをlower_bound検索する
	ModBoolean lowerBound(const DocumentID uiDocumentID_, bool bUndo_);

	// 終端か
	ModBoolean isEnd() const { return (m_uiCurrentID == UndefinedDocumentID) ? ModTrue : ModFalse; }

	// 文書IDを得る
	DocumentID getDocumentId() { return m_uiCurrentID; }
	
	// 位置情報内の文書頻度を得る
	virtual ModSize getInDocumentFrequency();
	// 位置情報へのイテレータを得る
	virtual ModInvertedLocationListIterator* getLocationListIterator();
	
	// 現在位置のデータを削除する
	virtual void expunge();
	// 現在位置にデータを挿入する
	virtual void undoExpunge(ModUInt32 uiDocumentID_, const ModInvertedSmartLocationList& cLocationList_);

	// 現在の位置情報のオフセットを得る
	ModSize getLocationOffset();
	// 現在の位置情報のビット長を得る
	ModSize getLocationBitLength();
	// 現在の位置情報データのオフセットを得る
	ModSize getLocationDataOffset();
	// 現在の位置情報データのビット長を得る
	ModSize getLocationDataBitLength();

protected:
	// 位置情報の位置を文書IDの位置まで移動する
	virtual void synchronize();
	
	// 位置の先頭に戻る
	virtual void resetLocation();
	
	// 現在位置のデータを削除する
	void expungeDocumentID();
	// 位置情報を削除する
	virtual void expungeLocation();

	// 文書IDの削除を取り消す
	void undoExpungeDocumentID(ModUInt32 uiDocumentID_);
	// 位置情報の削除を取り消す
	virtual void undoExpungeLocation(const ModInvertedSmartLocationList& cLocationList_);
	
	//
	// メンバ変数
	// 本来ならサブクラスで宣言しアクセッサを使うべきだが、
	// 何万回も呼ばれるとアクセッサのコストも無視できないので、
	// 共通するメンバ変数はBaseクラスで宣言する。
	//

	// 転置リスト
	InvertedList& m_cInvertedList;
	// LeafPageのエリア
	LeafPage::Area* m_pArea;
	// エリアの終端アドレス
	ModUInt32* m_pTailAddress;
	
	// 現在の位置
	int m_iCurrentPosition;

	// 直前の文書ID
	DocumentID m_uiPrevID;
	// 現在の文書ID
	DocumentID m_uiCurrentID;
	ModSize m_uiCurrentOffset;
	ModSize m_uiNextOffset;
	
private:
	// 位置情報変数を設定する
	virtual void setLocationInformation();
	// 位置情報を読み飛ばす
	virtual void skipLocationInformation();

	// アクセッサ
	// 位置情報
	virtual int getLocPosition() const;
	virtual void setLocPosition(int uiPosition_);
	virtual void incrementLocPosition();
	virtual ModSize getLocOffset() const;
	virtual void setLocOffset(ModSize uiOffset_);
	virtual ModSize getLocLength() const;
	virtual void setLocLength(ModSize uiLength_);
	virtual ModSize getLocDataOffset() const;
	virtual ModSize getLocDataLength() const;
	virtual ModSize getTermFrequency() const;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_SHORTBASELISTITERATOR_H

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
