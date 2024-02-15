// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedListCore.cpp --
// 
// Copyright (c) 2010, 2017, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/InvertedListCore.h"
#include "FullText2/InvertedUpdateFile.h"
#include "FullText2/Parameter.h"

#include "Common/Assert.h"

#include "Exception/BadArgument.h"

#include "ModUnicodeString.h"
#include "ModAutoPointer.h"

#include "ModInvertedCoder.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$::_cIDBlockUnitSize -- IDブロックのユニットサイズ
	//
	ParameterInteger _cIDBlockUnitSize("FullText_IDBlockUnitSize", 16, false);

	//
	//	VARIABLE local
	//	_$$::_cWordIDBlockUnitSize -- IDブロックのユニットサイズ(単語境界用)
	//
	ParameterInteger _cWordIDBlockUnitSize("FullText_WordIDBlockUnitSize",
										   4, false);
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::InvertedListCore -- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUpdateFile& cInvertedFile_
//		転置ファイル
//	const ModUnicodeChar* pszKey_
//		索引単位
///	ModSize uiKeyLength_
//		索引単位文字数
//	
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
InvertedListCore::InvertedListCore(InvertedUpdateFile& cInvertedFile_,
								   const ModUnicodeChar* pszKey_,
								   ModSize uiKeyLength_)
	: m_cInvertedFile(cInvertedFile_),
	  m_cstrKey(pszKey_, uiKeyLength_)
{
	// 圧縮器を設定する
	m_pIdCoder = m_cInvertedFile.getIdCoder(m_cstrKey);
	m_pFrequencyCoder = m_cInvertedFile.getFrequencyCoder(m_cstrKey);
	m_pLengthCoder = m_cInvertedFile.getLengthCoder(m_cstrKey);
	m_pLocationCoder = m_cInvertedFile.getLocationCoder(m_cstrKey);
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::InvertedListCore -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUpdateFile& cInvertedFile_
//		転置ファイル
//	const ModUnicodeChar* pszKey_
//		索引単位
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
InvertedListCore::InvertedListCore(InvertedUpdateFile& cInvertedFile_,
								   const ModUnicodeChar* pszKey_)
	: m_cInvertedFile(cInvertedFile_),
	  m_cstrKey(pszKey_)
{
	// 圧縮器を設定する
	m_pIdCoder = m_cInvertedFile.getIdCoder(m_cstrKey);
	m_pFrequencyCoder = m_cInvertedFile.getFrequencyCoder(m_cstrKey);
	m_pLengthCoder = m_cInvertedFile.getLengthCoder(m_cstrKey);
	m_pLocationCoder = m_cInvertedFile.getLocationCoder(m_cstrKey);
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::~InvertedListCore -- デストラクタ
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
InvertedListCore::~InvertedListCore()
{
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::InvertedListCore
//		-- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
// 	const FullText2::InvertedListCore& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
InvertedListCore::InvertedListCore(const InvertedListCore& src_)
	: m_cInvertedFile(src_.m_cInvertedFile),
	  m_cstrKey(src_.m_cstrKey)
{
	// 圧縮器を設定する
	m_pIdCoder = m_cInvertedFile.getIdCoder(m_cstrKey);
	m_pFrequencyCoder = m_cInvertedFile.getFrequencyCoder(m_cstrKey);
	m_pLengthCoder = m_cInvertedFile.getLengthCoder(m_cstrKey);
	m_pLocationCoder = m_cInvertedFile.getLocationCoder(m_cstrKey);
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::isWordIndex -- 単語単位索引かどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		単語単位索引の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InvertedListCore::isWordIndex() const
{
	return m_cInvertedFile.isWordIndex();
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::getKey -- 索引単位を得る
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
InvertedListCore::getKey() const
{
	return m_cstrKey;
}

//
//	FUNCTION public static
//	FullText2::InvertedListCore::getIDBlockUnitSize
//		-- IDブロックのユニット数を得る
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
InvertedListCore::getIDBlockUnitSize(const ModUnicodeChar* pszKey_)
{
	return (*pszKey_ == 0 ?
			_cWordIDBlockUnitSize.get() : _cIDBlockUnitSize.get());
}

//
//	FUNCTION public static
//	FullText2::InvertedListCore::move -- ビット単位のmove
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
InvertedListCore::move(ModUInt32* pDst_,
					   ModSize uiDstBegin_,
					   const ModUInt32* pSrc_,
					   ModSize uiSrcBegin_,
					   ModSize uiSize_)
{
	if (uiSize_)
	{
		ModInvertedCoder::move(const_cast<ModUInt32*>(pSrc_),
							   uiSrcBegin_,
							   uiSrcBegin_ + uiSize_,
							   uiDstBegin_,
							   pDst_);
	}
}

//
//	FUNCTION public static
//	FullText2::InvertedListCore::moveBack -- ビット単位のmove(後ろ向き)
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
InvertedListCore::moveBack(ModUInt32* pDst_,
						   ModSize uiDstBegin_,
						   const ModUInt32* pSrc_,
						   ModSize uiSrcBegin_,
						   ModSize uiSize_)
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
//	FullText2::InvertedListCore::setOff -- ビット単位のmemset
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
InvertedListCore::setOff(ModUInt32* pUnit_, ModSize uiBegin_, ModSize uiSize_)
{
	ModInvertedCoder::setOff(pUnit_, uiBegin_, uiBegin_+ uiSize_);
}

//
//	FUNCTION public static
//	FullText2::InvertedListCore::setOffBack -- ビット単位のmemset(後ろ向き)
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
InvertedListCore::setOffBack(ModUInt32* pUnit_,
							 ModSize uiBegin_, ModSize uiSize_)
{
	ModInvertedCoder::setOffBack(pUnit_, uiBegin_, uiBegin_ + uiSize_);
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::getCompressedBitLengthDocumentID
//		-- 文書IDの圧縮ビット長を得る
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
InvertedListCore::getCompressedBitLengthDocumentID(ModUInt32 uiLastID_,
												   ModUInt32 uiID_)
{
	return m_pIdCoder->getBitLength(uiID_ - uiLastID_);
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::getCompressedBitLengthLocationList
//		-- 位置情報リストの圧縮ビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::SmartLocationList& cLocationList_
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
InvertedListCore::
getCompressedBitLengthLocationList(const SmartLocationList& cLocationList_,
								   ModSize& uiBitLength_)
{
	// 頻度情報
	ModSize length = getCompressedBitLengthFrequency(cLocationList_.getCount());
	// 位置情報
	uiBitLength_ = cLocationList_.getBitLength();
	length += uiBitLength_;
	if (cLocationList_.getCount() > 1)
		// 圧縮ビット長
		length += getCompressedBitLengthBitLength(uiBitLength_);

	return length;
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::getCompressedBitLengthFrequency
//		-- 位置情報の頻度情報の圧縮ビット長を得る
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
InvertedListCore::getCompressedBitLengthFrequency(ModSize uiFrequency_)
{
	return m_pFrequencyCoder->getBitLength(uiFrequency_);
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::getCompressedBitLengthBitLength
//		-- 位置情報のビット長の圧縮ビット長を得る
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
InvertedListCore::getCompressedBitLengthBitLength(ModSize uiBitLength_)
{
	return m_pLengthCoder->getBitLength(uiBitLength_);
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::getCompressedBitLengthLocation
//		-- 位置情報の圧縮ビット長を得る
//
//	NOTES
//	ARGUMENTS
//	ModSize uiLastLocation_
//		最終位置情報
//	FullText2::LocationListIterator* i_
//		位置情報イテレータ
//
//	RETURN
//	ModSize
//		位置情報の圧縮ビット長
//
//	EXCEPTIONS
//
ModSize
InvertedListCore::getCompressedBitLengthLocation(ModSize uiLastLocation_,
												 LocationListIterator* i_)
{
	ModSize last = uiLastLocation_;
	ModSize location;
	int dummy;
	ModSize length = 0;
	
	while ((location = (*i_).next(dummy)) != UndefinedLocation)
	{
		length += m_pLocationCoder->getBitLength(location - last);
		last = location;
	}

	return length;
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::getCompressedBitLengthLocation
//		-- 位置情報の圧縮ビット長を得る
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
InvertedListCore::getCompressedBitLengthLocation(ModSize uiLastLocation_,
												 ModSize uiLocation_)
{
	return m_pLocationCoder->getBitLength(uiLocation_ - uiLastLocation_);
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::readDocumentID -- 文書IDを1つ読み出す
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
InvertedListCore::readDocumentID(ModUInt32 uiLastDocumentID_,
								 const ModUInt32* pTailAddress_,
								 ModSize uiBitLength_,
								 ModSize& uiBitOffset_)
{
	ModUInt32 id;
	--pTailAddress_;	// ModのTailアドレスは最後のユニットの先頭アドレスのこと
	if (m_pIdCoder->getBack(id, pTailAddress_, uiBitLength_, uiBitOffset_)
		== ModTrue)
		return uiLastDocumentID_ + id;
	else
		return UndefinedDocumentID;
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::readLocationFrequency
//		-- 位置情報の頻度情報を読み出す
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
InvertedListCore::readLocationFrequency(const ModUInt32* pHeadAddress_,
										ModSize uiBitLength_,
										ModSize& uiBitOffset_)
{
	ModSize frequency;
	m_pFrequencyCoder->get(frequency,
						   pHeadAddress_,
						   uiBitLength_,
						   uiBitOffset_);
	return frequency;
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::readLocationBitLength
//		-- 位置情報のビット長を読み出す
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
InvertedListCore::readLocationBitLength(const ModUInt32* pHeadAddress_,
										ModSize uiBitLength_,
										ModSize& uiBitOffset_)
{
	ModSize length;
	m_pLengthCoder->get(length, pHeadAddress_, uiBitLength_, uiBitOffset_);
	return length;
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::readLocationData
//		-- 位置情報の位置情報を1つ読み出す
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
InvertedListCore::readLocationData(ModSize uiLastLocation_,
								   const ModUInt32* pHeadAddress_,
								   ModSize uiBitLength_,
								   ModSize& uiBitOffset_)
{
	ModSize location;
	m_pLocationCoder->get(location, pHeadAddress_, uiBitLength_, uiBitOffset_);
	return uiLastLocation_ + location;
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::writeDocumentID -- 文書IDを1つ書き込む
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
InvertedListCore::writeDocumentID(ModUInt32 uiLastID_,
								  ModUInt32 uiID_,
								  ModUInt32* pTailAddress_,
								  ModSize& uiBitOffset_)
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
//	FullText2::InvertedListCore::writeLocationList -- 位置情報リストを書き込む
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::SmartLocationList& cLocationList_
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
InvertedListCore::writeLocationList(const SmartLocationList& cLocationList_,
									ModSize uiBitLength_,
									ModUInt32* pHeadAddress_,
									ModSize& uiBitOffset_)
{
	// 頻度
	writeLocationFrequency(cLocationList_.getCount(),
						   pHeadAddress_,
						   uiBitOffset_);
	if (cLocationList_.getCount() > 1)
	{
		// ビット長
		writeLocationBitLength(uiBitLength_, pHeadAddress_, uiBitOffset_);
	}
	// 位置情報
#ifdef DEBUG
	ModSize last = 0;
	LocationListIterator::AutoPointer i = cLocationList_.getIterator();
	ModSize location;
	int dummy;
	while ((location = (*i).next(dummy)) != UndefinedLocation)
	{
		writeLocationData(last, location, pHeadAddress_, uiBitOffset_);
		last = location;
	}
#else
	move(pHeadAddress_, uiBitOffset_, cLocationList_.getBuffer(), 0,
		 cLocationList_.getBitLength());
	uiBitOffset_ += cLocationList_.getBitLength();
#endif
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::writeLocationFrequency
//		-- 位置情報の頻度情報を書き込む
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
InvertedListCore::writeLocationFrequency(ModSize uiFrequency_,
										 ModUInt32* pHeadAddress_,
										 ModSize& uiBitOffset_)
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
//	FullText2::InvertedListCore::writeLocationBitLength
//		-- 位置情報のビット長を書き込む
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
InvertedListCore::writeLocationBitLength(ModSize uiBitLength_,
										 ModUInt32* pHeadAddress_,
										 ModSize& uiBitOffset_)
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
//	FullText2::InvertedListCore::writeLocationData
//		-- 位置情報の位置情報を書き込む
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
InvertedListCore::writeLocationData(ModSize uiLastLocation_,
									ModSize uiLocation_,
									ModUInt32* pHeadAddress_,
									ModSize& uiBitOffset_)
{
#ifdef DEBUG
	ModSize uiSave = uiBitOffset_;
#endif
	m_pLocationCoder->append(uiLocation_ - uiLastLocation_,
							 pHeadAddress_,
							 uiBitOffset_);
#ifdef DEBUG
	ModSize uiGap;
	m_pLocationCoder->get(uiGap, pHeadAddress_, uiBitOffset_, uiSave);
	; _SYDNEY_ASSERT(uiLocation_ - uiLastLocation_ == uiGap);
#endif
}

//
//	FUNCTION public
//	FullText2::InvertedListCore::writeLocationData
//		-- 位置情報の位置情報を書き込む
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiLastLocation_
//		直前の位置情報
//	ModSize uiCurrentLocation_
//		今の位置情報
//	FullText2::LocationListIterator* i_
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
InvertedListCore::writeLocationData(ModSize uiLastLocation_,
									ModSize& uiCurrentLocation_,
									LocationListIterator* i_,
									ModUInt32* pHeadAddress_,
									ModSize& uiBitOffset_,
									ModSize uiMaxBitLength_)
{
	ModSize last = uiLastLocation_;
	ModSize location = uiCurrentLocation_;
	int dummy;

	do 
	{
		if (location == 0)
			continue;
		
		if (uiMaxBitLength_ != 0)
		{
			// 最大値が与えられているのでチェックする
			ModSize bitLength = getCompressedBitLengthLocation(last, location);
			if (uiBitOffset_ + bitLength > uiMaxBitLength_)
			{
				uiCurrentLocation_ = location;
				return last;
			}
		}
		writeLocationData(last, location, pHeadAddress_, uiBitOffset_);
		last = location;
	}
	while ((location = (*i_).next(dummy)) != UndefinedLocation);
	
	uiCurrentLocation_ = location;
	return last;
}

//
//	Copyright (c) 2010, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
