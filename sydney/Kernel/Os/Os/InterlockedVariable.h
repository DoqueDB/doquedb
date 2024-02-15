// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InterlockedVariable.h -- 内部ロック変数関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__TRMEISTER_OS_INTERLOCKEDVARIABLE_H
#define	__TRMEISTER_OS_INTERLOCKEDVARIABLE_H

#if !defined(SYD_OS_WINDOWS) && !defined(SYD_OS_POSIX)
#error require #include "SyDefault.h"
#endif

#ifdef SYD_OS_WINDOWS
// #include <windows.h>
#endif

#include "Os/Module.h"
#ifdef SYD_OS_WINDOWS
#else
#include "Os/CriticalSection.h"
#endif

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	CLASS
//	Os::InterlockedVariable -- 内部ロック変数を表すクラス
//
//	NOTES
//		同プロセススレッド間で同期的に 32 ビット長符号付整数を
//		操作可能にするためのインタフェースを提供する

class InterlockedVariable
{
public:
	// コンストラクター
	InterlockedVariable(int v = 0);
	// デストラクター
	~InterlockedVariable();

	// int へのキャスト演算子
	operator				int() const;
#ifdef OBSOLETE
	// ++ 前置演算子
	InterlockedVariable&	operator ++();
	// -- 前置演算子
	InterlockedVariable&	operator --();
#endif
	// 変数の値を1 増やす
	int						increment();
	// 変数の値を 1 減らす
	int						decrement();
	// 変数に値を格納する
	int						exchange(int v);
	// 変数に値を加算する
	int						exchangeAdd(int v);
	// 等しければ変数に値を格納する
	int						compareExchange(int v, int compared);

private:
#ifdef SYD_OS_WINDOWS
#else
	// スレッド間で同期を取るためのラッチ
	CriticalSection			_latch;
#endif
	// 操作する 32 ビット長符号付整数
	int						_v;
};

//	FUNCTION public
//	Os::InterlockedVariable::InterlockedVariable --
//		内部ロック変数を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		int&				v
//			指定されたとき
//				操作する 32 ビット長符号付整数の初期値
//			指定されないとき
//				0 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
InterlockedVariable::InterlockedVariable(int v)
	: _v(v)
{}

//	FUNCTION public
//	Os::InterlockedVariable::~InterlockedVariable --
//		内部ロック変数を表すクラスのデストラクター
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
InterlockedVariable::~InterlockedVariable()
{}

//	FUNCTION public
//	InterlockedVariable::operator int -- int へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		操作する 32 ビット長符号付整数の値
//
//	EXCEPTIONS
//		なし 

inline
InterlockedVariable::operator int() const
{
	return _v;
}

#ifdef OBSOLETE
//	FUNCTION public
//	Os::InterlockedVariable::operator ++ -- ++ 前置演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		1 増やした後の自分自身
//
//	EXCEPTIONS
//		なし

inline
InterlockedVariable&
InterlockedVariable::operator ++()
{
	(void) increment();
	return *this;
}

//	FUNCTION public
//	Os::InterlockedVariable::operator -- -- -- 前置演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		1 減らした後の自分自身
//
//	EXCEPTIONS
//		なし

inline
InterlockedVariable&
InterlockedVariable::operator --()
{
	 (void) decrement();
	 return *this;
}
#endif

//	FUNCTION public
//	Os::InterlockedVariable::increment --
//		内部ロック変数の管理する 32 ビット長符号付整数の値を 1 増やす
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		1 増やした後の 32 ビット長符号付整数の値
//
//	EXCEPTIONS
//		なし

inline
int
InterlockedVariable::increment()
{
#ifdef SYD_OS_WINDOWS
	return ::InterlockedIncrement(syd_reinterpret_cast<LPLONG>(&_v));
#else
	_latch.lock();
	int	ret = ++_v;
	_latch.unlock();

	return ret;
#endif
}

//	FUNCTION public
//	Os::InterlockedVariable::decrement --
//		内部ロック変数の管理する 32 ビット長符号付整数の値を 1 減らす
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		1 減らした後の 32 ビット長符号付整数の値
//
//	EXCEPTIONS
//		なし

inline
int
InterlockedVariable::decrement()
{
#ifdef SYD_OS_WINDOWS
	return ::InterlockedDecrement(syd_reinterpret_cast<LPLONG>(&_v));
#else
	_latch.lock();
	int ret = --_v;
	_latch.unlock();

	return ret;
#endif
}

//	FUNCTION public
//	Os::InterlockedVariable::exchange --
//		内部ロック変数の管理する 32 ビット長符号付整数に値を格納する
//
//	NOTES
//
//	ARGUMENTS
//		int					v
//			内部ロック変数の管理する 32 ビット長符号付整数に格納する値
//
//	RETURN
//		与えられた値を格納する前の 32 ビット長符号付整数の値
//
//	EXCEPTIONS
//		なし

inline
int
InterlockedVariable::exchange(int v)
{
#ifdef SYD_OS_WINDOWS
	return ::InterlockedExchange(syd_reinterpret_cast<LPLONG>(&_v), v);
#else
	_latch.lock();
	int	ret = _v;
	_v = v;
	_latch.unlock();

	return ret;
#endif
}

//	FUNCTION public
//	Os::InterlockedVariable::exchangeAdd --
//		内部ロック変数の管理する 32 ビット長符号付整数に値を加算する
//
//	NOTES
//
//	ARGUMENTS
//		int					v
//			内部ロック変数の管理する 32 ビット長符号付整数に加算する値
//
//	RETURN
//		与えられた値を加算する前の 32 ビット長符号付整数の値
//
//	EXCEPTIONS
//		なし

inline
int
InterlockedVariable::exchangeAdd(int v)
{
#ifdef SYD_OS_WINDOWS
	return ::InterlockedExchangeAdd(syd_reinterpret_cast<LPLONG>(&_v), v);
#else
	_latch.lock();
	int ret = _v;
	_v += v;
	_latch.unlock();

	return ret;
#endif
}

//	FUNCTION public
//	Os::InterlockedVariable::compareExchange --
//		与えられた値と内部ロック変数の管理する
//		32 ビット長符号付整数の値が等しければ、ある値を格納する
//
//	NOTES
//
//	ARGUMENTS
//		int					v
//			内部ロック変数の管理する 32 ビット長符号付整数に格納する値
//		int					compared
//			内部ロック変数の管理する 32 ビット長符号付整数の値と比較される値
//
//	RETURN
//		与えられた値を格納する前の 32 ビット長符号付整数の値
//
//	EXCEPTIONS
//		なし

inline
int
InterlockedVariable::compareExchange(int v, int compared)
{
#ifdef SYD_OS_WINDOWS
	return ::InterlockedCompareExchange(
		syd_reinterpret_cast<LONG* volatile>(&_v), v, compared);
#else
	_latch.lock();
	int ret = _v;
	if (_v == compared)
		_v = v;
	_latch.unlock();

	return ret;
#endif
}

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_INTERLOCKEDVARIABLE_H

//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
