// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedUnaryCoder.h -- UNARY法による符号器
// 
// Copyright (c) 2001, 2002, 2004, 2010, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedUnaryCoder_H__
#define __ModInvertedUnaryCoder_H__

#include "FullText2/Module.h"

#include "ModInvertedCoder.h"

class ModInvertedUnaryCoder : public ModInvertedCoder
{
	friend class ModInvertedCoder;
	
public:
	// 圧縮したビット長を得る
	ModSize getBitLength(const ModSize) const;

	// 圧縮してデータを格納する
	void append(const ModSize, Unit*, ModSize&) const;
	ModBoolean append(const ModSize, Unit*, const ModSize, ModSize&) const;
	void appendBack(const ModSize, Unit*, ModSize&) const;
	ModBoolean appendBack(const ModSize, Unit*, const ModSize, ModSize&) const;

	// 伸長してデータを格納する
	ModBoolean get(ModSize&, const Unit*, const ModSize, ModSize&) const;
	ModBoolean getBack(ModSize&, const Unit*, const ModSize, ModSize&) const;

	ModBoolean find(const ModSize, const Unit*,
					const ModSize, const ModSize) const;
	ModBoolean findBack(const ModSize, const Unit*,
						const ModSize, const ModSize) const;
	ModBoolean searchable() const { return ModTrue; }

private:

	// .cpp ファイルは作らないので、ModInvertedCoder.cpp で定義する("UNA")
	static const ModUnicodeChar coderName[];
};

//
// FUNCTION
// ModInvertedUnaryCoder::getBitLength --- ビット長の計算
//
// NOTES
// value が占めるビット長を返す。
//
// ARGUMENTS
// const ModSize value
//	  計算対象
//
// RETURN
// ビット長
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedUnaryCoder::getBitLength(const ModSize value) const
{
	return value;
}

//
// FUNCTION
// ModInvertedUnaryCoder::append --- 値を前向きにセット
//
// NOTES
// value をデータ領域 dataBegin の bitOffset から圧縮して、前向きに
// セットする。詰め終った位置は bitOffset にセットし直される。
// 領域にデータを詰めるだけのスペースがあるかのチェックを行なわない 3 引数の
// ものと、行なう 4 引数のものがある。
//
// ARGUMENTS
// const ModSize value
//	  計算対象
// Unit* dataBegin
//	  データをセットする領域の先頭
// const ModSize dataLength
//	  データ領域の長さ（ビット単位）
// ModSize& bitOffset
//	  データのセット開始位置の領域の先頭からのビット位置
//
// RETURN
// 3 引数の場合、なし
// 4 引数の場合、データのセットに成功すれば ModTrue, そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedUnaryCoder::append(const ModSize value,
							  Unit* dataBegin,
							  ModSize& bitOffset) const
{
	; ModAssert(value > 0);

	set(dataBegin, bitOffset + value - 1, 1, 1);
	bitOffset += value;
}

inline ModBoolean
ModInvertedUnaryCoder::append(const ModSize value,
							  Unit* dataBegin,
							  const ModSize dataLength,
							  ModSize& bitOffset) const
{
	; ModAssert(value > 0);

	if (//bitOffset >= dataLength ||
		(bitOffset + getBitLength(value)) > dataLength) {
		// データは収まらない
		return ModFalse;
	}

	set(dataBegin, bitOffset + value - 1, 1, 1);
	bitOffset += value;

	return ModTrue;
}

//
// FUNCTION
// ModInvertedUnaryCoder::appendBack --- 値を後向きにセット
//
// NOTES
// value をデータ領域 dataBegin の bitOffset から圧縮して、後向きに
// セットする。詰め終った位置は bitOffset にセットし直される。
// 領域にデータを詰めるだけのスペースがあるかのチェックを行なわない 3 引数の
// ものと、行なう 4 引数のものがある。
//
// ARGUMENTS
// const ModSize value
//	  計算対象
// Unit* dataTail
//	  データをセットする領域の先頭
// const ModSize dataLength
//	  データ領域の長さ（ビット単位）
// ModSize& bitOffset
//	  データのセット開始位置の領域の先頭からのビット位置
//
// RETURN
// 3 引数の場合、なし
// 4 引数の場合、データのセットに成功すれば ModTrue, そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedUnaryCoder::appendBack(const ModSize value,
								  Unit* dataTail,
								  ModSize& bitOffset) const
{
	; ModAssert(value > 0);

	setBack(dataTail, bitOffset + value - 1, 1, 1);
	bitOffset += value;
}

/* purecov: begin deadcode */
//		appendBack() は shortList でのみ使用されるが、shortList では圧縮データ
//		が領域に収まるかを事前にチェックしておくので、圧縮しながら領域に収まる
//		かを調べるタイプの appendBack() はどこからも使われない
inline ModBoolean
ModInvertedUnaryCoder::appendBack(const ModSize value,
								  Unit* dataTail,
								  const ModSize dataLength,
								  ModSize& bitOffset) const
{
	; ModAssert(value > 0);

	if (//bitOffset >= dataLength ||
		(bitOffset + getBitLength(value)) > dataLength) {
		// データは収まらない
		return ModFalse;
	}

	setBack(dataTail, bitOffset + value - 1, 1, 1);
	bitOffset += value;

	return ModTrue;
}
/* purecov: end */

//
// FUNCTION
// ModInvertedUnaryCoder::get --- 値を前向きに取得
//
// NOTES
// データ領域 dataBegin の bitOffset から前向きに圧縮されている値を
// value にセットする。新しい位置は bitOffset にセットし直される。
// bitOffset から bitLength までがすべて 0 であれば有効な値がセットされて
// いないことになるので、ModFalse を返す。
//
// ARGUMENTS
// ModSize& value
//	  取得した値をセットする
// const Unit* dataBegin
//	  データを取得する領域の先頭
// const ModSize bitLength
//	  データ領域の長さ（ビット単位）
// ModSize& bitOffset
//	  データの取得開始位置の領域の先頭からのビット位置
//
// RETURN
// データの取得に成功すれば ModTrue, そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedUnaryCoder::get(ModSize& value,
						   const Unit* dataBegin,
						   const ModSize bitLength,
						   ModSize& bitOffset) const
{
	if (bitOffset >= bitLength) {
		// ここは事前のチェックなので等号が必要
		return ModFalse;
	}

	ModSize bitMask(0x80000000>>(bitOffset&31));
	ModSize tmpOffset(bitOffset + 1);
	dataBegin += bitOffset>>5;
	Unit tmp(*dataBegin);

	for (; (tmp&bitMask) == 0; ++tmpOffset) {
		// 最初の１回は無駄になる
		if (tmpOffset >= bitLength) {
			return ModFalse;
		}
		if (bitMask == 1) {
			tmp = *++dataBegin;
			while (tmp == 0) {
				tmpOffset += 32;
				if (tmpOffset >= bitLength) {
					return ModFalse;
				}
				tmp = *++dataBegin;
			}
			bitMask = 0x80000000;
		} else {
			bitMask >>= 1;
		}
	}

	value = tmpOffset - bitOffset;
	bitOffset = tmpOffset;

	return ModTrue;
}

//
// FUNCTION
// ModInvertedUnaryCoder::getBack --- 値を後向きに取得
//
// NOTES
// データ領域 dataBegin の bitOffset から後向きに圧縮されている値を
// value にセットする。新しい位置は bitOffset にセットし直される。
// bitOffset から bitLength までがすべて 0 であれば有効な値がセットされて
// いないことになるので、ModFalse を返す。
//
// ARGUMENTS
// ModSize& value
//	  取得した値をセットする
// const Unit* dataTail
//	  データを取得する領域の先頭
// const ModSize bitLength
//	  データ領域の長さ（ビット単位）
// ModSize& bitOffset
//	  データの取得開始位置の領域の先頭からのビット位置
//
// RETURN
// データの取得に成功すれば ModTrue, そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedUnaryCoder::getBack(ModSize& value,
							   const Unit* dataTail,
							   const ModSize bitLength,
							   ModSize& bitOffset) const
{
	if (bitOffset >= bitLength) {
		// ここは事前のチェックなので等号が必要
		return ModFalse;
	}

	dataTail -= bitOffset>>5;
	ModSize bitMask(1<<(bitOffset&31));
	Unit tmp(*dataTail);
	value = 1;

	for (; (tmp&bitMask) == 0; ++value) {
		// 最初の１回は無駄になる
		if (bitOffset + value >= bitLength) {
			return ModFalse;
		}
		if (bitMask == 0x80000000) {
			tmp = *--dataTail;
			while (tmp == 0) {
				value += 32;
				if (bitOffset + value >= bitLength) {
					return ModFalse;
				}
				tmp = *--dataTail;
			}
			bitMask = 1;
		} else {
			bitMask <<= 1;
		}
	}

	bitOffset += value;

	return ModTrue;
}

//
// FUNCTION
// ModInvertedUnaryCoder::find --- 値がデータ領域に存在するか前向きにビット直接参照で調査
//
// NOTES
// パラメータで指定された値がデータ領域に存在するかどうかを
// 先頭からのビット数で直接参照して調査する。
//
// ARGUMENTS
// const ModSize value
//    調査対象の値
// const Unit* dataBegin
//    データを取得する領域の先頭
// const ModSize bitLength
//    データ領域の長さ（ビット単位）
// const ModSize bitOffset
//    データ領域の先頭のビット位置
//
// RETURN
// 値が存在すれば ModTrue, そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedUnaryCoder::find(const ModSize value,
							const Unit* dataBegin,
							const ModSize bitLength,
							const ModSize bitOffset) const
{
	ModSize position(bitOffset + value);	// 先頭から何ビット目か
	ModSize unitPos(position>>5);			// 先頭から何個目のユニットか
	ModSize bitPos(position&31);			// そのユニットの何ビット目か

	if(bitPos == 0) {
		// 0ビット目とは、最終ビットを指す
		bitPos = 31;
		--unitPos;
	} else {
		--bitPos;
	}

	if(position > bitLength ||
		(*(dataBegin+unitPos) & (0x80000000>>bitPos) ) == 0) {
		return ModFalse;
	}

	return ModTrue;
} 

//
// FUNCTION
// ModInvertedUnaryCoder::findBack --- 値がデータ領域に存在するか後向きにビット直接参照で調査
//
// NOTES
// パラメータで指定された値がデータ領域に存在するかどうかを
// 後方からのビット数で直接参照して調査する。
//
// ARGUMENTS
// const ModSize value
//    調査対象の値
// const Unit* dataBegin
//    データを取得する領域の先頭
// const ModSize bitLength
//    データ領域の長さ（ビット単位）
// const ModSize bitOffset
//    データ領域の先頭のビット位置
//
// RETURN
// 値が存在すれば ModTrue, そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedUnaryCoder::findBack(const ModSize value,
								const Unit* dataTail,
								const ModSize bitLength,
								const ModSize bitOffset) const
{
	ModSize position(bitOffset + value);	// 先頭から何ビット目か
	ModSize unitPos(position>>5);			// 先頭から何個目のユニットか
	ModSize bitPos(position&31);			// そのユニットの何ビット目か

	if(bitPos == 0) {
		// 0ビット目とは、一つ前のユニットの最終ビットを指す
		--unitPos;
		bitPos = 31;
	} else {
		--bitPos;
	}

	if(position > bitLength ||
		(*(dataTail-unitPos) & (1<<bitPos) ) == 0) {
		return ModFalse;
	}
	return ModTrue;
} 

#endif// __ModInvertedUnaryCoder_H__

//
// Copyright (c) 2001, 2002, 2004, 2010, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
