// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Count.h -- ロック数関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOCK_COUNT_H
#define	__SYDNEY_LOCK_COUNT_H

#include "Lock/Module.h"
#include "Lock/Mode.h"

#include "ModTypes.h"

_SYDNEY_BEGIN
_SYDNEY_LOCK_BEGIN

class Request;

//	CLASS
//	Lock::Count -- ロック数を表すクラス
//
//	NOTES

class Count
{
public:

	//	TYPEDEF
	//	Lock::Count::Value -- ロック数の値を表す型
	//
	//	NOTES

	typedef	ModUInt32	Value;

	//	CLASS
	//	Lock::Count::Bit --
	//		ロックモード別にロックの有無を示すビットを表すクラス
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している

	class Bit
	{
	public:

		//	ENUM
		//	Lock::Count::Bit::Value --
		//		ロックモード別にロックの有無を示すビットの値を表す列挙型
		//
		//	NOTES

		enum Value
		{
			VIS =	0x1 << Mode::VIS,			// VIS ロック
			VS =	0x1 << Mode::VS,			// VS ロック
			IS =	0x1 << Mode::IS,			// IS ロック
			VSIS =	0x1 << Mode::VSIS,			// VSIS ロック
			IX =	0x1 << Mode::IX,			// IX ロック
			S =		0x1 << Mode::S,				// S ロック
			VSIX =	0x1 << Mode::VSIX,			// VSIX ロック
			VIX =	0x1 << Mode::VIX,			// VIX ロック
			VSVIX =	0x1 << Mode::VSVIX,			// VSVIX ロック
			SIX =	0x1 << Mode::SIX,			// SIX ロック
			U =		0x1 << Mode::U,				// U ロック
			SVIX =	0x1 << Mode::SVIX,			// SVIX ロック
			X =		0x1 << Mode::X,				// X ロック
			VIXX =	0x1 << Mode::VIXX,			// VIXX ロック
			VX =	0x1 << Mode::VX,			// VX ロック
			Mask =	VIS|VS|IS|VSIS|IX|S|VSIX|VIX|VSVIX|SIX|U|SVIX|X|VIXX|VX
												// 全ビットのマスク
		};
	};

	static void initializeCountTable();

	// デフォルトコンストラクター
	Count();
	// コピーコンストラクター
	Count(const Count& src);

	// ロック数を生成する
	static Count*
	attach(const Count& count);
	// 参照数を 1 増やす
	Count*
	attach();
	// ロック数を破棄する
	static void
	detach(Count*& count);
#ifdef OBSOLETE
	// 参照数を 1 減らす
	void
	detach();
	// 参照数を得る
	unsigned int
	getRefCount() const;
#endif

	// = 演算子
	Count&
	operator =(const Count&	r);
#ifdef OBSOLETE
	// += 演算子
	Count&
	operator +=(const Count& r);
#endif
	// -= 演算子
	Count&
	operator -=(const Count& r);
	// == 演算子
	bool
	operator ==(const Count& r) const;
	// * 単項演算子
	Mode::Value
	operator *() const;
	// [] 演算子
	Value
	operator [](Mode::Value mode) const;

	// あるロック数を代入する
	static void
	substitute(Count*& l, const Count& r);

	// あるロックモードのロック数を増やす
	Value
	up(Mode::Value mode, Value n = 1);
	static void
	up(Count*& l, Mode::Value mode, Value n = 1);
#ifdef OBSOLETE
	// あるロック数を増やす
	static void
	up(Count*& l, const Count& r);
#endif

	// あるロックモードのロック数を減らす
#ifdef OBSOLETE
	Value
	down(Mode::Value mode);
#endif
	Value
	down(Mode::Value mode, Value& n);
	// あるロックモードのロック数を減らす
#ifdef OBSOLETE
	static void
	down(Count*& l, Mode::Value mode);
#endif
	static void
	down(Count*& l, Mode::Value mode, Value& n);
	// あるロック数を減らす
	static void
	down(Count*& l, const Count& r);

	// あるロックモードのロック数を 0 にする
	void
	clear(Mode::Value mode);
	static void
	clear(Count*& l, Mode::Value mode);
	// すべてのロックモードのロック数を 0 にする
	void
	clear();
	static void
	clear(Count*& l);

	// 存在するすべてのロックの最小上界を得る
	Mode::Value
	getLeastUpperBound() const;
#ifdef OBSOLETE
	// 与えられたロックとの最小上界を得る
	Mode::Value
	getLeastUpperBound(Mode::Value requested) const;

	// 与えられたロック数と両立するか
	bool
	isCompatible(Mode::Value requested) const;
	// 親のロック数に対してロック可能か
	Mode::Possibility::Value					
	isPossible(Mode::Value child) const;
#endif

private:
	//	TYPEDEF
	//	Lock::Count::Bitmap --
	//		ロックモード別にロックの有無を表すビットからなる
	//		ビットマップを表す型
	//
	//	NOTES

	typedef	unsigned short	Bitmap;

	// 存在するすべてのロックの最小上界を得る
	static Mode::Value
	getLeastUpperBound(Bitmap bitmap);

	// 参照数
	mutable unsigned int	_refCount;
	// ハッシュリストでの直前の要素へのポインタ
	Count*					_hashPrev;
	// ハッシュリストでの直後の要素へのポインタ
	Count*					_hashNext;

	// ロックモード別のロック数
	Value					_value[Mode::N];
	// ロックモード別のロックの有無を記録するビットマップ
	Bitmap					_bitmap;
};
	
//	FUNCTION public
//	Lock::Count::Count -- ロック数を表すクラスのデフォルトコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Count::Count()
	: _refCount(0),
	  _hashPrev(0),
	  _hashNext(0)
{
	clear();
}

//	FUNCTION public
//	Lock::Count::Count -- ロック数を表すクラスのコピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Count&	src
//			生成されたロック数に代入するロック数
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Count::Count(const Count& src)
	: _refCount(0),
	  _hashPrev(0),
	  _hashNext(0)
{
	*this = src;
}

//	FUNCTION public
//	Lock::Count::operator * -- * 単項演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//		
//	RETURN
//		自分自身に記録されているすべてのロックの最小上界
//
//	EXCEPTIONS
//		なし

inline
Mode::Value
Count::operator *() const
{
	return getLeastUpperBound();
}

//	FUNCTION public
//	Lock::Count::operator [] -- [] 演算子
//
//	NOTES
//		Lock::Mode::N を与えたときの動作は不定である
//
//	ARGUMENTS
//		Lock::Mode::Value	mode
//			そのロックモードのロック数を得る
//
//	RETURN
//		得られたロック数
//
//	EXCEPTIONS
//		なし

inline
Count::Value
Count::operator [](Mode::Value mode) const
{
	return _value[mode];
}

#ifdef OBSOLETE
//	FUNCTION public
//	Lock::Count::down -- あるロックモードのロック数を 1 減らす
//

//	NOTES
//		Lock::Mode::N を与えたときの動作は不定である
//
//	ARGUMENTS
//		Lock::Mode::Value	mode
//			ロック数を減らすロックのモード
//
//	RETURN
//		指定されたロックモードの減算後のロック数
//
//	EXCEPTIONS
//		なし

inline
Count::Value
Count::down(Mode::Value mode)
{
	Value n = 1;
	return down(mode, n);
}

//	FUNCTION public
//	Lock::Count::down -- あるロックモードのロック数を 1 減らしたロック数を得る
//

//	NOTES
//		Lock::Mode::N を与えたときの動作は不定である
//
//	ARGUMENTS
//		Lock::Count*&		l
//			あるロックモードのロック数を 1 減らしたいロック数が
//			格納された領域の先頭アドレスで、
//			呼出し後、得られたロック数を格納する領域の先頭アドレスが設定される
//		Lock::Mode::Value	mode
//			ロック数を減らすロックのモード
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
inline
void
Count::down(Count*& l, Mode::Value mode)
{
	Value n = 1;
	down(l, mode, n);
}
#endif

//	FUNCTION public
//	Lock::Count::clear -- あるロックモードのロック数を 0 にする
//
//	NOTES
//		Lock::Mode::N を与えたときの動作は不定である
//
//	ARGUMENTS
//		Lock::Mode::Value	mode
//			ロック数を 0 にするロックのモード
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
Count::clear(Mode::Value mode)
{
	Value n = _value[mode];
	(void) down(mode, n);
}

//	FUNCTION public
//	Lock::Count::clear -- あるロックモードのロック数を 0 にしたロック数を得る
//
//	NOTES
//		Lock::Mode::N を与えたときの動作は不定である
//
//	ARGUMENTS
//		Lock::Count*&		l
//			あるロックモードのロック数を 0 にしたいロック数が
//			格納された領域の先頭アドレスで、
//			呼出し後、得られたロック数を格納する領域の先頭アドレスが設定される
//		Lock::Mode::Value	mode
//			ロック数を 0 にするロックのモード
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
inline
void
Count::clear(Count*& l, Mode::Value mode)
{
	if (l) {
		Value n = (*l)[mode];
		down(l, mode, n);
	}
}

//	FUNCTION public
//	Lock::Count::getLeastUpperBound --
//		自分自身に記録されているすべてのロックの最小上界となる
//		ロックモードを得る
//
//	NOTES
//		あるトランザクションが自分自身に記録されているロックを行っているとき、
//		そのトランザクションは
//		どのモードで実際にロックしているかを求めるために使用する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		自分自身に記録されているすべてのロックの最小上界
//
//	EXCEPTIONS
//		なし

inline
Mode::Value
Count::getLeastUpperBound() const
{
	return getLeastUpperBound(_bitmap);
}

#ifdef OBSOLETE
//	FUNCTION public
//	Lock::Count::getLeastUpperBound --
//		あるロック対象の現在のロック数が自分自身のとき、
//		指定されたモードとの最小上界となるロックモードを得る
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Mode::Value	requested
//			自分自身との最小上界を得る要求されたロックのモード
//			
//	RETURN
//		自分自身と与えられたロックモードの最小上界
//
//	EXCEPTIONS
//		なし

inline
Mode::Value
Count::getLeastUpperBound(Mode::Value requested) const
{
	return Mode::getLeastUpperBound(*(*this), requested);
}

//	FUNCTION public
//	Lock::Count::isCompatible --
//		あるロック対象の現在のロック数が自分自身のとき、
//		指定されたモードのロックが両立するか調べる
//
//	NOTES
//		すでに自分自身ぶんロックされている対象を、
//		指定されたモードのロックでさらにロック可能か調べるために使用する
//
//	ARGUMENTS
//		Lock::Mode::Value	requested
//			調べるロック対象にさらに要求されたロックのモード
//
//	RETURN
//		true
//			要求されたロックは現在のロックと両立する
//		false
//			両立しない
//
//	EXCEPTIONS
//		なし

inline
bool
Count::isCompatible(Mode::Value requested) const
{
	return Mode::isCompatible(*(*this), requested);
}

//	FUNCTION public
//	Lock::Count::isPossible --
//		あるロック対象の現在のロック数が自分自身のとき、
//		その子に対して指定されたモードのロックが可能か調べる
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Mode::Value	child
//			子に要求するロックのモード
//
//	RETURN
//		Lock::Mode::Impossible
//			子に対して要求されたモードでロックできない
//		Lock::Mode::Possible
//			子に対して要求されたモードでロック可能であり、必要である
//		Lock::Mode::Unnecessary
//			子に対して要求されたモードでロック可能であるが、必要ない
//
//	EXCEPTIONS
//		なし

inline
Mode::Possibility::Value
Count::isPossible(Mode::Value child) const
{
	return Mode::isPossible(*(*this), child);
}
#endif

_SYDNEY_LOCK_END
_SYDNEY_END

#endif	// __SYDNEY_LOCK_COUNT_H

//
// Copyright (c) 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
