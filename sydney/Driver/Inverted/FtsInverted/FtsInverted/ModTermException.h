// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
//	ModTermException.h --- 転置ファイルモジュールにおける例外の定義
// 
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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

#ifndef __ModTermException_H__
#define __ModTermException_H__

#include "ModError.h"

//
// MACRO
// ModThrowTermRetry -- Term関連のRetryレベルの例外を送出する
//
// NOTES
// Term関連のRetryレベルの例外を送出する。
//
#define	ModThrowTermRetry(n) \
	ModThrow(ModModuleTerm, n, ModErrorLevelRetry)

//
// MACRO
// ModThrowTermOk -- Term関連のOkレベルの例外を送出する
//
// NOTES
// 転置Term関連のOkレベルの例外を送出する。
//
#define ModThrowTermOk(n) \
	ModThrow(ModModuleTerm, n, ModErrorLevelOk)

//
// MACRO
// ModThrowTermWarning -- Term関連のWarningレベルの例外を送出する
//
// NOTES
// Term関連のWarningレベルの例外を送出する。
//
#define ModThrowTermWarning(n) \
	ModThrow(ModModuleTerm, n, ModErrorLevelWarning)

//
// MACRO
// ModThrowTermError -- Term関連のErrorレベルの例外を送出する
//
// NOTES
// Term関連のErrorレベルの例外を送出する。
//
#define ModThrowTermError(n) \
	ModThrow(ModModuleTerm, n, ModErrorLevelError)

//
// MACRO
// ModThrowTermFatal -- Term関連のFatalレベルの例外を送出する
//
// NOTES
// Term関連のFatalレベルの例外を送出する。
//
#define ModThrowTermFatal(n) \
	ModThrow(ModModuleTerm, n, ModErrorLevelFatal)

//
// ENUM
// ModTermErrorNumber -- メモリ管理モジュール専用のエラー番号
//
// NOTES
// 転置ファイルモジュール(ModModuleInverted)で利用するエラー番号である。
//

enum ModTermErrorNumber{
//	ModTermErrorXXX								= 70000,
	ModTermErrorFileOpenFail					= 70001,
	ModTermErrorFileReadFail					= 70002,
  	ModTermErrorPoolSize						= 70003,
	ModTermErrorRxCompile						= 70004,
	
	// 最後におくこと -- 実際には未使用
	ModTermErrorEnd								= 70999
};


//
// エラーメッセージのための定義
//
#include "ModExceptionMessage.h"
extern ModExceptionMessageAssoc ModModuleTermMessageArray[];
static ModExceptionMessage termMessage(ModModuleTerm,
									   ModModuleTermMessageArray);

#endif // __ModTermException_H__

//
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
