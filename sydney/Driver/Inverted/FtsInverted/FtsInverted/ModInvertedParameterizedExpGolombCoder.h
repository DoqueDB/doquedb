// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedParameterizedExpGolombCoder.h -- Exponential Golomb 法による符号器
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedParameterizedExpGolombCoder_H__
#define __ModInvertedParameterizedExpGolombCoder_H__

#include "ModInvertedTypes.h"
#include "ModInvertedCoder.h"

class ModCharString;

class ModInvertedCoderParameter;

//
// CLASS
// ModInvertedParameterizedExpGolombCoder -- パラメータ付 ExpGolomb 法による符号器
//
// NOTES
// 転置リストデータの圧縮伸長を行なう符号器のパラメータ付 ExpGolomb 法による
// 実装。パラメータλ = 0 の場合は ModInvertedExpGolombCoder と等価になる。
// パラメータ付 ExpGolomb 法は以下の文書を参考にした。
//
// * J. Teuhola, "A compression method for clustered bit-vectors", Information
//	 Processing Letters, 7(6), 1978
//
// ただし、あるブロックに圧縮されたデータがないことを表すために 1 を詰める
// 手間を省くため、prefix と separator の 0, 1 をオリジナルとは逆にした。
// 以下に、パラメータを変えた場合の符合の例（ただし前向きのみ）を示す。
//
//		value	λ=0			λ=1			λ=2
//		1		1				1,0				1,00
//		2		0,1,0			1,1				1,01
//		3		0,1,1			0,1,00			1,10
//		4		00,1,00			0,1,01			1,11
//		5		00,1,01			0,1,10			0,1,000
//		6		00,1,10			0,1,11			0,1,001
//		7		00,1,11			00,1,000		0,1,010
//		8		000,1,000		00,1,001		0,1,011
//		9		000,1,001		00,1,010		0,1,100
//		10		000,1,010		00,1,011		0,1,101
//
// この方式では、lambda=0 の場合、x = y + z のとき、x のビット
// 数が y のビット数と z のビット数の和より大きくなることがあるので、
// 文書IDの圧縮に使用することができない。
// 例えば、lambda=0 では、2 = 1 + 1 において、2 のビット数 3 は 
// 1 のビット数 1 の和よりも大きくなる。
//
class
ModInvertedParameterizedExpGolombCoder : public ModInvertedCoder
{
	friend class ModInvertedCoder;
	friend class ModInvertedCoderParameter;
public:

	ModInvertedParameterizedExpGolombCoder(const int = 0);
	ModInvertedParameterizedExpGolombCoder(const ModCharString&);

#ifdef CODER_HAS_CURRENT_LOCATION
	ModInvertedParameterizedExpGolombCoder(const ModInvertedParameterizedExpGolombCoder&);
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

	void setLambda(const int);
	int getLambda() const { return lambda; }

	virtual void getDescription(ModCharString& description_) const;

private:
	ModSize getPrefixBitLength(ModSize) const;

	ModSize lambda;					// λ
	ModSize lambda1;				// λ + 1
	ModSize lambda2;				// 2^λ - 1
	ModSize lambda3;				// 2^λ

	void parse(const ModCharString&);

	// .cpp ファイルは作らないので、ModInvertedCoder.cpp で定義する
	static const char coderName[];
};


//
// FUNCTION
// ModInvertedParameterizedExpGolombCoder::getPrefixBitLength --- プレフィックス長の計算
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
ModInvertedParameterizedExpGolombCoder::getPrefixBitLength(ModSize value) const
{
#if 1
	if (value <= lambda3) {
		return 0;
	}
	ModSize prefixLength(1);
	value += lambda2;
	value >>= 1;
	while ((value >>= 1) > lambda2) {
		++prefixLength;
	}
	return prefixLength;
#else
	ModSize prefixLength(0);
	value += lambda2;
	while ((value >>= 1) > lambda2) {
		++prefixLength;
	}
	return prefixLength;
#endif
}


//
// FUNCTION
// ModInvertedParameterizedExpGolombCoder::getBitLength --- ビット長の計算
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
ModInvertedParameterizedExpGolombCoder::getBitLength(const ModSize value) const
{
	return (getPrefixBitLength(value)<<1) + lambda1;
}


//
// FUNCTION
// ModInvertedParameterizedExpGolombCoder::append --- 値を前向きにセット
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
ModInvertedParameterizedExpGolombCoder::append(const ModSize value,
											   Unit* dataBegin,
											   ModSize& bitOffset) const
{
	ModSize prefixBitLength(getPrefixBitLength(value));

	ModInvertedCoder::set(dataBegin, bitOffset + prefixBitLength,
						  prefixBitLength + lambda1,
						  value + lambda2);
	bitOffset += (prefixBitLength<<1) + lambda1;
}

inline ModBoolean
ModInvertedParameterizedExpGolombCoder::append(const ModSize value,
											   Unit* dataBegin,
											   const ModSize dataLength,
											   ModSize& bitOffset) const
{
	ModSize prefixBitLength(getPrefixBitLength(value));
	ModSize totalBitLength((prefixBitLength<<1) + lambda1);
	if (bitOffset + totalBitLength > dataLength) {
		return ModFalse;
	}

	ModInvertedCoder::set(dataBegin, bitOffset + prefixBitLength,
						  prefixBitLength + lambda1,
						  value + lambda2);
	bitOffset += totalBitLength;

	return ModTrue;
}


//
// FUNCTION
// ModInvertedParameterizedExpGolombCoder::appendBack --- 値を後向きにセット
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
ModInvertedParameterizedExpGolombCoder::appendBack(const ModSize value,
												   Unit* dataBegin,
												   ModSize& bitOffset) const
{
	ModSize prefixBitLength(getPrefixBitLength(value));

	// 本当の suffixBitLength は prefixBitLength + lambda だが、以下の計算では
	// + 1 した値をしようするので lambda1 (= lambda + 1) を用いる
	ModSize suffixBitLength(prefixBitLength + lambda1);

	// 多くの場合 lambda != 0 なので else 以下は実施されず、場合分けは無駄
///	if (suffixBitLength > 1) {
		// suffix があるばあい
		ModInvertedCoder::setBack(dataBegin, bitOffset + prefixBitLength,
								  suffixBitLength,
								  1 + ((value + lambda2)<<1)
								  - (1<<suffixBitLength));
		bitOffset += prefixBitLength + suffixBitLength;
///	} else {
///		// suffix がないばあい
///		ModInvertedCoder::setBack(dataBegin, bitOffset + prefixBitLength,
///								  1, 1);
///		// ModInvertedCoder::setBack(dataBegin, bitOffset,
///		// 						  prefixBitLength + 1, 1<<prefixBitLength);
///		bitOffset += prefixBitLength + 1;
///	}
}

/* purecov: begin deadcode */
//		appendBack() は shortList でのみ使用されるが、shortList では圧縮データ
//		が領域に収まるかを事前にチェックしておくので、圧縮しながら領域に収まる
//		かを調べるタイプの appendBack() はどこからも使われない
inline ModBoolean
ModInvertedParameterizedExpGolombCoder::appendBack(const ModSize value,
												   Unit* dataBegin,
												   const ModSize dataLength,
												   ModSize& bitOffset) const
{
	ModSize prefixBitLength(getPrefixBitLength(value));
	if ((bitOffset + (prefixBitLength<<1) + lambda) > dataLength) {
		return ModFalse;
	}

	ModSize suffixBitLength(prefixBitLength + lambda1);

	// 多くの場合 lambda != 0 なので else 以下は実施されず、場合分けは無駄
///	if (suffixBitLength > 1) {
		// suffix があるばあい
		ModInvertedCoder::setBack(dataBegin, bitOffset + prefixBitLength,
								  suffixBitLength,
								  1 + ((value + lambda2)<<1)
								  - (1<<suffixBitLength));
		bitOffset += prefixBitLength + suffixBitLength;
///	} else {
///		// suffix がないばあい
///		ModInvertedCoder::setBack(dataBegin, bitOffset + prefixBitLength,
///								  1, 1);
///		// ModInvertedCoder::setBack(dataBegin, bitOffset,
///		// 						  prefixBitLength + 1, 1<<prefixBitLength);
///		bitOffset += prefixBitLength + 1;
///	}

	return ModTrue;
}
/* purecov: end */


#if 1 // defined(OS_SOLARIS2_5) || defined(OS_RHLINUX6_0)
//
// FUNCTION
// ModInvertedParameterizedExpGolombCoder::get --- 値を前向きに取得
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
ModInvertedParameterizedExpGolombCoder::get(ModSize& value,
											const Unit* dataBegin_,
											const ModSize bitLength_,
											ModSize& bitOffset_) const
{
	if (bitOffset_ >= bitLength_) {
		// ここは事前のチェックなので等号が必要
		return ModFalse;
	}
	// prefix 長をはかる
	dataBegin_ += bitOffset_>>5;
	ModSize shift(bitOffset_&31);
	ModSize bitMask(0x80000000>>shift);
	Unit tmp(*dataBegin_);

	if ( tmp&bitMask ) {
		// prefix がない場合
		++bitOffset_;			// skip separator
		shift += lambda1;
		if (shift <= 32 ) {
			bitOffset_ += lambda;
			value = lambda2&(tmp>>(32 - (bitOffset_& 31)));
		} else {
			++dataBegin_;
			if ((bitOffset_&31) == 0)
				value = ((*dataBegin_)>>(32 - lambda));
			else
			{
				// ２つのユニットに跨る場合
				value = (((ModUInt32Max>>((bitOffset_ &31)))&tmp)<< (shift - 32));
				value += (*dataBegin_)>>(64 - shift);
			}
			bitOffset_ += lambda;
		}
		++value;
#ifdef DEBUG
		++*((int*)(&count));
#endif
		return ModTrue;
	}
	ModSize prefixBitLength(0);
	do {
		++prefixBitLength;
		if (bitMask & 0x01)
		{
			if ((bitOffset_ + prefixBitLength) >= bitLength_)
				// これを外に出す方が遅くなる
				return ModFalse;
			++dataBegin_;
			tmp = *dataBegin_;
			bitMask = 0x80000000;
		}
		else
			bitMask >>= 1;
	} while ((tmp&bitMask) == 0);

	// tail 値を調べる
	bitOffset_ += prefixBitLength + 1;	// 現在位置を tail の先頭にする
	prefixBitLength += lambda;
	bitMask = 1<<prefixBitLength;		// bitMask はもう使わない
	value = bitMask - lambda2;
	shift = (bitOffset_&31) + prefixBitLength;
	if (shift <= 32) {
		// １つのユニットに収まる場合
		bitOffset_ += prefixBitLength;
		if (shift == prefixBitLength)
		{
			// tail が新しいユニットの先頭になる場合
			++dataBegin_;
			value += (*dataBegin_)>>(32 - prefixBitLength);
		} else
			value += (bitMask - 1)&(tmp>>(32 - (bitOffset_&31)));
	}
	else
	{
		// ２つのユニットに跨る場合
		++dataBegin_;
		value += ((ModUInt32Max>>(bitOffset_&31))&tmp)<<(shift - 32);
		value += (*dataBegin_)>>(64 - shift);
		bitOffset_ += prefixBitLength;
	}
#ifdef DEBUG
	++*((int*)(&count));
#endif
	return ModTrue;
}
#else
//
// FUNCTION
// ModInvertedParameterizedExpGolombCoder::get --- 値を前向きに取得
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
ModInvertedParameterizedExpGolombCoder::get(ModSize& value,
										const Unit* dataBegin_,
										const ModSize bitLength_,
										ModSize& bitOffset_) const
{
	if(bitOffset_ < bitLength_)
	{
		dataBegin_ += bitOffset_>>5;
		ModSize shift(bitOffset_&31);

#if defined(OS_SOLARIS2_5) || defined(OS_RHLINUX6_0)
		// shift to dword alignment
		ModUInt64 data ;					
		((ModStructuredInt64*)&data)->halfs.high = *dataBegin_;
		if(bitOffset_/32 + 1 < (bitLength_+ 31)/32)
			((ModStructuredInt64*)&data)->halfs.low = *( dataBegin_ + 1);
		data <<= shift;	
		value = ((ModStructuredInt64*)&data)->halfs.high;
#endif
#if defined(OS_WINDOWSNT4_0) || defined(OS_WINDOWS98)
		if(bitOffset_/32 + 1 >= (bitLength_+ 31)/32)
			_asm
			{
				mov	 edx,dataBegin_
				mov  edx,[edx]			// ((ModStructuredInt64*)&data)->halfs.high = *dataBegin_;
				mov  ecx,shift
				shl edx,cl			// data <<= shift;	
			}
		else
			__asm
			{							// ModInt64 data ;					
				mov	 eax,dataBegin_ 	// ((ModStructuredInt64*)&data)->halfs.low = *( dataBegin_ + 1);
				mov  edx,[eax]			// ((ModStructuredInt64*)&data)->halfs.high = *dataBegin_;
				mov  eax,[eax + 4]			
				mov  ecx,shift
				shld edx,eax,cl			// data <<= shift;	
			}
		_asm
		{
				mov  eax,value
				mov	[eax],edx			// value = ((ModStructuredInt64*)&data)->halfs.high;
		}
#endif
		ModSize prefixBitLength(0);
		while((value&0x80000000) == 0)
		{
			++prefixBitLength;
			if(bitOffset_ + prefixBitLength >= bitLength_)
				return ModFalse;
			value <<= 1;
		}
		value <<= 1;
		if(prefixBitLength + lambda)
			value >>= 32 - (prefixBitLength + lambda);
		else
			value = 0;
		// calculate values
		value += (1 << (prefixBitLength + lambda))  - lambda2;
		bitOffset_	+= 2*prefixBitLength + lambda1;
#ifdef DEBUG
	++*((int*)(&count));
#endif
		return ModTrue;
	}
	else
		return ModFalse;
}
#endif
//
// FUNCTION
// ModInvertedParameterizedExpGolombCoder::getBack --- 値を後向きに取得
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
ModInvertedParameterizedExpGolombCoder::getBack(ModSize& value,
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
	ModSize shift(bitOffset&31);
	ModSize bitMask(1<<shift);
	Unit tmp(*dataTail);

	if ((tmp&bitMask) != 0) {
		// prefix がない場合
		++bitOffset;
		if (bitOffset + lambda > bitLength) {
			return ModFalse;
		}
///		shift = (bitOffset_&31) + lambda;
		shift += lambda1;
		if (shift <= 32) {
			value = (lambda2&(tmp>>(bitOffset&31))) + 1; // 1<<lambda - lambda2
		} else if ((bitOffset&31) == 0) {
			--dataTail;
			value = (lambda2&(*dataTail)) + 1;
		} else {
			// ２つのユニットに跨る場合
			value = (tmp>>(bitOffset&31)) + 1;
			--dataTail;
			value += lambda2&((*dataTail)<<(32 - (bitOffset&31)));
		}
		bitOffset += lambda;
#ifdef DEBUG
		++*((int*)(&count));
#endif
		return ModTrue;
	}

	ModSize prefixBitLength(0);
	do {
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
	} while ((tmp&bitMask) == 0);

	// tail 値を調べる
	bitOffset += prefixBitLength + 1;	// 現在位置を tail の先頭にする
	prefixBitLength += lambda;
	if (bitOffset + prefixBitLength > bitLength) {
		return ModFalse;
	}
	bitMask = 1<<prefixBitLength;		// bitMask はもう使わない
	shift = (bitOffset&31) + prefixBitLength;
	if ((bitOffset&31) + prefixBitLength <= 32) {
		// １つのユニットに収まる場合
		if ((bitOffset&31) == 0) {
			// tail が新しいユニットの先頭になる場合
			--dataTail;
			value = ((bitMask - 1)&(*dataTail)) + bitMask - lambda2;
		} else {
			value = ((bitMask - 1)&(tmp>>(bitOffset&31))) + bitMask - lambda2;
		}
	} else {
		// ２つのユニットに跨る場合
		value = (tmp>>(bitOffset&31)) + bitMask - lambda2;
		--dataTail;
		value += (bitMask - 1)&((*dataTail)<<(32 - (bitOffset&31)));
	}
	bitOffset += prefixBitLength;

#ifdef DEBUG
	++*((int*)(&count));
#endif
	return ModTrue;
}
//
// 以下は、符号器が現在位置などを保持するように改めた場合のためのコード
//
#if 0
inline void
ModInvertedParameterizedExpGolombCoder::set(Unit* dataBegin_,
											const ModSize bitLength_,
											const ModSize bitOffset_)
{
	bitLength = bitLength_;
	bitOffset = bitOffset_;
	dataUnit = dataBegin_ + (bitOffset>>5);
	dataValue = *dataUnit;
}

inline ModBoolean
ModInvertedParameterizedExpGolombCoder::get(ModSize& value)
{
	if (currentOffset + lambda1 >= currentLength) {
		// ここは事前のチェックなので等号が必要
		return ModFalse;
	}
#ifdef DEBUG
	++*((int*)(&count));
#endif

	// prefix 長をはかる
	ModSize shift(currentOffset&31);
	ModSize bitMask(0x80000000>>shift);

	if (shift == 0 && currentOffset != 0) {
		dataValue = *++dataUnit;
	}

	if ((dataValue&bitMask) != 0) {
		++currentOffset;
		if (currentOffset + lambda > currentLength) {
			return ModFalse;
		}
///		shift = (bitOffset&31) + lambda;
		shift += lambda1;
		if (shift <= 32) {
			currentOffset += lambda;
			value = (lambda2&(dataValue>>(32 - (currentOffset&31)))) + 1;
		} else if ((currentOffset&31) == 0) {
			dataValue = *++dataUnit;
			currentOffset += lambda;
			value = (lambda2&(dataValue>>(32 - (currentOffset&31)))) + 1;
		} else {
			// ２つのユニットに跨る場合
			value = (((ModUInt32Max>>(currentOffset&31))&dataValue)<<(shift - 32)) + 1;
			currentOffset += lambda;
			dataValue = *++dataUnit;
			shift = 64 - shift;
			value += (((ModUInt32Max<<shift)&dataValue)>>shift);
		}
		return ModTrue;
	}

	ModSize prefixBitLength(0);
	do {
		++prefixBitLength;
		if (bitMask == 1) {
			if ((currentOffset + prefixBitLength) >= currentLength) {
				// これを外に出す方が遅くなる
				return ModFalse;
			}
			dataValue = *++dataUnit;
			bitMask = 0x80000000;
		} else {
			bitMask >>= 1;
		}
	} while ((dataValue&bitMask) == 0);

	// tail 値を調べる
	currentOffset += prefixBitLength + 1;	// 現在位置を tail の先頭にする
	prefixBitLength += lambda;				// 実際には suffixBitLength 
	if (currentOffset + prefixBitLength > currentLength) {
		return ModFalse;
	}
	bitMask = (1<<prefixBitLength);		// bitMask はもういらない
	shift = (currentOffset&31) + prefixBitLength;
	if (shift <= 32) {
		// １つのユニットに収まる場合
		if (shift == prefixBitLength) {	// (bitOffset&31) == 0) {
			// tail が新しいユニットの先頭になる場合
			dataValue = *++dataUnit;
		}
		currentOffset += prefixBitLength;
		value = ((bitMask - 1)&(dataValue>>(32 - (currentOffset&31))))
			+ bitMask - lambda2;
	} else {
		// ２つのユニットに跨る場合
		value = (((ModUInt32Max>>(currentOffset&31))&dataValue)<<(shift - 32))
			+ bitMask - lambda2;
		currentOffset += prefixBitLength;
		dataValue = *++dataUnit;
		shift = 64 - shift;
		value += (((ModUInt32Max<<shift)&dataValue)>>shift);
	}

	return ModTrue;
}
#endif

#endif	// __ModInvertedParameterizedExpGolombCoder_H__

//
// Copyright (c) 1997, 1998, 1999, 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
