// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Math.h -- 算術関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2002, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_OS_MATH_H
#define	__TRMEISTER_OS_MATH_H

#ifdef SYD_OS_WINDOWS
//#include <math.h>
#endif
#ifdef SYD_OS_POSIX
//#include <math.h>
#endif

#include "Os/Module.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	CLASS
//	Os::Math -- 算術を表す名前空間
//
//	NOTES

namespace Math
{
	// 対数を計算する
	inline
	double					log(double x);
	// 平方根を計算する
	inline
	double					sqrt(double x);
	// 乗数を計算する
	inline
	double					pow(double x, double y);
}

//	FUNCTION
//	Os::Math::log -- 対数を計算する
//
//	NOTES
//
//	ARGUMENTS
//		double				x
//			対数を計算する数
//
//	RETURN
//		与えられた数の対数
//
//	EXCEPTIONS
//		なし

inline
double
Math::log(double x)
{
	return ::log(x);
}

//	FUNCTION
//	Os::Math::sqrt -- 平方根を計算する
//
//	NOTES
//
//	ARGUMENTS
//		double				x
//			平方根を計算する数
//
//	RETURN
//		与えられた数の平方根
//
//	EXCEPTIONS
//		なし

inline
double
Math::sqrt(double x)
{
	return ::sqrt(x);
}

//	FUNCTION
//	Os::Math::pow -- 乗数を計算する
//
//	NOTES
//		x の y 乗を計算する
//
//	ARGUMENTS
//		double				x
//			底
//		double				y
//			指数
//
//	RETURN
//		x の y 乗
//
//	EXCEPTIONS
//		なし

inline
double
Math::pow(double x, double y)
{
	return ::pow(x, y);
}

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_MATH_H

//
// Copyright (c) 2000, 2002, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
