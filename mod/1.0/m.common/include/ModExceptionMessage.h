// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
//	ModExceptionMessage.h --- 例外にメッセージを結び付ける
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

#ifndef __ModExceptionMessage_H__
#define __ModExceptionMessage_H__

#include "ModCommonDLL.h"
#include "ModException.h"

//
// TYPEDEF
// ModExceptionMessage
//
// NOTES
// ModExceptionに対応するメッセージを引くためのマップの要素になる型
//

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

typedef struct
{
	ModErrorNumber			number;
	const char*				message;

} ModExceptionMessageAssoc;

//
// TYPEDEF
// ModNumberNameAssoc -- 番号と名前の対応
//
// NOTES
// メッセージでモジュール番号などからモジュール名などを引くために用いる。
//

//【注意】	ライブラリ外に公開しないクラスなので dllexport しない

typedef struct
{
	ModModule				module;
	const char*				name;

} ModNumberNameAssoc;

//
// CLASS
// ModExceptionMessage
//
// NOTES
// ModExceptionに対応するメッセージを保持するためのクラス
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModExceptionMessage
{
	friend class ModException;
public:
	ModCommonDLL
	ModExceptionMessage(ModModule module,
						ModExceptionMessageAssoc* messageArray);

	ModCommonDLL
	const char*				getMessage(ModErrorNumber n) const;
	ModCommonDLL
	static const char*		getMessage(ModModule m, ModErrorNumber n);

private:
	// 以下の関数は使わないので宣言して実装しない
	ModExceptionMessage();
	ModExceptionMessage(const ModExceptionMessage& other);
	ModExceptionMessage& operator=(const ModExceptionMessage& other);

	// この関数は ModException からしか使われない
	static void setMessage(const ModException& exception, char* buffer);

	ModModule				_module;		// 生成したモジュール

	static ModNumberNameAssoc moduleName[];	// モジュール名配列

	static ModExceptionMessageAssoc* array[ModModuleMax];
										// メッセージ配列へのポインタ
};

#endif	// __ModExceptionMessage_H__

//
// Copyright (c) 1998, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
