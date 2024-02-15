// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModCommonInitialize.cpp -- 汎用ライブラリーの初期化を行う
// 
// Copyright (c) 1997, 1998, 2009, 2023 Ricoh Company, Ltd.
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


//#include <stdlib.h>					// atexit
#include <iostream>					// cerr
#include "ModCommonInitialize.h"
#include "ModMessage.h"
#include "ModParameter.h"
#include "ModThread.h"
#include "ModMemoryHandle.h"
#include "ModDefaultManager.h"
#include "ModOsDriver.h"
#include "ModError.h"
#include "ModMultiByteString.h"

#ifdef MOD_DEBUG
#include "ModFakeError.h"
#endif

// 汎用ライブラリー関連のエラーメッセージの登録

#include "ModCommonException.h"
#include "ModExceptionMessage.h"
#include "ModCommonExceptionMessage.h"

using namespace std;		// iostream

//
// VARIABLE private
// ModCommonInitialize::status -- 初期化されたかどうかを示す
//
// NOTES
// この変数は汎用ライブラリーの初期化が行なわれたかを示す。
// また、プログラムが終了したかどうかも示す。
//
ModCommonInitialize::Status ModCommonInitialize::status = ModCommonInitialize::novice;

//
// FUNCTION private
// ModCommonInitialize::initialize -- 初期化する
//
// NOTES
// この関数は汎用ライブラリーの初期化を行なうのに用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位の例外をそのまま送出する。
//
void
ModCommonInitialize::initialize()
{
	if (ModCommonInitialize::status == ModCommonInitialize::novice) {
		try {
			// atexitで終了処理関数を追加する
//			(void)atexit(ModCommonInitialize::terminateAtExit);

			//
			// initialize 中にこの関数が呼ばれてももう呼ばないため こ
			// こで ModTrue にしてしまっているから、以後の処理は
			// 依存関係が正しく処理されていることに注意して実装すべし。
			//
			ModCommonInitialize::status = ModCommonInitialize::initialized;

#if MOD_CONF_MEMORY_MANAGEMENT == 3
			// Heap関連
			ModOsDriver::Memory::initialize();
#endif

			// パラメーター関係
			ModParameter::initialize();

			// メッセージ関係
			ModMessageSelection::initialize();
#ifdef MOD_DEBUG
			// 疑似エラー関係
			ModFakeError::initialize();
#endif
			// エラー関係
			ModErrorHandle::initialize();

			// デバッグ環境関係
			ModDebug::initialize();

			// スレッド関係
			ModThread::initialize();

			// ModOsManager::doInitializeのかわりに以下の3行で初期化する
			// メモリ関係(メモリプールはこの中で初期化)
			ModMemoryHandle::initialize();
			ModOs::initialize();
			ModDefaultManager::doInitialize();

			ModDebugMessage << "common library initialized." << ModEndl;
		} catch (ModException& exception) {
			// これが返るということは必要な初期化はされている
			ModErrorMessage << "Initialization failed: " << exception
							<< ModEndl;
			ModCommonInitialize::status = ModCommonInitialize::exited;
		} catch (char* string) {
			cerr << "Initialization failed: " << string << endl;
			// 初期化状態を off にする
			ModCommonInitialize::status = ModCommonInitialize::exited;
		} catch (...) {
			cerr << "Initialization failed: unexpected exception." << endl;
			ModCommonInitialize::status = ModCommonInitialize::exited;
		}
	}

	// 初期化が完全に完了したことを文字コード変換クラスに通知
	ModMultiByteString::setCommonIsInitialized();
}

//
// FUNCTION private
// ModCommonInitialize::terminate -- 終了処理を行なう
//
// NOTES
// この関数は汎用ライブラリーの終了処理を行なうのに用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位の例外をそのまま送出する。
//
void
ModCommonInitialize::terminate()
{
	if (ModCommonInitialize::status == ModCommonInitialize::initialized) {
		try {
			ModDebugMessage << "common library terminating." << ModEndl;

			// メモリマネージャ
			ModDefaultManager::doTerminate();

			// 仮想OS
			ModOs::terminate();

			// メモリ関係
			ModMemoryHandle::terminate();

			// スレッド関係
			ModThread::terminate();

			// デバッグ環境関係
			ModDebug::terminate();
#ifdef MOD_DEBUG
			// 疑似エラー関係
			ModFakeError::terminate();
#endif
			// メッセージ関係
			ModMessageSelection::terminate();

			// パラメーター関係
			ModParameter::terminate();

#if MOD_CONF_MEMORY_MANAGEMENT == 3
			// Heap関連
			ModOsDriver::Memory::terminate();
#endif

			ModCommonInitialize::status = ModCommonInitialize::novice;
		} catch (char* string) {
			cerr << "Termination failed: " << string << endl;
			// terminationのエラーなのでエラー状態の解除はしない
		} catch (...) {
			cerr << "Termination failed: unexpected exception." << endl;
			// terminationのエラーなのでエラー状態の解除はしない
		}
	}
}

//
// FUNCTION private
// ModCommonInitialize::terminateAtExit -- プログラム終了時の終了処理
//
// NOTES
// この関数は汎用ライブラリーのプログラム終了に伴う終了処理を行なうのに用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// 下位の例外をそのまま送出する。
//
void
ModCommonInitialize::terminateAtExit()
{
	ModCommonInitialize::terminate();

	ModCommonInitialize::status = ModCommonInitialize::exited;
}

//
// Copyright (c) 1997, 1998, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
