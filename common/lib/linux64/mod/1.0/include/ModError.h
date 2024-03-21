// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModError.h --- エラー処理に関する宣言、定義
// 
// Copyright (c) 1997, 2002, 2003, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __ModError_H__
#define __ModError_H__

#include "ModCommonDLL.h"
#include "ModTypes.h"
#include "ModMessage.h"
#include "ModException.h"

// 
// CLASS 
// ModErrorHandle -- エラーに関する関数を集めたクラス
//
// NOTES
//	エラー関係の関数を集めたクラスである。
//	ModErrorHandle::xxx()の形式で呼び出す。
//	enum ModStatusのメンバにModErrorが存在し、バッテイングするので改名した。
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModErrorHandle
{
	friend class ModCommonInitialize;
public:
	// 例外オブジェクトへの参照を得る。ModThreadの関数を直接呼び出さないため。
	ModCommonDLL
	static ModException* getException();
	// 実行スレッドのエラー状態をリセット
	ModCommonDLL
	static void reset();
	// 実行スレッドのエラー状態を設定
	ModCommonDLL
	static void setError();
	// 実行スレッドのエラー状態を得る
	ModCommonDLL
	static ModBoolean isError();
#ifdef DEBUG
	// ModAssertからDEBUG時のみ呼び出されるためのブレークポイント用関数
	ModCommonDLL
	static void assertCheck();
#endif
	// バックトレースメッセージを出力すべきか
	ModCommonDLL
	static ModBoolean shouldOutputMessage(ModErrorLevel level);
	// 送出時の出力メッセージのエラーレベルの上限を設定する
	ModCommonDLL
	static void setOutputLevel(ModErrorLevel level);

private:
	// インスタンスは作らないので宣言して作らない
	ModErrorHandle();
	~ModErrorHandle();

	// 出力エラーレベル設定の初期化
	static void initialize();
	// 例外送出時にメッセージ出力するエラーレベルの上限
	ModCommonDLL
	static ModErrorLevel outputLevel;
};

//
// MACRO
// ModThrow() -- 新たにModExceptionを送出する
//
// NOTES
// 引数にはModException::setError()の引数を指定する。以下の動作をする
//
// (1) 使用すべきModExceptionを得てエラー状態と内容をセット、
// (2) エラーレベルが出力設定エラーレベルよりも深刻な場合のみ、エラー内容を
//		エラーメッセージとして出力する。出力設定エラーレベルはパラメータで
//		設定できるが、デフォルトではModErrorLevelFatalである。
// (3) throw
//

#define ModThrow2(module, number, level, error)							\
{																		\
	ModException* exception__ = ModErrorHandle::getException();			\
	if (exception__) {													\
		exception__->setError(module, number, level, error);			\
		if (ModErrorHandle::shouldOutputMessage(level)) {				\
			ModErrorMessage << *exception__ << ModEndl;					\
		}																\
		exception__->setThrowInfo(__LINE__, __FILE__);					\
		throw *exception__;												\
	} else {															\
		ModException e__;												\
		e__.setError(module, number, level, error);						\
		if (ModErrorHandle::shouldOutputMessage(level)) {				\
			ModErrorMessage << e__ << ModEndl;							\
		}																\
		e__.setThrowInfo(__LINE__, __FILE__);							\
		throw e__;														\
	}																	\
}

#define ModThrow(module, number, level)			\
	ModThrow2(module, number, level, 0)

//
// MACRO
// ModRethrow() -- 下位から受け取ったものを上位に対してそのまま送出
//
// NOTES
// exceptionには catch(ModException& exception)で受け取った
// 例外オブジェクトを渡す。
// ログを残す必要があるので、直接throwを呼び出してはいけない。
// エラーレベルが出力設定エラーレベルよりも深刻な場合のみ、エラーメッセージと
// して出力する。出力設定エラーレベルはパラメータで設定できるが、
// デフォルトではModErrorLevelFatalである。
//

#define ModRethrow(exception)											\
{																		\
	if (ModErrorHandle::shouldOutputMessage(exception.getErrorLevel())) {	\
	    ModErrorMessage << exception << ModEndl;						\
    }																	\
    throw;																\
}

//
// MACRO
// ModUnexpectedThrow() -- 予期しないエラーをModException&として送出
//
// NOTES
// MOD以外の関数から送出された例外が何かわからない場合でも、
// とりあえず水際で止めて、MODの例外にセットして送出する。
// これが呼び出される場合はバグである。
//

#define ModUnexpectedThrow(module)										\
	ModThrow(module, ModCommonErrorUnexpected, ModErrorLevelFatal)

//
// MACRO
// ModMemoryErrorThrow() -- メモリエラーのModExceptionを送出する
//
// NOTES
//	エラー状態を設定しない点がModThrowとは異なる。あとは同じ。
//

#define ModMemoryErrorThrow(module, number, level, error)				\
{																		\
	ModException* exception__ = ModErrorHandle::getException();			\
	if (exception__) {													\
		exception__->setError(module, number, level, error, ModFalse);	\
		if (ModErrorHandle::shouldOutputMessage(level)) {				\
			ModErrorMessage << *exception__ << ModEndl;					\
		}																\
		exception__->setThrowInfo(__LINE__, __FILE__);					\
		throw *exception__;												\
	} else {															\
		ModException e__;												\
		e__.setError(module, number, level, error, ModFalse);			\
		if (ModErrorHandle::shouldOutputMessage(level)) {				\
			ModErrorMessage << e__ << ModEndl;							\
		}																\
		e__.setThrowInfo(__LINE__, __FILE__);							\
		throw e__;														\
	}																	\
}

//
// FUNCTION public
// ModErrorHandle::setErrorLevel -- 例外発生出力メッセージのエラーレベルの上限を設定する//
// NOTES
//	例外送出時に出力すべきエラーメッセージのエラーレベルを設定する。
//	設定値より深刻なエラーレベルはすべて出力される。
//
// ARGUMENTS
//	ModErrorLevel	level
//		設定する例外のエラーレベル
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//

// static
inline
void
ModErrorHandle::setOutputLevel(ModErrorLevel level)
{
	ModErrorHandle::outputLevel = level;
}

#endif	// __ModError_H__

//
// Copyright (c) 1997, 2002, 2003, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
