// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedList.h -- 転置リストをあらわすクラス
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_INVERTEDLIST_H
#define __SYDNEY_INVERTED_INVERTEDLIST_H

#include "Inverted/Module.h"
#include "Inverted/LeafPage.h"
#include "Inverted/Types.h"

#include "Common/Object.h"

#include "ModUnicodeString.h"
#include "ModInvertedTypes.h"

class ModInvertedLocationListIterator;
class ModInvertedSmartLocationList;
class ModInvertedCoder;

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class InvertedUnit;
class InvertedIterator;

//
//	CLASS
//	Inverted::InvertedList --
//
//	NOTES
//
//
class InvertedList : public Common::Object
{
public:
	// コンストラクタ(1)
	InvertedList(InvertedUnit& cInvertedUnit_,
				 const ModUnicodeChar* pszKey_, ModSize uiKeyLength_,
				 ModUInt32 uiListType_,
				 LeafPage::PagePointer pLeafPage_,
				 LeafPage::Iterator ite_);
	// コンストラクタ(2)
	InvertedList(InvertedUnit& cInvertedUnit_,
				 const ModUnicodeChar* pszKey_,
				 ModUInt32 uiListType_,
				 LeafPage::PagePointer pLeafPage_,
				 LeafPage::Iterator ite_);
	// コンストラクタ(3)
	InvertedList(InvertedUnit& cInvertedUnit_,
				 const ModUnicodeChar* pszKey_, ModSize uiKeyLength_,
				 ModUInt32 uiListType_);
	// コンストラクタ(4)
	InvertedList(InvertedUnit& cInvertedUnit_,
				 const ModUnicodeChar* pszKey_,
				 ModUInt32 uiListType_);
	// デストラクタ
	virtual ~InvertedList();

	// リストタイプを得る
	ModUInt32 getListType() const { return m_uiListType; }
	// 位置情報を格納していないか
	virtual bool isNolocation() const = 0;
	// TFを格納していないか (TFを格納しない時は位置情報も格納しない)
	virtual bool isNoTF() const = 0;

	// 転置リストの挿入 - 1文書挿入用
	virtual bool insert(ModUInt32 uiDocumentID_,
						const ModInvertedSmartLocationList& cLocationList_) = 0;
	// 転置リストの挿入 - マージ挿入用
	virtual bool insert(InvertedList& cInvertedList_) = 0;

	// 転置リストの削除 - 1文書削除用
	virtual void expunge(ModUInt32 uiDocumentID_);
	// 転置リストの削除 - マージ削除用
	virtual void expunge(InvertedList& cInvertedList_);

	// 転置リスト削除のUNDO - 1文書削除用
	virtual void undoExpunge(ModUInt32 uiDocumentID_,
								const ModInvertedSmartLocationList& cLocationList_);

	// 整合性検査を行う
	virtual void verify(Admin::Verification::Treatment::Value uiTreatment_,
						Admin::Verification::Progress& cProgress_,
						const Os::Path& cRootPath_) = 0;

	// イテレータを得る -- 得られたインスタンスは呼び出し元で開放する
	virtual InvertedIterator* begin() const = 0;

	// 文書ID -> ROWIDの変換を行う
	ModUInt32 convertToRowID(ModUInt32 uiDocumentID_);
	// ROWID -> 文書IDの変換を行う
	ModUInt32 convertToDocumentID(ModUInt32 uiRowID_);
	ModUInt32 convertToDocumentID(ModUInt32 uiRowID_, ModInt32& iUnit_);

	// 索引単位文字列を得る
	SYD_INVERTED_FUNCTION
	const ModUnicodeString& getKey() const;

	// エリアを得る
	virtual LeafPage::Area* getArea() { return *m_ite; }
	virtual const LeafPage::Area* getArea() const { return *m_ite; }

	// 転置リストタイプを変更する
	virtual InvertedList* convert() = 0;

	// リーフページを得る
	LeafPage::PagePointer getLeafPage() { return m_pLeafPage; }
	// リーフページのイテレータを得る
	LeafPage::Iterator getLeafPageIterator() { return m_ite; }

	// IDブロック長を得る(索引単位ごとに違う)
	static ModSize getIDBlockUnitSize(const ModUnicodeChar* pszKey_);
	// ビット長を格納するためのユニット数を求める
	static ModSize calcUnitSize(ModSize uiBitLength_)
	{
		return (uiBitLength_ + 31) / 32;
	}

	// ビット単位のmove (領域が重なっていてもいい)
	static void move(ModUInt32* pDst_, ModSize uiDstBegin_,
						const ModUInt32* pSrc_, ModSize uiSrcBegin_, ModSize uiSize_);
	static void moveBack(ModUInt32* pDst_, ModSize uiDstBegin_,
						const ModUInt32* pSrc_, ModSize uiSrcBegin_, ModSize uiSize_);

	// ビット単位のmemset
	static void setOff(ModUInt32* pUnit_, ModSize uiBegin_, ModSize uiSize_);
	static void setOffBack(ModUInt32* pUnit_, ModSize uiBegin_, ModSize uiSize_);

	// 文書IDの圧縮ビット長を得る
	ModSize getCompressedBitLengthDocumentID(ModUInt32 uiLastID_, ModUInt32 uiID_);
	// 位置情報の圧縮ビット長を得る
	virtual ModSize getCompressedBitLengthLocationList(const ModInvertedSmartLocationList& cLocationList_,
													   ModSize& uiBitLength_);
	// 位置情報の頻度情報の圧縮ビット長を得る
	ModSize getCompressedBitLengthFrequency(ModSize uiFrequency_);
	// 位置情報のビット長の圧縮ビット長を得る
	ModSize getCompressedBitLengthBitLength(ModSize uiBitLength_);
	// 位置情報の位置情報の圧縮ビット長を得る
	ModSize getCompressedBitLengthLocation(ModSize uiLastLocation_, ModInvertedLocationListIterator* i_);
	ModSize getCompressedBitLengthLocation(ModSize uiLastLocation_, ModSize uiLocation_);

	// 文書IDを1つ読み出す
	ModUInt32 readDocumentID(ModUInt32 uiLastID_, const ModUInt32* pTailAddress_, ModSize uiBitLength_, ModSize& uiBitOffset_);
	// 位置情報の頻度情報を読み出す
	ModSize readLocationFrequency(const ModUInt32* pHeadAddress_, ModSize uiBitLength_, ModSize& uiBitOffset_);
	// 位置情報のビット長を読み出す
	ModSize readLocationBitLength(const ModUInt32* pHeadAddress_, ModSize uiBitLength_, ModSize& uiBitOffset_);
	// 位置情報の位置情報を1つ読み出す
	ModSize readLocationData(ModSize uiLastLocation_, const ModUInt32* pHeadAddress_, ModSize uiBitLength_, ModSize& uiBitOffset_);

	// 文書IDを圧縮して書き出す(直前のものとの差分を記録する)
	void writeDocumentID(ModUInt32 uiLastID_, ModUInt32 uiID_, ModUInt32* pTailAddress_, ModSize& uiBitOffset_);
	// 位置情報を圧縮して書き出す
	virtual void writeLocationList(const ModInvertedSmartLocationList& cLocationList_, ModSize uiBitLength_, ModUInt32* pHeadAddress_, ModSize& uiBitOffset_);
	// 位置情報の頻度情報を書き出す
	void writeLocationFrequency(ModSize uiFrequency_, ModUInt32* pHeadAddress_, ModSize& uiBitOffset_);
	// 位置情報のビット長を書き出す
	void writeLocationBitLength(ModSize uiBitLength_, ModUInt32* pHeadAddress_, ModSize& uiBitOffset_);
	// 位置情報の位置情報を書き出す(直前のものとの差分を記録する)
	void writeLocationData(ModSize uiLastLocation_, ModSize uiLocation_, ModUInt32* pHeadAddress_, ModSize& uiBitOffset_);
	// 位置情報の位置情報を書き出す(書けるところまで書く)
	ModSize writeLocationData(ModSize uiLastLocation_, ModInvertedLocationListIterator* i_, ModUInt32* pHeadAddress_, ModSize& uiBitOffset_, ModSize uiMaxBitLength_ = 0);

	// 転置ファイルを得る
	InvertedUnit& getInvertedUnit() { return m_cInvertedUnit; }

	// 位置情報圧縮器を得る
	ModInvertedCoder* getLocationCoder() { return m_pLocationCoder; }

	// 不用なIDブロックを登録する
	void enterDeleteIdBlock(ModUInt32 uiFirstDocumentID_);
	// 先頭文書IDの削除ログを登録する
	void enterExpungeFirstDocumentID(ModUInt32 uiOldDocumentID_, ModUInt32 uiNewDocumentID_);
	// 先頭文書ID削除のログを得る
	ModUInt32 getExpungeFirstDocumentID(ModUInt32 uiOldDocumentID_);

protected:
	// リーフページ
	LeafPage::PagePointer m_pLeafPage;
	// リーフページ内のイテレータ
	LeafPage::Iterator m_ite;

private:
	// 転置ファイル
	InvertedUnit& m_cInvertedUnit;

	// 索引単位文字列
	ModUnicodeString m_cstrKey;

	// 転置リストタイプ
	ModUInt32 m_uiListType;

	// 圧縮器
	ModInvertedCoder* m_pIdCoder;			// 文書ID用
	ModInvertedCoder* m_pFrequencyCoder;	// 位置情報の頻度用
	ModInvertedCoder* m_pLengthCoder;		// 位置情報のビット長用
	ModInvertedCoder* m_pLocationCoder;		// 位置情報の位置情報用
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_INVERTEDLIST_H

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
