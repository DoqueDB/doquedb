// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Exteralizable.cpp -- シリアル化可能クラス
// 
// Copyright (c) 1999, 2001, 2004, 2012, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/Externalizable.h"
#include "Common/UnicodeString.h"
#include "Common/Message.h"

_TRMEISTER_USING
using namespace Common;

namespace
{
	//
	//	TYPEDEF
	//
	typedef Externalizable* (*func)(int);

	//
	//	VARIABLE local
	//	_$$::_functionArray -- クラスインスタンスを得るための関数の配列
	//
	func _functionArray[Externalizable::NumberOfValue / 1000];
}

//
//	FUNCTION public
//	Common::Externalizable::Externalizable -- コンストラクタ
//
//	NOTES
//	シリアル化可能クラスのコンストラクタ。
//	今のところ何も行わない。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

Externalizable::Externalizable()
{
}

//
//	FUNCTION public
//	Common::Externalizable::~Externalizable -- デストラクタ
//
//	NOTES
//	シリアル化可能クラスのデストラクタ。
//	今のところ何も行わない。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

Externalizable::~Externalizable()
{
}

//
//	FUNCTION public static
//	Common::Externalizable::getClassInstance -- クラスのインスタンスを得る
//
//	NOTES
//	クラスIDに対応したすべてのモジュールのシリアル化可能クラスの
//	インスタンスを得る。
//
//	ARGUMENTS
//	int iClassID_
//		クラスID
//
//	RETURN
//	Common::Externalizable*
//		クラスのインスタンス。
//		存在しないクラスIDの場合は0を返す。
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Externalizable*
Externalizable::getClassInstance(int iClassID_)
{
	if (iClassID_ < 0 || iClassID_ >= NumberOfValue)
		return 0;
	if (_functionArray[iClassID_ / 1000] == 0)
		return 0;
	return _functionArray[iClassID_ / 1000](iClassID_);
}

//
//	FUNCTION public static
//	Common::Externalizable::insertFunction
//							-- クラスのインスタンスを得る関数を登録する
//
//	NOTES
//	各モジュールのクラスのインスタンスを得る関数を登録する。
//
//	ARGUMENTS
//	int iClassID_
//		クラスID
//	Common::Externalizable* (*pFunc_)(int)
//		インスタンスを確保する関数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Externalizable::insertFunction(int iClassID_,
							   Externalizable* (*pFunc_)(int))
{
	_functionArray[iClassID_ / 1000] = pFunc_;
}

//
//	Copyright (c) 1999, 2001, 2004, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
