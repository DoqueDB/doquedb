// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Mode.h -- ロックモード関連のクラス定義、関数宣言
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

#ifndef	__SYDNEY_LOCK_MODE_H
#define	__SYDNEY_LOCK_MODE_H

#include "Lock/Module.h"

class ModMessageStream;
class ModOstream;

_SYDNEY_BEGIN
_SYDNEY_LOCK_BEGIN

//	CLASS
//	Lock::Mode -- ロックモードを表すクラス
//
//	NOTES

class SYD_LOCK_FUNCTION Mode
{
public:
	//	ENUM
	//	Lock::Mode::Value -- ロックモードの値を表す列挙型
	//
	//	NOTES

	enum Value
	{
		VIS = 0,	// VIS ロック
		VS,			// VS ロック
		IS,			// IS ロック
		VSIS,		// VSIS ロック
		IX,			// IX ロック
		S,			// S ロック
		VSIX,		// VSIX ロック
		VIX,		// VIX ロック
		VSVIX,		// VSVIX ロック
		SIX,		// SIX ロック
		U,			// U ロック
		SVIX,		// SVIX ロック
		X,			// X ロック
		VIXX,		// VIXX ロック
		VX,			// VX ロック
		N,			// N ロック
		ValueNum	// 値の種類数
	};

	//	CLASS
	//	Lock::Possibility -- ロック可能性を表すクラス
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している

	class Possibility
	{
	public:
		//	ENUM
		//	Lock::Possibility::Value -- ロック可能性の値を表す列挙型
		//
		//	NOTES

		enum Value
		{
			Impossible =		0,				// 不可
			Possible,							// 可能かつ必要である
			Unnecessary,						// 可能だが必要ない
			ValueNum							// 値の種類数
		};
	};

	// コンストラクター
	Mode(Value	value = N);

	Value						operator *() const;
												// * 単項演算子

	Mode&						operator =(const Mode&	src);
	Mode&						operator =(Value src);
												// = 演算子

	bool						operator ==(const Mode&	src) const;
	bool						operator ==(Value r) const;
												// == 演算子
	bool						operator !=(const Mode&	src) const;
	bool						operator !=(Value r) const;
												// != 演算子
	bool						operator >(const Mode&	src) const;
	bool						operator >(Value r) const;
												// > 演算子
	bool						operator >=(const Mode&	src) const;
	bool						operator >=(Value r) const;
												// >= 演算子
	bool						operator <(const Mode&	src) const;
	bool						operator <(Value r) const;
												// < 演算子
	bool						operator <=(const Mode&	src) const;
	bool						operator <=(Value r) const;
												// <= 演算子

	// 与えられたロックとの最小上界を得る
	Value						getLeastUpperBound(Value	requested) const;
	static Value				getLeastUpperBound(Value	granted,
												   Value	requested);

	// 与えられたロックと両立するか
	bool						isCompatible(Value	requested) const;
	static bool					isCompatible(Value	granted,
											 Value	requested);

	// 親のロックに対してロック可能か
	Possibility::Value			isPossible(Value	child) const;
	static Possibility::Value	isPossible(Value	parent,
										   Value	child);

private:
	Value						_value;			// ロックモード値
};

//	FUNCTION public
//	Lock::Mode::Mode -- ロックモードクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Mode::Value	value
//			指定されたとき
//				新しいロックモードの値
//			指定されないとき
//				Lock::Mode::N が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Lock::Mode::Mode(Mode::Value	value)
	: _value(value)
{ }

//	FUNCTION public
//	Lock::Mode::operator * -- * 単項演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		自分自身の保持するロックモード値
//
//	EXCEPTIONS
//		なし

inline
Lock::Mode::Value
Lock::Mode::operator *() const
{
	return _value;
}

//	FUNCTION public
//	Lock::Mode::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Mode::Value	src
//			自分自身の値として代入するロックモード値
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		なし

inline
Lock::Mode&
Lock::Mode::operator =(const Mode&	src)
{
	_value = src._value;
	return *this;
}

inline
Lock::Mode&
Lock::Mode::operator =(Mode::Value	src)
{
	_value = src;
	return *this;
}

//	FUNCTION public
//	Lock::Mode::operator == -- == 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Mode::Value	r
//			自分自身の値と比較するロックモード値
//
//	RETURN
//		true
//			与えられたロックモード値と自分自身の値は等しい
//		false
//			自分自身と等しくない
//
//	EXCEPTIONS
//		なし

inline
bool
Lock::Mode::operator ==(const Mode&	src) const
{
	return _value == src._value;
}

inline
bool
Lock::Mode::operator ==(Mode::Value	r) const
{
	return _value == r;
}

//	FUNCTION public
//	Lock::Mode::operator != -- != 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Mode::Value	r
//			自分自身の値と比較するロックモード値
//
//	RETURN
//		true
//			与えられたロックモード値と自分自身の値は等しくない
//		false
//			自分自身と等しい
//
//	EXCEPTIONS
//		なし

inline
bool
Lock::Mode::operator !=(const Mode&	src) const
{
	return _value != src._value;
}

inline
bool
Lock::Mode::operator !=(Mode::Value	r) const
{
	return _value != r;
}

//	FUNCTION public
//	Lock::Mode::operator > -- > 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Mode::Value	r
//			自分自身の値と比較するロックモード値
//
//	RETURN
//		true
//			自分自身の値は与えられたロックモード値より大きい
//		false
//			より大きくない
//
//	EXCEPTIONS
//		なし

inline
bool
Lock::Mode::operator >(const Mode&	src) const
{
	return !(*this <= src);
}

inline
bool
Lock::Mode::operator >(Mode::Value	r) const
{
	return !(*this <= r);
}

//	FUNCTION public
//	Lock::Mode::operator >= -- >= 演算子
//
//	NOTES
//		自分自身(の保持するロックモード値)がロックモード値 r 以上であるとは、
//		たんに列挙子の値の比較をするのではなく、
//		自分自身と要求されたロックモード値 r の最小上界が
//		自分自身と等しいことを調べる
//
//	ARGUMENTS
//		Lock::Mode::Value	r
//			自分自身と比較するロックモード値
//
//	RETURN
//		true
//			自分自身は与えられたロックモード値以上である
//		false
//			以上でない
//
//	EXCEPTIONS
//		なし

inline
bool
Lock::Mode::operator >=(const Mode&	src) const
{
	return _value == getLeastUpperBound(_value, src._value);
}

inline
bool
Lock::Mode::operator >=(Mode::Value	r) const
{
	return _value == getLeastUpperBound(_value, r);
}

//	FUNCTION public
//	Lock::Mode::operator < -- < 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Mode::Value	r
//			自分自身の値と比較するロックモード値
//
//	RETURN
//		true
//			自分自身の値は与えられたロックモード値より小さい
//		false
//			小さくない
//
//	EXCEPTIONS
//		なし

inline
bool
Lock::Mode::operator <(const Mode&	src) const
{
	return !(*this >= src);
}

inline
bool
Lock::Mode::operator <(Mode::Value	r) const
{
	return !(*this >= r);
}

//	FUNCTION public
//	Lock::Mode::operator <= -- <= 演算子
//
//	NOTES
//		自分自身(の保持するロックモード値)がロックモード値 r 以下であるとは、
//		たんに列挙子の値の比較をするのではなく、
//		ロックモード値 r と要求された自分自身の最小上界が
//		ロックモード値 r と等しいことを調べる
//
//	ARGUMENTS
//		Lock::Mode::Value	r
//			自分自身と比較するロックモード値
//
//	RETURN
//		true
//			自分自身は与えられたロックモード値以下である
//		false
//			以下でない
//
//	EXCEPTIONS
//		なし

inline
bool
Lock::Mode::operator <=(const Mode&	src) const
{
	return src._value == getLeastUpperBound(src._value, _value);
}

inline
bool
Lock::Mode::operator <=(Mode::Value	r) const
{
	return r == getLeastUpperBound(r, _value);
}

//	FUNCTION public
//	Lock::Mode::getLeastUpperBound --
//		自分自身と要求されたロックモードとの最小上界となる
//		ロックモードを得る
//
//	NOTES
//		自分自身でロックされているロック対象を、
//		あるモードでさらにロックしたときに、そのロック対象は、
//		結果的にどういうモードでロックされるかを得るために使用する
//
//	ARGUMENTS
//		Lock::Mode::Value	requested
//			自分自身との最小上界を得る要求されたロックのロックモード値
//
//	RETURN
//		自分自身と与えられたロックモードの最小上界
//
//	EXCEPTIONS
//		なし

inline
Lock::Mode::Value
Lock::Mode::getLeastUpperBound(Mode::Value	requested) const
{
	return getLeastUpperBound(_value, requested);
}

//	FUNCTION public
//	Lock::Mode::isCompatible --
//		自分自身と要求されたロックモードの両立性があるか調べる
//
//	NOTES
//		自分自身でロックされているロック対象を、
//		あるモードでロック可能か調べるために使用する
//
//	ARGUMENTS
//		Lock::Mode::Value	requested
//			自分自身と両立するか調べる要求されたロックのロックモード値
//
//	RETURN
//		true
//			要求されたロックは、自分自身と両立する
//		false
//			両立しない
//
//	EXCEPTIONS
//		なし

inline
bool
Lock::Mode::isCompatible(Mode::Value	requested) const
{
	return isCompatible(_value, requested);
}

//	FUNCTION public
//	Lock::Mode::isPossible --
//		あるロック対象の現在のロックモードが自分自身のとき、
//		その子に対して要求されたロックモードでロック可能か調べる
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Mode::Value	child
//			子に要求されたロックのロックモード値
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
Lock::Mode::Possibility::Value
Lock::Mode::isPossible(Mode::Value	child) const
{
	return isPossible(_value, child);
}

_SYDNEY_LOCK_END
_SYDNEY_END

// ロックモードをメッセージに出力するための関数
ModMessageStream& operator<<(ModMessageStream& cStream_, _SYDNEY::Lock::Mode::Value eValue_);
ModOstream& operator<<(ModOstream& cStream_, _SYDNEY::Lock::Mode::Value eValue_);

#endif	// __SYDNEY_LOCK_MODE_H

//
// Copyright (c) 2000, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
