// -*-Mode: C++; tab-width: 4; -*-
// vi:set ts=4 sw=4:	
//
// ModInvertedSmartLocationList.cpp -- 賢い文書内出現位置リスト
// 
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
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

#ifdef SYD_INVERTED // SYDNEY 対応
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#endif

#include "ModInvertedSmartLocationList.h"
#ifdef MOD_INV_SMARTLOC_AS_VECTOR
#include "ModInvertedUncompressedLocationListIterator.h"
#else
#include "ModInvertedCompressedLocationListIterator.h"
#endif // MOD_INV_SMARTLOC_AS_VECTOR

#ifdef MOD_INV_SMARTLOC_AS_VECTOR

//
// 関数ヘッダーは MOD_INV_SMARTLOC_AS_VECTOR が定義していない側に記述してある。
//

void
ModInvertedSmartLocationList::setFirstValue(const ModSize value)
{
	if (getSize() == 0)
		pushBack(value);
	else
		operator[](0) = value;
}

ModInvertedLocationListIterator*
ModInvertedSmartLocationList::begin() const
{
	return new ModInvertedUncompressedLocationListIterator(*this);
}

ModSize
ModInvertedSmartLocationList::getUnitNum() const
{
	return getSize();
}

ModSize
ModInvertedSmartLocationList::getDataSize() const
{
	return getSize()*sizeof(ModSize);
}

void
ModInvertedSmartLocationList::setCoder(ModInvertedCoder* coder_)
{
	coder = coder_;
}

ModInvertedSmartLocationList::ModInvertedSmartLocationList(ModInvertedCoder* coder_)
	: coder(coder_)
{}

ModSize
ModInvertedSmartLocationList::getBitLength() const
{
	if (getSize() == 0) {
		return 0;
	}

	ModSize bitLength(0);
	ModSize n(getSize() - 1);
	for (; n > 0; --n) {
		; ModAssert(operator[](n) > operator[](n - 1));
		bitLength += coder->getBitLength(operator[](n) - operator[](n - 1));
	}

	; ModAssert(operator[](0) > 0);
	bitLength += coder->getBitLength(operator[](0));

	return bitLength;
}

#else // MOD_INV_SMARTLOC_AS_VECTOR

//
// FUNCTION 
// ModInvertedSmartLocationList::ModInvertedSmartLocationList -- コンストラクタ
//
// NOTES
// コンストラクトする。
// 符合化器を引数にとる場合、デフォルト引数を用いるのは ModPair で
// デフォルト引数なしのものが必要になるから。
//
// ARGUMENTS
// ModInvertedCoder* coder_
//		符合化器
// const ModInvertedSmartLocationList& orig_
//		コピー元
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModInvertedSmartLocationList::ModInvertedSmartLocationList(
	ModInvertedCoder* coder_)
	:
	coder(coder_), dataNum(0), first(0), second(0)
{}

ModInvertedSmartLocationList::ModInvertedSmartLocationList(
	const ModInvertedSmartLocationList& orig_)
	:
	coder(orig_.coder), dataNum(orig_.dataNum)
{
	if (dataNum > 2) {
		// dataArea が確保されている場合
#ifdef MOD_INV_SMARTLOC_UNION
		unitNum = orig_.unitNum;
		allocate();
		ModOsDriver::memcpy(dataArea, orig_.dataArea, sizeof(DataUnit)*unitNum);
#else
		first = orig_.first;
		allocate();
		ModOsDriver::memcpy((DataUnit*)second, (const DataUnit*)orig_.second,
							sizeof(DataUnit)*first);
#endif
	} else {
		// dataArea が確保されていない場合
		first = orig_.first;
		second = orig_.second;
	}
}

//
// FUNCTION 
// ModInvertedSmartLocationList::~ModInvertedSmartLocationList -- デストラクタ
//
// NOTES
// デストラクトする。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModInvertedSmartLocationList::~ModInvertedSmartLocationList()
{
	clear();
}

//
// FUNCTION 
// ModInvertedSmartLocationList::clear -- クリア
//
// NOTES
// 必要に応じて領域を解放し、データが入っていない状態にする。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedSmartLocationList::clear()
{
	if (dataNum > 2) {
		// 2個以上のデータが入っているときは、領域が動的に確保されている
#ifdef MOD_INV_SMARTLOC_UNION
#ifdef MOD_INV_SMARTLOC_USE_MANAGER
		ModInvertedManager::free(dataArea, sizeof(DataUnit)*unitNum);
#else
		delete[] dataArea;
#endif
#else
#ifdef MOD_INV_SMARTLOC_USE_MANAGER
		ModInvertedManager::free((DataUnit*)second, sizeof(DataUnit)*first);
#else
		delete[] (DataUnit*)second;
#endif
#endif
	}
	dataNum = 0;
	first = 0;
	second = 0;
}

//
// FUNCTION 
// ModInvertedSmartLocationList::pushBack -- プッシュバック
//
// NOTES
// 値を最後の位置に追加する。
//
// ARGUMENTS
// const ModSize value
//		値
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedSmartLocationList::pushBack(const ModSize value)
{
#ifndef MOD_INV_SMARTLOC_UNION
	DataUnit* dataArea = (DataUnit*)second;
#endif

	if (dataNum > 4) {
		// 特許データでは、この条件を先に判定する方が少しだけ高速
		append(value);
	}
	else if (dataNum == 0) {
		first = value;
	}
	else if (dataNum == 1) {
		second = value;
	}
	else if (dataNum == 2) {
		// これまで挿入されていた値を配列に入れる
		ModSize d1(first), d2(second);
#ifdef MOD_INV_SMARTLOC_UNION
		unitNum = ModInvertedSmartLocationListMinimumUnitNum;
		; ModAssert(unitNum > 2); 
#else
		first = ModInvertedSmartLocationListMinimumUnitNum;
		; ModAssert(first > 2); 
#endif
		allocate();

		dataArea[0] = d1;
		dataArea[1] = d2;
		dataArea[2] = value;
	}
	else if (dataNum == 3) {
#ifdef MOD_INV_SMARTLOC_UNION
		; ModAssert(unitNum > 3); 
#else
		; ModAssert(first > 3); 
#endif
		dataArea[3] = value;
	}
	else if (dataNum == 4) {
		// これまで挿入されていた値を圧縮して詰める
		ModSize d1(dataArea[0]), d2(dataArea[1]), d3(dataArea[2]), d4(dataArea[3]);
		ModOsDriver::memset(dataArea, 0,
#ifdef MOD_INV_SMARTLOC_UNION
							sizeof(DataUnit)*unitNum
#else
							sizeof(DataUnit)*first
#endif
			);
		// dataArea[0] = 0;		// lastValue
		// dataArea[1] = 0;		// tailOffset
		append(d1);
		append(d2);
		append(d3);
		append(d4);
		// 新しい値を追加する
		append(value);
	}
	else {
		; ModAssert(0);
	}

	++dataNum;
}

//
// FUNCTION 
// ModInvertedSmartLocationList::setFirstValue -- 先頭の値の設定
//
// NOTES
// 先頭の値を設定する。
//
// ARGUMENTS
// const ModSize value
//		値
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModInvertedSmartLocationList::setFirstValue(const ModSize value)
{
	; ModAssert(dataNum <= 1);
	dataNum = 1, first = value;
}

//
// FUNCTION 
// ModInvertedSmartLocationList::append -- 追加
//
// NOTES
// 領域が確保されている場合に、値を圧縮して追加する。
//
// ARGUMENTS
// const ModSize value
//		値
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedSmartLocationList::append(const ModSize value)
{
#ifndef MOD_INV_SMARTLOC_UNION
	DataUnit* dataArea = (DataUnit*)second;
#endif

	; ModAssert(value > dataArea[0]);
	ModSize gap(value - dataArea[0]);

	while (coder->append(gap, dataArea + 2,
#ifdef MOD_INV_SMARTLOC_UNION
						 ((unitNum - 2)<<5),
#else
						 ((first - 2)<<5),
#endif
						 dataArea[1]) == ModFalse) {
		// 現在の領域には詰められなかったので、領域を大きくして再挑戦
		enlarge();
	}
	dataArea[0] = value;
}

//
// FUNCTION 
// ModInvertedSmartLocationList::allocate -- 領域確保
//
// NOTES
// unitNum 分の領域を確保する。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// ModOsErrorSystemMemoryExhaust
//		メモリが確保できない
//
void
ModInvertedSmartLocationList::allocate()
{
	try {
#ifdef MOD_INV_SMARTLOC_UNION
#ifdef MOD_INV_SMARTLOC_USE_MANAGER
		dataArea =
			(DataUnit*)ModInvertedManager::allocate(sizeof(DataUnit)*unitNum);
#else
		dataArea = new DataUnit[unitNum];
#endif
#else
#ifdef MOD_INV_SMARTLOC_USE_MANAGER
		second =
			(DataUnit*)ModInvertedManager::allocate(sizeof(DataUnit)*first);
#else
		second = (ModSize)(new DataUnit[first]);
#endif
#endif
	} catch (ModException& exception) {
		ModRethrow(exception);
	}
}
	
//
// FUNCTION 
// ModInvertedSmartLocationList::enlarge -- 領域の拡大
//
// NOTES
// 領域を拡大する。
// ModInvertedSmartLocationListLargeUnitNum より大きいときは、
// ModInvertedSmartLocationListLargeUnitNum 分だけ拡大し、
// ModInvertedSmartLocationListLargeUnitNum より小さいときは 2 倍にする。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedSmartLocationList::enlarge()
{
#ifdef MOD_INV_SMARTLOC_UNION
	ModSize oldNum(unitNum);
	DataUnit* oldArea = dataArea;

	if (unitNum >= ModInvertedSmartLocationListBlockUnitNum) {
		// ある程度大きくなったら BlockUnitNum 単位で大きくする
		unitNum += ModInvertedSmartLocationListBlockUnitNum;
	} else {
		// 大きくないうちは倍々にする
		unitNum *= 2;
	}

	allocate();

	ModOsDriver::memcpy(dataArea, oldArea, sizeof(DataUnit)*oldNum);
	ModOsDriver::memset(dataArea + oldNum, 0,
						sizeof(DataUnit)*(unitNum - oldNum));
#else
	ModSize oldNum(first);
	DataUnit* oldArea = (DataUnit*)second;

	if (first >= ModInvertedSmartLocationListBlockUnitNum) {
		// ある程度大きくなったら BlockUnitNum 単位で大きくする
		first += ModInvertedSmartLocationListBlockUnitNum;
	} else {
		// 大きくないうちは倍々にする
		first *= 2;
	}

	allocate();

	ModOsDriver::memcpy(((DataUnit*)second), oldArea, sizeof(DataUnit)*oldNum);
	ModOsDriver::memset(((DataUnit*)second) + oldNum, 0,
						sizeof(DataUnit)*(first - oldNum));
#endif

#ifdef MOD_INV_SMARTLOC_USE_MANAGER
	ModInvertedManager::free(oldArea, sizeof(DataUnit)*oldNum);
#else
	delete[] oldArea;
#endif
}
	
//
// FUNCTION
// ModInvertedSmartLocationList::begin -- 反復子の取得
//
// NOTES
// 反復子を返す。
//
// ARGUMENTS
// なし
//
// RETURN
// 反復子
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
ModInvertedLocationListIterator*
ModInvertedSmartLocationList::begin() const
{
	if (dataNum > 4) {
		// 圧縮されてデータが格納されている場合
#ifndef MOD_INV_SMARTLOC_UNION
		DataUnit* dataArea = (DataUnit*)second;
#endif
		return new ModInvertedCompressedLocationListIterator(
			dataArea + 2, 0, dataArea[1], dataNum, coder,
			&ModInvertedCoder::get);
	}
	if (dataNum > 2) {
		return new SimpleIterator2(this);
	}
	return new SimpleIterator1(this);
}

//
// FUNCTION
// ModInvertedSmartLocationList::getBitLength -- ビット長の取得
//
// NOTES
// 圧縮時のビット長を返す。
//
// ARGUMENTS
// なし
//
// RETURN
// ビット長
//
// EXCEPTIONS
// なし
//
ModSize
ModInvertedSmartLocationList::getBitLength() const
{
#ifndef MOD_INV_SMARTLOC_UNION
	DataUnit* dataArea = (DataUnit*)second;
#endif

	if (dataNum > 4) {
		// 圧縮されてデータが格納されている場合、記録されている値を返す
		return dataArea[1];
	}

	// 圧縮されていない場合、値を計算する

	ModSize bitLength(0);

	switch (dataNum) {
	case 4:
		; ModAssert(dataArea[3] > dataArea[2]);
		bitLength += coder->getBitLength(dataArea[3] - dataArea[2]);
	case 3:
		; ModAssert(dataArea[2] > dataArea[1]);
		bitLength += coder->getBitLength(dataArea[2] - dataArea[1]);
		; ModAssert(dataArea[1] > dataArea[0]);
		bitLength += coder->getBitLength(dataArea[1] - dataArea[0]);
		; ModAssert(dataArea[0] > 0);
		bitLength += coder->getBitLength(dataArea[0]);
		break;
	case 2:
		; ModAssert(second > first);
		bitLength += coder->getBitLength(second - first);
	case 1:
		; ModAssert(first > 0);
		bitLength += coder->getBitLength(first);
		break;
	default:
		; ModAssert(dataNum == 0);
		break;
	}
	return bitLength;
}

//
// FUNCTION
// ModInvertedSmartLocationList::getDataSize -- データサイズの取得
//
// NOTES
// データサイズ（バイト単位）を返す。
// データサイズは自分のサイズ＋確保した領域のサイズ。
//
// ARGUMENTS
// なし
//
// RETURN
// データサイズ
//
// EXCEPTIONS
// なし
//
ModSize
ModInvertedSmartLocationList::getDataSize() const
{
	if (dataNum > 2) {
		// 圧縮されてデータが格納されている場合
#ifdef MOD_INV_SMARTLOC_UNION
		return sizeof(*this) + unitNum*sizeof(DataUnit);
#else
		return sizeof(*this) + first*sizeof(DataUnit);
#endif
	}
	return sizeof(*this);
}

//
// FUNCTION
// ModInvertedSmartLocationList::operator[] -- []演算子
//
// NOTES
// index 番目の要素を返す。
//
// ARGUMENTS
// const ModSize index
//		要素のインデックス
//
// RETURN
// 要素
//
// EXCEPTIONS
// なし
//
ModSize 
ModInvertedSmartLocationList::operator[](const ModSize index) const
{
	; ModAssert(index < dataNum);

#ifndef MOD_INV_SMARTLOC_UNION
	DataUnit* dataArea = (DataUnit*)second;
#endif

	if (dataNum > 4) {
		ModInvertedCompressedLocationListIterator
			iterator(dataArea + 2, 0, dataArea[1], dataNum, coder,
					 &ModInvertedCoder::get);
		for (ModSize i(0); i < index; ++i) {
			iterator.next();
		}
		return iterator.getLocation();
	}
	if (dataNum > 2) {
		return dataArea[index];
	}
	if (index == 0) {
		return first;
	}
	; ModAssert(index == 1);
	return second;
}

//
// FUNCTION
// ModInvertedSmartLocationList::copy -- コピー
//
// NOTES
// 圧縮データを渡された領域にコピーする
//
// ARGUMENTS
// DataUnit* target_
//		コピーする領域
// ModSize& bitOffset_
//		コピーするビット位置
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
void
ModInvertedSmartLocationList::copy(DataUnit* target_, ModSize& bitOffset_) const
{
	; ModAssert(dataNum > 4);

#ifndef MOD_INV_SMARTLOC_UNION
	DataUnit* dataArea = (DataUnit*)second;
#endif

	ModInvertedCoder::move(dataArea + 2, 0, dataArea[1],
						   bitOffset_, target_);
	bitOffset_ += dataArea[1];
}	

#endif // MOD_INV_SMARTLOC_AS_VECTOR

//
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
