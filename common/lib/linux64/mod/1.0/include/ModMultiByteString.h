// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModMultiByteString.h -- マルチバイト文字のクラス定義
// 
// Copyright (c) 2000, 2002, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModMultiByteString_H__
#define __ModMultiByteString_H__

#include "ModCommonDLL.h"
#include "ModCommon.h"

//	CLASS
//	ModMultiByteString -- マルチバイト文字列の領域
//
//	NOTES
// このクラスは MOD の初期化が完了する前にインスタンス化される事に注意！
// したがって、このクラスのメソッドから安心して呼び出せるメソッドは
// 「一切メモリのアロケートを行なわないメソッド」であり、 
// 「MOD のメッセージを一切呼び出さないメソッド」である。
// 

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModMultiByteString
{
public:
	ModCommonDLL
	ModMultiByteString(const char*							src,
					   const ModKanjiCode::KanjiCodeType	srcKanjiCode,
					   const ModKanjiCode::KanjiCodeType	bufferKanjiCode,
					   const ModBoolean						useAllocatedMemory = ModFalse);
	ModCommonDLL
	~ModMultiByteString();

	// 内部バッファを返す
	ModCommonDLL
	const char* get() const;

	// common の初期化が完全に終了した事を通知。
	ModCommonDLL
	static void setCommonIsInitialized();

private:

	char* transfer(char*								outString,
				   const int							outBytes,
				   const ModKanjiCode::KanjiCodeType	outType,
				   const char*							inString,
				   const ModKanjiCode::KanjiCodeType	inType);

	ModSize getTransferredSize(
		const ModKanjiCode::KanjiCodeType	outType,
		const char*							inString,
		const ModKanjiCode::KanjiCodeType	inType);

	int getJJCodeType(const ModKanjiCode::KanjiCodeType codeType);

	// マルチバイト文字列のバッファのバイト数
	ModSize				d_size;

	// bufferKanjiCode で指定された文字コードのマルチバイト文字列の
	// バッファ (終端文字で終っていることを保証する)
	char*				d_buffer;

	// staticだったが、これではこのクラスがsingletonになってしまうので、
	// メンバー変数とした
	enum {				s_staticSize = 256 };
	char				s_staticBuffer[s_staticSize];
	
	//
	// STATIC なデータメンバ
	//

	static ModBoolean	s_CommonIsInitialized;

};

#endif	// __ModMultiByteString_H__

//
// Copyright (c) 2000, 2002, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
