// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedList.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2010, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Inverted/InvertedList.h"
#include "Inverted/InvertedUnit.h"
#include "Inverted/InvertedIterator.h"
#include "Inverted/LeafPage.h"
#include "Inverted/Parameter.h"

#include "Common/Assert.h"

#include "ModUnicodeString.h"
#include "ModAutoPointer.h"

#include "ModInvertedCoder.h"
#include "ModInvertedLocationListIterator.h"
#include "ModInvertedSmartLocationList.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$::_cIDBlockUnitSize -- IDブロックのユニットサイズ
	//
	ParameterInteger _cIDBlockUnitSize("Inverted_IDBlockUnitSize", 16);

	//
	//	VARIABLE local
	//	_$$::_cWordIDBlockUnitSize -- IDブロックのユニットサイズ(単語境界用)
	//
	ParameterInteger _cWordIDBlockUnitSize("Inverted_WordIDBlockUnitSize", 4);
}

//
//	FUNCTION public
//	Inverted::InvertedList::InvertedList -- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	const ModUnicodeChar* pszKey_
//		索引単位
///	ModSize uiKeyLength_
//		索引単位文字数
//	ModUInt32 uiListType_
//		リスト種別
//	Inverted::LeafPage::PagePointer pLeafPage_
//		リーフページ
//	Inverted::LeafPage::Iterator ite_
//		リーフページのエリアへのイテレータ
//	
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
InvertedList::InvertedList(InvertedUnit& cInvertedUnit_,
						   const ModUnicodeChar* pszKey_,
						   ModSize uiKeyLength_,
						   ModUInt32 uiListType_,
						   LeafPage::PagePointer pLeafPage_,
						   LeafPage::Iterator ite_)
	: m_cInvertedUnit(cInvertedUnit_),
	  m_cstrKey(pszKey_, uiKeyLength_),
	  m_uiListType(uiListType_),
	  m_pLeafPage(pLeafPage_),
	  m_ite(ite_)
{
	// 圧縮器を設定する
	m_pIdCoder = m_cInvertedUnit.getIdCoder(m_cstrKey);
	m_pFrequencyCoder = m_cInvertedUnit.getFrequencyCoder(m_cstrKey);
	m_pLengthCoder = m_cInvertedUnit.getLengthCoder(m_cstrKey);
	m_pLocationCoder = m_cInvertedUnit.getLocationCoder(m_cstrKey);
}

//
//	FUNCTION public
//	Inverted::InvertedList::InvertedList -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	const ModUnicodeChar* pszKey_
//		索引単位
//	ModSize uiListType_
//		リスト種別
//	Inverted::LeafPage::PagePointer pLeafPage_
//		リーフページ
//	Inverted::LeafPage::Iterator ite_
//		リーフページのエリアへのイテレータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
InvertedList::InvertedList(InvertedUnit& cInvertedUnit_,
						   const ModUnicodeChar* pszKey_,
						   ModUInt32 uiListType_,
						   LeafPage::PagePointer pLeafPage_,
						   LeafPage::Iterator ite_)
	: m_cInvertedUnit(cInvertedUnit_),
	  m_cstrKey(pszKey_),
	  m_uiListType(uiListType_),
	  m_pLeafPage(pLeafPage_),
	  m_ite(ite_)
{
	// 圧縮器を設定する
	m_pIdCoder = m_cInvertedUnit.getIdCoder(m_cstrKey);
	m_pFrequencyCoder = m_cInvertedUnit.getFrequencyCoder(m_cstrKey);
	m_pLengthCoder = m_cInvertedUnit.getLengthCoder(m_cstrKey);
	m_pLocationCoder = m_cInvertedUnit.getLocationCoder(m_cstrKey);
}

//
//	FUNCTION public
//	Inverted::InvertedList::InvertedList -- コンストラクタ(3)
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	const ModUnicodeChar* pszKey_
//		索引単位
///	ModSize uiKeyLength_
//		索引単位文字数
//	ModSize uiListType_
//		リスト種別
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
InvertedList::InvertedList(InvertedUnit& cInvertedUnit_,
						   const ModUnicodeChar* pszKey_,
						   ModSize uiKeyLength_,
						   ModUInt32 uiListType_)
	: m_cInvertedUnit(cInvertedUnit_),
	  m_cstrKey(pszKey_, uiKeyLength_),
	  m_uiListType(uiListType_)
{
	// 圧縮器を設定する
	m_pIdCoder = m_cInvertedUnit.getIdCoder(m_cstrKey);
	m_pFrequencyCoder = m_cInvertedUnit.getFrequencyCoder(m_cstrKey);
	m_pLengthCoder = m_cInvertedUnit.getLengthCoder(m_cstrKey);
	m_pLocationCoder = m_cInvertedUnit.getLocationCoder(m_cstrKey);
}

//
//	FUNCTION public
//	Inverted::InvertedList::InvertedList -- コンストラクタ(4)
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	const ModUnicodeChar* pszKey_
//		索引単位
//	ModSize uiListType_
//		リスト種別
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
InvertedList::InvertedList(InvertedUnit& cInvertedUnit_,
						   const ModUnicodeChar* pszKey_,
						   ModUInt32 uiListType_)
	: m_cInvertedUnit(cInvertedUnit_),
	  m_cstrKey(pszKey_),
	  m_uiListType(uiListType_)
{
	// 圧縮器を設定する
	m_pIdCoder = m_cInvertedUnit.getIdCoder(m_cstrKey);
	m_pFrequencyCoder = m_cInvertedUnit.getFrequencyCoder(m_cstrKey);
	m_pLengthCoder = m_cInvertedUnit.getLengthCoder(m_cstrKey);
	m_pLocationCoder = m_cInvertedUnit.getLocationCoder(m_cstrKey);
}

//
//	FUNCTION public
//	Inverted::InvertedList::~InvertedList -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
InvertedList::~InvertedList()
{
}

//
//	FUNCTION public
//	Inverted::InvertedList::expunge -- 転置リストからの削除(1文書)
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		削除する文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedList::expunge(ModUInt32 uiDocumentID_)
{
	ModAutoPointer<InvertedIterator> i = begin();

	// 削除する文書を検索する
	if ((*i).find(uiDocumentID_) == ModTrue)
	{
		// まずdirtyにする。削除の途中で例外が発生するかもしれないから
		if (m_pLeafPage) m_pLeafPage->dirty();

		// 削除する
		(*i).expunge();
	}
}

//
//	FUNCTION public
//	Inverted::InvertedList::expunge -- 転置リストからの削除(転置リスト単位)
//
//	NOTES
//	マージ時に転置リスト単位の削除を行うためのメソッド
//
//	ARGUMENTS
//	const Inverted::InvertedList& cInvertedList_
//		削除する転置リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedList::expunge(InvertedList& cInvertedList_)
{
	ModAutoPointer<InvertedIterator> src = cInvertedList_.begin();
	ModAutoPointer<InvertedIterator> dst = begin();
	while ((*src).isEnd() == ModFalse)
	{
		// 削除する文書IDを求める
		ModUInt32 uiRowID
			= cInvertedList_.convertToRowID((*src).getDocumentId());
		ModInt32 iUnit;
		ModUInt32 uiDocumentID = convertToDocumentID(uiRowID, iUnit);

		// 削除する文書を検索する
		if (uiDocumentID != UndefinedDocumentID &&
			iUnit == m_cInvertedUnit.getUnit() &&
			(*dst).find(uiDocumentID) == ModTrue)
		{
			// まずdirtyにする。削除の途中で例外が発生するかもしれないから
			if (m_pLeafPage) m_pLeafPage->dirty();

			// 削除する
			(*dst).expunge();
		}

		// 次へ
		(*src).next();
	}
}

//
//	FUNCTION public
//	Inverted::InvertedList::undoExpunge -- 削除の取り消しを行う(1文書単位)
//
//	NOTES
//	削除中にエラーが発生した場合に、途中の削除を取り消すために呼び出す
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		挿入する文書ID
//	const ModInvertedSmartLocationList& cLocationList_
//		挿入する位置情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedList::undoExpunge(ModUInt32 uiDocumentID_, const ModInvertedSmartLocationList& cLocationList_)
{
	ModAutoPointer<InvertedIterator> i = begin();

	// lower_boundで検索する
	(*i).lowerBound(uiDocumentID_, true);

	// 削除を取り消す
	(*i).undoExpunge(uiDocumentID_, cLocationList_);

	if (m_pLeafPage) m_pLeafPage->dirty();
}

//
//	FUNCTION public
//	Inverted::InvertedList::convertToRowID -- 文書ID->ROWIDの変換を行う
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		ROWIDに変換する文書ID
//
//	RETURN
//	ModUInt32
//		変換したROWID
//
//	EXCEPTIONS
//
ModUInt32
InvertedList::convertToRowID(ModUInt32 uiDocumentID_)
{
	return m_cInvertedUnit.convertToRowID(uiDocumentID_);
}

//
//	FUNCTION public
//	Inverted::InvertedList::convertToDocumentID -- ROWID->文書IDの変換を行う
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		文書IDに変換するROWID
//
//	RETURN
//	ModUInt32
//		変換した文書ID
//
//	EXCEPTIONS
//
ModUInt32
InvertedList::convertToDocumentID(ModUInt32 uiRowID_)
{
	return m_cInvertedUnit.convertToDocumentID(uiRowID_);
}

//
//	FUNCTION public
//	Inverted::InvertedList::convertToDocumentID -- ROWID->文書IDの変換を行う
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		文書IDに変換するROWID
//	ModInt32 iUnit_
//		ユニット番号
//
//	RETURN
//	ModUInt32
//		変換した文書ID
//
//	EXCEPTIONS
//
ModUInt32
InvertedList::convertToDocumentID(ModUInt32 uiRowID_, ModInt32& iUnit_)
{
	return m_cInvertedUnit.convertToDocumentID(uiRowID_, iUnit_);
}

//
//	FUNCTION public
//	Inverted::InvertedList::getKey -- 索引単位を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUnicodeString&
//		索引単位
//
//	EXCEPTIONS
//
const ModUnicodeString&
InvertedList::getKey() const
{
	return m_cstrKey;
}

//
//	FUNCTION public static
//	Inverted::InvertedList::getIDBlockUnitSize -- IDブロックのユニット数を得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar* pszKey_
//		索引単位
//
//	RETURN
//	ModSize
//		IDブロックのユニット数
//
//	EXCEPTIONS
//
ModSize
InvertedList::getIDBlockUnitSize(const ModUnicodeChar* pszKey_)
{
	return (*pszKey_ == 0 ? _cWordIDBlockUnitSize.get() : _cIDBlockUnitSize.get());
}

//
//	FUNCTION public static
//	Inverted::InvertedList::move -- ビット単位のmove
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* pDst_
//		移動先のメモリー領域の先頭
//	ModSize uiDstBegin_
//		pDst_からのビットオフセット
//	const ModUInt32* pSrc_
//		移動元のメモリー領域の先頭
//	ModSize uiSrcBegin_
//		pSrc_からのビットオフセット
//	ModSize uiSize_
//		移動する領域のビット長
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedList::move(ModUInt32* pDst_, ModSize uiDstBegin_,
				   const ModUInt32* pSrc_, ModSize uiSrcBegin_, ModSize uiSize_)
{
	if (uiSize_)
	{
		ModInvertedCoder::move(const_cast<ModUInt32*>(pSrc_), uiSrcBegin_, uiSrcBegin_ + uiSize_, uiDstBegin_, pDst_);
	}
}

//
//	FUNCTION public static
//	Inverted::InvertedList::moveBack -- ビット単位のmove(後ろ向き)
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* pDst_
//		移動先のメモリー領域の終端
//	ModSize uiDstBegin_
//		pDst_からのビットオフセット
//	const ModUInt32* pSrc_
//		移動元のメモリー領域の終端
//	ModSize uiSrcBegin_
//		pSrc_からのビットオフセット
//	ModSize uiSize_
//		移動する領域のビット長
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedList::moveBack(ModUInt32* pDst_, ModSize uiDstBegin_,
					   const ModUInt32* pSrc_, ModSize uiSrcBegin_, ModSize uiSize_)
{
	// 後ろ向き用のmoveはModにないので、前向きを使用する
	pSrc_ -= ((uiSrcBegin_ + uiSize_) + 31) / 32;
	pDst_ -= ((uiDstBegin_ + uiSize_) + 31) / 32;
	uiSrcBegin_ = 32 - ((uiSrcBegin_ + uiSize_) & 31);
	if (uiSrcBegin_ == 32) uiSrcBegin_ = 0;
	uiDstBegin_ = 32 - ((uiDstBegin_ + uiSize_) & 31);
	if (uiDstBegin_ == 32) uiDstBegin_ = 0;

	move(pDst_, uiDstBegin_, pSrc_, uiSrcBegin_, uiSize_);
}

//
//	FUNCTION public static
//	Inverted::InvertedList::setOff -- ビット単位のmemset
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* pUnit_
//		先頭のアドレス
//	ModSize uiBegin_
//		ビットをクリアするオフセット
//	ModSize uiSize_
//		ビット長
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedList::setOff(ModUInt32* pUnit_, ModSize uiBegin_, ModSize uiSize_)
{
	ModInvertedCoder::setOff(pUnit_, uiBegin_, uiBegin_+ uiSize_);
}

//
//	FUNCTION public static
//	Inverted::InvertedList::setOffBack -- ビット単位のmemset(後ろ向き)
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* pUnit_
//		終端のアドレス
//	ModSize uiBegin_
//		ビットをクリアするオフセット
//	ModSize uiSize_
//		ビット長
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedList::setOffBack(ModUInt32* pUnit_, ModSize uiBegin_, ModSize uiSize_)
{
	ModInvertedCoder::setOffBack(pUnit_, uiBegin_, uiBegin_ + uiSize_);
}

//
//	FUNCTION public
//	Inverted::InvertedList::getCompressedBitLengthDocumentID -- 文書IDの圧縮ビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiLastID_
//		最終文書ID
//	ModUInt32 uiID_
//		追加する文書ID
//
//	RETURN
//	ModSize
//		uiLastID_からの差分を圧縮したビット長
//
//	EXCEPTIONS
//
ModSize
InvertedList::getCompressedBitLengthDocumentID(ModUInt32 uiLastID_, ModUInt32 uiID_)
{
	return m_pIdCoder->getBitLength(uiID_ - uiLastID_);
}

//
//	FUNCTION public
//	Inverted::InvertedList::getCompressedBitLengthLocationList -- 位置情報リストの圧縮ビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModInvertedSmartLocationList& cLocationList_
//		位置情報リスト
//	ModSize& uiBitLength_
//		位置情報データのビット長
//
//	RETURN
//	ModSize
//		位置情報全体の圧縮ビット長
//
//	EXCEPTIONS
//
ModSize
InvertedList::getCompressedBitLengthLocationList(const ModInvertedSmartLocationList& cLocationList_,
												 ModSize& uiBitLength_)
{
	// 頻度情報
	ModSize length = getCompressedBitLengthFrequency(cLocationList_.getSize());
	// 位置情報
	uiBitLength_ = cLocationList_.getBitLength();
	length += uiBitLength_;
	if (cLocationList_.getSize() > 1)
		// 圧縮ビット長
		length += getCompressedBitLengthBitLength(uiBitLength_);

	return length;
}

//
//	FUNCTION public
//	Inverted::InvertedList::getCompressedBitLengthFrequency -- 位置情報の頻度情報の圧縮ビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiFrequency_
//		頻度情報
//
//	RETURN
//	ModSize
//		頻度情報の圧縮ビット長
//
//	EXCEPTIONS
//
ModSize
InvertedList::getCompressedBitLengthFrequency(ModSize uiFrequency_)
{
	return m_pFrequencyCoder->getBitLength(uiFrequency_);
}

//
//	FUNCTION public
//	Inverted::InvertedList::getCompressedBitLengthBitLength -- 位置情報のビット長の圧縮ビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiBitLength_
//		ビット長
//
//	RETURN
//	ModSize
//		ビット長の圧縮ビット長
//
//	EXCEPTIONS
//
ModSize
InvertedList::getCompressedBitLengthBitLength(ModSize uiBitLength_)
{
	return m_pLengthCoder->getBitLength(uiBitLength_);
}

//
//	FUNCTION public
//	Inverted::InvertedList::getCompressedBitLengthLocation -- 位置情報の圧縮ビット長を得る
//
//	NOTES
//	ARGUMENTS
//	ModSize uiLastLocation_
//		最終位置情報
//	ModInvertedLocationListIterator* i_
//		位置情報イテレータ
//
//	RETURN
//	ModSize
//		位置情報の圧縮ビット長
//
//	EXCEPTIONS
//
ModSize
InvertedList::getCompressedBitLengthLocation(ModSize uiLastLocation_, ModInvertedLocationListIterator* i_)
{
	ModSize length = 0;
	ModSize last = uiLastLocation_;
	while ((*i_).isEnd() == ModFalse)
	{
		ModSize location = (*i_).getLocation();
		length += m_pLocationCoder->getBitLength(location - last);
		last = location;
		(*i_).next();
	}

	return length;
}

//
//	FUNCTION public
//	Inverted::InvertedList::getCompressedBitLengthLocation -- 位置情報の圧縮ビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiLastLocation_
//		最終位置情報
//	ModSize uiLocation_
//		位置情報
//
//	RETURN
//	ModSize
//		位置情報の圧縮ビット長
//
//	EXCEPTIONS
//
ModSize
InvertedList::getCompressedBitLengthLocation(ModSize uiLastLocation_, ModSize uiLocation_)
{
	return m_pLocationCoder->getBitLength(uiLocation_ - uiLastLocation_);
}

//
//	FUNCTION public
//	Inverted::InvertedList::readDocumentID -- 文書IDを1つ読み出す
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiLastDocumentID_
//		直前の文書ID
//	const ModUInt32* pTailAddress_
//		終端のアドレス
//	ModSize uiBitLength_
//		全体のビット長
//	ModSize& uiBitOffset_
//		読み出すビット位置。実行後に次の位置になる
//
//	RETURN
//	ModUInt32
//		読み出した文書ID。終端に達した場合はUndefinedDocumentID
//
//	EXCEPTIONS
//
ModUInt32
InvertedList::readDocumentID(ModUInt32 uiLastDocumentID_, const ModUInt32* pTailAddress_,
							 ModSize uiBitLength_, ModSize& uiBitOffset_)
{
	ModUInt32 id;
	--pTailAddress_;	// ModのTailアドレスは最後のユニットの先頭アドレスのこと
	if (m_pIdCoder->getBack(id, pTailAddress_, uiBitLength_, uiBitOffset_) == ModTrue)
		return uiLastDocumentID_ + id;
	else
		return UndefinedDocumentID;
}

//
//	FUNCTION public
//	Inverted::InvertedList::readLocationFrequency -- 位置情報の頻度情報を読み出す
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pHeadAddress_
//		先頭のアドレス
//	ModSize uiBitLength_
//		全体のビット長
//	ModSize& uiBitOffset_
//		読み出すビット位置。実行後に次の位置になる
//
//	RETURN
//	ModSize
//		読み出した頻度情報
//
//	EXCEPTIONS
//
ModSize
InvertedList::readLocationFrequency(const ModUInt32* pHeadAddress_, ModSize uiBitLength_, ModSize& uiBitOffset_)
{
	// [NOTE] 未定義だと取得できなかった時にUMRなので、0で初期化する。
	//  取得できなくても0を返してしまうが、通常は1以上の値が取得されるので、
	//  呼び出し側で判定できる。
	ModSize frequency = 0;
	m_pFrequencyCoder->get(frequency, pHeadAddress_, uiBitLength_, uiBitOffset_);
	return frequency;
}

//
//	FUNCTION public
//	Inverted::InvertedList::readLocationBitLength -- 位置情報のビット長を読み出す
//
//	NOTES
//
//	ARGUMENTS
//	const ModUInt32* pHeadAddress_
//		先頭のアドレス
//	ModSize uiBitLength_
//		全体のビット長
//	ModSize& uiBitOffset_
//		読み出すビット位置。実行後に次の位置になる
//
//	RETURN
//	ModSize
//		読み出したビット長
//
//	EXCEPTIONS
//
ModSize
InvertedList::readLocationBitLength(const ModUInt32* pHeadAddress_, ModSize uiBitLength_, ModSize& uiBitOffset_)
{
	// [NOTE] 未定義だと取得できなかった時にUMRなので、0で初期化する。
	//  取得できなくても0を返してしまうが、通常は1以上の値が取得されるので、
	//  呼び出し側で判定できる。
	ModSize length = 0;
	m_pLengthCoder->get(length, pHeadAddress_, uiBitLength_, uiBitOffset_);
	return length;
}

//
//	FUNCTION public
//	Inverted::InvertedList::readLocationData -- 位置情報の位置情報を1つ読み出す
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiLastLocation_
//		 直前の位置情報
//	const ModUInt32* pHeadAddress_
//		先頭のアドレス
//	ModSize uiBitLength_
//		全体のビット長
//	ModSize& uiBitOffset_
//		読み出すビット位置。実行後に告ぎの位置になる
//
//	RETURN
//	ModSize
//		読み出した位置情報
//
//	EXCEPTIONS
//
ModSize
InvertedList::readLocationData(ModSize uiLastLocation_, const ModUInt32* pHeadAddress_, ModSize uiBitLength_, ModSize& uiBitOffset_)
{
	// [NOTE] 未定義だと取得できなかった時にUMRなので、0で初期化する。
	//  取得できないとuiLocation_をそのまま返してしまうが、通常は+1以上の値が
	//  取得されるので、呼び出し側で判定できる。
	// [NOTE] locationには、1-baseの位置で前との差分が格納されている。
	//  参考: writeLocationData(), ModInvertedTokenizer::tokenize()
	ModSize location = 0;
	m_pLocationCoder->get(location, pHeadAddress_, uiBitLength_, uiBitOffset_);
	return uiLastLocation_ + location;
}

//
//	FUNCTION public
//	Inverted::InvertedList::writeDocumentID -- 文書IDを1つ書き込む
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiLastID_
//		直前の文書ID
//	ModUInt32 uiID_
//		書き込む文書ID
//	ModUInt32* pTailAddress_
//		終端のアドレス
//	ModSize& uiBitOffset_
//		書き込むビット位置。呼び出し後は次に書き込むビット位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedList::writeDocumentID(ModUInt32 uiLastID_, ModUInt32 uiID_,
							  ModUInt32* pTailAddress_, ModSize& uiBitOffset_)
{
#ifdef DEBUG
	ModSize uiSave = uiBitOffset_;
#endif
	--pTailAddress_;	// ModのTailアドレスは最後のユニットの先頭アドレスのこと
	m_pIdCoder->appendBack(uiID_ - uiLastID_, pTailAddress_, uiBitOffset_);
#ifdef DEBUG
	ModSize uiGap;
	m_pIdCoder->getBack(uiGap, pTailAddress_, uiBitOffset_, uiSave);
	; _SYDNEY_ASSERT(uiID_ - uiLastID_ == uiGap);
#endif
}

//
//	FUNCTION public
//	Inverted::InvertedList::writeLocationList -- 位置情報リストを書き込む
//
//	NOTES
//
//	ARGUMENTS
//	const ModInvertedSmartLocationList& cLocationList_
//		位置情報リスト
//	ModSize uiBitLength_
//		位置情報データのビット長
//	ModUInt32* pHeadAddress_
//		先頭のアドレス
//	ModSize& uiBitOffset_
//		書き込むビット位置。呼び出し後は次に書き込むビット位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedList::writeLocationList(const ModInvertedSmartLocationList& cLocationList_,
								ModSize uiBitLength_,
								ModUInt32* pHeadAddress_, ModSize& uiBitOffset_)
{
	// 頻度
	writeLocationFrequency(cLocationList_.getSize(), pHeadAddress_, uiBitOffset_);
	if (cLocationList_.getSize() > 1)
	{
		// ビット長
		writeLocationBitLength(uiBitLength_, pHeadAddress_, uiBitOffset_);
	}
	// 位置情報
	ModSize last = 0;
	ModAutoPointer<ModInvertedLocationListIterator> i = cLocationList_.begin();
	while ((*i).isEnd() == ModFalse)
	{
		ModSize location = (*i).getLocation();
		writeLocationData(last, location, pHeadAddress_, uiBitOffset_);
		last = location;
		(*i).next();
	}
}

//
//	FUNCTION public
//	Inverted::InvertedList::writeLocationFrequency -- 位置情報の頻度情報を書き込む
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiFrequency_
//		書き込む頻度情報
//	ModUInt32* pHeadAddress_
//		先頭のアドレス
//	ModSize& uiBitOffset_
//		書き込むビット位置。呼び出し後は次に書き込むビット位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedList::writeLocationFrequency(ModSize uiFrequency_,
									 ModUInt32* pHeadAddress_, ModSize& uiBitOffset_)
{
#ifdef DEBUG
	ModSize uiSave = uiBitOffset_;
#endif
	m_pFrequencyCoder->append(uiFrequency_, pHeadAddress_, uiBitOffset_);
#ifdef DEBUG
	ModSize uiData;
	m_pFrequencyCoder->get(uiData, pHeadAddress_, uiBitOffset_, uiSave);
	; _SYDNEY_ASSERT(uiData == uiFrequency_);
#endif
}

//
//	FUNCTION public
//	Inverted::InvertedList::writeLocationBitLength -- 位置情報のビット長を書き込む
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiBitLength_
//		書き込むビット長
//	ModUInt32* pHeadAddress_
//		先頭のアドレス
//	ModSize& uiBitOffset_
//		書き込むビット位置。呼び出し後は次に書き込むビット位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedList::writeLocationBitLength(ModSize uiBitLength_,
									 ModUInt32* pHeadAddress_, ModSize& uiBitOffset_)
{
#ifdef DEBUG
	ModSize uiSave = uiBitOffset_;
#endif
	m_pLengthCoder->append(uiBitLength_, pHeadAddress_, uiBitOffset_);
#ifdef DEBUG
	ModSize uiData;
	m_pLengthCoder->get(uiData, pHeadAddress_, uiBitOffset_, uiSave);
	; _SYDNEY_ASSERT(uiData == uiBitLength_);
#endif
}

//
//	FUNCTION public
//	Inverted::InvertedList::writeLocationData - 位置情報の位置情報を書き込む
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiLastLocation_
//		直前の位置情報
//	ModSize uiLocation_
//		書き込む位置情報
//	ModUInt32* pHeadAddress_
//		先頭のアドレス
//	ModSize& uiBitOffset_
//		書き込むビット位置。呼び出し後は次に書き込むビット位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedList::writeLocationData(ModSize uiLastLocation_, ModSize uiLocation_,
									 ModUInt32* pHeadAddress_, ModSize& uiBitOffset_)
{
#ifdef DEBUG
	ModSize uiSave = uiBitOffset_;
#endif
	m_pLocationCoder->append(uiLocation_ - uiLastLocation_, pHeadAddress_, uiBitOffset_);
#ifdef DEBUG
	ModSize uiGap;
	m_pLocationCoder->get(uiGap, pHeadAddress_, uiBitOffset_, uiSave);
	; _SYDNEY_ASSERT(uiLocation_ - uiLastLocation_ == uiGap);
#endif
}

//
//	FUNCTION public
//	Inverted::InvertedList::writeLocationData - 位置情報の位置情報を書き込む
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiLastLocation_
//		直前の位置情報
//	ModInvertedLocationListIterator* i_
//		書き込む位置情報
//	ModUInt32* pHeadAddress_
//		先頭のアドレス
//	ModSize& uiBitOffset_
//		書き込むビット位置。呼び出し後は次に書き込むビット位置
//	ModSize uiMaxBitLength_
//		書き込んだビットの最大値 (default 0)
//
//	RETURN
//	ModSize
//		書き込んだ最終の位置情報値
//
//	EXCEPTIONS
//
ModSize
InvertedList::writeLocationData(ModSize uiLastLocation_, ModInvertedLocationListIterator* i_,
									 ModUInt32* pHeadAddress_, ModSize& uiBitOffset_, ModSize uiMaxBitLength_)
{
	ModSize last = uiLastLocation_;
	while ((*i_).isEnd() == ModFalse)
	{
		ModSize location = (*i_).getLocation();
		if (uiMaxBitLength_ != 0)
		{
			// 最大値が与えられているのでチェックする
			ModSize bitLength = getCompressedBitLengthLocation(last, location);
			if (uiBitOffset_ + bitLength > uiMaxBitLength_)
				return last;
		}
		writeLocationData(last, location, pHeadAddress_, uiBitOffset_);
		last = location;
		(*i_).next();
	}
	return last;
}

//
//	FUNCTION public
//	Inverted::InvertedList::enterDeleteIdBlock -- 不用なIDブロックを登録する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiFirstDocumentID_
//		不用なIDブロックの先頭文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedList::enterDeleteIdBlock(ModUInt32 uiFirstDocumentID_)
{
	m_cInvertedUnit.enterDeleteIdBlock(getKey(), uiFirstDocumentID_);
}

//
//	FUNCTION public
//	Inverted::InvertedList::enterExpungeFirstDocumentID -- 先頭文書ID削除のログを登録する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiOldDocumentID_
//		削除前の文書ID
//	ModUInt32 uiNewDocumentID_
//		削除後の文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedList::enterExpungeFirstDocumentID(ModUInt32 uiOldDocumentID_, ModUInt32 uiNewDocumentID_)
{
	m_cInvertedUnit.enterExpungeFirstDocumentID(getKey(), uiOldDocumentID_, uiNewDocumentID_);
}

//
//	FUNCTION public
//	Inverted::InvertedList::getExpungeFirstDocumentID -- 先頭文書IDが削除されたものか
//
//	NOTES
//
//	ARUGMENTS
//	ModUInt32 uiOldDocumentID_
//		削除前の文書ID
//
//	RETURN
//	ModUInt32
//		削除後の文書ID、存在しない場合はUndefinedDocumentID
//
//	EXCEPTIONS
//
ModUInt32
InvertedList::getExpungeFirstDocumentID(ModUInt32 uiOldDocumentID_)
{
	return m_cInvertedUnit.getExpungeFirstDocumentID(getKey(), uiOldDocumentID_);
}

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
