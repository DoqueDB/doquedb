// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleBaseListIterator.h --
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

#ifndef __SYDNEY_INVERTED_MIDDLEBASELISTITERATOR_H
#define __SYDNEY_INVERTED_MIDDLEBASELISTITERATOR_H

#include "Inverted/InvertedIterator.h"
#include "Inverted/LeafPage.h"
#include "Inverted/Module.h"
#include "Inverted/OverflowPage.h"

class ModInvertedLocationListIterator;
class ModInvertedSmartLocationList;

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class InvertedList;
class MiddleBaseList;
class MiddleListLocationListIterator;
class OverflowFile;

//
//	CLASS
//	Inverted::MiddleBaseListIterator --
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

	// 次へ進める
	void next();
	// 先頭に戻る
	void reset();
	
	// 文書IDを検索する
	ModBoolean find(const DocumentID uiDocumentID_, bool bUndo_);
	// 文書IDをlower_bound検索する
	ModBoolean lowerBound(const DocumentID uiDocumentID_, bool bUndo_);

	// 終端か
	ModBoolean isEnd() const
	{
		return (m_uiCurrentID == UndefinedDocumentID)
			? ModTrue : ModFalse;
	}

	// 文書IDを得る
	DocumentID getDocumentId();

	// 位置情報内の文書頻度を得る
	virtual ModSize getInDocumentFrequency();
	// 位置情報へのイテレータを得る
	virtual ModInvertedLocationListIterator* getLocationListIterator();

	// 現在位置のデータを削除する
	virtual void expunge();

	// 現在位置にデータを挿入する
	virtual void undoExpunge(ModUInt32 uiDocumentID_,
							 const ModInvertedSmartLocationList& cLocationList_);

	// 位置情報の先頭アドレスを得る
	virtual ModUInt32* getHeadAddress();
	// 現在の位置情報のオフセットを得る
	virtual ModSize getLocationOffset();
	// 現在の位置情報のビット長を得る
	virtual ModSize getLocationBitLength();
	// 現在の位置情報データのオフセットを得る
	virtual ModSize getLocationDataOffset();
	// 現在の位置情報データのビット長を得る
	virtual ModSize getLocationDataBitLength();

	// IDブロックを削除する
	virtual bool expungeIdBlock();

	// 不要な位置情報クラスをpushする
	virtual void pushBack(MiddleListLocationListIterator* pFree_);

protected:
	// 位置情報の位置を文書IDの位置まで移動する
	void synchronize(bool bUndo_ = false) { doSynchronize(bUndo_); }
	
	// 先頭文書IDを設定する
	void setFirstDocumentID();
	// 先頭文書IDを設定する
	void setFirstDocumentID(const OverflowPage::IDBlock& cIdBlock_);
	// 次のIDブロックへ移動する
	bool nextIdBlock();
	// 次のLOCブロックへ移動する
	virtual bool nextLocBlock(ModSize uiOddLength_ = 0);

	// 先頭文書IDを削除する
	void expungeFirstDocumentID();
	// 文書IDを削除する
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

	// オーバーフローファイル
	OverflowFile* m_pOverflowFile;
	// IDブロック
	OverflowPage::IDBlock m_cIdBlock;
	// IDページ
	OverflowPage::PagePointer m_pIdPage;

	// DIRブロック
	LeafPage::DirBlock* m_pDirBlock;
	// 現在のDIRブロック位置
	int m_iDirBlockPosition;
	// 現在のIDブロック位置(IDページ中の)
	ModSize m_uiIDBlockPosition;

	// 転置リスト
	MiddleBaseList& m_cInvertedList;
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

	//
	// 以下は、NoTFでは使わないが高速化のためここで宣言する
	//
	
	// 現在の文書頻度
	ModSize m_uiCurrentFrequency;
	
	// 位置情報と同期しているか
	bool m_bSynchronized;
	
private:
	// 次へ進める (引数1個)
	void next(bool bUndo_);
	// nextの下請け
	void next2();
	
	// 位置情報の位置を文書IDの位置まで移動する (引数1個)
	virtual void doSynchronize(bool bUndo_) = 0;
	
	// 位置情報変数を設定する
	virtual void setLocationInformation();

	//
	// アクセッサ
	//
	
	// LOCページ
	virtual OverflowPage::PagePointer getLocPage() const;
	virtual void setLocPage(OverflowPage* pPage_);
	
	// LOCブロック
	virtual OverflowPage::LocBlock& getLocBlock();
	virtual void setLocBlock(OverflowPage::LocBlock cLocBlock_);
	virtual void setLocBlockByEmpty() = 0;
	// LOCブロックのデータ部の先頭アドレス
	virtual ModUInt32* getLocBlockDataHeadAddress();
	virtual void setLocBlockDataHeadAddress(ModUInt32* p_);
	// LOCブロックのデータ部のビット長
	virtual ModSize getLocBlockDataLength() const;
	virtual void setLocBlockDataLength(ModSize uiLength_);
	
	// 位置情報
	virtual ModSize getLocOffset() const;
	virtual void setLocOffset(ModSize uiOffset_);
	virtual ModSize getLocLength() const;
	virtual ModSize getLocDataOffset() const;
	virtual ModSize getLocDataLength() const;
	
	// 開放された位置情報クラス
	virtual ModInvertedLocationListIterator* getFreeLocationListIterator() const;
	virtual void setFreeLocationListIterator(ModInvertedLocationListIterator* ite_);

	//
	// 各変数について
	//

	// ここでの用語の使い方は以下のとおり。
	// 位置情報 : TF、ビット長(or位置) [、 位置リスト ] の組
	// 位置リスト : ある索引語のある文書内における出現位置のリスト
	//
	// 以下の書式で説明する。
	// ------------------------
	// 変数 / アクセッサ / 関数
	//		説明
	// ------------------------
	//
	// m_pHeadAddress / getLocBlockDataHeadAddress() / getHeadAddress()
	//		LOCブロックのデータ部の先頭アドレス
	//
	// m_uiBitLength / getLocBlockDataLength() / ---
	//		LOCブロックのデータ部のビット長
	//		ある索引語に対するLOCブロックが複数のページに分割して存在しても、
	//		各LOCブロックの長さを足し合わせた長さではなく、
	//		参照中の一つのLOCブロックのデータ部のビット長を示す。
	//
	// m_uiCurrentLocLength / getLocLength() / getLocationBitLength()
	//		ある索引語を含むある文書IDに対応する位置情報のビット長
	// m_uiCurrentLocOffset / getLocOffset() / getLocationOffset()
	//		ある索引語を含むある文書IDに対応する位置情報のオフセット
	//		オフセットの基準はLOCブロックのデータ部の先頭(m_pHeadAddress)。
	//
	// m_uiCurrentLocDataLength / getLocDataLength() / getLocationDataBitLength()
	//		位置情報ではなく位置リストのビット長
	// m_uiCurrentLocDataOffset / getLocDataOffset() / getLocationDataBitOffset()
	//		位置情報ではなく位置リストのオフセット
	//
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_MIDDLEBASELISTITERATOR_H

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
