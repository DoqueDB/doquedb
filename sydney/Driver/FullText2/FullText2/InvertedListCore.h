// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedListCore.h -- 転置リストをあらわすクラス
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

#ifndef __SYDNEY_FULLTEXT2_INVERTEDLISTCORE_H
#define __SYDNEY_FULLTEXT2_INVERTEDLISTCORE_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"
#include "FullText2/SmartLocationList.h"

#include "ModUnicodeString.h"

class ModInvertedCoder;

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class InvertedUpdateFile;

//
//	CLASS
//	FullText2::InvertedList --
//
//	NOTES
//
//
class InvertedListCore
{
public:
	// コンストラクタ
	InvertedListCore(InvertedUpdateFile& cInvertedFile_,
					 const ModUnicodeChar* pszKey_, ModSize uiKeyLength_);
	// コンストラクタ(2)
	InvertedListCore(InvertedUpdateFile& cInvertedFile_,
					 const ModUnicodeChar* pszKey_);
	// デストラクタ
	virtual ~InvertedListCore();

	// コピーコンストラクタ
	InvertedListCore(const InvertedListCore& src_);

	// 単語単位索引かどうか
	bool isWordIndex() const;

	// 索引単位文字列を得る
	const ModUnicodeString& getKey() const;

	// IDブロック長を得る(索引単位ごとに違う)
	static ModSize getIDBlockUnitSize(const ModUnicodeChar* pszKey_);
	// ビット長を格納するためのユニット数を求める
	static ModSize calcUnitSize(ModSize uiBitLength_)
	{
		return (uiBitLength_ + 31) / 32;
	}

	// ビット単位のmove (領域が重なっていてもいい)
	static void move(ModUInt32* pDst_, ModSize uiDstBegin_,
					 const ModUInt32* pSrc_, ModSize uiSrcBegin_,
					 ModSize uiSize_);
	static void moveBack(ModUInt32* pDst_, ModSize uiDstBegin_,
						 const ModUInt32* pSrc_, ModSize uiSrcBegin_,
						 ModSize uiSize_);

	// ビット単位のmemset
	static void setOff(ModUInt32* pUnit_,
					   ModSize uiBegin_,
					   ModSize uiSize_);
	static void setOffBack(ModUInt32* pUnit_,
						   ModSize uiBegin_,
						   ModSize uiSize_);

	// 文書IDの圧縮ビット長を得る
	ModSize getCompressedBitLengthDocumentID(ModUInt32 uiLastID_,
											 ModUInt32 uiID_);
	// 位置情報の圧縮ビット長を得る
	ModSize getCompressedBitLengthLocationList(
		const SmartLocationList& cLocationList_, ModSize& uiBitLength_);
	// 位置情報の頻度情報の圧縮ビット長を得る
	ModSize getCompressedBitLengthFrequency(ModSize uiFrequency_);
	// 位置情報のビット長の圧縮ビット長を得る
	ModSize getCompressedBitLengthBitLength(ModSize uiBitLength_);
	// 位置情報の位置情報の圧縮ビット長を得る
	ModSize getCompressedBitLengthLocation(ModSize uiLastLocation_,
										   LocationListIterator* i_);
	ModSize getCompressedBitLengthLocation(ModSize uiLastLocation_,
										   ModSize uiLocation_);

	// 文書IDを1つ読み出す
	ModUInt32 readDocumentID(ModUInt32 uiLastID_,
							 const ModUInt32* pTailAddress_,
							 ModSize uiBitLength_,
							 ModSize& uiBitOffset_);
	// 位置情報の頻度情報を読み出す
	ModSize readLocationFrequency(const ModUInt32* pHeadAddress_,
								  ModSize uiBitLength_,
								  ModSize& uiBitOffset_);
	// 位置情報のビット長を読み出す
	ModSize readLocationBitLength(const ModUInt32* pHeadAddress_,
								  ModSize uiBitLength_,
								  ModSize& uiBitOffset_);
	// 位置情報の位置情報を1つ読み出す
	ModSize readLocationData(ModSize uiLastLocation_,
							 const ModUInt32* pHeadAddress_,
							 ModSize uiBitLength_,
							 ModSize& uiBitOffset_);

	// 文書IDを圧縮して書き出す(直前のものとの差分を記録する)
	void writeDocumentID(ModUInt32 uiLastID_,
						 ModUInt32 uiID_,
						 ModUInt32* pTailAddress_,
						 ModSize& uiBitOffset_);
	// 位置情報を圧縮して書き出す
	void writeLocationList(const SmartLocationList& cLocationList_,
						   ModSize uiBitLength_,
						   ModUInt32* pHeadAddress_,
						   ModSize& uiBitOffset_);
	// 位置情報の頻度情報を書き出す
	void writeLocationFrequency(ModSize uiFrequency_,
								ModUInt32* pHeadAddress_,
								ModSize& uiBitOffset_);
	// 位置情報のビット長を書き出す
	void writeLocationBitLength(ModSize uiBitLength_,
								ModUInt32* pHeadAddress_,
								ModSize& uiBitOffset_);
	// 位置情報の位置情報を書き出す(直前のものとの差分を記録する)
	void writeLocationData(ModSize uiLastLocation_,
						   ModSize uiLocation_,
						   ModUInt32* pHeadAddress_,
						   ModSize& uiBitOffset_);
	// 位置情報の位置情報を書き出す(書けるところまで書く)
	ModSize writeLocationData(ModSize uiLastLocation_,
							  ModSize& uiCurrentLocation_,
							  LocationListIterator* i_,
							  ModUInt32* pHeadAddress_,
							  ModSize& uiBitOffset_,
							  ModSize uiMaxBitLength_ = 0);

	// 位置情報圧縮器を得る
	ModInvertedCoder* getLocationCoder() { return m_pLocationCoder; }

protected:
	// 転置ファイル
	InvertedUpdateFile& m_cInvertedFile;

	// 索引単位文字列
	ModUnicodeString m_cstrKey;

	// 圧縮器
	ModInvertedCoder* m_pIdCoder;			// 文書ID用
	ModInvertedCoder* m_pFrequencyCoder;	// 位置情報の頻度用
	ModInvertedCoder* m_pLengthCoder;		// 位置情報のビット長用
	ModInvertedCoder* m_pLocationCoder;		// 位置情報の位置情報用
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_INVERTEDLISTCORE_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
