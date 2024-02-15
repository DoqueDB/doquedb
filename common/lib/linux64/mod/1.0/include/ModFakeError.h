// -*-Mode: C; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4;	
//
// ModFakeError.h -- ModFakeError のクラス定義
// 
// Copyright (c) 1998, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModFakeError_H__
#define __ModFakeError_H__

#include "ModCommonDLL.h"
#include "ModTypes.h"

//
// CLASS
// ModFakeError -- 疑似エラー発生のためのクラス
//
// NOTES
// このクラスはmallocやfreeなどのエラーを起こしにくい関数のエラー処理を
// テストするために嘘のエラーを発生させる機構のために用いる。
//
// 他のモジュールに依存しないようにするため、ModObject のサブクラスには
// しない。
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModFakeError
{
public:

	//【注意】	ライブラリ外に公開しないクラスなので dllexport しない

	struct Spec
	{
		const char* name;				// 対象の関数名
		int errorNumber;				// エラーでセットする errno
		ModUInt64 limit;				// 比較対象
		ModUInt64 count;				// 現在の呼びだし回数(work area)

		enum Operator {					// 比較オペレーター
			equal,
			less,
			greater,
			lessEqual,
			greaterEqual
		} compare;
	};

	ModCommonDLL
	static void initialize();
	ModCommonDLL
	static void terminate();
	ModCommonDLL
	static ModBoolean check(const char* name_, int errno_ = -1,
							int defaultErrno = 0);
	ModCommonDLL
	static ModUInt64 getCount(const char* name_);
	ModCommonDLL
	static void reInitialize(const char* specString_);

private:
	static void parseSpec();
	static char specString[1024];		// パラメータや環境変数の設定文字列
	static Spec spec[128];
};

#endif	// __ModFakeError_H__

//
// Copyright (c) 1998, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
