// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Data.cpp -- Data の定義ファイル
// 
// Copyright (c) 2005, 2009, 2023 Ricoh Company, Ltd.
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

#include "LibUna/Data.h"

_UNA_USING

_DATA_USING

//
// FUNCTION public
//	Object::Object
//		-- Object クラスのコンストラクタ
//
// NOTES
//		デフォルトのコンストラクタは使用しない
//
// ARGUMENTS
//		Data::Type データ種別
//
// RETURN
//	なし
//
// EXCEPTIONS
//
Object::Object(Object::Type type_)
	 : _type(type_)
{
}

//
// FUNCTION public
//	Object::~Object
//		-- Object クラスのデストラクタ
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//
Object::~Object()
{
}

#ifdef OBSOLETE
//
// FUNCTION public
//	Object::getType
//		-- データ種別を取得する
//
// NOTES
//
// ARGUMENTS
//		なし
//
// RETURN
//		Object::Type データ種別
//
// EXCEPTIONS
//
Object::Type
Object::getType() const
{
	return _type;
}
#endif

//
// Copyright (c) 2005, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
