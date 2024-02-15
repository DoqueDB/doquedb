// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModDebug.h -- デバッグ関連のクラス定義
// 
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModDebug_H__
#define __ModDebug_H__

#include "ModCommonDLL.h"
#include "ModTypes.h"

//	CLASS
//	ModDebug -- デバッグに関する内容を管理するクラス
//
//	NOTES
//		デバッグ時の動作を規定する

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModDebug
{
public:
	ModCommonDLL
	static void				initialize();		// 初期化する
	static void				terminate();		// 後処理する

	ModCommonDLL
	static ModBoolean		check;
	ModCommonDLL
	static ModBoolean		assertingException;
};

//	FUNCTION public
//	ModDebug::terminate -- デバッグ環境の後処理
//
//	NOTES
//		とくになにもしない
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
inline
void
ModDebug::terminate()
{ }

#endif	// __ModDebug_H__

//
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
