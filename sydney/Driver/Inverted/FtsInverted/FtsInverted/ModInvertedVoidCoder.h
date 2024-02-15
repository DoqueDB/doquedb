// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedVoidCoder.h -- 圧縮しない符号器
// 
// Copyright (c) 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedVoidCoder_H__
#define __ModInvertedVoidCoder_H__

#ifdef V1_4

#include "ModCharString.h"
#include "ModInvertedTypes.h"
#include "ModInvertedCoder.h"

class ModInvertedCoderParameter;

//
// CLASS
// ModInvertedVoidCoder -- 圧縮しない符号器
//
// NOTES
// 圧縮を行なわない符号器の実装。
class
ModInvertedVoidCoder : public ModInvertedCoder
{
	friend class ModInvertedCoder;
	friend class ModInvertedCoderParameter;
public:
#ifdef CODER_HAS_CURRENT_LOCATION
	ModInvertedCoder* duplicate() const;
#endif

	// 圧縮したビット長を得る
	ModSize getBitLength(const ModSize) const;

	// 圧縮してデータを格納する
	void append(const ModSize, Unit*, ModSize&) const;
	ModBoolean append(const ModSize, Unit*, const ModSize, ModSize&) const;
	void appendBack(const ModSize, Unit*, ModSize&) const;
	ModBoolean appendBack(const ModSize, Unit*, const ModSize,
						  ModSize&) const;

	// 伸長してデータを格納する
	ModBoolean get(ModSize&, const Unit*, const ModSize, ModSize&) const;
	ModBoolean getBack(ModSize&, const Unit*, const ModSize,
					   ModSize&) const;

	virtual void getDescription(ModCharString& description_) const;

private:
	static void getBack(Unit*, ModSize, ModSize, ModSize&);

	// .cpp ファイルは作らないので、ModInvertedCoder.cpp で定義する
	static const char coderName[];
};


#ifdef CODER_HAS_CURRENT_LOCATION
//
// FUNCTION
// ModInvertedVoidCoder::duplicate -- 複製の生成
//
// NOTES
// 自分の複製を生成する。
//
// ARGUMENTS
// なし
//
// RETURN
// 複製された符号器
//
// EXCEPTIONS
// 下位のモジュールからの例外をそのまま返す
// 
inline ModInvertedCoder*
ModInvertedVoidCoder::duplicate() const
{
	return new ModInvertedVoidCoder;
}
#endif

//
// FUNCTION
// ModInvertedVoidCoder::getBitLength --- ビット長の計算
//
// NOTES
// value が占めるビット長を返す。
// 
// ARGUMENTS
// const ModSize value
//		計算対象
//
// RETURN
// ビット長
//
// EXCEPTIONS
// なし
// 
inline ModSize
ModInvertedVoidCoder::getBitLength(const ModSize value) const
{
	return ModInvertedDataUnitBitSize;
}


//
// FUNCTION
// ModInvertedVoidCoder::append --- 値を前向きにセット
//
// NOTES
// value をデータ領域 dataBegin の bitOffset から圧縮して、前向きに
// セットする。詰め終った位置は bitOffset にセットし直される。
// 領域にデータを詰めるだけのスペースがあるかのチェックを行なわない 3 引数の
// ものと、行なう 4 引数のものがある。
// 
// ARGUMENTS
// const ModSize value
//		計算対象
// Unit* dataBegin
//		データをセットする領域の先頭
// const ModSize dataLength
//		データ領域の長さ（ビット単位）
// ModSize& bitOffset
//		データのセット開始位置の領域の先頭からのビット位置
//
// RETURN
// 3 引数の場合、なし
// 4 引数の場合、データのセットに成功すれば ModTrue, そうでなければ ModFalse
//
// EXCEPTIONS
// なし
// 
inline void
ModInvertedVoidCoder::append(const ModSize value,
							 Unit* dataBegin,
							 ModSize& bitOffset) const
{
	; ModAssert(value > 0);

	if ((bitOffset%ModInvertedDataUnitBitSize) == 0) {
		// ちょうど境界に一致している時
		dataBegin[bitOffset/ModInvertedDataUnitBitSize] = value;
	} else {
		ModFileOffset tmp(value);
		set(dataBegin, bitOffset, ModInvertedDataUnitBitSize, tmp);
	}
	bitOffset += ModInvertedDataUnitBitSize;
}

inline ModBoolean
ModInvertedVoidCoder::append(const ModSize value,
							 Unit* dataBegin,
							 const ModSize dataLength,
							 ModSize& bitOffset) const
{
	; ModAssert(value > 0);

	if (bitOffset + ModInvertedDataUnitBitSize > dataLength) {
		// データは収まらない
		return ModFalse;
	}

	if ((bitOffset%ModInvertedDataUnitBitSize) == 0) {
		// ちょうど境界に一致している時
		dataBegin[bitOffset/ModInvertedDataUnitBitSize] = value;
	} else {
		ModFileOffset tmp(value);
		set(dataBegin, bitOffset, ModInvertedDataUnitBitSize, tmp);
	}
	bitOffset += ModInvertedDataUnitBitSize;

	return ModTrue;
}


//
// FUNCTION
// ModInvertedVoidCoder::appendBack --- 値を後向きにセット
//
// NOTES
// value をデータ領域 dataBegin の bitOffset から圧縮して、後向きに
// セットする。詰め終った位置は bitOffset にセットし直される。
// 領域にデータを詰めるだけのスペースがあるかのチェックを行なわない 3 引数の
// ものと、行なう 4 引数のものがある。
// 
// ARGUMENTS
// const ModSize value
//		計算対象
// Unit* dataTail
//		データをセットする領域の先頭
// const ModSize dataLength
//		データ領域の長さ（ビット単位）
// ModSize& bitOffset
//		データのセット開始位置の領域の先頭からのビット位置
//
// RETURN
// 3 引数の場合、なし
// 4 引数の場合、データのセットに成功すれば ModTrue, そうでなければ ModFalse
//
// EXCEPTIONS
// なし
// 
inline void
ModInvertedVoidCoder::appendBack(const ModSize value,
								 Unit* dataTail,
								 ModSize& bitOffset) const
{
	; ModAssert(value > 0);

	if ((bitOffset%ModInvertedDataUnitBitSize) == 0) {
		*(dataTail - (bitOffset/ModInvertedDataUnitBitSize)) = value;
	} else {
		ModFileOffset tmp(value);
		setBack(dataTail, bitOffset, ModInvertedDataUnitBitSize, tmp);
	}
	bitOffset += ModInvertedDataUnitBitSize;
}

/* purecov: begin deadcode */
//		appendBack() は shortList でのみ使用されるが、shortList では圧縮データ
//		が領域に収まるかを事前にチェックしておくので、圧縮しながら領域に収まる
//		かを調べるタイプの appendBack() はどこからも使われない
inline ModBoolean
ModInvertedVoidCoder::appendBack(const ModSize value,
								 Unit* dataTail,
								 const ModSize dataLength,
								 ModSize& bitOffset) const
{
	; ModAssert(value > 0);

	if (bitOffset + ModInvertedDataUnitBitSize > dataLength) {
		// データは収まらない
		return ModFalse;
	}

	if ((bitOffset%ModInvertedDataUnitBitSize) == 0) {
		*(dataTail - (bitOffset/ModInvertedDataUnitBitSize)) = value;
	} else {
		Unit tmp(value);
		move(&tmp,
			 0, ModInvertedDataUnitBitSize, bitOffset, dataTail);
	}
	bitOffset += ModInvertedDataUnitBitSize;

	return ModTrue;
}
/* purecov: end */


//
// FUNCTION
// ModInvertedVoidCoder::get --- 値を前向きに取得
//
// NOTES
// データ領域 dataBegin の bitOffset から前向きに圧縮されている値を 
// value にセットする。新しい位置は bitOffset にセットし直される。
// bitOffset から bitLength までがすべて 0 であれば有効な値がセットされて
// いないことになるので、ModFalse を返す。
//
// ARGUMENTS
// ModSize& value
//		取得した値をセットする
// const Unit* dataBegin
//		データを取得する領域の先頭
// const ModSize bitLength
//		データ領域の長さ（ビット単位）
// ModSize& bitOffset
//		データの取得開始位置の領域の先頭からのビット位置
//
// RETURN
// データの取得に成功すれば ModTrue, そうでなければ ModFalse
//
// EXCEPTIONS
// なし
// 
inline ModBoolean
ModInvertedVoidCoder::get(ModSize& value,
						  const Unit* dataBegin,
						  const ModSize bitLength,
						  ModSize& bitOffset) const
{
	if (bitOffset >= bitLength) {
		// ここは事前のチェックなので等号が必要
		return ModFalse;
	}
#ifdef DEBUG
	++*((int*)(&count));
#endif

	if (bitOffset + ModInvertedDataUnitBitSize > bitLength) {
		// データが収まるだけの領域がない
		return ModFalse;
	}

	if ((bitOffset%ModInvertedDataUnitBitSize) == 0) {
		value = dataBegin[bitOffset/ModInvertedDataUnitBitSize];
	} else {
		Unit tmp;
		move(const_cast<Unit*>(dataBegin),
			 bitOffset, bitOffset + ModInvertedDataUnitBitSize, 0, &tmp);
		value = tmp;
	}
	if (value == 0) {
		// 0 はセットされないはず
		return ModFalse;
	}

	bitOffset += ModInvertedDataUnitBitSize;

	return ModTrue;
}


//
// FUNCTION
// ModInvertedVoidCoder::getBack --- 値を後向きに取得
//
// NOTES
// データ領域 dataBegin の bitOffset から後向きに圧縮されている値を 
// value にセットする。新しい位置は bitOffset にセットし直される。
// bitOffset から bitLength までがすべて 0 であれば有効な値がセットされて
// いないことになるので、ModFalse を返す。
//
// ARGUMENTS
// ModSize& value
//		取得した値をセットする
// const Unit* dataTail
//		データを取得する領域の先頭
// const ModSize bitLength
//		データ領域の長さ（ビット単位）
// ModSize& bitOffset
//		データの取得開始位置の領域の先頭からのビット位置
//
// RETURN
// データの取得に成功すれば ModTrue, そうでなければ ModFalse
//
// EXCEPTIONS
// なし
// 
inline ModBoolean
ModInvertedVoidCoder::getBack(ModSize& value,
							  const Unit* dataTail,
							  const ModSize bitLength,
							  ModSize& bitOffset) const
{
	if (bitOffset >= bitLength) {
		// ここは事前のチェックなので等号が必要
		return ModFalse;
	}
#ifdef DEBUG
	++*((int*)(&count));
#endif

	if (bitOffset + ModInvertedDataUnitBitSize > bitLength) {
		// データが収まるだけの領域がない
		return ModFalse;
	}

	if ((bitOffset%ModInvertedDataUnitBitSize) == 0) {
		value = *(dataTail - (bitOffset/ModInvertedDataUnitBitSize));
	} else {
		getBack(const_cast<Unit*>(dataTail),
				bitOffset, ModInvertedDataUnitBitSize, value);
	}
	if (value == 0) {
		// 0 はセットされないはず
		return ModFalse;
	}

	bitOffset += ModInvertedDataUnitBitSize;

	return ModTrue;
}

inline void
ModInvertedVoidCoder::getBack(Unit* dataTail,
							  ModSize bitOffset,
							  ModSize bitLength,
							  ModSize& value)
{
	dataTail -= (bitOffset>>5);
	bitOffset &= 31;

	// bitOffset はシフトするのに使用するので、set() とは異なり bitLength を
	// 判定に用いる
	bitLength += bitOffset;

	// この関数は public な getBack からしから呼ばれない。
	// VoidCoder では値の長さは 32 ビット固定であるが、この関数が呼び出される
	// 時は値の開始位置がユニット境界とずれている場合だけである。
	// したがって、以下の場合わけのうち、２ユニットに跨る場合しか起こり得ない。
	if (bitLength <= 32) {
		// １つのユニットに収まる場合
		; ModAssert(0);
#if 0
		value = (ModSize)((*dataTail)<<bitOffset) & 0xffffffff;
#endif
	} else if (bitLength <= 64) {
		// ２つのユニットに跨る場合 - これしか呼ばれないはず
		value = (ModSize)((*dataTail)>>bitOffset) & 0xffffffff;
		--dataTail;
		value += (ModSize)((*dataTail)<<(32 - bitOffset)) & 0xffffffff;
	} else {
		// ３つのユニットに跨る場合
		; ModAssert(0);
#if 0
		ModFileOffset bitString(0);
		bitString |= (ModSize)((*dataTail)<<bitOffset) & 0xffffffff;
		--dataTail;
		bitString |= (ModSize)((*dataTail)>>(32 - bitOffset)) & 0xffffffff;
		--dataTail;
		bitString |= (ModSize)((*dataTail)>>(64 - bitOffset)) & 0xffffffff;
		bitString >>= bitOffset;
		value = bitString;
#endif
	}
}

//
// FUNCTION
// ModInvertedVoidCoder::getDescription -- 符合器記述の取得
//
// NOTES
// 符合器記述を取得する。
//
// ARGUMENTS
// ModCharString& description_
//		出力バッファ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedVoidCoder::getDescription(ModCharString& description_) const
{
	description_ = coderName;
}

#endif // V1_4

#endif	// __ModInvertedVoidCoder_H__

//
// Copyright (c) 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
