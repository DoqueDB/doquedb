// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Message.h -- エラーメッセージを作成するクラス
// 
// Copyright (c) 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_EXCEPTION_MESSAGE_H
#define __TRMEISTER_EXCEPTION_MESSAGE_H

#include "Exception/Module.h"

class ModMessageStream;
class ModOstream;

_TRMEISTER_BEGIN
_TRMEISTER_EXCEPTION_BEGIN

class Object;

_TRMEISTER_EXCEPTION_END
_TRMEISTER_END

// 例外クラスをメッセージに出力する関数の宣言
// グローバル名前空間で宣言する

// 例外を ModMessage に書出す
SYD_EXCEPTION_FUNCTION
ModMessageStream&
operator <<(ModMessageStream& cStream,
			const _TRMEISTER::Exception::Object& cObject);

// 例外を MessageStream に書出す
SYD_EXCEPTION_FUNCTION
ModOstream&
operator<<(ModOstream& cStream,
		   const _TRMEISTER::Exception::Object& cObject);

#endif //__TRMEISTER_EXCEPTION_MESSAGE_H

//
//	Copyright (c) 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
