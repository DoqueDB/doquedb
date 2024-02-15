// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BatchBaseList.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Inverted/BatchBaseList.h"
#include "Inverted/BatchListMap.h"
#include "Inverted/Parameter.h"

#include "Common/Message.h"

#include "ModOsDriver.h"
#include "ModList.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

namespace
{
	//
	//	VARIABLE
	//	_$$::_cAllocateUnitSize -- エリアの初期ユニット数
	//
	ParameterInteger
	_cAllocateUnitSize("Inverted_BatchListInitialUnitSize", 32);	// 128B

	//
	//	VARIABLE
	//	_$$::_cRegularUnitSize -- 一定間隔でアロケートするユニット数
	//
	ParameterInteger
	_cRegularUnitSize("Inverted_BatchListRegularUnitSize", 1024);	// 4KB

	//
	//	VARIABLE
	//	_$$::_cMaxUnitSize -- エリアの最大ユニット数
	//
	ParameterInteger
	_cMaxUnitSize("Inverted_BatchListMaxUnitSize", 16384);			// 64KB
}

//
//	FUNCTION public
//	Inverted::BatchBaseList::BatchBaseList -- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	Inverted::BatchBaseListMap& cBatchListMap_
//		バッチリストマップ
//	const ModUnicodeChar* pKey_
//		索引単位
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
BatchBaseList::BatchBaseList(InvertedUnit& cInvertedUnit_,
					 BatchListMap& cBatchListMap_,
					 const ModUnicodeChar* pszKey_)
: InvertedList(cInvertedUnit_, pszKey_, ListType::Batch)
{
}

//
//	FUNCTION public
//	Inverted::BatchBaseList::BatchBaseList -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	LeafPage::Area* pArea_
//		リーフページのエリア
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
BatchBaseList::BatchBaseList(InvertedUnit& cInvertedUnit_, LeafPage::Area* pArea_)
	: InvertedList(cInvertedUnit_,
				   pArea_->getKey(),
				   pArea_->getKeyLength(),
				   ListType::Batch)
{
}

//
//	FUNCTION public
//	Inverted::BatchBaseList::~BatchBaseList -- デストラクタ
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
BatchBaseList::~BatchBaseList()
{
}

//
//	FUNCTION public
//	Inverted::BatchBaseList::insert -- 転置リストの挿入(1文書挿入用)
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//	const ModInvertedSmartLocationList& cLocationList_
//		位置情報リスト
//
//	RETURN
//	bool
//		挿入できた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
BatchBaseList::insert(ModUInt32 uiDocumentID_,
					  const ModInvertedSmartLocationList& cLocationList_)
{
	//
	//	必要なユニット数を求める
	//
	ModSize idLength = 0;
	if (getArea()->getDocumentCount())
	{
		idLength = getCompressedBitLengthDocumentID(
			getArea()->getLastDocumentID(), uiDocumentID_);
	}
	ModSize bitLength;
	ModSize locLength
		= getCompressedBitLengthLocationList(cLocationList_, bitLength);
	ModSize totalLength = getArea()->getDocumentOffset() + idLength
							+ getArea()->getLocationOffset() + locLength;
	ModSize unitSize = calcUnitSize(totalLength);

	//
	//	大きさをチェックする
	//
	if (getArea()->getDataUnitSize() < unitSize)
	{
		// 広げる
		ModSize newSize = getArea()->getDataUnitSize();
		
		if (unitSize >= static_cast<ModSize>(_cRegularUnitSize.get()))
		{
			while (newSize < unitSize)
				newSize += _cRegularUnitSize.get();
		}
		else
		{
			while (newSize < unitSize)
				newSize *= 2;
		}

		if (newSize > static_cast<ModSize>(_cMaxUnitSize.get()) &&
			getArea()->getDocumentCount() != 0)
		{
			// エリアの最大値を超えた
			return false;
		}
		
		ModSize expand = newSize - getArea()->getDataUnitSize();
		LeafPage::Area* pArea = 0;

		try
		{
			pArea = LeafPage::Area::allocateArea(getKey(),
										   getArea()->getDataUnitSize() + expand);
		}
		catch (Exception::Object&)
		{
			// メモリーの確保に失敗した
			return false;
		}

		// コピーする
		pArea->copy(getArea());
		// 文書IDを広げた分移動する
		ModSize unit = calcUnitSize(getArea()->getDocumentOffset());
		ModOsDriver::Memory::move(pArea->getTailAddress() - unit,
			pArea->getTailAddress() - unit - expand, unit*sizeof(ModUInt32));
		setOff(pArea->getHeadAddress(), pArea->getLocationOffset(), 
			pArea->getDataUnitSize() * 32 - pArea->getLocationOffset() - pArea->getDocumentOffset());

		setArea(pArea);

		// バッチリストマップに広げた分のバイト数を登録する
		getMap()->addListSize(expand * sizeof(ModUInt32));
	}

	//
	//	文書IDを書く
	//
	if (getArea()->getDocumentCount() == 0)
	{
		getArea()->setFirstDocumentID(uiDocumentID_);
	}
	else
	{
		// 文書IDビットオフセット
		ModSize offset = getArea()->getDocumentOffset();
		// 圧縮して格納する
		writeDocumentID(getArea()->getLastDocumentID(), uiDocumentID_, getArea()->getTailAddress(), offset);
		// 文書IDビットオフセットを設定する
		getArea()->setDocumentOffset(offset);
	}

	//
	//	位置情報リスト(とTF)を書く
	//
	{
		// 位置情報ビットオフセット
		ModSize offset = getArea()->getLocationOffset();
		// 圧縮して格納する
		writeLocationList(cLocationList_, bitLength, getArea()->getHeadAddress(), offset);
		// 位置情報ビットオフセットを設定する
		getArea()->setLocationOffset(offset);
	}

	// 最終文書IDを設定する
	getArea()->setLastDocumentID(uiDocumentID_);
	// 頻度情報を設定する
	getArea()->setDocumentCount(getArea()->getDocumentCount() + 1);

	return true;
}

//
//	FUNCTION protected
//	Inverted::BatchBaseList::getAllocateUnitSize -- エリアの初期ユニット数を取得する
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModSize
//
//	EXCEPTIONS
//
ModSize
BatchBaseList::getAllocateUnitSize() const
{
	return _cAllocateUnitSize.get();
}

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
