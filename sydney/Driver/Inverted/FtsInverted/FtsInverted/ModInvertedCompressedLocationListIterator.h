// -*-Mode: C++; tab-width: 4; -*-
// vi:set ts=4 sw=4:	
//
// ModInvertedCompressedLocationListIterator.h -- 圧縮された文書内出現位置リストの反復子
// 
// Copyright (c) 1997, 1998, 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedCompressedLocationListIterator_H__
#define __ModInvertedCompressedLocationListIterator_H__

#include "ModOs.h"
#include "ModInvertedTypes.h"
#include "ModInvertedLocationListIterator.h"
#include "ModInvertedCoder.h"

//
// DECODE_IN_NEXT を define すると、next() でつぎの出現位置を計算ようになる。
// 処理フローが単純になり、若干高速化される。
//
#define DECODE_IN_NEXT

//
// CLASS
// ModInvertedCompressedLocationListIterator --- 圧縮された位置エントリに対する反復子
//
// NOTES
// 圧縮された位置エントリに対する反復子。
//
class
ModInvertedCompressedLocationListIterator :
	public ModInvertedLocationListIterator
{
public:
	typedef ModBoolean (ModInvertedCoder::*Getfunction)(
		ModSize&, const ModInvertedDataUnit*,
		const ModSize, ModSize&) const;

	ModInvertedCompressedLocationListIterator();
	ModInvertedCompressedLocationListIterator(const ModInvertedCompressedLocationListIterator&);
	ModInvertedCompressedLocationListIterator(const ModInvertedDataUnit*,
											  ModSize, ModSize,
											  ModSize,
											  ModInvertedCoder*,
											  Getfunction,
											  ModBoolean = ModTrue);
	virtual ~ModInvertedCompressedLocationListIterator() {}

#ifdef DECODE_IN_NEXT
	virtual void reset();
	void reset(const ModBoolean);
	virtual void next();
#else
	void reset();
	void next();
#endif
	virtual ModSize getLocation();
	ModBoolean isEnd() const;

	void setLength(const ModSize);
	ModSize getEndLocation();
	ModBoolean find(ModSize);

	virtual ModSize getFrequency() { return number; }

	void release() {}

protected:
	const ModInvertedDataUnit* start;
	ModSize startBitOffset;
	ModSize endBitOffset;
	ModSize number;
	ModSize decodedNumber;
	ModSize currentBitOffset;
	ModSize currentLocation;
	ModSize locationGap;
	ModInvertedCoder* locationCoder;
	Getfunction getfunc;
	ModSize length;						// 対応するトークンの長さ
										// - トークンの出現末尾位置を求めるのに
										//   使用される
};


//
// FUNCTION
// ModInvertedCompressedLocationListIterator::ModInvertedCompressedLocationListIterator --- コンストラクタ
//
// NOTES
// 圧縮された文書内出現位置リストの反復子を構築する。
// デフォルトコンストラクタ、コピーコンストラクタ、データ設定用コンストラクタが
// ある。
// 
// ARGUMENTS
// const ModInvertedCompressedLocationListIterator& target
//		コピーもと（コピーコンストラクタの場合）
// const ModInvertedDataUnit* start_,
//		圧縮データがセットされてる領域の開始位置
// const ModSize startBitOffset_,
//		先頭ビットオフセット
// const ModSize endBitOffset_
//		末尾ビットオフセット
// const ModSize number_
//		文書内出現頻度
// ModInvertedCoder* locationCoder_
//		符合化器
// Getfunction getfunc_
//		符合化器の伸長関数
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModInvertedCompressedLocationListIterator::ModInvertedCompressedLocationListIterator()
	:
	start(0), startBitOffset(0), endBitOffset(0), number(0), decodedNumber(0),
	currentBitOffset(0), currentLocation(0), locationGap(0), locationCoder(0),
	getfunc(0), length(0)
{}

inline
ModInvertedCompressedLocationListIterator::ModInvertedCompressedLocationListIterator(
	const ModInvertedCompressedLocationListIterator& target)
	:
	start(target.start),
	startBitOffset(target.startBitOffset),
	endBitOffset(target.endBitOffset),
	number(target.number),
	decodedNumber(target.decodedNumber),
	currentBitOffset(target.currentBitOffset),
	currentLocation(target.currentLocation),
	locationGap(target.locationGap),
	locationCoder(target.locationCoder),
	getfunc(target.getfunc),
	length(target.length)
{}

inline
ModInvertedCompressedLocationListIterator::ModInvertedCompressedLocationListIterator(
	const ModInvertedDataUnit* start_,
	ModSize startBitOffset_,
	ModSize endBitOffset_,
	ModSize number_,
	ModInvertedCoder* locationCoder_,
	Getfunction getfunc_,
	ModBoolean callNext_)
	:
	start(start_),
	startBitOffset(startBitOffset_),
	endBitOffset(endBitOffset_),
	number(number_),
	decodedNumber(0),
	currentBitOffset(startBitOffset_),
	currentLocation(0),
	locationGap(0),
	locationCoder(locationCoder_),
	getfunc(getfunc_),
	length(0)
{
#ifdef DECODE_IN_NEXT
	if (callNext_ == ModTrue) {
		next();
	}
#endif
}


//
// FUNCTION
// ModInvertedCompressedLocationListIterator::setLength -- 長さの設定
//
// NOTES
// トークンの長さを設定する。
//
// ARGUMENTS
// const ModSize length_
//		長さ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedCompressedLocationListIterator::setLength(const ModSize length_)
{
	length = length_;
}


#if 0
//
// FUNCTION
// ModInvertedCompressedLocationListIterator::getLength -- 長さの取得
//
// NOTES
// トークンの長さを取得する。
//
// ARGUMENTS
// なし
//
// RETURN
// トークンの長さ
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedCompressedLocationListIterator::getLength()
{
	return length;
}
#endif


//
// FUNCTION
// ModInvertedCompressedLocationListIterator::next --- つぎに進める
//
// NOTES
// 反復子の現在位置をつぎに進める。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedCompressedLocationListIterator::next()
{
#ifdef DECODE_IN_NEXT
	if (decodedNumber >= number) {
		// 末尾にいるので何もしない
		decodedNumber = number + 1;
		return;
	}

	// 出現位置の差分を読み出す
	if ((locationCoder->*getfunc)(locationGap, start, endBitOffset,
								  currentBitOffset) == ModFalse) {
		// 読み出しに失敗するはずはない
		ModErrorMessage << decodedNumber << ' ' << number << ModEndl;
		; ModAssert(0);
	}
	currentLocation += locationGap;
	++decodedNumber;
#else
	(void)getLocation();
	; ModAssert(locationGap != 0);

	locationGap = 0;
	if (decodedNumber < number) {
		++decodedNumber;
	}
#endif
}

//
// FUNCTION
// ModInvertedCompressedLocationListIterator::isEnd --- 現在位置が末尾か調べる
//
// NOTES
// 反復子の現在位置が末尾に達したかを調べる。
//
// ARGUMENTS
// なし
//
// RETURN
// 末尾であれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModInvertedCompressedLocationListIterator::isEnd() const
{
#ifdef DECODE_IN_NEXT
	return (ModBoolean)(decodedNumber > number);
#else
	if (decodedNumber < number) {
		return ModFalse;
	}
	return ModTrue;
#endif
}


//
// FUNCTION
// ModInvertedCompressedLocationListIterator::reset --- 先頭に戻す
//
// NOTES
// 反復子の現在位置を先頭に戻す。
// 
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedCompressedLocationListIterator::reset()
{
	decodedNumber = 0;
	currentBitOffset = startBitOffset;
	currentLocation = 0;
	locationGap = 0;

#ifdef DECODE_IN_NEXT
	next();
#endif
}

#ifdef DECODE_IN_NEXT
inline void
ModInvertedCompressedLocationListIterator::reset(const ModBoolean callNext_)
{
	decodedNumber = 0;
	currentBitOffset = startBitOffset;
	currentLocation = 0;
	locationGap = 0;

	if (callNext_ == ModTrue) {
		next();
	}
}
#endif


//
// FUNCTION
// ModInvertedCompressedLocationListIterator::getLocation --- 出現位置の取得
//
// NOTES
// 反復子の現在位置における、出現位置の先頭位置を返す。
//
// ARGUMENTS
// なし
//
// RETURN
// 出現の末尾位置
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedCompressedLocationListIterator::getLocation()
{
#ifdef DECODE_IN_NEXT
	return currentLocation;
#else
	if (decodedNumber >= number || locationGap != 0) {
		return currentLocation;
	}

	// 出現位置の差分を読み出す
	if ((locationCoder->*getfunc)(locationGap, start, endBitOffset,
								  currentBitOffset) == ModFalse) {
		// 読み出しに失敗するはずはない
		; ModAssert(0);
	}
	currentLocation += locationGap;
	return currentLocation;
#endif
}


//
// FUNCTION
// ModInvertedCompressedLocationListIterator::getEndLocation -- 出現の末尾位置の取得
//
// NOTES
// 反復子の現在位置における、出現位置の末尾位置を返す。
//
// ARGUMENTS
// なし
//
// RETURN
// 出現の末尾位置
//
// EXCEPTIONS
// なし
//
inline ModSize
ModInvertedCompressedLocationListIterator::getEndLocation()
{
#ifdef DECODE_IN_NEXT
	return currentLocation + length;
#else
	return getLocation() + length;
#endif
}

//
// FUNCTION
// ModInvertedCompressedLocationListIterator::find -- 文書IDの出現位置を直接参照で取得
//
// NOTES
// 与えられた値が存在するか、UnaryCoderで圧縮されたブロックに対して、
// 直接対象ビットを参照して検索する。
//
// ARGUMENTS
// const ModSize target_
//      調査対象の値
//
// RETURN
// ビットが立っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModBoolean
ModInvertedCompressedLocationListIterator::find(ModSize target)
{
	if(locationCoder->findBack(target, start,
							   endBitOffset, startBitOffset) == ModTrue) {

		currentBitOffset = startBitOffset + target;
		currentLocation = target;
		return ModTrue;
	}
	return ModFalse;
}

#endif	__ModInvertedCompressedLocationListIterator_H__

//
// Copyright (c) 1997, 1998, 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
