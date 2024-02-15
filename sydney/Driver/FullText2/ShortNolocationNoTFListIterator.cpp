// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortNolocationNoTFListIterator.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "FullText2/ShortNolocationNoTFListIterator.h"

#include "FullText2/FakeError.h"
#include "FullText2/InvertedList.h"
#include "FullText2/Types.h"

#include "Exception/Unexpected.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::ShortNolocationNoTFListIterator::ShortNolocationNoTFListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedList& cInvertedList_
//		転置リスト(Short or Batch)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ShortNolocationNoTFListIterator::ShortNolocationNoTFListIterator(
	InvertedList& cInvertedList_)
	: ShortBaseListIterator(cInvertedList_)
{
}

//
//	FUNCTION public
//	FullText2::ShortNolocationNoTFListIterator::~ShortNolocationNoTFListIterator
//		-- デストラクタ
//
//	NOTES
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
ShortNolocationNoTFListIterator::~ShortNolocationNoTFListIterator()
{
}

//
//	FUNCTION public
//	FullText2::ShortNolocationNoTFListIterator::getTermFrequency
//		-- 位置情報内の頻度情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		位置情報内の頻度情報
//
//	EXCEPTIONS
//
ModSize
ShortNolocationNoTFListIterator::getTermFrequency()
{
	return 1;	// 常に1を返す
}

//
//	FUNCTION public
//	FullText2::ShortNolocationNoTFListIterator::expunge
//		-- 現在位置のデータを削除する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortNolocationNoTFListIterator::expunge()
{
	// 文書IDを削除する
	expungeDocumentID();

	; _FULLTEXT2_FAKE_ERROR(ShortNolocationNoTFListIterator::expunge);
}

//
//	FUNCTION public
//	FullText2::ShortNolocationNoTFListIterator::undoExpunge -- 削除の取り消し
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		挿入する文書ID
//	const FullText2::SmartLocationList& cLocationList_
//		挿入する位置情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortNolocationNoTFListIterator::
undoExpunge(ModUInt32 uiDocumentID_,
			const SmartLocationList& cLocationList_)
{
	// 文書IDを挿入する
	undoExpungeDocumentID(uiDocumentID_);

	; _FULLTEXT2_FAKE_ERROR(ShortNolocationNoTFListIterator::undoExpunge);
}

//
//	FUNCTION public
//	FullText2::ShortNolocationNoTFListIterator::getHeadAddress
//		-- 現在の位置情報の先頭アドレスを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32*
//		現在の位置情報のアドレス
//
//	EXCEPTIONS
//
ModUInt32*
ShortNolocationNoTFListIterator::getHeadAddress()
{
	return 0;
}

//
//	FUNCTION public
//	FullText2::ShortNolocationNoTFListIterator::getLocationOffset
//		-- 現在の位置情報のオフセットを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		現在の位置情報のオフセット
//
//	EXCEPTIONS
//
ModSize
ShortNolocationNoTFListIterator::getLocationOffset()
{
	return 0;
}

//
//	FUNCTION public
//	FullText2::ShortNolocationNoTFListIterator::getLocationBitLength
//		-- 現在の位置情報のビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		現在の位置情報のビット長
//
//	EXCEPTIONS
//
ModSize
ShortNolocationNoTFListIterator::getLocationBitLength()
{
	return 0;
}

//
//	FUNCTION public
//	FullText2::ShortNolocationNoTFListIterator::getLocationDataOffset
//		-- 現在の位置情報データのオフセットを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		現在の位置情報データのオフセット
//
//	EXCEPTIONS
//
ModSize
ShortNolocationNoTFListIterator::getLocationDataOffset()
{
	return 0;
}

//
//	FUNCTION public
//	FullText2::ShortNolocationNoTFListIterator::getLocationDataBitLength
//		-- 現在の位置情報データのビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		現在の位置情報データのビット長
//
//	EXCEPTIONS
//
ModSize
ShortNolocationNoTFListIterator::getLocationDataBitLength()
{
	return 0;
}

//
//	FUNCTION public
//	FullText2::ShortNolocationNoTFListIterator::copy -- コピーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::ListIterator*
//		コピーしたオブジェクト
//
//	EXCEPTIONS
//
ListIterator*
ShortNolocationNoTFListIterator::copy() const
{
	return new ShortNolocationNoTFListIterator(*this);
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
