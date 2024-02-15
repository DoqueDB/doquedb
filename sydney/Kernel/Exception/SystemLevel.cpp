// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemLevel.cpp -- システムレベルの例外クラス
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

namespace {
}


#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Exception/SystemLevel.h"
							
_TRMEISTER_USING
_TRMEISTER_EXCEPTION_USING

//
//	FUNCTION public
//	Exception::SystemLevel::SystemLevel -- コンストラクタ(1)
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	OBJECTS
//	なし
//
SystemLevel::SystemLevel()
{
}

//
//	FUNCTION public
//	Exception::SystemLevel::SystemLevel -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Exception::ErrorNumber::Type uiErrorNumber_
//		エラー番号
//
//	RETURN
//	なし
//
//	OBJECTS
//	なし
//
SystemLevel::SystemLevel(ErrorNumber::Type uiErrorNumber_)
	: Object(uiErrorNumber_)
{
}

//
//	FUNCTION public
//	Exception::SystemLevel::SystemLevel -- コンストラクタ(3)
//
//	NOTES
//	コンストラクタ。引数をデータメンバーに設定する。
//
//	ARGUMENTS
//	unsigned int uiErrorNumber_
//		エラー番号(メッセージ番号)
//	const char* pszModuleName_
//		モジュール名
//	const char* pszFileName_
//		例外が発生したファイル名
//	int iLineNumber_
//		例外が発生した場所
//	const char* pszStateCode_
//		SQLSTATEコード
//
//	RETURN
//	なし
//
//	OBJECTS
//	なし
//
SystemLevel::SystemLevel(unsigned int uiErrorNumber_,
					 const char* pszModuleName_,
					 const char* pszFileName_,
					 int iLineNumber_,
					 const char* pszStateCode_)
	: Object(uiErrorNumber_,
			 pszModuleName_,
			 pszFileName_,
			 iLineNumber_,
			 pszStateCode_)
{
}

//
//	FUNCTION public
//	Exception::SystemLevel::SystemLevel -- コンストラクタ(4)
//
//	NOTES
//	コンストラクタ。引数をデータメンバーに設定する。
//
//	ARGUMENTS
//	unsigned int uiErrorNumber_
//		エラー番号(メッセージ番号)
//	const ModUnicodeChar* pszModuleName_
//		モジュール名
//	const ModUnicodeChar* pszFileName_
//		例外が発生したファイル名
//	int iLineNumber_
//		例外が発生した場所
//
//	RETURN
//	なし
//
//	OBJECTS
//	なし
//
SystemLevel::SystemLevel(unsigned int uiErrorNumber_,
					 const ModUnicodeChar* pszModuleName_,
					 const ModUnicodeChar* pszFileName_,
					 int iLineNumber_,
					 const char* pszStateCode_)
	: Object(uiErrorNumber_,
			 pszModuleName_,
			 pszFileName_,
			 iLineNumber_,
			 pszStateCode_)
{
}

//
//	FUNCTION public
//	Exception::SystemLevel::SystemLevel -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Exception::Object& cObject_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
SystemLevel::SystemLevel(const Exception::Object& cObject_,
						 const char* pszStateCode_)
	: Object(cObject_, pszStateCode_)
{
}

//
//	Copyright (c) 2005, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
