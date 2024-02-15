// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemLevel.h -- システムレベル例外クラス
// 
// Copyright (c) 2005, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_EXCEPTION_SYSTEMLEVEL_H
#define __TRMEISTER_EXCEPTION_SYSTEMLEVEL_H

#include "Exception/Object.h"

_TRMEISTER_BEGIN
_TRMEISTER_EXCEPTION_BEGIN

//
//	CLASS
//	SystemLevel -- システムレベルの例外をあらわすクラス
//
//	NOTES
//
class SYD_EXCEPTION_FUNCTION SystemLevel : public Object
{
public:
	//コンストラクタ(1)
	SystemLevel();
	//コンストラクタ(2)
	SystemLevel(ErrorNumber::Type uiErrorNumber_);		//エラー番号
	//コンストラクタ(3)
	SystemLevel(ErrorNumber::Type uiErrorNumber_,			//エラー番号
			  const char* pszModuleName_,				//モジュール名
			  const char* pszFileName_,					//ファイル名
			  int iLineNumber_,							//行番号
			  const char* pszStateCode_);				//SQLSTATEコード
	//コンストラクタ(4)
	SystemLevel(ErrorNumber::Type uiErrorNumber_,			//エラー番号
			  const ModUnicodeChar* pszModuleName_,		//モジュール名
			  const ModUnicodeChar* pszFileName_,		//ファイル名
			  int iLineNumber,							//行番号
			  const char* pszStateCode_);				//SQLSTATEコード

	//コピーコンストラクタ
	SystemLevel(const Object& cObject_,
				const char* pszStateCode_);				//SQLSTATEコード

};

_TRMEISTER_EXCEPTION_END
_TRMEISTER_END

#endif //__TRMEISTER_EXCEPTION_SYSTEMLEVEL_H
//
//	Copyright (c) 2005, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
