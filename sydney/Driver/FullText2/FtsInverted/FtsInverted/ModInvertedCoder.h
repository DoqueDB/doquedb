// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedCoder.h -- 転置ファイル用符号器
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2002, 2004, 2009, 2010, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedCoder_H__
#define __ModInvertedCoder_H__

#include "FullText2/Module.h"

#include "ModUnicodeString.h"
#include "ModTypes.h"

typedef ModUInt32 ModInvertedDataUnit;
const ModSize ModInvertedDataUnitBitSize = sizeof(ModInvertedDataUnit)*8;

//
// CLASS
// ModInvertedCoder -- 転置リスト符合器
//
// NOTES
// 転置リストデータの圧縮伸長を行なうインタフェースを規定する
//
class ModInvertedCoder
{
public:
	typedef ModInvertedDataUnit Unit;
	
	// パラメータにあった符合器を生成する
	static ModInvertedCoder* create(const ModUnicodeString&);

	ModInvertedCoder();
	virtual ~ModInvertedCoder() {}

	// 圧縮したビット長を得る
	virtual ModSize getBitLength(const ModSize) const = 0;
	// 圧縮してデータを格納する
	virtual void append(const ModSize, Unit*, ModSize&) const = 0;
	virtual ModBoolean append(const ModSize, Unit*, const ModSize,
							  ModSize&) const = 0;
	virtual void appendBack(const ModSize, Unit*, ModSize&) const = 0;
	virtual ModBoolean appendBack(const ModSize, Unit*, const ModSize,
								  ModSize&) const = 0;
	// 伸長してデータを取得する
	virtual ModBoolean get(ModSize&, const Unit*, const ModSize,
						   ModSize&) const = 0;
	virtual ModBoolean getBack(ModSize&, const Unit*, const ModSize,
							   ModSize&) const = 0;

	virtual ModBoolean find(const ModSize, const Unit*,
							const ModSize, const ModSize) const {
		; ModAssert(0); return ModFalse; }
	virtual ModBoolean findBack(const ModSize, const Unit*,
								const ModSize, const ModSize) const {
		; ModAssert(0); return ModFalse; }
	virtual ModBoolean searchable() const { return ModFalse; }

	// 配列データに対する処理
	ModSize	getBitLength(const ModSize*, const ModSize*) const;
	void append(ModSize, const ModSize*, const ModSize*,
				Unit*, ModSize&) const;
//	ModSize append(ModSize, const ModSize*, const ModSize*,
//				   Unit*, const ModSize, ModSize&) const;
//	ModSize append(ModSize&, ModInvertedLocationListIterator*,
//				   Unit*, const ModSize, ModSize&) const;
	void appendBack(ModSize, const ModSize*, const ModSize*,
					Unit*, ModSize&) const;
	ModSize appendBack(ModSize, const ModSize*, const ModSize*,
					   Unit*, const ModSize, ModSize&) const;

	ModSize get(ModSize, ModSize*, const ModSize*,
				const Unit*, const ModSize, ModSize&) const;
	ModSize getBack(ModSize, ModSize*, const ModSize*,
					const Unit*, const ModSize, ModSize&) const;

	// 値の設定に使用する関数群
	static void set(Unit*, ModSize, ModSize, ModFileOffset);
	static void setBack(Unit*, ModSize, ModSize, ModFileOffset);
	static void setOff(Unit*, const Unit*, ModSize, ModSize);
	static void setOff(Unit*, ModSize, ModSize);
	static void setOffBack(Unit*, ModSize, ModSize);
	static void move(Unit*, ModSize, ModSize, ModSize, Unit* = 0);
	static ModBoolean isZero(const Unit*, const Unit*, ModSize);

	// デバッグ用
	static void dump(const Unit*, const Unit*);

protected:
private:
};


//
// FUNCTION
// ModInvertedCoder::ModInvertedCoder -- コンストラクタ
//
// NOTES
// コンストラクタ
//
// ARGUMENTS
// const ModInvertedCoder& orig_
//		コピー元
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModInvertedCoder::ModInvertedCoder()
{}

//
// FUNCTION
// ModInvertedCoder::getBitLength -- 複数の値のビット長の計算
//
// NOTES
// valueBegin から valueEnd の値が占めるビット長を返す。
// 
// ARGUMENTS
// const ModSize* valueBegin
//		値の先頭
// const ModSize* valueEnd
//		値の末尾
//
// RETURN
// ビット長の合計
//
// EXCEPTIONS
// なし
// 
inline ModSize
ModInvertedCoder::getBitLength(const ModSize* valueBegin,
							   const ModSize* valueEnd) const
{
	ModSize length(0);
	ModSize oldValue(0);
	while (valueBegin < valueEnd) {
		length += getBitLength(*valueBegin - oldValue);
		oldValue = *valueBegin;
		++valueBegin;
	}
	return length;
}


//
// FUNCTION
// ModInvertedCoder::append -- 複数の値を前向きにセット
//
// NOTES
// valueBegin から valueEnd の値をデータ領域 dataBegin の bitOffset から圧縮
// して、前向きにセットする。詰め終った位置は bitOffset にセットし直される。
// データを詰めるだけのスペースがあるかのチェックは既に行なわれるものとして
// この関数の内部では行なわない 5 引数のものと、データを詰めるだけのスペースが
// あるかのチェックを行う 6 引数のものがある。
// 
// ARGUMENTS
// ModSize oldValue
//		前回の最終の値
// const ModSize* valueBegin
//		値の先頭
// const ModSize* valueEnd
//		値の末尾
// Unit* dataBegin
//		データ領域の先頭
// const ModSize dataLength
//		データ領域のビット長（6 引数の場合のみ）
// ModSize& bitOffset
//		ビット位置
//
// RETURN
// 5 引数の場合、なし
// 6 引数の場合、値をセットできた個数
//
// EXCEPTIONS
// なし
// 
inline void
ModInvertedCoder::append(ModSize oldValue,
						 const ModSize* valueBegin,
						 const ModSize* valueEnd,
						 Unit* dataBegin,
						 ModSize& bitOffset) const
{
	while (valueBegin < valueEnd) {
		append(*valueBegin - oldValue, dataBegin, bitOffset);
		oldValue = *valueBegin;
		++valueBegin;
	}
	return;
}
/*
inline ModSize
ModInvertedCoder::append(ModSize oldValue,
						 const ModSize* valueBegin,
						 const ModSize* valueEnd,
						 Unit* dataBegin,
						 const ModSize dataLength,
						 ModSize& bitOffset) const
{
	const ModSize* valueHead = valueBegin;
	while (valueBegin < valueEnd) {
		if (append(*valueBegin - oldValue, dataBegin, dataLength, bitOffset)
			== ModFalse) {
			break;
		}
		oldValue = *valueBegin;
		++valueBegin;
	}
	return valueBegin - valueHead;
}

inline ModSize
ModInvertedCoder::append(ModSize& oldValue,
						 ModInvertedLocationListIterator* iterator,
						 Unit* dataBegin,
						 const ModSize dataLength,
						 ModSize& bitOffset) const
{
	ModSize num(0);
	while (iterator->isEnd() == ModFalse) {
		if (append(iterator->getLocation() - oldValue,
				   dataBegin, dataLength, bitOffset)
			== ModFalse) {
			break;
		}
		oldValue = iterator->getLocation();
		iterator->next();
		++num;
	}
	return num;
}
*/

//
// FUNCTION
// ModInvertedCoder::appendBack -- 複数の値を後向きにセット
//
// NOTES
// valueBegin から valueEnd の値をデータ領域 dataBegin の bitOffset から圧縮
// して、後向きにセットする。詰め終った位置は bitOffset にセットし直される。
// データを詰めるだけのスペースがあるかのチェックは既に行なわれるものとして
// この関数の内部では行なわない 5 引数のものと、データを詰めるだけのスペースが
// あるかのチェックを行う 6 引数のものがある。
// 
// ARGUMENTS
// ModSize oldValue
//		前回の最終の値
// const ModSize* valueBegin
//		値の先頭
// const ModSize* valueEnd
//		値の末尾
// Unit* dataBegin
//		データ領域の先頭
// const ModSize dataLength
//		データ領域のビット長（6 引数の場合のみ）
// ModSize& bitOffset
//		ビット位置
//
// RETURN
// 5 引数の場合、なし
// 6 引数の場合、値をセットできた個数
//
// EXCEPTIONS
// なし
// 
inline void
ModInvertedCoder::appendBack(ModSize oldValue,
							 const ModSize* valueBegin,
							 const ModSize* valueEnd,
							 Unit* dataBegin,
							 ModSize& bitOffset) const
{
	while (valueBegin < valueEnd) {
		appendBack(*valueBegin - oldValue, dataBegin, bitOffset);
		oldValue = *valueBegin;
		++valueBegin;
	}
	return;
}

inline ModSize
ModInvertedCoder::appendBack(ModSize oldValue,
							 const ModSize* valueBegin,
							 const ModSize* valueEnd,
							 Unit* dataBegin,
							 const ModSize dataLength,
							 ModSize& bitOffset) const
{
	const ModSize* valueHead = valueBegin;
	while (valueBegin < valueEnd) {
		if (appendBack(*valueBegin - oldValue,
					   dataBegin, dataLength, bitOffset) == ModFalse) {
			break;
		}
		oldValue = *valueBegin;
		++valueBegin;
	}
	return static_cast<ModSize>(valueBegin - valueHead);
}


//
// FUNCTION
// ModInvertedCoder::get -- 複数の値を前向きに取得
//
// NOTES
// データ領域 dataBegin の bitOffset から前向きに圧縮されているデータを
// valueBegin から valueEnd にセットする。
// 新しい位置は bitOffset にセットし直される。
// 
// ARGUMENTS
// ModSize oldValue
//		前回の最終の値
// ModSize* valueBegin
//		値の先頭
// const ModSize* valueEnd
//		値の末尾
// const Unit* dataBegin
//		データ領域の先頭
// const ModSize dataLength
//		データ領域のビット長
// ModSize& bitOffset
//		ビット位置
//
// RETURN
// 取得した値の個数
//
// EXCEPTIONS
// なし
// 
inline ModSize
ModInvertedCoder::get(ModSize oldValue,
					  ModSize* valueBegin,
					  const ModSize* valueEnd, 
					  const Unit* dataBegin,
					  const ModSize bitLength,
					  ModSize& bitOffset) const
{
	ModSize* valueHead = valueBegin;
	ModSize tmpValue;
	while (valueBegin < valueEnd) {
		if (get(tmpValue, dataBegin, bitLength, bitOffset) == ModFalse) {
			break;
		}
		*valueBegin = tmpValue + oldValue;
		oldValue = *valueBegin;
		++valueBegin;
	}
	return static_cast<ModSize>(valueBegin - valueHead);
}


//
// FUNCTION
// ModInvertedCoder::get -- 複数の値を後向きに取得
//
// NOTES
// データ領域 dataBegin の bitOffset から後向きに圧縮されているデータを
// valueBegin から valueEnd にセットする。
// 新しい位置は bitOffset にセットし直される。
// 
// ARGUMENTS
// ModSize oldValue
//		前回の最終の値
// ModSize* valueBegin
//		値の先頭
// const ModSize* valueEnd
//		値の末尾
// const Unit* dataBegin
//		データ領域の先頭
// const ModSize dataLength
//		データ領域のビット長
// ModSize& bitOffset
//		ビット位置
//
// RETURN
// 取得した値の個数
//
// EXCEPTIONS
// なし
// 
inline ModSize
ModInvertedCoder::getBack(ModSize oldValue,
						  ModSize* valueBegin,
						  const ModSize* valueEnd, 
						  const Unit* dataBegin,
						  const ModSize bitLength,
						  ModSize& bitOffset) const
{
	ModSize* valueHead = valueBegin;
	ModSize tmpValue;
	while (valueBegin < valueEnd) {
		if (getBack(tmpValue, dataBegin, bitLength, bitOffset) == ModFalse) {
			break;
		}
		*valueBegin = tmpValue + oldValue;
		oldValue = *valueBegin;
		++valueBegin;
	}
	return static_cast<ModSize>(valueBegin - valueHead);
}


//
// FUNCTION
// ModInvertedCoder::set --- データを前向きにセット
//
// NOTES
// 領域に収まるか否かのチェックは行なわない。上位が行なうこと。
// ModOffset が 8 バイト整数であることを前提にしている。
// 
// ARGUMENTS
// Unit* dataBegin
//		領域の先頭
// ModSize bitOffset
//		セットする位置のビットオフセット
// ModSize bitLength
//		セットするビット長
// ModFileOffset bitString
//		セットするビット列
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedCoder::set(Unit* dataBegin,
					  ModSize bitOffset,
					  ModSize bitLength,
					  ModFileOffset bitString)
{
	dataBegin += (bitOffset>>5);
	bitOffset &= 31;	//	bitOffset %= 32;

#if 1
	bitOffset += bitLength;
	if (bitOffset <= 32) {				// shift >= 0
		// １つのユニットに収まる場合
		*dataBegin |= (ModSize)(bitString<<(32 - bitOffset)) & 0xffffffff;
	} else if (bitOffset <= 64) {
		// ２つのユニットに跨る場合
		*dataBegin |= (ModSize)(bitString>>(bitOffset - 32)) & 0xffffffff;
		++dataBegin;
		*dataBegin |= (ModSize)(bitString<<(64 - bitOffset)) & 0xffffffff;
	} else {
		// ３つのユニットに跨る場合
		*dataBegin |= (ModSize)(bitString>>(bitOffset - 32)) & 0xffffffff;
		++dataBegin;
		*dataBegin |= (ModSize)(bitString>>(bitOffset - 64)) & 0xffffffff;
		++dataBegin;
		*dataBegin |= (ModSize)(bitString<<(96 - bitOffset)) & 0xffffffff;
	}
#else
	// シフト量を計算して、与えられたビット列を書き込む。
	// ビット列は高々２ユニットに収めることができる。
	bitOffset += bitLength;
	if (bitOffset <= 32) {				// shift >= 0
		// １つのユニットに収まる場合
		bitString <<= 32 - bitOffset;
		*dataBegin |= 0xffffffff&bitString;
	} else {
		// ２つのユニットに跨る場合
		;ModAssert(bitOffset <= 64);
		bitString <<= 64 - bitOffset;
		*dataBegin |= (bitString>>32);
		++dataBegin;
		*dataBegin |= 0xffffffff&bitString;
	}
#endif
}


//
// FUNCTION
// ModInvertedCoder::setBack --- データを後向きにセット
//
// NOTES
// 領域に収まるか否かのチェックは行なわない。上位が行なうこと。
// ModOffset が 8 バイト整数であることを前提にしている。
// 
// ARGUMENTS
// Unit* dataBegin
//		領域の先頭
// ModSize bitOffset
//		セットする位置のビットオフセット
// ModSize bitLength
//		セットするビット長
// ModFileOffset bitString
//		セットするビット列
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedCoder::setBack(Unit* dataTail,
						  ModSize bitOffset,
						  ModSize bitLength,
						  ModFileOffset bitString)
{
	dataTail -= (bitOffset>>5);
	bitOffset &= 31;

	// bitOffset はシフトするのに使用するので、set() とは異なり bitLength を
	// 判定に用いる
	bitLength += bitOffset;
	if (bitLength <= 32) {
		// １つのユニットに収まる場合
		*dataTail |= (ModSize)(bitString<<bitOffset) & 0xffffffff;
	} else if (bitLength <= 64) {
		// ２つのユニットに跨る場合
		bitString <<= bitOffset;
		*dataTail |= (ModSize)bitString & 0xffffffff;
		--dataTail;
		*dataTail |= (ModSize)(bitString>>32) & 0xffffffff;
	} else {
		// ３つのユニットに跨る場合
		*dataTail |= (ModSize)(bitString<<bitOffset) & 0xffffffff;
		--dataTail;
		*dataTail |= (ModSize)(bitString>>(32 - bitOffset)) & 0xffffffff;
		--dataTail;
		*dataTail |= (ModSize)(bitString>>(64 - bitOffset)) & 0xffffffff;
	}
}


//
// FUNCTION
// ModInvertedCoder::setOff -- 末尾のクリア
//
// NOTES
// dataBegin の前から bitOffset1 から dataEnd の後ろから bitOffset2 の範囲を
// 0 でクリアする。
// 具象クラスにおいて 0 でクリアする以外の処理が必要な場合はこの関数をオーバー
// ライトすること。
//
// ARGUMENTS
// Unit* dataBegin
//		領域の先頭
// const Unit* dataEnd
//		領域の末尾
// ModSize bitOffset1
//		クリアする開始位置のビットオフセット
// ModSize bitOffset2
//		クリアする末尾位置のビットオフセット
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedCoder::setOff(Unit* dataBegin,
						 const Unit* dataEnd,
						 ModSize bitOffset1,
						 ModSize bitOffset2)
{
	; ModAssert(dataBegin <= dataEnd);
	; ModAssert(bitOffset1 + bitOffset2 <= (ModSize)((dataEnd - dataBegin)<<5));

	// 0 を詰めはじめるべきユニットまでシフトする
	dataBegin += bitOffset1>>5;

	// 0 を詰めおわるべきユニットを得る
	const Unit* dataTail = dataEnd - 1;
	dataTail -= bitOffset2>>5;

	bitOffset1 &= 31;
	bitOffset2 &= 31;

	if (dataBegin == dataTail) {
		// 単一のユニットの場合
		if (bitOffset1 == 0 && bitOffset2 == 0) {
			*dataBegin = 0;
		} else if (bitOffset1 == 0) {
			*dataBegin &= ~(0xffffffff<<bitOffset2);
		} else if (bitOffset2 == 0) {
			*dataBegin &= (0xffffffff<<(32 - bitOffset1));
		} else {
			*dataBegin &= (0xffffffff<<(32 - bitOffset1))|
				~(0xffffffff<<bitOffset2);
		}
	} else {
		// 複数のユニットの場合
		if (bitOffset1 == 0) {
			*dataBegin = 0;
		} else {
			*dataBegin &= (0xffffffff<<(32 - bitOffset1));
		}
		while (++dataBegin < dataTail) {
			*dataBegin = 0;
		}
		if (bitOffset2 == 0) {
			*dataBegin = 0;
		} else {
			*dataBegin &= ~(0xffffffff<<bitOffset2);
		}
	}
}


//
// FUNCTION
// ModInvertedCoder::setOff -- 末尾のクリア
//
// NOTES
// dataBegin の前から bitOffset1 から bitOffset2 の範囲を 0 でクリアする。
// 具象クラスにおいて 0 でクリアする以外の処理が必要な場合はこの関数をオーバー
// ライトすること。
//
// ARGUMENTS
// Unit* dataBegin
//		領域の先頭
// ModSize bitOffset1
//		クリアする開始位置のビットオフセット
// ModSize bitOffset2
//		クリアする末尾位置のビットオフセット
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedCoder::setOff(Unit* dataBegin,
						 ModSize bitOffset1, ModSize bitOffset2)
{
	; ModAssert(bitOffset1 < bitOffset2);

	// 0 を詰めおわるべきユニットを得る
	const Unit* dataTail = dataBegin + (bitOffset2>>5);
	// 0 を詰めはじめるべきユニットまでシフトする
	dataBegin += bitOffset1>>5;

	bitOffset1 &= 31;
	bitOffset2 &= 31;

	if (dataBegin == dataTail) {
		// 単一のユニットの場合
		; ModAssert(bitOffset1 < bitOffset2);
		if (bitOffset1 == 0) {
			*dataBegin &= ~(0xffffffff<<(32 - bitOffset2));
//		} else if (bitOffset2 == 32) {
//			*dataBegin &= (0xffffffff<<(32 - bitOffset1));
		} else {
			*dataBegin &= (0xffffffff<<(32 - bitOffset1))|
				~(0xffffffff<<(32 - bitOffset2));
		}
	} else {
		// 複数のユニットの場合
		if (bitOffset1 == 0) {
			*dataBegin = 0;
		} else {
			*dataBegin &= (0xffffffff<<(32 - bitOffset1));
		}
		while (++dataBegin < dataTail) {
			*dataBegin = 0;
		}
		if (bitOffset2 != 0) {
			// bitOffset2 == 0 の時は前のユニットを 0 で詰めればよいので
			// 何もしない。bitOffset2 != 0 のときだけ処理する。
			*dataBegin &= ~(0xffffffff<<(32 - bitOffset2));
		}
	}
}


//
// FUNCTION
// ModInvertedCoder::setOffBack -- 末尾のクリア
//
// NOTES
// dataEnd の後ろから bitOffset1 から bitOffset2 の範囲を 0 でクリアする。
// 具象クラスにおいて 0 でクリアする以外の処理が必要な場合はこの関数をオーバー
// ライトすること。
//
// ARGUMENTS
// Unit* dataEnd
//		領域の末尾
// ModSize bitOffset1
//		クリアする開始位置のビットオフセット
// ModSize bitOffset2
//		クリアする末尾位置のビットオフセット
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedCoder::setOffBack(Unit* dataEnd,
							 ModSize bitOffset1, ModSize bitOffset2)
{
	; ModAssert(bitOffset1 < bitOffset2);

	// 0 を詰めおわるべきユニットを得る
	const Unit* dataTail = dataEnd - 1 - (bitOffset1>>5);
	// 0 を詰めはじめるべきユニットを得る
	Unit* dataBegin = dataEnd - 1 - (bitOffset2>>5);

	bitOffset1 &= 31;
	bitOffset2 &= 31;

	if (dataBegin == dataTail) {
		// 単一のユニットの場合
		;ModAssert(bitOffset1 < bitOffset2);
		if (bitOffset1 == 0) {
			*dataBegin &= (0xffffffff<<bitOffset2);
		} else {
			*dataBegin &= ~(0xffffffff<<bitOffset1)|
				(0xffffffff<<bitOffset2);
		}
	} else {
		// 複数のユニットの場合
		if (bitOffset2 != 0) {
			*dataBegin &= (0xffffffff<<bitOffset2);
		}
		while (++dataBegin < dataTail) {
			*dataBegin = 0;
		}
		if (bitOffset1 == 0) {
			*dataBegin = 0;
		} else {
			*dataBegin &= ~(0xffffffff<<bitOffset1);
		}
	}
}


//
// FUNCTION
// ModInvertedCoder::isZero -- 0 か否かの検査
//
// NOTES
// dataBegin の bitOffset から dataEnd までが 0 か否かを調べる
//
// ARGUMENTS
// Unit* dataBegin
//		領域の先頭
// const Unit* dataEnd
//		領域の末尾
// ModSize bitOffset
//		検査開始位置のビットオフセット
//
// RETURN
// 0 であれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedCoder::isZero(const Unit* dataBegin,
						 const Unit* dataEnd,
						 ModSize bitOffset)
{
	if (dataBegin == dataEnd) {
		; ModAssert(bitOffset == 0);
		return ModTrue;
	}
	; ModAssert(dataBegin < dataEnd);

	if ((ModSize)((dataEnd - dataBegin)<<5) <= bitOffset) {
		// 末尾を越えている場合は常に正しいとする
		; ModAssert((ModSize)((dataEnd - dataBegin)<<5) == bitOffset);
		return ModTrue;
	}

	dataBegin += bitOffset>>5;
	bitOffset &= 31;

	if (((*dataBegin)&(ModUInt32Max>>bitOffset)) != 0) {
		return ModFalse;
	}

	++dataBegin;
	while (dataBegin < dataEnd) {
		if (*dataBegin != 0) {
			return ModFalse;
		}
		++dataBegin;
	}

	return ModTrue;
}

#endif	// __ModInvertedCoder_H__

//
// Copyright (c) 1997, 1998, 1999, 2000, 2002, 2004, 2009, 2010, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
