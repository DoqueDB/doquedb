// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
//	ModExceptionMessage.cpp --- 例外にメッセージを結び付ける
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


extern "C" {
#include <stdio.h>
#include <string.h>
}
#include "ModExceptionMessage.h"
#include "ModCommonException.h"

namespace
{
	// CONST
	// _noImplementedMessage --
	//
	// NOTES
	const char* const _noImplementedMessage = "(message not implemented)";
}

//
// VARIABLE static
// ModExceptionMessage::moduleName -- 番号とモジュール名の対応
//
// NOTES
// メッセージでモジュール番号からモジュール名を引くために用いる。
//
ModNumberNameAssoc ModExceptionMessage::moduleName[] = {
	{ ModModuleMemory, "Memory" },
	{ ModModuleOs, "Os" },
	{ ModModuleStandard, "Standard" },

	{ ModModuleBuffer, "Buffer" },
	{ ModModuleLog, "Log" },
	{ ModModuleLogicalFile, "LogicalFile" },
	{ ModModulePhysicalFile, "PhysicalFile" },
	{ ModModuleRequest, "Request" },
	{ ModModuleTransaction, "Transaction" },
	{ ModModuleLock, "Lock" },
	{ ModModuleSystem, "System" },

	{ ModModuleHashFile, "Hash" },
	{ ModModuleHeapFile, "Heap" },
	{ ModModuleInvertedFile, "Inverted" },
	{ ModModuleVectorFile, "Vector" },

	{ ModModuleAppServer, "AppServer" },
	{ ModModuleAppClient, "AppClient" },

	{ ModModuleUndefined, "Undefined" }
};

//
// VARIABLE static
// ModExceptionMessage::array -- エラー番号とメッセージのマップ
//
// NOTES
// エラー番号からエラーメッセージを引くマップ
//
ModExceptionMessageAssoc* ModExceptionMessage::array[ModModuleMax];

//
// FUNCTION
// ModExceptionMessage::ModExceptionMessage -- コンストラクタ
//
// NOTES
// エラーメッセージ配列をマップに入れる
//
// ARGUMENTS
// ModModule module
//		モジュール番号
// ModExceptionMessageAssoc* array
//		エラーメッセージ配列
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
// その他
// 
//

ModExceptionMessage::ModExceptionMessage(ModModule module,
										 ModExceptionMessageAssoc* array_)
{
	_module = module;
	if (ModExceptionMessage::array[_module] == 0)
		ModExceptionMessage::array[_module] = array_;
}

//	FUNCTION public
//	ModExceptionMessage::getMessage --
//		エラー番号に対応するメッセージ文字列を得る
//
//	NOTES
//
//	ARGUMENTS
//		ModErrorNumber		n
//			メッセージ文字列を得たいエラーのエラー番号
//		ModModule			m
//			エラーを登録したモジュール
//			
//	RETURN
//		0 以外の値
//			メッセージ文字列の先頭アドレス
//		0
//			対応するメッセージ文字列は登録されていない
//
//	EXCEPTIONS
//		なし

const char*
ModExceptionMessage::getMessage(ModErrorNumber n) const
{
	return ModExceptionMessage::getMessage(_module, n);
}

// static
const char*
ModExceptionMessage::getMessage(ModModule m, ModErrorNumber n)
{
	ModExceptionMessageAssoc*	assoc = ModExceptionMessage::array[m];
	for (; assoc && assoc->number; assoc++)
		if (assoc->number == n)
			return assoc->message;

	return (m == ModModuleOs) ?
		0 : ModExceptionMessage::getMessage(ModModuleOs, n);
}

// FUNCTION
// ModExceptionMessage::setMessage -- 例外のメッセージを得る
//
// NOTES
// 例外から対応するエラーメッセージを得る
//
// ARGUMENTS
// const ModException& exception
//		例外
// char* buffer
//		セットするバッファ
//
// RETURN
// なし

void
ModExceptionMessage::setMessage(const ModException& exception, char* buffer)
{
	ModModule		module = exception.getErrorModule();
	ModErrorNumber	errorNumber = exception.getErrorNumber();
	ModErrorLevel	errorLevel = exception.getErrorLevel();
	const char*		path = exception.getPath();
	int				lineNumber = exception.getLineNumber();
	int				osError = exception.getOsError();

	const char*	levelString = ((errorLevel == ModErrorLevelError) ? "ERROR" :
							   (errorLevel == ModErrorLevelFatal) ? "FATAL" :
							   (errorLevel == ModErrorLevelOk) ? "OK" :
							   (errorLevel == ModErrorLevelRetry) ? "RETRY" :
							   (errorLevel == ModErrorLevelWarning) ?
							   "WARNING" : "UNKNOWN");

	// モジュール名を得る

	const ModNumberNameAssoc* assoc = ModExceptionMessage::moduleName;
	for (; assoc->module != module &&
		   assoc->module != ModModuleUndefined; assoc++);

	if (errorNumber > ModOsErrorUndefined &&
		errorNumber < ModOsErrorOtherReason) {

		// 仮想 OS ドライバーのエラーメッセージを生成する

		if (*path) {
			if (osError) {
				::sprintf(buffer, "%s [%s, %d(%d), %s] (%s %d)",
						  ::strerror(errorNumber - ModOsErrorUndefined),
						  assoc->name, errorNumber, osError, levelString,
						  path, lineNumber);
			} else {
				::sprintf(buffer, "%s [%s, %d, %s] (%s %d)",
						  ::strerror(errorNumber - ModOsErrorUndefined),
						  assoc->name, errorNumber, levelString,
						  path, lineNumber);
			}
		} else {
			if (osError) {
				::sprintf(buffer, "%s [%s, %d(%d), %s]",
						  ::strerror(errorNumber - ModOsErrorUndefined),
						  assoc->name, errorNumber, osError, levelString);
			} else {
				::sprintf(buffer, "%s [%s, %d, %s]",
						  ::strerror(errorNumber - ModOsErrorUndefined),
						  assoc->name, errorNumber, levelString);
			}
		}
		return;
	}

	// メッセージ配列を探索する

	const char* message = ModExceptionMessage::getMessage(module, errorNumber);
	if (!message) {
		message = _noImplementedMessage;
	}
	if (*path) {
		if (osError) {
			::sprintf(buffer, "%s [%s, %d(%d), %s] (%s %d)",
					  message, assoc->name, errorNumber, osError, levelString,
					  path, lineNumber);
		} else {
			::sprintf(buffer, "%s [%s, %d, %s] (%s %d)",
					  message, assoc->name, errorNumber, levelString,
					  path, lineNumber);
		}
	} else {
		if (osError) {
			::sprintf(buffer, "%s [%s, %d(%d), %s]",
					  message, assoc->name, errorNumber, osError, levelString);
		} else {
			::sprintf(buffer, "%s [%s, %d, %s]",
					  message, assoc->name, errorNumber, levelString);
		}
	}
}

//
// Copyright (c) 1998, 1999, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
