// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModDebug.cpp -- デバッグ関連のメソッドの定義
// 
// Copyright (c) 1997, 1999, 2023 Ricoh Company, Ltd.
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


#include "ModDebug.h"
#include "ModError.h"
#include "ModParameter.h"

//	VARIABLE 
//	ModDebug::check -- デバッグ用の検査を行うか
//
//	NOTES
//		デバッグ用の以下の検査を行うかを表す
//
//		*	ModAssert の実行
//		*	~ModMemoryHandle で未解放領域の有無の検査

ModBoolean ModDebug::check = ModTrue;

//	VARIABLE
//	ModDebug::assertingException --
//		ModAssert の実行で評価結果が偽になったときに例外を発生するか
//
//	NOTES
//		ModAssert の実行で評価結果が偽になったときに例外を発生するかを表す
//		ModTrue のとき、例外を発生し、ModFalse のとき、::abort を実行する

ModBoolean	ModDebug::assertingException = ModFalse;

//	FUNCTION public
//	ModDebug::initialize -- デバッグ環境の初期化
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
ModDebug::initialize()
{
	try {
		// パラメーターの検査

		ModParameter	parameter(ModFalse);
		ModBoolean		boolean;

		if (parameter.getBoolean(boolean, "DebugCheck") == ModTrue)
			ModDebug::check = boolean;
		if (parameter.getBoolean(boolean, "AssertingException") == ModTrue)
			ModDebug::assertingException = boolean;

	} catch (...) {
		ModErrorHandle::reset();
	}
}

//
// Copyright (c) 1997, 1999, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
