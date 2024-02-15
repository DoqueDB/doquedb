// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModOsException.h -- 汎用ライブラリーの下位での例外関連の定義
// 
// Copyright (c) 1998, 1999, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModOsException_H__
#define __ModOsException_H__

#include "ModError.h"
#include "ModExceptionMessage.h"
#include "ModCommonException.h"
#include "ModThread.h"

//	MACRO
//	ModThrowOs -- OS 関連の例外を送出する
//
//	NOTES
//		スレッド関連の初期化が行われていないようならば、
//		まず、スレッド関連の初期化をしてから ModException を送出する

#define	ModThrowOs(n, l, e)		ModThrow2(ModModuleOs, n, l, e)

//	MACRO
//	ModUnexpectedThrowOs -- OS 関連の予期しない例外を送出する
//
//	NOTES

#define	ModUnexpectedThrowOs() 	ModUnexpectedThrow(ModModuleOs)

//	MACRO
//	ModThrowOsWarning -- OS 関連の Warning レベルの例外を送出する
//
//	NOTES

#define	ModThrowOsWarning(n)	ModThrowOs(n, ModErrorLevelWarning, 0)

//	MACRO
//	ModThrowOsError -- OS 関連の Error レベルの例外を送出する
//
//	NOTES

#define	ModThrowOsError(n)		ModThrowOs(n, ModErrorLevelError, 0)

//	MACRO
//	ModThrowOsFatal -- OS 関連の Fatal レベルの例外を送出する
//
//	NOTES

#define	ModThrowOsFatal(n)		ModThrowOs(n, ModErrorLevelFatal, 0)

//	MACRO
//	ModMessageThrow -- エラー番号に対応した文字列の例外を送出する
//
//	NOTES

#define	ModMessageThrow(m, n)									\
{																\
	throw ModExceptionMessage::getMessage(m, n);				\
}

//	MACRO
//	ModMessageThrowOs -- OS 関連の例外を初期化状態に応じて送出する
//
//	NOTES
//		スレッド関連の初期化が行われていないようならば、
//		与えられたエラー番号に対応した文字列の例外を送出し、
//		初期化されているようならば、ModException を送出する

#define	ModMessageThrowOs(n, l)									\
{																\
	if (ModThread::isInitialized() == ModTrue) {				\
		ModThrow(ModModuleOs, n, l);							\
	} else {													\
		ModMessageThrow(ModModuleOs, n);						\
	}															\
}

//	MACRO
//	ModMessageThrowOsWarning --
//		OS 関連の Warning レベルの例外を初期化状態に応じて送出する
//
//	NOTES

#define	ModMessageThrowOsWarning(n)	ModMessageThrowOs(n, ModErrorLevelWarning)

//	MACRO
//	ModMessageThrowOsError --
//		OS 関連の Error レベルの例外を初期化状態に応じて送出する
//
//	NOTES

#define	ModMessageThrowOsError(n)	ModMessageThrowOs(n, ModErrorLevelError)

//	MACRO
//	ModThrowOsFatal --
//		OS 関連の Fatal レベルの例外を初期化状態に応じて送出する
//
//	NOTES

#define	ModMessageThrowOsFatal(n)	ModThrowOs(n, ModErrorLevelFatal, 0)

#endif // __ModOsException_H__

//
// Copyright (c) 1998, 1999, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
