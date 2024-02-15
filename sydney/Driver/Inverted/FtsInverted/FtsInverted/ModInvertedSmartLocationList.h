// -*-Mode: C++; tab-width: 4; -*-
// vi:set ts=4 sw=4:	
//
// ModInvertedSmartLocationList.h -- 賢い文書内出現位置リスト
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedSmartLocationList_H__
#define __ModInvertedSmartLocationList_H__

#include "ModInvertedTypes.h"
#include "ModInvertedManager.h"
#include "ModInvertedException.h"
#include "ModInvertedCoder.h"
#include "ModInvertedLocationListIterator.h"

//
// CONST
// ModInvertedSmartLocationListMinimumUnitNum -- 最小領域ユニット数
//
// NOTES
// 領域確保において最小の領域のユニット数。
//
const int ModInvertedSmartLocationListMinimumUnitNum = 4;

//
// CONST
// ModInvertedSmartLocationListBlockUnitNum -- ブロックを構成する領域ユニット数
//
// NOTES
// ブロック単位に領域確保を行う場合のブロックの領域のユニット数。
//
const int ModInvertedSmartLocationListBlockUnitNum = 1024;

//
// MACRO
// MOD_INV_SMARTLOC_AS_VECTOR
//
// NOTES
// これを定義すると圧縮せず、ModVector で出現位置を保持する。
// Windows ではこれを定義しないとなぜか動作が正しくない。
// （検索時の ModInvertedQueryTokenizedResult の削除時に unexpected error が
// 発生する。）
// デフォルトでは、Windows 以外では定義しないが、Windows で定義する。
//
#if defined(OS_WINDOWSNT4_0) || defined(OS_WINDOWS98)
#define MOD_INV_SMARTLOC_AS_VECTOR
#endif

//
// MACRO
// MOD_INV_SMARTLOC_USE_MANAGER
//
// NOTES
// これを定義すると領域の確保に ModInvertedManager を使用する。
// デフォルトでは定義しない。
//
// #define MOD_INV_SMARTLOC_USE_MANAGER

//
// MACRO
// MOD_INV_SMARTLOC_UNION
//
// NOTES
// これを定義すると無名 UNION を使用する。
// Windows での問題を回避できるかと期待して導入してみたが、効果がなかった。
// デフォルトでは定義する。
//
#define MOD_INV_SMARTLOC_UNION


#ifdef MOD_INV_SMARTLOC_AS_VECTOR

//
// CLASS
// ModInvertedSmartLocationList -- 賢い文書内出現位置リスト
//
// NOTES
// MOD_INV_SMARTLOC_AS_VECTOR が定義されていない時とは異なり、単純に
// ModInvertedLocationList で出現位置を管理する。
//
class
ModInvertedSmartLocationList : public ModInvertedLocationList
{
public:
	ModInvertedSmartLocationList(ModInvertedCoder* coder_ = 0);

	void setFirstValue(const ModSize value);

	ModInvertedLocationListIterator* begin() const;

	ModSize getBitLength() const;
	ModSize getUnitNum() const;
	ModSize getDataSize() const;

	void setCoder(ModInvertedCoder* coder_);

private:
	ModInvertedCoder* coder;
};

#else // MOD_INV_SMARTLOC_AS_VECTOR

#ifdef SYD_INVERTED
#include "ModVector.h"
#endif

//
// CLASS
// ModInvertedSmartLocationList -- 賢い文書内出現位置リスト
//
// NOTES
// トークナイズ結果の文書内出現位置を賢く（メモリをあまり消費しないように）
// 管理するリスト。
// 管理方法は３段階に分かれている。
//
// (1)	領域の動的確保は行わず、用意されたメンバに値を保持する。
//		使えるメンバは２つなので、この状態はデータ数が２まで。
// (2)	領域を動的に確保し、その領域に圧縮せず値を保持する。
//		今回の実装ではデータ数４までがこの状態である。
// (3)	領域を動的に確保し、その領域に圧縮して値を保持する。
//		領域の最初の２つは、ビットオフセットと最終データ値に使用する。
//		領域の拡張方針は enlarge() に書いてある。
//
class
ModInvertedSmartLocationList : public ModInvertedObject
{
public:
	typedef ModInvertedDataUnit DataUnit;

	// CLASS
	// SimpleIterator1 -- 反復子（その１）
	//
	// NOTES
	// 値が 1 あるいは 2 個ある場合のための反復子
	//
	class SimpleIterator1 : public ModInvertedLocationListIterator
	{
	public:
		SimpleIterator1(const ModInvertedSmartLocationList* original_)
			: position(0), original(original_) {}
		void reset() { position = 0; }
		void next() { if (position < original->dataNum) ++position; }
		ModBoolean isEnd() const {
			return ModBoolean(position == original->dataNum); }
		ModSize getLocation() {
			switch (position) {
			case 0:
				return original->first;
			case 1:
				return original->second;
			default:
				break;
			}
			return 0;
		}
		ModSize getEndLocation() { ; ModAssert(0); return 0; }
	private:
		ModSize position;
		const ModInvertedSmartLocationList* original;
	};

	// CLASS
	// SimpleIterator2 -- 反復子（その２）
	//
	// NOTES
	// 値が 3 あるいは 4 個ある場合のための反復子
	//
	class SimpleIterator2 : public ModInvertedLocationListIterator
	{
	public:
		SimpleIterator2(const ModInvertedSmartLocationList* original_)
			: position(0), original(original_) {}
		void reset() { position = 0; }
		void next() { if (position < original->dataNum) ++position; }
		ModBoolean isEnd() const {
			return ModBoolean(position == original->dataNum); }
		ModSize getLocation() {
			if (position < original->dataNum) {
#ifdef MOD_INV_SMARTLOC_UNION
				return original->dataArea[position];
#else
				return ((DataUnit*)(original->second))[position];
#endif
			}
			return 0;
		}
		ModSize getEndLocation() { ; ModAssert(0); return 0; }
	private:
		ModSize position;
		const ModInvertedSmartLocationList* original;
	};
	friend class SimpleIterator1;
	friend class SimpleIterator2;

	ModInvertedSmartLocationList(ModInvertedCoder* coder_ = 0);
	ModInvertedSmartLocationList(const ModInvertedSmartLocationList&);

	~ModInvertedSmartLocationList();

	void clear();

	void pushBack(const ModSize value);
	void setFirstValue(const ModSize value);

	ModInvertedLocationListIterator* begin() const;

	ModSize getSize() const { return dataNum; }
	ModSize getBitLength() const;
	ModSize getUnitNum() const {
#ifdef MOD_INV_SMARTLOC_UNION
		return (dataNum <= 2) ? 0 : unitNum;
#else
		return (dataNum <= 2) ? 0 : first;
#endif
	}
	ModSize getDataSize() const;

	ModSize operator[](const ModSize) const;
	void setCoder(ModInvertedCoder* coder_) { coder = coder_; }

	ModInvertedSmartLocationList& operator=(const ModInvertedSmartLocationList&);

	void copy(DataUnit*, ModSize&) const;

private:
	void append(const ModSize value);
	void allocate();
	void enlarge();		// 圧縮してある時に使用


	ModInvertedCoder* coder;
	ModSize dataNum;
#ifdef MOD_INV_SMARTLOC_UNION
	union {
		ModSize unitNum;
		ModSize first;
	};
	union {
		DataUnit* dataArea;
		ModSize second;
	};
#else
	ModSize first;
	ModSize second;
#endif // MOD_INV_SMARTLOC_UNION
};

//
// FUNCTION 
// ModInvertedSmartLocationList::operator= -- 代入演算子
//
// NOTES
// 代入演算子。
//
// ARGUMENTS
// const ModInvertedSmartLocationList& orig_
//		コピー元
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModInvertedSmartLocationList&
ModInvertedSmartLocationList::operator=(
	const ModInvertedSmartLocationList& orig_)
{
	coder = orig_.coder;
	dataNum = orig_.dataNum;

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

	return *this;
}	

#if 0
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
inline
ModInvertedSmartLocationList::ModInvertedSmartLocationList(
	ModInvertedCoder* coder_)
	:
	coder(coder_), dataNum(0), first(0), second(0)
{}

inline
ModInvertedSmartLocationList::ModInvertedSmartLocationList(
	const ModInvertedSmartLocationList& orig_)
	:
	coder(orig_.coder), dataNum(orig_.dataNum)
{
	if (dataNum > 2) {
		// dataArea が確保されている場合
		unitNum = orig_.unitNum;
		allocate();
		ModOsDriver::memcpy(dataArea, orig_.dataArea, sizeof(DataUnit)*unitNum);
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
inline
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
inline void
ModInvertedSmartLocationList::clear()
{
	if (dataNum > 2) {
		// 2個以上のデータが入っているときは、領域が動的に確保されている
#ifdef MOD_INV_SMARTLOC_USE_MANAGER
		ModInvertedManager::free(dataArea, sizeof(DataUnit)*unitNum);
#else
		delete[] dataArea;
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
inline void
ModInvertedSmartLocationList::pushBack(const ModSize value)
{
	switch (dataNum) {
	case 0:
		first = value;
		break;
	case 1:
		second = value;
		break;
	case 2:
	{
		// これまで挿入されていた値を配列に入れる
		ModSize d1(first), d2(second);
		unitNum = ModInvertedSmartLocationListMinimumUnitNum;
		; ModAssert(unitNum > 2); 
		allocate();
		dataArea[0] = d1;
		dataArea[1] = d2;
		dataArea[2] = value;
	}
		break;
	case 3:
		; ModAssert(unitNum > 3); 
		dataArea[3] = value;
		break;
	case 4:
	{
		// これまで挿入されていた値を圧縮して詰める
		ModSize d1(dataArea[0]), d2(dataArea[1]), d3(dataArea[2]), d4(dataArea[3]);
		ModOsDriver::memset(dataArea, 0, sizeof(DataUnit)*unitNum);
		// dataArea[0] = 0;		// lastValue
		// dataArea[1] = 0;		// tailOffset
		append(d1);
		append(d2);
		append(d3);
		append(d4);
		// 新しい値を追加する
		append(value);
	}
		break;
	default:
		append(value);
		break;
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
inline void
ModInvertedSmartLocationList::setFirstValue(const ModSize value)
{
	; ModAssert(dataNum <= 1);
	dataNum = 1, unitNum = value;
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
inline void
ModInvertedSmartLocationList::append(const ModSize value)
{
	; ModAssert(value > dataArea[0]);
	ModSize gap(value - dataArea[0]);

	if (coder->append(gap, dataArea + 2, ((unitNum - 2)<<5), dataArea[1])
		== ModFalse) {
		// 現在の領域には詰められなかった
		enlarge();
		if (coder->append(gap, dataArea + 2, ((unitNum - 2)<<5), dataArea[1])
			== ModFalse) {
			// これはありえないはず
			ModThrowInvertedFileError(ModInvertedErrorInternal);
		}
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
inline void
ModInvertedSmartLocationList::allocate()
{
	try {
#ifdef MOD_INV_SMARTLOC_USE_MANAGER
		dataArea =
			(DataUnit*)ModInvertedManager::allocate(sizeof(DataUnit)*unitNum);
#else
		dataArea = new DataUnit[unitNum];
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
inline void
ModInvertedSmartLocationList::enlarge()
{
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
inline ModInvertedLocationListIterator*
ModInvertedSmartLocationList::begin() const
{
	if (dataNum > 4) {
		// 圧縮されてデータが格納されている場合
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
inline ModSize
ModInvertedSmartLocationList::getBitLength() const
{
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
		; ModAssert(ModSize(dataArea) > unitNum);
		bitLength += coder->getBitLength(ModSize(dataArea) - unitNum);
	case 1:
		; ModAssert(unitNum > 0);
		bitLength += coder->getBitLength(unitNum);
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
inline ModSize
ModInvertedSmartLocationList::getDataSize() const
{
	if (dataNum > 2) {
		// 圧縮されてデータが格納されている場合
		return sizeof(*this) + unitNum*sizeof(DataUnit);
	}
	return sizeof(*this);
}

inline ModSize 
ModInvertedSmartLocationList::operator[](const ModSize index) const
{
	; ModAssert(index >= dataNum);

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
#endif
#endif // MOD_INV_SMARTLOC_AS_VECTOR

#endif	// __ModInvertedSmartLocationList_H__

//
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
