// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataOperation.h -- 演算の種類
// 
// Copyright (c) 2000, 2001, 2004, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_DATAOPERATION_H
#define __TRMEISTER_COMMON_DATAOPERATION_H

#include "Common/Common.h"

_TRMEISTER_BEGIN

namespace Common
{

namespace DataOperation
{
	//
	//	ENUM
	//	Common::DataOperation::Type -- 演算種別
	//
	//	NOTES
	//	演算種別をあらわす列挙子
	//
	enum Type
	{
		DyadicStart = 0,
		Addition = DyadicStart,	//足し算
		Subtraction,		//引き算
		Multiplication,		//掛け算
		Division,			//割り算
		Modulus,			//余り

		MonadicStart = 100,
		Increment = MonadicStart,//インクリメント
		Decrement,			//デクリメント
		Negation,			//符号反転
		AbsoluteValue,		//絶対値

		Unknown
	};
	
}

}

_TRMEISTER_END

#endif //__TRMEISTER_COMMON_DATAOPERATION_H

//
//	Copyright (c) 2000, 2001, 2004, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
