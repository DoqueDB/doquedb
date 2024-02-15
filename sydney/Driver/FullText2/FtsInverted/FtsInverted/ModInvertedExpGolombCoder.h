// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedExpGolombCoder.h -- Exponential Golomb 法による符号器
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2002, 2004, 2010, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedExpGolombCoder_H__
#define __ModInvertedExpGolombCoder_H__

#include "FullText2/Module.h"

#include "ModInvertedCoder.h"

//
// CLASS
// ModInvertedExpGolombCoder -- ExpGolomb法による符号器
//
// NOTES
// 転置リストデータの圧縮伸長を行なう符号器の ExpGolomb 法（δコード）による
// 実装。
// δコードについては以下の文書を参考にした。
//
//	  *	I.H. Witten,  A. Moffat, and T.C. Bell, "Managing Gigabytes: Compressing
//		and Indexing Documents and Images", Van Nostrand Reinhold, pp. 83--85,
//		1994
//    *	J. Teuhola, "A compression method for clustered bit-vectors", Information
//		Processing Letters, 7(6), 1978
//
// ただし、あるブロックに圧縮されたデータがないことを表すために 1 を詰める
// 手間を省くため、prefix と separator の 0, 1 をオリジナルとは逆にした。
// 以下に、符合の例を示す。
//
//		value	codes(前向き)	codes(後向き)
//		1		1				1
//		2		0,1,0			0,1,0
//		3		0,1,1			1,1,0
//		4		00,1,00			00,1,00
//		5		00,1,01			01,1,00
//		6		00,1,10			10,1,00
//		7		00,1,11			11,1,00
//		8		000,1,000		000,1,000
//		9		000,1,001		001,1,000
//		10		000,1,010		010,1,000
//
// この方式では、x = y + z のとき、x のビット数が y のビット数と 
// z のビット数の和より大きくなることがあるので、文書IDの圧縮に
// 使用することができない。
// 例えば、2 = 1 + 1 において、2 のビット数 3 は 1 のビット数 1 の
// 和よりも大きくなる。
//
class ModInvertedExpGolombCoder : public ModInvertedCoder
{
	friend class ModInvertedCoder;
public:
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

private:
	static ModSize getPrefixBitLength(ModSize);

	// .cpp ファイルは作らないので、ModInvertedCoder.cpp で定義する
	static const ModUnicodeChar coderName[];
};

//
// FUNCTION
// ModInvertedExpGolombCoder::getPrefixBitLength --- プレフィックス長の計算
//
// NOTES
// value のプレフィックスのビット長を計算する。
//
// ARGUMENTS
// ModSize value
//		計算対象
//
// RETURN
// プレフィックス長
//
// EXCEPTIONS
// なし
// 
inline ModSize
ModInvertedExpGolombCoder::getPrefixBitLength(ModSize value)
{
	ModSize prefixLength(0);
	while (value >>= 1) {
		++prefixLength;
	}
	return prefixLength;
}


//
// FUNCTION
// ModInvertedExpGolombCoder::getBitLength --- ビット長の計算
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
ModInvertedExpGolombCoder::getBitLength(const ModSize value) const
{
	return (getPrefixBitLength(value)<<1) + 1;
}


//
// FUNCTION
// ModInvertedExpGolombCoder::append --- 値を前向きにセット
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
ModInvertedExpGolombCoder::append(const ModSize value,
								  Unit* dataBegin,
								  ModSize& bitOffset) const
{
	; ModAssert(value > 0);

	ModSize prefixBitLength(getPrefixBitLength(value));

	if (prefixBitLength == 0) {
		// separator だけを書き、位置を進める
		set(dataBegin, bitOffset, 1, 1);
		bitOffset += 1;
	} else {
		// まず prefix, separator を書き、位置を進める
		set(dataBegin, bitOffset, prefixBitLength + 1, 1);
		bitOffset += prefixBitLength + 1;
		// 次に value を書き、位置を進める
		set(dataBegin, bitOffset, prefixBitLength,
			value - (1<<prefixBitLength));
		bitOffset += prefixBitLength;
	}
}

inline ModBoolean
ModInvertedExpGolombCoder::append(const ModSize value,
								  Unit* dataBegin,
								  const ModSize dataLength,
								  ModSize& bitOffset) const
{
	; ModAssert(value > 0);

	ModSize prefixBitLength(getPrefixBitLength(value));
	if ((bitOffset + (prefixBitLength<<1) + 1) > dataLength) {
		// データは収まらない
		return ModFalse;
	}

	if (prefixBitLength == 0) {
		// separator だけを書き、位置を進める
		set(dataBegin, bitOffset, 1, 1);
		bitOffset += 1;
	} else {
		// まず prefix, separator を書き、位置を進める
		set(dataBegin, bitOffset, prefixBitLength + 1, 1);
		bitOffset += prefixBitLength + 1;
		// 次に value を書き、位置を進める
		set(dataBegin, bitOffset, prefixBitLength,
			value - (1<<prefixBitLength));
		bitOffset += prefixBitLength;
	}

	return ModTrue;
}


//
// FUNCTION
// ModInvertedExpGolombCoder::appendBack --- 値を後向きにセット
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
ModInvertedExpGolombCoder::appendBack(const ModSize value,
									  Unit* dataTail,
									  ModSize& bitOffset) const
{
	; ModAssert(value > 0);

	ModSize prefixBitLength(getPrefixBitLength(value));

	if (prefixBitLength == 0) {
		// separator だけを書き、位置を進める
		setBack(dataTail, bitOffset, 1, 1);
		bitOffset += 1;
	} else {
		// まず prefix, separator を書き、位置を進める
		// prefix は LSB から書くので、append とは異なり separator をシフトする
		setBack(dataTail, bitOffset, prefixBitLength + 1, 1<<prefixBitLength);
		bitOffset += prefixBitLength + 1;
		// 次に value を書き、位置を進める
		setBack(dataTail, bitOffset, prefixBitLength,
				value - (1<<prefixBitLength));
		bitOffset += prefixBitLength;
	}
}

/* purecov: begin deadcode */
//		appendBack() は shortList でのみ使用されるが、shortList では圧縮データ
//		が領域に収まるかを事前にチェックしておくので、圧縮しながら領域に収まる
//		かを調べるタイプの appendBack() はどこからも使われない
inline ModBoolean
ModInvertedExpGolombCoder::appendBack(const ModSize value,
									  Unit* dataTail,
									  const ModSize dataLength,
									  ModSize& bitOffset) const
{
	; ModAssert(value > 0);

	ModSize prefixBitLength(getPrefixBitLength(value));
	if ((bitOffset + (prefixBitLength<<1) + 1) > dataLength) {
		// データは収まらない
		return ModFalse;
	}

	if (prefixBitLength == 0) {
		// separator だけを書き、位置を進める
		setBack(dataTail, bitOffset, 1, 1);
		bitOffset += 1;
	} else {
		// まず prefix, separator を書き、位置を進める
		// prefix は LSB から書くので、append とは異なり separator をシフトする
		setBack(dataTail, bitOffset, prefixBitLength + 1, 1<<prefixBitLength);
		bitOffset += prefixBitLength + 1;
		// 次に value を書き、位置を進める
		setBack(dataTail, bitOffset, prefixBitLength,
				value - (1<<prefixBitLength));
		bitOffset += prefixBitLength;
	}

	return ModTrue;
}
/* purecov: end */


//
// FUNCTION
// ModInvertedExpGolombCoder::get --- 値を前向きに取得
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
ModInvertedExpGolombCoder::get(ModSize& value,
							   const Unit* dataBegin,
							   const ModSize bitLength,
							   ModSize& bitOffset) const
{
	if (bitOffset >= bitLength) {
		// ここは事前のチェックなので等号が必要
		return ModFalse;
	}

	// prefix 長をはかる
	dataBegin += bitOffset>>5;
	ModSize bitMask(0x80000000>>(bitOffset&31));
	Unit tmp(*dataBegin);

	if ((tmp&bitMask) != 0) {
		// prefix がない場合 -- 1 に決まっている！
		++bitOffset;
		value = 1;
		return ModTrue;
	}

	ModSize prefixBitLength(0);
	do {
		++prefixBitLength;
		if (bitMask == 1) {
			if ((bitOffset + prefixBitLength) >= bitLength) {
				return ModFalse;
			}
			tmp = *++dataBegin;
			bitMask = 0x80000000;
		} else {
			bitMask >>= 1;
		}
	} while ((tmp&bitMask) == 0);

	// tail 値を調べる
	bitOffset += prefixBitLength + 1;	// 現在位置を tail の先頭にする
	if (bitOffset + prefixBitLength > bitLength) {
		return ModFalse;
	}

	bitMask = 1<<prefixBitLength;		// bitMask はもう使わない
	ModSize shift((bitOffset&31) + prefixBitLength);
	if (shift <= 32) {
		// １つのユニットに収まる場合
		if (shift == prefixBitLength) { // (bitOffset&31) == 0) {
			// tail が新しいユニットの先頭になる場合
			++dataBegin;
			bitOffset += prefixBitLength;
			value = ((*dataBegin)>>(32 - prefixBitLength)) + bitMask;
		} else {
			bitOffset += prefixBitLength;
			value = ((bitMask - 1)&(tmp>>(32 - (bitOffset&31)))) + bitMask;
		}
	} else {
		// ２つのユニットに跨る場合
		value = (((ModUInt32Max>>(bitOffset&31))&tmp)<<(shift - 32)) + bitMask;
		bitOffset += prefixBitLength;
		++dataBegin;
		value += (*dataBegin)>>(64 - shift);
	}

	return ModTrue;
}


//
// FUNCTION
// ModInvertedExpGolombCoder::getBack --- 値を後向きに取得
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
ModInvertedExpGolombCoder::getBack(ModSize& value,
								   const Unit* dataTail,
								   const ModSize bitLength,
								   ModSize& bitOffset) const
{
	if (bitOffset >= bitLength) {
		// ここは事前のチェックなので等号が必要
		return ModFalse;
	}

	// prefix 長をはかる
	dataTail -= bitOffset>>5;
	ModSize bitMask(1<<(bitOffset&31));
	Unit tmp(*dataTail);

	if ((tmp&bitMask) != 0) {
		// prefix がない場合 -- 1 に決まっている！
		++bitOffset;
		value = 1;
		return ModTrue;
	}

	ModSize prefixBitLength(0);
	do {
		++prefixBitLength;
		if (bitMask == 0x80000000) {
			if ((bitOffset + prefixBitLength) >= bitLength) {
				return ModFalse;
			}
			tmp = *--dataTail;
			bitMask = 1;
		} else {
			bitMask <<= 1;
		}
	} while ((tmp&bitMask) == 0);

	// tail 値を調べる
	bitOffset += prefixBitLength + 1;	// 現在位置を tail の先頭にする
	if (bitOffset + prefixBitLength > bitLength) {
		return ModFalse;
	}

	bitMask = 1<<prefixBitLength;		// bitMask はもう使わない
	if ((bitOffset&31) + prefixBitLength <= 32) {
		// １つのユニットに収まる場合
		if ((bitOffset&31) == 0) {
			// tail が新しいユニットの先頭になる場合
			--dataTail;
			value = ((bitMask - 1)&(*dataTail)) + bitMask;
		} else {
			value = ((bitMask - 1)&(tmp>>(bitOffset&31))) + bitMask;
		}
	} else {
		// ２つのユニットに跨る場合
		value = (tmp>>(bitOffset&31)) + bitMask;
		--dataTail;
		value += (bitMask - 1)&((*dataTail)<<(32 - (bitOffset&31)));
	}
	bitOffset += prefixBitLength;

	return ModTrue;
}

#endif	// __ModInvertedExpGolombCoder_H__

//
// Copyright (c) 1997, 1998, 1999, 2000, 2002, 2004, 2010, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
