// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedExtendedGolombCoder.h -- 拡張 Golomb 法による符号器
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

#ifndef	__ModInvertedExtendedGolombCoder_H__
#define __ModInvertedExtendedGolombCoder_H__

#include "FullText2/Module.h"

#include "ModInvertedCoder.h"

//
// CLASS
// ModInvertedExtendedGolombCoder -- 拡張 Golomb 法による符号器
//
// NOTES
// 転置リストデータの圧縮伸長を行なう符号器の拡張 ExpGolomb 法による実装。
//
// Parameterlized Exponential Golomb 符合を、prefix の 1 ビットに対し 
// suffix を factor ビット増加させるように拡張したものである。
// Factor を調整することで大きな値をさらに効率的に圧縮することができる。
// なお、factor=1 は Parameterlized Exponential Golomb 符合と一致する。
//
//		value	λ=0, f=1	λ=0, f=2	λ=1, f=2	λ=2, f=2
//		1		1			1			1,0			1,00
//		2		0,1,0		0,1,00		1,1			1,01
//		3		0,1,1		0,1,01		0,1,000		1,10
//		4		00,1,00		0,1,10		0,1,001		1,11
//		5		00,1,01		0,1,11		0,1,010		0,1,0000
//		6		00,1,10		00,1,0000	0,1,011		0,1,0001
//		7		00,1,11		00,1,0001	0,1,100		0,1,0010
//		8		000,1,000	00,1,0010	0,1,101		0,1,0011
//		9		000,1,001	00,1,0011	0,1,110		0,1,0100
//		10		000,1,010	00,1,0100	0,1,111		0,1,0101
//
// この方式では、lambda < factor の場合、x = y + z のとき、x のビット
// 数が y のビット数と z のビット数の和より大きくなることがあるので、
// 文書IDの圧縮に使用することができない。
// 例えば、lambda=0, factor=1 では、2 = 1 + 1 において、2 のビット数
// 3 は 1 のビット数 1 の和よりも大きくなる。
//
class ModInvertedExtendedGolombCoder : public ModInvertedCoder
{
	friend class ModInvertedCoder;
public:

	ModInvertedExtendedGolombCoder(const int = 0, const int = 1);
	ModInvertedExtendedGolombCoder(const ModUnicodeString&);

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
	ModSize getPrefixBitLength(ModSize) const;

	void setLambda(const int);
	void setFactor(const int);
	void setValues();

	int lambda;				// λ
	int lambda1;			// λ + 1
	int	lambda2;			// 2^λ - 1
	int factor;				// 1 以上であること！
	int factor1;
	int factor2;
	int values[32];

	void parse(const ModUnicodeString&);

	// .cpp ファイルは作らないので、ModInvertedCoder.cpp で定義する
	static const ModUnicodeChar coderName[];
};


//
// FUNCTION
// ModInvertedExtendedGolombCoder::ModInvertedExtendedGolombCoder -- コンストラクタ
//
// NOTES
// コンストラクタ。
// パラメータを引数にとるもの、引数オブジェクトを引数にとるもの、
// コピーコンストラクタの３種類がある。
//
// ARGUMENTS
// const int lambda_
//		パラメータ
// const int factor_
//		パラメータ
// const Argument* argument_
//		引数オブジェクト
// const ModInvertedExtendedGolombCoder& orig_
//		コピー元
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位のモジュールからの例外をそのまま返す
// 
inline
ModInvertedExtendedGolombCoder::ModInvertedExtendedGolombCoder(
	const int lambda_, const int factor_)
	: lambda(0), lambda1(1), lambda2(1), factor(1), factor1(2), factor2(2)
{
	setLambda(lambda_);
	setFactor(factor_);
	setValues();
}

inline
ModInvertedExtendedGolombCoder::ModInvertedExtendedGolombCoder(
	const ModUnicodeString& description_)
	: lambda(0), lambda1(1), lambda2(1), factor(1), factor1(2), factor2(2)
{
	parse(description_);		// setValues() も実行される
}

//
// FUNCTION
// ModInvertedExtendedGolombCoder::getPrefixBitLength --- プレフィックス長の計算
//
// NOTES
// value のプレフィックスのビット長を計算する。
// ModInvertedExpGolombCoder で異なり、パラメータがあるので static 関数には
// できない。
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
ModInvertedExtendedGolombCoder::getPrefixBitLength(ModSize value) const
{
	ModSize tmp(lambda2);
	ModSize prefixLength(0);
	while (value > tmp) {
		++prefixLength;
		tmp += lambda2<<(factor*prefixLength);
	}
	return prefixLength;
}


//
// FUNCTION
// ModInvertedExtendedGolombCoder::getBitLength --- ビット長の計算
//
// NOTES
// value が占めるビット長を返す。
// 
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
ModInvertedExtendedGolombCoder::getBitLength(const ModSize value) const
{
	return getPrefixBitLength(value)*factor1 + lambda1;
}


//
// FUNCTION
// ModInvertedExtendedGolombCoder::append --- 値を前向きにセット
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
ModInvertedExtendedGolombCoder::append(const ModSize value,
									   Unit* dataBegin,
									   ModSize& bitOffset) const
{
	ModSize prefixBitLength(getPrefixBitLength(value));

	// まず prefix, separator を書き、位置を進める
	set(dataBegin, bitOffset, prefixBitLength + 1, 1);
	bitOffset += prefixBitLength + 1;

	// 次に必要ならば value を書き、位置を進める
	if (prefixBitLength + lambda > 0) {
		set(dataBegin, bitOffset, prefixBitLength*factor + lambda,
			value -
			lambda2*((1<<(prefixBitLength*factor)) - 1)/(factor2 - 1) - 1);
		bitOffset += prefixBitLength*factor + lambda;
	}
}

inline ModBoolean
ModInvertedExtendedGolombCoder::append(const ModSize value,
									   Unit* dataBegin,
									   const ModSize dataLength,
									   ModSize& bitOffset) const
{
	ModSize prefixBitLength(getPrefixBitLength(value));
	if ((bitOffset + prefixBitLength*factor1 + lambda1) > dataLength) {
		return ModFalse;
	}

	// まず prefix, separator を書き、位置を進める
	set(dataBegin, bitOffset, prefixBitLength + 1, 1);
	bitOffset += prefixBitLength + 1;

	// 次に必要ならば value を書き、位置を進める
	if (prefixBitLength + lambda > 0) {
		set(dataBegin, bitOffset, prefixBitLength*factor + lambda,
			value
			- lambda2*((1<<(prefixBitLength*factor)) - 1)/(factor2 - 1) - 1);
		bitOffset += prefixBitLength*factor + lambda;
	}

	return ModTrue;
}


//
// FUNCTION
// ModInvertedExtendedGolombCoder::appendBack --- 値を後向きにセット
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
ModInvertedExtendedGolombCoder::appendBack(const ModSize value,
										   Unit* dataBegin,
										   ModSize& bitOffset) const
{
	ModSize prefixBitLength(getPrefixBitLength(value));

	// まず prefix, separator を書き、位置を進める
	// prefix は LSB から書くので、append とは異なり separator をシフトする
	setBack(dataBegin, bitOffset, prefixBitLength + 1, 1<<prefixBitLength);
	bitOffset += prefixBitLength + 1;

	// 次に必要ならば value を書き、位置を進める
	if (prefixBitLength + lambda > 0) {
		setBack(dataBegin, bitOffset, prefixBitLength*factor + lambda,
				value -
				lambda2*((1<<(prefixBitLength*factor)) - 1)/(factor2 - 1) - 1);
		bitOffset += prefixBitLength*factor + lambda;
	}
}

/* purecov: begin deadcode */
//		appendBack() は shortList でのみ使用されるが、shortList では圧縮データ
//		が領域に収まるかを事前にチェックしておくので、圧縮しながら領域に収まる
//		かを調べるタイプの appendBack() はどこからも使われない
inline ModBoolean
ModInvertedExtendedGolombCoder::appendBack(const ModSize value,
										   Unit* dataBegin,
										   const ModSize dataLength,
										   ModSize& bitOffset) const
{
	ModSize prefixBitLength(getPrefixBitLength(value));
	if ((bitOffset + prefixBitLength*factor1 + lambda1) > dataLength) {
		return ModFalse;
	}

	// まず prefix, separator を書き、位置を進める
	// prefix は LSB から書くので、append とは異なり separator をシフトする
	setBack(dataBegin, bitOffset, prefixBitLength + 1, 1<<prefixBitLength);
	bitOffset += prefixBitLength + 1;

	// 次に必要ならば value を書き、位置を進める
	if (prefixBitLength + lambda > 0) {
		setBack(dataBegin, bitOffset, prefixBitLength*factor + lambda,
				value -
				lambda2*((1<<(prefixBitLength*factor)) - 1)/(factor2 - 1) - 1);
		bitOffset += prefixBitLength*factor + lambda;
	}

	return ModTrue;
}
/* purecov: end */


//
// FUNCTION
// ModInvertedExtendedGolombCoder::get --- 値を前向きに取得
//
// NOTES
// データ領域 dataBegin の bitOffset から前向きに圧縮されている値を 
// value にセットする。
// 新しい位置は bitOffset にセットし直される。
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
ModInvertedExtendedGolombCoder::get(ModSize& value,
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
	ModSize prefixBitLength(0), shift;
	ModSize bitMask(1<<(31 - bitOffset&31));
	Unit tmp(*dataBegin);

	while ((tmp&bitMask) == 0) {
		++prefixBitLength;
		if (bitMask == 1) {
			if ((bitOffset + prefixBitLength) >= bitLength) {
				return ModFalse;
			}
			++dataBegin;
			tmp = *dataBegin;
			bitMask = 0x80000000;
		} else {
			bitMask >>= 1;
		}
	}

	// tail 値を調べる
	bitOffset += prefixBitLength + 1;	// 現在位置を tail の先頭にする
	prefixBitLength *= factor;
//	value = lambda2*((1<<prefixBitLength) - 1)/(factor2 - 1) + 1;
	value = values[prefixBitLength];
	prefixBitLength += lambda;
#if 0
	// prefix が確定できる場合には suffix も存在している！
	if (bitOffset + prefixBitLength > bitLength) {
		return ModFalse;
	}
#endif
	shift = (bitOffset&31) + prefixBitLength;
	if (shift <= 32) {
		// １つのユニットに収まる場合
		if ((bitOffset&31) == 0) {
			// tail が新しいユニットの先頭になる場合
			++dataBegin;
			tmp = *dataBegin;
		}
		bitOffset += prefixBitLength;
		value += ((1<<(prefixBitLength)) - 1)&(tmp>>(32 - (bitOffset&31)));
	} else {
		// ２つのユニットに跨る場合
		value += ((((1<<(32 - (bitOffset&31))) - 1)&tmp)<<(shift - 32));
		bitOffset += prefixBitLength;
		++dataBegin;
		value += (((1<<shift) - 1)&((*dataBegin)>>(64 - shift)));
	}

	return ModTrue;
}


//
// FUNCTION
// ModInvertedExtendedGolombCoder::getBack --- 値を後向きに取得
//
// NOTES
// データ領域 dataBegin の bitOffset から後向きに圧縮されている値を 
// value にセットする。
// 新しい位置は bitOffset にセットし直される。
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
ModInvertedExtendedGolombCoder::getBack(ModSize& value,
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
	ModSize prefixBitLength(0);
	ModSize bitMask(1<<(bitOffset&31));
	Unit tmp(*dataTail);
	while ((tmp&bitMask) == 0) {
		++prefixBitLength;
		if (bitMask == 0x80000000) {
			if ((bitOffset + prefixBitLength) >= bitLength) {
				return ModFalse;
			}
			--dataTail;
			tmp = *dataTail;
			bitMask = 1;
		} else {
			bitMask <<= 1;
		}
	}

	// tail 値を調べる
	bitOffset += prefixBitLength + 1;	// 現在位置を tail の先頭にする
	prefixBitLength *= factor;
	value = lambda2*((1<<prefixBitLength) - 1)/(factor2 - 1) + 1;
	prefixBitLength += lambda;
	if (bitOffset + prefixBitLength > bitLength) {
		return ModFalse;
	}

	if ((bitOffset&31) + prefixBitLength <= 32) {
		// １つのユニットに収まる場合
		if ((bitOffset&31) == 0) {
			// tail が新しいユニットの先頭になる場合
			--dataTail;
		}
		value += ((1<<(prefixBitLength)) - 1)&((*dataTail)>>(bitOffset&31));
		bitOffset += prefixBitLength;
	} else {
		// ２つのユニットに跨る場合
		value += ((((1<<(32 - (bitOffset&31))) - 1)&
				   ((*dataTail))>>(bitOffset&31)));
		--dataTail;
		value += (((1<<prefixBitLength) - 1)&
					  ((*dataTail)<<(32 - (bitOffset&31))));
		bitOffset += prefixBitLength;
	}

	return ModTrue;
}

#endif	// __ModInvertedExtendedGolombCoder_H__

//
// Copyright (c) 1997, 1998, 1999, 2000, 2002, 2004, 2010, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
