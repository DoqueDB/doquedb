// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ErrorMessage.h -- エラーメッセージを作成するクラス
// 
// Copyright (c) 1999, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_EXCEPTION_ERRORMESSAGE_H
#define __TRMEISTER_EXCEPTION_ERRORMESSAGE_H

#include "Exception/Module.h"
#include "Exception/ErrorNumber.h"

#include "ModUnicodeChar.h"

_TRMEISTER_BEGIN
_TRMEISTER_EXCEPTION_BEGIN

//
//	NAMESPACE
//	Exception::ErrorMessage -- エラーメッセージの引数を管理する関数群
//
//	NOTES

namespace ErrorMessage
{
	//エラーメッセージ引数フォーマットを得る
	SYD_EXCEPTION_FUNCTION
	const ModUnicodeChar* getArgumentFormat(ErrorNumber::Type uiErrorNumber_);
	//エラーメッセージ引数の数を得る
	SYD_EXCEPTION_FUNCTION
	int getArgumentNumber(ErrorNumber::Type uiErrorNumber_);

	//エラーメッセージを作成するための引数を作成する
	SYD_EXCEPTION_FUNCTION
	ModUnicodeChar* makeMessageArgument(ModUnicodeChar* pszArgument_,
										ErrorNumber::Type iErrorNumber_, ...);

	//エラーメッセージ引数の要素を取出す
	SYD_EXCEPTION_FUNCTION
	const ModUnicodeChar* getMessageArgumentElement(
									const ModUnicodeChar* 	pszArgument_,
									int 					iElement_);
}

_TRMEISTER_EXCEPTION_END
_TRMEISTER_END

#endif //__TRMEISTER_EXCEPTION_ERRORMESSAGE_H

//
//	Copyright (c) 1999, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

