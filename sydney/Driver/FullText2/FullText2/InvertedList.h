// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedList.h -- 転置リストをあらわすクラス
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

#ifndef __SYDNEY_FULLTEXT2_INVERTEDLIST_H
#define __SYDNEY_FULLTEXT2_INVERTEDLIST_H

#include "FullText2/Module.h"
#include "FullText2/InvertedListCore.h"
#include "FullText2/LeafPage.h"

#include "ModUnicodeString.h"

class ModInvertedCoder;

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class InvertedUpdateFile;
class InvertedIterator;

//
//	CLASS
//	FullText2::InvertedList --
//
//	NOTES
//
//
class InvertedList : public InvertedListCore
{
public:
	// コンストラクタ(1)
	InvertedList(InvertedUpdateFile& cInvertedFile_,
				 const ModUnicodeChar* pszKey_, ModSize uiKeyLength_,
				 ModUInt32 uiListType_,
				 LeafPage::PagePointer pLeafPage_,
				 LeafPage::Iterator ite_);
	// コンストラクタ(2)
	InvertedList(InvertedUpdateFile& cInvertedFile_,
				 const ModUnicodeChar* pszKey_,
				 ModUInt32 uiListType_,
				 LeafPage::PagePointer pLeafPage_,
				 LeafPage::Iterator ite_);
	// コンストラクタ(3)
	InvertedList(InvertedUpdateFile& cInvertedFile_,
				 const ModUnicodeChar* pszKey_, ModSize uiKeyLength_,
				 ModUInt32 uiListType_);
	// コンストラクタ(4)
	InvertedList(InvertedUpdateFile& cInvertedFile_,
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
						const SmartLocationList& cLocationList_) = 0;
	// 転置リストの挿入 - マージ挿入用
	virtual bool insert(InvertedList& cInvertedList_);

	// 転置リストの削除 - 1文書削除用
	virtual void expunge(ModUInt32 uiDocumentID_);
	// 転置リストの削除 - マージ削除用
	virtual int expunge(InvertedList& cInvertedList_);

	// 転置リスト削除のUNDO - 1文書削除用
	virtual void undoExpunge(ModUInt32 uiDocumentID_,
							 const SmartLocationList& cLocationList_);

	// 整合性検査を行う
	virtual void verify(Admin::Verification::Treatment::Value uiTreatment_,
						Admin::Verification::Progress& cProgress_,
						const Os::Path& cRootPath_) = 0;

	// イテレータを得る -- 得られたインスタンスは呼び出し元で開放する
	virtual InvertedIterator* getIterator() = 0;

	// 大転置の文書IDに変換する
	ModUInt32 convertToBigDocumentID(ModUInt32 uiDocumentID_,
									 int& iUnitNumber_);

	// エリアを得る
	virtual LeafPage::Area* getArea() { return *m_ite; }
	virtual const LeafPage::Area* getArea() const { return *m_ite; }

	// 転置リストタイプを変更する
	virtual InvertedList* convert() = 0;

	// バキュームする
	virtual void vacuum();

	// リーフページを得る
	LeafPage::PagePointer getLeafPage() { return m_pLeafPage; }
	// リーフページのイテレータを得る
	LeafPage::Iterator getLeafPageIterator() { return m_ite; }

	// IDブロック長を得る(索引単位ごとに違う)
	static ModSize getIDBlockUnitSize(const ModUnicodeChar* pszKey_)
		{ return InvertedListCore::getIDBlockUnitSize(pszKey_); }
	// ビット長を格納するためのユニット数を求める
	static ModSize calcUnitSize(ModSize uiBitLength_)
		{ return InvertedListCore::calcUnitSize(uiBitLength_); }

	// ビット単位のmove (領域が重なっていてもいい)
	static void move(ModUInt32* pDst_, ModSize uiDstBegin_,
					 const ModUInt32* pSrc_, ModSize uiSrcBegin_,
					 ModSize uiSize_)
		{ InvertedListCore::move(pDst_, uiDstBegin_,
								 pSrc_, uiSrcBegin_, uiSize_); }
	static void moveBack(ModUInt32* pDst_, ModSize uiDstBegin_,
						 const ModUInt32* pSrc_, ModSize uiSrcBegin_,
						 ModSize uiSize_)
		{ InvertedListCore::moveBack(pDst_, uiDstBegin_,
									 pSrc_, uiSrcBegin_, uiSize_); }

	// ビット単位のmemset
	static void setOff(ModUInt32* pUnit_,
					   ModSize uiBegin_,
					   ModSize uiSize_)
		{ InvertedListCore::setOff(pUnit_, uiBegin_, uiSize_); }
	static void setOffBack(ModUInt32* pUnit_,
						   ModSize uiBegin_,
						   ModSize uiSize_)
		{ InvertedListCore::setOffBack(pUnit_, uiBegin_, uiSize_); }

protected:
	// リーフページ
	LeafPage::PagePointer m_pLeafPage;
	// リーフページ内のイテレータ
	LeafPage::Iterator m_ite;

private:
	// 転置リストタイプ
	ModUInt32 m_uiListType;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_INVERTEDLIST_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
